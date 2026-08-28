#include "app_vault.hpp"
#include "common_crc.hpp"
#include <psa/crypto.h>
#include <mbedtls/platform_util.h>
#include <cstring>

using namespace PocketTrustee;

template <typename T> requires std::is_trivially_copyable_v<T>
WipeGuard<T>::~WipeGuard() { mbedtls_platform_zeroize(self, sizeof(T)); }


VaultDatabaseV1::~VaultDatabaseV1() {
    mbedtls_platform_zeroize(master_key_, sizeof(master_key_));
}

uint32_t VaultDatabaseV1::ComputeCRC32(const uint8_t* data, size_t len) {
    Crc32Mpeg2 crc;
    return crc.Continue(data, len);
}

int VaultDatabaseV1::GenerateRandom(uint8_t* output, size_t len) {
    psa_status_t status = psa_generate_random(output, len);
    return (status == PSA_SUCCESS) ? 0 : -1;
}

void VaultDatabaseV1::DeriveKeyCheck(const uint8_t* salt, const uint8_t* key, uint8_t* key_check) {
    // Use HMAC-SHA256 to derive key check value
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_MESSAGE);
    psa_set_key_algorithm(&attributes, PSA_ALG_HMAC(PSA_ALG_SHA_256));
    psa_set_key_type(&attributes, PSA_KEY_TYPE_HMAC);

    psa_key_id_t key_id;
    psa_status_t status = psa_import_key(&attributes, key, 32, &key_id);
    if (status != PSA_SUCCESS) {
        mbedtls_platform_zeroize(key_check, 16);
        return;
    }

    uint8_t mac[32];
    size_t mac_len;
    status = psa_mac_compute(key_id, PSA_ALG_HMAC(PSA_ALG_SHA_256),
                             salt, 16, mac, sizeof(mac), &mac_len);

    if (status == PSA_SUCCESS) {
        memcpy(key_check, mac, 16);
    }
    else {
        mbedtls_platform_zeroize(key_check, 16);
    }

    psa_destroy_key(key_id);
    mbedtls_platform_zeroize(mac, sizeof(mac));
}

int VaultDatabaseV1::InitializeDatabase() {
    FileHeader header{};
    header.magic = kDbHeaderMagic;
    header.version = kDbVersion;

    // Generate random salt
    if (GenerateRandom(header.salt, sizeof(header.salt)) != 0) {
        return -1;
    }

    // Derive key check value
    DeriveKeyCheck(header.salt, master_key_, header.key_check);

    // Compute CRC32 for header (excluding the crc32 field itself)
    header.header_crc32 = ComputeCRC32(reinterpret_cast<const uint8_t*>(&header),
                                       offsetof(FileHeader, header_crc32));

    // Write header to file
    LfsFileGuard file(fs_);
    int err = file.open(path_.c_str(), LFS_O_WRONLY | LFS_O_CREAT | LFS_O_EXCL);
    if (err < 0) {
        return err;
    }

    lfs_ssize_t written = lfs_file_write(fs_, &file.f, &header, sizeof(header));
    if (written != sizeof(header)) {
        return LFS_ERR_IO;
    }

    err = lfs_file_sync(fs_, &file.f);
    if (err < 0) {
        return err;
    }

    next_seq_ = 1;
    return 0;
}

int VaultDatabaseV1::LoadDatabase() {
    LfsFileGuard file(fs_);
    int err = file.open(path_.c_str(), LFS_O_RDONLY);
    if (err < 0) {
        return err;
    }

    // Read file header
    FileHeader header{};
    lfs_ssize_t read_bytes = lfs_file_read(fs_, &file.f, &header, sizeof(header));
    if (read_bytes != sizeof(header)) {
        return LFS_ERR_IO;
    }

    // Verify magic and version
    if (header.magic != kDbHeaderMagic) {
        return LFS_ERR_CORRUPT;
    }

    if (header.version != kDbVersion) {
        return LFS_ERR_INVAL;
    }

    // Verify CRC32
    uint32_t computed_crc = ComputeCRC32(reinterpret_cast<const uint8_t*>(&header),
                                         offsetof(FileHeader, header_crc32));
    if (computed_crc != header.header_crc32) {
        return LFS_ERR_CORRUPT;
    }

    // Verify key
    {
        uint8_t expected_key_check[16];
        WipeGuard expected_key_guard{expected_key_check};
        DeriveKeyCheck(header.salt, master_key_, expected_key_check);
        if (memcmp(expected_key_check, header.key_check, 16) != 0) {
            return LFS_ERR_INVAL; // Wrong key
        }
    }
    // Scan through all records to find the highest sequence number and build index
    entity_index_.clear();
    uint32_t max_seq = 0;
    for (;;) {
        RecordHeader rec_header{};
        lfs_ssize_t bytes = lfs_file_read(fs_, &file.f, &rec_header, sizeof(rec_header));
        if (bytes == 0) {
            break; // End of file
        }
        if (bytes != sizeof(rec_header) || rec_header.magic != kRecordHeaderMagic) {
            return LFS_ERR_CORRUPT;
        }

        // Track maximum sequence number
        if (rec_header.seq > max_seq) {
            max_seq = rec_header.seq;
        }

        // Read name
        std::string name(rec_header.name_size, '\0');
        if (rec_header.name_size > 0) {
            bytes = lfs_file_read(fs_, &file.f, name.data(), rec_header.name_size);
            if (bytes != static_cast<lfs_ssize_t>(rec_header.name_size)) {
                return LFS_ERR_IO;
            }
        }

        // Read encrypted data
        std::vector<uint8_t> encrypted_data(rec_header.data_size);
        if (rec_header.data_size > 0) {
            bytes = lfs_file_read(fs_, &file.f, encrypted_data.data(), rec_header.data_size);
            if (bytes != static_cast<lfs_ssize_t>(rec_header.data_size)) {
                return LFS_ERR_IO;
            }
        }

        // Read trailer
        RecordTrailer trailer{};
        bytes = lfs_file_read(fs_, &file.f, &trailer, sizeof(trailer));
        if (bytes != sizeof(trailer) || trailer.trailer_magic != kTrailerMagic) {
            return LFS_ERR_CORRUPT;
        }

        // Decrypt data
        std::vector<uint8_t> decrypted_data(rec_header.data_size);
        if (rec_header.data_size > 0) {
            err = DecryptData(rec_header, encrypted_data.data(), rec_header.data_size,
                            trailer.auth_tag, decrypted_data.data());
            if (err < 0) {
                return LFS_ERR_CORRUPT; // Decryption failed
            }
        }

        // Update index based on record type
        auto type = static_cast<DBRecordType>(rec_header.type);
        if (type == DBRecordType::New) {
            // Add or update entity in index
            entity_index_[rec_header.entity_id] = EntityState{
                std::move(name),
                std::move(decrypted_data)
            };
        } else if (type == DBRecordType::Remove) {
            // Remove entity from index
            entity_index_.erase(rec_header.entity_id);
        }
    }

    next_seq_ = max_seq + 1;
    return 0;
}

int VaultDatabaseV1::Unlock(const uint8_t master_key[32]) {
    if (unlocked_) {
        return 0; // Already unlocked
    }

    // Copy master key
    memcpy(master_key_, master_key, 32);

    // Check if database file exists
    lfs_info info{};
    int err = lfs_stat(fs_, path_.c_str(), &info);

    if (err == LFS_ERR_NOENT) {
        // Database doesn't exist, create it
        err = InitializeDatabase();
        if (err < 0) {
            goto clean;
        }
    }
    else if (err == 0) {
        // Database exists, load it
        err = LoadDatabase();
        if (err < 0) {
            goto clean;
        }
    }
    else {
        // Error accessing file
        goto clean;
    }

    unlocked_ = true;
    return 0;
clean:
    mbedtls_platform_zeroize(master_key_, sizeof(master_key_));
    return err;
}

void VaultDatabaseV1::Lock() {
    if (unlocked_) {
        mbedtls_platform_zeroize(master_key_, sizeof(master_key_));
        entity_index_.clear(); // Clear the index
        unlocked_ = false;
    }
}

int VaultDatabaseV1::EncryptData(const RecordHeader& header, const uint8_t* plaintext,
                                 size_t plaintext_len, uint8_t* ciphertext, uint8_t* tag) {
    // Import master key for AES-GCM
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_ENCRYPT);
    psa_set_key_algorithm(&attributes, PSA_ALG_GCM);
    psa_set_key_type(&attributes, PSA_KEY_TYPE_AES);

    psa_key_id_t key_id;
    psa_status_t status = psa_import_key(&attributes, master_key_, 32, &key_id);
    if (status != PSA_SUCCESS) {
        return -1;
    }

    // Prepare AAD (first 16 bytes of header: magic, entity_id, seq, type)
    const auto* aad = reinterpret_cast<const uint8_t*>(&header);
    size_t aad_len = 16; // magic(4) + entity_id(4) + seq(4) + type(4)

    // Perform AEAD encryption
    size_t output_len;
    status = psa_aead_encrypt(key_id, PSA_ALG_GCM,
                              header.iv, sizeof(header.iv),
                              aad, aad_len,
                              plaintext, plaintext_len,
                              ciphertext, plaintext_len + 16, &output_len);

    psa_destroy_key(key_id);

    if (status != PSA_SUCCESS) {
        return -1;
    }

    // Extract tag from the end of output
    memcpy(tag, ciphertext + plaintext_len, 16);

    return 0;
}

int VaultDatabaseV1::DecryptData(const RecordHeader& header, const uint8_t* ciphertext,
                                  size_t ciphertext_len, const uint8_t* tag, uint8_t* plaintext) {
    // Import master key for AES-GCM
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_DECRYPT);
    psa_set_key_algorithm(&attributes, PSA_ALG_GCM);
    psa_set_key_type(&attributes, PSA_KEY_TYPE_AES);

    psa_key_id_t key_id;
    psa_status_t status = psa_import_key(&attributes, master_key_, 32, &key_id);
    if (status != PSA_SUCCESS) {
        return -1;
    }

    // Prepare AAD (first 16 bytes of header: magic, entity_id, seq, type)
    const auto* aad = reinterpret_cast<const uint8_t*>(&header);
    size_t aad_len = 16; // magic(4) + entity_id(4) + seq(4) + type(4)

    // Combine ciphertext and tag for PSA API
    std::vector<uint8_t> ciphertext_with_tag(ciphertext_len + 16);
    if (ciphertext_len > 0) {
        memcpy(ciphertext_with_tag.data(), ciphertext, ciphertext_len);
    }
    memcpy(ciphertext_with_tag.data() + ciphertext_len, tag, 16);

    // Perform AEAD decryption
    size_t output_len;
    status = psa_aead_decrypt(key_id, PSA_ALG_GCM,
                             header.iv, sizeof(header.iv),
                             aad, aad_len,
                             ciphertext_with_tag.data(), ciphertext_with_tag.size(),
                             plaintext, ciphertext_len, &output_len);

    psa_destroy_key(key_id);

    if (status != PSA_SUCCESS) {
        return -1;
    }

    return 0;
}

int VaultDatabaseV1::WriteRecord(DBRecordType type, uint32_t entity_id, uint32_t seq,
                                 std::string_view name, const uint8_t* data, size_t data_len) {
    if (!unlocked_) {
        return LFS_ERR_INVAL;
    }

    // Prepare record header
    RecordHeader header{};
    header.magic = kRecordHeaderMagic;
    header.entity_id = entity_id;
    header.seq = seq;
    header.type = static_cast<uint32_t>(type);
    header.name_size = name.size();
    header.data_size = data_len;

    // Generate IV: entity_id (4) | seq (4) | random (4)
    memcpy(header.iv, &entity_id, 4);
    memcpy(header.iv + 4, &seq, 4);
    if (GenerateRandom(header.iv + 8, 4) != 0) {
        return -1;
    }

    // Allocate buffers for encrypted data and tag
    std::vector<uint8_t> ciphertext(data_len);
    RecordTrailer trailer{};
    trailer.trailer_magic = kTrailerMagic;

    // Encrypt data
    if (data_len > 0) {
        if (EncryptData(header, data, data_len, ciphertext.data(), trailer.auth_tag) != 0) {
            return -1;
        }
    }
    else {
        // For empty data, we still need to generate a tag
        uint8_t dummy_output[16];
        if (EncryptData(header, nullptr, 0, dummy_output, trailer.auth_tag) != 0) {
            return -1;
        }
    }

    // Open file for appending
    LfsFileGuard file(fs_);
    int err = file.open(path_.c_str(), LFS_O_WRONLY | LFS_O_APPEND);
    if (err < 0) {
        return err;
    }

    // Write record header
    lfs_ssize_t written = lfs_file_write(fs_, &file.f, &header, sizeof(header));
    if (written != sizeof(header)) {
        return LFS_ERR_IO;
    }

    // Write name (plaintext)
    if (header.name_size > 0) {
        written = lfs_file_write(fs_, &file.f, name.data(), header.name_size);
        if (written != static_cast<lfs_ssize_t>(header.name_size)) {
            return LFS_ERR_IO;
        }
    }

    // Write encrypted data
    if (data_len > 0) {
        written = lfs_file_write(fs_, &file.f, ciphertext.data(), data_len);
        if (written != static_cast<lfs_ssize_t>(data_len)) {
            return LFS_ERR_IO;
        }
    }

    // Write trailer
    written = lfs_file_write(fs_, &file.f, &trailer, sizeof(trailer));
    if (written != sizeof(trailer)) {
        return LFS_ERR_IO;
    }

    // Sync to ensure data is persisted
    err = lfs_file_sync(fs_, &file.f);
    if (err < 0) {
        return err;
    }

    return 0;
}

uint32_t VaultDatabaseV1::Append(std::string_view name, const uint8_t* data, size_t len) {
    if (!unlocked_) {
        return 0; // Failure
    }

    uint32_t entity_id = next_seq_;
    uint32_t seq = next_seq_;
    next_seq_++;

    int err = WriteRecord(DBRecordType::New, entity_id, seq, name, data, len);
    if (err < 0) {
        next_seq_--; // Rollback sequence number on failure
        return 0;
    }

    // Update index
    entity_index_[entity_id] = EntityState{
        std::string(name),
        std::vector<uint8_t>(data, data + len)
    };

    return entity_id;
}

int VaultDatabaseV1::Remove(uint32_t entity_id) {
    if (!unlocked_) {
        return LFS_ERR_INVAL;
    }

    uint32_t seq = next_seq_;
    next_seq_++;

    // Write a Remove record with empty name and data
    int err = WriteRecord(DBRecordType::Remove, entity_id, seq, "", nullptr, 0);
    if (err < 0) {
        next_seq_--; // Rollback on failure
        return err;
    }

    // Remove from index
    entity_index_.erase(entity_id);

    return 0;
}

int VaultDatabaseV1::Modify(uint32_t entity_id, const uint8_t* data, size_t len) {
    if (!unlocked_) {
        return LFS_ERR_INVAL;
    }

    // Get current entity state from index
    auto it = entity_index_.find(entity_id);
    if (it == entity_index_.end()) {
        return LFS_ERR_NOENT; // Entity not found
    }

    // Keep the current name, update data only
    const std::string& current_name = it->second.name;

    // Remove old entry
    int err = Remove(entity_id);
    if (err < 0) {
        return err;
    }

    // Append new entry with same entity_id, current name, but new data
    uint32_t seq = next_seq_;
    next_seq_++;

    err = WriteRecord(DBRecordType::New, entity_id, seq, current_name, data, len);
    if (err < 0) {
        next_seq_--; // Rollback on failure
        return err;
    }

    // Update index
    entity_index_[entity_id] = EntityState{
        current_name,
        std::vector<uint8_t>(data, data + len)
    };

    return 0;
}

int VaultDatabaseV1::Modify(uint32_t entity_id, std::string_view name) {
    if (!unlocked_) {
        return LFS_ERR_INVAL;
    }

    // Get current entity state from index
    auto it = entity_index_.find(entity_id);
    if (it == entity_index_.end()) {
        return LFS_ERR_NOENT; // Entity not found
    }

    // Keep the current data, update name only
    const std::vector<uint8_t>& current_data = it->second.data;

    // Remove old entry
    int err = Remove(entity_id);
    if (err < 0) {
        return err;
    }

    // Append new entry with same entity_id, new name, but current data
    uint32_t seq = next_seq_;
    next_seq_++;

    err = WriteRecord(DBRecordType::New, entity_id, seq, name,
                     current_data.data(), current_data.size());
    if (err < 0) {
        next_seq_--; // Rollback on failure
        return err;
    }

    // Update index
    entity_index_[entity_id] = EntityState{
        std::string(name),
        current_data
    };

    return 0;
}

int VaultDatabaseV1::Modify(uint32_t entity_id, std::string_view name, const uint8_t* data, size_t len) {
    if (!unlocked_) {
        return LFS_ERR_INVAL;
    }

    // Check if entity exists
    auto it = entity_index_.find(entity_id);
    if (it == entity_index_.end()) {
        return LFS_ERR_NOENT; // Entity not found
    }

    // Remove old entry
    int err = Remove(entity_id);
    if (err < 0) {
        return err;
    }

    // Append new entry with same entity_id but new name and data
    uint32_t seq = next_seq_;
    next_seq_++;

    err = WriteRecord(DBRecordType::New, entity_id, seq, name, data, len);
    if (err < 0) {
        next_seq_--; // Rollback on failure
        return err;
    }

    // Update index
    entity_index_[entity_id] = EntityState{
        std::string(name),
        std::vector<uint8_t>(data, data + len)
    };

    return 0;
}

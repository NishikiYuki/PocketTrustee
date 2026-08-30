#include "app_vault.hpp"
#include "app_ptt_utility.hpp"
#include <cstring>
#include <mbedtls/platform_util.h>
#include <psa/crypto.h>
#include "common_crc.hpp"

using namespace PocketTrustee;

VaultDatabaseV1::~VaultDatabaseV1() { mbedtls_platform_zeroize(encrypt_key_, sizeof(encrypt_key_)); }

uint32_t VaultDatabaseV1::ComputeCRC32(const uint8_t* data, size_t len) {
    Crc32Mpeg2 crc;
    return crc.Continue(data, len);
}

int VaultDatabaseV1::GenerateRandom(uint8_t* output, size_t len) {
    psa_status_t status = psa_generate_random(output, len);
    return (status == PSA_SUCCESS) ? 0 : -1;
}

VaultDatabaseV1::Error VaultDatabaseV1::LfsErrorToError(int lfs_err) {
    switch (lfs_err) {
    case LFS_ERR_OK:
        return Error::Ok;
    case LFS_ERR_CORRUPT:
        return Error::Corrupt;
    case LFS_ERR_NOENT:
        return Error::NoEntity;
    case LFS_ERR_EXIST:
        return Error::Exists;
    case LFS_ERR_INVAL:
        return Error::InvalidParameter;
    default:
        return Error::IoError;
    }
}

VaultDatabaseV1::Error VaultDatabaseV1::DeriveKeyHMAC(const std::span<const uint8_t>& key,
                                                      const std::span<const uint8_t>& salt,
                                                      const std::span<uint8_t>& mac) {
    // Use HMAC-SHA256 to derive key
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_MESSAGE);
    psa_set_key_algorithm(&attributes, PSA_ALG_HMAC(PSA_ALG_SHA_256));
    psa_set_key_type(&attributes, PSA_KEY_TYPE_HMAC);

    psa_key_id_t key_id;
    psa_status_t status = psa_import_key(&attributes, key.data(), key.size(), &key_id);
    if (status != PSA_SUCCESS) {
        mbedtls_platform_zeroize(mac.data(), mac.size());
        return Error::InvalidParameter;
    }

    uint8_t mac_buffer[32];
    size_t mac_len;
    status = psa_mac_compute(key_id, PSA_ALG_HMAC(PSA_ALG_SHA_256),
                             salt.data(), salt.size(),
                             mac_buffer, sizeof(mac_buffer), &mac_len);

    if (status != PSA_SUCCESS) {
        mbedtls_platform_zeroize(mac.data(), mac.size());
        psa_destroy_key(key_id);
        return Error::CryptoError;
    }

    memcpy(mac.data(), mac_buffer, std::min(mac_len, mac.size()));

    psa_destroy_key(key_id);
    return Error::Ok;
}

VaultDatabaseV1::Error VaultDatabaseV1::InitializeDatabase(const std::span<const uint8_t>& master_key) {
    FileHeader header{};
    header.magic = kDbHeaderMagic;
    header.version = kDbVersion;

    // Generate random salt
    if (GenerateRandom(header.salt, sizeof(header.salt)) != 0) {
        return Error::CryptoError;
    }

    // Derive key check value
    constexpr char check_info[] = "check";
    uint8_t salt_check[sizeof(header.salt) + sizeof(check_info)];
    memcpy(salt_check, header.salt, sizeof(header.salt));
    memcpy(salt_check + sizeof(header.salt), check_info, sizeof(check_info));
    auto perr = DeriveKeyHMAC(master_key, salt_check, header.key_check);
    if (perr != Error::Ok) {
        return perr;
    }

    // Compute CRC32 for header (excluding the crc32 field itself)
    header.header_crc32 = ComputeCRC32(reinterpret_cast<const uint8_t*>(&header),
                                       offsetof(FileHeader, header_crc32));

    // Write header to file
    int err;
    {
        LfsFileGuard file(fs_);
        err = file.open(path_.c_str(), LFS_O_WRONLY | LFS_O_CREAT | LFS_O_EXCL);
        if (err >= 0) {
            lfs_ssize_t written = lfs_file_write(fs_, &file.f, &header, sizeof(header));
            if (written != sizeof(header)) {
                err = LFS_ERR_IO;
            }
            else {
                err = lfs_file_sync(fs_, &file.f);
            }
        }
    }
    if (err < 0) {
        // Remove partially created database file
        lfs_remove(fs_, path_.c_str());
        return LfsErrorToError(err);
    }

    // Derive encrypt key
    constexpr char encrypted_info[] = "encrypt";
    uint8_t salt_encrypt[sizeof(header.salt) + sizeof(encrypted_info)];
    memcpy(salt_encrypt, header.salt, sizeof(header.salt));
    memcpy(salt_encrypt + sizeof(header.salt), encrypted_info, sizeof(encrypted_info));
    perr = DeriveKeyHMAC(master_key, salt_encrypt, encrypt_key_);
    if (perr != Error::Ok) {
        lfs_remove(fs_, path_.c_str());
        return perr;
    }

    next_seq_ = 1;
    return Error::Ok;
}

VaultDatabaseV1::Error VaultDatabaseV1::LoadDatabase(const std::span<const uint8_t>& master_key) {
    LfsFileGuard file(fs_);
    int err = file.open(path_.c_str(), LFS_O_RDONLY);
    if (err < 0) {
        return LfsErrorToError(err);
    }

    // Read file header
    FileHeader header{};
    lfs_ssize_t read_bytes = lfs_file_read(fs_, &file.f, &header, sizeof(header));
    if (read_bytes != sizeof(header)) {
        return Error::IoError;
    }

    // Verify magic and version
    if (header.magic != kDbHeaderMagic) {
        return Error::Corrupt;
    }

    if (header.version != kDbVersion) {
        return Error::Corrupt;
    }

    // Verify CRC32
    uint32_t computed_crc = ComputeCRC32(reinterpret_cast<const uint8_t*>(&header),
                                         offsetof(FileHeader, header_crc32));
    if (computed_crc != header.header_crc32) {
        return Error::Corrupt;
    }

    // Verify key
    uint8_t expected_key_check[16];
    constexpr char check_info[] = "check";
    uint8_t salt_check[sizeof(header.salt) + sizeof(check_info)];
    memcpy(salt_check, header.salt, sizeof(header.salt));
    memcpy(salt_check + sizeof(header.salt), check_info, sizeof(check_info));
    auto perr = DeriveKeyHMAC(master_key, salt_check, expected_key_check);
    if (perr != Error::Ok) {
        return perr;
    }
    if (memcmp(expected_key_check, header.key_check, sizeof(expected_key_check)) != 0) {
        return Error::WrongKey; // Wrong key
    }

    // Scan through all records to find the highest sequence number and build index
    entity_index_.clear();
    uint32_t max_seq = 0;

    for (;;) {
        // Save current position (start of record)
        uint32_t record_offset = lfs_file_tell(fs_, &file.f);

        RecordHeader rec_header{};
        lfs_ssize_t bytes = lfs_file_read(fs_, &file.f, &rec_header, sizeof(rec_header));
        if (bytes == 0) {
            break; // End of file
        }
        if (bytes != sizeof(rec_header) || rec_header.magic != kRecordHeaderMagic) {
            return Error::Corrupt;
        }

        // Skip to tail
        RecordTrailer tailer{};
        auto skip_size = static_cast<lfs_soff_t>(rec_header.name_size + rec_header.data_size);
        err = lfs_file_seek(fs_, &file.f, skip_size, LFS_SEEK_CUR);
        if (err < 0) {
            return LfsErrorToError(err);
        }
        bytes = lfs_file_read(fs_, &file.f, &tailer, sizeof(tailer));
        if (bytes != sizeof(tailer) || tailer.trailer_magic != kTrailerMagic) {
            return Error::Corrupt;
        }

        // Track maximum sequence number
        if (rec_header.seq > max_seq) {
            max_seq = rec_header.seq;
        }

        // Update index based on record type
        auto type = static_cast<DbRecordType>(rec_header.type);
        if (type == DbRecordType::New) {
            // Add or update entity in index with current record offset
            AddOrUpdateIndex(rec_header.entity_id, record_offset);
        }
        else if (type == DbRecordType::Remove) {
            // Remove entity from index
            RemoveFromIndex(rec_header.entity_id);
        }
    }

    // Derive encrypt key
    constexpr char encrypted_info[] = "encrypt";
    uint8_t salt_encrypt[sizeof(header.salt) + sizeof(encrypted_info)];
    memcpy(salt_encrypt, header.salt, sizeof(header.salt));
    memcpy(salt_encrypt + sizeof(header.salt), encrypted_info, sizeof(encrypted_info));
    perr = DeriveKeyHMAC(master_key, salt_encrypt, encrypt_key_);
    if (perr != Error::Ok) {
        return perr;
    }

    next_seq_ = max_seq + 1;
    return Error::Ok;
}

VaultDatabaseV1::Error VaultDatabaseV1::Unlock(const std::span<const uint8_t>& master_key) {
    if (unlocked_) {
        return Error::Ok; // Already unlocked
    }

    // Check if database file exists
    lfs_info info{};
    int err = lfs_stat(fs_, path_.c_str(), &info);

    if (err == LFS_ERR_NOENT) {
        // Database doesn't exist, create it
        auto perr = InitializeDatabase(master_key);
        if (perr != Error::Ok) {
            return perr;
        }
    }
    else if (err == 0) {
        // Database exists, load it
        auto perr = LoadDatabase(master_key);
        if (perr != Error::Ok) {
            return perr;
        }
    }
    else {
        // Error accessing file
        return Error::IoError;
    }

    unlocked_ = true;
    return Error::Ok;
}

void VaultDatabaseV1::Lock() {
    if (unlocked_) {
        mbedtls_platform_zeroize(encrypt_key_, sizeof(encrypt_key_));
        entity_index_.clear(); // Clear the index
        unlocked_ = false;
    }
}

VaultDatabaseV1::Error VaultDatabaseV1::EncryptData(const RecordHeader& header,
                                                    const std::span<const uint8_t>& plaintext,
                                                    const std::span<uint8_t>& ciphertext) {
    if (ciphertext.size() < plaintext.size() + 16) {
        return Error::InvalidParameter;
    }

    // Import master key for AES-GCM
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_ENCRYPT);
    psa_set_key_algorithm(&attributes, PSA_ALG_GCM);
    psa_set_key_type(&attributes, PSA_KEY_TYPE_AES);

    psa_key_id_t key_id;
    psa_status_t status = psa_import_key(&attributes, encrypt_key_, sizeof(encrypt_key_), &key_id);
    if (status != PSA_SUCCESS) {
        return Error::CryptoError;
    }

    // Prepare AAD
    const auto* aad = reinterpret_cast<const uint8_t*>(&header);
    constexpr size_t aad_len = offsetof(RecordHeader, iv);

    // Perform AEAD encryption
    size_t output_len;
    status = psa_aead_encrypt(key_id, PSA_ALG_GCM,
                              header.iv, sizeof(header.iv),
                              aad, aad_len,
                              plaintext.data(), plaintext.size(),
                              ciphertext.data(), ciphertext.size(), &output_len);

    psa_destroy_key(key_id);

    if (status != PSA_SUCCESS) {
        return Error::CryptoError;
    }

    return Error::Ok;
}

VaultDatabaseV1::Error VaultDatabaseV1::DecryptData(const RecordHeader& header,
                                                    const std::span<const uint8_t>& ciphertext,
                                                    const std::span<uint8_t>& plaintext) {
    if (ciphertext.size() > plaintext.size() + 16) {
        return Error::InvalidParameter;
    }

    // Import master key for AES-GCM
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_DECRYPT);
    psa_set_key_algorithm(&attributes, PSA_ALG_GCM);
    psa_set_key_type(&attributes, PSA_KEY_TYPE_AES);

    psa_key_id_t key_id;
    psa_status_t status = psa_import_key(&attributes, encrypt_key_, sizeof(encrypt_key_), &key_id);
    if (status != PSA_SUCCESS) {
        return Error::CryptoError;
    }

    // Prepare AAD
    const auto* aad = reinterpret_cast<const uint8_t*>(&header);
    constexpr size_t aad_len = offsetof(RecordHeader, iv);

    // Perform AEAD decryption
    size_t output_len;
    status = psa_aead_decrypt(key_id, PSA_ALG_GCM,
                              header.iv, sizeof(header.iv),
                              aad, aad_len,
                              ciphertext.data(), ciphertext.size(),
                              plaintext.data(), plaintext.size(), &output_len);

    psa_destroy_key(key_id);

    if (status != PSA_SUCCESS) {
        return Error::CryptoError;
    }

    return Error::Ok;
}

int VaultDatabaseV1::FindEntityInIndex(uint32_t entity_id) const {
    // Linear search through the index
    for (size_t i = 0; i < entity_index_.size(); ++i) {
        if (entity_index_[i].entity_id == entity_id) {
            return static_cast<int>(i);
        }
    }
    return -1; // Not found
}

void VaultDatabaseV1::AddOrUpdateIndex(uint32_t entity_id, uint32_t offset) {
    int idx = FindEntityInIndex(entity_id);
    if (idx >= 0) {
        // Update existing entry
        entity_index_[idx].offset = offset;
    }
    else {
        // Add new entry
        entity_index_.push_back(IdxSection{entity_id, offset});
    }
}

void VaultDatabaseV1::RemoveFromIndex(uint32_t entity_id) {
    int idx = FindEntityInIndex(entity_id);
    if (idx >= 0) {
        // Remove by swapping with last element and popping
        if (idx < static_cast<int>(entity_index_.size()) - 1) {
            entity_index_[idx] = entity_index_.back();
        }
        entity_index_.pop_back();
    }
}

VaultDatabaseV1::Error VaultDatabaseV1::WriteRecord(DbRecordType type, uint32_t entity_id, std::string_view name,
                                                    const std::span<const uint8_t>& data) {
    if (!unlocked_) {
        return Error::NotUnlocked;
    }

    const uint32_t seq = next_seq_++;

    // Prepare record header
    RecordHeader header{};
    header.magic = kRecordHeaderMagic;
    header.entity_id = entity_id;
    header.seq = seq;
    header.type = static_cast<uint16_t>(type);
    header.name_size = name.size();
    header.data_size = data.size();

    RecordTrailer trailer{};
    trailer.trailer_magic = kTrailerMagic;

    std::vector<uint8_t> data_filed;

    // Remove type carry no data, skip encrypt
    if (type != DbRecordType::Remove) {
        // Generate IV: entity_id (4) | seq (4) | random (4)
        memcpy(header.iv, &entity_id, sizeof(entity_id));
        memcpy(header.iv + sizeof(entity_id), &seq, sizeof(seq));
        if (GenerateRandom(header.iv + sizeof(entity_id) + sizeof(seq), 4) != 0) {
            return Error::CryptoError;
        }

        // Allocate buffers for encrypted data and tag
        std::vector<uint8_t> ciphertext(data.size() + 16);
        // Encrypt data
        auto err = EncryptData(header, data, ciphertext);
        if (err != Error::Ok) {
            return err;
        }
        data_filed = std::move(ciphertext);
    }

    // Open file for appending
    LfsFileGuard file(fs_);
    int err = file.open(path_.c_str(), LFS_O_WRONLY | LFS_O_APPEND);
    if (err < 0) {
        return LfsErrorToError(err);
    }

    // Write record header
    lfs_ssize_t written = lfs_file_write(fs_, &file.f, &header, sizeof(header));
    if (written != sizeof(header)) {
        return Error::IoError;
    }

    // Write name (plaintext)
    if (header.name_size > 0) {
        written = lfs_file_write(fs_, &file.f, name.data(), header.name_size);
        if (written != static_cast<lfs_ssize_t>(header.name_size)) {
            return Error::IoError;
        }
    }

    // Write data
    if (!data_filed.empty()) {
        written = lfs_file_write(fs_, &file.f, data_filed.data(), data_filed.size());
        if (written != static_cast<lfs_ssize_t>(data_filed.size())) {
            return Error::IoError;
        }
        // data filed include auth tag, write trailer magic only
        lfs_file_write(fs_, &file.f, &trailer.trailer_magic, sizeof(trailer.trailer_magic));
    }
    else {
        // Write trailer with empty tag
        lfs_file_write(fs_, &file.f, &trailer, sizeof(trailer));
    }

    // Sync to ensure data is persisted
    err = lfs_file_sync(fs_, &file.f);
    if (err < 0) {
        return Error::IoError;
    }

    return Error::Ok;
}

VaultDatabaseV1::Error VaultDatabaseV1::Append(std::string_view name, const std::span<const uint8_t>& data,
                                               uint32_t& entity_id) {
    if (!unlocked_) {
        return Error::NotUnlocked;
    }
    uint32_t offset = 0;
    entity_id = next_seq_;
    {
        // Get current file size (offset where new record will be written)
        LfsFileGuard file(fs_);
        int err = file.open(path_.c_str(), LFS_O_RDONLY);
        if (err < 0) {
            return LfsErrorToError(err);
        }
        lfs_soff_t file_size = lfs_file_size(fs_, &file.f);
        if (file_size < 0) {
            return Error::IoError;
        }
        offset = static_cast<uint32_t>(file_size);
    }
    // Write the record
    auto err = WriteRecord(DbRecordType::New, entity_id, name, data);
    if (err != Error::Ok) {
        return err;
    }
    // Update index with the new record offset
    AddOrUpdateIndex(entity_id, offset);
    return Error::Ok;
}

VaultDatabaseV1::Error VaultDatabaseV1::Remove(uint32_t entity_id) {
    if (!unlocked_) {
        return Error::NotUnlocked;
    }
    // Find entity in index
    int idx = FindEntityInIndex(entity_id);
    if (idx < 0) {
        return Error::NoEntity;
    }
    // Write a Remove record with empty name and data
    auto err = WriteRecord(DbRecordType::Remove, entity_id, "", {});
    if (err != Error::Ok) {
        return err;
    }
    // Remove from index
    RemoveFromIndex(entity_id);
    return Error::Ok;
}

VaultDatabaseV1::Error VaultDatabaseV1::Modify(uint32_t entity_id, std::string_view name,
                                               const std::span<const uint8_t>& data) {
    if (!unlocked_) {
        return Error::NotUnlocked;
    }
    // Find entity in index
    int idx = FindEntityInIndex(entity_id);
    if (idx < 0) {
        return Error::NoEntity;
    }
    uint32_t offset = 0;
    {
        // Get current file size (offset where new record will be written)
        LfsFileGuard file(fs_);
        int err = file.open(path_.c_str(), LFS_O_RDONLY);
        if (err < 0) {
            return LfsErrorToError(err);
        }
        lfs_soff_t file_size = lfs_file_size(fs_, &file.f);
        if (file_size < 0) {
            return Error::IoError;
        }
        offset = static_cast<uint32_t>(file_size);
    }
    // Write the record
    auto err = WriteRecord(DbRecordType::New, entity_id, name, data);
    if (err != Error::Ok) {
        return err;
    }
    // Update index with the new record offset
    AddOrUpdateIndex(entity_id, offset);
    return Error::Ok;
}

VaultDatabaseV1::Error VaultDatabaseV1::ReadData(uint32_t entity_id, const std::span<uint8_t>& data,
                                                 size_t& actual_size) {
    if (!unlocked_) {
        return Error::NotUnlocked;
    }
    // Find entity in index
    int idx = FindEntityInIndex(entity_id);
    if (idx < 0) {
        return Error::NoEntity;
    }
    uint32_t offset = entity_index_.at(idx).offset;
    // Read record header
    LfsFileGuard file(fs_);
    int err = file.open(path_.c_str(), LFS_O_RDONLY);
    if (err < 0) {
        return LfsErrorToError(err);
    }
    err = lfs_file_seek(fs_, &file.f, static_cast<long>(offset), LFS_SEEK_SET);
    if (err < 0) {
        return LfsErrorToError(err);
    }
    RecordHeader rec_header{};
    lfs_ssize_t bytes = lfs_file_read(fs_, &file.f, &rec_header, sizeof(rec_header));
    if (bytes != sizeof(rec_header) || rec_header.magic != kRecordHeaderMagic || rec_header.entity_id != entity_id) {
        return Error::Corrupt;
    }
    actual_size = rec_header.data_size;
    err = lfs_file_seek(fs_, &file.f, rec_header.name_size, LFS_SEEK_CUR);
    if (err < 0) {
        return LfsErrorToError(err);
    }
    // Read data
    if (!data.empty()) {
        std::vector<uint8_t> ciphertext(static_cast<size_t>(rec_header.data_size) + 16);
        bytes = lfs_file_read(fs_, &file.f, ciphertext.data(), ciphertext.size());
        if (bytes != static_cast<lfs_ssize_t>(ciphertext.size())) {
            return (bytes < 0) ? LfsErrorToError(bytes) : Error::IoError;
        }
        if (auto perr = DecryptData(rec_header, ciphertext, data); perr != Error::Ok) {
            return perr;
        }
    }
    return Error::Ok;
}

VaultDatabaseV1::Error VaultDatabaseV1::ReadName(uint32_t entity_id, std::string& name) {
    // Find entity in index
    int idx = FindEntityInIndex(entity_id);
    if (idx < 0) {
        return Error::NoEntity;
    }
    uint32_t offset = entity_index_.at(idx).offset;
    // Read record header
    LfsFileGuard file(fs_);
    int err = file.open(path_.c_str(), LFS_O_RDONLY);
    if (err < 0) {
        return LfsErrorToError(err);
    }
    err = lfs_file_seek(fs_, &file.f, static_cast<long>(offset), LFS_SEEK_SET);
    if (err < 0) {
        return LfsErrorToError(err);
    }
    RecordHeader rec_header{};
    lfs_ssize_t bytes = lfs_file_read(fs_, &file.f, &rec_header, sizeof(rec_header));
    if (bytes != sizeof(rec_header) || rec_header.magic != kRecordHeaderMagic || rec_header.entity_id != entity_id) {
        return Error::Corrupt;
    }
    // Read name
    name.resize(rec_header.name_size);
    if (rec_header.name_size > 0) {
        bytes = lfs_file_read(fs_, &file.f, name.data(), rec_header.name_size);
        if (bytes != static_cast<lfs_ssize_t>(name.size())) {
            return (bytes < 0) ? LfsErrorToError(bytes) : Error::IoError;
        }
    }
    return Error::Ok;
}

VaultDatabaseV1::Error VaultDatabaseV1::BuildIndex() {
    LfsFileGuard file(fs_);
    int err = file.open(path_.c_str(), LFS_O_RDONLY);
    if (err < 0) {
        return LfsErrorToError(err);
    }

    // Read file header
    FileHeader header{};
    lfs_ssize_t read_bytes = lfs_file_read(fs_, &file.f, &header, sizeof(header));
    if (read_bytes != sizeof(header)) {
        return Error::IoError;
    }

    // Verify magic and version
    if (header.magic != kDbHeaderMagic) {
        return Error::Corrupt;
    }

    if (header.version != kDbVersion) {
        return Error::Corrupt;
    }

    // Verify CRC32
    uint32_t computed_crc = ComputeCRC32(reinterpret_cast<const uint8_t*>(&header),
                                         offsetof(FileHeader, header_crc32));
    if (computed_crc != header.header_crc32) {
        return Error::Corrupt;
    }

    // Scan through all records to find the highest sequence number and build index
    entity_index_.clear();
    uint32_t max_seq = 0;

    for (;;) {
        // Save current position (start of record)
        uint32_t record_offset = lfs_file_tell(fs_, &file.f);

        RecordHeader rec_header{};
        lfs_ssize_t bytes = lfs_file_read(fs_, &file.f, &rec_header, sizeof(rec_header));
        if (bytes == 0) {
            break; // End of file
        }
        if (bytes != sizeof(rec_header) || rec_header.magic != kRecordHeaderMagic) {
            return Error::Corrupt;
        }

        // Skip to tail
        RecordTrailer tailer{};
        auto skip_size = static_cast<lfs_soff_t>(rec_header.name_size + rec_header.data_size);
        err = lfs_file_seek(fs_, &file.f, skip_size, LFS_SEEK_CUR);
        if (err < 0) {
            return LfsErrorToError(err);
        }
        bytes = lfs_file_read(fs_, &file.f, &tailer, sizeof(tailer));
        if (bytes != sizeof(tailer) || tailer.trailer_magic != kTrailerMagic) {
            return Error::Corrupt;
        }

        // Track maximum sequence number
        if (rec_header.seq > max_seq) {
            max_seq = rec_header.seq;
        }

        // Update index based on record type
        auto type = static_cast<DbRecordType>(rec_header.type);
        if (type == DbRecordType::New) {
            // Add or update entity in index with current record offset
            AddOrUpdateIndex(rec_header.entity_id, record_offset);
        }
        else if (type == DbRecordType::Remove) {
            // Remove entity from index
            RemoveFromIndex(rec_header.entity_id);
        }
    }

    next_seq_ = max_seq + 1;
    return Error::Ok;
}

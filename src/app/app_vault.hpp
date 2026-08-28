#ifndef POCKETTRUSTEE_APP_VAULT_HPP
#define POCKETTRUSTEE_APP_VAULT_HPP

#include <span>
#include <string>
#include <vector>
#include "app_ptt_utility.hpp"

namespace PocketTrustee {
    class VaultDatabaseV1 {
    public:
        VaultDatabaseV1(lfs_t* fs, std::string_view path) : fs_(fs), path_(path), unlocked_(false), next_seq_(1) {
            entity_index_.reserve(32);
        }

        ~VaultDatabaseV1();

        VaultDatabaseV1(const VaultDatabaseV1&) = delete;
        VaultDatabaseV1& operator=(const VaultDatabaseV1&) = delete;

        enum class Error : int {
            Ok = 0,
            InvalidParameter, // Invalid parameter or state
            IoError, // I/O error
            NoEntity, // Entity not found
            Corrupt, // Data corruption detected
            Exists, // Entity already exists
            NotUnlocked, // Database not unlocked
            CryptoError, // Cryptographic operation failed
            WrongKey, // Key verification failed
        };

        struct DataField {
            uint32_t offset;
            size_t size;
        };

        // Unlock database with master key
        Error Unlock(const std::span<const uint8_t>& master_key);

        // Lock database
        void Lock();

        // Append a new entry, returns entity_id through output parameter
        Error Append(std::string_view name, const std::span<const uint8_t>& data, uint32_t& entity_id);

        // Remove an entry by entity_id
        Error Remove(uint32_t entity_id);

        // Modify both entry name and data
        Error Modify(uint32_t entity_id, std::string_view name, const std::span<const uint8_t>& data);

        // Read entry name and data; name is fully read into the string
        Error Read(uint32_t entity_id, std::string& name, const std::span<uint8_t>& data);

    private:
        static constexpr uint32_t kDbHeaderMagic = 0x56444231;
        static constexpr uint32_t kRecordHeaderMagic = 0x52454344;
        static constexpr uint32_t kTrailerMagic = 0x5441494C;
        static constexpr uint32_t kDbVersion = 1;

        // Record type enumeration
        enum class DbRecordType : uint16_t { New, Remove };

        // Database file header
        struct __attribute__((packed)) FileHeader {
            uint32_t magic; // Magic number to identify file format
            uint32_t version; // Database version
            uint8_t salt[16]; // Key derivation salt
            uint8_t key_check[16]; // Key verification segment
            uint32_t header_crc32; // CRC32 of magic, version, salt, key_check
        };

        // Record header
        struct __attribute__((packed)) RecordHeader {
            uint32_t magic; // Record magic number
            uint32_t entity_id; // Unique entity ID
            uint32_t seq; // Global monotonically increasing sequence number
            uint16_t type; // DBRecordType
            uint16_t name_size; // Size of name field in bytes
            uint32_t data_size; // Size of encrypted payload in bytes
            uint8_t iv[12]; // 96-bit AES-GCM IV
        };

        // Record trailer
        struct __attribute__((packed)) RecordTrailer {
            uint8_t auth_tag[16]; // GCM authentication tag
            uint32_t trailer_magic; // Trailer magic for integrity
        };

        // Index section storing entity location in file
        struct IdxSection {
            uint32_t entity_id;
            uint32_t offset; // File offset to the record header
        };

        lfs_t* fs_;
        std::string path_;

        uint8_t encrypt_key_[32]{};
        bool unlocked_;
        uint32_t next_seq_; // Next sequence number to use

        // In-memory index of active entities (sorted by entity_id for binary search)
        std::vector<IdxSection> entity_index_;

        // Derive key using HMAC
        static Error DeriveKeyHMAC(const std::span<const uint8_t>& key, const std::span<const uint8_t>& salt,
                                   const std::span<uint8_t>& mac);
        // Utilities
        static uint32_t ComputeCRC32(const uint8_t* data, size_t len);
        static int GenerateRandom(uint8_t* output, size_t len);
        static Error LfsErrorToError(int lfs_err);

        // Initialize a new database file
        Error InitializeDatabase(const std::span<const uint8_t>& master_key);

        // Verify the database file and load metadata
        Error LoadDatabase(const std::span<const uint8_t>& master_key);

        // Write a record to the database
        Error WriteRecord(DbRecordType type, uint32_t entity_id, std::string_view name,
                          const std::span<const uint8_t>& data);

        // Encrypt data using AES-256-GCM
        // Ciphertext is followed by tag
        Error EncryptData(const RecordHeader& header, const std::span<const uint8_t>& plaintext,
                          const std::span<uint8_t>& ciphertext);

        // Decrypt data using AES-256-GCM
        // Tag must follow the ciphertext
        Error DecryptData(const RecordHeader& header, const std::span<const uint8_t>& ciphertext,
                          const std::span<uint8_t>& plaintext);


        // Index management helpers
        int FindEntityInIndex(uint32_t entity_id) const; // Returns index in vector, or -1 if not found
        void AddOrUpdateIndex(uint32_t entity_id, uint32_t offset);
        void RemoveFromIndex(uint32_t entity_id);
    };
} // namespace PocketTrustee

#endif // POCKETTRUSTEE_APP_VAULT_HPP

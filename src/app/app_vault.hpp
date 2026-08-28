#ifndef POCKETTRUSTEE_APP_VAULT_HPP
#define POCKETTRUSTEE_APP_VAULT_HPP

#include "app_ptt_utility.hpp"
#include <optional>
#include <vector>
#include <string>
#include <unordered_map>

namespace PocketTrustee {
    template <typename T> requires std::is_trivially_copyable_v<T>
    class WipeGuard {
    public:
        explicit WipeGuard(T& self) : self(&self) {
        }

        ~WipeGuard();
        T* self;
        WipeGuard(const WipeGuard&) = delete;
        WipeGuard& operator=(const WipeGuard&) = delete;
    };

    class VaultDatabaseV1 {
    public:
        VaultDatabaseV1(lfs_t* fs, lfs_config* disk, std::string_view path) :
            fs_(fs), disk_(disk), path_(path), unlocked_(false), next_seq_(1) {
        }

        ~VaultDatabaseV1();

        VaultDatabaseV1(const VaultDatabaseV1&) = delete;
        VaultDatabaseV1& operator=(const VaultDatabaseV1&) = delete;

        static constexpr uint32_t kDbHeaderMagic = 0x56444231;
        static constexpr uint32_t kRecordHeaderMagic = 0x52454344;
        static constexpr uint32_t kTrailerMagic = 0x5441494C;
        static constexpr uint32_t kDbVersion = 1;

        // Record type enumeration
        enum class DBRecordType : uint32_t {
            New,
            Remove
        };

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

        // Unlock database with master key
        int Unlock(const uint8_t master_key[32]);

        // Lock database
        void Lock();

        // Append a new entry, returns entity_id on success, 0 on failure
        uint32_t Append(std::string_view name, const uint8_t* data, size_t len);

        // Remove an entry by entity_id
        int Remove(uint32_t entity_id);

        // Modify entry data
        int Modify(uint32_t entity_id, const uint8_t* data, size_t len);

        // Modify entry name
        int Modify(uint32_t entity_id, std::string_view name);

        // Modify both entry name and data
        int Modify(uint32_t entity_id, std::string_view name, const uint8_t* data, size_t len);

    private:
        // Entity state in memory
        struct EntityState {
            std::string name;
            std::vector<uint8_t> data;
        };

        lfs_t* fs_;
        lfs_config* disk_;
        std::string path_;

        uint8_t master_key_[32]{};
        bool unlocked_;
        uint32_t next_seq_; // Next sequence number to use

        // In-memory index of active entities
        std::unordered_map<uint32_t, EntityState> entity_index_;

        // Initialize a new database file
        int InitializeDatabase();

        // Verify the database file and load metadata
        int LoadDatabase();

        // Write a record to the database
        int WriteRecord(DBRecordType type, uint32_t entity_id, uint32_t seq,
                        std::string_view name, const uint8_t* data, size_t data_len);

        // Compute CRC32
        static uint32_t ComputeCRC32(const uint8_t* data, size_t len);

        // Generate random bytes
        static int GenerateRandom(uint8_t* output, size_t len);

        // Encrypt data using AES-256-GCM
        int EncryptData(const RecordHeader& header, const uint8_t* plaintext, size_t plaintext_len,
                        uint8_t* ciphertext, uint8_t* tag);

        // Decrypt data using AES-256-GCM
        int DecryptData(const RecordHeader& header, const uint8_t* ciphertext, size_t ciphertext_len,
                        const uint8_t* tag, uint8_t* plaintext);

        // Derive key check value
        static void DeriveKeyCheck(const uint8_t* salt, const uint8_t* key, uint8_t* key_check);
    };
}

#endif //POCKETTRUSTEE_APP_VAULT_HPP

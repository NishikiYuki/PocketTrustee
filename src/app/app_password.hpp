#ifndef POCKETTRUSTEE_APP_PASSWORD_HPP
#define POCKETTRUSTEE_APP_PASSWORD_HPP

#include <cstdint>
#include <string>
#include <span>
#include <optional>

class PasswordRecord {
public:
    PasswordRecord() = default;

    struct GenerateConfig {
        uint8_t Length;
        bool ContainSpecial;
    };

    static PasswordRecord Generate(const GenerateConfig& config);

    bool Serialize(const std::span<uint8_t>& buffer, size_t& size) const;

    static std::optional<PasswordRecord> TryDeserialize(const std::span<uint8_t>& buffer);

    std::string Name;
    std::string Account;
    std::string Password;
};

#endif //POCKETTRUSTEE_APP_PASSWORD_HPP

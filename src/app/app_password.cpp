#include "app_password.hpp"
#include <vector>
#include "psa/crypto.h"
#include "zcbor_encode.h"

PasswordRecord PasswordRecord::Generate(const GenerateConfig& config) {
    constexpr char LetterBase[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_-+=[]{}|;:,.<>?`~";
    PasswordRecord record;
    size_t len = config.Length > 32 ? 32 : config.Length;
    std::vector<uint8_t> poll(len);
    if (psa_generate_random(poll.data(), len) != PSA_SUCCESS) {
        return record;
    }
    record.Name.resize(len);
    len = sizeof(LetterBase) - 1;
    if (!config.ContainSpecial) {
        len -= 28;
    }
    for (size_t i = 0; i < poll.size(); i++) {
        record.Name[i] = LetterBase[poll[i] % len];
    }
    return record;
}

bool PasswordRecord::Serialize(const std::span<uint8_t>& buffer, size_t& size) const {
    ZCBOR_STATE_E(state, 2, buffer.data(), buffer.size(), 1);
    bool success = zcbor_list_start_encode(state, 3);
    if (!success) return false;
    success = zcbor_tstr_put_term(state, Name.c_str(), Name.length());
    success = success && zcbor_tstr_put_term(state, Account.c_str(), Account.length());
    success = success && zcbor_tstr_put_term(state, Password.c_str(), Password.length());
    success = success && zcbor_list_end_encode(state, 3);
    size = state->payload - buffer.data();
    return success;
}

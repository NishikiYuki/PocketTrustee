#ifndef POCKETTRUSTEE_DRIVER_AES_HPP
#define POCKETTRUSTEE_DRIVER_AES_HPP

#include <cstdint>

namespace PocketTrustee {
    /**
     * @brief AES-ECB 加解密接口(driver 层定义,ll 层实现)。
     *        密钥由调用方通过 SetKey 覆盖(默认密钥在 ll 层初始化时设置)
     */
    class IAesEcbCipher {
    public:
        virtual ~IAesEcbCipher() = default;

        /** @brief 覆盖当前密钥;key 须为 16 字节(AES-128) */
        virtual void SetKey(const uint8_t* key) = 0;

        /** @return HAL_StatusTypeDef 值(0 = HAL_OK) */
        virtual int Encrypt(const uint8_t* in, uint32_t in_len, uint8_t* out, uint32_t timeout_ms) = 0;
        virtual int Decrypt(const uint8_t* in, uint32_t in_len, uint8_t* out, uint32_t timeout_ms) = 0;
    };
}

#endif //POCKETTRUSTEE_DRIVER_AES_HPP

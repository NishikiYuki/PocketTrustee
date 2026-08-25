/* Project platform hooks for Mbed TLS on the STM32 target. */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "stm32l4xx_hal.h"
#include "stm32l4xx_ll_rng.h"

#include "mbedtls/platform.h"
#include "psa/crypto.h"

mbedtls_ms_time_t mbedtls_ms_time(void) { return (mbedtls_ms_time_t)HAL_GetTick(); }

int mbedtls_platform_get_entropy(psa_driver_get_entropy_flags_t flags, size_t* estimate_bits, unsigned char* output,
                                 size_t output_size) {
    const uint32_t timeout = 100U;
    const uint32_t start = HAL_GetTick();
    size_t offset = 0;

    (void)flags;

    while (offset < output_size) {
        while (!LL_RNG_IsActiveFlag_DRDY(RNG)) {
            if ((HAL_GetTick() - start) > timeout) {
                return PSA_ERROR_HARDWARE_FAILURE;
            }
        }

        const uint32_t random_word = LL_RNG_ReadRandData32(RNG);
        const size_t remaining = output_size - offset;
        const size_t copy_size = remaining < sizeof(random_word) ? remaining : sizeof(random_word);

        memcpy(output + offset, &random_word, copy_size);
        offset += copy_size;
    }

    *estimate_bits = output_size * 8U;
    return PSA_SUCCESS;
}

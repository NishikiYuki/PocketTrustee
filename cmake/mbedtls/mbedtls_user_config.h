#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

void* osCalloc(size_t count, size_t size);
void osFree(void* p);
time_t osRtcTime(time_t* time);

#ifdef __cplusplus
}
#endif

#undef MBEDTLS_NET_C
#undef MBEDTLS_TIMING_C
#define MBEDTLS_HAVE_TIME_DATE
#define MBEDTLS_PLATFORM_TIME_MACRO osRtcTime
#define MBEDTLS_PLATFORM_MS_TIME_ALT
#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_PLATFORM_CALLOC_MACRO osCalloc
#define MBEDTLS_PLATFORM_FREE_MACRO osFree
#define MBEDTLS_PSA_DRIVER_GET_ENTROPY
#define MBEDTLS_MD_C
#undef MBEDTLS_PSA_BUILTIN_GET_ENTROPY
#undef MBEDTLS_FS_IO
#undef MBEDTLS_PSA_CRYPTO_STORAGE_C
#undef MBEDTLS_PSA_ITS_FILE_C

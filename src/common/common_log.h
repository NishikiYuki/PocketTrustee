#ifndef POCKETTRUSTEE_COMMON_LOG_HPP
#define POCKETTRUSTEE_COMMON_LOG_HPP

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned long int HAL_GetTick();

#define PKT_DEBUG_LEVEL_NONE (0)
#define PKT_DEBUG_LEVEL_ERROR (1)
#define PKT_DEBUG_LEVEL_WARN (2)
#define PKT_DEBUG_LEVEL_INFO (3)

#ifndef PKT_DEBUG_LEVEL
#define PKT_DEBUG_LEVEL PKT_DEBUG_LEVEL_NONE
#endif

#if PKT_DEBUG_LEVEL > PKT_DEBUG_LEVEL_NONE
#include <stdio.h>
#endif

#if PKT_DEBUG_LEVEL >= PKT_DEBUG_LEVEL_INFO
#define DEBUG_INFO(fmt, ...) printf("[%lu][I]" fmt "\n", HAL_GetTick(), ##__VA_ARGS__)
#else
#define DEBUG_INFO(...)
#endif

#if PKT_DEBUG_LEVEL >= PKT_DEBUG_LEVEL_WARN
#define DEBUG_WARN(fmt, ...) printf("[%lu][W]" fmt "\n", HAL_GetTick(), ##__VA_ARGS__)
#else
#define DEBUG_WARN(...)
#endif

#if PKT_DEBUG_LEVEL >= PKT_DEBUG_LEVEL_ERROR
#define DEBUG_ERROR(fmt, ...) printf("[%lu][E]" fmt "\n", HAL_GetTick(), ##__VA_ARGS__)
#else
#define DEBUG_ERROR(...)
#endif

#ifdef PKT_ENABLE_ASSERT
#define DEBUG_ASSERT(f) \
do { \
    if (!(f)) {DEBUG_ERROR("Assert Fail %s:%d", __FILE__, __LINE__); while(1){}; } \
} while(0)
#endif

struct Exception {
    const char* msg;
    int err;
    int line;
};

#ifdef __cplusplus
}
#endif

#endif //POCKETTRUSTEE_COMMON_LOG_HPP

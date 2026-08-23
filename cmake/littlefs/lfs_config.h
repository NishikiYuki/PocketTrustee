#ifndef POCKETTRUSTEE_LFS_CONFIG_H
#define POCKETTRUSTEE_LFS_CONFIG_H

#include <stddef.h>

extern void* osMalloc(size_t size);
extern void osFree(void* p);

#define LFS_MALLOC(x) osMalloc(x)
#define LFS_FREE(x) osFree(x)

#endif //POCKETTRUSTEE_LFS_CONFIG_H

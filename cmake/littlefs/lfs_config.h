#ifndef POCKETTRUSTEE_LFS_CONFIG_H
#define POCKETTRUSTEE_LFS_CONFIG_H

#include <stddef.h>

extern void *pvPortMalloc( size_t xWantedSize );
extern void vPortFree( void *pv );

#define LFS_MALLOC(x) pvPortMalloc(x)
#define LFS_FREE(x) vPortFree(x)

#endif //POCKETTRUSTEE_LFS_CONFIG_H

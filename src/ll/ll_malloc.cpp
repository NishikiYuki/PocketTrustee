#include "main.h"
#include "FreeRTOS.h"
#include "portable.h"

extern "C" {
void* osMalloc(size_t size) {
    auto p = pvPortMalloc(size);
    DEBUG_ASSERT(p != nullptr);
    DEBUG_INFO("malloc %d at %p", size, p);
    return p;
}

void osFree(void* p) {
    DEBUG_INFO("free %p", p);
    vPortFree(p);
}
}

void* operator new(size_t size) {
    auto p = pvPortMalloc(size);
    DEBUG_ASSERT(p != nullptr);
    DEBUG_INFO("malloc %d at %p", size, p);
    return p;
}

void operator delete(void* p) noexcept {
    DEBUG_INFO("free %p", p);
    vPortFree(p);
}

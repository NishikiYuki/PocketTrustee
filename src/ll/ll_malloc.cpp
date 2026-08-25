#include "FreeRTOS.h"
#include "main.h"
#include "portable.h"
#include <cstring>
#include <limits>

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

void* osCalloc(size_t count, size_t size) {
    if (size != 0 && count > std::numeric_limits<size_t>::max() / size) {
        return nullptr;
    }

    const size_t total_size = count * size;
    auto* p = osMalloc(total_size);
    if (p != nullptr) {
        std::memset(p, 0, total_size);
    }
    return p;
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

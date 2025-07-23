// kernel/memory.cpp
#include <stddef.h>
#include "memory.h"

void* operator new(size_t size) {
    return kmalloc(size); // Usa tu allocator del kernel
}

void* operator new[](size_t size) {
    return kmalloc(size);
}

void operator delete(void* ptr) noexcept {
    kfree(ptr);
}

void operator delete[](void* ptr) noexcept {
    kfree(ptr);
}

void operator delete(void* ptr, size_t) noexcept {
    kfree(ptr);
}

void operator delete[](void* ptr, size_t) noexcept {
    kfree(ptr);
}

// kernel/memory.cpp (continuación)
static char memory_pool[1024 * 1024]; // 1MB pool
static size_t pool_offset = 0;

void* kmalloc(size_t size) {
    if (pool_offset + size > sizeof(memory_pool)) return nullptr;
    void* ptr = &memory_pool[pool_offset];
    pool_offset += size;
    return ptr;
}

void kfree(void* ptr) {
    // Implementación simple (no hace nada en esta versión)
    (void)ptr;
}
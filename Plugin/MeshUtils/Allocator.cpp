#include "pch.h"
#include "Allocator.h"

void* AlignedMalloc(size_t size, size_t alignment)
{
    size_t mask = alignment - 1;
    size = (size + mask) & (~mask);
#ifdef __APPLE__
    void *ret;
    posix_memalign(&ret, alignment, size);
    return ret;
#else
    return _mm_malloc(size, alignment);
#endif
}

void AlignedFree(void *addr)
{
#ifdef __APPLE__
    free(addr);
#else
    _mm_free(addr);
#endif
}

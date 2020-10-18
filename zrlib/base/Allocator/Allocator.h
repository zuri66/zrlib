/**
 * @author zuri
 * @date dimanche 20 janvier 2019, 18:08:52 (UTC+0100)
 */

#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <zrlib/config.h>
#include <zrlib/base/struct.h>

#include <stddef.h>

// ============================================================================

typedef struct ZRAllocatorS ZRAllocator;

struct ZRAllocatorS
{
	void* (*falloc)__(ZRAllocator *allocator, size_t nbBytes);
	void* (*faalloc)_(ZRAllocator *allocator, size_t alignment, size_t nbBytes);
	void* (*frealloc)(ZRAllocator *allocator, void *allocated, size_t nbBytes);
	void (*ffree)(ZRAllocator *allocator, void *allocated);
};

// ============================================================================

ZRMUSTINLINE
static inline void* ZRALLOC(ZRAllocator *allocator, size_t nbBytes)
{
	return allocator->falloc(allocator, nbBytes);
}

ZRMUSTINLINE
static inline void* ZRAALLOC(ZRAllocator *allocator, size_t alignment, size_t nbBytes)
{
	return allocator->faalloc(allocator, alignment, nbBytes);
}

ZRMUSTINLINE
static inline void* ZROBJALLOC(ZRAllocator *allocator, ZRObjInfos objInfos)
{
	return allocator->faalloc(allocator, ZROBJINFOS_ALIGNMENT_SIZE(objInfos));
}

ZRMUSTINLINE
static inline void* ZRREALLOC(ZRAllocator *allocator, void *allocated, size_t nbBytes)
{
	return allocator->frealloc(allocator, allocated, nbBytes);
}

ZRMUSTINLINE
static inline void ZRFREE(ZRAllocator *allocator, void *allocated)
{
	allocator->ffree(allocator, allocated);
}

#endif

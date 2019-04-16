/**
 * @author zuri
 * @date dimanche 20 janvier 2019, 18:08:52 (UTC+0100)
 */

#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <zrlib/config.h>

#include <stddef.h>
#include <zrlib/syntax_pad.h>

// ============================================================================

typedef struct ZRAllocatorS ZRAllocator;

struct ZRAllocatorS
{
	void* (*falloc)__(ZRAllocator *allocator, size_t nbBytes);
	void* (*frealloc)(ZRAllocator *allocator, void *allocated, size_t nbBytes);
	void _(*ffree)___(ZRAllocator *allocator, void *allocated);
};

// ============================================================================

#define ZRALLOC(allocator,nbBytes) (allocator)->falloc(allocator,nbBytes)
#define ZRREALLOC(allocator,allocated,nbBytes) (allocator)->frealloc(allocator,allocated,nbBytes)
#define ZRFREE(allocator,allocated) (allocator)->ffree(allocator,allocated)

#endif

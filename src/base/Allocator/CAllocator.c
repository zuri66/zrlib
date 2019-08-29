#include <zrlib/base/Allocator/CAllocator.h>

#include <stdlib.h>

static void* _alloc(ZRAllocator *allocator, size_t nbBytes)
{
	return malloc(nbBytes);
}

static void* _aalloc(ZRAllocator *allocator, size_t alignment, size_t nbBytes)
{
	return aligned_alloc(alignment, nbBytes);
}

static void* _realloc(ZRAllocator *allocator, void *allocated, size_t nbBytes)
{
	return realloc(allocated, nbBytes);
}

static void _free(ZRAllocator *allocator, void *allocated)
{
	free(allocated);
}

void ZRCAllocator_init(ZRAllocator *allocator)
{
	ZRAllocator tmp = { .falloc = _alloc, .faalloc = _aalloc, .frealloc = _realloc, .ffree = _free };
	*allocator = tmp;
}

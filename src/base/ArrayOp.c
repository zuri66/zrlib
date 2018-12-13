#include <zrlib/base/ArrayOp.h>
#include <zrlib/base/MemoryOp.h>

#include <string.h>

void* ZRArrayOp_get(void *offset, size_t objSize, size_t pos)
{
	return ZRARRAYOP_GET(offset, objSize, pos);
}

void ZRArrayOp_set(void *restrict offset, size_t objSize, size_t pos, void *restrict source)
{
	ZRARRAYOP_SET(offset, objSize, pos, source);
}

void ZRArrayOp_swap(void *offset, size_t objSize, size_t posa, size_t posb)
{
	ZRARRAYOP_SWAP(offset, objSize, posa, posb);
}

void ZRArrayOp_shift(void *offset, size_t objSize, size_t nbObj, size_t shift, bool toTheRight)
{
	ZRARRAYOP_SHIFT(offset, objSize, nbObj, shift, toTheRight);
}

void ZRArrayOp_rotate(void *offset, size_t objSize, size_t nbObj, size_t rotate, bool toTheRight)
{
	ZRARRAYOP_ROTATE(offset, objSize, nbObj, rotate, toTheRight);
}

void ZRArrayOp_reverse(void *offset, size_t objSize, size_t nbObj)
{
	ZRARRAYOP_REVERSE(offset, objSize, nbObj);
}

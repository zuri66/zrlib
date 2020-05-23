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

void ZRArrayOp_fill(void *restrict offset, size_t objSize, size_t nbObj, void *restrict object)
{
	ZRARRAYOP_FILL(offset, objSize, nbObj, object);
}

void ZRArrayOp_cpy(void *restrict offset, size_t objSize, size_t nbObj, void *restrict source)
{
	ZRARRAYOP_CPY(offset, objSize, nbObj, source);
}

void ZRArrayOp_move(void *offset, size_t objSize, size_t nbObj, void *source)
{
	ZRARRAYOP_MOVE(offset, objSize, nbObj, source);
}

void ZRArrayOp_deplace(void *offset, size_t objSize, size_t nbObj, void *source)
{
	ZRARRAYOP_DEPLACE(offset, objSize, nbObj, source);
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

void* ZRArrayOp_search(
	void *offset, size_t objSize, size_t nbObj, void *search,
	int (*fcmp)(void *a, void *b)
	)
{
	return ZRARRAYOP_SEARCH(offset, objSize, nbObj, search, fcmp);
}

void* ZRArrayOp_bsearch(
	void *offset, size_t objSize, size_t nbObj, void *search,
	int (*fcmp)(void *a, void *b)
	)
{
	return ZRARRAYOP_BSEARCH(offset, objSize, nbObj, search, fcmp);
}

size_t ZRArrayOp_binsert_pos(
	void *offset, size_t objSize, size_t nbObj, void *search,
	int (*fcmp)(void *a, void *b)
	)
{
	return ZRARRAYOP_BINSERT_POS(offset, objSize, nbObj, search, fcmp);
}

void ZRArrayOp_walk(void *offset, size_t objSize, size_t nbObj, void (*fconsume)(void *item))
{
	ZRARRAYOP_WALK(offset, objSize, nbObj, fconsume);
}

void ZRArrayOp_map(void *restrict offset, size_t objSize, size_t nbObj, void (*fmap)(void *restrict item, void *restrict out), void *restrict dest, size_t dest_objSize, size_t dest_nbObj)
{
	ZRARRAYOP_MAP(offset, objSize, nbObj, fmap, dest, dest_objSize, dest_nbObj);
}

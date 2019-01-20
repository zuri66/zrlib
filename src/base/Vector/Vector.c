/**
 * @author zuri
 * @date dimanche 20 janvier 2019, 18:00:06 (UTC+0100)
 */

#include <zrlib/base/Vector/Vector.h>

ZRVector ZRVector_init(size_t objSize, ZRAllocator *allocator, ZRVectorStrategy *strategy)
{
	return ZRVECTOR_INIT(objSize, allocator, strategy);
}

void ZRVector_construct(ZRVector *vec, size_t objSize, ZRAllocator *allocator, ZRVectorStrategy *strategy)
{
	ZRVECTOR_CONSTRUCT(vec, objSize, allocator, strategy);
}

size_t ZRVector_size(ZRVector *vec)
{
	ZRVECTOR_SIZE(vec);
}

void ZRVector_insert(ZRVector *vec, size_t pos, void *obj)
{
	ZRVECTOR_INSERT(vec, pos, obj);
	vec->nbObj++;
}

void ZRVector_delete(ZRVector *vec, size_t pos)
{
	ZRVECTOR_DELETE(vec, pos);
	vec->nbObj--;
}

void* ZRVector_get(ZRVector *vec, size_t pos)
{
	return ZRVECTOR_GET(vec, pos);
}

void ZRVector_set(ZRVector *vec, size_t pos, void *obj)
{
	ZRVECTOR_SET(vec, pos, obj);
}

void ZRVector_add(ZRVector *vec, void *obj)
{
	ZRVECTOR_ADD(vec, obj);
}

void ZRVector_addFirst(ZRVector *vec, void *obj)
{
	ZRVECTOR_ADDFIRST(vec, obj);
}

void ZRVector_dec(ZRVector *vec)
{
	ZRVECTOR_DEC(vec);

}
void ZRVector_decFirst(ZRVector *vec)
{
	ZRVECTOR_DECFIRST(vec);
}

void ZRVector_pop(ZRVector *vec, void *dest)
{
	ZRVECTOR_POP(vec, dest);
}

void ZRVector_popFirst(ZRVector *vec, void *dest)
{
	ZRVECTOR_POPFIRST(vec, dest);
}

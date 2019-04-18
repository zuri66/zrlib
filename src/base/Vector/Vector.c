/**
 * @author zuri
 * @date dimanche 20 janvier 2019, 18:00:06 (UTC+0100)
 */

#include <zrlib/base/Vector/Vector.h>

void ZRVector_init(ZRVector *vec, size_t objSize, ZRVectorStrategy *strategy)
{
	ZRVECTOR_INIT(vec, objSize, strategy);
}

size_t ZRVector_nbObj(ZRVector *vec)
{
	return ZRVECTOR_NBOBJ(vec);
}

size_t ZRVector_objSize(ZRVector *vec)
{
	return ZRVECTOR_OBJSIZE(vec);
}

void ZRVector_insert(ZRVector *vec, size_t pos, void *obj)
{
	ZRVECTOR_INSERT(vec, pos, obj);
}

void ZRVector_insert_nb(ZRVector *vec, size_t pos, size_t nb, void *obj)
{
	ZRVECTOR_INSERT_NB(vec, pos, nb, obj);
}

void ZRVector_delete(ZRVector *vec, size_t pos)
{
	ZRVECTOR_DELETE(vec, pos);
}

void ZRVector_delete_nb(ZRVector *vec, size_t pos, size_t nb)
{
	ZRVECTOR_DELETE_NB(vec, pos, nb);
}

void ZRVector_delete_all(ZRVector *vec, size_t pos)
{
	ZRVECTOR_DELETE_ALL(vec, pos);
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

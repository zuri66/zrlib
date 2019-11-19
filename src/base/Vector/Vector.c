/**
 * @author zuri
 * @date dimanche 20 janvier 2019, 18:00:06 (UTC+0100)
 */

#include <zrlib/base/Vector/Vector.h>

void ZRVector_init(ZRVector *vec, size_t objSize, ZRVectorStrategy *strategy)
{
	ZRVECTOR_INIT(vec, objSize, strategy);
}

void ZRVector_done(ZRVector *vec)
{
	ZRVECTOR_DONE(vec);
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

void ZRVector_delete_all(ZRVector *vec)
{
	ZRVECTOR_DELETE_ALL(vec);
}

void* ZRVector_get(ZRVector *vec, size_t pos)
{
	return ZRVECTOR_GET(vec, pos);
}

void ZRVector_get_nb(ZRVector *vec, size_t pos, size_t nb, void *restrict dest)
{
	return ZRVECTOR_GET_NB(vec, pos, nb, dest);
}

void ZRVector_set(ZRVector *vec, size_t pos, void *obj)
{
	ZRVECTOR_SET(vec, pos, obj);
}

void ZRVector_set_nb(ZRVector *vec, size_t pos, size_t nb, void *src)
{
	ZRVECTOR_SET_NB(vec, pos, nb, src);
}

void ZRVector_reserve(ZRVector *vec, size_t pos, size_t nb)
{
	ZRVECTOR_RESERVE(vec, pos, nb);
}

void ZRVector_add(ZRVector *vec, void *obj)
{
	ZRVECTOR_ADD(vec, obj);
}

void ZRVector_add_nb(ZRVector *vec, size_t nb, void *src)
{
	ZRVECTOR_ADD_NB(vec, nb, src);
}

void ZRVector_addFirst(ZRVector *vec, void *obj)
{
	ZRVECTOR_ADDFIRST(vec, obj);
}

void ZRVector_addFirst_nb(ZRVector *vec, size_t nb, void *src)
{
	ZRVECTOR_ADDFIRST_NB(vec, nb, src);
}

void ZRVector_dec(ZRVector *vec)
{
	ZRVECTOR_DEC(vec);
}

void ZRVector_dec_nb(ZRVector *vec, size_t nb)
{
	ZRVECTOR_DEC_NB(vec, nb);
}

void ZRVector_decFirst(ZRVector *vec)
{
	ZRVECTOR_DECFIRST(vec);
}

void ZRVector_decFirst_nb(ZRVector *vec, size_t nb)
{
	ZRVECTOR_DECFIRST_NB(vec, nb);
}

void ZRVector_pop(ZRVector *vec, void *restrict dest)
{
	ZRVECTOR_POP(vec, dest);
}

void ZRVector_pop_nb(ZRVector *vec, size_t nb, void *restrict dest)
{
	ZRVECTOR_POP_NB(vec, nb, dest);
}

void ZRVector_popFirst(ZRVector *vec, void *restrict dest)
{
	ZRVECTOR_POPFIRST(vec, dest);
}

void ZRVector_popFirst_nb(ZRVector *vec, size_t nb, void *restrict dest)
{
	ZRVECTOR_POPFIRST_NB(vec, nb, dest);
}

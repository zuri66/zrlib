/**
 * @author zuri
 * @date dimanche 20 janvier 2019, 18:00:06 (UTC+0100)
 */

#include <zrlib/base/Vector/Vector.h>

void ZRVector_init(ZRVector *vec, size_t objSize, size_t objAlignment, ZRVectorStrategy *strategy)
{
	ZRVECTOR_INIT(vec, objSize, objAlignment, strategy);
}

void ZRVector_copy(ZRVector *restrict dest, ZRVector *restrict src)
{
	ZRVECTOR_COPY(dest, src);
}

void ZRVector_done(ZRVector *vec)
{
	ZRVECTOR_DONE(vec);
}

void ZRVector_destroy(ZRVector *vec)
{
	ZRVECTOR_DESTROY(vec);
}

void ZRVector_changeObjSize(ZRVector *vec, size_t objSize, size_t objAlignment)
{
	ZRVECTOR_CHANGEOBJSIZE(vec, objSize, objAlignment);
}

void ZRVector_memoryTrim(ZRVector *vec)
{
	ZRVECTOR_MEMORYTRIM(vec);
}

void *ZRVector_parray(ZRVector *vec){
	return ZRVECTOR_PARRAY(vec);
}

size_t ZRVector_nbObj(ZRVector *vec)
{
	return ZRVECTOR_NBOBJ(vec);
}

size_t ZRVector_objSize(ZRVector *vec)
{
	return ZRVECTOR_OBJSIZE(vec);
}

size_t ZRVector_objAlignment(ZRVector *vec)
{
	return ZRVECTOR_OBJALIGNMENT(vec);
}

size_t ZRVector_capacity(ZRVector *vec)
{
	return ZRVECTOR_CAPACITY(vec);
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

void ZRVector_get_nb(ZRVector *vec, size_t pos, size_t nb, void *dest)
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

void ZRVector_fill(_ ZRVector *vec, size_t pos, size_t nb, void *obj)
{
	ZRVECTOR_FILL(vec, pos, nb, obj);
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

void ZRVector_pop(ZRVector *vec, void *dest)
{
	ZRVECTOR_POP(vec, dest);
}

void ZRVector_pop_nb(ZRVector *vec, size_t nb, void *dest)
{
	ZRVECTOR_POP_NB(vec, nb, dest);
}

void ZRVector_popFirst(ZRVector *vec, void *dest)
{
	ZRVECTOR_POPFIRST(vec, dest);
}

void ZRVector_popFirst_nb(ZRVector *vec, size_t nb, void *dest)
{
	ZRVECTOR_POPFIRST_NB(vec, nb, dest);
}

// ============================================================================
// Pointer help functions

void ZRVector_setPtr_nb(ZRVector *vec, size_t pos, size_t nb, void *src, size_t srcObjSize)
{
	ZRVECTOR_SETPTR_NB(vec, pos, nb, src, srcObjSize);
}

void ZRVector_insertPtr_nb(ZRVector *vec, size_t pos, size_t nb, void *src, size_t srcObjSize)
{
	ZRVECTOR_INSERTPTR_NB(vec, pos, nb, src, srcObjSize);
}

void ZRVector_addPtr_nb(ZRVector *vec, size_t nb, void *src, size_t srcObjSize)
{
	ZRVECTOR_ADDPTR_NB(vec, nb, src, srcObjSize);
}

void ZRVector_addFirstPtr_nb(ZRVector *vec, size_t nb, void *src, size_t srcObjSize)
{
	ZRVECTOR_ADDFIRSTPTR_NB(vec, nb, src, srcObjSize);
}


/**
 * @author zuri
 * @date lundi 25 novembre 2019, 23:15:50 (UTC+0100)
 */

#include <zrlib/base/List/List.h>

void ZRList_init(ZRList *list, size_t objSize, ZRListStrategy *strategy)
{
	ZRLIST_INIT(list, objSize, strategy);
}

void ZRList_done(ZRList *list)
{
	ZRLIST_DONE(list);
}

size_t ZRList_nbObj(ZRList *list)
{
	return ZRLIST_NBOBJ(list);
}

size_t ZRList_objSize(ZRList *list)
{
	return ZRLIST_OBJSIZE(list);
}

void ZRList_insert(ZRList *list, size_t pos, void *obj)
{
	ZRLIST_INSERT(list, pos, obj);
}

void ZRList_insert_nb(ZRList *list, size_t pos, size_t nb, void *obj)
{
	ZRLIST_INSERT_NB(list, pos, nb, obj);
}

void ZRList_delete(ZRList *list, size_t pos)
{
	ZRLIST_DELETE(list, pos);
}

void ZRList_delete_nb(ZRList *list, size_t pos, size_t nb)
{
	ZRLIST_DELETE_NB(list, pos, nb);
}

void ZRList_delete_all(ZRList *list)
{
	ZRLIST_DELETE_ALL(list);
}

void* ZRList_get(ZRList *list, size_t pos)
{
	return ZRLIST_GET(list, pos);
}

void ZRList_get_nb(ZRList *list, size_t pos, size_t nb, void * dest)
{
	return ZRLIST_GET_NB(list, pos, nb, dest);
}

void ZRList_set(ZRList *list, size_t pos, void *obj)
{
	ZRLIST_SET(list, pos, obj);
}

void ZRList_set_nb(ZRList *list, size_t pos, size_t nb, void *src)
{
	ZRLIST_SET_NB(list, pos, nb, src);
}

void ZRList_fill(_ ZRList *list, size_t pos, size_t nb, void *obj)
{
	ZRLIST_FILL(list, pos, nb, obj);
}

void ZRList_add(ZRList *list, void *obj)
{
	ZRLIST_ADD(list, obj);
}

void ZRList_add_nb(ZRList *list, size_t nb, void *src)
{
	ZRLIST_ADD_NB(list, nb, src);
}

void ZRList_addFirst(ZRList *list, void *obj)
{
	ZRLIST_ADDFIRST(list, obj);
}

void ZRList_addFirst_nb(ZRList *list, size_t nb, void *src)
{
	ZRLIST_ADDFIRST_NB(list, nb, src);
}

void ZRList_dec(ZRList *list)
{
	ZRLIST_DEC(list);
}

void ZRList_dec_nb(ZRList *list, size_t nb)
{
	ZRLIST_DEC_NB(list, nb);
}

void ZRList_decFirst(ZRList *list)
{
	ZRLIST_DECFIRST(list);
}

void ZRList_decFirst_nb(ZRList *list, size_t nb)
{
	ZRLIST_DECFIRST_NB(list, nb);
}

void ZRList_pop(ZRList *list, void * dest)
{
	ZRLIST_POP(list, dest);
}

void ZRList_pop_nb(ZRList *list, size_t nb, void * dest)
{
	ZRLIST_POP_NB(list, nb, dest);
}

void ZRList_popFirst(ZRList *list, void * dest)
{
	ZRLIST_POPFIRST(list, dest);
}

void ZRList_popFirst_nb(ZRList *list, size_t nb, void * dest)
{
	ZRLIST_POPFIRST_NB(list, nb, dest);
}

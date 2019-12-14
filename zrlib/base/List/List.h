/**
 * @author zuri
 * @date lundi 25 novembre 2019, 23:16:00 (UTC+0100)
 */

#ifndef LIST_H
#define LIST_H

#include <zrlib/config.h>
#include <zrlib/base/Allocator/Allocator.h>

#include <stddef.h>
#include <string.h>

// ============================================================================

typedef struct ZRListS ZRList;
typedef struct ZRListStrategyS ZRListStrategy;

// ============================================================================

struct ZRListStrategyS
{
	size_t (*fsdataSize)(ZRList *list);
	size_t (*fstrategySize)();

	/**
	 * (optional)
	 */
	void (*finitList)(ZRList *list);

	/*
	 * The insert/delete functions are responsible to update properly the list.nbObj value.
	 */
	void (*finsert)(ZRList *list, size_t pos, size_t nb);
	void (*fdelete)(ZRList *list, size_t pos, size_t nb);

	void* (*fget)(ZRList *list, size_t pos);

	/**
	 * Clean the memory used by the list.
	 * The list MUST NOT be used after this call.
	 */
	void (*fdone)(ZRList *list);
};

struct ZRListS
{
	size_t objSize;
	size_t nbObj;

	/*
	 * The strategy for memory management and insertion/deletion routines.
	 */
	ZRListStrategy *strategy;

	/*
	 * Data for Strategy purpose.
	 */
	char sdata[];
};

// ============================================================================

static inline void ZRLIST_INIT(ZRList *list, size_t objSize, ZRListStrategy *strategy)
{
	*list = (ZRList )
		{ //
		.objSize = objSize, //
		.nbObj = 0, //
		.strategy = strategy, //
		};

	/*
	 * Initialisation of the listtor
	 */
	if (strategy->finitList)
		strategy->finitList(list);
}

static inline void ZRLIST_DONE(ZRList *list)
{
	list->strategy->fdone(list);
}

static inline size_t ZRLIST_NBOBJ(ZRList *list)
{
	return list->nbObj;
}

static inline size_t ZRLIST_OBJSIZE(ZRList *list)
{
	return list->objSize;
}

static inline void* ZRLIST_GET(ZRList *list, size_t pos)
{
	return list->strategy->fget(list, pos);
}

static inline void ZRLIST_GET_NB(ZRList *list, size_t pos, size_t nb, void *dest)
{
	for (size_t i = pos; nb; nb--, i++)
	{
		memcpy(dest, ZRLIST_GET(list, i), list->objSize);
		dest = (char*)dest + list->objSize;
	}
}

static inline void ZRLIST_SET(ZRList *list, size_t pos, void *obj)
{
	memcpy(ZRLIST_GET(list, pos), obj, list->objSize);
}

static inline void ZRLIST_SET_NB(ZRList *list, size_t pos, size_t nb, void *src)
{
	for (size_t i = pos; nb; nb--, i++)
	{
		memcpy(ZRLIST_GET(list, i), src, list->objSize);
		src = (char*)src + list->objSize;
	}
}

static inline void ZRLIST_INSERT(ZRList *list, size_t pos, void *obj)
{
	list->strategy->finsert(list, pos, 1);
	ZRLIST_SET(list, pos, obj);
}

static inline void ZRLIST_INSERT_NB(ZRList *list, size_t pos, size_t nb, void *src)
{
	list->strategy->finsert(list, pos, nb);
	ZRLIST_SET_NB(list, pos, nb, src);
}

static inline void ZRLIST_FILL(ZRList *list, size_t pos, size_t nb, void *obj)
{
	list->strategy->finsert(list, pos, nb);

	for (size_t i = pos; nb; nb--, i++)
		memcpy(ZRLIST_GET(list, i), obj, list->objSize);
}

static inline void ZRLIST_DELETE(ZRList *list, size_t pos)
{
	list->strategy->fdelete(list, pos, 1);
}

static inline void ZRLIST_DELETE_NB(ZRList *list, size_t pos, size_t nb)
{
	list->strategy->fdelete(list, pos, nb);
}

static inline void ZRLIST_DELETE_ALL(ZRList *list)
{
	list->strategy->fdelete(list, 0, list->nbObj);
}

static inline void ZRLIST_ADD(ZRList *list, void *obj)
{
	ZRLIST_INSERT(list, list->nbObj, obj);
}

static inline void ZRLIST_ADD_NB(ZRList *list, size_t nb, void *src)
{
	ZRLIST_INSERT_NB(list, list->nbObj, nb, src);
}

static inline void ZRLIST_ADDFIRST(ZRList *list, void *obj)
{
	ZRLIST_INSERT(list, 0, obj);
}

static inline void ZRLIST_ADDFIRST_NB(ZRList *list, size_t nb, void *src)
{
	ZRLIST_INSERT_NB(list, 0, nb, src);
}

static inline void ZRLIST_DEC(ZRList *list)
{
	ZRLIST_DELETE(list, list->nbObj - 1);
}

static inline void ZRLIST_DEC_NB(ZRList *list, size_t nb)
{
	ZRLIST_DELETE_NB(list, list->nbObj - nb, nb);
}

static inline void ZRLIST_DECFIRST(ZRList *list)
{
	ZRLIST_DELETE(list, 0);
}

static inline void ZRLIST_DECFIRST_NB(ZRList *list, size_t nb)
{
	ZRLIST_DELETE_NB(list, 0, nb);
}

static inline void ZRLIST_POP(ZRList *list, void *dest)
{
	memcpy(dest, ZRLIST_GET(list, list->nbObj - 1), list->objSize);
	ZRLIST_DEC(list);
}

static inline void ZRLIST_POP_NB(ZRList *list, size_t nb, void *dest)
{
	for (size_t i = 0; nb; nb--)
		memcpy(dest, ZRLIST_GET(list, list->nbObj - 1 - i), list->objSize);

	ZRLIST_DEC_NB(list, nb);
}

static inline void ZRLIST_POPFIRST(ZRList *list, void *dest)
{
	memcpy(dest, ZRLIST_GET(list, 0), list->objSize);
	ZRLIST_DECFIRST(list);
}

static inline void ZRLIST_POPFIRST_NB(ZRList *list, size_t nb, void *dest)
{
	for (size_t i = 0; nb; nb--)
		memcpy(dest, ZRLIST_GET(list, i), list->objSize);

	ZRLIST_DECFIRST_NB(list, nb);
}

// ============================================================================

void ZRList_init(ZRList *list, size_t objSize, ZRListStrategy *strategy);
void ZRList_done(ZRList *list);

size_t ZRList_nbObj(_ ZRList *list);
size_t ZRList_objSize(ZRList *list);

void* ZRList_get(__ ZRList *list, size_t pos);
void _ ZRList_get_nb(ZRList *list, size_t pos, size_t nb, void *dest);
void _ ZRList_set(__ ZRList *list, size_t pos, __________ void *obj);
void _ ZRList_set_nb(ZRList *list, size_t pos, size_t nb, void *src);
void _ ZRList_fill(_ ZRList *list, size_t pos, size_t nb, void *obj);

void ZRList_insert(__ ZRList *list, size_t pos, __________ void *obj);
void ZRList_insert_nb(ZRList *list, size_t pos, size_t nb, void *src);

void ZRList_delete(_____ ZRList *list, size_t pos);
void ZRList_delete_nb(__ ZRList *list, size_t pos, size_t nb);
void ZRList_delete_all(_ ZRList *list);

void ZRList_add(_______ ZRList *list, __________ void *obj);
void ZRList_add_nb(____ ZRList *list, size_t nb, void *src);
void ZRList_addFirst(__ ZRList *list, __________ void *obj);
void ZRList_addFirst_nb(ZRList *list, size_t nb, void *src);

void ZRList_dec(_______ ZRList *list);
void ZRList_dec_nb(____ ZRList *list, size_t nb);
void ZRList_decFirst(__ ZRList *list);
void ZRList_decFirst_nb(ZRList *list, size_t nb);

void ZRList_pop(_______ ZRList *list, __________ void *dest);
void ZRList_pop_nb(____ ZRList *list, size_t nb, void *dest);
void ZRList_popFirst(__ ZRList *list, __________ void *dest);
void ZRList_popFirst_nb(ZRList *list, size_t nb, void *dest);

#endif

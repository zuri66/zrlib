/**
 * @author zuri
 * @date samedi 28 juin 2014, 13:27:45 (UTC+0200)
 */

#ifndef ZRMEMORY_ARRAY_OP_H
#define ZRMEMORY_ARRAY_OP_H

#include <zrlib/config.h>
#include <zrlib/base/MemoryOp.h>

#include <zrlib/syntax_pad.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// ============================================================================

#define ZRARRAYOP_GET(offset, objSize, pos) \
	((char*)(offset) + (pos) * (objSize))

#define ZRARRAYOP_SET(offset, objSize, pos, source) \
	memcpy(ZRARRAYOP_GET(offset, objSize, pos), (source), (objSize))

#define ZRARRAYOP_SWAP(offset, objSize, posa, posb) \
	ZRMemoryOp_swap(ZRARRAYOP_GET(offset, objSize, posa), ZRARRAYOP_GET(offset, objSize, posb), (objSize))

#define ZRARRAYOP_SHIFT(offset, objSize, nbObj, shift, toTheRight) \
	ZRMemoryOp_shift((offset), ZRARRAYOP_GET(offset, objSize, nbObj), (shift) * (objSize), (toTheRight))

#define ZRARRAYOP_ROTATE(offset, objSize, nbObj, rotate, toTheRight) \
	ZRMemoryOp_rotate((offset), ZRARRAYOP_GET(offset, objSize, nbObj), (rotate) * (objSize), (toTheRight))

#define ZRARRAYOP_FILL(offset, objSize, nbObj, object) \
	ZRMemoryOp_fill((offset), (object), (objSize), (nbObj))

#define ZRARRAYOP_CPY(offset, objSize, nbObj, source) \
	memcpy((offset), (source), (objSize) * (nbObj))

#define ZRARRAYOP_MOVE(offset, objSize, nbObj, source) \
	memmove((offset), (source), (objSize) * (nbObj))

#define ZRARRAYOP_DEPLACE(offset, objSize, nbObj, source) \
	ZRMemoryOp_deplace((offset), (source), (objSize) * (nbObj))

ZRMUSTINLINE
static inline void ZRARRAYOP_REVERSE(void *offset, size_t objSize, size_t nbObj)
{
	char *a = offset;
	char *b = (char*)offset + (objSize * (nbObj - 1));
	char buffer[objSize];

	nbObj >>= 1;

	while (nbObj--)
	{
		ZRMemoryOp_swapB(a, b, objSize, buffer);
		a += objSize;
		b -= objSize;
	}
}

ZRMUSTINLINE
static inline size_t ZRARRAYOP_SEARCH_POS(void *offset, size_t objSize, size_t nbObj, void *search, zrfucmp fucmp, void *data)
{
	size_t pos = 0;

	while (nbObj--)
	{
		if (0 == fucmp(search, offset, data))
			return pos;

		offset = (char*)offset + objSize;
		pos++;
	}
	return SIZE_MAX;
}

ZRMUSTINLINE
static inline void* ZRARRAYOP_SEARCH(void *offset, size_t objSize, size_t nbObj, void *search, zrfucmp fucmp, void *data)
{
	while (nbObj--)
	{
		if (0 == fucmp(search, offset, data))
			return offset;

		offset = (char*)offset + objSize;
	}
	return NULL;
}

ZRMUSTINLINE
static inline size_t ZRARRAYOP_BSEARCH_POS(void *offset, size_t objSize, size_t nbObj, void *search, zrfucmp fucmp, void *data)
{
	if(nbObj == 0)
		return SIZE_MAX;

	size_t begin = 0;
	size_t end = nbObj - 1;

	while (begin != end)
	{
		size_t const mid = begin + (end - begin + 1) / 2;
		int const cmp = fucmp(search, ZRARRAYOP_GET(offset, objSize, mid), data);

		if (cmp < 0)
			end = mid - 1;
		else
			begin = mid;
	}
	void *const place = ZRARRAYOP_GET(offset, objSize, begin);

	if (fucmp(search, place, data) == 0)
		return begin;

	return SIZE_MAX;
}

ZRMUSTINLINE
static inline void* ZRARRAYOP_BSEARCH(void *offset, size_t objSize, size_t nbObj, void *search, zrfucmp fucmp, void *data)
{
	size_t pos = ZRARRAYOP_BSEARCH_POS(offset, objSize, nbObj, search, fucmp, data);

	if (pos == SIZE_MAX)
		return NULL;

	return ZRARRAYOP_GET(offset, objSize, pos);
}

ZRMUSTINLINE
static inline size_t ZRARRAYOP_BINSERT_POS(void *offset, size_t objSize, size_t nbObj, void *search, zrfucmp fucmp, void *data)
{
	if(nbObj == 0)
		return SIZE_MAX;

	size_t begin = 0;
	size_t end = nbObj - 1;
	int cmp;

	while (begin != end)
	{
		size_t const mid = begin + (end - begin + 1) / 2;
		int const cmp = fucmp(search, ZRARRAYOP_GET(offset, objSize, mid), data);

		if (cmp < 0)
			end = mid - 1;
		else
			begin = mid;
	}

	if (fucmp(search, ZRARRAYOP_GET(offset, objSize, begin), data) > 0)
		return begin + 1;

	return begin;
}

ZRMUSTINLINE
static inline size_t ZRARRAYOP_BINSERT_POS_FIRST(void *offset, size_t objSize, size_t nbObj, void *search, zrfucmp fucmp, void *data)
{
	size_t begin = 0;
	size_t end = nbObj;

	while (begin < end)
	{
		size_t const mid = begin + (end - begin) / 2;
		int const cmp = fucmp(search, ZRARRAYOP_GET(offset, objSize, mid), data);

		if (cmp > 0)
			begin = mid + 1;
		else
			end = mid;
	}
	return begin;
}

ZRMUSTINLINE
static inline size_t ZRARRAYOP_BINSERT_POS_LAST(void *offset, size_t objSize, size_t nbObj, void *search, zrfucmp fucmp, void *data)
{
	size_t begin = 0;
	size_t end = nbObj;

	while (begin < end)
	{
		size_t const mid = begin + (end - begin) / 2;
		int const cmp = fucmp(search, ZRARRAYOP_GET(offset, objSize, mid), data);

		if (cmp < 0)
			end = mid;
		else
			begin = mid + 1;
	}
	return end;
}

ZRMUSTINLINE
static inline size_t ZRARRAYOP_BSEARCH_POS_FIRST(void *offset, size_t objSize, size_t nbObj, void *search, zrfucmp fucmp, void *data)
{
	size_t pos = ZRARRAYOP_BINSERT_POS_FIRST(offset, objSize, nbObj, search, fucmp, data);
	void *const item = ZRARRAYOP_GET(offset, objSize, pos);

	if (fucmp(search, item, data) == 0)
		return pos;

	return SIZE_MAX;
}

ZRMUSTINLINE
static inline void* ZRARRAYOP_BSEARCH_FIRST(void *offset, size_t objSize, size_t nbObj, void *search, zrfucmp fucmp, void *data)
{
	size_t pos = ZRARRAYOP_BINSERT_POS_FIRST(offset, objSize, nbObj, search, fucmp, data);
	void *const item = ZRARRAYOP_GET(offset, objSize, pos);

	if (fucmp(search, item, data) == 0)
		return item;

	return NULL;
}

ZRMUSTINLINE
static inline size_t ZRARRAYOP_BSEARCH_POS_LAST(void *offset, size_t objSize, size_t nbObj, void *search, zrfucmp fucmp, void *data)
{
	size_t pos = ZRARRAYOP_BINSERT_POS_LAST(offset, objSize, nbObj, search, fucmp, data);

	if (pos != 0)
		pos -= 1;

	void *const item = ZRARRAYOP_GET(offset, objSize, pos);

	if (fucmp(search, item, data) == 0)
		return pos;

	return SIZE_MAX;
}

ZRMUSTINLINE
static inline void* ZRARRAYOP_BSEARCH_LAST(void *offset, size_t objSize, size_t nbObj, void *search, zrfucmp fucmp, void *data)
{
	size_t pos = ZRARRAYOP_BINSERT_POS_LAST(offset, objSize, nbObj, search, fucmp, data);

	if (pos != 0)
		pos -= 1;

	void *const item = ZRARRAYOP_GET(offset, objSize, pos);

	if (fucmp(search, item, data) == 0)
		return item;

	return NULL;
}

ZRMUSTINLINE
static inline void ZRARRAYOP_WALK(void *offset, size_t objSize, size_t nbObj, void (*fconsume)(void *item))
{
	while (nbObj--)
	{
		fconsume(offset);
		offset = (char*)offset + objSize;
	}
}

ZRMUSTINLINE
static inline void ZRARRAYOP_MAP(
	void *restrict offset, size_t objSize, size_t nbObj,
	void (*fmap)(___ void *restrict item, void *restrict out),
	void *restrict dest, size_t dest_objSize, size_t dest_nbObj
	)
{
	while (nbObj-- && dest_nbObj--)
	{
		fmap(offset, dest);
		offset = (char*)offset + objSize;
		dest = (char*)dest + dest_objSize;
	}
}

// ============================================================================

void* _ ZRArrayOp_get(_ void ________ *offset, size_t objSize, size_t pos);
void _ ZRArrayOp_set(__ void *restrict offset, size_t objSize, size_t pos, void *restrict source);
void _ ZRArrayOp_swap(_ void ________ *offset, size_t objSize, size_t posa, size_t posb);

void ZRArrayOp_fill(____ void *restrict offset, size_t objSize, size_t nbObj, void *restrict object);
void ZRArrayOp_cpy(_____ void *restrict offset, size_t objSize, size_t nbObj, void *restrict source);
void ZRArrayOp_move(____ void ________ *offset, size_t objSize, size_t nbObj, void _________ *source);
void ZRArrayOp_deplace(_ void ________ *offset, size_t objSize, size_t nbObj, void _________ *source);

void ZRArrayOp_shift(___ void *offset, size_t objSize, size_t nbObj, size_t shift, bool toTheRight);
void ZRArrayOp_rotate(__ void *offset, size_t objSize, size_t nbObj, size_t rotate, bool toTheRight);
void ZRArrayOp_reverse(_ void *offset, size_t objSize, size_t nbObj);

size_t ZRArrayOp_search_pos(
	void *offset, size_t objSize, size_t nbObj, void *search,
	zrfucmp fucmp, void *data
	);
void* ZRArrayOp_search(
	void *offset, size_t objSize, size_t nbObj, void *search,
	zrfucmp fucmp, void *data
	);

size_t ZRArrayOp_bsearch_pos(
	void *offset, size_t objSize, size_t nbObj, void *search,
	zrfucmp fucmp, void *data
	);
size_t ZRArrayOp_bsearch_pos_first(
	void *offset, size_t objSize, size_t nbObj, void *search,
	zrfucmp fucmp, void *data
	);
size_t ZRArrayOp_bsearch_pos_last(
	void *offset, size_t objSize, size_t nbObj, void *search,
	zrfucmp fucmp, void *data
	);

void* ZRArrayOp_bsearch(
	void *offset, size_t objSize, size_t nbObj, void *search,
	zrfucmp fucmp, void *data
	);
void* ZRArrayOp_bsearch_first(
	void *offset, size_t objSize, size_t nbObj, void *search,
	zrfucmp fucmp, void *data
	);
void* ZRArrayOp_bsearch_last(
	void *offset, size_t objSize, size_t nbObj, void *search,
	zrfucmp fucmp, void *data
	);

size_t ZRArrayOp_binsert_pos(
	void *offset, size_t objSize, size_t nbObj, void *search,
	zrfucmp fucmp, void *data
	);
size_t ZRArrayOp_binsert_pos_first(
	void *offset, size_t objSize, size_t nbObj, void *search,
	zrfucmp fucmp, void *data
	);
size_t ZRArrayOp_binsert_pos_last(
	void *offset, size_t objSize, size_t nbObj, void *search,
	zrfucmp fucmp, void *data
	);

void ZRArrayOp_walk(void *offset, size_t objSize, size_t nbObj, void (*fconsume)(void *item));

void ZRArrayOp_map(
	void *restrict offset, size_t objSize, size_t nbObj,
	void (*fmap)(void *restrict item, void *restrict out),
	void *restrict dest, size_t dest_objSize, size_t dest_nbObj
	);

#endif

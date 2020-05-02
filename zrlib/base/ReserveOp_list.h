/**
 * @author zuri
 * @date samedi 23 novembre 2019, 22:32:22 (UTC+0100)
 */

#ifndef ZRRESERVE_OP_LIST_H
#define ZRRESERVE_OP_LIST_H

#include <zrlib/config.h>
#include <zrlib/base/MemoryOp.h>

#include <zrlib/syntax_pad.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// ============================================================================

typedef uint_fast32_t ZRReserveNextUnused;

// ============================================================================

static inline void ZRRESERVEOPLIST_RESERVENB(void *reserve, size_t objSize, size_t nbObj, size_t offsetReserveNextUnused, size_t pos, size_t nbAvailables);

// ============================================================================

ZRMUSTINLINE
inline static void ZRRESERVEOPLIST_INITARRAY(ZRReserveNextUnused *next, size_t nbObj)
{
	memset(next, (int)0, nbObj * sizeof(ZRReserveNextUnused));
}

ZRMUSTINLINE
static inline void ZRRESERVEOPLIST_INIT(void *reserve, size_t objSize, size_t nbObj, size_t offsetReserveNextUnused)
{
	char *current = (char*)reserve + offsetReserveNextUnused;
	char *const lastObj = current + (nbObj * objSize);

	while (current != lastObj)
	{
		*(ZRReserveNextUnused*)current = 0;
		current += objSize;
	}
}

ZRMUSTINLINE
static inline size_t ZRRESERVEOPLIST_RESERVEFIRSTAVAILABLES(void *reserve, size_t objSize, size_t nbObj, size_t offsetReserveNextUnused, size_t nbAvailables)
{
	char *current = (char*)reserve + offsetReserveNextUnused;
	char *const lastObj = current + (nbObj * objSize);
	size_t i = 0;
	size_t pos = 0;

	while (true)
	{
		ZRReserveNextUnused const nextUnused = *(ZRReserveNextUnused*)current;

		if (nextUnused != 0)
		{
			current += nextUnused * objSize;

			// The reserve is full
			if (current == lastObj)
				return SIZE_MAX;

			pos += i + nextUnused;
			i = 0;
			continue;
		}
		current += objSize;
		i++;

		// Founded
		if (i == nbAvailables)
		{
			ZRRESERVEOPLIST_RESERVENB(reserve, objSize, nbObj, offsetReserveNextUnused, pos, nbAvailables);
			return pos;
		}

		if (current == lastObj)
			return SIZE_MAX;
	}
}

ZRMUSTINLINE
static inline void ZRRESERVEOPLIST_RESERVENB(void *reserve, size_t objSize, size_t nbObj, size_t offsetReserveNextUnused, size_t pos, size_t nbAvailables)
{
	assert(pos < nbObj);
	assert(nbAvailables <= nbObj - pos);
	reserve = (char*)reserve + offsetReserveNextUnused;
	char *current = (char*)reserve + ((pos + nbAvailables) * objSize);
	char *const last = (char*)reserve + (nbObj * objSize);
	ZRReserveNextUnused next;

	if (current == last)
		next = 0;
	else
		next = *(ZRReserveNextUnused*)current;

	size_t i = 1;
	ZRReserveNextUnused *nextUnused;

	while (nbAvailables--)
	{
		current -= objSize;
		nextUnused = (ZRReserveNextUnused*)current;
		*nextUnused = next + i;
		i++;
	}

	if (current == reserve)
		return;

	current -= objSize;
	nextUnused = (ZRReserveNextUnused*)current;

	while (*nextUnused != 0)
	{
		*nextUnused = next + i;

		if (current == reserve)
			return;

		current -= objSize;
		nextUnused = (ZRReserveNextUnused*)current;
		i++;
	}
}

ZRMUSTINLINE
static inline bool ZRRESERVEOPLIST_AVAILABLES(void *reserve, size_t objSize, size_t offsetReserveNextUnused, size_t pos, size_t nbAvailables)
{
	char *current = (char*)reserve + (pos * objSize) + offsetReserveNextUnused;

	while (nbAvailables--)
	{
		if (*(ZRReserveNextUnused*)current != 0)
			return false;

		current += objSize;
	}
	return true;
}

ZRMUSTINLINE
static inline void ZRRESERVEOPLIST_RELEASENB(void *reserve, size_t objSize, size_t nbObj, size_t offsetReserveNextUnused, size_t pos, size_t nbToRelease)
{
	assert(pos < nbObj);
	assert(nbToRelease <= nbObj - pos);
	reserve = (char*)reserve + offsetReserveNextUnused;
	char *current = (char*)reserve + ((pos + nbToRelease) * objSize);

	while (nbToRelease--)
	{
		current -= objSize;
		*(ZRReserveNextUnused*)current = 0;
	}
	size_t i = 1;

	if (current == reserve)
		return;

	current -= objSize;
	ZRReserveNextUnused *nextUnused = (ZRReserveNextUnused*)current;

	while (*nextUnused != 0)
	{
		*nextUnused = i;

		if (current == reserve)
			return;

		current -= objSize;
		nextUnused = (ZRReserveNextUnused*)current;
		i++;
	}
}
// ============================================================================

void ZRReserveOpList_init(____ void *reserve, size_t objSize, size_t nbObj, size_t offsetReserveNextUnused);
void ZRReserveOpList_initArray(void *reserve, _______________ size_t nbObj);

size_t ZRReserveOpList_reserveFirstAvailables(void *reserve, size_t objSize, size_t nbObj, size_t offsetReserveNextUnused, ___________ size_t nbAvailables);
void _ ZRReserveOpList_reserveNb(____________ void *reserve, size_t objSize, size_t nbObj, size_t offsetReserveNextUnused, size_t pos, size_t nbAvailables);

bool ZRReserveOpList_availables(_ void *reserve, size_t objSize, _____________ size_t offsetReserveNextUnused, size_t pos, size_t nbAvailables);
void ZRReserveOpList_releaseNb(__ void *reserve, size_t objSize, size_t nbObj, size_t offsetReserveNextUnused, size_t pos, size_t nbToRelease);

#endif

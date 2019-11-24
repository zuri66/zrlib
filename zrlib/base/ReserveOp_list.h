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

static inline size_t ZRRESERVEOPLIST_RESERVEFIRSTAVAILABLES(void *reserve, size_t objSize, size_t nbObj, size_t offsetReserveNextUnused, size_t nbAvailables)
{
	char *base = reserve;
	char *current = base;
	char *const lastObj = base + (nbObj * objSize);
	size_t i = 0;
	size_t pos = 0;

	while (true)
	{
		ZRReserveNextUnused nextUnused = *(ZRReserveNextUnused*)(current + offsetReserveNextUnused);
		i++;

		// Founded
		if (i == nbAvailables)
		{
			ZRRESERVEOPLIST_RESERVENB(reserve, objSize, nbObj, offsetReserveNextUnused, pos, nbAvailables);
			return pos;
		}

		if (nextUnused != 0)
		{
			current += nextUnused * objSize;
			base = current;
			pos += i;
			i = 0;
		}
		else
			current += objSize;

		if (current >= lastObj)
			return SIZE_MAX;
	}
}

static inline void ZRRESERVEOPLIST_RESERVENB(void *reserve, size_t objSize, size_t nbObj, size_t offsetReserveNextUnused, size_t pos, size_t nbAvailables)
{
	assert(pos < nbObj);
	assert(nbAvailables <= nbObj - pos);
	char *current = (char*)reserve + ((pos + nbAvailables - 1) * objSize);
	char *last = (char*)reserve + (nbObj * objSize);
	ZRReserveNextUnused next;

	if (current + objSize == last)
		next = *(ZRReserveNextUnused*)((char*)reserve + offsetReserveNextUnused) + 1;
	else
		next = *(ZRReserveNextUnused*)(current + objSize + offsetReserveNextUnused);

	size_t i = 1;

	while (nbAvailables--)
	{
		*(ZRReserveNextUnused*)(current + offsetReserveNextUnused) = next + i;
		current -= objSize;
	}
}

static inline bool ZRRESERVEOPLIST_AVAILABLES(void *reserve, size_t objSize, size_t offsetReserveNextUnused, size_t pos, size_t nbAvailables)
{
	char *current = (char*)reserve + (pos * objSize);

	while (nbAvailables--)
	{
		if (*(ZRReserveNextUnused*)(current + offsetReserveNextUnused) != 0)
			return false;

		current += objSize;
	}
	return true;
}

static inline void ZRRESERVEOPLIST_RELEASENB(void *reserve, size_t objSize, size_t nbObj, size_t offsetReserveNextUnused, size_t pos, size_t nbToRelease)
{
	assert(pos < nbObj);
	assert(nbToRelease <= nbObj - pos);
	char *current = (char*)reserve + ((pos + nbToRelease - 1) * objSize);
	char *last = (char*)reserve + (nbObj * objSize);

	for (size_t i = nbToRelease; i; i--)
	{
		*(ZRReserveNextUnused*)(current + offsetReserveNextUnused) = 0;
		current -= objSize;
	}
	current += objSize;
	size_t i = 1;
	ZRReserveNextUnused *nextUnused = (ZRReserveNextUnused*)(current + offsetReserveNextUnused);

	while (current != reserve && *nextUnused != 0)
	{
		*nextUnused = i;
		current -= objSize;
		i++;
	}
}
// ============================================================================

size_t ZRReserveOpList_reserveFirstAvailables(void *reserve, size_t objSize, size_t nbObj, size_t offsetReserveNextUnused, size_t nbAvailables);
void ZRReserveOpList_reserveNb(void *reserve, size_t objSize, size_t nbObj, size_t offsetReserveNextUnused, size_t pos, size_t nbAvailables);

bool ZRReserveOpList_availables(void *reserve, size_t objSize, size_t offsetReserveNextUnused, size_t pos, size_t nbAvailables);
void ZRReserveOpList_releaseNb(void *reserve, size_t objSize, size_t nbObj, size_t offsetReserveNextUnused, size_t pos, size_t nbToRelease);

#endif

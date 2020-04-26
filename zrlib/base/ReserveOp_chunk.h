/**
 * @author zuri
 * @date samedi 25 avril 2020
 */

#ifndef ZRRESERVE_OP_CHUNK_H
#define ZRRESERVE_OP_CHUNK_H

#include <zrlib/config.h>
#include <zrlib/base/MemoryOp.h>

#include <zrlib/syntax_pad.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// ============================================================================

typedef struct ZRReserveMemoryChunkS ZRReserveMemoryChunk;

/**
 * Information for a free chunk of memory.
 */
struct ZRReserveMemoryChunkS
{
	size_t nbFree;
	size_t offset;
	ZRReserveMemoryChunk *nextChunk;
};

// ============================================================================

static inline void ZRRESERVEOPCHUNK_INIT(ZRReserveMemoryChunk *chunk)
{
	*chunk = (ZRReserveMemoryChunk ) { 0 };
}

static inline void ZRRESERVEOPCHUNK_INITARRAY(ZRReserveMemoryChunk *chunk, size_t nb)
{
	memset(chunk, 0, sizeof(ZRReserveMemoryChunk) * nb);
}

static inline bool ZRRESERVEOPCHUNK_INITED(ZRReserveMemoryChunk *chunk)
{
	return chunk->nbFree == 0 && chunk->offset == 0 && chunk->nextChunk == NULL;
}

static inline size_t ZRRESERVEOPCHUNK_RESERVEFIRSTAVAILABLES(ZRReserveMemoryChunk **firstChunk, size_t nbObj, void fchunkDone(ZRReserveMemoryChunk*))
{
	while (true)
	{
		ZRReserveMemoryChunk *chunk = *firstChunk;

		if (chunk == NULL)
			return SIZE_MAX; ////

		if (chunk->nbFree == nbObj)
		{
			*firstChunk = (*firstChunk)->nextChunk;

			if (fchunkDone)
				fchunkDone(chunk);

			return chunk->offset;
		}
		else if (chunk->nbFree > nbObj)
		{
			size_t offset = chunk->offset;
			chunk->offset += nbObj;
			chunk->nbFree -= nbObj;
			return offset;
		}
		firstChunk = &(*firstChunk)->nextChunk;
	}
}

static inline bool ZRRESERVEOPCHUNK_AVAILABLES(ZRReserveMemoryChunk *firstChunk, size_t nbObj)
{
	ZRReserveMemoryChunk *chunk = firstChunk;

	while (true)
	{
		if (firstChunk->nbFree >= nbObj)
			return true;

		firstChunk = firstChunk->nextChunk;
	}
	return false;
}

static inline void ZRRESERVEOPCHUNK_RELEASENB(ZRReserveMemoryChunk **firstChunk, ZRReserveMemoryChunk *newChunk, size_t nbObj, size_t pos, size_t nbToRelease, void fchunkDone(ZRReserveMemoryChunk*))
{
	ZRReserveMemoryChunk *leftChunk;
	ZRReserveMemoryChunk *rightChunk;

	if (*firstChunk == NULL)
	{
		*firstChunk = newChunk;
		newChunk->nbFree = nbObj;
		newChunk->offset = 0;
		newChunk->nextChunk = NULL;
		return;
	}

	// We must insert it on the first place
	if ((*firstChunk)->offset > pos)
	{
		rightChunk = *firstChunk;
		leftChunk = NULL;
	}
	else
	{
		leftChunk = *firstChunk;

		// Find the place to be
		while (true)
		{
			rightChunk = leftChunk->nextChunk;

			if (rightChunk == NULL || rightChunk->offset > pos)
				break;

			leftChunk = rightChunk;
		}
	}
	ZRReserveMemoryChunk *current;

	if (leftChunk == NULL)
	{
		current = newChunk;
		current->offset = pos;
		current->nbFree = nbToRelease;
		current->nextChunk = rightChunk;
		*firstChunk = current;
	}
	// The previous free chunk end just before pos
	else if (leftChunk->offset + leftChunk->nbFree == pos)
	{
		current = leftChunk;
		current->nbFree += nbToRelease;
	}
	else
	{
		current = newChunk;
		current->offset = pos;
		current->nbFree = nbToRelease;
		current->nextChunk = leftChunk->nextChunk;
		leftChunk->nextChunk = current;
	}

	if (rightChunk == NULL)
		;
	// The current chunk end just before the next
	else
	{
		assert(current->offset + current->nbFree <= rightChunk->offset);

		if (current->offset + current->nbFree == rightChunk->offset)
		{
			current->nbFree += rightChunk->nbFree;
			current->nextChunk = rightChunk->nextChunk;

			if (fchunkDone != NULL)
				fchunkDone(rightChunk);
		}
	}
}

// ============================================================================

void ZRReserveOpChunk_init(____ ZRReserveMemoryChunk *chunk);
void ZRReserveOpChunk_initArray(ZRReserveMemoryChunk *chunk, size_t nb);

size_t ZRReserveOpChunk_reserveFirstAvailables(ZRReserveMemoryChunk **firstChunk, size_t nbObj, void fchunkDone(ZRReserveMemoryChunk*));

bool ZRReserveOpChunk_availables(ZRReserveMemoryChunk *firstChunk, size_t nbObj);
void ZRReserveOpChunk_releaseNb(ZRReserveMemoryChunk **firstChunk, ZRReserveMemoryChunk *newChunk, size_t nbObj, size_t pos, size_t nbToRelease, void fchunkDone(ZRReserveMemoryChunk*));

#endif

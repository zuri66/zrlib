/**
 * @author zuri
 * @date samedi 28 juin 2014, 13:27:45 (UTC+0200)
 */

#ifndef ZRMEMORY_ARRAY_OP_H
#define ZRMEMORY_ARRAY_OP_H

#include <stdlib.h>
#include <stdbool.h>

// ============================================================================

#define ZRARRAYOP_GET(offset, objSize, pos) \
	((char*)(offset) + (pos) * (objSize))

#define ZRARRAYOP_SET(offset, objSize, pos, source) \
	memcpy(ZRARRAYOP_GET(offset, objSize, pos), (source), (objSize))

#define ZRARRAYOP_SWAP(offset, objSize, posa, posb) \
	ZRMemoryOp_swap(ZRARRAYOP_GET(offset, objSize, posa), ZRARRAYOP_GET(offset, objSize, posb), (objSize))

#define ZRARRAYOP_SHIFT(offset, objSize, nbObj, shift, toTheRight) \
	ZRMemoryOp_shift((offset), ZRARRAYOP_GET(offset, objSize, nbObj) - 1 , (shift)  * (objSize), (toTheRight))

#define ZRARRAYOP_ROTATE(offset, objSize, nbObj, rotate, toTheRight) \
	ZRMemoryOp_rotate((offset), ZRARRAYOP_GET(offset, objSize, nbObj) - 1, (rotate) * (objSize), (toTheRight))

#include "MemoryOp.h"

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

// ============================================================================

void* ZRArrayOp_get (void *offset, size_t objSize, size_t pos);
void  ZRArrayOp_set (void *restrict offset, size_t objSize, size_t pos, void *restrict source);
void  ZRArrayOp_swap(void *offset, size_t objSize, size_t posa, size_t posb);

void ZRArrayOp_shift  (void *offset, size_t objSize, size_t nbObj, size_t shift, bool toTheRight);
void ZRArrayOp_rotate (void *offset, size_t objSize, size_t nbObj, size_t rotate, bool toTheRight);
void ZRArrayOp_reverse(void *offset, size_t objSize, size_t nbObj);

#endif

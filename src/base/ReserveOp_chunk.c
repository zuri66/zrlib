/**
 * @author zuri
 * @date samedi 25 avril 2020
 */

#include <zrlib/base/ReserveOp_chunk.h>

// ============================================================================

void ZRReserveOpChunk_init(ZRReserveMemoryChunk *chunk)
{
	ZRRESERVEOPCHUNK_INIT(chunk);
}

void ZRReserveOpChunk_initArray(ZRReserveMemoryChunk *chunk, size_t nb)
{
	ZRRESERVEOPCHUNK_INITARRAY(chunk, nb);
}

size_t ZRReserveOpChunk_reserveFirstAvailables(ZRReserveMemoryChunk **firstChunk, size_t nbObj, void fchunkDone(ZRReserveMemoryChunk*))
{
	return ZRRESERVEOPCHUNK_RESERVEFIRSTAVAILABLES(firstChunk, nbObj, fchunkDone);
}

bool ZRReserveOpChunk_availables(ZRReserveMemoryChunk *firstChunk, size_t nbObj)
{
	return ZRRESERVEOPCHUNK_AVAILABLES(firstChunk, nbObj);
}

void ZRReserveOpChunk_releaseNb(ZRReserveMemoryChunk **firstChunk, ZRReserveMemoryChunk *newChunk, size_t nbObj, size_t pos, size_t nbToRelease, void fchunkDone(ZRReserveMemoryChunk*))
{
	ZRRESERVEOPCHUNK_RELEASENB(firstChunk, newChunk, nbObj, pos, nbToRelease, fchunkDone);
}

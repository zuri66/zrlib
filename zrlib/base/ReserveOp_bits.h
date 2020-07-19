/**
 * @author zuri
 * @date samedi 23 novembre 2019, 20:53:19 (UTC+0100)
 */

#ifndef ZRMEMORY_RESERVE_OP_BITS_H
#define ZRMEMORY_RESERVE_OP_BITS_H

#include <zrlib/base/Bits/Bits.h>

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define ZRRESERVEOPBITS_BLOCKISEMPTY 1
#define ZRRESERVEOPBITS_BLOCKISLOCKD 0
#define ZRRESERVEOPBITS_FULLEMPTY ZRBITS_MASK_FULL
#define ZRRESERVEOPBITS_FULLLOCKD 0

// ============================================================================

static inline void ZRRESERVEOPBITS_RESERVENB(ZRBits *bits, size_t pos, size_t nbAvailables);

// ============================================================================

ZRMUSTINLINE
static inline size_t ZRRESERVEOPBITS_RESERVEFIRSTAVAILABLES(ZRBits *bits, size_t nbZRBits, size_t nbAvailables)
{
	size_t pos;
	ZRBits *retBits;
	ZRBits_searchFixedPattern(bits, 0, nbZRBits, nbAvailables, &retBits, &pos);

	if (retBits == NULL)
		return SIZE_MAX;

	ZRRESERVEOPBITS_RESERVENB(retBits, pos, nbAvailables);
	pos += (retBits - bits) * ZRBITS_NBOF;
	return pos;
}

ZRMUSTINLINE
static inline void ZRRESERVEOPBITS_RESERVENB(ZRBits *bits, size_t pos, size_t nbAvailables)
{
	size_t const nbZRBits = ((pos + nbAvailables) / ZRBITS_NBOF) + ((pos + nbAvailables) % ZRBITS_NBOF ? 1 : 0);
	ZRBits set[nbZRBits];

	memset(set, (int)ZRRESERVEOPBITS_FULLLOCKD, nbZRBits * sizeof(ZRBits));
	ZRBits_copy(set, 0, nbAvailables, bits, pos);
}

ZRMUSTINLINE
static inline bool ZRRESERVEOPBITS_AVAILABLES(ZRBits *bits, size_t pos, size_t nbAvailables)
{
	size_t const nbAddPos = pos + nbAvailables;
	size_t const rest = nbAddPos % ZRBITS_NBOF;
	size_t const nbZRBits = (nbAddPos / ZRBITS_NBOF) + (rest ? 1 : 0);
	ZRBits set[nbZRBits];

	if (nbZRBits == 1)
	{
		ZRBits_fill(set, pos, nbAvailables);
	}
	else
	{
		set[0] = ZRBits_getRMask(ZRBITS_NBOF - pos);
		set[nbZRBits - 1] = ZRBits_getLMask(rest);

		if (nbZRBits > 2)
			memset(&set[1], (int)ZRRESERVEOPBITS_FULLEMPTY, (nbZRBits - 2) * sizeof(ZRBits));
	}
	return ZRBits_cmp(bits, set, pos, nbAvailables) == 0;
}

ZRMUSTINLINE
static inline void ZRRESERVEOPBITS_RELEASENB(ZRBits *bits, size_t pos, size_t nbToRelease)
{
	size_t const nbZRBits = ((pos + nbToRelease) / ZRBITS_NBOF) + ((pos + nbToRelease) % ZRBITS_NBOF ? 1 : 0);
	ZRBits set[nbZRBits];
	memset(set, (int)ZRRESERVEOPBITS_FULLEMPTY, nbZRBits * sizeof(ZRBits));
	ZRBits_copy(set, 0, nbToRelease, bits, pos);
}

// ============================================================================

size_t ZRReserveOpBits_reserveFirstAvailables(ZRBits *bits, size_t nbZRBits, size_t nb);

bool ZRReserveOpBits_availables(_ ZRBits *bits, size_t pos, size_t nb);
void ZRReserveOpBits_reserveNb(__ ZRBits *bits, size_t pos, size_t nb);
void ZRReserveOpBits_releaseNb(__ ZRBits *bits, size_t pos, size_t nb);

#endif

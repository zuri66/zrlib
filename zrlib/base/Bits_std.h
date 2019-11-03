/**
 * @author zuri
 * @date vendredi 1 novembre 2019, 15:56:25 (UTC+0100)
 */

#ifndef ZRBITS_H
#error "This file: " __FILE__ "cannot be used outside of Bits.h"
#endif

#ifndef BITS_STD_H
#define BITS_STD_H

#include <assert.h>

// ============================================================================

static inline ZRBits ZRBITS_GETLMASK_STD(size_t nbBits)
{
	assert(nbBits <= ZRBITS_NBOF);

	if (nbBits == 0)
		return (ZRBits)0;
	if (nbBits == ZRBITS_NBOF)
		return ZRBITS_MASK_FULL;

	ZRBits ret = ZRBITS_MASK_1L;

	while (--nbBits)
	{
		ret >>= 1;
		ret |= ZRBITS_MASK_1L;
	}
	return ret;
}

static inline ZRBits ZRBITS_GETRMASK_STD(size_t nbBits)
{
	assert(nbBits <= ZRBITS_NBOF);

	if (nbBits == 0)
		return (ZRBits)0;
	if (nbBits == ZRBITS_NBOF)
		return ZRBITS_MASK_FULL;

	ZRBits ret = ZRBITS_MASK_1R;

	while (--nbBits)
	{
		ret <<= 1;
		ret |= ZRBITS_MASK_1R;
	}
	return ret;
}

static inline ZRBits ZRBITS_SELECTBITS(ZRBits bits, size_t pos, size_t nbBits)
{
	assert(pos + nbBits <= ZRBITS_NBOF);
	ZRBits const mask = ZRBITS_GETLMASK(nbBits) >> pos;
	return (bits & mask) << pos;
}

static inline void ZRBITS_SETBIT_STD(ZRBits *bits, size_t pos, bool bit)
{
	ADJUST_POS(bits, pos);
	const ZRBits mask = (ZRBITS_MASK_1L >> pos);

	if (bit)
		*bits |= mask;
	else
		*bits &= ~mask;
}

static inline void ZRBITS_SETBITSFROMTHERIGHT_STD(ZRBits *bits, size_t pos, size_t nbBits, ZRBits source)
{
	ADJUST_POS(bits, pos);
	size_t const nbAddPos = nbBits + pos;
	ZRBits const lmask = ZRBITS_GETLMASK(nbBits);

	// We align to the left
	source <<= ZRBITS_NBOF - nbBits;

	source &= ZRBITS_GETLMASK(nbBits);
	*bits = (*bits & ~(lmask >> pos)) | source >> pos;

	if (nbAddPos > ZRBITS_NBOF)
	{
		size_t const finalBits = (pos + nbBits) % ZRBITS_NBOF;
		bits++;
		*bits = (*bits & (lmask << (ZRBITS_NBOF - finalBits))) | source << (nbBits - finalBits);
	}
}

static inline void ZRBITS_SETBITSFROMTHELEFT_STD(ZRBits *bits, size_t pos, size_t nbBits, ZRBits source)
{
	ADJUST_POS(bits, pos);
	size_t const nbAddPos = nbBits + pos;
	ZRBits const lmask = ZRBITS_GETLMASK(nbBits);

	source &= ZRBITS_GETLMASK(nbBits);
	*bits = (*bits & ~(lmask >> pos)) | source >> pos;

	if (nbAddPos > ZRBITS_NBOF)
	{
		size_t const finalBits = (pos + nbBits) % ZRBITS_NBOF;
		bits++;
		*bits = (*bits & (lmask << (ZRBITS_NBOF - finalBits))) | source << (nbBits - finalBits);
	}
}

static inline bool ZRBITS_GETBIT_STD(ZRBits const *bits, size_t pos)
{
	ADJUST_POS(bits, pos);
	return (bool)(*bits & (ZRBITS_MASK_1L >> pos));
}

// ============================================================================
// COPY
// ============================================================================

static inline void std_ZRBits_copyBlocks(ZRBits const * restrict bits, size_t nbZRBits, size_t finalBits, ZRBits * restrict out)
{
	if (nbZRBits)
	{
		memcpy(out, bits, nbZRBits * sizeof(ZRBits));
		out += nbZRBits;
		bits += nbZRBits;
	}

	if (!finalBits)
	return;

	*out = *bits & ZRBITS_GETLMASK(finalBits);
}

static inline void std_ZRBits_copy_posEQOutPos(ZRBits const * restrict bits, size_t pos, size_t nbBits, ZRBits * restrict out)
{
	size_t const nbAddPos = nbBits + pos;

	if (pos == 0)
	{
		size_t const nbZRBits = nbBits / ZRBITS_NBOF;
		size_t const finalBits = nbBits % ZRBITS_NBOF;
		std_ZRBits_copyBlocks(bits, nbZRBits, finalBits, out);
	}
	// The selection stay inside a ZRBits object
	else if (nbAddPos <= ZRBITS_NBOF)
	{
		ZRBits const mask = ZRBITS_GETLMASK(nbBits) >> pos;
		*out = *bits & mask;
	}
	else
	{
		size_t const nbZRBits = (nbBits - (ZRBITS_NBOF - pos)) / ZRBITS_NBOF;
		size_t const finalBits = (nbBits + pos) % ZRBITS_NBOF;

		*out = *bits & ZRBITS_GETRMASK(ZRBITS_NBOF - pos);
		out++, bits++;
		std_ZRBits_copyBlocks(bits, nbZRBits, finalBits, out);
	}
}

static inline void std_ZRBits_copy_posGTOutPos(ZRBits const * restrict bits, size_t pos, size_t nbBits, ZRBits * restrict out, size_t outPos)
{
	size_t const nbAddPos = nbBits + pos;
	size_t const nbAddOutPos = nbBits + outPos;
	size_t const posSubOutPos = pos - outPos;

	// The selection stay inside a ZRBits object
	if (nbAddPos <= ZRBITS_NBOF)
	{
		*out = (*bits & (ZRBITS_GETLMASK(nbBits) >> pos)) << posSubOutPos;
	}
	// The result is store in only one ZRBits
	else if (nbAddOutPos <= ZRBITS_NBOF)
	{
		size_t const firstPart1Size = ZRBITS_NBOF - pos;
		size_t const firstPart2Size = nbBits - firstPart1Size;
		ZRBits const firstPart1Mask = ZRBITS_GETRMASK(firstPart1Size);
		ZRBits const firstPart2Mask = ZRBITS_GETLMASK(firstPart2Size);

		*out _= ((*bits & firstPart1Mask) << posSubOutPos) | ((*(bits + 1) & firstPart2Mask) >> (firstPart1Size + outPos));
	}
	else if (outPos == 0)
	{
		size_t const firstPart1Size = ZRBITS_NBOF - pos;
		size_t nbZRBits = nbBits / ZRBITS_NBOF;
		size_t const finalBits = nbBits % ZRBITS_NBOF;

		size_t const part1Size = ZRBITS_NBOF - pos;
		ZRBits const part1Mask = ZRBITS_GETRMASK(part1Size);
		ZRBits const part2Mask = ~part1Mask;

		while (nbZRBits--)
		{
			*out = (*bits & part1Mask) << pos | (*(bits + 1) & part2Mask) >> part1Size;
			bits++, out++;
		}

		if (!finalBits)
		return;

		std_ZRBits_copy_posGTOutPos(bits,pos,finalBits,out,0);
	}
	// The most general case
	else
	{
		size_t const firstPart1Size = ZRBITS_NBOF - pos;
		size_t const firstPart2Size = posSubOutPos;
		size_t const firstPartTotalSize = firstPart1Size + firstPart2Size;
		ZRBits const firstPart1Mask = ZRBITS_GETRMASK(firstPart1Size);
		ZRBits const firstPart2Mask = ZRBITS_GETLMASK(firstPart2Size);

		*out = ((*bits & firstPart1Mask) << posSubOutPos) | ((*(bits + 1) & firstPart2Mask) >> (firstPart1Size + outPos));
		bits++, out++;
		std_ZRBits_copy_posGTOutPos(bits, posSubOutPos, nbBits - firstPartTotalSize, out, 0);
	}
}

static inline void std_ZRBits_copy_posLTOutPos(ZRBits const * restrict bits, size_t pos, size_t nbBits, ZRBits * restrict out, size_t outPos)
{
	size_t const nbAddOutPos = nbBits + outPos;
	size_t outPosSubPos = outPos - pos;

	// The result is stored in only one ZRBits
	if (nbAddOutPos <= ZRBITS_NBOF)
	*out = (*bits & (ZRBITS_GETLMASK(nbBits) >> pos)) >> outPosSubPos;
	else
	{
		size_t const firstPart1Size = ZRBITS_NBOF - outPos;
		ZRBits const firstPart1Mask = ZRBITS_GETLMASK(firstPart1Size);

		*out = (*bits & (firstPart1Mask >> pos)) >> outPosSubPos;
		std_ZRBits_copy_posGTOutPos(bits, pos + firstPart1Size, nbBits - firstPart1Size, out + 1, 0);
	}
}

static inline void ZRBITS_COPY_STD(ZRBits const * restrict bits, size_t pos, size_t nbBits, ZRBits * restrict out, size_t outPos)
{
	ADJUST_POS(bits, pos);
	ADJUST_POS(out, outPos);

	if (pos == outPos)
	std_ZRBits_copy_posEQOutPos(bits, pos, nbBits, out);
	else if (pos > outPos)
	std_ZRBits_copy_posGTOutPos(bits, pos, nbBits, out, outPos);
	else
	std_ZRBits_copy_posLTOutPos(bits, pos, nbBits, out, outPos);
}

static inline void ZRBITS_INARRAYRSHIFT_STD(ZRBits *bits, size_t nbZRBits, size_t shift)
{
	assert(nbZRBits > 0);

	if (shift % CHAR_BIT == 0)
	{
		shift /= CHAR_BIT;
		ZRARRAYOP_SHIFT(bits, sizeof(char), nbZRBits * sizeof(*bits) / sizeof(char), shift, true);
		ZRARRAYOP_FILL(bits, sizeof(char), shift, &(ZRBits ) { 0 });
		return;
	}
	size_t const rightPartBits = ZRBITS_NBOF - shift;
	ZRBits const rightPartMask = ZRBITS_GETRMASK(rightPartBits);
	ZRBits const rightPartComplementMask = ~rightPartMask;

	if (nbZRBits > 1)
	{
		bits += nbZRBits - 1;

		while (--nbZRBits)
		{
			*bits = ((*bits & rightPartMask) >> shift) | ((*(bits - 1) & rightPartComplementMask) << rightPartBits);
			bits--;
		}
	}
	*bits >>= shift;
}

static inline void ZRBITS_INARRAYLSHIFT_STD(ZRBits *bits, size_t nbZRBits, size_t shift)
{
	assert(nbZRBits > 0);

	if (shift % CHAR_BIT == 0)
	{
		shift /= CHAR_BIT;
		char *bits0fill = (char*)bits + nbZRBits * (sizeof(*bits) / sizeof(char)) - shift;

		ZRARRAYOP_SHIFT(bits, sizeof(char), nbZRBits * sizeof(*bits) / sizeof(char), shift, false);
		ZRARRAYOP_FILL(bits0fill, sizeof(char), shift, &(ZRBits ) { 0 });
		return;
	}
	size_t const leftPartBits = ZRBITS_NBOF - shift;
	ZRBits const leftPartMask = ZRBITS_GETRMASK(leftPartBits);
	ZRBits const leftPartComplementMask = ~leftPartMask;

	if (nbZRBits > 1)
	{
		while (--nbZRBits)
		{
			*bits = ((*bits & leftPartMask) << shift) | ((*(bits + 1) & leftPartComplementMask) >> leftPartBits);
			bits++;
		}
	}
	*bits <<= shift;
}

static inline void ZRBITS_SEARCHFIXEDPATTERN_STD(ZRBits *bits, size_t pos, size_t nbZRBits, size_t nbBits, ZRBits **dest, size_t *outPos)
{
	ADJUST_POS(bits, pos);
	size_t const mask_nbZRBits = nbBits / ZRBITS_NBOF;
	size_t const mask_rest = (nbBits % ZRBITS_NBOF);
	size_t const offsetMask = ZRBITS_NBOF - mask_rest;
	size_t const maskSize = mask_nbZRBits + (bool)mask_rest;
	ZRBits mask[maskSize];
	ZRBits buf[maskSize];

	mask[0] = ZRBITS_GETRMASK(mask_rest);

	if (maskSize > 0)
		memset(&mask[1], 0, mask_nbZRBits * sizeof(ZRBits));

	memset(buf, 0, maskSize * sizeof(ZRBits));
	int ipos = pos;

	for (;;)
	{
		ZRBITS_COPY_STD(bits, ipos, nbBits, buf, offsetMask);

		if (memcmp(buf, mask, maskSize * sizeof(ZRBits)) == 0)
		{
			*dest = bits;
			*outPos = ipos;
			return;
		}
		ipos++;

		if (ipos == sizeof(ZRBits) * CHAR_BIT)
		{
			if (nbZRBits == 0)
				break;

			ipos = 0;
			bits++;
		}
	}
	*dest = NULL;
	*outPos = 0;
}

// ============================================================================

ZRBits ZRBits_getLMask_std(size_t nbBits);
ZRBits ZRBits_getRMask_std(size_t nbBits);

void ZRBits_setBit_std(______________ ZRBits *bits, size_t pos, bool bit);
void ZRBits_setBitsFromTheRight_std(_ ZRBits *bits, size_t pos, size_t nbBits, ZRBits source);
void ZRBits_setBitsFromTheLeft_std(__ ZRBits *bits, size_t pos, size_t nbBits, ZRBits source);

bool ZRBits_getBit_std(ZRBits const * ________ bits, size_t pos);
void ZRBits_copy_std(_ ZRBits const * restrict bits, size_t pos, size_t nbBits, ZRBits * restrict out, size_t outPos);

void ZRBits_inArrayLShift_std(ZRBits *bits, size_t nbZRBits, size_t shift);
void ZRBits_inArrayRShift_std(ZRBits *bits, size_t nbZRBits, size_t shift);

void ZRBits_searchFixedPattern_std(ZRBits *bits, size_t pos, size_t nbZRBits, size_t nbBits, ZRBits **dest, size_t *outPos);

#endif

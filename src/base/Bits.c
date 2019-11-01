#include <assert.h>
#include <zrlib/base/ArrayOp.h>
#include <zrlib/base/Bits.h>

// ============================================================================

/**
 * Adapt the bits offset (and the position) if the position refer to a bit in another offset.
 */
#define ADJUST_POS(bits, pos) \
if (pos >= ZRBITS_NBOF) \
{ \
	bits += pos / ZRBITS_NBOF; \
	pos %= ZRBITS_NBOF; \
}

// ============================================================================
// GETMASK
// ============================================================================

ZRBits ZRBits_getLMask(size_t nbBits)
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

ZRBits ZRBits_getRMask(size_t nbBits)
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

ZRBits ZRBits_getMask(size_t nb1, bool toTheRight)
{
	if (toTheRight)
		return ZRBits_getRMask(nb1);

	return ZRBits_getLMask(nb1);
}

// ============================================================================

void ZRBits_cpack(ZRBits * bits, size_t nbBits, char * source, size_t sourceSize)
{
	size_t i = 0;

	while (sourceSize--)
	{
		ZRBits_setBitsFromTheRight(bits, i, nbBits, *source);
		source++;
		i += nbBits;
	}
}

/**
 * @param ZRBits *pack The destination of the packing
 * @param size_t nbBits Number of bit to copy for each ZRBits
 * @param unsigned int *source which contains the bits to get
 * @param size_t sourceSize The size of the source.
 */
void ZRBits_pack(ZRBits * restrict bits, size_t nbBits, ZRBits * restrict source, size_t sourceSize)
{
	size_t i = 0;

	while (sourceSize--)
	{
		ZRBits_setBitsFromTheRight(bits, i, nbBits, *source);
		source++;
		i += nbBits;
	}
}

// ============================================================================

void ZRBits_setBit(ZRBits *bits, size_t pos, bool bit)
{
	ADJUST_POS(bits, pos);
	const ZRBits mask = (ZRBITS_MASK_1L >> pos);

	if (bit)
		*bits |= mask;
	else
		*bits &= ~mask;
}

void ZRBits_setBitsFromTheRight(ZRBits *bits, size_t pos, size_t nbBits, ZRBits source)
{
	ZRBits_setBits(bits, pos, nbBits, source, true);
}

void ZRBits_setBitsFromTheLeft(ZRBits *bits, size_t pos, size_t nbBits, ZRBits source)
{
	ZRBits_setBits(bits, pos, nbBits, source, false);
}

void ZRBits_setBits(ZRBits *bits, size_t pos, size_t nbBits, ZRBits source, bool fromTheRight)
{
	ADJUST_POS(bits, pos);
	size_t const nbAddPos = nbBits + pos;
	ZRBits const lmask = ZRBits_getLMask(nbBits);

	// We align to the left
	if (fromTheRight)
		source <<= ZRBITS_NBOF - nbBits;

	source &= ZRBits_getLMask(nbBits);
	*bits = (*bits & ~(lmask >> pos)) | source >> pos;

	if (nbAddPos > ZRBITS_NBOF)
	{
		size_t const finalBits = (pos + nbBits) % ZRBITS_NBOF;
		bits++;
		*bits = (*bits & (lmask << (ZRBITS_NBOF - finalBits))) | source << (nbBits - finalBits);
	}
}

bool ZRBits_getBit(ZRBits const *bits, size_t pos)
{
	ADJUST_POS(bits, pos);
	return (bool)(*bits & (ZRBITS_MASK_1L >> pos));
}

void ZRBits_getBits(ZRBits const * restrict bits, size_t pos, size_t nbBits, ZRBits * restrict out)
{
	ZRBits_copy(bits, pos, nbBits, out, 0);
}

// ============================================================================
// COPY
// ============================================================================

static inline void ZRBits_copyBlocks(ZRBits const * restrict bits, size_t nbZRBits, size_t finalBits, ZRBits * restrict out)
{
	if (nbZRBits)
	{
		memcpy(out, bits, nbZRBits * sizeof(ZRBits));
		out += nbZRBits;
		bits += nbZRBits;
	}

	if (!finalBits)
		return;

	*out = *bits & ZRBits_getLMask(finalBits);
}

static inline void ZRBits_copy_posEQOutPos(ZRBits const * restrict bits, size_t pos, size_t nbBits, ZRBits * restrict out)
{
	size_t const nbAddPos = nbBits + pos;

	if (pos == 0)
	{
		size_t const nbZRBits = nbBits / ZRBITS_NBOF;
		size_t const finalBits = nbBits % ZRBITS_NBOF;
		ZRBits_copyBlocks(bits, nbZRBits, finalBits, out);
	}
	// The selection stay inside a ZRBits object
	else if (nbAddPos <= ZRBITS_NBOF)
	{
		ZRBits const mask = ZRBits_getLMask(nbBits) >> pos;
		*out = *bits & mask;
	}
	else
	{
		size_t const nbZRBits = (nbBits - (ZRBITS_NBOF - pos)) / ZRBITS_NBOF;
		size_t const finalBits = (nbBits + pos) % ZRBITS_NBOF;

		*out = *bits & ZRBits_getRMask(ZRBITS_NBOF - pos);
		out++, bits++;
		ZRBits_copyBlocks(bits, nbZRBits, finalBits, out);
	}
}

static inline void ZRBits_copy_posGTOutPos(ZRBits const * restrict bits, size_t pos, size_t nbBits, ZRBits * restrict out, size_t outPos)
{
	size_t const nbAddPos = nbBits + pos;
	size_t const nbAddOutPos = nbBits + outPos;
	size_t const posSubOutPos = pos - outPos;

	// The selection stay inside a ZRBits object
	if (nbAddPos <= ZRBITS_NBOF)
	{
		*out = (*bits & (ZRBits_getLMask(nbBits) >> pos)) << posSubOutPos;
	}
	// The result is store in only one ZRBits
	else if (nbAddOutPos <= ZRBITS_NBOF)
	{
		size_t const firstPart1Size = ZRBITS_NBOF - pos;
		size_t const firstPart2Size = nbBits - firstPart1Size;
		ZRBits const firstPart1Mask = ZRBits_getRMask(firstPart1Size);
		ZRBits const firstPart2Mask = ZRBits_getLMask(firstPart2Size);

		*out = ((*bits & firstPart1Mask) << posSubOutPos) | ((*(bits + 1) & firstPart2Mask) >> (firstPart1Size + outPos));
	}
	else if (outPos == 0)
	{
		size_t const firstPart1Size = ZRBITS_NBOF - pos;
		size_t nbZRBits = nbBits / ZRBITS_NBOF;
		size_t const finalBits = nbBits % ZRBITS_NBOF;

		size_t const part1Size = ZRBITS_NBOF - pos;
		ZRBits const part1Mask = ZRBits_getRMask(part1Size);
		ZRBits const part2Mask = ~part1Mask;

		while (nbZRBits--)
		{
			*out = (*bits & part1Mask) << pos | (*(bits + 1) & part2Mask) >> part1Size;
			bits++, out++;
		}

		if (!finalBits)
			return;

		ZRBits_copy_posGTOutPos(bits,pos,finalBits,out,0);
	}
	// The most general case
	else
	{
		size_t const firstPart1Size = ZRBITS_NBOF - pos;
		size_t const firstPart2Size = posSubOutPos;
		size_t const firstPartTotalSize = firstPart1Size + firstPart2Size;
		ZRBits const firstPart1Mask = ZRBits_getRMask(firstPart1Size);
		ZRBits const firstPart2Mask = ZRBits_getLMask(firstPart2Size);

		*out = ((*bits & firstPart1Mask) << posSubOutPos) | ((*(bits + 1) & firstPart2Mask) >> (firstPart1Size + outPos));
		bits++, out++;
		ZRBits_copy_posGTOutPos(bits, posSubOutPos, nbBits - firstPartTotalSize, out, 0);
	}
}

static inline void ZRBits_copy_posLTOutPos(ZRBits const * restrict bits, size_t pos, size_t nbBits, ZRBits * restrict out, size_t outPos)
{
	size_t const nbAddOutPos = nbBits + outPos;
	size_t outPosSubPos = outPos - pos;

	// The result is stored in only one ZRBits
	if (nbAddOutPos <= ZRBITS_NBOF)
		*out = (*bits & (ZRBits_getLMask(nbBits) >> pos)) >> outPosSubPos;
	else
	{
		size_t const firstPart1Size = ZRBITS_NBOF - outPos;
		ZRBits const firstPart1Mask = ZRBits_getLMask(firstPart1Size);

		*out = (*bits & (firstPart1Mask >> pos)) >> outPosSubPos;
		ZRBits_copy_posGTOutPos(bits, pos + firstPart1Size, nbBits - firstPart1Size, out + 1, 0);
	}
}

void ZRBits_copy(ZRBits const * restrict bits, size_t pos, size_t nbBits, ZRBits * restrict out, size_t outPos)
{
	ADJUST_POS(bits, pos);
	ADJUST_POS(out, outPos);

	if (pos == outPos)
		ZRBits_copy_posEQOutPos(bits, pos, nbBits, out);
	else if (pos > outPos)
		ZRBits_copy_posGTOutPos(bits, pos, nbBits, out, outPos);
	else
		ZRBits_copy_posLTOutPos(bits, pos, nbBits, out, outPos);
}

// ============================================================================
// SHIFT
// ============================================================================

static inline void ZRBits_inArrayLShift_(ZRBits *bits, size_t nbZRBits, size_t shift)
{
	size_t const leftPartBits = ZRBITS_NBOF - shift;
	ZRBits const leftPartMask = ZRBits_getRMask(leftPartBits);
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

static inline void ZRBits_inArrayRShift_(ZRBits *bits, size_t nbZRBits, size_t shift)
{
	size_t const rightPartBits = ZRBITS_NBOF - shift;
	ZRBits const rightPartMask = ZRBits_getRMask(rightPartBits);
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

void ZRBits_inArrayRShift(ZRBits *bits, size_t nbZRBits, size_t shift)
{
	ZRBits_inArrayRShift_(bits, nbZRBits, shift);
}

void ZRBits_inArrayLShift(ZRBits *bits, size_t nbZRBits, size_t shift)
{
	ZRBits_inArrayLShift_(bits, nbZRBits, shift);
}

void ZRBits_inArrayShift(ZRBits *bits, size_t nbZRBits, size_t shift, size_t toTheRight)
{
	assert(nbZRBits > 0);

	if (shift % CHAR_BIT == 0)
	{
		shift /= CHAR_BIT;
		char *bits0fill;

		if (toTheRight)
			bits0fill = (char*)bits;
		else
			bits0fill = (char*)bits + nbZRBits * (sizeof(*bits) / sizeof(char)) - shift;

		ZRBits zeros = ZRBITS_0;
		ZRARRAYOP_SHIFT(bits, sizeof(char), nbZRBits * sizeof(*bits) / sizeof(char), shift, toTheRight);
		ZRARRAYOP_FILL(bits0fill, sizeof(char), shift, &zeros);
		return;
	}

	if (toTheRight)
		ZRBits_inArrayRShift_(bits, nbZRBits, shift);
	else
		ZRBits_inArrayLShift_(bits, nbZRBits, shift);
}

// ============================================================================
// SEARCH
// ============================================================================

void ZRBits_searchFixedPattern(ZRBits *bits, size_t pos, size_t nbZRBits, size_t nbBits, ZRBits **dest, size_t *outPos)
{
	ADJUST_POS(bits, pos);
	size_t const mask_nbZRBits = nbBits / ZRBITS_NBOF;
	size_t const mask_rest = (nbBits % ZRBITS_NBOF);
	size_t const offsetMask = ZRBITS_NBOF - mask_rest;
	size_t const maskSize = mask_nbZRBits + (bool)mask_rest;
	ZRBits mask[maskSize];
	ZRBits buf[maskSize];

	mask[0] = ZRBits_getRMask(mask_rest);

	if (maskSize > 0)
		memset(&mask[1], 0, mask_nbZRBits * sizeof(ZRBits));

	memset(buf, 0, maskSize * sizeof(ZRBits));
	int ipos = pos;

	for (;;)
	{
		ZRBits_copy(bits, ipos, nbBits, buf, offsetMask);

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

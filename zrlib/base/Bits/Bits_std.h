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

static inline ZRBits ZRBITS_GETLMASK_STD(unsigned nbBits)
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

static inline ZRBits ZRBITS_GETRMASK_STD(unsigned nbBits)
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

static inline ZRBits ZRBITS_BEXTR_STD(ZRBits bits, unsigned start, unsigned len)
{
	assert(start + len <= ZRBITS_NBOF);
	ZRBits const mask = ZRBITS_GETLMASK(len) >> start;
	return (bits & mask);
}

static inline unsigned ZRBITS_LZCNT_STD(ZRBits bits)
{
	if (bits == 0)
		return ZRBITS_NBOF;

	unsigned ret = 0;

	for (;;)
	{
		if (bits & ZRBITS_MASK_1L)
			return ret;

		ret++;
		bits <<= 1;
	}
}

static inline unsigned ZRBITS_RZCNT_STD(ZRBits bits)
{
	unsigned ret = 0;

	if (bits == 0)
		return ZRBITS_NBOF;

	bits = (bits ^ (bits - 1)) >> 1;

	for (; bits; ret++)
		bits >>= 1;

	return ret;
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
	assert(nbBits <= ZRBITS_NBOF);
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
	assert(nbBits <= ZRBITS_NBOF);
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

static inline void ZRBITS_FILL_STD(ZRBits *bits, size_t pos, size_t nbBits)
{
	ADJUST_POS(bits, pos);
	size_t const nbAddPos = nbBits + pos;

	if (nbAddPos <= ZRBITS_NBOF)
	{
		ZRBits const mask = ZRBITS_GETLMASK(nbBits) >> pos;
		*bits = (*bits & ~mask) | (mask & ZRBITS_MASK_FULL);
	}
	else
	{
		size_t const firstSize = ZRBITS_NBOF - pos;
		ZRBits const firstMask = ZRBITS_GETLMASK(firstSize) >> pos;
		size_t const nbZRBits = (nbBits - firstSize) / ZRBITS_NBOF;
		size_t const finalBits = (nbBits - firstSize) % ZRBITS_NBOF;
		ZRBits const finalMask = ZRBITS_GETLMASK(finalBits);

		*bits = (*bits & ~firstMask) | (firstMask & ZRBITS_MASK_FULL);
		memset(bits + 1, (int)ZRBITS_MASK_FULL, nbZRBits * sizeof(ZRBits));
		bits += nbZRBits + 1;
		*bits = (*bits & ~finalMask) | (finalMask & ZRBITS_MASK_FULL);
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

	ZRBits const mask = ZRBITS_GETLMASK(finalBits);
	*out = (*out & ~mask) | (*bits & mask);
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
		*out = (*out & ~mask) | (*bits & mask);
	}
	else
	{
		size_t const nbZRBits = (nbBits - (ZRBITS_NBOF - pos)) / ZRBITS_NBOF;
		size_t const finalBits = (nbBits + pos) % ZRBITS_NBOF;
		ZRBits const mask = ZRBITS_GETRMASK(ZRBITS_NBOF - pos);
		*out = (*out & ~mask) | (*bits & mask);
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
		ZRBits const mask = ZRBITS_GETLMASK(nbBits) >> pos;
		ZRBits const outMask = ~(mask << posSubOutPos);
		*out = (*out & outMask) | ((*bits & mask) << posSubOutPos);
	}
	// The result is store in only one ZRBits
	else if (nbAddOutPos <= ZRBITS_NBOF)
	{
		size_t const part1Size = ZRBITS_NBOF - pos;
		size_t const part2Size = nbBits - part1Size;
		ZRBits const part1Mask = ZRBITS_GETRMASK(part1Size);
		ZRBits const part2Mask = ZRBITS_GETLMASK(part2Size);
		size_t const part1Shift = posSubOutPos;
		size_t const part2Shift = (part1Size + outPos);
		ZRBits const outMask = ~((part1Mask << part1Shift) | (part2Mask >> part2Shift));
		*out = (*out & outMask) | ((*bits & part1Mask) << part1Shift) | ((*(bits + 1) & part2Mask) >> part2Shift);
	}
	else if (outPos == 0)
	{
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

		std_ZRBits_copy_posGTOutPos(bits, pos, finalBits, out, 0);
	}
	// The most general case
	else
	{
		size_t const part1Size = ZRBITS_NBOF - pos;
		size_t const part2Size = posSubOutPos;
		size_t const partTotalSize = part1Size + part2Size;
		ZRBits const part1Mask = ZRBITS_GETRMASK(part1Size);
		ZRBits const part2Mask = ZRBITS_GETLMASK(part2Size);
		ZRBits const outMask = ZRBITS_GETLMASK(outPos);

		*out = (*out & outMask) | ((*bits & part1Mask) << posSubOutPos) | ((*(bits + 1) & part2Mask) >> (part1Size + outPos));
		bits++, out++;
		std_ZRBits_copy_posGTOutPos(bits, posSubOutPos, nbBits - partTotalSize, out, 0);
	}
}

static inline void std_ZRBits_copy_posLTOutPos(ZRBits const * restrict bits, size_t pos, size_t nbBits, ZRBits * restrict out, size_t outPos)
{
	size_t const nbAddOutPos = nbBits + outPos;
	size_t const outPosSubPos = outPos - pos;

	// The result is stored in only one ZRBits
	if (nbAddOutPos <= ZRBITS_NBOF)
	{
		ZRBits const mask = ZRBITS_GETLMASK(nbBits) >> pos;
		ZRBits const outMask = ~(mask >> outPosSubPos);
		*out = (*out & outMask) | (*bits & mask) >> outPosSubPos;
	}
	else
	{
		size_t const maskSize = ZRBITS_NBOF - outPos;
		ZRBits const mask = ZRBITS_GETLMASK(maskSize) >> pos;
		ZRBits const outMask = ZRBITS_GETLMASK(outPos);

		*out = (*out & ~mask) | (*bits & mask) >> outPosSubPos;
		std_ZRBits_copy_posGTOutPos(bits, pos + maskSize, nbBits - maskSize, out + 1, 0);
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

static inline int ZRBITS_CMP_STD(ZRBits *a, ZRBits *b, size_t pos, size_t nb)
{
	// ADJUSTPOS
	if (pos >= ZRBITS_NBOF)
	{
		size_t const shift = pos / ZRBITS_NBOF;
		a += shift;
		b += shift;
		pos %= ZRBITS_NBOF;
	}
	size_t const nbAddPos = nb + pos;
	size_t const rest = nbAddPos % ZRBITS_NBOF;
	size_t const nbZRBits = nbAddPos / ZRBITS_NBOF + (rest ? 1 : 0);

	if (nbZRBits == 1)
		return ZRBITS_BEXTR(*a, pos, nb) - ZRBITS_BEXTR(*b, pos, nb);
	else
	{
		int cmp;

		// First cmp
		{
			size_t const nbBits = ZRBITS_NBOF - pos;
			cmp = ZRBITS_BEXTR(*a, pos, nbBits) - ZRBITS_BEXTR(*b, pos, nbBits);
		}

		if (cmp)
			return cmp;

		if (nbZRBits > 2)
		{
			size_t const nbZRBits2 = nbZRBits - 2;
			a++, b++;
			cmp = memcmp(a, b, (nbZRBits2) * sizeof(ZRBits));

			if (cmp)
				return cmp;

			a += nbZRBits2;
			b += nbZRBits2;
		}
		return ZRBITS_BEXTR(*a, 0, rest) - ZRBITS_BEXTR(*b, 0, rest);
	}
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

static inline size_t ZRBITS_1RPOS_STD(ZRBits *bits, size_t nbZRBits, size_t pos)
{
	if (nbZRBits == 0)
		return 0;

	bits += nbZRBits - 1;

	if (*bits)
	{
		if (pos)
		{
			ZRBits const current = *bits & ZRBITS_GETLMASK(pos + 1);

			if (current != 0)
				return ZRBITS_RZCNT(current);
		}
		else
			return ZRBITS_RZCNT(*bits);
	}

	if (nbZRBits == 1)
		return ZRBITS_NBOF;

	ZRBits *const last = bits;

	while (--nbZRBits && !*--bits)
		;

	size_t const offset = (last - bits) * ZRBITS_NBOF;
	return offset + ZRBITS_RZCNT(*bits);
}

static inline size_t ZRBITS_1LPOS_STD(ZRBits *bits, size_t nbZRBits, size_t pos)
{
	if (nbZRBits == 0)
		return 0;

	if (*bits)
	{
		if (pos)
		{
			ZRBits const current = *bits & ZRBITS_GETRMASK(ZRBITS_NBOF - pos);

			if (current != 0)
				return ZRBITS_LZCNT(current);
		}
		else
			return ZRBITS_LZCNT(*bits);
	}

	if (nbZRBits == 1)
		return ZRBITS_NBOF;

	ZRBits *const first = bits;

	while (--nbZRBits && !*++bits)
		;

	size_t const offset = (bits - first) * ZRBITS_NBOF;
	return offset + ZRBITS_LZCNT(*bits);
}

static inline void ZRBITS_SEARCHFIXEDPATTERN_STD(ZRBits *bits, size_t pos, size_t nbZRBits, size_t nbBits, ZRBits **dest, size_t *outPos)
{
	ADJUST_POS(bits, pos);

	if (nbBits == 1)
	{
		size_t ret = ZRBITS_1LPOS(bits, nbZRBits, pos);

		if (ret < ZRBITS_NBOF * nbZRBits)
		{
			*dest = bits + ret / ZRBITS_NBOF;
			*outPos = ret % ZRBITS_NBOF;
		}
		else
		{
			*dest = NULL;
			*outPos = 0;
		}
		return;
	}
	size_t const mask_nbZRBits = nbBits / ZRBITS_NBOF;
	size_t const mask_rest = (nbBits % ZRBITS_NBOF);
	size_t const maskSize = mask_nbZRBits + (bool)mask_rest;
	size_t const maskNbOfBits = maskSize * ZRBITS_NBOF;
	size_t const maskFirstBitSet = mask_rest ? ZRBITS_NBOF - mask_rest : 0;
	ZRBits mask[maskSize];
	ZRBits buf[maskSize];

	if (mask_rest)
	{
		mask[0] = ZRBITS_GETRMASK(mask_rest);

		if (maskSize > 1)
			memset(&mask[1], (int)ZRBITS_MASK_FULL, mask_nbZRBits * sizeof(ZRBits));
	}
	else
		memset(mask, (int)ZRBITS_MASK_FULL, maskSize * sizeof(ZRBits));

	memset(buf, 0, maskSize * sizeof(ZRBits));
	int ipos = pos;

	for (;;)
	{
		ZRBITS_COPY_STD(bits, ipos, nbBits, buf, maskFirstBitSet);

		if (memcmp(buf, mask, maskSize * sizeof(ZRBits)) == 0)
		{
			*dest = bits;
			*outPos = ipos;
			return;
		}
		buf[1] &= mask[1];

		size_t const firstSetBitPos = ZRBITS_1LPOS(buf, maskSize, maskFirstBitSet + 1);
		ipos += firstSetBitPos - maskFirstBitSet;

		if (ipos >= ZRBITS_NBOF)
		{
			size_t const ipos_nbZRBits = ipos / ZRBITS_NBOF;

			if (nbZRBits <= ipos_nbZRBits)
				break;

			nbZRBits -= ipos_nbZRBits;
			bits += ipos_nbZRBits;
			ipos %= ZRBITS_NBOF;
		}
	}
	*dest = NULL;
	*outPos = 0;
}

#endif

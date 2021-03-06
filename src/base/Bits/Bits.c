#include <zrlib/base/ArrayOp.h>
#include <zrlib/base/Bits/Bits.h>

#include <assert.h>

// ============================================================================

#include "Bits_std.c"
#include "Bits_intrinsic.c"

// ============================================================================
// GETMASK
// ============================================================================

ZRBits ZRBits_getLMask(unsigned nbBits)
{
	return ZRBITS_GETLMASK(nbBits);
}

ZRBits ZRBits_getRMask(unsigned nbBits)
{
	return ZRBITS_GETRMASK(nbBits);
}

ZRBits ZRBits_getMask(unsigned nbBits, bool toTheRight)
{
	if (toTheRight)
		return ZRBITS_GETRMASK(nbBits);

	return ZRBITS_GETLMASK(nbBits);
}

ZRBits ZRBits_bextr(ZRBits bits, unsigned start, unsigned len)
{
	return ZRBITS_BEXTR(bits, start, len);
}

unsigned ZRBits_lzcnt(ZRBits bits)
{
	return ZRBITS_LZCNT(bits);
}

unsigned ZRBits_rzcnt(ZRBits bits)
{
	return ZRBITS_RZCNT(bits);
}

// ============================================================================

size_t ZRBits_1LPos(ZRBits *bits, size_t nbZRBits, size_t pos)
{
	return ZRBITS_1LPOS(bits, nbZRBits, pos);
}

size_t ZRBits_1RPos(ZRBits *bits, size_t nbZRBits, size_t pos)
{
	return ZRBITS_1RPOS(bits, nbZRBits, pos);
}

// ============================================================================

void ZRBits_cpack(ZRBits *bits, size_t nbBits, char *source, size_t sourceSize)
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
void ZRBits_pack(ZRBits *restrict bits, size_t nbBits, ZRBits *restrict source, size_t sourceSize)
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
	return ZRBITS_SETBIT_STD(bits, pos, bit);
}

void ZRBits_setBitsFromTheRight(ZRBits *bits, size_t pos, size_t nbBits, ZRBits source)
{
	ZRBITS_SETBITSFROMTHERIGHT_STD(bits, pos, nbBits, source);
}

void ZRBits_setBitsFromTheLeft(ZRBits *bits, size_t pos, size_t nbBits, ZRBits source)
{
	ZRBITS_SETBITSFROMTHELEFT_STD(bits, pos, nbBits, source);
}

void ZRBits_setBits(ZRBits *bits, size_t pos, size_t nbBits, ZRBits source, bool fromTheRight)
{
	if (fromTheRight)
		ZRBITS_SETBITSFROMTHERIGHT_STD(bits, pos, nbBits, source);
	else
		ZRBITS_SETBITSFROMTHELEFT_STD(bits, pos, nbBits, source);
}

void ZRBits_fill(ZRBits *bits, size_t pos, size_t nbBits)
{
	return ZRBITS_FILL_STD(bits, pos, nbBits);
}

bool ZRBits_getBit(ZRBits const *bits, size_t pos)
{
	return ZRBITS_GETBIT_STD(bits, pos);
}

void ZRBits_getBits(ZRBits const *restrict bits, size_t pos, size_t nbBits, ZRBits *restrict out)
{
	ZRBITS_COPY_STD(bits, pos, nbBits, out, 0);
}

void ZRBits_copy(ZRBits const *restrict bits, size_t pos, size_t nbBits, ZRBits *restrict out, size_t outPos)
{
	ZRBITS_COPY_STD(bits, pos, nbBits, out, outPos);
}

int ZRBits_cmp(ZRBits *a, ZRBits *b, size_t pos, size_t nb)
{
	return ZRBITS_CMP_STD(a, b, pos, nb);
}

// ============================================================================
// SHIFT
// ============================================================================

void ZRBits_inArrayRShift(ZRBits *bits, size_t nbZRBits, size_t shift)
{
	ZRBITS_INARRAYRSHIFT_STD(bits, nbZRBits, shift);
}

void ZRBits_inArrayLShift(ZRBits *bits, size_t nbZRBits, size_t shift)
{
	ZRBITS_INARRAYLSHIFT_STD(bits, nbZRBits, shift);
}

void ZRBits_inArrayShift(ZRBits *bits, size_t nbZRBits, size_t shift, size_t toTheRight)
{
	if (toTheRight)
		ZRBITS_INARRAYRSHIFT_STD(bits, nbZRBits, shift);
	else
		ZRBITS_INARRAYLSHIFT_STD(bits, nbZRBits, shift);
}

void ZRBits_searchFixedPattern(ZRBits *bits, size_t pos, size_t nbZRBits, size_t nbBits, ZRBits **dest, size_t *outPos)
{
	ZRBITS_SEARCHFIXEDPATTERN_STD(bits, pos, nbZRBits, nbBits, dest, outPos);
}

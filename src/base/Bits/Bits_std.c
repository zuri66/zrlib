/**
 * @author zuri
 * @date vendredi 1 novembre 2019, 15:07:48 (UTC+0100)
 */

ZRBits ZRBits_getLMask_std(unsigned nbBits)
{
	return ZRBITS_GETLMASK_STD(nbBits);
}

ZRBits ZRBits_getRMask_std(unsigned nbBits)
{
	return ZRBITS_GETRMASK_STD(nbBits);
}

ZRBits ZRBitd_bextr_std(ZRBits bits, unsigned start, unsigned len)
{
	return ZRBITS_BEXTR_STD(bits, start, len);
}

unsigned ZRBits_lzcnt_std(ZRBits bits)
{
	return ZRBITS_LZCNT_STD(bits);
}

unsigned ZRBits_rzcnt_std(ZRBits bits)
{
	return ZRBITS_RZCNT_STD(bits);
}

// ============================================================================

size_t ZRBits_1LPos_std(ZRBits *bits, size_t nbZRBits, size_t pos)
{
	return ZRBITS_1LPOS_STD(bits, nbZRBits, pos);
}

size_t ZRBits_1RPos_std(ZRBits *bits, size_t nbZRBits, size_t pos)
{
	return ZRBITS_1RPOS_STD(bits, nbZRBits, pos);
}

// ============================================================================

void ZRBits_setBit_std(ZRBits *bits, size_t pos, bool bit)
{
	return ZRBITS_SETBIT_STD(bits, pos, bit);
}

void ZRBits_setBitsFromTheRight_std(ZRBits *bits, size_t pos, size_t nbBits, ZRBits source)
{
	ZRBITS_SETBITSFROMTHERIGHT_STD(bits, pos, nbBits, source);
}

void ZRBits_setBitsFromTheLeft_std(ZRBits *bits, size_t pos, size_t nbBits, ZRBits source)
{
	ZRBITS_SETBITSFROMTHELEFT_STD(bits, pos, nbBits, source);
}

void ZRBits_fill_std(ZRBits *bits, size_t pos, size_t nbBits)
{
	return ZRBITS_FILL_STD(bits, pos, nbBits);
}

bool ZRBits_getBit_std(ZRBits const *bits, size_t pos)
{
	return ZRBITS_GETBIT_STD(bits, pos);
}

void ZRBits_copy_std(ZRBits const *restrict bits, size_t pos, size_t nbBits, ZRBits *restrict out, size_t outPos)
{
	ZRBITS_COPY_STD(bits, pos, nbBits, out, outPos);
}

int ZRBits_cmp_std(ZRBits *a, ZRBits *b, size_t pos, size_t nb)
{
	return ZRBITS_CMP_STD(a, b, pos, nb);
}

void ZRBits_inArrayLShift_std(ZRBits *bits, size_t nbZRBits, size_t shift)
{
	return ZRBITS_INARRAYLSHIFT_STD(bits, nbZRBits, shift);
}

void ZRBits_inArrayRShift_std(ZRBits *bits, size_t nbZRBits, size_t shift)
{
	return ZRBITS_INARRAYRSHIFT_STD(bits, nbZRBits, shift);
}

void ZRBits_searchFixedPattern_std(ZRBits *bits, size_t pos, size_t nbZRBits, size_t nbBits, ZRBits **dest, size_t *outPos)
{
	ZRBITS_SEARCHFIXEDPATTERN_STD(bits, pos, nbZRBits, nbBits, dest, outPos);
}

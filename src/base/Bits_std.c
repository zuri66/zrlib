/**
 * @author zuri
 * @date vendredi 1 novembre 2019, 15:07:48 (UTC+0100)
 */

ZRBits ZRBits_getLMask_std(size_t nbBits)
{
	return ZRBITS_GETLMASK_STD(nbBits);
}

ZRBits ZRBits_getRMask_std(size_t nbBits)
{
	return ZRBITS_GETRMASK_STD(nbBits);
}

void ZRBits_setBit_std(ZRBits *bits, size_t pos, bool bit)
{
	return ZRBITS_SETBIT_STD(bits, pos, bit);
}

bool ZRBits_getBit_std(ZRBits const *bits, size_t pos)
{
	return ZRBITS_GETBIT_STD(bits, pos);
}

void ZRBits_copy_std(ZRBits const * restrict bits, size_t pos, size_t nbBits, ZRBits * restrict out, size_t outPos)
{
	ZRBITS_COPY_STD(bits, pos, nbBits, out, outPos);
}

void ZRBits_inArrayLShift_std(ZRBits *bits, size_t nbZRBits, size_t shift)
{

}

void ZRBits_inArrayRShift_std(ZRBits *bits, size_t nbZRBits, size_t shift)
{

}

void ZRBits_searchFixedPattern_std(ZRBits *bits, size_t pos, size_t nbZRBits, size_t nbBits, ZRBits **dest, size_t *outPos)
{
	ZRBITS_SEARCHFIXEDPATTERN_STD(bits, pos, nbZRBits, nbBits, dest, outPos);
}

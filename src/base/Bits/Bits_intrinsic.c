/**
 * @author zuri
 * @date  vendredi 1 novembre 2019, 11:24:28 (UTC+0100)
 */

ZRBits ZRBits_getLMask_i(unsigned nbBits)
{
	return ZRBITS_GETLMASK_I(nbBits);
}

ZRBits ZRBits_getRMask_i(unsigned nbBits)
{
	return ZRBITS_GETRMASK_I(nbBits);
}

ZRBits ZRBits_bextr_i(ZRBits bits, unsigned start, unsigned len)
{
	return ZRBITS_BEXTR_I(bits, start, len);
}

unsigned ZRBits_lzcnt_i(ZRBits bits)
{
	return ZRBITS_LZCNT_I(bits);
}

unsigned ZRBits_rzcnt_i(ZRBits bits)
{
	return ZRBITS_RZCNT_I(bits);
}

/**
 * @author zuri
 * @date vendredi 1 novembre 2019, 15:56:25 (UTC+0100)
 */

#ifndef ZRBITS_H
#error "This file: " __FILE__ "cannot be used outside of Bits.h"
#endif

#include <stdint.h>

#if ZRBITS_NBOF == 32 || ZRBITS_NBOF == 64
#else
#	error "Unable to define the size of ZRBits"
#endif

// ============================================================================
// LOCAL MACROS

#define FACTOR_NBOFBITS(pref,suff) ZRCONCAT(pref, ZRCONCAT(ZRBITS_NBOF,suff))
#define SUFFIX_NBOFBITS(suff) ZRCONCAT(suff,ZRBITS_NBOF)

#define _bextr_u SUFFIX_NBOFBITS(_bextr_u)
#define _lzcnt_u SUFFIX_NBOFBITS(_lzcnt_u)
#define _rzcnt_u SUFFIX_NBOFBITS(_tzcnt_u)

// ============================================================================

#ifdef _IMMINTRIN_H_INCLUDED

// ============================================================================
// 32/64 Bits
// ============================================================================

static inline ZRBits ZRBITS_GETLMASK_I(unsigned nbBits)
{
	return _bextr_u(ZRBITS_MASK_FULL, 0, nbBits) << ZRBITS_NBOF - nbBits;
}

static inline ZRBits ZRBITS_GETRMASK_I(unsigned nbBits)
{
	return _bextr_u(ZRBITS_MASK_FULL, 0, nbBits);
}

static inline ZRBits ZRBITS_BEXTR_I(ZRBits bits, unsigned start, unsigned len)
{
	return _bextr_u(bits, start, len);
}

static inline unsigned ZRBITS_LZCNT_I(ZRBits bits)
{
	return _lzcnt_u(bits);
}

static inline unsigned ZRBITS_RZCNT_I(ZRBits bits)
{
	return _rzcnt_u(bits);
}

#endif
// _IMMINTRIN_H_INCLUDED

// ============================================================================

#undef FACTOR_NBOFBITS
#undef SUFFIX_NBOFBITS
#undef _bextr_u
#undef _lzcnt_u
#undef _rzcnt_u

/**
 * @author zuri
 * @date vendredi 1 novembre 2019, 15:56:25 (UTC+0100)
 */

#ifndef ZRBITS_H
#error "This file: " __FILE__ "cannot be used outside of Bits.h"
#endif

#ifndef BITS_INTRINSIC_H
#define BITS_INTRINSIC_H

#include <zrlib/base/macro.h>
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

// ============================================================================

#ifdef _IMMINTRIN_H_INCLUDED

// ============================================================================
// 32/64 Bits
// ============================================================================

static inline ZRBits ZRBITS_GETLMASK_I(size_t nbBits)
{
	return _bextr_u(ZRBITS_MASK_FULL, 0, nbBits) << ZRBITS_NBOF - nbBits;
}

static inline ZRBits ZRBITS_GETRMASK_I(size_t nbBits)
{
	return _bextr_u(ZRBITS_MASK_FULL, 0, nbBits);
}

// ============================================================================

ZRBits ZRBits_getLMask_i(size_t nbBits);
ZRBits ZRBits_getRMask_i(size_t nbBits);

// ============================================================================

#endif
// _IMMINTRIN_H_INCLUDED

// ============================================================================

#undef FACTOR_NBOFBITS
#undef SUFFIX_NBOFBITS
#undef _bextr_u

// ============================================================================

#endif

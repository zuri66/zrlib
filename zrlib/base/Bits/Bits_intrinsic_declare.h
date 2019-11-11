/**
 * @author zuri
 * @date lundi 11 novembre 2019, 01:16:57 (UTC+0100)
 */

#ifndef ZRBITS_H
#error "This file: " __FILE__ "cannot be used outside of Bits.h"
#endif

// ============================================================================

#ifdef _IMMINTRIN_H_INCLUDED

#ifndef ZRBITS_GETLMASK
#	define ZRBITS_GETLMASK ZRBITS_GETLMASK_I
#endif
#ifndef ZRBITS_GETRMASK
#	define ZRBITS_GETRMASK ZRBITS_GETRMASK_I
#endif
#ifndef ZRBITS_BEXTR
#	define ZRBITS_BEXTR ZRBITS_BEXTR_I
#endif
#ifndef ZRBITS_LZCNT
#	define ZRBITS_LZCNT ZRBITS_LZCNT_I
#endif
#ifndef ZRBITS_RZCNT
#	define ZRBITS_RZCNT ZRBITS_RZCNT_I
#endif

// ============================================================================

ZRBits ZRBits_getLMask_i(unsigned nbBits);
ZRBits ZRBits_getRMask_i(unsigned nbBits);

ZRBits ZRBits_bextr_i(ZRBits bits, unsigned start, unsigned len);

unsigned ZRBits_lzcnt_i(ZRBits bits);
unsigned ZRBits_rzcnt_i(ZRBits bits);

#endif
// _IMMINTRIN_H_INCLUDED

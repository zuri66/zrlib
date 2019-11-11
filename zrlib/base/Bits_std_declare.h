/**
 * @author zuri
 * @date lundi 11 novembre 2019, 01:16:57 (UTC+0100)
 */

#ifndef ZRBITS_H
#error "This file: " __FILE__ "cannot be used outside of Bits.h"
#endif

// ============================================================================

#ifndef ZRBITS_GETLMASK
#	define ZRBITS_GETLMASK ZRBITS_GETLMASK_STD
#endif
#ifndef ZRBITS_GETRMASK
#	define ZRBITS_GETRMASK ZRBITS_GETRMASK_STD
#endif
#ifndef ZRBITS_LZCNT
#	define ZRBITS_LZCNT ZRBITS_LZCNT_STD
#endif
#ifndef ZRBITS_RZCNT
#	define ZRBITS_RZCNT ZRBITS_RZCNT_STD
#endif
#ifndef ZRBITS_BEXTR
#	define ZRBITS_BEXTR ZRBITS_BEXTR_STD
#endif
#ifndef ZRBITS_1LPOS
#	define ZRBITS_1LPOS ZRBITS_1LPOS_STD
#endif
#ifndef ZRBITS_1RPOS
#	define ZRBITS_1RPOS ZRBITS_1RPOS_STD
#endif

// ============================================================================

static inline ZRBits ZRBITS_BEXTR_STD(ZRBits bits, unsigned start, unsigned len);

static inline ZRBits ZRBITS_GETLMASK_STD(unsigned nbBits);
static inline ZRBits ZRBITS_GETRMASK_STD(unsigned nbBits);

static inline unsigned ZRBITS_LZCNT_STD(ZRBits bits);
static inline unsigned ZRBITS_RZCNT_STD(ZRBits bits);

// ============================================================================

ZRBits ZRBits_getLMask_std(unsigned nbBits);
ZRBits ZRBits_getRMask_std(unsigned nbBits);

size_t ZRBits_1LPos_std(ZRBits *bits, size_t nbZRBits, size_t pos);
size_t ZRBits_1RPos_std(ZRBits *bits, size_t nbZRBits, size_t pos);

void ZRBits_setBit_std(______________ ZRBits *bits, size_t pos, bool bit);
void ZRBits_setBitsFromTheRight_std(_ ZRBits *bits, size_t pos, size_t nbBits, ZRBits source);
void ZRBits_setBitsFromTheLeft_std(__ ZRBits *bits, size_t pos, size_t nbBits, ZRBits source);
void ZRBits_fill_std(________________ ZRBits *bits, size_t pos, size_t nbBits);

bool ZRBits_getBit_std(ZRBits const * ________ bits, size_t pos);
void ZRBits_copy_std(_ ZRBits const * restrict bits, size_t pos, size_t nbBits, ZRBits * restrict out, size_t outPos);

void ZRBits_inArrayLShift_std(ZRBits *bits, size_t nbZRBits, size_t shift);
void ZRBits_inArrayRShift_std(ZRBits *bits, size_t nbZRBits, size_t shift);

void ZRBits_searchFixedPattern_std(ZRBits *bits, size_t pos, size_t nbZRBits, size_t nbBits, ZRBits **dest, size_t *outPos);

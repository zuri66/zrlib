/**
 * @author zuri
 * @date dimanche 9 décembre 2018, 21:11:56 (UTC+0100)
 */

#ifndef ZRBITFIELD_H
#define ZRBITFIELD_H

#include <zrlib/base/macro.h>

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// ============================================================================

#if defined(UINT32_MAX)
typedef uint32_t ZRBits;

#define ZRBITS_PRI_LENGTH ""
#define ZRBITS_PRI_WIDTH "8"
#else
typedef unsigned ZRBits;

//=== arbitrary, may be wrong
#define ZRBITS_PRI_LENGTH ""
#define ZRBITS_PRI_WIDTH "8"
//===

#endif

#define ZRBITS_PRI_PREFIX "0" ZRBITS_PRI_WIDTH
#define ZRBITS_PRI_SPECIFIER(spec) \
	"%" ZRBITS_PRI_PREFIX ZRBITS_PRI_LENGTH spec
#define ZRBITS_PRI \
	ZRBITS_PRI_SPECIFIER("X")

// ============================================================================

/**
 * Number of bits in an object ZRBits.
 */
#define ZRBITS_NBOF (sizeof(ZRBits) * CHAR_BIT)

#define ZRBITS_0 ((ZRBits)0)

#define ZRBITS_MASK_1R ((ZRBits)1)
#define ZRBITS_MASK_1L ~(ZRBITS_MASK_FULL >> 1)
#define ZRBITS_MASK_FULL (~(ZRBits)0)

// ============================================================================

#define ZRBITS_1ROTATE(bits,nb,shiftOp,shiftOpReverse) \
	(((bits) shiftOp nb) | ((bits) shiftOpReverse (sizeof(bits) * CHAR_BIT - nb)))
#define ZRBITS_1ROTATEL(bits,nb) ZRBITS_1ROTATE(bits,nb,<<,>>)
#define ZRBITS_1ROTATER(bits,nb) ZRBITS_1ROTATE(bits,nb,>>,<<)

// ============================================================================

#define ZRBITS_PACK_ARRAY(bits,nbBits,source) \
do{\
	register size_t pos = 0;\
	register size_t i = 0;\
	register size_t ss = ZRLIB_ARRAY_NBOBJ(source);\
	\
	while (ss--)\
	{\
		ZRBits_setBitsFromTheRight(bits, pos, nbBits, source[i++]);\
		pos += nbBits;\
	}\
}while(0)

#define ZRBITS_PACK_ARRAY_TYPE(bits,source) \
	ZRBITS_PACK_(bits,nbBits,sizeof(*source),ZRLIB_ARRAY_NBOBJ(source))

// ============================================================================

ZRBits ZRBits_getMask (size_t nbBits, bool toTheRight);
ZRBits ZRBits_getLMask(size_t nbBits);
ZRBits ZRBits_getRMask(size_t nbBits);

void ZRBits_cpack(ZRBits *          bits, size_t nbBits, char   *          source, size_t sourceSize);
void ZRBits_pack (ZRBits * restrict bits, size_t nbBits, ZRBits * restrict source, size_t sourceSize);

void ZRBits_setBit             (ZRBits *bits, size_t pos, bool bit);
void ZRBits_setBits            (ZRBits *bits, size_t pos, size_t nbBits, ZRBits source, bool fromTheRight);
void ZRBits_setBitsFromTheRight(ZRBits *bits, size_t pos, size_t nbBits, ZRBits source);
void ZRBits_setBitsFromTheLeft (ZRBits *bits, size_t pos, size_t nbBits, ZRBits source);

bool ZRBits_getBit (ZRBits const *          bits, size_t pos);
void ZRBits_getBits(ZRBits const * restrict bits, size_t pos, size_t nbBits, ZRBits * restrict out);
void ZRBits_copy   (ZRBits const * restrict bits, size_t pos, size_t nbBits, ZRBits * restrict out, size_t outPos);

void ZRBits_inArrayShift (ZRBits *bits, size_t nbZRBits, size_t shift, size_t toTheRight);
void ZRBits_inArrayLShift(ZRBits *bits, size_t nbZRBits, size_t shift);
void ZRBits_inArrayRShift(ZRBits *bits, size_t nbZRBits, size_t shift);

#endif

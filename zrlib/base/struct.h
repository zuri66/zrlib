/**
 * @author zuri
 * @date dimanche 12 janvier 2020
 */

#ifndef ZRSTRUCT_H
#define ZRSTRUCT_H

#include <zrlib/config.h>
#include <zrlib/base/ArrayOp.h>
#include <zrlib/base/math.h>

#include <assert.h>

#ifndef ZRSTRUCT_MAXFIELDS
#	define ZRSTRUCT_MAXFIELDS 128
#endif

typedef struct ZRObjInfosS
{
	size_t alignment;
	size_t size;
} ZRObjInfos;

#define ZROBJINFOS_SIZE_ALIGNMENT(I) (I).size, (I).alignment
#define ZROBJINFOS_ALIGNMENT_SIZE(I) (I).alignment, (I).size

#define ZROBJINFOS_DEF(A,S) ((ZRObjInfos) { (A), (S) })
#define ZROBJINFOS_DEF0() ZROBJINFOS_DEF(0, 0)
#define ZROBJINFOS_DEF_UNKNOWN() ZROBJINFOS_DEF(ZRSIZE_UNKNOWN, ZRSIZE_UNKNOWN)
#define ZROBJINFOS_ISUNKNOWN(A) (ZROBJINFOS_CMP(ZROBJINFOS_DEF_UNKNOWN(), A) == 0)

#define ZRSTRUCT_FLAG_POW2 0
#define ZRSTRUCT_FLAG_ARITHMETIC 1
#define ZRSTRUCT_FLAG_RESIZE 2

#ifndef ZRSTRUCT_DEFAULT_FLAGS
#define ZRSTRUCT_DEFAULT_FLAGS (ZRSTRUCT_FLAG_POW2)
#endif

ZRMUSTINLINE
static inline int ZROBJINFOS_CMP(ZRObjInfos a, ZRObjInfos b)
{
	return a.alignment - b.alignment + a.size - b.size;
}

ZRMUSTINLINE
static inline int ZROBJINFOS_EQ(ZRObjInfos a, ZRObjInfos b)
{
	return 0 == ZROBJINFOS_CMP(a, b);
}

typedef struct
{
	ZRObjInfos infos;
	void *object;
} ZRObjectP;

typedef struct
{
	ZRObjInfos infos;
	char object[];
} ZRObject;

#define ZROBJECTP(O) ((ZRObjectP*)(O))
#define ZROBJECT(O)  ((ZRObject*)(O))

#define ZROBJECTP_DEF(I,P) (ZRObjectP) { I, P }
#define ZROBJECTP_DEF0() ZROBJECTP_DEF(ZROBJINFOS_DEF0(), NULL)

typedef struct ZRObjAlignInfosS
{
	size_t offset;
	size_t alignment;
	size_t size;
} ZRObjAlignInfos;

#define ZROBJALIGNINFOS_SIZE_ALIGNMENT(I) (I).size, (I).alignment
#define ZROBJALIGNINFOS_ALIGNMENT_SIZE(I) (I).alignment, (I).size

#define ZROBJALIGNINFOS_DEF(O,A,S)  ((ZRObjAlignInfos) { (O), (A), (S) })
#define ZROBJALIGNINFOS_DEF0()      ((ZRObjAlignInfos) { 0, 0, 0 })
#define ZROBJALIGNINFOS_DEF_AS(A,S) ((ZRObjAlignInfos) { 0, (A), (S) })

ZRMUSTINLINE
static inline ZRObjAlignInfos ZROBJINFOS_CPYOBJALIGNINFOS(ZRObjInfos objInfos)
{
	ZRObjAlignInfos ret = { 0, objInfos.alignment, objInfos.size };
	return ret;
}

ZRMUSTINLINE
static inline ZRObjInfos ZROBJALIGNINFOS_CPYOBJINFOS(ZRObjAlignInfos alignInfos)
{
	ZRObjInfos ret = { alignInfos.alignment, alignInfos.size };
	return ret;
}

ZRMUSTINLINE
static inline void ZROBJALIGNINFOS_SETOBJINFOS(ZRObjAlignInfos *alignInfos, ZRObjInfos objInfos)
{
	alignInfos->alignment = objInfos.alignment;
	alignInfos->size = objInfos.size;
}

ZRMUSTINLINE
static inline size_t ZRSTRUCT_ALIGNOFFSET(size_t fieldOffset, size_t alignment)
{
	assert(alignment != 0);
	size_t rest = fieldOffset % alignment;

	if (rest)
		return fieldOffset + alignment - rest;

	return fieldOffset;
}

ZRMUSTINLINE
static inline size_t ZRALIGNMENT_UNION2_FLAGS(size_t a, size_t b, unsigned flags)
{
	bool const arithm = flags & ZRSTRUCT_FLAG_ARITHMETIC;

	/* Alignments may be any values */
	if (arithm)
	{
		if (a == b)
			return a;
		if (a > b)
		{
			/* alignments are multiples */
			if (0 == (a % b))
				return a;
		}
		else
		{
			if (0 == (b % a))
				return b;
		}
		/* Must compute the alignment */
		return (a * b) / ZRMATH_GCD(a, b);
	}
	/* We assert that alignments are all power of 2*/
	else
	{
		assert(ZRISPOW2(a));
		assert(ZRISPOW2(b));
		return ZRMAX(a, b);
	}
}

ZRMUSTINLINE
static inline size_t ZRALIGNMENT_UNION2(size_t a, size_t b)
{
	return ZRALIGNMENT_UNION2_FLAGS(a, b, ZRSTRUCT_DEFAULT_FLAGS);
}

/**
 * Construct an ObjInfos able to represent a or b in memory like a c union.
 */
ZRMUSTINLINE
static inline ZRObjInfos ZROBJINFOS_UNION2_FLAGS(ZRObjInfos a, ZRObjInfos b, unsigned flags)
{
	return ZROBJINFOS_DEF(ZRALIGNMENT_UNION2_FLAGS(a.alignment, b.alignment, flags), ZRMAX(a.size, b.size));
}

ZRMUSTINLINE
static inline ZRObjInfos ZROBJINFOS_UNION2(ZRObjInfos a, ZRObjInfos b)
{
	return ZROBJINFOS_DEF(ZRALIGNMENT_UNION2_FLAGS(a.alignment, b.alignment, ZRSTRUCT_DEFAULT_FLAGS), ZRMAX(a.size, b.size));
}

ZRMUSTINLINE
static inline ZRObjInfos ZROBJINFOS_UNION_FLAGS(ZRObjInfos *infos, size_t nb, unsigned flags)
{
	if (nb == 0)
		return ZROBJINFOS_DEF_UNKNOWN();

	ZRObjInfos ret = ZROBJINFOS_DEF(1, 0);

	while (nb--)
	{
		ret = ZROBJINFOS_UNION2_FLAGS(*infos, ret, flags);
		infos++;
	}
	return ret;
}

ZRMUSTINLINE
static inline ZRObjInfos ZROBJINFOS_UNION(ZRObjInfos *infos, size_t nb)
{
	return ZROBJINFOS_UNION_FLAGS(infos, nb, ZRSTRUCT_DEFAULT_FLAGS);
}

#define ZRObjInfos_union_flags_l_X(nb,depth,A,B) ZROBJINFOS_UNION2_FLAGS(A,B,flags)
#define ZRObjInfos_union_flags_l(flags, ...) ZRARGS_XCAPPLYREC_E(ZRObjInfos_union_flags_l_X,__VA_ARGS__)
#define ZRObjInfos_union_l(...) ZRObjInfos_union_flags_l(ZRSTRUCT_DEFAULT_FLAGS, __VA_ARGS__)

// ============================================================================

#define ZRSTRUCT_MAKEOFFSETS ZRStruct_makeOffsets

size_t ZRStruct_alignOffset(size_t fieldOffset, size_t alignment);
void ZRStruct_makeOffsets(size_t nb, ZRObjAlignInfos *infos);
void ZRStruct_makeOffsets_flags(size_t nb, ZRObjAlignInfos *infos, unsigned flags);
void ZRStruct_bestOrder(size_t nb, ZRObjAlignInfos *infos, ZRObjAlignInfos **pinfos);
void ZRStruct_bestOrderPos(size_t nb, ZRObjAlignInfos *infos, ZRObjAlignInfos **pinfos, size_t pos);
void ZRStruct_bestOffsets(size_t nb, ZRObjAlignInfos *infos);
void ZRStruct_bestOffsets_flags(size_t nb, ZRObjAlignInfos *infos, unsigned flags);
void ZRStruct_bestOffsetsPos(size_t nb, ZRObjAlignInfos *infos, size_t pos);
void ZRStruct_bestOffsetsPos_flags(size_t nb, ZRObjAlignInfos *infos, size_t pos, unsigned flags);

#endif

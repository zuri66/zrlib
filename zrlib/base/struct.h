/**
 * @author zuri
 * @date dimanche 12 janvier 2020
 */

#ifndef ZRSTRUCT_H
#define ZRSTRUCT_H

#include <zrlib/config.h>
#include <zrlib/base/ArrayOp.h>

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

// ============================================================================

#define ZRSTRUCT_FLAG_POW2 0
#define ZRSTRUCT_FLAG_ARITHMETIC 1
#define ZRSTRUCT_FLAG_RESIZE 2

#ifndef ZRSTRUCT_DEFAULT_FLAGS
#define ZRSTRUCT_DEFAULT_FLAGS (ZRSTRUCT_FLAG_POW2)
#endif

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

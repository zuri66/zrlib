/**
 * @author zuri
 * @date dimanche 12 janvier 2020
 */

#ifndef ZRSTRUCT_H
#define ZRSTRUCT_H

#include <zrlib/config.h>
#include <zrlib/base/ArrayOp.h>

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

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
#define ZROBJINFOS_UNKNOWN(A) (ZROBJINFOS_CMP(ZROBJINFOS_DEF0(), A) == 0)

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

/**
 * Size for structure with a flexible array member which conserve the structure alignment.
 * The size of the FAM must be counted in the structSize argument.
 *
 * DEPRECATED
 */
ZRMUSTINLINE
static inline size_t ZRSTRUCTSIZE_FAM_PAD(size_t structSize, size_t alignment)
{
	return ZRSTRUCT_ALIGNOFFSET(structSize, alignment);
}

/**
 * Compute the offset of infos.
 * Each infos item represents a field in a struct.
 * The infos[nb+1] item represent the struct infos.
 */
ZRMUSTINLINE
static inline void ZRSTRUCT_MAKEOFFSETS(size_t nb, ZRObjAlignInfos *infos)
{
	size_t offset = infos->offset;
	size_t max_alignment = 0;

	while (nb--)
	{
		if (infos->alignment == 0 || infos->size == 0)
		{
			infos++;
			continue;
		}

		if (infos->alignment > max_alignment)
			max_alignment = infos->alignment;

		offset = ZRSTRUCT_ALIGNOFFSET(offset, infos->alignment);
		infos->offset = offset;
		offset += infos->size;
		infos++;
	}
	// The struct infos
	infos->alignment = max_alignment;
	infos->size = ZRSTRUCT_ALIGNOFFSET(offset, max_alignment);
	infos->offset = 0;
}

static int cmp_infos(const void *va, const void *vb)
{
	const ZRObjAlignInfos *a = *(ZRObjAlignInfos**)va;
	const ZRObjAlignInfos *b = *(ZRObjAlignInfos**)vb;
	return b->alignment - a->alignment;
}

ZRMUSTINLINE
static inline void ZRSTRUCT_BESTORDERPOS(size_t nb, ZRObjAlignInfos *infos, ZRObjAlignInfos **pinfos, size_t pos)
{
	// Copy also the nb + 1 struct infos
	ZRCARRAY_TOPOINTERS(ZRObjAlignInfos, pinfos, ZRObjAlignInfos, infos, nb + 1);

	qsort(pinfos + pos, nb - pos, sizeof(ZRObjAlignInfos*), cmp_infos);
}

ZRMUSTINLINE
static inline void ZRSTRUCT_BESTORDER(size_t nb, ZRObjAlignInfos *infos, ZRObjAlignInfos **pinfos)
{
	ZRSTRUCT_BESTORDERPOS(nb, infos, pinfos, 0);
}

// ============================================================================

size_t ZRStruct_alignOffset(size_t fieldOffset, size_t alignment);
#define ZRStructSize_FAM_pad ZRStruct_alignOffset
void ZRStruct_makeOffsets(size_t nb, ZRObjAlignInfos *infos);
void ZRStruct_bestOrder(size_t nb, ZRObjAlignInfos *infos, ZRObjAlignInfos **pinfos);
void ZRStruct_bestOrderPos(size_t nb, ZRObjAlignInfos *infos, ZRObjAlignInfos **pinfos, size_t pos);
void ZRStruct_bestOffsets(size_t nb, ZRObjAlignInfos *infos);
void ZRStruct_bestOffsetsPos(size_t nb, ZRObjAlignInfos *infos, size_t pos);

#endif

/**
 * @author zuri
 * @date dimanche 12 janvier 2020
 */

#ifndef ZRSTRUCT_H
#define ZRSTRUCT_H

#include <zrlib/config.h>
#include <zrlib/base/ArrayOp.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifndef ZRSTRUCT_MAXFIELDS
#	define ZRSTRUCT_MAXFIELDS 128
#endif

typedef struct ZRObjAlignInfosS
{
	size_t offset;
	size_t alignment;
	size_t size;
} ZRObjAlignInfos;


ZRMUSTINLINE
static inline size_t ZRSTRUCT_ALIGNOFFSET(size_t fieldOffset, size_t alignment)
{
//	assert(alignment != 0);
	size_t rest = fieldOffset % alignment;

	if (rest)
		return fieldOffset + alignment - rest;

	return fieldOffset;
}

/**
 * Size for structure with a flexible array member which conserve the structure alignment.
 * The size of the FAM must be counted in the structSize argument.
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
	ZRARRAYOP_TOPOINTERS(pinfos, sizeof(*pinfos), nb + 1, infos, sizeof(*infos));

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

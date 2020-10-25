/**
 * @author zuri
 * @date ardi 14 janvier 2020, 19:15:11 (UTC+0100)
 */

#include <zrlib/base/struct.h>
#include <zrlib/base/math.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

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

ZRMUSTINLINE
static inline void _ZRSTRUCT_MAKEOFFSETS(size_t nb, ZRObjAlignInfos *infos, unsigned flags)
{
	bool const arithm = flags & ZRSTRUCT_FLAG_ARITHMETIC;
	bool const resize = flags & ZRSTRUCT_FLAG_RESIZE;

	size_t offset = infos->offset;
	size_t max_alignment = 1;

	while (nb--)
	{
		if (infos->alignment == 0 || infos->size == 0)
		{
			infos++;
			continue;
		}

		/* Change the size according to the alignment if needed */
		if (resize)
		{
			if (infos->size < infos->alignment)
				infos->size = infos->alignment;
			else if (infos->size != infos->alignment)
			{
				size_t rest = infos->size % infos->alignment;
				infos->size += rest;
			}
		}

		/* Alignments may be any values */
		if (arithm)
			do
			{
				size_t a, b, infosAlign = infos->alignment;

				/* Simple cases: alignments are multiples */
				if (infosAlign == max_alignment)
					break;
				else if (infosAlign > max_alignment)
				{
					if (0 == (infosAlign % max_alignment))
					{
						max_alignment = infosAlign;
						break;
					}
				}
				else
				{
					if (0 == (max_alignment % infosAlign))
						break;
				}
				/* Must compute the alignment */
				a = infos->alignment;
				b = max_alignment;
				size_t gcd = ZRMATH_GCD(a, b);
				max_alignment = (a * b) / gcd;
			} while (0);
		/* We assert that alignments are all power of 2*/
		else
		{
			assert(ZRISPOW2(infos->alignment));

			if (infos->alignment > max_alignment)
				max_alignment = infos->alignment;
		}
		offset = ZRSTRUCT_ALIGNOFFSET(offset, infos->alignment);
		infos->offset = offset;
		offset += infos->size;
		infos++;
	}
	ZRObjAlignInfos ret =
		{ //
		.alignment = max_alignment,
		.size = ZRSTRUCT_ALIGNOFFSET(offset, max_alignment),
		.offset = 0,
		};
	*infos = ret;
}

ZRMUSTINLINE
static inline void ZRSTRUCT_BESTOFFSETSPOS_FLAGS(size_t nb, ZRObjAlignInfos *infos, size_t pos, unsigned flags)
{
	assert(nb + 1 <= ZRSTRUCT_MAXFIELDS);
	ZRObjAlignInfos *pinfos[ZRSTRUCT_MAXFIELDS];
	ZRObjAlignInfos tmpInfos[ZRSTRUCT_MAXFIELDS];

	ZRSTRUCT_BESTORDERPOS(nb, infos, pinfos, pos);
	ZRCARRAY_FROMPOINTERSDATA(ZRObjAlignInfos, tmpInfos, pinfos, nb);

	_ZRSTRUCT_MAKEOFFSETS(nb, tmpInfos, flags);
	ZRCARRAY_TOPOINTERSDATA(ZRObjAlignInfos, pinfos, tmpInfos, nb + 1);
}

/* ========================================================================= */

/**
 * Compute the offset of infos.
 * Each infos item represents a field in a struct.
 * The infos[nb+1] item represent the struct infos.
 */
void ZRStruct_makeOffsets(size_t nb, ZRObjAlignInfos *infos)
{
	_ZRSTRUCT_MAKEOFFSETS(nb, infos, ZRSTRUCT_DEFAULT_FLAGS);
}

void ZRStruct_makeOffsets_flags(size_t nb, ZRObjAlignInfos *infos, unsigned flags)
{
	_ZRSTRUCT_MAKEOFFSETS(nb, infos, flags);
}

void ZRStruct_bestOffsets(size_t nb, ZRObjAlignInfos *infos)
{
	ZRSTRUCT_BESTOFFSETSPOS_FLAGS(nb, infos, 0, ZRSTRUCT_DEFAULT_FLAGS);
}

void ZRStruct_bestOffsets_flags(size_t nb, ZRObjAlignInfos *infos, unsigned flags)
{
	ZRSTRUCT_BESTOFFSETSPOS_FLAGS(nb, infos, 0, flags);
}

void ZRStruct_bestOffsetsPos(size_t nb, ZRObjAlignInfos *infos, size_t pos)
{
	ZRSTRUCT_BESTOFFSETSPOS_FLAGS(nb, infos, pos, ZRSTRUCT_DEFAULT_FLAGS);
}

void ZRStruct_bestOffsetsPos_flags(size_t nb, ZRObjAlignInfos *infos, size_t pos, unsigned flags)
{
	ZRSTRUCT_BESTOFFSETSPOS_FLAGS(nb, infos, pos, flags);
}

size_t ZRStruct_alignOffset(size_t fieldOffset, size_t alignment)
{
	return ZRSTRUCT_ALIGNOFFSET(fieldOffset, alignment);
}

void ZRStruct_bestOrderPos(size_t nb, ZRObjAlignInfos *infos, ZRObjAlignInfos **pinfos, size_t pos)
{
	ZRSTRUCT_BESTORDERPOS(nb, infos, pinfos, pos);
}

void ZRStruct_bestOrder(size_t nb, ZRObjAlignInfos *infos, ZRObjAlignInfos **pinfos)
{
	ZRSTRUCT_BESTORDER(nb, infos, pinfos);
}

/**
 * @author zuri
 * @date ardi 14 janvier 2020, 19:15:11 (UTC+0100)
 */

#include <zrlib/base/struct.h>

#include <assert.h>

void ZRStruct_bestOffsets(size_t nb, ZRObjAlignInfos *infos)
{
	ZRStruct_bestOffsetsPos(nb, infos, 0);
}

void ZRStruct_bestOffsetsPos(size_t nb, ZRObjAlignInfos *infos, size_t pos)
{
	assert(nb + 1 <= ZRSTRUCT_MAXFIELDS);
	ZRObjAlignInfos *pinfos[ZRSTRUCT_MAXFIELDS];
	ZRObjAlignInfos tmpInfos[ZRSTRUCT_MAXFIELDS];

	ZRSTRUCT_BESTORDERPOS(nb, infos, pinfos, pos);
	ZRARRAYOP_FROMPOINTERSDATA(tmpInfos, sizeof(*tmpInfos), nb, pinfos, sizeof(*pinfos));

	ZRSTRUCT_MAKEOFFSETS(nb, tmpInfos);
	ZRARRAYOP_TOPOINTERSDATA(pinfos, sizeof(*pinfos), nb + 1, tmpInfos, sizeof(*tmpInfos));
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

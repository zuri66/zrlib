/**
 * @author zuri
 * @date ardi 14 janvier 2020, 19:15:11 (UTC+0100)
 */

#include <zrlib/base/struct.h>

#include <assert.h>


void ZRStruct_bestOffsets(size_t nb, ZRObjAlignInfos *infos)
{
	assert(nb + 1 <= ZRSTRUCT_MAXFIELDS);
	ZRObjAlignInfos *pinfos[ZRSTRUCT_MAXFIELDS];
	ZRObjAlignInfos tmpInfos[ZRSTRUCT_MAXFIELDS];

	ZRSTRUCT_BESTORDER(nb, infos, pinfos);

	for (size_t i = 0; i < nb; i++)
		memcpy(&tmpInfos[i], pinfos[i], sizeof(ZRObjAlignInfos));

	ZRSTRUCT_MAKEOFFSETS(nb, tmpInfos);

	for (size_t i = 0; i <= nb; i++)
		memcpy(pinfos[i], &tmpInfos[i], sizeof(ZRObjAlignInfos));
}

size_t ZRStruct_alignOffset(size_t fieldOffset, size_t alignment)
{
	return ZRSTRUCT_ALIGNOFFSET(fieldOffset, alignment);
}

void ZRStruct_bestOrder(size_t nb, ZRObjAlignInfos *infos, ZRObjAlignInfos **pinfos)
{
	ZRSTRUCT_BESTORDER(nb, infos, pinfos);
}

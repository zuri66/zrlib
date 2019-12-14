/**
 * @author zuri
 * @date dimanche 24 novembre 2019, 00:08:55 (UTC+0100)
 */

#include <zrlib/base/ReserveOp_list.h>

// ============================================================================

size_t ZRReserveOpList_reserveFirstAvailables(void *reserve, size_t objSize, size_t nbObj, size_t offsetReserveNextUnused, size_t nbAvailables)
{
	return ZRRESERVEOPLIST_RESERVEFIRSTAVAILABLES(reserve, objSize, nbObj, offsetReserveNextUnused, nbAvailables);
}

void ZRReserveOpList_reserveNb(void *reserve, size_t objSize, size_t nbObj, size_t offsetReserveNextUnused, size_t pos, size_t nbAvailables)
{
	ZRRESERVEOPLIST_RELEASENB(reserve, objSize, nbObj, offsetReserveNextUnused, pos, nbAvailables);
}

bool ZRReserveOpList_availables(void *reserve, size_t objSize, size_t offsetReserveNextUnused, size_t pos, size_t nbAvailables)
{
	return ZRRESERVEOPLIST_AVAILABLES(reserve, objSize, offsetReserveNextUnused, pos, nbAvailables);
}

void ZRReserveOpList_releaseNb(void *reserve, size_t objSize, size_t nbObj, size_t offsetReserveNextUnused, size_t pos, size_t nbToRelease)
{
	ZRRESERVEOPLIST_RELEASENB(reserve, objSize, nbObj, offsetReserveNextUnused, pos, nbToRelease);
}

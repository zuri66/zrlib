/**
 * @author zuri
 * @date samedi 23 novembre 2019, 20:53:19 (UTC+0100)
 */

#include <zrlib/base/ReserveOp_bits.h>

// ============================================================================

size_t ZRReserveOpBits_reserveFirstAvailables(ZRBits *bits, size_t nbZRBits, size_t nb)
{
	return ZRRESERVEOPBITS_RESERVEFIRSTAVAILABLES(bits, nbZRBits, nb);
}

bool ZRReserveOpBits_availables(ZRBits *bits, size_t pos, size_t nb)
{
	return ZRRESERVEOPBITS_AVAILABLES(bits, pos, nb);
}

void ZRReserveOpBits_reserveNb(ZRBits *bits, size_t pos, size_t nb)
{
	ZRRESERVEOPBITS_RESERVENB(bits, pos, nb);
}

void ZRReserveOpBits_releaseNb(ZRBits *bits, size_t pos, size_t nb)
{
	ZRRESERVEOPBITS_RELEASENB(bits, pos, nb);
}

#include <zrlib/base/ResizeOp.h>

bool ZRResizeOp_mustGrowSimple(size_t total, size_t used, void *vec_p)
{
	return total < used;
}

bool ZRResizeOp_mustGrowTwice(size_t total, size_t used, void *vec_p)
{
	return (total / 2) < used;
}

size_t ZRResizeOp_increaseSpaceTwice(size_t totalSpace, size_t usedSpace, void *vec_p)
{
	return totalSpace * 2;
}

bool ZRResizeOp_mustShrink4(size_t total, size_t used, void *vec_p)
{
	return (total / 4) > used;
}

size_t ZRResizeOp_decreaseSpaceTwice(size_t totalSpace, size_t usedSpace, void *vec_p)
{
	return totalSpace / 2;
}

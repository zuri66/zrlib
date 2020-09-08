#include <zrlib/base/ResizeOp.h>

size_t ZRResizeOp_increase_25(size_t totalSpace, void *vec_p)
{
	return (5 * totalSpace) / 4;
}

size_t ZRResizeOp_increase_50(size_t totalSpace, void *vec_p)
{
	return (3 * totalSpace) / 2;
}

size_t ZRResizeOp_increase_75(size_t totalSpace, void *vec_p)
{
	return (7 * totalSpace) / 4;
}

size_t ZRResizeOp_increase_100(size_t totalSpace, void *vec_p)
{
	return totalSpace * 2;
}

size_t ZRResizeOp_limit_25(size_t totalSpace, void *vec_p)
{
	return totalSpace / 4;
}

size_t ZRResizeOp_limit_50(size_t totalSpace, void *data)
{
	return totalSpace / 2;
}

size_t ZRResizeOp_limit_55(size_t totalSpace, void *data)
{
	return (11 * totalSpace) / 20;
}

size_t ZRResizeOp_limit_60(size_t totalSpace, void *data)
{
	return (3 * totalSpace) / 5;
}

size_t ZRResizeOp_limit_70(size_t totalSpace, void *data)
{
	return (7 * totalSpace) / 10;
}

size_t ZRResizeOp_limit_75(size_t totalSpace, void *data)
{
	return (3 * totalSpace) / 4;
}

size_t ZRResizeOp_limit_80(size_t totalSpace, void *data)
{
	return (4 * totalSpace) / 5;
}

size_t ZRResizeOp_limit_85(size_t totalSpace, void *data)
{
	return (17 * totalSpace) / 20;
}

size_t ZRResizeOp_limit_90(size_t totalSpace, void *data)
{
	return (9 * totalSpace) / 10;
}

size_t ZRResizeOp_limit_95(size_t totalSpace, void *data)
{
	return (19 * totalSpace) / 20;
}

size_t ZRResizeOp_limit_100(size_t totalSpace, void *data)
{
	return totalSpace;
}

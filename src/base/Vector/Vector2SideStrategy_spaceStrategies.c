bool mustGrowTwice(size_t total, size_t used, ZRVector *vec)
{
	size_t const free = total - used;
	return (free / 2) < used;
}

size_t increaseSpaceTwice(size_t totalSpace, size_t usedSpace, ZRVector *vec)
{
	return totalSpace * 2;
}

bool mustShrink4(size_t total, size_t used, ZRVector *vec)
{
	size_t const free = total - used;
	return (free / 4) > used;
}

size_t decreaseSpaceTwice(size_t totalSpace, size_t usedSpace, ZRVector *vec)
{
	return totalSpace / 2;
}

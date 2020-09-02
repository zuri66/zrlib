bool mustGrowSimple(size_t total, size_t used, void *vec_p)
{
	return total < used;
}

bool mustGrowTwice(size_t total, size_t used, void *vec_p)
{
	return (total / 2) < used;
}

size_t increaseSpaceTwice(size_t totalSpace, size_t usedSpace, void *vec_p)
{
	return totalSpace * 2;
}

bool mustShrink4(size_t total, size_t used, void *vec_p)
{
	return (total / 4) > used;
}

size_t decreaseSpaceTwice(size_t totalSpace, size_t usedSpace, void *vec_p)
{
	return totalSpace / 2;
}

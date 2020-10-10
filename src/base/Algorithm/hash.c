
#include <zrlib/base/Algorithm/hash.h>

size_t zrhash_jenkins_one_at_a_time(char *key, size_t len)
{
	size_t hash, i;
	for (hash = i = 0; i < len; ++i)
	{
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return hash;
}

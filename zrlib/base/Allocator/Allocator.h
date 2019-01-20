/**
 * @author zuri
 * @date dimanche 20 janvier 2019, 18:08:52 (UTC+0100)
 */

typedef struct
{
	void* (*alloc)(size_t nbBytes);
	void* (*realloc)(void *allocated, size_t nbBytes);
	void* (*free)(void *allocated);
} ZRAllocator;

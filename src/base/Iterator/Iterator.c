/**
 * @author zuri
 * @date samedi 14 décembre 2019, 18:22:14 (UTC+0100)
 */

#include <zrlib/base/Iterator/Iterator.h>

void ZRIterator_destroy(ZRIterator *iterator)
{
	ZRITERATOR_DESTROY(iterator);
}

void* ZRIterator_current(ZRIterator *iterator)
{
	return ZRITERATOR_CURRENT(iterator);
}

void ZRIterator_next(ZRIterator *iterator)
{
	return ZRITERATOR_NEXT(iterator);
}

bool ZRIterator_hasNext(ZRIterator *iterator)
{
	return ZRITERATOR_HASNEXT(iterator);
}

// Help

static void* fcurrent(ZRIterator *iterator)
{
	return NULL ;
}

static void fvoid(ZRIterator *iterator)
{
}

static bool fhasNext(ZRIterator *iterator)
{
	return false;
}

ZRIterator* ZRIterator_emptyIterator(void)
{
	static ZRIteratorStrategy strategy = { //
		.fdestroy = fvoid, //
		.fcurrent = fcurrent, //
		.fnext = fvoid, //
		.fhasNext = fhasNext, //
		};
	static ZRIterator iterator = { //
		.strategy = &strategy, //
		};

	return &iterator;
}

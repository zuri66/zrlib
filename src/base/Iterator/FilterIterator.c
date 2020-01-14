/**
 * @author zuri
 * @date mardi 7 janvier 2020, 18:44:43 (UTC+0100)
 */

#include <zrlib/config.h>

#include <zrlib/base/Iterator/FilterIterator.h>

#include <assert.h>
#include <string.h>

// ============================================================================

#define ZRFILTERITERATOR_ITERATOR(FIT) (&(FIT)->iterator)
#define ZRFILTERITERATORSIZE(NBFILTERS) (sizeof(ZRFilterIterator) + (sizeof(ZRFilterIterator_fvalidate_t) * NBFILTERS))

typedef struct ZRFilterIteratorS ZRFilterIterator;

struct ZRFilterIteratorS
{
	ZRIterator iterator;
	ZRIteratorStrategy strategyArea;
	size_t nbFilters;
	void *data;
	ZRAllocator *allocator;
	ZRIterator *subject;
	unsigned hasNextComputed :2;
	ZRFilterIterator_fvalidate_t fvalidates[];
};

static bool fhasNext_OR(ZRIterator *iterator)
{
	ZRFilterIterator *const fiterator = (ZRFilterIterator*)iterator;

	switch (fiterator->hasNextComputed)
	{
	// Subject consumed
	case 0b10:
		return false;
	case 1:
		return true;
	case 0:
	{
		size_t const nbFilters = fiterator->nbFilters;
		void *const data = fiterator->data;
		ZRFilterIterator_fvalidate_t *const fvalidates = fiterator->fvalidates;

		for (;;)
		{
			if (!ZRIterator_hasNext(fiterator->subject))
			{
				fiterator->hasNextComputed = 0b10;
				return false;
			}
			ZRIterator_next(fiterator->subject);
			void *current = ZRIterator_current(fiterator->subject);

			for (size_t i = 0; i < nbFilters; i++)
			{
				if (fvalidates[i](current, data))
				{
					fiterator->hasNextComputed = 1;
					return true;
				}
			}
		}
	}
	}
	// Normally never happen
	return false;
}

static bool fhasNext_AND(ZRIterator *iterator)
{
	ZRFilterIterator *const fiterator = (ZRFilterIterator*)iterator;

	switch (fiterator->hasNextComputed)
	{
	// Subject consumed
	case 0b10:
		return false;
	case 1:
		return true;
	case 0:
	{
		size_t const nbFilters = fiterator->nbFilters;
		void *const data = fiterator->data;
		ZRFilterIterator_fvalidate_t *const fvalidates = fiterator->fvalidates;

		for (;;)
		{
			if (!ZRIterator_hasNext(fiterator->subject))
			{
				fiterator->hasNextComputed = 0b10;
				return false;
			}
			ZRIterator_next(fiterator->subject);
			void *current = ZRIterator_current(fiterator->subject);

			for (size_t i = 0; i < nbFilters; i++)
			{
				if (!fvalidates[i](current, data))
					goto END;
			}
			fiterator->hasNextComputed = 1;
			return true;
			END:
			;
		}
		break;
	}
	}
	// Normally never happen
	return false;
}

static void fnext(ZRIterator *iterator)
{
	ZRFilterIterator *const fiterator = (ZRFilterIterator*)iterator;

	switch (fiterator->hasNextComputed)
	{
	case 0b10:
		break;
	case 0:
		ZRITERATOR_HASNEXT((ZRIterator*)fiterator);
		fnext(ZRFILTERITERATOR_ITERATOR(fiterator));
		break;
	case 1:
		fiterator->hasNextComputed = 0;
		break;
	}
}

static void* fcurrent(ZRIterator *iterator)
{
	ZRFilterIterator *const fiterator = (ZRFilterIterator*)iterator;
	return ZRITERATOR_CURRENT(fiterator->subject);
}

static void fdestroy(ZRIterator *iterator)
{
	ZRFilterIterator *const fiterator = (ZRFilterIterator*)iterator;
	ZRITERATOR_DESTROY(fiterator->subject);
	ZRFREE(fiterator->allocator, fiterator);
}

ZRIterator* ZRFilterIterator_create(ZRIterator *subject, void *data, size_t nbFilters, ZRFilterIterator_fvalidate_t fvalidates[nbFilters], enum ZRFilterIteratorOpE op, ZRAllocator *allocator)
{
	assert(nbFilters > 0);
	ZRFilterIterator *it = ZRALLOC(allocator, ZRFILTERITERATORSIZE(nbFilters));
	*it = (ZRFilterIterator ) { //
		.iterator = { //
			.strategy = &it->strategyArea, //
			},//
		.data = data, //
		.nbFilters = nbFilters, //
		.subject = subject, //
		.allocator = allocator, //
		.hasNextComputed = 0, //
		};
	it->strategyArea = (ZRIteratorStrategy ) { //
		.fhasNext = (ZRIterator_fhasNext_t)(op == ZRFilterIteratorOp_AND ? fhasNext_AND : fhasNext_OR), //
		.fnext = (ZRIterator_fnext_t)fnext, //
		.fcurrent = (ZRIterator_fcurrent_t)fcurrent, //
		.fdestroy = (ZRIterator_fdestroy_t)fdestroy, //
		};
	memcpy(it->fvalidates, fvalidates, sizeof(*fvalidates) * nbFilters);
	return (ZRIterator*)it;
}

ZRIterator* ZRFilterIterator_createOr(ZRIterator *subject, void *data, size_t nbFilters, ZRFilterIterator_fvalidate_t fvalidates[nbFilters], ZRAllocator *allocator)
{
	return ZRFilterIterator_create(subject, data, nbFilters, fvalidates, ZRFilterIteratorOp_OR, allocator);
}

ZRIterator* ZRFilterIterator_createAnd(ZRIterator *subject, void *data, size_t nbFilters, ZRFilterIterator_fvalidate_t fvalidates[nbFilters], ZRAllocator *allocator)
{
	return ZRFilterIterator_create(subject, data, nbFilters, fvalidates, ZRFilterIteratorOp_AND, allocator);
}

ZRIterator* ZRFilterIterator_create1(ZRIterator *subject, void *data, ZRFilterIterator_fvalidate_t fvalidate, ZRAllocator *allocator)
{
	return ZRFilterIterator_create(subject, data, 1, &fvalidate, ZRFilterIteratorOp_OR, allocator);
}

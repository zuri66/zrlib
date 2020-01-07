/**
 * @author zuri
 * @date mardi 7 janvier 2020, 18:44:43 (UTC+0100)
 */

#include <zrlib/config.h>

#include <zrlib/base/Iterator/FilterIterator.h>

#include <assert.h>
#include <string.h>

// ============================================================================

#define FILTERITERATORHEAD_MEMBERS() \
	ZRITERATOR_MEMBERS(ZRIteratorStrategy); \
	size_t nbFilters; \
	void *data; \
	ZRAllocator *allocator; \
	ZRIterator *subject; \
	ZRIteratorStrategy strategyArea; \
	unsigned hasNextComputed : 2

struct ZRFilterIteratorHeadS
{
	FILTERITERATORHEAD_MEMBERS()
	;
};

#define STRUCT_FILTERITERATOR(NBFILTERS) STRUCT_FILTERITERATOR_NAME(, NBFILTERS)
#define STRUCT_FILTERITERATOR_NAME(NAME, NBFILTERS) \
struct NAME \
{ \
	FILTERITERATORHEAD_MEMBERS(); \
	ZRFilterIterator_fvalidate_t fvalidates[NBFILTERS]; \
}

static bool fhasNext_OR(struct ZRFilterIteratorHeadS *iterator)
{
	switch (iterator->hasNextComputed)
	{
	// Subject consumed
	case 0b10:
		return false;
	case 1:
		return true;
	case 0:
	{
		size_t const nbFilters = iterator->nbFilters;
		void *const data = iterator->data;
		typedef STRUCT_FILTERITERATOR(nbFilters)
		ZRFilterIterator;
		ZRFilterIterator_fvalidate_t *const fvalidates = ((ZRFilterIterator*)iterator)->fvalidates;

		for (;;)
		{
			if (!ZRIterator_hasNext(iterator->subject))
			{
				iterator->hasNextComputed = 0b10;
				return false;
			}
			ZRIterator_next(iterator->subject);
			void *current = ZRIterator_current(iterator->subject);

			for (size_t i = 0; i < nbFilters; i++)
			{
				if (fvalidates[i](current, data))
				{
					iterator->hasNextComputed = 1;
					return true;
				}
			}
		}
	}
	}
}

static bool fhasNext_AND(struct ZRFilterIteratorHeadS *iterator)
{
	switch (iterator->hasNextComputed)
	{
	// Subject consumed
	case 0b10:
		return false;
	case 1:
		return true;
	case 0:
	{
		size_t const nbFilters = iterator->nbFilters;
		void *const data = iterator->data;
		typedef STRUCT_FILTERITERATOR(nbFilters)
		ZRFilterIterator;
		ZRFilterIterator_fvalidate_t *const fvalidates = ((ZRFilterIterator*)iterator)->fvalidates;

		for (;;)
		{
			if (!ZRIterator_hasNext(iterator->subject))
			{
				iterator->hasNextComputed = 0b10;
				return false;
			}
			ZRIterator_next(iterator->subject);
			void *current = ZRIterator_current(iterator->subject);

			for (size_t i = 0; i < nbFilters; i++)
			{
				if (!fvalidates[i](current, data))
					goto END;
			}
			iterator->hasNextComputed = 1;
			return true;
			END:
			;
		}
		break;
	}
	}
}

static void fnext(struct ZRFilterIteratorHeadS *iterator)
{
	switch (iterator->hasNextComputed)
	{
	case 0b10:
		break;
	case 0:
		ZRITERATOR_HASNEXT((ZRIterator*)iterator);
		fnext(iterator);
		break;
	case 1:
		iterator->hasNextComputed = 0;
		break;
	}
}

static void* fcurrent(struct ZRFilterIteratorHeadS *iterator)
{
	return ZRITERATOR_CURRENT(iterator->subject);
}

static void fdestroy(struct ZRFilterIteratorHeadS *iterator)
{
	ZRITERATOR_DESTROY(iterator->subject);
	ZRFREE(iterator->allocator, iterator);
}

ZRIterator* ZRFilterIterator_create(ZRIterator *subject, void *data, size_t nbFilters, ZRFilterIterator_fvalidate_t fvalidates[nbFilters], enum ZRFilterIteratorOpE op, ZRAllocator *allocator)
{
	assert(nbFilters > 0);
	typedef STRUCT_FILTERITERATOR(nbFilters)
	ZRFilterIterator;
	ZRFilterIterator *it = ZRALLOC(allocator, sizeof(ZRFilterIterator));
	*((struct ZRFilterIteratorHeadS*)it) = (struct ZRFilterIteratorHeadS ) { //
		.data = data, //
		.nbFilters = nbFilters, //
		.subject = subject, //
		.strategy = &it->strategyArea, //
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

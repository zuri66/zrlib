/**
 * @author zuri
 * @date samedi 14 décembre 2019, 18:22:14 (UTC+0100)
 */

#ifndef ZRITERATOR_H
#define ZRITERATOR_H

#include <zrlib/config.h>
#include <zrlib/syntax_pad.h>

#include <stdlib.h>
#include <stdbool.h>

typedef struct ZRIteratorS ZRIterator;
typedef struct ZRIteratorStrategyS ZRIteratorStrategy;

// ============================================================================

typedef void __ (*ZRIterator_fdestroy_t)(ZRIterator*);

typedef void* _ (*ZRIterator_fcurrent_t)(ZRIterator*);
typedef void __ (*ZRIterator_fnext_t)(__ ZRIterator*);
typedef bool __ (*ZRIterator_fhasNext_t)(ZRIterator*);

#define ZRITERATORSTRATEGY_MEMBERS() \
	ZRIterator_fdestroy_t fdestroy; \
	ZRIterator_fcurrent_t fcurrent; \
	ZRIterator_fnext_t fnext; \
	ZRIterator_fhasNext_t fhasNext

struct ZRIteratorStrategyS
{
	ZRITERATORSTRATEGY_MEMBERS();
};

#define ZRITERATOR_MEMBERS(TYPE_STRATEGY) \
	TYPE_STRATEGY *strategy

struct ZRIteratorS
{
	ZRITERATOR_MEMBERS(ZRIteratorStrategy);
};

// ============================================================================

static inline void ZRITERATOR_DESTROY(ZRIterator *iterator)
{
	iterator->strategy->fdestroy(iterator);
}

static inline void* ZRITERATOR_CURRENT(ZRIterator *iterator)
{
	return iterator->strategy->fcurrent(iterator);
}

static inline void ZRITERATOR_NEXT(ZRIterator *iterator)
{
	iterator->strategy->fnext(iterator);
}

static inline bool ZRITERATOR_HASNEXT(ZRIterator *iterator)
{
	return iterator->strategy->fhasNext(iterator);
}

// ============================================================================

void __ ZRIterator_destroy(ZRIterator *iterator);

void* _ ZRIterator_current(ZRIterator *iterator);
void __ ZRIterator_next(__ ZRIterator *iterator);
bool __ ZRIterator_hasNext(ZRIterator *iterator);

// Help

ZRIterator* ZRIterator_emptyIterator(void);

#endif

/**
 * @author zuri
 * @date mardi 7 janvier 2020, 18:44:43 (UTC+0100)
 */

#ifndef ZRLIB_FILTERITERATOR_H
#define ZRLIB_FILTERITERATOR_H

#include <zrlib/base/Iterator/Iterator.h>
#include <zrlib/base/Allocator/Allocator.h>

#include <stdbool.h>

// ============================================================================

typedef bool (*ZRFilterIterator_fvalidate_t)(void *item, void *data);

enum ZRFilterIteratorOpE { ZRFilterIteratorOp_AND, ZRFilterIteratorOp_OR };

ZRIterator* ZRFilterIterator_create(____ ZRIterator *subject, void *data, size_t nbFilters, ZRFilterIterator_fvalidate_t fvalidates[nbFilters], enum ZRFilterIteratorOpE op, ZRAllocator *allocator);
ZRIterator* ZRFilterIterator_createOr(__ ZRIterator *subject, void *data, size_t nbFilters, ZRFilterIterator_fvalidate_t fvalidates[nbFilters], ____ ___________________ ___ ZRAllocator *allocator);
ZRIterator* ZRFilterIterator_createAnd(_ ZRIterator *subject, void *data, size_t nbFilters, ZRFilterIterator_fvalidate_t fvalidates[nbFilters], ____ ___________________ ___ ZRAllocator *allocator);
ZRIterator* ZRFilterIterator_create1(___ ZRIterator *subject, void *data, ______ __________ ZRFilterIterator_fvalidate_t fvalidate ___________, ____ ___________________ ___ ZRAllocator *allocator);

#endif /* ZRLIB_FILTERITERATOR_H_ */

/**
 * @author zuri
 * @date samedi 9 mai 2020, 16:38:08 (UTC+0200)
 */

#ifndef ZRSIMPLEGRAPH_H
#define ZRSIMPLEGRAPH_H

#include "Graph.h"

#include <zrlib/base/Allocator/Allocator.h>

ZRGraphBuilder* ZRSimpleGraphBuilder_create(
	size_t nodeObjSize, size_t nodeObjAlignment,
	size_t edgeObjSize, size_t edgeObjAlignment,
	ZRAllocator *allocator
	);

#endif

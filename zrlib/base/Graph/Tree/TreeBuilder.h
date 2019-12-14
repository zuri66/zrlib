/**
 * @author zuri
 * @date dimanche 1 décembre 2019, 15:06:14 (UTC+0100)
 */

#ifndef ZRTREE_H
#error __FILE__ " cannot be used outside Tree.h"
#endif

#ifndef ZRTREEBUILDER_H
#define ZRTREEBUILDER_H

#include <zrlib/syntax_pad.h>

#include <stdlib.h>

typedef struct ZRTreeBuilderS ZRTreeBuilder;
typedef struct ZRTreeBuilderStrategyS ZRTreeBuilderStrategy;
typedef void ZRTreeBuilderNode;

// ============================================================================

typedef ZRTreeBuilderNode* //
______________(*fcurrentNode_t)(_ ZRTreeBuilder *builder);
typedef void* (*fcurrentObj_t)(__ ZRTreeBuilder *builder);

typedef void __ (*fnode_t)(_ ZRTreeBuilder *builder, void *data);
typedef void __ (*fend_t)(__ ZRTreeBuilder *builder);
typedef ZRTree* (*fnew_t)(__ ZRTreeBuilder *builder);
typedef void __ (*fdone_t)(_ ZRTreeBuilder *builder);

#define ZRTREEBUILDERSTRATEGY_MEMBERS() \
	fcurrentNode_t fcurrentNode; \
	fcurrentObj_t fcurrentObj; \
	fnode_t fnode; \
	fend_t fend; \
	fnew_t fnew; \
	fdone_t fdone

struct ZRTreeBuilderStrategyS
{
	ZRTREEBUILDERSTRATEGY_MEMBERS()
	;
};

#define ZRTREEBUILDER_MEMBERS(TYPE_STRATEGY) \
	TYPE_STRATEGY *strategy

#define ZRTREEBUILDER_STRUCT(TYPE_STRATEGY) ZRTREEBUILDER_STRUCT_NAME(,TYPE_STRATEGY)
#define ZRTREEBUILDER_STRUCT_NAME(NAME, TYPE_STRATEGY) \
struct NAME \
{ \
	ZRTREEBUILDER_MEMBERS(TYPE_STRATEGY); \
}

ZRTREEBUILDER_STRUCT_NAME(ZRTreeBuilderS, ZRTreeBuilderStrategy);

// ============================================================================

ZRTreeBuilderNode* //
_______ ZRTreeBuilder_currentNode(_ ZRTreeBuilder *builder);
void* _ ZRTreeBuilder_currentObj(__ ZRTreeBuilder *builder);

void __ ZRTreeBuilder_node(_ ZRTreeBuilder *builder, void *data);
void __ ZRTreeBuilder_end(__ ZRTreeBuilder *builder);
ZRTree* ZRTreeBuilder_new(__ ZRTreeBuilder *builder);
void __ ZRTreeBuilder_done(_ ZRTreeBuilder *builder);

#endif

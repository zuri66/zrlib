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

//TODO ADD PREFIX for f#function

struct ZRTreeBuilderStrategyS
{
	ZRTreeBuilderNode* (*fcurrentNode)(ZRTreeBuilder *builder);
	void* (*fcurrentObj)(ZRTreeBuilder *builder);

	void __ (*fnode)(__ ZRTreeBuilder *builder, void *nodeData, void *edgeData);
	void __ (*fend)(___ ZRTreeBuilder *builder);
	ZRTree* (*fnew)(___ ZRTreeBuilder *builder);
	void __ (*fdone)(__ ZRTreeBuilder *builder);
	void __ (*fdestroy)(ZRTreeBuilder *builder);
};

struct ZRTreeBuilderS
{
	ZRTreeBuilderStrategy *strategy;
};

// ============================================================================

ZRTreeBuilderNode* //
_______ ZRTreeBuilder_currentNode(_ ZRTreeBuilder *builder);
void* _ ZRTreeBuilder_currentObj(__ ZRTreeBuilder *builder);

void __ ZRTreeBuilder_node(__ ZRTreeBuilder *builder, void *nodeData, void *edgeData);
void __ ZRTreeBuilder_end(___ ZRTreeBuilder *builder);
ZRTree* ZRTreeBuilder_new(___ ZRTreeBuilder *builder);
void __ ZRTreeBuilder_done(__ ZRTreeBuilder *builder);
void __ ZRTreeBuilder_destroy(ZRTreeBuilder *builder);

#endif

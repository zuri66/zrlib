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
typedef ZRTreeNode ZRTreeBuilderNode;

// ============================================================================

//TODO ADD PREFIX for f#function

struct ZRTreeBuilderStrategyS
{
	ZRTreeStrategy tree;
	ZRTreeBuilderNode* (*fcurrentNode)(ZRTreeBuilder *builder);

	void* (*fcurrentObj)(ZRTreeBuilder *builder);

	void __ (*fnode)(__ ZRTreeBuilder *builder, void *nodeData, void *edgeData);
	void __ (*fend)(___ ZRTreeBuilder *builder);
	ZRTree* (*fnew)(___ ZRTreeBuilder *builder);
};

#define ZRTBSTRATEGY(S) ((ZRTreeBuilderStrategy*)(S))
#define ZRTBSTRATEGY_T(TBS) (&(TBS)->tree)
#define ZRTBSTRATEGY_G(TBS) (&ZRTBSTRATEGY_T(TBS)->graph)

struct ZRTreeBuilderS
{
	ZRTree tree;
};

#define ZRTREEBUILDER(B) ((ZRTreeBuilder*)(B))
#define ZRTB_TREE(TB) (&(TB)->tree)
#define ZRTB_GRAPH(TB) ZRTREE_GRAPH(ZRTB_TREE(TB))
#define ZRTB_STRATEGY(TB) ((ZRTreeBuilderStrategy*)ZRTB_GRAPH(TB)->strategy)

// ============================================================================

ZRTreeBuilderNode* //
_______ ZRTreeBuilder_currentNode(_ ZRTreeBuilder *builder);
void* _ ZRTreeBuilder_currentObj(__ ZRTreeBuilder *builder);

void __ ZRTreeBuilder_node(__ ZRTreeBuilder *builder, void *nodeData, void *edgeData);
void __ ZRTreeBuilder_end(___ ZRTreeBuilder *builder);
ZRTree* ZRTreeBuilder_new(___ ZRTreeBuilder *builder);

// HELP

void ZRTreeBuilder_concatRootedTree(ZRTreeBuilder *builder, ZRTree *tree, ZRTreeNode *asRoot);
void ZRTreeBuilder_concatSubTree(ZRTreeBuilder *builder, ZRTree *tree, ZRTreeNode *node);
void ZRTreeBuilder_concatSubChilds(ZRTreeBuilder *builder, ZRTree *tree, ZRTreeNode *node);

#endif

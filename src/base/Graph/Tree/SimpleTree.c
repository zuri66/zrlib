/**
 * @author zuri
 * @date dimanche 1 décembre 2019, 15:36:56 (UTC+0100)
 */

#include <zrlib/base/Graph/Tree/Tree.h>
#include <zrlib/base/Vector/Vector2SideStrategy.h>

#include <assert.h>

#include "SimpleTree.h"

// ============================================================================
// STRATEGY FUNCTIONS
// ============================================================================

static size_t fstrategySize(void)
{
	return sizeof(ZRSimpleTreeStrategy);
}

static size_t fgetNNodes(ZRSimpleTree *tree, ZRTreeNode **nodes_out, size_t maxNbNodes)
{
	TYPEDEF_NODE_AUTO(tree);
	size_t const nbNodes = tree->nbNodes;
	ZRSimpleTreeNodeInstance *nodes = (ZRSimpleTreeNodeInstance*)tree->root;
	size_t const nb = nbNodes < maxNbNodes ? nbNodes : maxNbNodes;
	size_t i;

	for (i = 0; i < nb; i++, nodes++, nodes_out++)
		*nodes_out = (ZRTreeNode*)nodes;

	return nb;
}

static void fdone(ZRSimpleTree *tree)
{
	ZRVector2SideStrategy_destroy(tree->nodes);
	ZRFREE(tree->allocator, tree->strategy);
}

static void fdestroy(ZRSimpleTree *tree)
{
	ZRAllocator *allocator = tree->allocator;
	ZRVector2SideStrategy_destroy(tree->nodes);
	ZRFREE(allocator, tree->strategy);
	ZRFREE(allocator, tree);
}

// ============================================================================
// NODE
// ============================================================================

static ZRTreeNode* fNode_getObj(ZRTree *tree, ZRTreeNode *node)
{
	TYPEDEF_NODE_AUTO(tree);
	return ((ZRSimpleTreeNodeInstance*)node)->obj;
}

static ZRTreeNode* fNode_getTheParent(ZRTree *tree, ZRSimpleTreeNode *node)
{
	return node->parent;
}

static ZRTreeNode* fNode_getParent(ZRSimpleTree *tree, ZRSimpleTreeNode *node, size_t pos)
{
	TYPEDEF_NODE_AUTO(tree);
	ZRSimpleTreeNodeInstance *parents = (ZRSimpleTreeNodeInstance*)(node->parent);
	return (ZRTreeNode*)(parents + pos);
}

static ZRTreeNode* fNode_getChild(ZRSimpleTree *tree, ZRSimpleTreeNode *node, size_t pos)
{
	TYPEDEF_NODE_AUTO(tree);
	ZRSimpleTreeNodeInstance *childs = (ZRSimpleTreeNodeInstance*)(node->childs);
	return (ZRTreeNode*)(childs + pos);
}

static size_t fNode_getNbChilds(ZRSimpleTree *tree, ZRSimpleTreeNode *node)
{
	return node->nbChilds;
}

static size_t fNode_getNChilds(ZRSimpleTree *tree, ZRSimpleTreeNode *node, ZRTreeNode **nodes_out, size_t maxNbNodes)
{
	TYPEDEF_NODE_AUTO(tree);
	size_t const nbNodes = node->nbChilds;
	ZRSimpleTreeNodeInstance *nodes = (ZRSimpleTreeNodeInstance*)nodes;
	size_t const nb = nbNodes < maxNbNodes ? nbNodes : maxNbNodes;
	size_t i;

	for (i = 0; i < nb; i++, nodes++, nodes_out++)
		*nodes_out = (ZRTreeNode*)nodes;

	return nb;
}

// ============================================================================

static void ZRSimpleTreeStrategy_init(ZRSimpleTreeStrategy *strategy)
{
	*strategy = (ZRSimpleTreeStrategy ) { //
		.fstrategySize = fstrategySize, //
		.fNodeGetObj = (ZRGraphNode_fgetObj_t)fNode_getObj, //
		.fNodeGetTheParent = (ZRTreeNode_fgetTheParent_t)fNode_getTheParent, //
		.fNodeGetParent = (ZRGraphNode_fgetParent_t)fNode_getParent, //
		.fNodeGetChild = (ZRGraphNode_fgetChild_t)fNode_getChild, //
		.fNodeGetNbChilds = (ZRGraphNode_fgetNbChilds_t)fNode_getNbChilds, //
		.fNodeGetNChilds = (ZRGraphNode_fgetNChilds_t)fNode_getNChilds, //
		.fgetNNodes = (ZRGraph_fgetNNodes_t)fgetNNodes, //
		.fdone = (ZRGraph_fdone_t)fdone, //
//		.fdestroy = (ZRGraph_fdestroy_t)fdestroy, //
		};
}

ZRTree* ZRSimpleTree_alloc(size_t objSize, ZRAllocator *allocator)
{
	return ZRALLOC(allocator, sizeof(ZRSimpleTree));
}

ZRTree* ZRSimpleTree_create(size_t objSize, ZRAllocator *allocator, ZRVector *nodes)
{
	ZRSimpleTreeStrategy *strategy = ZRALLOC(allocator, sizeof(ZRSimpleTreeStrategy));
	ZRSimpleTreeStrategy_init(strategy);
	strategy->fdestroy = (ZRGraph_fdestroy_t)fdestroy;

	ZRSimpleTree *tree = (ZRSimpleTree*)ZRSimpleTree_alloc(objSize, allocator);
	*tree = (ZRSimpleTree ) { //
		.strategy = strategy, //
		.nodes = nodes, //
		.objSize = objSize, //
		.allocator = allocator, //
		};
	return (ZRTree*)tree;
}

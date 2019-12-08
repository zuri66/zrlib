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

static size_t fsdataSize(ZRTree *tree)
{
	return sizeof(ZRSimpleTreeData);
}

static size_t fstrategySize(void)
{
	return sizeof(ZRSimpleTreeStrategy);
}

static size_t fgetNbNodes(ZRTree *tree)
{
	return ZRVECTOR_NBOBJ(DATA(tree)->nodes);
}

static size_t fgetNNodes(ZRTree *tree, ZRTreeNode **nodes_out, size_t maxNbNodes)
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

static void fdone(ZRTree *tree)
{
	ZRVector2SideStrategy_destroy(DATA(tree)->nodes);
	ZRFREE(DATA(tree)->allocator, STRATEGY(tree));
}

static void fdestroy(ZRTree *tree)
{
	ZRSimpleTreeStrategy *strategy = STRATEGY(tree);
	ZRAllocator *allocator = DATA(tree)->allocator;
	ZRVector2SideStrategy_destroy(DATA(tree)->nodes);
	ZRFREE(allocator, tree);
	ZRFREE(allocator, strategy);
}

// ============================================================================
// NODE
// ============================================================================

static ZRTreeNode* fNode_getObj(ZRTree *tree, ZRTreeNode *node)
{
	TYPEDEF_NODE_AUTO(tree);
	return ((ZRSimpleTreeNodeInstance*)node)->obj;
}

static ZRTreeNode* fNode_getParent(ZRTree *tree, ZRSimpleTreeNode *node)
{
	return node->parent;
}

static ZRTreeNode* fNode_getChild(ZRTree *tree, ZRSimpleTreeNode *node, size_t pos)
{
	TYPEDEF_NODE_AUTO(tree);
	ZRSimpleTreeNodeInstance *childs = (ZRSimpleTreeNodeInstance*)(node->childs);
	return (ZRTreeNode*)(childs + pos);
}

static size_t fNode_getNbChilds(ZRTree *tree, ZRSimpleTreeNode *node)
{
	return node->nbChilds;
}

static size_t fNode_getNChilds(ZRTree *tree, ZRSimpleTreeNode *node, ZRTreeNode **nodes_out, size_t maxNbNodes)
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
		.fsdataSize = fsdataSize, //
		.fstrategySize = fstrategySize, //
		.fNodeGetObj = (ZRTreeNode_fgetObj_t)fNode_getObj, //
		.fNodeGetParent = (ZRTreeNode_fgetParent_t)fNode_getParent, //
		.fNodeGetChild = (ZRTreeNode_fgetChild_t)fNode_getChild, //
		.fNodeGetNbChilds = (ZRTreeNode_fgetNbChilds_t)fNode_getNbChilds, //
		.fNodeGetNChilds = (ZRTreeNode_fgetNChilds_t)fNode_getNChilds, //
		.fgetNbNodes = fgetNbNodes, //
		.fgetNNodes = fgetNNodes, //
		.fdone = fdone, //
		.fdestroy = fdestroy, //
		};
}

ZRTree* ZRSimpleTree_alloc(size_t objSize, ZRAllocator *allocator)
{
	return ZRALLOC(allocator, sizeof(ZRTree) + sizeof(ZRSimpleTreeData));
}

ZRTree* ZRSimpleTree_create(size_t objSize, ZRAllocator *allocator, ZRVector *nodes)
{
	ZRSimpleTreeStrategy *strategy = ZRALLOC(allocator, sizeof(ZRSimpleTreeStrategy));
	ZRSimpleTreeStrategy_init(strategy);
	strategy->fdestroy = fdestroy;

	ZRTree *tree = ZRSimpleTree_alloc(objSize, allocator);
	*tree = (ZRTree ) { //
		.strategy = (ZRTreeStrategy*)strategy, //
		};
	*DATA(tree) = (ZRSimpleTreeData ) { //
		.nodes = nodes, //
		.objSize = objSize, //
		.allocator = allocator, //
		};
	return tree;
}

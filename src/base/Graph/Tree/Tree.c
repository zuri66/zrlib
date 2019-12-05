/**
 * @author zuri
 * @date dimanche 1 décembre 2019, 15:06:14 (UTC+0100)
 */

#include <zrlib/base/Graph/Tree/Tree.h>

// ============================================================================
// BUILDER
// ============================================================================

void* ZRTreeBuilder_currentObj(ZRTreeBuilder *builder)
{
	return builder->strategy->fcurrentObj(builder);
}

void ZRTreeBuilder_node(ZRTreeBuilder *builder, void *data)
{
	builder->strategy->fnode(builder, data);
}

void ZRTreeBuilder_end(ZRTreeBuilder *builder)
{
	builder->strategy->fend(builder);
}

ZRTree* ZRTreeBuilder_new(ZRTreeBuilder *builder)
{
	return builder->strategy->fnew(builder);
}

void ZRTreeBuilder_done(ZRTreeBuilder *builder)
{
	builder->strategy->fdone(builder);
}

// ============================================================================
// TREE
// ============================================================================

ZRTreeNode* ZRTree_getRoot(ZRTree *tree)
{
	return tree->root;
}

void ZRTree_done(ZRTree *tree)
{
	tree->strategy->fdone(tree);
}

void ZRTree_destroy(ZRTree *tree)
{
	if (tree->strategy->fdestroy)
		tree->strategy->fdestroy(tree);
}

size_t ZRTree_getNbNodes(ZRTree *tree)
{
	return tree->strategy->fgetNbNodes(tree);
}

size_t ZRTree_getNNodes(ZRTree *tree, ZRTreeNode **nodes_out, size_t maxNbOut)
{
	return tree->strategy->fgetNNodes(tree, nodes_out, maxNbOut);
}

// ============================================================================
// NODE
// ============================================================================

void* ZRTreeNode_getObj(ZRTree *tree, ZRTreeNode *node)
{
	return tree->strategy->fNodeGetObj(tree, node);
}

ZRTreeNode* ZRTreeNode_getParent(ZRTree *tree, ZRTreeNode *node)
{
	return tree->strategy->fNodeGetParent(tree, node);
}

ZRTreeNode* ZRTreeNode_getChild(ZRTree *tree, ZRTreeNode *node, size_t pos)
{
	return tree->strategy->fNodeGetChild(tree, node, pos);
}

size_t ZRTreeNode_getNbChilds(ZRTree *tree, ZRTreeNode *node)
{
	return tree->strategy->fNodeGetNbChilds(tree, node);
}

size_t ZRTreeNode_getNChilds(ZRTree *tree, ZRTreeNode *node, ZRTreeNode **nodes_out, size_t maxNbOut)
{
	return tree->strategy->fNodeGetNChilds(tree, node, nodes_out, maxNbOut);
}

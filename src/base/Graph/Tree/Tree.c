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
	return ZRTREE_GETROOT(tree);
}

void ZRTree_done(ZRTree *tree)
{
	ZRTREE_DONE(tree);
}

void ZRTree_destroy(ZRTree *tree)
{
	ZRTREE_DESTROY(tree);
}

size_t ZRTree_getNbNodes(ZRTree *tree)
{
	return ZRTREE_GETNBNODES(tree);
}

size_t ZRTree_getNNodes(ZRTree *tree, ZRTreeNode **nodes_out, size_t offset, size_t maxNbOut)
{
	return ZRTREE_GETNNODES(tree, nodes_out, offset, maxNbOut);
}

size_t ZRTree_getNObjs(ZRTree *tree, void *objs_out, size_t offset, size_t maxNbOut)
{
	return ZRTREE_GETNOBJS(tree, objs_out, offset, maxNbOut);
}

// ============================================================================
// NODE
// ============================================================================

void* ZRTreeNode_getObj(ZRTree *tree, ZRTreeNode *node)
{
	return ZRTREENODE_GETOBJ(tree, node);
}

ZRTreeNode* ZRTreeNode_getParent(ZRTree *tree, ZRTreeNode *node, size_t pos)
{
	return ZRTREENODE_GETPARENT(tree, node, pos);
}

ZRTreeNode* ZRTreeNode_getTheParent(ZRTree *tree, ZRTreeNode *node)
{
	return ZRTREENODE_GETTHEPARENT(tree, node);
}

ZRTreeNode* ZRTreeNode_getChild(ZRTree *tree, ZRTreeNode *node, size_t pos)
{
	return ZRTREENODE_GETCHILD(tree, node, pos);
}

size_t ZRTreeNode_getNbChilds(ZRTree *tree, ZRTreeNode *node)
{
	return ZRTREENODE_GETNBCHILDS(tree, node);
}

size_t ZRTreeNode_getNChilds(ZRTree *tree, ZRTreeNode *node, ZRTreeNode **nodes_out, size_t offset, size_t maxNbOut)
{
	return ZRTREENODE_GETNCHILDS(tree, node, nodes_out, offset, maxNbOut);
}

size_t ZRTreeNode_getNObjs(ZRTree *tree, ZRTreeNode *node, void *objs_out, size_t offset, size_t maxNbOut)
{
	return ZRTREENODE_GETNOBJS(tree, node, objs_out, offset, maxNbOut);
}

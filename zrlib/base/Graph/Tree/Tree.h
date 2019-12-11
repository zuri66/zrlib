/**
 * @author zuri
 * @date dimanche 1 décembre 2019, 15:06:14 (UTC+0100)
 */

#ifndef ZRTREE_H
#define ZRTREE_H

#include "../Graph.h"

#include <zrlib/syntax_pad.h>

#include <stdlib.h>

// ============================================================================

typedef struct ZRTreeS ZRTree;
typedef struct ZRTreeStrategyS ZRTreeStrategy;

typedef char ZRTreeNode;
typedef struct ZRTreeEdgeS ZRTreeEdge;

#include "TreeBuilder.h"

// ============================================================================

typedef ZRTreeNode* (*ZRTreeNode_fgetTheParent_t)(ZRTree*, ZRTreeNode*);

#define ZRTREESTRATEGY_MEMBERS() \
	ZRGRAPHSTRATEGY_MEMBERS(); \
	ZRTreeNode_fgetTheParent_t fNodeGetTheParent;

struct ZRTreeStrategyS
{
	ZRTREESTRATEGY_MEMBERS()
};

#define ZRTREE_MEMBERS(TYPE_STRATEGY) \
	ZRGRAPH_MEMBERS(TYPE_STRATEGY); \
	ZRTreeNode *root

struct ZRTreeS
{
	ZRTREE_MEMBERS(ZRTreeStrategy)
	;
};

// ============================================================================

ZRTreeNode* ZRTree_getRoot(ZRTree *tree);

void ZRTree_done(__ ZRTree *tree);
void ZRTree_destroy(ZRTree *tree);

size_t ZRTree_getNbNodes(_ ZRTree *tree);
size_t ZRTree_getNNodes(__ ZRTree *tree, ZRTreeNode **nodes_out, size_t offset, size_t maxNbOut);
size_t ZRTree_getNObjs(___ ZRTree *tree, void *objs_out, size_t offset, size_t maxNbOut);

// ============================================================================
// NODE
// ============================================================================

void* _____ ZRTreeNode_getObj(______ ZRTree *tree, ZRTreeNode *node);
ZRTreeNode* ZRTreeNode_getTheParent(ZRTree *tree, ZRTreeNode *node);
ZRTreeNode* ZRTreeNode_getParent(___ ZRTree *tree, ZRTreeNode *node, size_t pos);
ZRTreeNode* ZRTreeNode_getChild(____ ZRTree *tree, ZRTreeNode *node, size_t pos);
size_t ____ ZRTreeNode_getNbChilds(_ ZRTree *tree, ZRTreeNode *node);
size_t ____ ZRTreeNode_getNChilds(__ ZRTree *tree, ZRTreeNode *node, ZRTreeNode **nodes_out, size_t offset, size_t maxNbOut);
size_t ____ ZRTreeNode_getNObjs(____ ZRTree *tree, ZRTreeNode *node, _______ void *objs_out, size_t offset, size_t maxNbOut);

// ============================================================================

// ============================================================================
// TREE
// ============================================================================

static inline void ZRTREE_DONE(ZRTree *tree)
{
	ZRGRAPH_DONE((ZRGraph*)tree);
}

static inline void ZRTREE_DESTROY(ZRTree *tree)
{
	ZRGRAPH_DESTROY((ZRGraph*)tree);
}

static inline size_t ZRTREE_GETNBNODES(ZRTree *tree)
{
	return ZRGRAPH_GETNBNODES((ZRGraph*)tree);
}

static inline size_t ZRTREE_GETNNODES(ZRTree *tree, ZRTreeNode **nodes_out, size_t offset, size_t maxNbOut)
{
	return ZRGRAPH_GETNNODES((ZRGraph*)tree, (ZRGraphNode**)nodes_out, offset, maxNbOut);
}

static inline size_t ZRTREE_GETNOBJS(ZRTree *tree, void *objs_out, size_t offset, size_t maxNbOutBytes)
{
	return ZRGRAPH_GETNOBJS((ZRGraph*)tree, objs_out, offset, maxNbOutBytes);
}

// ============================================================================
// EDGE
// ============================================================================

static inline void* ZRTREEEDGE_GETOBJ(ZRTree *tree, ZRGraphNode *a, ZRGraphNode *b)
{
	return ZRGRAPHEDGE_GETOBJ((ZRGraph*)tree, a, b);
}

// ============================================================================
// NODE
// ============================================================================

static inline void* ZRTREENODE_GETOBJ(ZRTree *tree, ZRTreeNode *node)
{
	return ZRGRAPHNODE_GETOBJ((ZRGraph*)tree, (ZRGraphNode*)node);
}

static inline ZRTreeNode* ZRTREENODE_GETTHEPARENT(ZRTree *tree, ZRTreeNode *node)
{
	return tree->strategy->fNodeGetTheParent(tree, node);
}

static inline ZRTreeNode* ZRTREENODE_GETPARENT(ZRTree *tree, ZRTreeNode *node, size_t pos)
{
	return (ZRTreeNode*)ZRGRAPHNODE_GETPARENT((ZRGraph*)tree, (ZRGraphNode*)node, pos);
}

static inline ZRTreeNode* ZRTREENODE_GETCHILD(ZRTree *tree, ZRTreeNode *node, size_t pos)
{
	return (ZRTreeNode*)ZRGRAPHNODE_GETCHILD((ZRGraph*)tree, (ZRGraphNode*)node, pos);
}

static inline size_t ZRTREENODE_GETNBPARENTS(ZRTree *tree, ZRTreeNode *node)
{
	return ZRGRAPHNODE_GETNBPARENTS((ZRGraph*)tree, (ZRGraphNode*)node);
}

static inline size_t ZRTREENODE_GETNBCHILDS(ZRTree *tree, ZRTreeNode *node)
{
	return ZRGRAPHNODE_GETNBCHILDS((ZRGraph*)tree, (ZRGraphNode*)node);
}

static inline size_t ZRTREENODE_GETNPARENTS(ZRTree *tree, ZRTreeNode *node, ZRTreeNode **nodes_out, size_t offset, size_t maxNbOut)
{
	return ZRGRAPHNODE_GETNPARENTS((ZRGraph*)tree, (ZRGraphNode*)node, (ZRGraphNode**)nodes_out, offset, maxNbOut);
}

static inline size_t ZRTREENODE_GETNCHILDS(ZRTree *tree, ZRTreeNode *node, ZRTreeNode **nodes_out, size_t offset, size_t maxNbOut)
{
	return ZRGRAPHNODE_GETNCHILDS((ZRGraph*)tree, (ZRGraphNode*)node, (ZRGraphNode**)nodes_out, offset, maxNbOut);
}

static inline size_t ZRTREENODE_GETNOBJS(ZRTree *tree, ZRTreeNode *node, void *objs_out, size_t offset, size_t maxNbOut)
{
	return ZRGRAPHNODE_GETNCHILDS((ZRGraph*)tree, (ZRGraphNode*)node, (ZRGraphNode**)objs_out, offset, maxNbOut);
}

#endif

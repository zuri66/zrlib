/**
 * @author zuri
 * @date dimanche 1 décembre 2019, 15:06:14 (UTC+0100)
 */

#ifndef ZRTREE_H
#define ZRTREE_H

#include <zrlib/config.h>
#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/Iterator/Iterator.h>
#include <zrlib/syntax_pad.h>

#include "../Graph.h"

#include <stdlib.h>

// ============================================================================

typedef struct ZRTreeS ZRTree;
typedef struct ZRTreeStrategyS ZRTreeStrategy;

typedef char ZRTreeNode;
typedef struct ZRTreeEdgeS ZRTreeEdge;

#include "TreeBuilder.h"

#define ZRTREE_GRAPH(TREE)         (&(TREE)->graph)
#define ZRTREE_STRATEGY(TREE)      ((ZRTreeStrategy*) (ZRTREE_GRAPH(TREE)->strategy))
#define ZRTREE_GRAPHSTRATEGY(TREE) ((ZRGraphStrategy*)(ZRTREE_GRAPH(TREE)->strategy))

// ============================================================================

typedef ZRTreeBuilder* (*ZRTree_fnewTreeBuilder_t)(ZRTree*, ZRTreeNode*);
typedef ZRTreeNode* _(*ZRTreeNode_fgetTheParent_t)(ZRTree*, ZRTreeNode*);

typedef ZRIterator* (*ZRTreeNode_fgetChilds_t)(_________ ZRTree*, ZRTreeNode*);
typedef ZRIterator* (*ZRTreeNode_fgetAscendants_t)(_____ ZRTree*, ZRTreeNode*);
typedef ZRIterator* (*ZRTreeNode_fgetDescendants_t)(____ ZRTree*, ZRTreeNode*);
typedef ZRIterator* (*ZRTreeNode_fgetDescendants_BF_t)(_ ZRTree*, ZRTreeNode*);
typedef ZRIterator* (*ZRTreeNode_fgetDescendants_DF_t)(_ ZRTree*, ZRTreeNode*);

struct ZRTreeStrategyS
{
	ZRGraphStrategy graph;
	ZRTree_fnewTreeBuilder_t fnewTreeBuilder;
	ZRTreeNode_fgetTheParent_t fNodeGetTheParent;
	ZRTreeNode_fgetChilds_t fNodeGetChilds;
	ZRTreeNode_fgetAscendants_t fNodeGetAscendants;
	ZRTreeNode_fgetDescendants_t fNodeGetDescendants;
	ZRTreeNode_fgetDescendants_BF_t fNodeGetDescendants_BF;
	ZRTreeNode_fgetDescendants_DF_t fNodeGetDescendants_DF;
};

struct ZRTreeS
{
	ZRGraph graph;
	ZRTreeNode *root;
};

// ============================================================================

ZRTreeNode* ZRTree_getRoot(ZRTree *tree);

ZRTreeBuilder* ZRTree_newBuilder(ZRTree *tree, ZRTreeNode *currentBuilderNode);

void ZRTree_done(__ ZRTree *tree);
void ZRTree_destroy(ZRTree *tree);

size_t ZRTree_getNbNodes(_ ZRTree *tree);
size_t ZRTree_getNNodes(__ ZRTree *tree, ZRTreeNode **nodes_out, size_t offset, size_t maxNbOut);
size_t ZRTree_getNObjs(___ ZRTree *tree, void *objs_out, size_t offset, size_t maxNbOut);

// ============================================================================
// NODE
// ============================================================================

void* _____ ZRTreeNode_getObj(_______ ZRTree *tree, ZRTreeNode *node);
ZRTreeNode* ZRTreeNode_getTheParent(_ ZRTree *tree, ZRTreeNode *node);
ZRTreeNode* ZRTreeNode_getParent(____ ZRTree *tree, ZRTreeNode *node, size_t pos);
ZRTreeNode* ZRTreeNode_getChild(_____ ZRTree *tree, ZRTreeNode *node, size_t pos);
size_t ____ ZRTreeNode_getNbChilds(__ ZRTree *tree, ZRTreeNode *node);
size_t ____ ZRTreeNode_getNChilds(___ ZRTree *tree, ZRTreeNode *node, ZRTreeNode **nodes_out, size_t offset, size_t maxNbOut);
size_t ____ ZRTreeNode_getNObjs(_____ ZRTree *tree, ZRTreeNode *node, _______ void *objs_out, size_t offset, size_t maxNbOut);

ZRIterator* ZRTreeNode_getChilds(______ ZRTree *tree, ZRTreeNode *node);
ZRIterator* ZRTreeNode_getAscendants(__ ZRTree *tree, ZRTreeNode *node);
ZRIterator* ZRTreeNode_getDescendants(_ ZRTree *tree, ZRTreeNode *node);

/* Breadth first */
ZRIterator* ZRTreeNode_getDescendants_BF(ZRTree *tree, ZRTreeNode *node);

/* Depth first */
ZRIterator* ZRTreeNode_getDescendants_DF(ZRTree *tree, ZRTreeNode *node);

// ------
// Help
// ------

#define ZRTREENODE_FROMARRAYCOORDINATE(tree,array) ZRTreeNode_getNodeFromCoordinate(tree, ZRCARRAY_NBOBJ(array), array)
ZRTreeNode* ZRTreeNode_getNodeFromCoordinate(ZRTree *tree, size_t nb, size_t coord[nb]);

// -----
// Std
// -----

ZRIterator* ZRTreeNode_std_getDescendants_BF(ZRTree *tree, ZRTreeNode *node, ZRAllocator *allocator);
ZRIterator* ZRTreeNode_std_getDescendants_DF(ZRTree *tree, ZRTreeNode *node, ZRAllocator *allocator);

// ============================================================================
// TREE
// ============================================================================

static inline ZRTreeNode* ZRTREE_GETROOT(ZRTree *tree)
{
	return tree->root;
}

static inline ZRTreeBuilder* ZRTREE_NEWBUILDER(ZRTree *tree, ZRTreeNode *currentBuilderNode)
{
	return ZRTREE_STRATEGY(tree)->fnewTreeBuilder(tree, currentBuilderNode);
}

static inline void ZRTREE_DONE(ZRTree *tree)
{
	ZRGRAPH_DONE(ZRTREE_GRAPH(tree));
}

static inline void ZRTREE_DESTROY(ZRTree *tree)
{
	ZRGRAPH_DESTROY(ZRTREE_GRAPH(tree));
}

static inline size_t ZRTREE_GETNBNODES(ZRTree *tree)
{
	return ZRGRAPH_GETNBNODES(ZRTREE_GRAPH(tree));
}

static inline size_t ZRTREE_GETNNODES(ZRTree *tree, ZRTreeNode **nodes_out, size_t offset, size_t maxNbOut)
{
	return ZRGRAPH_GETNNODES(ZRTREE_GRAPH(tree), (ZRGraphNode**)nodes_out, offset, maxNbOut);
}

static inline size_t ZRTREE_GETNOBJS(ZRTree *tree, void *objs_out, size_t offset, size_t maxNbOutBytes)
{
	return ZRGRAPH_GETNOBJS(ZRTREE_GRAPH(tree), objs_out, offset, maxNbOutBytes);
}

static inline ZRIterator* ZRTREENODE_GETCHILDS(ZRTree *tree, ZRTreeNode *node)
{
	return ZRTREE_STRATEGY(tree)->fNodeGetChilds(tree, node);
}

static inline ZRIterator* ZRTREENODE_GETASCENDANTS(ZRTree *tree, ZRTreeNode *node)
{
	return ZRTREE_STRATEGY(tree)->fNodeGetAscendants(tree, node);
}

static inline ZRIterator* ZRTREENODE_GETDESCENDANTS(ZRTree *tree, ZRTreeNode *node)
{
	return ZRTREE_STRATEGY(tree)->fNodeGetDescendants(tree, node);
}

static inline ZRIterator* ZRTREENODE_GETDESCENDANTS_BF(ZRTree *tree, ZRTreeNode *node)
{
	return ZRTREE_STRATEGY(tree)->fNodeGetDescendants_BF(tree, node);
}

static inline ZRIterator* ZRTREENODE_GETDESCENDANTS_DF(ZRTree *tree, ZRTreeNode *node)
{
	return ZRTREE_STRATEGY(tree)->fNodeGetDescendants_DF(tree, node);
}

// ============================================================================
// EDGE
// ============================================================================

static inline void* ZRTREEEDGE_GETOBJ(ZRTree *tree, ZRGraphNode *a, ZRGraphNode *b)
{
	return ZRGRAPHEDGE_GETOBJ(ZRTREE_GRAPH(tree), a, b);
}

// ============================================================================
// NODE
// ============================================================================

static inline void* ZRTREENODE_GETOBJ(ZRTree *tree, ZRTreeNode *node)
{
	return ZRGRAPHNODE_GETOBJ(ZRTREE_GRAPH(tree), (ZRGraphNode*)node);
}

static inline ZRTreeNode* ZRTREENODE_GETTHEPARENT(ZRTree *tree, ZRTreeNode *node)
{
	return ZRTREE_STRATEGY(tree)->fNodeGetTheParent(tree, node);
}

static inline ZRTreeNode* ZRTREENODE_GETPARENT(ZRTree *tree, ZRTreeNode *node, size_t pos)
{
	return (ZRTreeNode*)ZRGRAPHNODE_GETPARENT(ZRTREE_GRAPH(tree), (ZRGraphNode*)node, pos);
}

static inline ZRTreeNode* ZRTREENODE_GETCHILD(ZRTree *tree, ZRTreeNode *node, size_t pos)
{
	return (ZRTreeNode*)ZRGRAPHNODE_GETCHILD(ZRTREE_GRAPH(tree), (ZRGraphNode*)node, pos);
}

static inline size_t ZRTREENODE_GETNBPARENTS(ZRTree *tree, ZRTreeNode *node)
{
	return ZRGRAPHNODE_GETNBPARENTS(ZRTREE_GRAPH(tree), (ZRGraphNode*)node);
}

static inline size_t ZRTREENODE_GETNBCHILDS(ZRTree *tree, ZRTreeNode *node)
{
	return ZRGRAPHNODE_GETNBCHILDS(ZRTREE_GRAPH(tree), (ZRGraphNode*)node);
}

static inline size_t ZRTREENODE_GETNPARENTS(ZRTree *tree, ZRTreeNode *node, ZRTreeNode **nodes_out, size_t offset, size_t maxNbOut)
{
	return ZRGRAPHNODE_GETNPARENTS(ZRTREE_GRAPH(tree), (ZRGraphNode*)node, (ZRGraphNode**)nodes_out, offset, maxNbOut);
}

static inline size_t ZRTREENODE_GETNCHILDS(ZRTree *tree, ZRTreeNode *node, ZRTreeNode **nodes_out, size_t offset, size_t maxNbOut)
{
	return ZRGRAPHNODE_GETNCHILDS(ZRTREE_GRAPH(tree), (ZRGraphNode*)node, (ZRGraphNode**)nodes_out, offset, maxNbOut);
}

static inline size_t ZRTREENODE_GETNOBJS(ZRTree *tree, ZRTreeNode *node, void *objs_out, size_t offset, size_t maxNbOut)
{
	return ZRGRAPHNODE_GETNCHILDS(ZRTREE_GRAPH(tree), (ZRGraphNode*)node, (ZRGraphNode**)objs_out, offset, maxNbOut);
}

#endif

/**
 * @author zuri
 * @date dimanche 1 décembre 2019, 15:06:14 (UTC+0100)
 */

#ifndef ZRTREE_H
#define ZRTREE_H

#include <zrlib/syntax_pad.h>

#include <stdlib.h>

// ============================================================================

typedef struct ZRTreeS ZRTree;
typedef struct ZRTreeStrategyS ZRTreeStrategy;

typedef char ZRTreeNode;
typedef struct ZRTreeEdgeS ZRTreeEdge;

#include "TreeBuilder.h"

// ============================================================================

typedef void* _____ (*ZRTreeNode_fgetObj_t)(______ ZRTree*, ZRTreeNode*);
typedef ZRTreeNode* (*ZRTreeNode_fgetParent_t)(___ ZRTree*, ZRTreeNode*);
typedef ZRTreeNode* (*ZRTreeNode_fgetChild_t)(____ ZRTree*, ZRTreeNode*, size_t pos);
typedef size_t ____ (*ZRTreeNode_fgetNbChilds_t)(_ ZRTree*, ZRTreeNode*);
typedef size_t ____ (*ZRTreeNode_fgetNChilds_t)(__ ZRTree*, ZRTreeNode*, ZRTreeNode **nodes_out, size_t maxNbOut);

typedef size_t (*ZRTree_fgetNbNodes_t)(_ ZRTree*);
typedef size_t (*ZRTree_fgetNNodes_t)(__ ZRTree*, ZRTreeNode **nodes_out, size_t maxNbOut);
typedef void _ (*ZRTree_fdone_t)(__ ZRTree*);
typedef void _ (*ZRTree_fdestroy_t)(ZRTree*);

#define ZRTREESTRATEGY_MEMBERS() \
	size_t (*fsdataSize)(ZRTree *tree); \
	size_t (*fstrategySize)(void); \
	\
	ZRTreeNode_fgetObj_t fNodeGetObj; \
	ZRTreeNode_fgetParent_t fNodeGetParent; \
	ZRTreeNode_fgetChild_t fNodeGetChild; \
	ZRTreeNode_fgetNbChilds_t fNodeGetNbChilds; \
	ZRTreeNode_fgetNChilds_t fNodeGetNChilds; \
	\
	ZRTree_fgetNbNodes_t fgetNbNodes; \
	ZRTree_fgetNNodes_t fgetNNodes; \
	ZRTree_fdone_t fdone; \
	\
	/** @Optional */ \
	ZRTree_fdestroy_t fdestroy

struct ZRTreeStrategyS
{
	ZRTREESTRATEGY_MEMBERS();
};

struct ZRTreeS
{
	size_t nbNodes;

	ZRTreeNode *root;

	ZRTreeStrategy *strategy;

	char sdata[];
};

// ============================================================================

ZRTreeNode* ZRTree_getRoot(ZRTree *tree);

void ZRTree_done(__ ZRTree *tree);
void ZRTree_destroy(ZRTree *tree);

size_t ZRTree_getNbNodes(_ ZRTree *tree);
size_t ZRTree_getNNodes(__ ZRTree *tree, ZRTreeNode **nodes_out, size_t maxNbOut);

// ============================================================================
// NODE
// ============================================================================

void* _____ ZRTreeNode_getObj(______ ZRTree *tree, ZRTreeNode *node);
ZRTreeNode* ZRTreeNode_getParent(___ ZRTree *tree, ZRTreeNode *node);
ZRTreeNode* ZRTreeNode_getChild(____ ZRTree *tree, ZRTreeNode *node, size_t pos);
size_t ____ ZRTreeNode_getNbChilds(_ ZRTree *tree, ZRTreeNode *node);
size_t ____ ZRTreeNode_getNChilds(__ ZRTree *tree, ZRTreeNode *node, ZRTreeNode **nodes_out, size_t maxNbOut);

#endif

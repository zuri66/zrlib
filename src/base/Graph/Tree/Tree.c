/**
 * @author zuri
 * @date dimanche 1 décembre 2019, 15:06:14 (UTC+0100)
 */

#include <zrlib/config.h>

#include <zrlib/base/Graph/Tree/Tree.h>
#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/Vector/Vector2SideStrategy.h>

#include <assert.h>

// ============================================================================
// BUILDER
// ============================================================================

void* ZRTreeBuilder_currentObj(ZRTreeBuilder *builder)
{
	return ZRTB_STRATEGY(builder)->fcurrentObj(builder);
}

void ZRTreeBuilder_node(ZRTreeBuilder *builder, void *nodeData, void *edgeData)
{
	ZRTB_STRATEGY(builder)->fnode(builder, nodeData, edgeData);
}

void ZRTreeBuilder_end(ZRTreeBuilder *builder)
{
	ZRTB_STRATEGY(builder)->fend(builder);
}

ZRTree* ZRTreeBuilder_new(ZRTreeBuilder *builder)
{
	return ZRTB_STRATEGY(builder)->fnew(builder);
}

void ZRTreeBuilder_done(ZRTreeBuilder *builder)
{
	ZRTB_STRATEGY(builder)->fdone(builder);
}

void ZRTreeBuilder_destroy(ZRTreeBuilder *builder)
{
	ZRTB_STRATEGY(builder)->fdestroy(builder);
}

// HELP

/**
 * Concat the tree located by *asRoot in *tree.
 * *asRoot is considered to be the root of *tree, but has not to be.
 * All *node of *tree can be *asRoot.
 * The construction make sure that all *tree is added to the builder as if *asRoot was the root node.
 */
void ZRTreeBuilder_concatRootedTree(ZRTreeBuilder *builder, ZRTree *tree, ZRTreeNode *asRoot)
{
	ZRTreeEdge edge;
	ZRTreeNode *last = asRoot;
	ZRTreeNode *parent = ZRTREENODE_GETTHEPARENT(tree, asRoot);
	size_t nbParents = ZRTREENODE_GETNBASCENDANTS(tree, asRoot);

	ZRTreeBuilder_concatSubTree(builder, tree, asRoot);

	while (parent != NULL)
	{
		ZRTREENODE_CPYTHEPARENTEDGE(tree, last, &edge);
		size_t const nbChilds = ZRGRAPHNODE_GETNBCHILDS(ZRTREE_GRAPH(tree), parent);

		// Parent become a child
		ZRTreeBuilder_node(builder, ZRGRAPHNODE_GETOBJ(parent), edge.obj);

		// Add the childs of the parent but not the last node
		for (size_t i = 0; i < nbChilds; i++)
		{
			ZRTreeNode *const child = ZRGRAPHNODE_GETCHILD(ZRTREE_GRAPH(tree), parent, i);

			if (child == last)
				continue;

			ZRTREENODE_CPYTHEPARENTEDGE(tree, child, &edge);
			ZRTreeBuilder_concatSubTree(builder, tree, child);
			ZRTreeBuilder_end(builder);
		}
		last = parent;
		parent = ZRTREENODE_GETTHEPARENT(tree, parent);
	}

	// Get up to the root
	while(nbParents --)
		ZRTreeBuilder_end(builder);
}

/**
 * Concat the sub-tree located at *node inside *tree.
 * End the stack on *node.
 */
void ZRTreeBuilder_concatSubTree(ZRTreeBuilder *builder, ZRTree *tree, ZRTreeNode *node)
{
	ZRGraphEdge edge;
	ZRTREENODE_CPYTHEPARENTEDGE(tree, node, &edge);
	ZRTreeBuilder_node(builder, ZRGRAPHNODE_GETOBJ(node), edge.obj);

	size_t i = 0;
	size_t const c = ZRGRAPHNODE_GETNBCHILDS(ZRTREE_GRAPH(tree), node);

	for (; i < c; i++)
	{
		ZRTreeBuilder_concatSubTree(builder, tree, ZRGRAPHNODE_GETCHILD(ZRTREE_GRAPH(tree), node, i));
		ZRTreeBuilder_end(builder);
	}
}

void ZRTreeBuilder_concatSubChilds(ZRTreeBuilder *builder, ZRTree *tree, ZRTreeNode *node)
{
	size_t i = 0;
	size_t const c = ZRGRAPHNODE_GETNBCHILDS(ZRTREE_GRAPH(tree), node);

	for (; i < c; i++)
	{
		ZRTreeBuilder_concatSubTree(builder, tree, ZRGRAPHNODE_GETCHILD(ZRTREE_GRAPH(tree), node, i));
		ZRTreeBuilder_end(builder);
	}
}

// ============================================================================
// TREE
// ============================================================================

ZRTreeNode* ZRTree_getRoot(ZRTree *tree)
{
	return ZRTREE_GETROOT(tree);
}

void ZRTree_changeRoot(ZRTree *tree, ZRTreeNode *newRoot)
{
	return ZRTREE_CHANGEROOT(tree, newRoot);
}

ZRTreeBuilder* ZRTree_newBuilder(ZRTree *tree, ZRTreeNode *currentBuilderNode)
{
	return ZRTREE_NEWBUILDER(tree, currentBuilderNode);
}

// ============================================================================
// NODE
// ============================================================================

ZRTreeNode* ZRTreeNode_getTheParent(ZRTree *tree, ZRTreeNode *node)
{
	return ZRTREENODE_GETTHEPARENT(tree, node);
}

void ZRTreeNode_cpyTheParentEdge(ZRTree *tree, ZRTreeNode *node, ZRTreeEdge *edge)
{
	ZRTREENODE_CPYTHEPARENTEDGE(tree, node, edge);
}

size_t ZRTreeNode_getNbAscendants(ZRTree *tree, ZRTreeNode *node)
{
	return ZRTREENODE_GETNBASCENDANTS(tree, node);
}

size_t ZRTreeNode_getNbDescendants(ZRTree *tree, ZRTreeNode *node)
{
	return ZRTREENODE_GETNBDESCENDANTS(tree, node);
}

ZRIterator* ZRTreeNode_getChilds(ZRTree *tree, ZRTreeNode *node)
{
	return ZRTREENODE_GETCHILDS(tree, node);
}

ZRIterator* ZRTreeNode_getAscendants(ZRTree *tree, ZRTreeNode *node)
{
	return ZRTREENODE_GETASCENDANTS(tree, node);
}

ZRIterator* ZRTreeNode_getDescendants(ZRTree *tree, ZRTreeNode *node)
{
	return ZRTREENODE_GETDESCENDANTS(tree, node);
}

ZRIterator* ZRTreeNode_getDescendants_BF(ZRTree *tree, ZRTreeNode *node)
{
	return ZRTREENODE_GETDESCENDANTS_BF(tree, node);
}

ZRIterator* ZRTreeNode_getDescendants_DF(ZRTree *tree, ZRTreeNode *node)
{
	return ZRTREENODE_GETDESCENDANTS_DF(tree, node);
}

// Help

ZRTreeNode* ZRTreeNode_getNodeFromCoordinate(ZRTree *tree, size_t nb, size_t coord[nb])
{
	ZRTreeNode *current = tree->root;

	for (size_t i = 0; i < nb; i++)
	{
		current = ZRGRAPHNODE_GETCHILD(&tree->graph, current, coord[i]);

		if (current == NULL)
			return NULL ;
	}
	return current;
}

// ============================================================================
// Standard implementations
// ============================================================================

#define ZRTREEBFNODEITERATOR_ITERATOR(BFIT) &((BFIT)->iterator)
#define ZRTREEDFNODEITERATOR_ITERATOR(DFIT) &((DFIT)->iterator)

typedef struct
{
	ZRIterator iterator;
	ZRAllocator *allocator;
	union
	{
		ZRTree *subject;
		ZRGraph *subject_g;
	};
	union
	{
		ZRTreeNode *current;
		ZRGraphNode *current_g;
	};
	union
	{
		ZRVector *queue;
		ZRVector *stack;
	};
	ZRIteratorStrategy strategyArea;
} BFNodeIterator;

typedef BFNodeIterator DFNodeIterator;

static void BFNodeIterator_fdestroy(ZRIterator *iterator)
{
	BFNodeIterator *const bfiterator = (BFNodeIterator*)iterator;
	ZRVector_destroy(bfiterator->queue);
	ZRFREE(bfiterator->allocator, bfiterator);
}

static void* BFNodeIterator_fcurrent(ZRIterator *iterator)
{
	BFNodeIterator *const bfiterator = (BFNodeIterator*)iterator;
	return bfiterator->current;
}

static bool BFIterator_fhasNext(ZRIterator *iterator)
{
	BFNodeIterator *const bfiterator = (BFNodeIterator*)iterator;
	return ZRVECTOR_NBOBJ(bfiterator->queue) > 0;
}

static void BFIterator_fnext(ZRIterator *iterator)
{
	BFNodeIterator *const bfiterator = (BFNodeIterator*)iterator;
	assert(BFIterator_fhasNext(ZRTREEBFNODEITERATOR_ITERATOR(bfiterator)));
	ZRVECTOR_POPFIRST(bfiterator->queue, &bfiterator->current);
	size_t const nbChilds = ZRGRAPHNODE_GETNBCHILDS(bfiterator->subject_g, bfiterator->current_g);

	if (nbChilds > 0)
	{
		ZRGraphNode (*childs_g[nbChilds]);
		ZRGraphNode_getNChilds(bfiterator->subject_g, bfiterator->current_g, childs_g, 0, nbChilds);
		ZRVECTOR_ADD_NB(bfiterator->queue, nbChilds, childs_g);
	}
}

ZRIterator* ZRTreeNode_std_getDescendants_BF(ZRTree *tree, ZRTreeNode *node, ZRAllocator *allocator)
{
	BFNodeIterator *ret = ZRALLOC(allocator, sizeof(BFNodeIterator));
	*ret = (BFNodeIterator ) { //
		.iterator = (ZRIterator ) { //
			.strategy = &ret->strategyArea, //
			},//
		.subject = tree, //
		.allocator = allocator, //
		.queue = ZRVector2SideStrategy_createDynamic(256, ZRTYPE_SIZE_ALIGNMENT(void*), allocator), //
		};
	ret->strategyArea = (ZRIteratorStrategy ) { //
		.fdestroy = BFNodeIterator_fdestroy, //
		.fcurrent = BFNodeIterator_fcurrent, //
		.fhasNext = BFIterator_fhasNext, //
		.fnext = BFIterator_fnext, //
		};
	ZRVECTOR_ADD(ret->queue, &node);
	return ZRTREEBFNODEITERATOR_ITERATOR(ret);
}

static void DFIterator_fnext(ZRIterator *iterator)
{
	DFNodeIterator *const dfiterator = (DFNodeIterator*)iterator;
	assert(BFIterator_fhasNext(ZRTREEDFNODEITERATOR_ITERATOR(dfiterator)));
	ZRVECTOR_POP(dfiterator->queue, &dfiterator->current);
	size_t const nbChilds = ZRGRAPHNODE_GETNBCHILDS(dfiterator->subject_g, dfiterator->current_g);

	if (nbChilds > 0)
	{
		ZRGraphNode (*childs_g[nbChilds]);
		ZRGraphNode_getNChilds(dfiterator->subject_g, dfiterator->current_g, childs_g, 0, nbChilds);
		// Reverse to respect the order of the childs in the stack
		ZRARRAYOP_REVERSE(childs_g, sizeof(void*), nbChilds);
		ZRVECTOR_ADD_NB(dfiterator->queue, nbChilds, childs_g);
	}
}

ZRIterator* ZRTreeNode_std_getDescendants_DF(ZRTree *tree, ZRTreeNode *node, ZRAllocator *allocator)
{
	DFNodeIterator *ret = ZRALLOC(allocator, sizeof(DFNodeIterator));
	*ret = (DFNodeIterator ) { //
		.iterator = (ZRIterator ) { //
			.strategy = &ret->strategyArea, //
			},//
		.subject = tree, //
		.allocator = allocator, //
		.stack = ZRVector2SideStrategy_createDynamic(256, ZRTYPE_SIZE_ALIGNMENT(void*), allocator), //
		};
	ret->strategyArea = (ZRIteratorStrategy ) { //
		.fdestroy = BFNodeIterator_fdestroy, //
		.fcurrent = BFNodeIterator_fcurrent, //
		.fhasNext = BFIterator_fhasNext, //
		.fnext = DFIterator_fnext, //
		};
	ZRVECTOR_ADD(ret->queue, &node);
	return ZRTREEDFNODEITERATOR_ITERATOR(ret);
}

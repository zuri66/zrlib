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

void ZRTreeBuilder_destroy(ZRTreeBuilder *builder)
{
	builder->strategy->fdestroy(builder);
}

// ============================================================================
// TREE
// ============================================================================

ZRTreeNode* ZRTree_getRoot(ZRTree *tree)
{
	return ZRTREE_GETROOT(tree);
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
		.queue = ZRVector2SideStrategy_createDynamic(256, sizeof(void*), allocator), //
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
		.stack = ZRVector2SideStrategy_createDynamic(256, sizeof(void*), allocator), //
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

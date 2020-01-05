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
		current = ZRTREENODE_GETCHILD(tree, current, coord[i]);

		if (current == NULL)
			return NULL ;
	}
	return current;
}

// ============================================================================
// Standard implementations
// ============================================================================

typedef struct
{
	ZRITERATOR_MEMBERS(ZRIteratorStrategy);
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

static void BFNodeIterator_fdestroy(BFNodeIterator *iterator)
{
	ZRVector2SideStrategy_destroy(iterator->queue);
	ZRFREE(iterator->allocator, iterator);
}

static void* BFNodeIterator_fcurrent(BFNodeIterator *iterator)
{
	return iterator->current;
}

static bool BFIterator_fhasNext(BFNodeIterator *iterator)
{
	return ZRVECTOR_NBOBJ(iterator->queue) > 0;
}

static void BFIterator_fnext(BFNodeIterator *iterator)
{
	assert(BFIterator_fhasNext(iterator));
	ZRVECTOR_POPFIRST(iterator->queue, &iterator->current);
	size_t const nbChilds = ZRGRAPHNODE_GETNBCHILDS(iterator->subject_g, iterator->current_g);

	if (nbChilds > 0)
	{
		ZRGraphNode (*childs_g[nbChilds]);
		ZRGraphNode_getNChilds(iterator->subject_g, iterator->current_g, childs_g, 0, nbChilds);
		ZRVECTOR_ADD_NB(iterator->queue, nbChilds, childs_g);
	}
}

ZRIterator* ZRTreeNode_std_getDescendants_BF(ZRTree *tree, ZRTreeNode *node, ZRAllocator *allocator)
{
	BFNodeIterator *ret = ZRALLOC(allocator, sizeof(BFNodeIterator));
	*ret = (BFNodeIterator ) { //
		.subject = tree, //
		.allocator = allocator, //
		.strategy = &ret->strategyArea, //
		.queue = ZRVector2SideStrategy_createDynamic(256, sizeof(void*), allocator), //
		};
	ret->strategyArea = (ZRIteratorStrategy ) { //
		.fdestroy = (ZRIterator_fdestroy_t)BFNodeIterator_fdestroy, //
		.fcurrent = (ZRIterator_fcurrent_t)BFNodeIterator_fcurrent, //
		.fhasNext = (ZRIterator_fhasNext_t)BFIterator_fhasNext, //
		.fnext = (ZRIterator_fnext_t)BFIterator_fnext, //
		};
	ZRVECTOR_ADD(ret->queue, &node);
	return (ZRIterator*)ret;
}

static void DFIterator_fnext(DFNodeIterator *iterator)
{
	assert(BFIterator_fhasNext(iterator));
	ZRVECTOR_POP(iterator->queue, &iterator->current);
	size_t const nbChilds = ZRGRAPHNODE_GETNBCHILDS(iterator->subject_g, iterator->current_g);

	if (nbChilds > 0)
	{
		ZRGraphNode (*childs_g[nbChilds]);
		ZRGraphNode_getNChilds(iterator->subject_g, iterator->current_g, childs_g, 0, nbChilds);
		// Reverse to respect the order of the childs in the stack
		ZRARRAYOP_REVERSE(childs_g, sizeof(void*), nbChilds);
		ZRVECTOR_ADD_NB(iterator->queue, nbChilds, childs_g);
	}
}

ZRIterator* ZRTreeNode_std_getDescendants_DF(ZRTree *tree, ZRTreeNode *node, ZRAllocator *allocator)
{
	DFNodeIterator *ret = ZRALLOC(allocator, sizeof(DFNodeIterator));
	*ret = (DFNodeIterator ) { //
		.subject = tree, //
		.allocator = allocator, //
		.strategy = &ret->strategyArea, //
		.stack = ZRVector2SideStrategy_createDynamic(256, sizeof(void*), allocator), //
		};
	ret->strategyArea = (ZRIteratorStrategy ) { //
		.fdestroy = (ZRIterator_fdestroy_t)BFNodeIterator_fdestroy, //
		.fcurrent = (ZRIterator_fcurrent_t)BFNodeIterator_fcurrent, //
		.fhasNext = (ZRIterator_fhasNext_t)BFIterator_fhasNext, //
		.fnext = (ZRIterator_fnext_t)DFIterator_fnext, //
		};
	ZRVECTOR_ADD(ret->queue, &node);
	return (ZRIterator*)ret;
}

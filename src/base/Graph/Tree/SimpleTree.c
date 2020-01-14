/**
 * @author zuri
 * @date dimanche 1 décembre 2019, 15:36:56 (UTC+0100)
 */

#include <zrlib/base/Graph/Tree/Tree.h>
#include <zrlib/base/Graph/Tree/SimpleTree.h>
#include <zrlib/base/Vector/Vector2SideStrategy.h>

#include <assert.h>
#include <stdbool.h>

#include "SimpleTree.h"

// ============================================================================
// STRATEGY FUNCTIONS
// ============================================================================

static size_t fstrategySize(void)
{
	return sizeof(ZRSimpleTreeStrategy);
}

static ZRTreeBuilder* fnewTreeBuilder(ZRTree *tree, ZRTreeNode *currentForBuilder)
{
	return ZRSimpleTreeBuilder_fromSimpleTree((ZRSimpleTree*)tree, (ZRSimpleTreeNode*)currentForBuilder);
}

static size_t graph_fgetNNodes(ZRGraph *graph, ZRGraphNode **nodes_out, size_t offset, size_t maxNbNodes)
{
	ZRSimpleTree *const stree = (ZRSimpleTree*)graph;

	if (offset >= ZRSTREE_GRAPH(stree)->nbNodes)
		return 0;

	size_t const nbNodes = ZRSTREE_GRAPH(stree)->nbNodes - offset;
	size_t const nodeSize = ZRSTREE_NODESIZE(stree);
	char *nodes = (char*)ZRSTREE_TREE(stree)->root + (offset * nodeSize);
	size_t const nb = nbNodes < maxNbNodes ? nbNodes : maxNbNodes;
	size_t i;

	for (i = offset; i < nb; i++, nodes += nodeSize, nodes_out++)
		*nodes_out = (ZRGraphNode*)nodes;

	return nb;
}

static size_t graph_fgetNObjs(ZRGraph *graph, void *objs_out, size_t offset, size_t maxNbBytes)
{
	ZRSimpleTree *const stree = (ZRSimpleTree*)graph;

	if (offset >= ZRSTREE_GRAPH(stree)->nbNodes)
		return 0;

	size_t const objSize = ZRSTREE_GRAPH(stree)->objSize;
	size_t const nbObjs = ZRSTREE_GRAPH(stree)->nbNodes - offset;
	size_t const maxNbObjs = maxNbBytes / objSize;
	size_t const nodeSize = ZRSTREE_NODESIZE(stree);
	char *nodes = (char*)ZRSTREE_TREE(stree)->root + (offset * nodeSize);
	size_t const nb = nbObjs < maxNbObjs ? nbObjs : maxNbObjs;
	size_t i;

	for (i = offset; i < nb; i++, nodes += nodeSize)
	{
		memcpy(objs_out, ((ZRSimpleTreeNode*)nodes)->obj, objSize);
		objs_out = (char*)objs_out + objSize;
	}
	return nb;
}

static void graph_fdone(ZRGraph *graph)
{
	ZRSimpleTree *const stree = (ZRSimpleTree*)graph;
	ZRVector_destroy(stree->nodes);
	ZRFREE(stree->allocator, ZRSTREE_GRAPH(stree)->strategy);
}

static void graph_fdestroy(ZRGraph *graph)
{
	ZRSimpleTree *const stree = (ZRSimpleTree*)graph;
	ZRAllocator *allocator = stree->allocator;
	ZRVector_destroy(stree->nodes);
	ZRFREE(allocator, ZRSTREE_GRAPH(stree)->strategy);
	ZRFREE(allocator, stree);
}

// ============================================================================
// NODE
// ============================================================================

static void* graph_fNode_getObj(ZRGraph *graph, ZRGraphNode *gnode)
{
	ZRSimpleTreeNode *const snode = (ZRSimpleTreeNode*)gnode;
	return snode->obj;
}

static ZRTreeNode* fNode_getTheParent(ZRTree *tree, ZRTreeNode *tnode)
{
	ZRSimpleTreeNode *const snode = (ZRSimpleTreeNode*)tnode;
	return (ZRTreeNode*)snode->parent;
}

static ZRGraphNode* graph_fNode_getParent(ZRGraph *graph, ZRGraphNode *gnode, size_t pos)
{
	if (pos != 0)
		return NULL ;

	return (ZRGraphNode*)fNode_getTheParent((ZRTree*)graph, (ZRTreeNode*)gnode);
}

static ZRGraphNode* graph_fNode_getChild(ZRGraph *graph, ZRGraphNode *gnode, size_t pos)
{
	ZRSimpleTreeNode *const snode = (ZRSimpleTreeNode*)gnode;

	if (pos >= snode->nbChilds)
		return NULL ;

	ZRSimpleTree *const stree = (ZRSimpleTree*)graph;
	char *childs = (char*)(snode->childs);
	return (ZRGraphNode*)(childs + (pos * ZRSTREE_NODESIZE(stree)));
}

static size_t graph_fNode_getNbChilds(ZRGraph *graph, ZRGraphNode *gnode)
{
	ZRSimpleTreeNode *const snode = (ZRSimpleTreeNode*)gnode;
	return snode->nbChilds;
}

static size_t graph_fNode_getNChilds(ZRGraph *graph, ZRGraphNode *gnode, ZRGraphNode **gnodes_out, size_t offset, size_t maxNbNodes)
{
	ZRSimpleTreeNode *const snode = (ZRSimpleTreeNode*)gnode;

	if (offset >= snode->nbChilds)
		return 0;

	ZRSimpleTree *const stree = (ZRSimpleTree*)graph;
	size_t const nbNodes = snode->nbChilds - offset;
	size_t const nodeSize = ZRSTREE_NODESIZE(stree);
	char *nodes = (char*)snode->childs + (offset * nodeSize);
	size_t const nb = nbNodes < maxNbNodes ? nbNodes : maxNbNodes;
	size_t i;

	for (i = offset; i < nb; i++, nodes += nodeSize, gnodes_out++)
		*gnodes_out = (ZRGraphNode*)nodes;

	return nb;
}

static size_t graph_fNode_getNObjs(ZRGraph *graph, ZRGraphNode *gnode, void *objs_out, size_t offset, size_t maxNbBytes)
{
	ZRSimpleTreeNode *const snode = (ZRSimpleTreeNode*)gnode;

	if (offset >= snode->nbChilds)
		return 0;

	ZRSimpleTree *const stree = (ZRSimpleTree*)graph;
	size_t const objSize = ZRSTREE_GRAPH(stree)->objSize;
	size_t const nbObjs = snode->nbChilds - offset;
	size_t const maxNbObjs = maxNbBytes / objSize;
	size_t const nodeSize = ZRSTREE_NODESIZE(stree);
	char *nodes = (char*)nodes + (offset * nodeSize);
	size_t const nb = nbObjs < maxNbObjs ? nbObjs : maxNbObjs;
	size_t i;

	for (i = offset; i < nb; i++, nodes += nodeSize)
	{
		memcpy(objs_out, ((ZRSimpleTreeNode*)nodes)->obj, objSize);
		objs_out = (char*)objs_out + objSize;
	}
	return nb;
}

#define ZRNODEITERATOR_ITERATOR(NIT) (&(NIT)->iterator)
//#define ZRNODEITERATOR_STRATEGY(NIT) (ZRNodeIteratorStrategy*)(ZRNODEITERATOR_ITERATOR(NIT)->strategy)

typedef struct
{
	ZRIterator iterator;
	size_t nb;
	ZRSimpleTree *subject;
	ZRSimpleTreeNode *current;
	ZRSimpleTreeNode *next;

	ZRIteratorStrategy strategyArea;
} ZRNodeIterator;

static void Iterator_fdestroy(ZRIterator *iterator)
{
	ZRNodeIterator *const niterator = (ZRNodeIterator*)iterator;
	ZRFREE(niterator->subject->allocator, niterator);
}

static void* Iterator_fcurrent(ZRIterator *iterator)
{
	ZRNodeIterator *const niterator = (ZRNodeIterator*)iterator;
	return (ZRTreeNode*)niterator->current;
}

static bool ChildsIterator_fhasNext(ZRIterator *iterator)
{
	ZRNodeIterator *const niterator = (ZRNodeIterator*)iterator;
	return niterator->nb != 0;
}

static void ChildsIterator_fnext(ZRIterator *iterator)
{
	ZRNodeIterator *const niterator = (ZRNodeIterator*)iterator;
	assert(ChildsIterator_fhasNext(ZRNODEITERATOR_ITERATOR(niterator)) == true);
	niterator->current = (ZRSimpleTreeNode*)((char*)(niterator->current) + ZRSTREE_NODESIZE(niterator->subject));
	niterator->nb--;
}

static void ChildsIterator_fnext_first(ZRIterator *iterator)
{
	ZRNodeIterator *const niterator = (ZRNodeIterator*)iterator;
	assert(ChildsIterator_fhasNext(ZRNODEITERATOR_ITERATOR(niterator)) == true);
	niterator->current = niterator->next;
	niterator->nb--;
	ZRNODEITERATOR_ITERATOR(niterator)->strategy->fnext = ChildsIterator_fnext;
}

ZRIterator* fNode_getChilds(ZRTree *tree, ZRTreeNode *tnode)
{
	ZRSimpleTreeNode *const snode = (ZRSimpleTreeNode*)tnode;

	if (snode == NULL || snode->nbChilds == 0)
		return ZRIterator_emptyIterator();

	ZRSimpleTree *const stree = (ZRSimpleTree*)tree;

	ZRNodeIterator *iterator = ZRALLOC(stree->allocator, sizeof(ZRNodeIterator));
	*iterator = (ZRNodeIterator ) { //
		.iterator = (ZRIterator ) { //
			.strategy = &(iterator->strategyArea), //
			},//
		.subject = stree, //
		.nb = snode->nbChilds, //
		.next = snode->childs, //
		};
	iterator->strategyArea = (ZRIteratorStrategy ) { //
		.fdestroy = Iterator_fdestroy, //
		.fcurrent = Iterator_fcurrent, //
		.fnext = ChildsIterator_fnext_first, //
		.fhasNext = ChildsIterator_fhasNext, //
		};
	return (ZRIterator*)iterator;
}

static bool AscendantsIterator_fhasNext(ZRIterator *iterator)
{
	ZRNodeIterator *niterator = (ZRNodeIterator*)iterator;
	return niterator->next != NULL ;
}

static void AscendantsIterator_fnext(ZRIterator *iterator)
{
	ZRNodeIterator *niterator = (ZRNodeIterator*)iterator;
	assert(AscendantsIterator_fhasNext(iterator) == true);
	niterator->current = niterator->next;
	niterator->next = (ZRSimpleTreeNode*)niterator->current->parent;
}

ZRIterator* fNode_getAscendants(ZRTree *tree, ZRTreeNode *node)
{
	if (node == NULL)
		return ZRIterator_emptyIterator();

	ZRSimpleTree *stree = (ZRSimpleTree*)tree;
	ZRNodeIterator *niterator = ZRALLOC(stree->allocator, sizeof(ZRNodeIterator));
	ZRSimpleTreeNode *snode = (ZRSimpleTreeNode*)node;

	*niterator = (ZRNodeIterator ) { //
		.iterator = (ZRIterator ) { //
			.strategy = &(niterator->strategyArea), //
			},//
		.subject = stree, //
		.nb = snode->nbDescendants, //
		.next = snode, //
		};
	niterator->strategyArea = (ZRIteratorStrategy ) { //
		.fdestroy = Iterator_fdestroy, //
		.fcurrent = Iterator_fcurrent, //
		.fnext = AscendantsIterator_fnext, //
		.fhasNext = AscendantsIterator_fhasNext, //
		};
	return (ZRIterator*)niterator;
}

static void DescendantsIterator_fnext_first(ZRIterator *iterator)
{
	ZRNodeIterator *const niterator = (ZRNodeIterator*)iterator;
	assert(ChildsIterator_fhasNext(iterator) == true);
	niterator->nb--;
	niterator->current = niterator->next;
	niterator->next = niterator->current->childs;
	ZRNODEITERATOR_ITERATOR(niterator)->strategy->fnext = ChildsIterator_fnext_first;
}

ZRIterator* fNode_getDescendants(ZRTree *tree, ZRTreeNode *tnode)
{
	if (tnode == NULL)
		return ZRIterator_emptyIterator();

	ZRSimpleTree *stree = (ZRSimpleTree*)tree;
	ZRSimpleTreeNode *snode = (ZRSimpleTreeNode*)tnode;
	ZRNodeIterator *niterator = ZRALLOC(stree->allocator, sizeof(ZRNodeIterator));

	*niterator = (ZRNodeIterator ) { //
		.iterator = (ZRIterator ) { //
			.strategy = &(niterator->strategyArea), //
			},//
		.subject = stree, //
		.next = snode, //
		.nb = snode->nbDescendants + 1, //
		};
	niterator->strategyArea = (ZRIteratorStrategy ) { //
		.fdestroy = (ZRIterator_fdestroy_t)Iterator_fdestroy, //
		.fcurrent = (ZRIterator_fcurrent_t)Iterator_fcurrent, //
		.fnext = (ZRIterator_fnext_t)DescendantsIterator_fnext_first, //
		.fhasNext = (ZRIterator_fhasNext_t)ChildsIterator_fhasNext, //
		};
	return ZRNODEITERATOR_ITERATOR(niterator);
}

//static bool DescendantsBFIterator_fhasNext(ZRNodeIterator *iterator)
//{
//	return iterator->nb > 0;
//}

ZRIterator* fNode_getDescendants_BF(ZRTree *tree, ZRTreeNode *node)
{
	ZRSimpleTree *const stree = (ZRSimpleTree*)tree;
	return ZRTreeNode_std_getDescendants_BF(tree, node, stree->allocator);
}

ZRIterator* fNode_getDescendants_DF(ZRTree *tree, ZRTreeNode *node)
{
	ZRSimpleTree *const stree = (ZRSimpleTree*)tree;
	return ZRTreeNode_std_getDescendants_DF(tree, node, stree->allocator);
}

// ============================================================================

static void ZRSimpleTreeStrategy_init(ZRSimpleTreeStrategy *strategy)
{
	*strategy = (ZRSimpleTreeStrategy ) { //
		.tree = (ZRTreeStrategy ) { //
			.graph = (ZRGraphStrategy ) { //
				.fstrategySize = fstrategySize, //
				.fNodeGetObj = graph_fNode_getObj, //
				.fNodeGetParent = graph_fNode_getParent, //
				.fNodeGetChild = graph_fNode_getChild, //
				.fNodeGetNbChilds = graph_fNode_getNbChilds, //
				.fNodeGetNChilds = graph_fNode_getNChilds, //
				.fNodeGetNObjs = graph_fNode_getNObjs, //
				.fgetNNodes = graph_fgetNNodes, //
				.fgetNObjs = graph_fgetNObjs, //
				.fdone = graph_fdone, //
				},//
			.fNodeGetTheParent = fNode_getTheParent, //
			.fNodeGetChilds = fNode_getChilds, //
			.fNodeGetAscendants = fNode_getAscendants, //
			.fNodeGetDescendants = fNode_getDescendants, //
			.fNodeGetDescendants_BF = fNode_getDescendants_BF, //
			.fNodeGetDescendants_DF = fNode_getDescendants_DF, //
			.fnewTreeBuilder = fnewTreeBuilder, //
			} , //
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
	strategy->tree.graph.fdestroy = (ZRGraph_fdestroy_t)graph_fdestroy;

	ZRSimpleTree *tree = (ZRSimpleTree*)ZRSimpleTree_alloc(objSize, allocator);
	*tree = (ZRSimpleTree ) { //
		.tree = (ZRTree ) { //
			.graph = (ZRGraph ) { //
				.strategy = (ZRGraphStrategy*)strategy, //
				.objSize = objSize, //
				} , //
			},//
		.allocator = allocator, //
		.nodes = nodes, //
		};
	return (ZRTree*)tree;
}

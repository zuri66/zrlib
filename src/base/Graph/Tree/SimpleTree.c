/**
 * @author zuri
 * @date dimanche 1 décembre 2019, 15:36:56 (UTC+0100)
 */

#include <zrlib/base/Graph/Tree/Tree.h>
#include <zrlib/base/Graph/Tree/SimpleTree.h>
#include <zrlib/base/Vector/Vector2SideStrategy.h>

#include <assert.h>
#include <stdint.h>
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

static size_t fgraph_getNNodes(ZRGraph *graph, ZRGraphNode **nodes_out, size_t offset, size_t maxNbNodes)
{
	ZRSimpleTree *const stree = (ZRSimpleTree*)graph;

	if (offset >= ZRSTREE_GRAPH(stree)->nbNodes)
		return 0;

	size_t const nbNodes = ZRSTREE_GRAPH(stree)->nbNodes - offset;
	size_t const nb = nbNodes < maxNbNodes ? nbNodes : maxNbNodes;

	ZRCARRAY_TOPOINTERS(ZRGraphNode, nodes_out, ZRSimpleTreeNode, stree->nodes, nb);
	return nb;
}

static size_t fgraph_getNObjs(ZRGraph *graph, void *objs_out, size_t offset, size_t maxNbBytes)
{
	ZRSimpleTree *const stree = (ZRSimpleTree*)graph;

	if (offset >= ZRSTREE_GRAPH(stree)->nbNodes)
		return 0;

	size_t const objSize = ZRSTREE_GRAPH(stree)->objSize;
	size_t const nbObjs = ZRSTREE_GRAPH(stree)->nbNodes - offset;
	size_t const maxNbObjs = maxNbBytes / objSize;
	size_t const nb = nbObjs < maxNbObjs ? nbObjs : maxNbObjs;
	size_t i;

	memcpy(objs_out, (char*)stree->objs + offset * objSize, nb * objSize);
	return nb;
}

static void fgraph_done(ZRGraph *graph)
{
	ZRSimpleTree *const stree = (ZRSimpleTree*)graph;
	ZRFREE(stree->allocator, ZRSTREE_GRAPH(stree)->strategy);
}

static void fgraph_destroy(ZRGraph *graph)
{
	ZRSimpleTree *const stree = (ZRSimpleTree*)graph;
	ZRAllocator *const allocator = stree->allocator;
	ZRFREE(allocator, graph->strategy);
	ZRFREE(allocator, stree);
}

// ============================================================================
// EDGE
// ============================================================================

static size_t fgraphNode_getNbEdges(ZRGraph *graph, ZRGraphNode *node, enum ZRGraphEdge_selectE select)
{
	ZRSimpleTree *const stree = (ZRSimpleTree*)graph;
	ZRSimpleTreeNode *const snode = (ZRSimpleTreeNode*)node;

	switch (select)
	{
	case ZRGraphEdge_selectIN:
		return 1 - (node == ZRSTREE_TREE(stree)->root);
	case ZRGraphEdge_selectINOUT:
		return snode->nbChilds + 1 - (node == ZRSTREE_TREE(stree)->root);
	case ZRGraphEdge_selectOUT:
		return snode->nbChilds;
	default:
		return SIZE_MAX;
	}
}

static size_t fgraph_cpyNEdges(ZRGraph *graph, ZRGraphEdge *cpyTo, size_t offset, size_t maxNbCpy)
{
	assert(maxNbCpy > 0);

	if (offset >= graph->nbEdges)
		return 0;

	size_t nb = graph->nbEdges - offset;
	ZRSimpleTree *const stree = (ZRSimpleTree*)graph;
	ZRSimpleTreeNode *snode = (ZRSimpleTreeNode*)&stree->nodes[offset];

	if (nb > maxNbCpy)
		nb = maxNbCpy;

	snode++;
	size_t const ret = nb;

	while (nb--)
	{
		*cpyTo = (ZRGraphEdge )
			{
				.a = (ZRGraphNode*)snode->parent,
				.b = (ZRGraphNode*)snode,
			};
		cpyTo++;
		snode++;
	}
	return ret;
}

static size_t fgraphNode_cpyNEdges(ZRGraph *graph, ZRGraphNode *node, ZRGraphEdge *cpyTo, size_t offset, size_t maxNbCpy, enum ZRGraphEdge_selectE select)
{
	assert(maxNbCpy > 0);

	size_t ret = 0;
	ZRSimpleTree *const stree = (ZRSimpleTree*)graph;
	ZRSimpleTreeNode *snode = (ZRSimpleTreeNode*)node;
	bool cpyParent = (select & ZRGraphEdge_selectIN)
		&& node != ZRSTREE_TREE(stree)->root;

	if (offset >= snode->nbChilds + (int)cpyParent)
		return 0;

	if (cpyParent)
	{
		if (offset != 0)
			offset--;
		else
		{
			*cpyTo = (ZRGraphEdge )
				{
					.a = (ZRGraphNode*)snode->parent,
					.b = (ZRGraphNode*)snode,
				};
			cpyTo++;
			maxNbCpy--;
			ret++;
		}
	}

	if (!(select & ZRGraphEdge_selectOUT))
		return ret;

	size_t nb = snode->nbChilds - offset;

	if (nb > maxNbCpy)
		nb = maxNbCpy;

	for (size_t i = offset; i < nb; i++)
	{
		*cpyTo = (ZRGraphEdge )
			{
				.a = (ZRGraphNode*)snode,
				.b = (ZRGraphNode*)&snode->childs[i],
			};
		cpyTo++;
	}
	return ret + nb;
}

// ============================================================================
// NODE
// ============================================================================

static void* fgraph_node_getObj(ZRGraph *graph, ZRGraphNode *gnode)
{
	ZRSimpleTreeNode *const snode = (ZRSimpleTreeNode*)gnode;
	return snode->obj;
}

static ZRTreeNode* fNode_getTheParent(ZRTree *tree, ZRTreeNode *tnode)
{
	ZRSimpleTreeNode *const snode = (ZRSimpleTreeNode*)tnode;
	return (ZRTreeNode*)snode->parent;
}

static ZRGraphNode* fgraph_node_getParent(ZRGraph *graph, ZRGraphNode *gnode, size_t pos)
{
	if (pos != 0)
		return NULL ;

	return (ZRGraphNode*)fNode_getTheParent((ZRTree*)graph, (ZRTreeNode*)gnode);
}

static ZRGraphNode* fgraph_node_getChild(ZRGraph *graph, ZRGraphNode *gnode, size_t pos)
{
	ZRSimpleTreeNode *const snode = (ZRSimpleTreeNode*)gnode;

	if (pos >= snode->nbChilds)
		return NULL ;

	ZRSimpleTree *const stree = (ZRSimpleTree*)graph;
	return (ZRGraphNode*)(snode->childs + pos);
}

static size_t fgraph_node_getNbChilds(ZRGraph *graph, ZRGraphNode *gnode)
{
	ZRSimpleTreeNode *const snode = (ZRSimpleTreeNode*)gnode;
	return snode->nbChilds;
}

static size_t fgraph_node_getNChilds(ZRGraph *graph, ZRGraphNode *gnode, ZRGraphNode **gnodes_out, size_t offset, size_t maxNbNodes)
{
	ZRSimpleTreeNode *const snode = (ZRSimpleTreeNode*)gnode;

	if (offset >= snode->nbChilds)
		return 0;

	size_t const nbNodes = snode->nbChilds - offset;
	size_t const nb = nbNodes < maxNbNodes ? nbNodes : maxNbNodes;

	ZRCARRAY_TOPOINTERS(ZRGraphNode, gnodes_out, ZRSimpleTreeNode, snode->childs, nb);
	return nb;
}

static size_t fgraph_node_getNObjs(ZRGraph *graph, ZRGraphNode *gnode, void *objs_out, size_t offset, size_t maxNbBytes)
{
	ZRSimpleTreeNode *const snode = (ZRSimpleTreeNode*)gnode;

// Count gnode too
	size_t const nbNodes1 = snode->nbChilds + 1;

	if (offset >= nbNodes1)
		return 0;

	size_t const objSize = graph->objSize;
	size_t const nbObjs = nbNodes1 - offset;
	size_t const maxNbObjs = maxNbBytes / objSize;
	size_t const nb = nbObjs < maxNbObjs ? nbObjs : maxNbObjs;

	memcpy(objs_out, snode->obj, objSize * nb);
	return nb;
}

#define ZRNODEITERATOR_ITERATOR(NIT) (&(NIT)->iterator)

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
	niterator->current++;
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

size_t fNode_getNbAscendants(ZRTree *tree, ZRTreeNode *node)
{
	return ((ZRSimpleTreeNode*)node)->nbAscendants;
}

size_t fNode_getNbDescendants(ZRTree *tree, ZRTreeNode *node)
{
	return ((ZRSimpleTreeNode*)node)->nbDescendants;
}

// ============================================================================

static void ZRSimpleTreeStrategy_init(ZRSimpleTreeStrategy *strategy)
{
	*strategy = (ZRSimpleTreeStrategy ) { //
		.tree = (ZRTreeStrategy ) { //
			.graph = (ZRGraphStrategy ) { //
				.fstrategySize = fstrategySize, //
				.fnode_getNbEdges = fgraphNode_getNbEdges, //
				.fnode_cpyNEdges = fgraphNode_cpyNEdges, //
				.fcpyNEdges = fgraph_cpyNEdges, //
				.fnode_getObj = fgraph_node_getObj, //
				.fnode_getParent = fgraph_node_getParent, //
				.fnode_getChild = fgraph_node_getChild, //
				.fnode_getNbChilds = fgraph_node_getNbChilds, //
				.fnode_getNChilds = fgraph_node_getNChilds, //
				.fnode_getNObjs = fgraph_node_getNObjs, //
				.fgetNNodes = fgraph_getNNodes, //
				.fgetNObjs = fgraph_getNObjs, //
				.fdone = fgraph_done, //
				},//
			.ftreeNode_getNbAscendants = fNode_getNbAscendants, //
			.ftreeNode_getNbDescendants = fNode_getNbDescendants, //
			.ftreeNode_getTheParent = fNode_getTheParent, //
			.ftreeNode_getChilds = fNode_getChilds, //
			.ftreeNode_getAscendants = fNode_getAscendants, //
			.ftreeNode_getDescendants = fNode_getDescendants, //
			.ftreeNode_getDescendants_BF = fNode_getDescendants_BF, //
			.ftreeNode_getDescendants_DF = fNode_getDescendants_DF, //
			.fnewTreeBuilder = fnewTreeBuilder, //
			} , //
		};
}

ZRTree* ZRSimpleTree_create(size_t objSize, size_t nbObjs, size_t objAlignment, ZRAllocator *allocator)
{
	ZRSimpleTreeStrategy *strategy = ZRALLOC(allocator, sizeof(ZRSimpleTreeStrategy));
	ZRSimpleTreeStrategy_init(strategy);
	strategy->tree.graph.fdestroy = fgraph_destroy;

	ZRObjAlignInfos infos[ZRSIMPLETREENODE_INFOS_NB];
	ZRSimpleTreeInfos(infos, objSize, nbObjs, objAlignment);

	ZRSimpleTree *tree = ZRALLOC(allocator, infos[ZRSimpleTreeNodeInfos_struct].size);

	*tree = (ZRSimpleTree ) { //
		.tree = (ZRTree ) { //
			.graph = (ZRGraph ) { //
				.strategy = (ZRGraphStrategy*)strategy, //
				.objSize = objSize, //
				} , //
			},//
		.allocator = allocator, //
		.nbNodes = nbObjs, //
		.nodes = (ZRSimpleTreeNode*)((char*)tree + infos[ZRSimpleTreeNodeInfos_nodes].offset), //
		.objs = (char*)tree + infos[ZRSimpleTreeNodeInfos_objs].offset, //
		};
	return (ZRTree*)tree;
}

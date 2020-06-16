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

static void ftree_changeRoot(ZRTree *tree, ZRTreeNode *newRoot)
{
	ZRSimpleTree *const stree = (ZRSimpleTree*)tree;
	ZRTreeBuilder *tbuilder = ZRSimpleTreeBuilder_create( //
		ZRSTREE_GRAPH(stree)->nodeObjSize, ZRSTREE_GRAPH(stree)->nodeObjAlignment,
		ZRSTREE_GRAPH(stree)->edgeObjSize, ZRSTREE_GRAPH(stree)->edgeObjAlignment,
		stree->allocator
		);
	ZRTreeBuilder_concatRootedTree(tbuilder, tree, newRoot);
	fBuilder_build(tbuilder, ZRSTREE_TREE(stree));
	ZRTreeBuilder_destroy(tbuilder);
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
				ZRSTNODE_TNODE(snode->parent),
				ZRSTNODE_TNODE(snode),
				snode->edgeObj,
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
					ZRSTNODE_TNODE(snode->parent),
					ZRSTNODE_TNODE(snode),
					snode->edgeObj,
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
				ZRSTNODE_TNODE(snode),
				ZRSTNODE_TNODE(&snode->childs[i]),
				snode->edgeObj,
			};
		cpyTo++;
	}
	return ret + nb;
}

// ============================================================================
// NODE
// ============================================================================

static size_t fgraphNode_getNbParents(ZRGraph *graph, ZRGraphNode *gnode)
{
	return 1 - (gnode != ZRSTREE_TREE(ZRSTREE(graph))->root);
}

static size_t fgraphNode_getNbChilds(ZRGraph *graph, ZRGraphNode *gnode)
{
	ZRSimpleTreeNode *const snode = (ZRSimpleTreeNode*)gnode;
	return snode->nbChilds;
}

static size_t fgraphNode_getNParents(ZRGraph *graph, ZRGraphNode *gnode, ZRGraphNode **gnodes_out, size_t offset, size_t maxNbNodes)
{
	ZRSimpleTreeNode *const snode = (ZRSimpleTreeNode*)gnode;

	if (offset > 0 || maxNbNodes == 0)
		return 0;

	*gnodes_out = (ZRGraphNode*)snode->parent;
	return 1;
}

static size_t fgraphNode_getNChilds(ZRGraph *graph, ZRGraphNode *gnode, ZRGraphNode **gnodes_out, size_t offset, size_t maxNbNodes)
{
	ZRSimpleTreeNode *const snode = (ZRSimpleTreeNode*)gnode;

	if (offset >= snode->nbChilds)
		return 0;

	size_t const nbNodes = snode->nbChilds - offset;
	size_t const nb = nbNodes < maxNbNodes ? nbNodes : maxNbNodes;

	ZRCARRAY_TOPOINTERS(ZRGraphNode, gnodes_out, ZRSimpleTreeNode, snode->childs + offset, nb);
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
				.fnode_getNbParents = fgraphNode_getNbParents, //
				.fnode_getNbChilds = fgraphNode_getNbChilds, //
				.fnode_getNParents = fgraphNode_getNParents, //
				.fnode_getNChilds = fgraphNode_getNChilds, //
				.fgetNNodes = fgraph_getNNodes, //
				.fdone = fgraph_done, //
				},//
			.ftreeNode_getNbAscendants = fNode_getNbAscendants, //
			.ftreeNode_getNbDescendants = fNode_getNbDescendants, //
			.ftreeNode_getChilds = fNode_getChilds, //
			.ftreeNode_getAscendants = fNode_getAscendants, //
			.ftreeNode_getDescendants = fNode_getDescendants, //
			.ftreeNode_getDescendants_BF = fNode_getDescendants_BF, //
			.ftreeNode_getDescendants_DF = fNode_getDescendants_DF, //
			.fnewTreeBuilder = fnewTreeBuilder, //
			.ftree_changeRoot = ftree_changeRoot, //
			} , //
		};
}

ZRTree* ZRSimpleTree_create(size_t nbNodes,
	size_t nodeObjSize, size_t nodeObjAlignment,
	size_t edgeObjSize, size_t edgeObjAlignment,
	ZRAllocator *allocator
	)
{
	ZRSimpleTreeStrategy *strategy = ZRALLOC(allocator, sizeof(ZRSimpleTreeStrategy));
	ZRSimpleTreeStrategy_init(strategy);
	strategy->tree.graph.fdestroy = fgraph_destroy;

	ZRObjAlignInfos infos[ZRSIMPLETREENODE_INFOS_NB];
	ZRSimpleTreeInfos(infos, nbNodes, nodeObjSize, nodeObjAlignment, edgeObjSize, edgeObjAlignment);

	ZRSimpleTree *tree = ZRALLOC(allocator, infos[ZRSimpleTreeNodeInfos_struct].size);

	*tree = (ZRSimpleTree ) { //
		.tree = (ZRTree ) { //
			.graph = (ZRGraph ) { //
				.strategy = (ZRGraphStrategy*)strategy, //
				.nodeObjSize = nodeObjSize, //
				.nodeObjAlignment = nodeObjAlignment, //
				.edgeObjSize = edgeObjSize, //
				.edgeObjAlignment = edgeObjAlignment, //
				} , //
			},//
		.allocator = allocator, //
		.nbNodes = nbNodes, //
		.nodes = (ZRSimpleTreeNode*)((char*)tree + infos[ZRSimpleTreeNodeInfos_nodes].offset), //
		.nodeObjs = (char*)tree + infos[ZRSimpleTreeNodeInfos_nodeObjs].offset, //
		.edgeObjs = (char*)tree + infos[ZRSimpleTreeNodeInfos_edgeObjs].offset, //
		};
	return (ZRTree*)tree;
}

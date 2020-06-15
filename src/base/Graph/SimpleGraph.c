/**
 * @author zuri
 * @date samedi 9 mai 2020, 16:36:44 (UTC+0200)
 */

#include <zrlib/base/Graph/Graph.h>
#include <zrlib/base/Graph/SimpleGraph.h>
#include <zrlib/base/Vector/Vector2SideStrategy.h>

#include "SimpleGraph.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

// ============================================================================
// STRATEGY FUNCTIONS
// ============================================================================

static size_t fstrategySize(void)
{
	return sizeof(ZRSimpleGraphStrategy);
}

static size_t fgraph_getNNodes(ZRGraph *graph, ZRGraphNode **nodes_out, size_t offset, size_t maxNbNodes)
{
	ZRSimpleGraph *const sgraph = (ZRSimpleGraph*)graph;

	if (offset >= ZRSGRAPH_GRAPH(sgraph)->nbNodes)
		return 0;

	size_t const nbNodes = ZRSGRAPH_GRAPH(sgraph)->nbNodes - offset;
	size_t const nb = ZRMIN(nbNodes, maxNbNodes);

	ZRCARRAY_TOPOINTERS(ZRGraphNode, nodes_out, ZRSimpleGraphNode, &sgraph->nodes[offset], nb);
	return nb;
}

static void fgraph_done(ZRGraph *graph)
{
	ZRSimpleGraph *const sgraph = ZRSGRAPH(graph);
	ZRFREE(sgraph->allocator, ZRSGRAPH_GRAPH(sgraph)->strategy);
}

static void fgraph_destroy(ZRGraph *graph)
{
	ZRSimpleGraph *const sgraph = ZRSGRAPH(graph);
	ZRAllocator *const allocator = sgraph->allocator;
	ZRFREE(allocator, ZRSGRAPH_GRAPH(sgraph)->strategy);
	ZRFREE(allocator, sgraph);
}

// ============================================================================
// EDGE
// ============================================================================

static size_t fgraph_cpyNEdges(ZRGraph *graph, ZRGraphEdge *cpyTo, size_t offset, size_t maxNbCpy)
{
	size_t const nbEdges = graph->nbEdges;
	ZRSimpleGraph *sgraph = ZRSGRAPH(graph);

	if (offset >= nbEdges)
		return 0;

	size_t const nbFromOffset = nbEdges - offset;
	size_t const nb = ZRMIN(maxNbCpy, nbFromOffset);

	for (size_t i = 0; i < nb; i++)
	{
		ZRSimpleGraphEdge *echild = &sgraph->childEdges[offset + i];
		cpyTo[i] = ZRSimpleGraphEdge_cpyGE(echild);
	}
	return nb;
}

// ============================================================================
// NODE
// ============================================================================

static void* fgraphNode_getObj(ZRGraph *graph, ZRGraphNode *gnode)
{
	ZRSimpleGraphNode *const snode = (ZRSimpleGraphNode*)gnode;
	return snode->obj;
}

static size_t fgraphNode_getNbParents(ZRGraph *graph, ZRGraphNode *gnode)
{
	ZRSimpleGraphNode *const snode = (ZRSimpleGraphNode*)gnode;
	return snode->nbParents;
}

static size_t fgraphNode_getNbChilds(ZRGraph *graph, ZRGraphNode *gnode)
{
	ZRSimpleGraphNode *const snode = (ZRSimpleGraphNode*)gnode;
	return snode->nbChilds;
}

static size_t fgraphNode_getNbEdges(ZRGraph *graph, ZRGraphNode *node, enum ZRGraphEdge_selectE select)
{
	ZRSimpleGraphNode *const snode = (ZRSimpleGraphNode*)node;

	switch (select)
	{
	case ZRGraphEdge_selectIN:
		return snode->nbParents;
	case ZRGraphEdge_selectINOUT:
		return snode->nbChilds + snode->nbParents;
	case ZRGraphEdge_selectOUT:
		return snode->nbChilds;
	default:
		return SIZE_MAX;
	}
}

static size_t fgraphNode_getNParents(ZRGraph *graph, ZRGraphNode *gnode, ZRGraphNode **gnodes_out, size_t offset, size_t maxNbNodes)
{
	ZRSimpleGraphNode *const snode = (ZRSimpleGraphNode*)gnode;
	ZRSimpleGraphEdge **gedges = snode->parents;
	size_t nbNodes = snode->nbParents;

	if (offset >= nbNodes)
		return 0;

	size_t const nbFromOffset = nbNodes - offset;
	size_t const nb = ZRMIN(maxNbNodes, nbFromOffset);

	for (size_t i = 0; i < nb; i++)
		gnodes_out[i] = (ZRGraphNode*)(gedges[offset + i]->b);

	return nb;
}

static size_t fgraphNode_getNChilds(ZRGraph *graph, ZRGraphNode *gnode, ZRGraphNode **gnodes_out, size_t offset, size_t maxNbNodes)
{
	ZRSimpleGraphNode *const snode = (ZRSimpleGraphNode*)gnode;
	ZRSimpleGraphEdge *gedges = snode->childs;
	size_t nbNodes = snode->nbChilds;

	if (offset >= nbNodes)
		return 0;

	size_t const nbFromOffset = nbNodes - offset;
	size_t const nb = ZRMIN(maxNbNodes, nbFromOffset);

	for (size_t i = 0; i < nb; i++)
		gnodes_out[i] = (ZRGraphNode*)(gedges[offset + i].b);

	return nb;
}

ZRMUSTINLINE
static inline size_t node_cpyNParentEdges(
	ZRGraph *graph, ZRGraphNode *gnode, ZRGraphEdge *cpyTo, size_t offset, size_t maxNbCpy,
	ZRSimpleGraphEdge **gedges, size_t nbEdges
	)
{
	if (offset >= nbEdges)
		return 0;

	size_t const nbFromOffset = nbEdges - offset;
	size_t const nb = ZRMIN(maxNbCpy, nbFromOffset);

	for (size_t i = 0; i < nb; i++)
	{
		ZRSimpleGraphEdge *gedge = gedges[offset + i];
		cpyTo[i] = (ZRGraphEdge ) { (ZRGraphNode*)gedge->a, (ZRGraphNode*)gedge->b, gedge->obj };
	}
	return nb;
}

ZRMUSTINLINE
static inline size_t node_cpyNChildEdges(
	ZRGraph *graph, ZRGraphNode *gnode, ZRGraphEdge *cpyTo, size_t offset, size_t maxNbCpy,
	ZRSimpleGraphEdge *gedges, size_t nbEdges
	)
{
	if (offset >= nbEdges)
		return 0;

	size_t const nbFromOffset = nbEdges - offset;
	size_t const nb = ZRMIN(maxNbCpy, nbFromOffset);

	for (size_t i = 0; i < nb; i++)
	{
		ZRSimpleGraphEdge *gedge = &gedges[offset + i];
		cpyTo[i] = (ZRGraphEdge ) { (ZRGraphNode*)gedge->a, (ZRGraphNode*)gedge->b, gedge->obj };
	}
	return nb;
}

ZRMUSTINLINE
static inline size_t node_cpyNEdges(ZRGraph *graph, ZRGraphNode *gnode, ZRGraphEdge *cpyTo, size_t offset, size_t maxNbCpy)
{
	ZRSimpleGraphNode *const snode = (ZRSimpleGraphNode*)gnode;

	// Only childs to copy
	if (offset >= snode->nbParents)
		return node_cpyNChildEdges(graph, gnode, cpyTo, offset - snode->nbParents, maxNbCpy, snode->childs, snode->nbChilds);
	// Parents and childs to copy
	else
	{
		size_t const parent_nb = node_cpyNParentEdges(graph, gnode, cpyTo, offset, maxNbCpy, snode->parents, snode->nbParents);

		if (maxNbCpy == parent_nb)
			return parent_nb;

		return parent_nb + node_cpyNChildEdges(graph, gnode, &cpyTo[parent_nb], offset + parent_nb, maxNbCpy - parent_nb, snode->childs, snode->nbChilds);
	}
}

static size_t fgraphNode_cpyNEdges(ZRGraph *graph, ZRGraphNode *gnode, ZRGraphEdge *cpyTo, size_t offset, size_t maxNbCpy, enum ZRGraphEdge_selectE select)
{
	ZRSimpleGraphNode *const snode = (ZRSimpleGraphNode*)gnode;
	switch (select)
	{
	case ZRGraphEdge_selectIN:
		return node_cpyNParentEdges(graph, gnode, cpyTo, offset, maxNbCpy, snode->parents, snode->nbParents);
	case ZRGraphEdge_selectOUT:
		return node_cpyNChildEdges(graph, gnode, cpyTo, offset, maxNbCpy, snode->childs, snode->nbChilds);
	case ZRGraphEdge_selectINOUT:
		return node_cpyNEdges(graph, gnode, cpyTo, offset, maxNbCpy);
	default:
		fprintf(stderr, "Bad select value in %s: %d", __func__, select);
		exit(1);
	}
}

// ============================================================================

static void ZRSimpleGraphStrategy_init(ZRSimpleGraphStrategy *strategy)
{
	*strategy = (ZRSimpleGraphStrategy ) { //
		.graph = (ZRGraphStrategy ) { //
			.fstrategySize = fstrategySize, //

			.fnode_getObj = fgraphNode_getObj, //
			.fnode_getNbParents = fgraphNode_getNbParents, //
			.fnode_getNbChilds = fgraphNode_getNbChilds, //
			.fnode_getNbEdges = fgraphNode_getNbEdges, //

			.fnode_getNParents = fgraphNode_getNParents, //
			.fnode_getNChilds = fgraphNode_getNChilds, //
			.fnode_cpyNEdges = fgraphNode_cpyNEdges, //

			.fgetNNodes = fgraph_getNNodes, //
			.fcpyNEdges = fgraph_cpyNEdges, //
			.fdone = fgraph_done, //
			} , //
		};
}

ZRGraph* ZRSimpleGraph_create(
	size_t nbNodes, size_t nodeObjSize, size_t nodeObjAlignment,
	size_t nbEdges, size_t edgeObjSize, size_t edgeObjAlignment,
	ZRAllocator *allocator
	)
{
	ZRSimpleGraphStrategy *strategy = ZRALLOC(allocator, sizeof(ZRSimpleGraphStrategy));
	ZRSimpleGraphStrategy_init(strategy);
	strategy->graph.fdestroy = fgraph_destroy;

	ZRObjAlignInfos infos[ZRSIMPLEGRAPH_INFOS_NB];
	ZRSimpleGraph_makeInfos(infos,
		nbNodes, nodeObjSize, nodeObjAlignment,
		nbEdges, edgeObjSize, edgeObjAlignment
		);

	ZRSimpleGraph *graph = ZRALLOC(allocator, infos[ZRSimpleGraphInfos_struct].size);

	*graph = (ZRSimpleGraph ) { //
		.graph = (ZRGraph ) { //
			.strategy = (ZRGraphStrategy*)strategy, //
			.nodeObjSize = nodeObjSize, //
			.nodeObjAlignment = nodeObjAlignment, //
			.edgeObjSize = edgeObjSize, //
			.edgeObjAlignment = edgeObjAlignment, //
			.nbNodes = nbNodes, //
			.nbEdges = nbEdges, //
			},//
		.allocator = allocator, //
		.nodes =
			(ZRSimpleGraphNode*)((char*)graph + infos[ZRSimpleGraphInfos_nodes].offset), //
		.parentEdges =
			(ZRSimpleGraphEdge**)((char*)graph + infos[ZRSimpleGraphInfos_parentEdges].offset), //
		.childEdges =
			(ZRSimpleGraphEdge*)((char*)graph + infos[ZRSimpleGraphInfos_childEdges].offset), //
		.nodeObjs = (char*)graph + infos[ZRSimpleGraphInfos_nodeObjs].offset, //
		.edgeObjs = (char*)graph + infos[ZRSimpleGraphInfos_edgeObjs].offset, //
		};
	return ZRSGRAPH_GRAPH(graph);
}

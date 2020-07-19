/**
 * @author zuri
 * @date samedi 9 mai 2020, 16:36:44 (UTC+0200)
 */

#include <zrlib/base/macro.h>
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

static size_t fgraph_getNNodes(ZRGraph *graph, ZRGraphNode **nodes_out, size_t offset, size_t maxNbNodes)
{
	size_t nb;
	ZRCARRAY_CHECKFORCPY(offset, graph->nbNodes, maxNbNodes, return 0, nb = _nb);
	ZRCARRAY_TOPOINTERS(ZRGraphNode, nodes_out, ZRSimpleGraphNode, &ZRSGRAPH(graph)->nodes[offset], nb);
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

	ZRCARRAY_SAFECPY(offset, nbEdges, maxNbCpy, i, return 0, ZRCODE(
		ZRSimpleGraphEdge *echild = &sgraph->childEdges[offset + i];
		cpyTo[i] = ZRSimpleGraphEdge_cpyGE(echild);
		),
		return _nb);
}

// ============================================================================
// NODE
// ============================================================================

static size_t fgraphNode_getNbParents(ZRGraph *graph, ZRGraphNode *gnode)
{
	return ZRSGRAPHNODE(gnode)->nbParents;
}

static size_t fgraphNode_getNbChilds(ZRGraph *graph, ZRGraphNode *gnode)
{
	return ZRSGRAPHNODE(gnode)->nbChilds;
}

static size_t fgraphNode_getNbEdges(ZRGraph *graph, ZRGraphNode *gnode, enum ZRGraphEdge_selectE select)
{
	ZRGRAPHNODE_GETNBEDGES_MDEF(select, ZRSGRAPHNODE(gnode)->nbParents, ZRSGRAPHNODE(gnode)->nbChilds);
}

static size_t fgraphNode_getNParents(ZRGraph *graph, ZRGraphNode *gnode, ZRGraphNode **gnodes_out, size_t offset, size_t maxNbNodes)
{
	ZRSimpleGraphNode *const snode = ZRSGRAPHNODE(gnode);
	ZRSimpleGraphEdge **gedges = snode->parents;
	size_t nbNodes = snode->nbParents;

	ZRCARRAY_SAFECPY(offset, nbNodes, maxNbNodes, i, return 0,
		ZRCODE(gnodes_out[i] = (ZRGraphNode*)(gedges[offset + i]->b)),
		return _nb);
}

static size_t fgraphNode_getNChilds(ZRGraph *graph, ZRGraphNode *gnode, ZRGraphNode **gnodes_out, size_t offset, size_t maxNbNodes)
{
	ZRSimpleGraphNode *const snode = ZRSGRAPHNODE(gnode);
	ZRSimpleGraphEdge *gedges = snode->childs;
	size_t nbNodes = snode->nbChilds;

	ZRCARRAY_SAFECPY(offset, nbNodes, maxNbNodes, i, return 0,
		ZRCODE(gnodes_out[i] = (ZRGraphNode*)(gedges[offset + i].b)),
		return _nb);
}

ZRMUSTINLINE
static inline size_t node_cpyNParentEdges(
	ZRGraph *graph, ZRGraphNode *gnode, ZRGraphEdge *cpyTo, size_t offset, size_t maxNbCpy
	)
{
	ZRCARRAY_SAFECPY(offset, ZRSGRAPHNODE(gnode)->nbParents, maxNbCpy, i, return 0, ZRCODE(
		ZRSimpleGraphEdge *gedge = ZRSGRAPHNODE(gnode)->parents[offset + i];
		cpyTo[i] = (ZRGraphEdge ) { (ZRGraphNode*)gedge->a, (ZRGraphNode*)gedge->b, gedge->obj };
		),
		return _nb);
}

ZRMUSTINLINE
static inline size_t node_cpyNChildEdges(
	ZRGraph *graph, ZRGraphNode *gnode, ZRGraphEdge *cpyTo, size_t offset, size_t maxNbCpy
	)
{
	ZRCARRAY_SAFECPY(offset, ZRSGRAPHNODE(gnode)->nbChilds, maxNbCpy, i, return 0, ZRCODE(
		ZRSimpleGraphEdge *gedge = &ZRSGRAPHNODE(gnode)->childs[offset + i];
		cpyTo[i] = (ZRGraphEdge ) { (ZRGraphNode*)gedge->a, (ZRGraphNode*)gedge->b, gedge->obj };
		),
		return _nb);
}

ZRMUSTINLINE
static inline size_t node_cpyNEdges(ZRGraph *graph, ZRGraphNode *gnode, ZRGraphEdge *cpyTo, size_t offset, size_t maxNbCpy)
{
	ZRGRAPHNODE_CPYNEDGES_MDEF2(graph, gnode, cpyTo, offset, maxNbCpy, ZRSGRAPHNODE(gnode)->nbParents, ZRSGRAPHNODE(gnode)->nbChilds, node_cpyNParentEdges, node_cpyNChildEdges);
}

static size_t fgraphNode_cpyNEdges(ZRGraph *graph, ZRGraphNode *gnode, ZRGraphEdge *cpyTo, size_t offset, size_t maxNbCpy, enum ZRGraphEdge_selectE select)
{
	ZRGRAPHNODE_CPYNEDGES_MDEF(graph, gnode, cpyTo, offset, maxNbCpy, select, node_cpyNParentEdges, node_cpyNChildEdges, node_cpyNEdges);
}

// ============================================================================

static void ZRSimpleGraphStrategy_init(ZRSimpleGraphStrategy *strategy)
{
	*strategy = (ZRSimpleGraphStrategy ) { //
		.graph = (ZRGraphStrategy ) { //

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

/**
 * @author zuri
 * @date samedi 9 mai 2020, 16:36:54 (UTC+0200)
 */

#include <zrlib/base/Graph/SimpleGraph.h>
#include <zrlib/base/Map/VectorMap.h>
#include <zrlib/base/Vector/Vector2SideStrategy.h>
#include <zrlib/base/MemoryPool/MPoolDynamicStrategy.h>
#include <zrlib/base/macro.h>
#include <zrlib/base/Algorithm/fcmp.h>
#include "SimpleGraph.h"

#include <assert.h>
#include <stdalign.h>
#include <stdio.h>

#define INITIAL_VECTOR_SIZE 256
#define INITIAL_NB_NODES 1024

typedef struct
{
	ZRGraphBuilderStrategy graphBuilder;
} ZRSimpleGraphBuilderStrategy;

typedef struct
{
	/*
	 * In the SimpleGraphBuilder the GraphNode.id is the index of the node in the vector Graph.pnodes.
	 */
	ZRGraphBuilderNode gbNode;

	ZRVector *parents;
	ZRVector *childs;
	ZRVector *childEdgeObjs;
} ZRSimpleGraphBuilderNode;

typedef struct
{
	ZRSimpleGraphBuilderNode *node;
	void *obj;
} VectorParents_item;

#define ZRSGBNODE(N) ((ZRSimpleGraphBuilderNode*)(N))
#define ZRSGBNODE_GBNODE(N) (&(N)->gbNode)
#define ZRSGBNODE_GNODE(N) (ZRSGBNODE_GBNODE(N))

#define ZRSGBNODE_NBPARENTS(N) ZRVECTOR_NBOBJ((N)->parents)
#define ZRSGBNODE_NBCHILDS(N) ZRVECTOR_NBOBJ((N)->childs)

typedef struct
{
	ZRGraphBuilder graphBuilder;

	ZRObjAlignInfos nodeObjInfos;
	ZRObjAlignInfos edgeObjInfos;

	ZRVector *build_pedges;
	size_t build_nbChildEdges;
	size_t build_nbParentEdges;

	ZRAllocator *allocator;
	ZRVector *pnodes;

	ZRMemoryPool *pool_nodes;
	ZRMemoryPool *pool_nodeObjs;
	ZRMemoryPool *pool_edgeObjs;
} ZRSimpleGraphBuilder;

#define ZRSGRAPHBUILDER(B) ((ZRSimpleGraphBuilder*)(B))
#define ZRSGRAPHBUILDER_GB(B) (&(B)->graphBuilder)
#define ZRSGRAPHBUILDER_GRAPH(B) (&ZRSGRAPHBUILDER_GB(B)->graph)
#define ZRSGRAPHBUILDER_STRATEGY(B) (ZRSimpleGraphBuilderStrategy*)(ZRSGRAPHBUILDER_GRAPH(B)->strategy)

#define ZRSGRAPHBUILDER_NBNODES(B) (ZRSGRAPHBUILDER_GRAPH(B)->nbNodes)
#define ZRSGRAPHBUILDER_NBEDGES(B) (ZRSGRAPHBUILDER_GRAPH(B)->nbEdges)

#define ZRSIMPLEGRAPHBUILDERNODE_INFOS_NB 4

enum ZRSimpleGraphBuilderNode_infos
{
	ZRSimpleGraphBuilderNode_base,
	ZRSimpleGraphBuilderNode_nodeObj,
	ZRSimpleGraphBuilderNode_edgeObj,
	ZRSimpleGraphBuilderNode_struct,
};

ZRMUSTINLINE
static inline void node_infos(ZRObjAlignInfos *infos, size_t nodeObjSize, size_t nodeObjAlignment, size_t edgeObjSize, size_t edgeObjAlignment)
{
	infos[ZRSimpleGraphBuilderNode_base] = (ZRObjAlignInfos ) { 0, alignof(ZRSimpleGraphBuilderNode), sizeof(ZRSimpleGraphBuilderNode) };
	infos[ZRSimpleGraphBuilderNode_nodeObj] = (ZRObjAlignInfos ) { 0, nodeObjAlignment, nodeObjSize };
	infos[ZRSimpleGraphBuilderNode_edgeObj] = (ZRObjAlignInfos ) { 0, edgeObjAlignment, edgeObjSize };
	infos[ZRSimpleGraphBuilderNode_struct] = (ZRObjAlignInfos ) { };
	ZRStruct_bestOffsetsPos(ZRSIMPLEGRAPHBUILDERNODE_INFOS_NB - 1, infos, 1);
}

ZRMUSTINLINE
static inline ZRGraphEdge node_cpyParentEdge(ZRSimpleGraphBuilderNode *node, size_t edgeOffset)
{
	VectorParents_item from = *(VectorParents_item*)ZRVECTOR_GET(node->parents, edgeOffset);
	ZRGraphEdge ret = { ZRSGBNODE_GNODE(from.node), ZRSGBNODE_GNODE(node), from.obj };
	return ret;
}

ZRMUSTINLINE
static inline ZRGraphEdge node_cpyChildEdge(ZRSimpleGraphBuilderNode *node, size_t edgeOffset)
{
	ZRSimpleGraphBuilderNode *nodeChild = *(ZRSimpleGraphBuilderNode**)ZRVECTOR_GET(node->childs, edgeOffset);
	ZRGraphEdge ret = { ZRSGBNODE_GNODE(node), ZRSGBNODE_GNODE(nodeChild), ZRVECTOR_GET(node->childEdgeObjs, edgeOffset) };
	return ret;
}

// ============================================================================
// GRAPH FUNCTIONS
// ============================================================================

static size_t fgraphNode_getNbParents(ZRGraph *graph, ZRGraphNode *node)
{
	return ZRSGBNODE_NBPARENTS(ZRSGBNODE(node));
}

static size_t fgraphNode_getNbChilds(ZRGraph *graph, ZRGraphNode *node)
{
	return ZRSGBNODE_NBCHILDS(ZRSGBNODE(node));
}

static size_t fgraphNode_getNbEdges(
	ZRGraph *graph, ZRGraphNode *node, enum ZRGraphEdge_selectE select
	)
{
	ZRGRAPHNODE_GETNBEDGES_MDEF(select, ZRSGBNODE_NBPARENTS(ZRSGBNODE(node)), ZRSGBNODE_NBCHILDS(ZRSGBNODE(node)));
}

static size_t fgraphNode_getNParents(
	ZRGraph *graph, ZRGraphNode *node, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut
	)
{
	ZRCARRAY_SAFECPY(offset, ZRSGBNODE_NBPARENTS(ZRSGBNODE(node)), maxNbOut, i, return 0, ZRCODE(
		ZRSimpleGraphBuilderNode *bnode_out = *(ZRSimpleGraphBuilderNode**)(ZRVECTOR_GET(ZRSGBNODE(node)->parents, offset + i));
		nodes_out[i] = ZRSGBNODE_GBNODE(bnode_out);
		), return _nb);
}

static size_t fgraphNode_getNChilds(
	ZRGraph *graph, ZRGraphNode *node, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut
	)
{
	ZRCARRAY_SAFECPY(offset, ZRSGBNODE_NBCHILDS(ZRSGBNODE(node)), maxNbOut, i, return 0, ZRCODE(
		ZRSimpleGraphBuilderNode *bnode_out = *(ZRSimpleGraphBuilderNode**)(ZRVECTOR_GET(ZRSGBNODE(node)->childs, offset + i));
		nodes_out[i] = ZRSGBNODE_GBNODE(bnode_out);
		), return _nb);
}

ZRMUSTINLINE
static inline size_t node_cpyNParentEdges(
	ZRGraph *graph, ZRGraphNode *node, ZRGraphEdge *cpyTo, size_t offset, size_t maxNbCpy
	)
{
	ZRCARRAY_SAFECPY(offset, ZRSGBNODE_NBPARENTS(ZRSGBNODE(node)), maxNbCpy, i, return 0, ZRCODE(
		cpyTo[i] = node_cpyParentEdge(ZRSGBNODE(node), i + offset);
		), return _nb);
}

ZRMUSTINLINE
static inline size_t node_cpyNChildEdges(
	ZRGraph *graph, ZRGraphNode *node, ZRGraphEdge *cpyTo, size_t offset, size_t maxNbCpy
	)
{
	ZRCARRAY_SAFECPY(offset, ZRSGBNODE_NBCHILDS(ZRSGBNODE(node)), maxNbCpy, i, return 0, ZRCODE(
		cpyTo[i] = node_cpyChildEdge(ZRSGBNODE(node), i + offset);
		), return _nb);
}

ZRMUSTINLINE
static inline size_t node_cpyNEdges(ZRGraph *graph, ZRGraphNode *node, ZRGraphEdge *cpyTo, size_t offset, size_t maxNbCpy)
{
	ZRGRAPHNODE_CPYNEDGES_MDEF2(graph, node, cpyTo, offset, maxNbCpy,
		ZRSGBNODE_NBPARENTS(ZRSGBNODE(node)), ZRSGBNODE_NBCHILDS(ZRSGBNODE(node)),
		node_cpyNParentEdges, node_cpyNChildEdges
		);
}

static size_t fgraphNode_cpyNEdges(ZRGraph *graph, ZRGraphNode *gnode, ZRGraphEdge *cpyTo, size_t offset, size_t maxNbCpy, enum ZRGraphEdge_selectE select)
{
	ZRGRAPHNODE_CPYNEDGES_MDEF(graph, gnode, cpyTo, offset, maxNbCpy, select, node_cpyNParentEdges, node_cpyNChildEdges, node_cpyNEdges);
}

static size_t fgraph_getNNodes(ZRGraph *graph, ZRGraphNode **nodes_out, size_t offset, size_t maxNbNodes)
{
	ZRSimpleGraphBuilder *const sbuilder = ZRSGRAPHBUILDER(graph);
	size_t nb;

	ZRCARRAY_CHECKFORCPY(offset, ZRSGRAPHBUILDER_NBNODES(sbuilder), maxNbNodes, return 0, nb = _nb);
	ZRCARRAY_CPYPOINTERS(ZRGraphNode, nodes_out, ZRSimpleGraphBuilderNode, ZRVECTOR_GET(sbuilder->pnodes, offset), nb);
	return nb;
}

ZRMUSTINLINE
static inline size_t nodeOffset_cpyNChildEdges_getOffsets(
	ZRSimpleGraphBuilder *sbuilder, size_t *nodeOffset_p, size_t *edgeOffset_p, size_t maxNbCpy
	)
{
	size_t const nbNodes = ZRSGRAPHBUILDER_GRAPH(sbuilder)->nbNodes;
	size_t const nbEdges = ZRSGRAPHBUILDER_GRAPH(sbuilder)->nbEdges;
	size_t nb;
	size_t edgeOffset = *edgeOffset_p;
	size_t nodeOffset = 0;

	ZRCARRAY_CHECKFORCPY(edgeOffset, nbEdges, maxNbCpy, return 0, nb = _nb);

// Walk through the node to be at the good offset
	for (;; nodeOffset++)
	{
		ZRSimpleGraphBuilderNode *node = *(ZRSimpleGraphBuilderNode**)ZRVECTOR_GET(sbuilder->pnodes, nodeOffset);
		size_t nodeNbEdges = ZRSGBNODE_NBCHILDS(node);

		// nodeOffset founded
		if (nodeNbEdges > edgeOffset)
			break;

		edgeOffset -= nbEdges;
	}
	*nodeOffset_p = nodeOffset;
	*edgeOffset_p = edgeOffset;
	return nb;
}

ZRMUSTINLINE
static inline size_t nodeOffset_cpyNChildEdges(
	ZRSimpleGraphBuilder *sbuilder, size_t nodeOffset, size_t edgeOffset, ZRGraphEdge *cpyTo, size_t nb
	)
{
	size_t const nbNodes = ZRSGRAPHBUILDER_GRAPH(sbuilder)->nbNodes;
	size_t i = 0;

	for (; nodeOffset < nbNodes; nodeOffset++)
	{
		ZRSimpleGraphBuilderNode *node = *(ZRSimpleGraphBuilderNode**)ZRVECTOR_GET(sbuilder->pnodes, nodeOffset);
		size_t const nodeNbEdges = ZRSGBNODE_NBCHILDS(node);

		for (; edgeOffset < nodeNbEdges; edgeOffset++)
		{
			cpyTo[i++] = node_cpyChildEdge(node, edgeOffset);

			if (i == nb)
				return nb;
		}
	}
}

static size_t fgraph_cpyNEdges(ZRGraph *graph, ZRGraphEdge *cpyTo, size_t edgeOffset, size_t maxNbCpy)
{
	ZRSimpleGraphBuilder *sbuilder = ZRSGRAPHBUILDER(graph);
	size_t nodeOffset = 0;

	size_t nb = nodeOffset_cpyNChildEdges_getOffsets(sbuilder, &nodeOffset, &edgeOffset, maxNbCpy);
	nodeOffset_cpyNChildEdges(sbuilder, nodeOffset, edgeOffset, cpyTo, nb);
	return nb;
}

// ============================================================================

ZRMUSTINLINE
static inline void sbnode_cpyData(ZRSimpleGraphBuilder *sbuilder, ZRSimpleGraphBuilderNode *sbnode, void *nodeData)
{
	if (nodeData != NULL)
		memcpy(ZRSGBNODE_GBNODE(sbnode)->obj, nodeData, sbuilder->nodeObjInfos.size);
}

ZRMUSTINLINE
static inline void sbnode_add(ZRSimpleGraphBuilder *sbuilder, ZRSimpleGraphBuilderNode *sbnode, void *nodeData)
{
	ZRVector *const childs = ZRVector2SideStrategy_createDynamic(ZRTYPE_OBJINFOS(ZRSimpleGraphBuilderNode*), INITIAL_VECTOR_SIZE, sbuilder->allocator);
	ZRVector *const parents = ZRVector2SideStrategy_createDynamic(ZRTYPE_OBJINFOS(VectorParents_item), INITIAL_VECTOR_SIZE, sbuilder->allocator);
	ZRVector *const echilds = ZRVector2SideStrategy_createDynamic(ZRTYPE_OBJINFOS(sbuilder->edgeObjInfos), INITIAL_VECTOR_SIZE, sbuilder->allocator);
	ZRVECTOR_ADD(sbuilder->pnodes, &sbnode);

	*sbnode = (ZRSimpleGraphBuilderNode ) { //
		.gbNode = (ZRGraphNode ) { //
			/*
			 * Correspond to its index in the vector 'pnodes' (NBNODES is incremented after this call)
			 */
			.id = ZRSGRAPHBUILDER_NBNODES(sbuilder), //
			.obj = ZRMPOOL_RESERVE(sbuilder->pool_nodeObjs), //
			},//
		.childs = childs, //
		.parents = parents, //
		.childEdgeObjs = echilds, //
		};
	sbnode_cpyData(sbuilder, sbnode, nodeData);
}

ZRMUSTINLINE
static inline void sbnode_destroy(ZRSimpleGraphBuilder *sbuilder, ZRSimpleGraphBuilderNode *node)
{
	ZRVector_destroy(node->childs);
	ZRVector_destroy(node->parents);

	if (NULL != node->childEdgeObjs)
		ZRVector_destroy(node->childEdgeObjs);
}

static void fbuilder_edge(ZRGraphBuilder *builder, ZRGraphBuilderNode *a, ZRGraphBuilderNode *b, void *edgeData)
{
	ZRSimpleGraphBuilder *const sbuilder = ZRSGRAPHBUILDER(builder);
	ZRSimpleGraphBuilderNode *sa = ZRSGBNODE(a);
	ZRSimpleGraphBuilderNode *sb = ZRSGBNODE(b);

// Copy edge object
	if (sbuilder->edgeObjInfos.size > 0)
	{
		if (NULL == edgeData)
		{
			size_t const pos = ZRVECTOR_NBOBJ(sa->childEdgeObjs);
			ZRVECTOR_RESERVE(sa->childEdgeObjs, pos, 1);
			memset(ZRVECTOR_GET(sa->childEdgeObjs, pos), 0, sbuilder->edgeObjInfos.size);
		}
		else
			ZRVECTOR_ADD(sa->childEdgeObjs, edgeData);
	}
	VectorParents_item item = { sa, ZRVECTOR_GET(sa->childEdgeObjs, ZRVECTOR_NBOBJ(sa->childEdgeObjs) - 1) };
	ZRVECTOR_ADD(sb->parents, &item);
	ZRVECTOR_ADD(sa->childs, &sb);

	ZRSGRAPHBUILDER_NBEDGES(sbuilder)++;
}

static ZRGraphBuilderNode* fbuilder_node(ZRGraphBuilder *builder, void *nodeData)
{
	ZRSimpleGraphBuilder *const sbuilder = ZRSGRAPHBUILDER(builder);
	size_t const nodePos = ZRSGRAPHBUILDER_NBNODES(sbuilder);
	ZRSimpleGraphBuilderNode *bnode;

	bnode = (ZRSimpleGraphBuilderNode*)ZRMPOOL_RESERVE(sbuilder->pool_nodes);
	sbnode_add(sbuilder, bnode, nodeData);
	ZRSGRAPHBUILDER_NBNODES(sbuilder)++;
	return ZRGRAPHNODE(bnode);
}

// ============================================================================
// BUILD FUNCTIONS
// ============================================================================

ZRMUSTINLINE
static inline ZRSimpleGraphNode* build_graphGetNode(ZRSimpleGraphBuilderNode *bnode, ZRSimpleGraph *sgraph)
{
	return &sgraph->nodes[ZRSGBNODE_GNODE(bnode)->id];
}

ZRMUSTINLINE
static inline void* build_graphGetNodeObj(ZRSimpleGraphBuilderNode *bnode, ZRSimpleGraph *sgraph)
{
	return ZRARRAYOP_GET(sgraph->nodeObjs, ZRSGRAPH_GRAPH(sgraph)->nodeObjSize, ZRSGBNODE_GNODE(bnode)->id);
}

ZRMUSTINLINE
inline static void build_nodeEdges(
	ZRSimpleGraphBuilder *sbuilder, ZRSimpleGraphBuilderNode *sbnode,
	ZRSimpleGraph *sgraph
	)
{
	size_t const edgeObjSize = sbuilder->edgeObjInfos.size;
	ZRSimpleGraphNode *gnode = build_graphGetNode(sbnode, sgraph);

	for (size_t i = 0, c = ZRSGBNODE_NBCHILDS(sbnode); i < c; i++)
	{
		ZRSimpleGraphBuilderNode *sbnode_to = *(ZRSimpleGraphBuilderNode**)ZRVECTOR_GET(sbnode->childs, i);
		ZRSimpleGraphNode *gnode_to = build_graphGetNode(sbnode_to, sgraph);

		// Copy edge object
		void *gedgeObj = ZRARRAYOP_GET(gnode->edgeChildsObjs, edgeObjSize, i);

		if (edgeObjSize > 0)
		{
			void *bedgeObj = ZRVECTOR_GET(sbnode->childEdgeObjs, i);
			memcpy(gedgeObj, bedgeObj, edgeObjSize);
		}
		// Set the edge
		ZRSimpleGraphEdge *edge = &gnode->childs[i];
		*edge = (ZRSimpleGraphEdge ) { gnode, gnode_to, gedgeObj };

		/* Build the parent edge */
		gnode_to->parents[gnode_to->nbParents++] = edge;
	}
}

ZRMUSTINLINE
static inline void build_node(
	ZRSimpleGraphBuilder *sbuilder, ZRSimpleGraphBuilderNode *sbnode,
	ZRSimpleGraph *sgraph, ZRSimpleGraphNode *gnode
	)
{
	size_t const nodeObjSize = sbuilder->nodeObjInfos.size;

	*gnode = (ZRSimpleGraphNode ) { //
		.node = (ZRGraphNode ) { //
			.id = ZRGRAPHNODE(sbnode)->id, //
			.obj = build_graphGetNodeObj(sbnode, sgraph), //
			},//
		.nbParents = 0, /* Must be set by the building phase */
		.nbChilds = ZRVECTOR_NBOBJ(sbnode->childs), //
		.parents = &sgraph->parentEdges[sbuilder->build_nbParentEdges],
		.childs = &sgraph->childEdges[sbuilder->build_nbChildEdges],
		.edgeChildsObjs = ZRARRAYOP_GET(sgraph->edgeObjs, ZRSGRAPH_GRAPH(sgraph)->edgeObjSize, sbuilder->build_nbChildEdges), //
		};

	if ( ZRSGBNODE_GBNODE(sbnode)->obj != NULL)
		memcpy(ZRSGRAPHNODE_GN(gnode)->obj, ZRSGBNODE_GBNODE(sbnode)->obj, nodeObjSize);

	sbuilder->build_nbParentEdges += ZRSGBNODE_NBPARENTS(sbnode);
	sbuilder->build_nbChildEdges += ZRSGBNODE_NBCHILDS(sbnode);
}

static void fBuilder_build(ZRSimpleGraphBuilder *sbuilder, ZRSimpleGraph *sgraph, ZRMap *bnodesRef)
{
	ZRSimpleGraphNode *gnode;
	ZRSimpleGraphBuilderNode *bnode;
	size_t const graphNbNodes = ZRSGRAPHBUILDER_NBNODES(sbuilder);

// Add each node to the graph
	for (size_t i = 0; i < graphNbNodes; i++)
	{
		bnode = *(ZRSimpleGraphBuilderNode**)(ZRVECTOR_GET(sbuilder->pnodes, i));
		gnode = &sgraph->nodes[i];
		void ***refNode = ZRMAP_GET(bnodesRef, &ZRGRAPHNODE(bnode)->id);

		if (refNode != NULL)
		{
			**refNode = gnode;
			ZRMAP_DELETE(bnodesRef, &ZRGRAPHNODE(bnode)->id);
		}
	}

// Build each node
	for (size_t i = 0; i < graphNbNodes; i++)
	{
		bnode = *(ZRSimpleGraphBuilderNode**)(ZRVECTOR_GET(sbuilder->pnodes, i));
		build_node(sbuilder, bnode, sgraph, build_graphGetNode(bnode, sgraph));
	}
// Build edges
	for (size_t i = 0; i < graphNbNodes; i++)
	{
		bnode = *(ZRSimpleGraphBuilderNode**)(ZRVECTOR_GET(sbuilder->pnodes, i));
		build_nodeEdges(sbuilder, bnode, sgraph);
	}

// Set to null resting nodes from 'noderef'
// TODO: is it a good strategy ? Best to avoid this ?
	ZRVector *bnodesRef_vec = ZRVectorMap_vector(bnodesRef);

	for (size_t i = 0, c = ZRMAP_NBOBJ(bnodesRef); i < c; i++)
		**(void***)ZRVECTOR_GET(bnodesRef_vec, i) = NULL;

// Reset build values
	sbuilder->build_nbParentEdges = 0;
	sbuilder->build_nbChildEdges = 0;
	ZRVECTOR_DELETE_ALL(sbuilder->build_pedges);
}

static ZRGraph* fbuilder_new(ZRGraphBuilder *builder, void **nodes, size_t nbNodes)
{
	ZRSimpleGraphBuilder *const sbuilder = (ZRSimpleGraphBuilder*)builder;
	ZRSimpleGraph *sgraph = (ZRSimpleGraph*)ZRSimpleGraph_create(
		ZRSGRAPHBUILDER_NBNODES(sbuilder), ZROBJALIGNINFOS_SIZE_ALIGNMENT(sbuilder->nodeObjInfos),
		ZRSGRAPHBUILDER_NBEDGES(sbuilder), ZROBJALIGNINFOS_SIZE_ALIGNMENT(sbuilder->edgeObjInfos),
		sbuilder->allocator);

	if (ZRSGRAPHBUILDER_NBNODES(sbuilder) > 0)
	{
		// Key : size_t Value : *void
		ZRMap *bnodesRef = ZRVectorMap_create(
			ZRTYPE_OBJINFOS(size_t), ZRTYPE_OBJINFOS(void*),
			zrfcmp_size_t, NULL, sbuilder->allocator,
			ZRVectorMap_modeOrder
			);

		// Construct the nodeRef Map
		for (size_t i = 0; i < nbNodes; i++)
		{
			void **ref = &nodes[i];
			ZRSimpleGraphBuilderNode *bnodeRef = *ref;
			ZRMAP_PUT(bnodesRef, &ZRGRAPHNODE(bnodeRef)->id, &ref);
		}
		fBuilder_build(sbuilder, sgraph, bnodesRef);
		ZRMAP_DESTROY(bnodesRef);
	}
	return ZRSGRAPH_GRAPH(sgraph);
}

// ============================================================================
// BUILDER HELP
// ============================================================================

static void ZRSimpleGraphBuilder_done(ZRGraph *builder);
static void ZRSimpleGraphBuilder_destroy(ZRGraph *builder);

static void ZRSimpleGraphBuilderS_init(ZRSimpleGraphBuilderStrategy *strategy)
{
	*strategy = (ZRSimpleGraphBuilderStrategy ) { //
		.graphBuilder = (ZRGraphBuilderStrategy ) { //
			.graph = (ZRGraphStrategy ) { //
				.fnode_getNbParents = fgraphNode_getNbParents, //
				.fnode_getNbChilds = fgraphNode_getNbChilds, //
				.fnode_getNbEdges = fgraphNode_getNbEdges, //

				.fnode_getNParents = fgraphNode_getNParents, //
				.fnode_getNChilds = fgraphNode_getNChilds, //
				.fnode_cpyNEdges = fgraphNode_cpyNEdges, //

				.fgetNNodes = fgraph_getNNodes, //
				.fcpyNEdges = fgraph_cpyNEdges, //

				.fdone = ZRSimpleGraphBuilder_done, //
				},//
			.fnew = fbuilder_new, //
			.fnode = fbuilder_node, //
			.fedge = fbuilder_edge, //
			} , //
		};
}

static void ZRSimpleGraphBuilder_init(ZRSimpleGraphBuilder *sbuilder, ZRSimpleGraphBuilderStrategy *strategy,
	ZRObjAlignInfos *nodeObjInfos, ZRObjAlignInfos *edgeObjInfos,
	ZRAllocator *allocator
	)
{
	ZRVector *const pnodes = ZRVector2SideStrategy_createDynamic(ZRTYPE_OBJINFOS(ZRSimpleGraphBuilderNode*), INITIAL_NB_NODES, allocator);
	ZRVector *const pedges = ZRVector2SideStrategy_createDynamic(ZRTYPE_OBJINFOS(ZRSimpleGraphEdge*), INITIAL_NB_NODES, allocator);

	ZRMemoryPool *const nodes = ZRMPoolDS_createBS(INITIAL_NB_NODES, ZRTYPE_OBJINFOS(ZRSimpleGraphBuilderNode), allocator);
	ZRMemoryPool *const nodeObjs = ZRMPoolDS_createBS(INITIAL_NB_NODES, ZROBJALIGNINFOS_CPYOBJINFOS(*nodeObjInfos), allocator);
	ZRMemoryPool *const edgeObjs = ZRMPoolDS_createBS(INITIAL_NB_NODES, ZROBJALIGNINFOS_CPYOBJINFOS(*edgeObjInfos), allocator);

	*sbuilder = (ZRSimpleGraphBuilder ) { //
		.graphBuilder = (ZRGraphBuilder ) { //
			.graph = (ZRGraph ) { //
				.strategy = (ZRGraphStrategy*)strategy, //
				.nodeObjSize = nodeObjInfos->size,
				.nodeObjAlignment = nodeObjInfos->alignment,
				.edgeObjSize = edgeObjInfos->size,
				.edgeObjAlignment = edgeObjInfos->alignment,
				} , //
			},//
		.build_pedges = pedges, //
		.nodeObjInfos = *nodeObjInfos, //
		.edgeObjInfos = *edgeObjInfos, //
		.allocator = allocator, //
		.pnodes = pnodes, //
		.pool_nodes = nodes, //
		.pool_nodeObjs = nodeObjs, //
		.pool_edgeObjs = edgeObjs, //
		};
}

static void ZRSimpleGraphBuilder_done(ZRGraph *builder)
{
	ZRSimpleGraphBuilder *const sbuilder = ZRSGRAPHBUILDER(builder);
	size_t const nbNodes = ZRVECTOR_NBOBJ(sbuilder->pnodes);

	for (size_t i = 0; i < nbNodes; i++)
		sbnode_destroy(sbuilder, *(ZRSimpleGraphBuilderNode**)ZRVECTOR_GET(sbuilder->pnodes, i));

	ZRVECTOR_DESTROY(sbuilder->pnodes);
	ZRVECTOR_DESTROY(sbuilder->build_pedges);
	ZRMPOOL_DESTROY(sbuilder->pool_nodes);
	ZRMPOOL_DESTROY(sbuilder->pool_nodeObjs);
	ZRMPOOL_DESTROY(sbuilder->pool_edgeObjs);
}

static void ZRSimpleGraphBuilder_destroy(ZRGraph *builder)
{
	ZRSimpleGraphBuilder *const sbuilder = ZRSGRAPHBUILDER(builder);

	ZRSimpleGraphBuilder_done(builder);

	ZRFREE(sbuilder->allocator, ZRSGRAPHBUILDER_STRATEGY(sbuilder));
	ZRFREE(sbuilder->allocator, builder);
}

ZRGraphBuilder* ZRSimpleGraphBuilder_create(
	size_t nodeObjSize, size_t nodeObjAlignment,
	size_t edgeObjSize, size_t edgeObjAlignment,
	ZRAllocator *allocator
	)
{
	ZRObjAlignInfos infos[ZRSIMPLEGRAPHBUILDERNODE_INFOS_NB];

	ZRSimpleGraphBuilderStrategy *strategy = ZRALLOC(allocator, sizeof(ZRSimpleGraphBuilderStrategy));
	ZRSimpleGraphBuilderS_init(strategy);
	ZRGBSTRATEGY_GSTRATEGY(&strategy->graphBuilder)->fdestroy = ZRSimpleGraphBuilder_destroy;

	node_infos(infos, nodeObjSize, nodeObjAlignment, edgeObjSize, edgeObjAlignment);

	ZRSimpleGraphBuilder *sbuilder = ZRALLOC(allocator, sizeof(ZRSimpleGraphBuilder));
	ZRSimpleGraphBuilder_init(sbuilder, strategy,
		&infos[ZRSimpleGraphBuilderNode_nodeObj],
		&infos[ZRSimpleGraphBuilderNode_edgeObj],
		allocator
		);
	return ZRSGRAPHBUILDER_GB(sbuilder);
}

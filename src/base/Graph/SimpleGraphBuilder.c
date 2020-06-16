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

#define INITIAL_VECTOR_SIZE 256
#define INITIAL_NB_NODES 1024

typedef struct
{
	ZRGraphBuilderStrategy graphBuilder;
} ZRSimpleGraphBuilderStrategy;

typedef struct
{
	ZRGraphBuilderNode gbNode;
	ZRVector *parents;
	ZRVector *childs;
	ZRVector *childEdgeObjs;
	int mark :1;
} ZRSimpleGraphBuilderNode;

#define ZRSGBNODE(N) ((ZRSimpleGraphBuilderNode*)(N))
#define ZRSGBNODE_GBNODE(N) (&(N)->gbNode)

typedef struct
{
	ZRGraphBuilder graphBuilder;

	ZRObjAlignInfos nodeObjInfos;
	ZRObjAlignInfos edgeObjInfos;

	size_t nbEdges;

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
#define ZRSGRAPHBUILDER_GB(B) ((B)->graphBuilder)
#define ZRSGRAPHBUILDER_STRATEGY(B) (ZRSimpleGraphBuilderStrategy*)(ZRSGRAPHBUILDER_GB(B)->strategy)

#define ZRSGRAPHBUILDER_NBNODES(B) ZRVECTOR_NBOBJ((B)->pnodes)
#define ZRSGRAPHBUILDER_NBEDGES(B) ((B)->nbEdges)

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
	ZRVector *const childs = ZRVector2SideStrategy_createDynamic(INITIAL_VECTOR_SIZE,ZRTYPE_SIZE_ALIGNMENT(ZRSimpleGraphBuilderNode*),sbuilder->allocator);
	ZRVector *const parents = ZRVector2SideStrategy_createDynamic(INITIAL_VECTOR_SIZE,ZRTYPE_SIZE_ALIGNMENT(ZRSimpleGraphBuilderNode*),sbuilder->allocator);
	ZRVector *const echilds = ZRVector2SideStrategy_createDynamic(INITIAL_VECTOR_SIZE, ZROBJALIGNINFOS_SIZE_ALIGNMENT(sbuilder->edgeObjInfos), sbuilder->allocator);
	ZRVECTOR_ADD(sbuilder->pnodes, &sbnode);

	*sbnode = (ZRSimpleGraphBuilderNode ) { //
		.gbNode = (ZRGraphNode ) { //
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

	ZRVECTOR_ADD(sb->parents, &sa);
	ZRVECTOR_ADD(sa->childs, &sb);

	// Copy edge object
	if (sbuilder->edgeObjInfos.size > 0)
	{
		if (NULL == edgeData)
		{
			size_t const pos = sa->childEdgeObjs->nbObj;
			ZRVECTOR_RESERVE(sa->childEdgeObjs, pos, 1);
			memset(ZRVECTOR_GET(sa->childEdgeObjs, pos), 0, sbuilder->edgeObjInfos.size);
		}
		else
			ZRVECTOR_ADD(sa->childEdgeObjs, edgeData);
	}
	sbuilder->nbEdges++;
}

static ZRGraphBuilderNode* fbuilder_node(ZRGraphBuilder *builder, void *nodeData)
{
	ZRSimpleGraphBuilder *const sbuilder = ZRSGRAPHBUILDER(builder);
	size_t const nodePos = ZRSGRAPHBUILDER_NBNODES(sbuilder);
	ZRSimpleGraphBuilderNode *bnode;

	bnode = (ZRSimpleGraphBuilderNode*)ZRMPOOL_RESERVE(sbuilder->pool_nodes);
	sbnode_add(sbuilder, bnode, nodeData);

	return ZRGRAPHNODE(bnode);
}

ZRMUSTINLINE
inline static void build_childEdges(
	ZRSimpleGraphBuilder *sbuilder, ZRSimpleGraphBuilderNode *sbnode,
	ZRSimpleGraph *sgraph, ZRSimpleGraphNode *gnode,
	ZRMap *map_bnodes
	)
{
	size_t const edgeObjSize = sbuilder->edgeObjInfos.size;

	for (size_t i = 0, c = sbnode->childs->nbObj; i < c; i++)
	{
		ZRSimpleGraphBuilderNode *sbnode_to = *(ZRSimpleGraphBuilderNode**)ZRVECTOR_GET(sbnode->childs, i);
		ZRSimpleGraphNode *gnode_to = *(ZRSimpleGraphNode**)ZRMAP_GET(map_bnodes, &sbnode_to);

		// Copy edge object
		void *gedgeObj = ZRARRAYOP_GET(sgraph->edgeObjs, edgeObjSize, sbuilder->build_nbChildEdges);

		if (edgeObjSize > 0)
		{
			void *bedgeObj = ZRVECTOR_GET(sbnode->childEdgeObjs, sbuilder->build_nbChildEdges);
			memcpy(gedgeObj, bedgeObj, edgeObjSize);
		}

		// Set the edge
		ZRSimpleGraphEdge *edge = &sgraph->childEdges[sbuilder->build_nbChildEdges];
		*edge = (ZRSimpleGraphEdge ) { gnode, gnode_to, gedgeObj };

		sbuilder->build_nbChildEdges++;
	}
}

ZRMUSTINLINE
inline static void build_parentEdges(
	ZRSimpleGraphBuilder *sbuilder, ZRSimpleGraphBuilderNode *sbnode,
	ZRSimpleGraph *sgraph, ZRSimpleGraphNode *gnode,
	ZRMap *map_bnodes
	)
{
	size_t const edgeObjSize = sbuilder->edgeObjInfos.size;
	size_t childEdgesOffset = 0;

	for (size_t i = 0, c = sbnode->parents->nbObj; i < c; i++)
	{
		ZRSimpleGraphBuilderNode *bnode_from = *(ZRSimpleGraphBuilderNode**)ZRVECTOR_GET(sbnode->parents, i);
		ZRSimpleGraphNode *gnode_from = *(ZRSimpleGraphNode**)ZRMAP_GET(map_bnodes, &bnode_from);

		/*
		 * Get all the edges of the form (gnode_from, gnode).
		 * We will use the first edge founded, that will be added to sbuilder->build_pedges and so will not be used anymore.
		 */
		ZRSimpleGraph_getEdges(sgraph, gnode_from, gnode, sbuilder->build_pedges);
		size_t const nbChilds = sbuilder->build_pedges->nbObj;
		size_t const nbFounded = nbChilds - childEdgesOffset;
		assert(nbFounded > 0);
		ZRSimpleGraphEdge *edge = ZRVECTOR_GET(sbuilder->build_pedges, childEdgesOffset);

		// Set the edge
		sgraph->parentEdges[sbuilder->build_nbParentEdges] = edge;
		sbuilder->build_nbParentEdges++;

		childEdgesOffset++;

		// Delete the edges after the used one
		if (nbFounded > 1)
			ZRVECTOR_DELETE_NB(sbuilder->build_pedges, childEdgesOffset, nbFounded - 1);

	}
	ZRVECTOR_DELETE_ALL(sbuilder->build_pedges);
}

ZRMUSTINLINE
static inline void build_node(
	ZRSimpleGraphBuilder *sbuilder, ZRSimpleGraphBuilderNode *sbnode,
	ZRSimpleGraph *sgraph, ZRSimpleGraphNode *gnode, size_t pos,
	ZRMap *map_bnodes
	)
{
	if (sbnode->mark == 1)
		return;

	size_t const nodeObjSize = sbuilder->nodeObjInfos.size;

	sbnode->mark = 1;
	*gnode = (ZRSimpleGraphNode ) { //
		.node = (ZRGraphNode ) { //
			.id = ZRGRAPHNODE(sbnode)->id, //
			.obj = ZRARRAYOP_GET(sgraph->nodeObjs, ZRSGRAPH_GRAPH(sgraph)->nodeObjSize, pos), //
			},//
		.nbParents = sbnode->parents->nbObj, //
		.nbChilds = sbnode->childs->nbObj, //
		.parents = &sgraph->parentEdges[sbuilder->build_nbParentEdges],
		.childs = &sgraph->childEdges[sbuilder->build_nbChildEdges],
		};

	if ( ZRSGBNODE_GBNODE(sbnode)->obj != NULL)
		memcpy(ZRSGRAPHNODE_GN(gnode)->obj, ZRSGBNODE_GBNODE(sbnode)->obj, nodeObjSize);

	build_childEdges(sbuilder, sbnode, sgraph, gnode, map_bnodes);
	build_parentEdges(sbuilder, sbnode, sgraph, gnode, map_bnodes);
}

static void fBuilder_build(ZRSimpleGraphBuilder *sbuilder, ZRSimpleGraph *sgraph, ZRMap *bnodesRef)
{
	ZRMap *map_bnodes = ZRVectorMap_create(
		ZRTYPE_SIZE_ALIGNMENT(ZRSimpleGraphBuilderNode*), ZRTYPE_SIZE_ALIGNMENT(ZRSimpleGraphNode*),
		ZRGraphNode_cmp, NULL, sbuilder->allocator,
		ZRVectorMap_modeOrder
	);

	ZRSimpleGraphNode *gnode;
	ZRSimpleGraphBuilderNode *bnode;
	size_t const graphNbNodes = ZRSGRAPHBUILDER_NBNODES(sbuilder);

// Add each node to the graph
	for (size_t i = 0; i < graphNbNodes; i++)
	{
		bnode = *(ZRSimpleGraphBuilderNode**)(ZRVECTOR_GET(sbuilder->pnodes, i));
		gnode = &sgraph->nodes[i];
		ZRMAP_PUT(map_bnodes, &bnode, &gnode);
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
		gnode = &sgraph->nodes[i];
		build_node(sbuilder, bnode, sgraph, gnode, i, map_bnodes);
	}
	ZRMAP_DESTROY(map_bnodes);

// Set to null resting nodes from 'noderef'
// TODO: is it a good strategy ? Best to avoid this ?
	ZRVector *bnodesRef_vec = ZRVectorMap_vector(bnodesRef);

	for (size_t i = 0, c = ZRMAP_NBOBJ(bnodesRef); i < c; i++)
		**(void***)ZRVECTOR_GET(bnodesRef_vec, i) = NULL;

// Reset build values
	sbuilder->build_nbParentEdges = 0;
	sbuilder->build_nbChildEdges = 0;
	ZRVECTOR_DELETE_ALL(sbuilder->build_pedges);

	for (size_t i = 0; i < graphNbNodes; i++)
		(*(ZRSimpleGraphBuilderNode**)(ZRVECTOR_GET(sbuilder->pnodes, i)))->mark = 0;
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
			ZRTYPE_SIZE_ALIGNMENT(size_t), ZRTYPE_SIZE_ALIGNMENT(void*),
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

static void ZRSimpleGraphBuilder_done(ZRGraphBuilder *builder);
static void ZRSimpleGraphBuilder_destroy(ZRGraphBuilder *builder);

static void ZRSimpleGraphBuilderS_init(ZRSimpleGraphBuilderStrategy *strategy)
{
	*strategy = (ZRSimpleGraphBuilderStrategy ) { //
		.graphBuilder = (ZRGraphBuilderStrategy ) { //
			.fnew = fbuilder_new, //
			.fnode = fbuilder_node, //
			.fedge = fbuilder_edge, //
			.fdone = ZRSimpleGraphBuilder_done, //
			} , //
		};
}

static void ZRSimpleGraphBuilder_init(ZRSimpleGraphBuilder *sbuilder, ZRSimpleGraphBuilderStrategy *strategy,
	ZRObjAlignInfos *nodeObjInfos, ZRObjAlignInfos *edgeObjInfos,
	ZRAllocator *allocator
	)
{
	ZRVector *const pnodes = ZRVector2SideStrategy_createDynamic(INITIAL_NB_NODES, ZRTYPE_SIZE_ALIGNMENT(ZRSimpleGraphBuilderNode*), allocator);
	ZRVector *const pedges = ZRVector2SideStrategy_createDynamic(INITIAL_NB_NODES, ZRTYPE_SIZE_ALIGNMENT(ZRSimpleGraphEdge*), allocator);

	ZRMemoryPool *const nodes = ZRMPoolDS_createBS(INITIAL_NB_NODES, ZRTYPE_SIZE_ALIGNMENT(ZRSimpleGraphBuilderNode), allocator);
	ZRMemoryPool *const nodeObjs = ZRMPoolDS_createBS(INITIAL_NB_NODES, ZROBJALIGNINFOS_SIZE_ALIGNMENT(*nodeObjInfos), allocator);
	ZRMemoryPool *const edgeObjs = ZRMPoolDS_createBS(INITIAL_NB_NODES, ZROBJALIGNINFOS_SIZE_ALIGNMENT(*edgeObjInfos), allocator);

	*sbuilder = (ZRSimpleGraphBuilder ) { //
		.graphBuilder = (ZRGraphBuilder ) { //
			.graph = (ZRGraph ) { //
				.strategy = (ZRGraphStrategy*)strategy, //
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

static void ZRSimpleGraphBuilder_done(ZRGraphBuilder *builder)
{
	ZRSimpleGraphBuilder *const sbuilder = ZRSGRAPHBUILDER(builder);
	size_t const nbNodes = sbuilder->pnodes->nbObj;

	for (size_t i = 0; i < nbNodes; i++)
		sbnode_destroy(sbuilder, *(ZRSimpleGraphBuilderNode**)ZRVECTOR_GET(sbuilder->pnodes, i));

	ZRVECTOR_DESTROY(sbuilder->pnodes);
	ZRVECTOR_DESTROY(sbuilder->build_pedges);
	ZRMPOOL_DESTROY(sbuilder->pool_nodes);
	ZRMPOOL_DESTROY(sbuilder->pool_nodeObjs);
	ZRMPOOL_DESTROY(sbuilder->pool_edgeObjs);
}

static void ZRSimpleGraphBuilder_destroy(ZRGraphBuilder *builder)
{
	ZRSimpleGraphBuilder *const sbuilder = ZRSGRAPHBUILDER(builder);

	ZRSimpleGraphBuilder_done(builder);

	ZRFREE(sbuilder->allocator, ZRGB_STRATEGY(builder));
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
	strategy->graphBuilder.fdestroy = ZRSimpleGraphBuilder_destroy;

	node_infos(infos, nodeObjSize, nodeObjAlignment, edgeObjSize, edgeObjAlignment);

	ZRSimpleGraphBuilder *sbuilder = ZRALLOC(allocator, sizeof(ZRSimpleGraphBuilder));
	ZRSimpleGraphBuilder_init(sbuilder, strategy,
		&infos[ZRSimpleGraphBuilderNode_nodeObj],
		&infos[ZRSimpleGraphBuilderNode_edgeObj],
		allocator
		);
	return &ZRSGRAPHBUILDER_GB(sbuilder);
}

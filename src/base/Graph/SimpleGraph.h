/**
 * @author zuri
 * @date samedi 9 mai 2020, 16:37:33 (UTC+0200)
 */

#include <stdalign.h>

#include <zrlib/base/struct.h>
#include <zrlib/base/MemoryPool/MemoryPool.h>

typedef struct ZRSimpleGraphS ZRSimpleGraph;
typedef struct ZRSimpleGraphEdgeS ZRSimpleGraphEdge;
typedef struct ZRSimpleGraphStrategyS ZRSimpleGraphStrategy;
typedef struct ZRSimpleGraphNodeS ZRSimpleGraphNode;

struct ZRSimpleGraphStrategyS
{
	ZRGraphStrategy graph;
};

struct ZRSimpleGraphS
{
	ZRGraph graph;

	ZRAllocator *allocator;

	ZRSimpleGraphNode *nodes;

	ZRSimpleGraphEdge **parentEdges;
	ZRSimpleGraphEdge *childEdges;

	void *nodeObjs;
	void *edgeObjs;
};

#define ZRSGRAPH(SGRAPH) ((ZRSimpleGraph*)(SGRAPH))
#define ZRSGRAPH_GRAPH(SGRAPH)  (&(SGRAPH)->graph)
#define ZRSGRAPH_STRATEGY(SGRAPH) (ZRSimpleGraphStrategy*)(ZRSGRAPH_GRAPH(SGRAPH)->strategy)

// ============================================================================

struct ZRSimpleGraphEdgeS
{
	ZRSimpleGraphNode *a;
	ZRSimpleGraphNode *b;
	void *obj;
};

// ============================================================================

struct ZRSimpleGraphNodeS
{
	ZRGraphNode node;
	size_t nbParents;
	size_t nbChilds;

	ZRSimpleGraphEdge **parents;
	ZRSimpleGraphEdge *childs;

	void *edgeChildsObjs;
};

#define ZRSGRAPHNODE(N) ((ZRSimpleGraphNode*)(N))
#define ZRSGRAPHNODE_GN(SN) ((ZRGraphNode*)(SN))

// ============================================================================

#define ZRSIMPLEGRAPH_INFOS_NB 7

typedef enum
{
	ZRSimpleGraphInfos_base,
	ZRSimpleGraphInfos_nodes,
	ZRSimpleGraphInfos_nodeObjs,
	ZRSimpleGraphInfos_edgeObjs,
	ZRSimpleGraphInfos_childEdges,
	ZRSimpleGraphInfos_parentEdges,
	ZRSimpleGraphInfos_struct,
} ZRSimpleGraphInfos;

static void ZRSimpleGraph_makeInfos(ZRObjAlignInfos *out,
	size_t nbNodes, size_t nodeObjSize, size_t nodeObjAlignment,
	size_t nbEdges, size_t edgeObjSize, size_t edgeObjAlignment
	)
{
	out[ZRSimpleGraphInfos_base] = (ZRObjAlignInfos ) { 0, alignof(ZRSimpleGraph), sizeof(ZRSimpleGraph) };
	out[ZRSimpleGraphInfos_nodes] = (ZRObjAlignInfos ) { 0, alignof(ZRSimpleGraphNode), nbNodes * sizeof(ZRSimpleGraphNode) };
	out[ZRSimpleGraphInfos_nodeObjs] = (ZRObjAlignInfos ) { 0, nodeObjAlignment, nbNodes * nodeObjSize };
	out[ZRSimpleGraphInfos_edgeObjs] = (ZRObjAlignInfos ) { 0, edgeObjAlignment, nbEdges * edgeObjSize };
	out[ZRSimpleGraphInfos_parentEdges] = (ZRObjAlignInfos ) { 0, alignof(ZRSimpleGraphEdge*), nbEdges * sizeof(ZRSimpleGraphEdge*) };
	out[ZRSimpleGraphInfos_childEdges] = (ZRObjAlignInfos ) { 0, alignof(ZRSimpleGraphEdge), nbEdges * sizeof(ZRSimpleGraphEdge) };
	out[ZRSimpleGraphInfos_struct] = (ZRObjAlignInfos ) { };
	ZRStruct_bestOffsetsPos(ZRSIMPLEGRAPH_INFOS_NB - 1, out, 1);
}

// ============================================================================

ZRMUSTINLINE
static inline ZRGraphEdge ZRSimpleGraphEdge_cpyGE(ZRSimpleGraphEdge *se)
{
	ZRGraphEdge ret = { ZRSGRAPHNODE_GN(se->a), ZRSGRAPHNODE_GN(se->b), se->obj };
	return ret;
}

static int fucmp_edge(void *a, void *b, void *data)
{
	ZRSimpleGraphEdge *gea = (ZRSimpleGraphEdge*)a;
	ZRSimpleGraphEdge *geb = *(ZRSimpleGraphEdge**)b;
	return gea == geb;
}

/**
 * Get all the edges of the form ($sa, $sb) not already present in $out.
 */
ZRMUSTINLINE
static inline void ZRSimpleGraph_getEdges(ZRSimpleGraph *sgraph, ZRSimpleGraphNode *sa, ZRSimpleGraphNode *sb, ZRVector *out)
{
	size_t lastNbObj = out->nbObj;

	for (size_t i = 0, c = sa->nbChilds; i < c; i++)
	{
		ZRSimpleGraphEdge *sedge = &sa->childs[i];

		if (sedge->b == sb)
		{
			if (NULL != ZRARRAYOP_SEARCH(out->array, out->objSize, lastNbObj, sedge, fucmp_edge, NULL))
				continue;

			ZRVECTOR_ADD(out, &sb);
		}
	}
}

// ============================================================================

ZRGraph* ZRSimpleGraph_create(
	size_t nbNodes, size_t nodeObjSize, size_t nodeObjAlignment,
	size_t nbEdges, size_t edgeObjSize, size_t edgeObjAlignment,
	ZRAllocator *allocator
	);

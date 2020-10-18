/**
 * @author zuri
 * @date dimanche 1 décembre 2019, 15:06:14 (UTC+0100)
 */

#include <zrlib/base/Graph/Graph.h>
#include <zrlib/base/Algorithm/fcmp.h>
#include <zrlib/base/Map/VectorMap.h>

int ZRGraphNode_cmp(void *a, void *b)
{
	return ZRGRAPHNODE(a)->id - ZRGRAPHNODE(b)->id;
}

int ZRGraphNode_ucmp(void *a, void *b, void *data_unused)
{
	return ZRGRAPHNODE(a)->id - ZRGRAPHNODE(b)->id;
}

// ============================================================================
// BUILDER
// ============================================================================

ZRGraph* ZRGraphBuilder_new(ZRGraphBuilder *builder, void **nodes, size_t nbNodes)
{
	return ZRGRAPHBUILDER_NEW(builder, nodes, nbNodes);
}

ZRGraphBuilderNode* ZRGraphBuilder_node(ZRGraphBuilder *builder, void *nodeData)
{
	return ZRGRAPHBUILDER_NODE(builder, nodeData);
}

void ZRGraphBuilder_edge(ZRGraphBuilder *builder, ZRGraphBuilderNode *a, ZRGraphBuilderNode *b, void *edgeData)
{
	return ZRGRAPHBUILDER_EDGE(builder, a, b, edgeData);
}

// ============================================================================
// GRAPH
// ============================================================================

void ZRGraph_done(ZRGraph *graph)
{
	ZRGRAPH_DONE(graph);
}

void ZRGraph_destroy(ZRGraph *graph)
{
	ZRGRAPH_DESTROY(graph);
}

size_t ZRGraph_getNNodes(ZRGraph *graph, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut)
{
	return ZRGRAPH_GETNNODES(graph, nodes_out, offset, maxNbOut);
}

size_t ZRGraph_cpyNEdges(ZRGraph *graph, ZRGraphEdge *cpyTo, size_t offset, size_t maxNbCpy)
{
	return ZRGRAPH_CPYNEDGES(graph, cpyTo, offset, maxNbCpy);
}

// ============================================================================
// NODE
// ============================================================================

size_t ZRGraphNode_getId(ZRGraphNode *node)
{
	return ZRGRAPHNODE_GETID(node);
}

void* ZRGraphNode_getObj(ZRGraphNode *node)
{
	return ZRGRAPHNODE_GETOBJ(node);
}

size_t ZRGraphNode_getNbParents(ZRGraph *graph, ZRGraphNode *node)
{
	return ZRGRAPHNODE_GETNBPARENTS(graph, node);
}

size_t ZRGraphNode_getNbChilds(ZRGraph *graph, ZRGraphNode *node)
{
	return ZRGRAPHNODE_GETNBCHILDS(graph, node);
}

size_t ZRGraphNode_getNParents(ZRGraph *graph, ZRGraphNode *node, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut)
{
	return ZRGRAPHNODE_GETNPARENTS(graph, node, nodes_out, offset, maxNbOut);
}

size_t ZRGraphNode_getNChilds(ZRGraph *graph, ZRGraphNode *node, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut)
{
	return ZRGRAPHNODE_GETNCHILDS(graph, node, nodes_out, offset, maxNbOut);
}

size_t ZRGraphNode_getNbEdges(ZRGraph *graph, ZRGraphNode *node, enum ZRGraphEdge_selectE select)
{
	return ZRGRAPHNODE_GETNBEDGES(graph, node, select);
}

size_t ZRGraphNode_cpyNEdges(ZRGraph *graph, ZRGraphNode *node, ZRGraphEdge *cpyTo, size_t offset, size_t maxNbCpy, enum ZRGraphEdge_selectE select)
{
	return ZRGRAPHNODE_CPYNEDGES(graph, node, cpyTo, offset, maxNbCpy, select);
}

// ============================================================================
// HELP
// ============================================================================

ZRGraphNode* ZRGraphNode_getParent(ZRGraph *graph, ZRGraphNode *node, size_t offset)
{
	return ZRGRAPHNODE_GETPARENT(graph, node, offset);
}

ZRGraphNode* ZRGraphNode_getChild(ZRGraph *graph, ZRGraphNode *node, size_t offset)
{
	return ZRGRAPHNODE_GETCHILD(graph, node, offset);
}

ZRGraphEdge ZRGraphEdge_cpy(ZRGraph *graph, ZRGraphNode *a, size_t offset, enum ZRGraphEdge_selectE select)
{
	return ZRGRAPHEDGE_CPY(graph, a, offset, select);
}

ZRMUSTINLINE
static inline void graphBuilderNode_addEdges(ZRGraphBuilder *to, ZRGraph *from, ZRGraphNode *node_from, ZRMap *map_bnodes, ZRGraphEdge *edgeBuffer, size_t edgeBufferSize)
{
	size_t edgeOffset = 0;
	size_t nbEdgesCpy;
	do
	{
		nbEdgesCpy = ZRGRAPHNODE_CPYNEDGES(from, node_from, edgeBuffer, edgeOffset, edgeBufferSize, ZRGraphEdge_selectOUT);

		for (size_t i = 0; i < nbEdgesCpy; i++)
		{
			ZRGraphEdge edge = edgeBuffer[i];
			ZRGRAPHBUILDER_EDGE(to,
				*(ZRGraphNode**)ZRMAP_GET(map_bnodes, &edge.a->id),
				*(ZRGraphNode**)ZRMAP_GET(map_bnodes, &edge.b->id),
				edge.obj
				);
		}
		edgeOffset += edgeBufferSize;
	} while (nbEdgesCpy == edgeBufferSize);
}

void ZRGraphBuilder_cpyGraph(ZRGraphBuilder *to, ZRGraph *from, ZRGraphNode **refNodes, size_t nbNodes, ZRAllocator *allocator)
{
	assert(ZRGB_GRAPH(to)->nodeObjSize == from->nodeObjSize);
	assert(ZRGB_GRAPH(to)->nodeObjAlignment == from->nodeObjAlignment);
	assert(ZRGB_GRAPH(to)->edgeObjSize == from->edgeObjSize);
	assert(ZRGB_GRAPH(to)->edgeObjAlignment == from->edgeObjAlignment);

	if (from->nbNodes == 0)
		return;

	// TODO: node with int identifier for ordered map
	// Key : size_t Value : *void
	ZRMap *map_refNodes = ZRVectorMap_create(
		ZRTYPE_OBJINFOS(size_t), ZRTYPE_OBJINFOS(ZRGraphNode**),
		zrfcmp_size_t, NULL, allocator,
		ZRVectorMap_modeOrder
		);
	ZRMap *map_bnodes = ZRVectorMap_create(
		ZRTYPE_OBJINFOS(size_t), ZRTYPE_OBJINFOS(ZRGraphNode*),
		zrfcmp_size_t, NULL, allocator,
		ZRVectorMap_modeOrder
		);

	/* Register the ref nodes */
	for (size_t i = 0; i < nbNodes; i++)
	{
		ZRGraphNode **ref = &refNodes[i];
		ZRMAP_PUT(map_refNodes, &refNodes[i]->id, &ref);
	}

	ZRGraphBuilderNode *gnode_bnode[from->nbNodes];
	ZRGraphNode *nodeBuffer[ZRGRAPH_NODEBUFFER_SIZE];
	size_t const nodeBufferSize = ZRCARRAY_NBOBJ(nodeBuffer);
	size_t nodeOffset = 0;
	size_t nbNodesGet;

	/* Add nodes */
	do
	{
		nbNodesGet = ZRGRAPH_GETNNODES(from, nodeBuffer, nodeOffset, nodeBufferSize);

		for (size_t i = 0; i < nbNodesGet; i++)
		{
			ZRGraphNode *node_from = nodeBuffer[nodeOffset + i];
			ZRGraphBuilderNode *node_new = ZRGRAPHBUILDER_NODE(to, node_from->obj);
			ZRMAP_PUT(map_bnodes, &node_from->id, &node_new);

			/* Search if we have a ref node */
			ZRGraphNode ***ref = ZRMAP_GET(map_refNodes, &node_from->id);

			if (ref != NULL)
			{
				**ref = node_new;
				ZRMAP_DELETE(map_refNodes, node_from);
			}
		}
		nodeOffset += nodeBufferSize;
	} while (nbNodesGet == nodeBufferSize);

	ZRGraphEdge edgeBuffer[ZRGRAPH_EDGEBUFFER_SIZE];
	size_t edgeBufferSize = ZRCARRAY_NBOBJ(edgeBuffer);
	nodeOffset = 0;

	/* Add edges */
	do
	{
		nbNodesGet = ZRGRAPH_GETNNODES(from, nodeBuffer, nodeOffset, nodeBufferSize);

		for (size_t i = 0; i < nbNodesGet; i++)
			graphBuilderNode_addEdges(to, from, nodeBuffer[nodeOffset + i], map_bnodes, edgeBuffer, edgeBufferSize);

		nodeOffset += nodeBufferSize;
	} while (nbNodesGet == nodeBufferSize);

	/* Set all remaining ref nodes to NULL */
	// TODO: best behaviour
	{
		ZRVector *refVec = ZRVectorMap_vector(map_refNodes);

		for (size_t i = 0, c = ZRVECTOR_NBOBJ(refVec); i < c; i++)
			**(ZRGraphNode***)ZRVECTOR_GET(refVec, i) = NULL;
	}
	ZRMAP_DESTROY(map_bnodes);
	ZRMAP_DESTROY(map_refNodes);
}

/**
 * @author zuri
 * @date dimanche 1 décembre 2019, 15:06:14 (UTC+0100)
 */

#include <zrlib/base/Graph/Graph.h>

// ============================================================================
// BUILDER
// ============================================================================

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

void* ZRGraphNode_getObj(ZRGraph *graph, ZRGraphNode *node)
{
	return ZRGRAPHNODE_GETOBJ(graph, node);
}

ZRGraphNode* ZRGraphNode_getParent(ZRGraph *graph, ZRGraphNode *node, size_t pos)
{
	return ZRGRAPHNODE_GETPARENT(graph, node, pos);
}

ZRGraphNode* ZRGraphNode_getChild(ZRGraph *graph, ZRGraphNode *node, size_t pos)
{
	return ZRGRAPHNODE_GETCHILD(graph, node, pos);
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

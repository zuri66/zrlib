/**
 * @author zuri
 * @date dimanche 1 décembre 2019, 15:06:14 (UTC+0100)
 */

#include <zrlib/base/Graph/Graph.h>

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

ZRGraphEdge ZRGraphEdge_cpy(ZRGraph *graph, ZRGraphNode *a, ZRGraphNode *b, size_t offset, enum ZRGraphEdge_selectE select)
{
	return ZRGRAPHEDGE_CPY(graph, a, b, offset, select);
}

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

size_t ZRGraph_objSize(ZRGraph *graph)
{
	return ZRGRAPH_OBJSIZE(graph);
}

size_t ZRGraph_getNbNodes(ZRGraph *graph)
{
	return ZRGRAPH_GETNBNODES(graph);
}

size_t ZRGraph_getNNodes(ZRGraph *graph, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut)
{
	return ZRGRAPH_GETNNODES(graph, nodes_out, offset, maxNbOut);
}

size_t ZRGraph_getNObjs(ZRGraph *graph, void *objs_out, size_t offset, size_t maxNbOut)
{
	return ZRGRAPH_GETNOBJS(graph, objs_out, offset, maxNbOut);
}

// ============================================================================
// EDGE
// ============================================================================

void* ZRGraphEdge_getObj(ZRGraph *graph, ZRGraphNode *a, ZRGraphNode *b)
{
	return ZRGRAPHEDGE_GETOBJ(graph, a, b);
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

size_t ZRGraphNode_getNObjs(ZRGraph *graph, ZRGraphNode *node, void *objs_out, size_t offset, size_t maxNbOut)
{
	return ZRGRAPHNODE_GETNOBJS(graph, node, objs_out, offset, maxNbOut);
}
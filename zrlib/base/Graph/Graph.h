/**
 * @author zuri
 * @date dimanche 1 décembre 2019, 14:15:50 (UTC+0100)
 */

#ifndef GRAPH_H
#define GRAPH_H

#include <zrlib/syntax_pad.h>

#include <stddef.h>

// ============================================================================

typedef struct ZRGraphS ZRGraph;
typedef struct ZRGraphEdgeS ZRGraphEdge;
typedef struct ZRGraphStrategyS ZRGraphStrategy;
typedef void ZRGraphNode;

// ============================================================================

typedef void* ______ (*ZRGraphEdge_fgetObj_t)(_______ ZRGraph*, ZRGraphNode *a, ZRGraphNode *b);

typedef void* ______ (*ZRGraphNode_fgetObj_t)(_______ ZRGraph*, ZRGraphNode*);
typedef ZRGraphNode* (*ZRGraphNode_fgetParent_t)(____ ZRGraph*, ZRGraphNode*, size_t pos);
typedef ZRGraphNode* (*ZRGraphNode_fgetChild_t)(_____ ZRGraph*, ZRGraphNode*, size_t pos);
typedef size_t _____ (*ZRGraphNode_fgetNbParents_t)(_ ZRGraph*, ZRGraphNode*);
typedef size_t _____ (*ZRGraphNode_fgetNbChilds_t)(__ ZRGraph*, ZRGraphNode*);
typedef size_t _____ (*ZRGraphNode_fgetNParents_t)(__ ZRGraph*, ZRGraphNode*, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut);
typedef size_t _____ (*ZRGraphNode_fgetNChilds_t)(___ ZRGraph*, ZRGraphNode*, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut);

typedef size_t (*ZRGraph_fgetNNodes_t)(_ ZRGraph*, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut);
typedef void _ (*ZRGraph_fdone_t)(______ ZRGraph*);
typedef void _ (*ZRGraph_fdestroy_t)(___ ZRGraph*);

#define ZRGRAPHSTRATEGY_MEMBERS() \
	size_t (*fsdataSize)(ZRGraph *graph); \
	size_t (*fstrategySize)(void); \
	\
	ZRGraphEdge_fgetObj_t fEdgeGetObj; \
	\
	ZRGraphNode_fgetObj_t fNodeGetObj; \
	ZRGraphNode_fgetParent_t fNodeGetParent; \
	ZRGraphNode_fgetChild_t fNodeGetChild; \
	ZRGraphNode_fgetNbParents_t fNodeGetNbParents; \
	ZRGraphNode_fgetNbChilds_t fNodeGetNbChilds; \
	ZRGraphNode_fgetNParents_t fNodeGetNParents; \
	ZRGraphNode_fgetNChilds_t fNodeGetNChilds; \
	\
	ZRGraph_fgetNNodes_t fgetNNodes; \
	ZRGraph_fdone_t fdone; \
	\
	/** @Optional */ \
	ZRGraph_fdestroy_t fdestroy

struct ZRGraphStrategyS
{
	ZRGRAPHSTRATEGY_MEMBERS()
	;
};

#define ZRGRAPH_MEMBERS(TYPE_STRATEGY) \
	size_t nbNodes; \
	size_t nbEdges; \
	\
	TYPE_STRATEGY *strategy; \

struct ZRGraphS
{
	ZRGRAPH_MEMBERS(ZRGraphStrategy);
};

struct ZRGraphEdgeS
{
	ZRGraphNode *a;
	ZRGraphNode *b;
};

// ============================================================================

size_t ZRGraph_getNbNodes(ZRGraph *graph);
size_t ZRGraph_getNbEdges(ZRGraph *graph);

void ZRGraph_done(__ ZRGraph *graph);
void ZRGraph_destroy(ZRGraph *graph);

size_t ZRGraph_getNbNodes(_ ZRGraph *graph);
size_t ZRGraph_getNNodes(__ ZRGraph *graph, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut);

// ============================================================================
// EDGE
// ============================================================================

void* ZRGraphEdge_getObj(ZRGraph *graph, ZRGraphNode *a, ZRGraphNode *b);

// ============================================================================
// NODE
// ============================================================================

void* ______ ZRGraphNode_getObj(_______ ZRGraph *graph, ZRGraphNode *node);
ZRGraphNode* ZRGraphNode_getParent(____ ZRGraph *graph, ZRGraphNode *node, size_t pos);
ZRGraphNode* ZRGraphNode_getChild(_____ ZRGraph *graph, ZRGraphNode *node, size_t pos);
size_t _____ ZRGraphNode_getNbParents(_ ZRGraph *graph, ZRGraphNode *node);
size_t _____ ZRGraphNode_getNbChilds(__ ZRGraph *graph, ZRGraphNode *node);
size_t _____ ZRGraphNode_getNParents(__ ZRGraph *graph, ZRGraphNode *node, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut);
size_t _____ ZRGraphNode_getNChilds(___ ZRGraph *graph, ZRGraphNode *node, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut);

// ============================================================================



// ============================================================================
// TREE
// ============================================================================

static inline void ZRGRAPH_DONE(ZRGraph *graph)
{
	graph->strategy->fdone(graph);
}

static inline void ZRGRAPH_DESTROY(ZRGraph *graph)
{
	if (graph->strategy->fdestroy)
		graph->strategy->fdestroy(graph);
}

static inline size_t ZRGRAPH_GETNBNODES(ZRGraph *graph)
{
	return graph->nbNodes;
}

static inline size_t ZRGRAPH_GETNNODES(ZRGraph *graph, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut)
{
	return graph->strategy->fgetNNodes(graph, nodes_out, offset, maxNbOut);
}
}

// ============================================================================
// EDGE
// ============================================================================

static inline void* ZRGRAPHEDGE_GETOBJ(ZRGraph *graph, ZRGraphNode *a, ZRGraphNode *b)
{
	return graph->strategy->fEdgeGetObj(graph, a, b);
}

// ============================================================================
// NODE
// ============================================================================

static inline void* ZRGRAPHNODE_GETOBJ(ZRGraph *graph, ZRGraphNode *node)
{
	return graph->strategy->fNodeGetObj(graph, node);
}

static inline ZRGraphNode* ZRGRAPHNODE_GETPARENT(ZRGraph *graph, ZRGraphNode *node, size_t pos)
{
	return graph->strategy->fNodeGetParent(graph, node, pos);
}

static inline ZRGraphNode* ZRGRAPHNODE_GETCHILD(ZRGraph *graph, ZRGraphNode *node, size_t pos)
{
	return graph->strategy->fNodeGetChild(graph, node, pos);
}

static inline size_t ZRGRAPHNODE_GETNBPARENTS(ZRGraph *graph, ZRGraphNode *node)
{
	return graph->strategy->fNodeGetNbParents(graph, node);
}

static inline size_t ZRGRAPHNODE_GETNBCHILDS(ZRGraph *graph, ZRGraphNode *node)
{
	return graph->strategy->fNodeGetNbChilds(graph, node);
}

static inline size_t ZRGRAPHNODE_GETNPARENTS(ZRGraph *graph, ZRGraphNode *node, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut)
{
	return graph->strategy->fNodeGetNParents(graph, node, nodes_out, offset, maxNbOut);
}

static inline size_t ZRGRAPHNODE_GETNCHILDS(ZRGraph *graph, ZRGraphNode *node, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut)
{
	return graph->strategy->fNodeGetNChilds(graph, node, nodes_out, offset, maxNbOut);
}
}

#endif

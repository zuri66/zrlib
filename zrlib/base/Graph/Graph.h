/**
 * @author zuri
 * @date dimanche 1 décembre 2019, 14:15:50 (UTC+0100)
 */

#ifndef GRAPH_H
#define GRAPH_H

#include <zrlib/syntax_pad.h>

#include <stddef.h>

// ============================================================================

typedef struct ZRGraphS _______ ZRGraph;
typedef struct ZRGraphEdgeS ___ ZRGraphEdge;
typedef struct ZRGraphStrategyS ZRGraphStrategy;
typedef struct ZRGraphNodeS ___ ZRGraphNode;

// ============================================================================

typedef void* ______ (*ZRGraphEdge_fgetObj_t)(_______ ZRGraph*, ZRGraphNode *a, ZRGraphNode *b);

typedef void* ______ (*ZRGraphNode_fgetObj_t)(_______ ZRGraph*, ZRGraphNode*);
typedef ZRGraphNode* (*ZRGraphNode_fgetParent_t)(____ ZRGraph*, ZRGraphNode*, size_t pos);
typedef ZRGraphNode* (*ZRGraphNode_fgetChild_t)(_____ ZRGraph*, ZRGraphNode*, size_t pos);
typedef size_t _____ (*ZRGraphNode_fgetNbParents_t)(_ ZRGraph*, ZRGraphNode*);
typedef size_t _____ (*ZRGraphNode_fgetNbChilds_t)(__ ZRGraph*, ZRGraphNode*);
typedef size_t _____ (*ZRGraphNode_fgetNParents_t)(__ ZRGraph*, ZRGraphNode*, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut);
typedef size_t _____ (*ZRGraphNode_fgetNChilds_t)(___ ZRGraph*, ZRGraphNode*, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut);
typedef size_t _____ (*ZRGraphNode_fgetNObjs_t)(_____ ZRGraph*, ZRGraphNode*, ZRGraphNode *objs_out, _ size_t offset, size_t maxNbOutBytes);

typedef size_t (*ZRGraph_fgetNNodes_t)(_ ZRGraph*, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut);
typedef size_t (*ZRGraph_fgetNObjs_t)(__ ZRGraph*, ________ void *objs_out, size_t offset, size_t maxNbOutBytes);
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
	ZRGraphNode_fgetNObjs_t fNodeGetNObjs; \
	\
	ZRGraph_fgetNNodes_t fgetNNodes; \
	ZRGraph_fgetNObjs_t fgetNObjs; \
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
	size_t objSize; \
	\
	TYPE_STRATEGY *strategy

struct ZRGraphS
{
	ZRGRAPH_MEMBERS(ZRGraphStrategy);
};

#define ZRGRAPHNODE_MEMBERS()

struct SRGraphNodeS
{
	ZRGRAPHNODE_MEMBERS();
};

struct ZRGraphEdgeS
{
	ZRGraphNode *a;
	ZRGraphNode *b;
};

// ============================================================================

size_t ZRGraph_objSize(ZRGraph *graph);
size_t ZRGraph_getNbNodes(ZRGraph *graph);
size_t ZRGraph_getNbEdges(ZRGraph *graph);

void ZRGraph_done(__ ZRGraph *graph);
void ZRGraph_destroy(ZRGraph *graph);

size_t ZRGraph_getNbNodes(_ ZRGraph *graph);
size_t ZRGraph_getNNodes(__ ZRGraph *graph, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut);
size_t ZRGraph_getNObjs(___ ZRGraph *graph, void ______ __*objs_out, size_t offset, size_t maxNbOut);

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
size_t _____ ZRGraphNode_getNObjs(_____ ZRGraph *graph, ZRGraphNode *node, ________ void *objs_out, size_t offset, size_t maxNbOutBytes);

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

static inline size_t ZRGRAPH_OBJSIZE(ZRGraph *graph)
{
	return graph->objSize;
}

static inline size_t ZRGRAPH_GETNBNODES(ZRGraph *graph)
{
	return graph->nbNodes;
}

static inline size_t ZRGRAPH_GETNNODES(ZRGraph *graph, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut)
{
	return graph->strategy->fgetNNodes(graph, nodes_out, offset, maxNbOut);
}

static inline size_t ZRGRAPH_GETNOBJS(ZRGraph *graph, void *objs_out, size_t offset, size_t maxNbOut)
{
	return graph->strategy->fgetNObjs(graph, objs_out, offset, maxNbOut);
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

static inline size_t ZRGRAPHNODE_GETNOBJS(ZRGraph *graph, ZRGraphNode *node, void *objs_out, size_t offset, size_t maxNbOutBytes)
{
	return graph->strategy->fNodeGetNObjs(graph, node, objs_out, offset, maxNbOutBytes);
}

#endif

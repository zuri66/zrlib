/**
 * @author zuri
 * @date dimanche 1 décembre 2019, 14:15:50 (UTC+0100)
 */

#ifndef ZRGRAPH_H
#define ZRGRAPH_H

#include <zrlib/config.h>
#include <zrlib/syntax_pad.h>
#include "GraphBuilder.h"

#include <stddef.h>

// ============================================================================

typedef struct ZRGraphS _______ ZRGraph;
typedef struct ZRGraphEdgeS ___ ZRGraphEdge;
typedef struct ZRGraphStrategyS ZRGraphStrategy;
typedef struct ZRGraphNodeS ___ ZRGraphNode;

// ============================================================================

enum ZRGraphEdge_selectE
{
	ZRGraphEdge_selectIN = 1, ZRGraphEdge_selectOUT = 2, ZRGraphEdge_selectINOUT = 3
};

struct ZRGraphStrategyS
{
	size_t (*fstrategySize)(void);

	size_t (*fnode_getNbEdges)(ZRGraph*, ZRGraphNode*, enum ZRGraphEdge_selectE);

	void* ______ (*fnode_getObj)(_______ ZRGraph*, ZRGraphNode*);
	ZRGraphNode* (*fnode_getParent)(____ ZRGraph*, ZRGraphNode*, size_t pos);
	ZRGraphNode* (*fnode_getChild)(_____ ZRGraph*, ZRGraphNode*, size_t pos);
	size_t _____ (*fnode_getNbParents)(_ ZRGraph*, ZRGraphNode*);
	size_t _____ (*fnode_getNbChilds)(__ ZRGraph*, ZRGraphNode*);
	size_t _____ (*fnode_getNParents)(__ ZRGraph*, ZRGraphNode*, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut);
	size_t _____ (*fnode_getNChilds)(___ ZRGraph*, ZRGraphNode*, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut);
	size_t _____ (*fnode_getNObjs)(_____ ZRGraph*, ZRGraphNode*, void *objs_out, ________ size_t offset, size_t maxNbOutBytes);
	size_t _____ (*fnode_cpyNEdges)(____ ZRGraph*, ZRGraphNode*, ZRGraphEdge *cpyTo, ____ size_t offset, size_t maxNbCpy, enum ZRGraphEdge_selectE);

	size_t (*fcpyNEdges)(_ ZRGraph*, ZRGraphEdge *cpyTo, ____ size_t offset, size_t maxNbCpy);
	size_t (*fgetNNodes)(_ ZRGraph*, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut);
	size_t (*fgetNObjs)(__ ZRGraph*, ________ void *objs_out, size_t offset, size_t maxNbOutBytes);
	void _ (*fdone)(______ ZRGraph*);
	void _ (*fdestroy)(___ ZRGraph*);
};

struct ZRGraphS
{
	size_t nbNodes;
	size_t nbEdges;
	size_t objSize;
	size_t objAlignment;

	ZRGraphStrategy *strategy;
};

struct SRGraphNodeS
{
};

struct ZRGraphEdgeS
{
	ZRGraphNode *a;
	ZRGraphNode *b;
	void *obj;
};

// ============================================================================

size_t ZRGraph_objSize(ZRGraph *graph);
size_t ZRGraph_getNbNodes(ZRGraph *graph);
size_t ZRGraph_getNbEdges(ZRGraph *graph);

void ZRGraph_done(__ ZRGraph *graph);
void ZRGraph_destroy(ZRGraph *graph);

size_t ZRGraph_getNbNodes(_ ZRGraph *graph);
size_t ZRGraph_getNNodes(__ ZRGraph *graph, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut);
size_t ZRGraph_getNObjs(___ ZRGraph *graph, void ______ __ *objs_out, size_t offset, size_t maxNbOut);
size_t ZRGraph_cpyNEdges(__ ZRGraph *graph, ZRGraphEdge *cpyTo, _____ size_t offset, size_t maxNbCpy);

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
size_t _____ ZRGraphNode_getNbEdges(___ ZRGraph *graph, ZRGraphNode *node, enum ZRGraphEdge_selectE select);
size_t _____ ZRGraphNode_cpyNEdges(____ ZRGraph *graph, ZRGraphNode *node, ZRGraphEdge *cpyTo, ____ size_t offset, size_t maxNbCpy, enum ZRGraphEdge_selectE select);

// ============================================================================
// TREE
// ============================================================================

ZRMUSTINLINE
static inline void ZRGRAPH_DONE(ZRGraph *graph)
{
	graph->strategy->fdone(graph);
}

ZRMUSTINLINE
static inline void ZRGRAPH_DESTROY(ZRGraph *graph)
{
	graph->strategy->fdestroy(graph);
}

ZRMUSTINLINE
static inline size_t ZRGRAPH_OBJSIZE(ZRGraph *graph)
{
	return graph->objSize;
}

ZRMUSTINLINE
static inline size_t ZRGRAPH_GETNBNODES(ZRGraph *graph)
{
	return graph->nbNodes;
}

ZRMUSTINLINE
inline size_t ZRGRAPH_GETNBEDGES(ZRGraph *graph)
{
	return graph->nbEdges;
}

ZRMUSTINLINE
static inline size_t ZRGRAPH_GETNNODES(ZRGraph *graph, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut)
{
	return graph->strategy->fgetNNodes(graph, nodes_out, offset, maxNbOut);
}

ZRMUSTINLINE
static inline size_t ZRGRAPH_GETNOBJS(ZRGraph *graph, void *objs_out, size_t offset, size_t maxNbOut)
{
	return graph->strategy->fgetNObjs(graph, objs_out, offset, maxNbOut);
}

ZRMUSTINLINE
inline size_t ZRGRAPH_CPYNEDGES(ZRGraph *graph, ZRGraphEdge *cpyTo, size_t offset, size_t maxNbCpy)
{
	return graph->strategy->fcpyNEdges(graph, cpyTo, offset, maxNbCpy);
}

// ============================================================================
// NODE
// ============================================================================

ZRMUSTINLINE
static inline void* ZRGRAPHNODE_GETOBJ(ZRGraph *graph, ZRGraphNode *node)
{
	return graph->strategy->fnode_getObj(graph, node);
}

ZRMUSTINLINE
static inline ZRGraphNode* ZRGRAPHNODE_GETPARENT(ZRGraph *graph, ZRGraphNode *node, size_t pos)
{
	return graph->strategy->fnode_getParent(graph, node, pos);
}

ZRMUSTINLINE
static inline ZRGraphNode* ZRGRAPHNODE_GETCHILD(ZRGraph *graph, ZRGraphNode *node, size_t pos)
{
	return graph->strategy->fnode_getChild(graph, node, pos);
}

ZRMUSTINLINE
static inline size_t ZRGRAPHNODE_GETNBPARENTS(ZRGraph *graph, ZRGraphNode *node)
{
	return graph->strategy->fnode_getNbParents(graph, node);
}

ZRMUSTINLINE
static inline size_t ZRGRAPHNODE_GETNBCHILDS(ZRGraph *graph, ZRGraphNode *node)
{
	return graph->strategy->fnode_getNbChilds(graph, node);
}

ZRMUSTINLINE
static inline size_t ZRGRAPHNODE_GETNPARENTS(ZRGraph *graph, ZRGraphNode *node, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut)
{
	return graph->strategy->fnode_getNParents(graph, node, nodes_out, offset, maxNbOut);
}

ZRMUSTINLINE
static inline size_t ZRGRAPHNODE_GETNCHILDS(ZRGraph *graph, ZRGraphNode *node, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut)
{
	return graph->strategy->fnode_getNChilds(graph, node, nodes_out, offset, maxNbOut);
}

ZRMUSTINLINE
static inline size_t ZRGRAPHNODE_GETNOBJS(ZRGraph *graph, ZRGraphNode *node, void *objs_out, size_t offset, size_t maxNbOutBytes)
{
	return graph->strategy->fnode_getNObjs(graph, node, objs_out, offset, maxNbOutBytes);
}

ZRMUSTINLINE
inline size_t ZRGRAPHNODE_GETNBEDGES(ZRGraph *graph, ZRGraphNode *node, enum ZRGraphEdge_selectE select)
{
	return graph->strategy->fnode_getNbEdges(graph, node, select);
}

ZRMUSTINLINE
inline size_t ZRGRAPHNODE_CPYNEDGES(ZRGraph *graph, ZRGraphNode *node, ZRGraphEdge *cpyTo, size_t offset, size_t maxNbCpy, enum ZRGraphEdge_selectE select)
{
	return graph->strategy->fnode_cpyNEdges(graph, node, cpyTo, offset, maxNbCpy, select);
}

#endif

/**
 * @author zuri
 * @date dimanche 1 décembre 2019, 14:15:50 (UTC+0100)
 */

#ifndef ZRGRAPH_H
#define ZRGRAPH_H

#include <zrlib/config.h>
#include <zrlib/syntax_pad.h>

#include <stddef.h>

// ============================================================================

typedef struct ZRGraphS _______ ZRGraph;
typedef struct ZRGraphEdgeS ___ ZRGraphEdge;
typedef struct ZRGraphStrategyS ZRGraphStrategy;
typedef struct ZRGraphNodeS ___ ZRGraphNode;

#define ZRGRAPH_EDGEBUFFER_SIZE 512
#define ZRGRAPH_NODEBUFFER_SIZE 512

// ============================================================================

enum ZRGraphEdge_selectE
{
	ZRGraphEdge_selectIN = 1, ZRGraphEdge_selectOUT = 2, ZRGraphEdge_selectINOUT = 3
};

struct ZRGraphStrategyS
{
	size_t (*fstrategySize)(void);

	size_t _____ (*fnode_getNbParents)(_ ZRGraph*, ZRGraphNode*);
	size_t _____ (*fnode_getNbChilds)(__ ZRGraph*, ZRGraphNode*);
	size_t _____ (*fnode_getNbEdges)(___ ZRGraph*, ZRGraphNode*, enum ZRGraphEdge_selectE);

	size_t _____ (*fnode_getNParents)(__ ZRGraph*, ZRGraphNode*, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut);
	size_t _____ (*fnode_getNChilds)(___ ZRGraph*, ZRGraphNode*, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut);
	size_t _____ (*fnode_cpyNEdges)(____ ZRGraph*, ZRGraphNode*, ZRGraphEdge *cpyTo, ____ size_t offset, size_t maxNbCpy, enum ZRGraphEdge_selectE);

	size_t (*fgetNNodes)(_ ZRGraph*, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut);
	size_t (*fcpyNEdges)(_ ZRGraph*, ZRGraphEdge *cpyTo, ____ size_t offset, size_t maxNbCpy);
	void _ (*fdone)(______ ZRGraph*);
	void _ (*fdestroy)(___ ZRGraph*);
};

struct ZRGraphS
{
	ZRGraphStrategy *strategy;

	size_t nbNodes;
	size_t nbEdges;

	// Node
	size_t nodeObjSize;
	size_t nodeObjAlignment;

	// Edge
	size_t edgeObjSize;
	size_t edgeObjAlignment;
};

struct ZRGraphNodeS
{
	/*
	 * Unique identifier for a node.
	 * Needed to make an order on nodes.
	 */
	size_t id;

	void *obj;
};

struct ZRGraphEdgeS
{
	ZRGraphNode *a;
	ZRGraphNode *b;
	void *obj;
};

#define ZRGRAPHNODE(N) ((ZRGraphNode*)(N))

// ============================================================================

void ZRGraph_done(__ ZRGraph *graph);
void ZRGraph_destroy(ZRGraph *graph);

size_t ZRGraph_getNNodes(__ ZRGraph *graph, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut);
size_t ZRGraph_cpyNEdges(__ ZRGraph *graph, ZRGraphEdge *cpyTo, _____ size_t offset, size_t maxNbCpy);

// ============================================================================
// NODE
// ============================================================================

int ZRGraphNode_cmp(void *a, void *b);
int ZRGraphNode_ucmp(void *a, void *b, void *data_unused);

size_t _ ZRGraphNode_getId(__ ZRGraphNode *node);
void* __ ZRGraphNode_getObj(_ ZRGraphNode *node);

size_t _____ ZRGraphNode_getNbParents(_ ZRGraph *graph, ZRGraphNode *node);
size_t _____ ZRGraphNode_getNbChilds(__ ZRGraph *graph, ZRGraphNode *node);
size_t _____ ZRGraphNode_getNbEdges(___ ZRGraph *graph, ZRGraphNode *node, enum ZRGraphEdge_selectE select);

size_t _____ ZRGraphNode_getNParents(__ ZRGraph *graph, ZRGraphNode *node, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut);
size_t _____ ZRGraphNode_getNChilds(___ ZRGraph *graph, ZRGraphNode *node, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut);
size_t _____ ZRGraphNode_cpyNEdges(____ ZRGraph *graph, ZRGraphNode *node, ZRGraphEdge *cpyTo, ____ size_t offset, size_t maxNbCpy, enum ZRGraphEdge_selectE select);

// Help

ZRGraphNode* ZRGraphNode_getParent(_ ZRGraph *graph, ZRGraphNode *node, size_t offset);
ZRGraphNode* ZRGraphNode_getChild(__ ZRGraph *graph, ZRGraphNode *node, size_t offset);

ZRGraphEdge ZRGraphEdge_cpy(________ ZRGraph *graph, ZRGraphNode *a, ZRGraphNode *b, size_t offset, enum ZRGraphEdge_selectE select);

// ============================================================================
// GRAPH
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
static inline size_t ZRGRAPH_GETNNODES(ZRGraph *graph, ZRGraphNode **nodes_out, size_t offset, size_t maxNbOut)
{
	return graph->strategy->fgetNNodes(graph, nodes_out, offset, maxNbOut);
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
static inline size_t ZRGRAPHNODE_GETID(ZRGraphNode *node)
{
	return node->id;
}

ZRMUSTINLINE
static inline void* ZRGRAPHNODE_GETOBJ(ZRGraphNode *node)
{
	return node->obj;
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
inline size_t ZRGRAPHNODE_GETNBEDGES(ZRGraph *graph, ZRGraphNode *node, enum ZRGraphEdge_selectE select)
{
	return graph->strategy->fnode_getNbEdges(graph, node, select);
}

ZRMUSTINLINE
inline size_t ZRGRAPHNODE_CPYNEDGES(ZRGraph *graph, ZRGraphNode *node, ZRGraphEdge *cpyTo, size_t offset, size_t maxNbCpy, enum ZRGraphEdge_selectE select)
{
	return graph->strategy->fnode_cpyNEdges(graph, node, cpyTo, offset, maxNbCpy, select);
}

// ============================================================================
// HELP
// ============================================================================

ZRMUSTINLINE
static inline ZRGraphNode* ZRGRAPHNODE_GETPARENT(ZRGraph *graph, ZRGraphNode *node, size_t offset)
{
	ZRGraphNode *ret;
	ZRGRAPHNODE_GETNPARENTS(graph, node, &ret, offset, 1);
	return ret;
}

ZRMUSTINLINE
static inline ZRGraphNode* ZRGRAPHNODE_GETCHILD(ZRGraph *graph, ZRGraphNode *node, size_t offset)
{
	ZRGraphNode *ret;
	ZRGRAPHNODE_GETNCHILDS(graph, node, &ret, offset, 1);
	return ret;
}

ZRMUSTINLINE
static inline ZRGraphEdge ZRGRAPHEDGE_CPY(ZRGraph *graph, ZRGraphNode *a, ZRGraphNode *b, size_t offset, enum ZRGraphEdge_selectE select)
{
	ZRGraphEdge ret = { };
	ZRGRAPHNODE_CPYNEDGES(graph, a, &ret, offset, 1, select);
	return ret;
}

// ============================================================================

#include "GraphBuilder.h"

#endif

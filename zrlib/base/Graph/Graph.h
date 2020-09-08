/**
 * @author zuri
 * @date dimanche 1 décembre 2019, 14:15:50 (UTC+0100)
 */

#ifndef ZRGRAPH_H
#define ZRGRAPH_H

#include <zrlib/config.h>

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

ZRGraphEdge ZRGraphEdge_cpy(________ ZRGraph *graph, ZRGraphNode *a, size_t offset, enum ZRGraphEdge_selectE select);

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
static inline ZRGraphEdge ZRGRAPHEDGE_CPY(ZRGraph *graph, ZRGraphNode *a, size_t offset, enum ZRGraphEdge_selectE select)
{
	ZRGraphEdge ret = { };
	ZRGRAPHNODE_CPYNEDGES(graph, a, &ret, offset, 1, select);
	return ret;
}

// ============================================================================
// FUNCTION DEFINITION HELP
// ============================================================================

#define ZRGRAPHNODE_GETNBEDGES_MDEF(select, nbParents, nbChilds) ZRBLOCK( \
	switch (select) \
	{ \
	case ZRGraphEdge_selectIN: \
		return (nbParents); \
	case ZRGraphEdge_selectOUT: \
		return (nbChilds); \
	case ZRGraphEdge_selectINOUT: \
		return (nbParents) + (nbChilds); \
	default: \
		return SIZE_MAX; \
	} \
)

#define ZRGRAPHNODE_CPYNEDGES_MDEF( \
	M_graph, M_node, M_cpyTo, M_offset, M_maxNbCpy, M_select, \
	M_fnode_cpyNParentEdges, M_fnode_cpyNChildEdges, M_fnode_cpyNEdges \
) ZRBLOCK( \
	ZRGraph *_graph = (M_graph); \
	ZRGraphNode *_node = (M_node); \
	ZRGraphEdge *_cpyTo = (M_cpyTo); \
	size_t const _offset = (M_offset); \
	size_t const _maxNbCpy = (M_maxNbCpy); \
	enum ZRGraphEdge_selectE _select = (M_select); \
	\
	switch (_select) \
	{ \
	case ZRGraphEdge_selectIN: \
		return M_fnode_cpyNParentEdges(_graph, _node, _cpyTo, _offset, _maxNbCpy); \
	case ZRGraphEdge_selectOUT: \
		return M_fnode_cpyNChildEdges(_graph, _node, _cpyTo, _offset, _maxNbCpy); \
	case ZRGraphEdge_selectINOUT: \
		return M_fnode_cpyNEdges(_graph, _node, _cpyTo, _offset, _maxNbCpy); \
	default: \
		fprintf(stderr, "Bad select value in %s: %d", __func__, _select); \
		exit(1); \
	} \
)

#define ZRGRAPHNODE_CPYNEDGES_MDEF2( \
	M_graph, M_node, M_cpyTo, M_offset, M_maxNbCpy, \
	M_nbParents, M_nbChilds, \
	M_fnode_cpyNParentEdges, M_fnode_cpyNChildEdges \
) ZRBLOCK( \
	ZRGraph *_graph = (M_graph); \
	ZRGraphNode *_node = (M_node); \
	ZRGraphEdge *_cpyTo = (M_cpyTo); \
	size_t const _offset = (M_offset); \
	size_t const _nbParents = (M_nbParents); \
	size_t const _nbChilds = (M_nbChilds); \
	size_t const _maxNbCpy = (M_maxNbCpy); \
	/* \
	 * size_t (*_fcpyNChildEdges)(__ ZRGraph *, ZRGraphNode *, ZRGraphEdge *cpyTo, size_t offset, size_t maxNbCpy) = (M_fcpyNChildEdges); \
	 * size_t (*_fcpyNParentEdges)(_ ZRGraph *, ZRGraphNode *, ZRGraphEdge *cpyTo, size_t offset, size_t maxNbCpy) = (M_fcpyNParentEdges); \
	 */ \
	\
	/* Only childs to copy */ \
	if (_offset >= _nbParents) \
		return M_fnode_cpyNChildEdges(_graph, _node, _cpyTo, _offset - _nbParents, _maxNbCpy); \
	\
	/* Parents and childs to copy */ \
	size_t const _parent_nb = M_fnode_cpyNParentEdges(_graph, _node, _cpyTo, offset, maxNbCpy); \
	\
	if (M_maxNbCpy == _parent_nb) \
		return _parent_nb; \
	\
	return _parent_nb + M_fnode_cpyNChildEdges(_graph, _node, &_cpyTo[_parent_nb], offset + _parent_nb, _maxNbCpy - _parent_nb); \
)

#include "GraphBuilder.h"

#endif

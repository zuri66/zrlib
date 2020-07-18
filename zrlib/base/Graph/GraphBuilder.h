/**
 * @author zuri
 * @date mercredi 4 décembre 2019, 22:48:38 (UTC+0100)
 */

#ifndef ZRGRAPH_H
#error __FILE__ " cannot be used outside Graph.h"
#endif

#ifndef ZRGRAPHBUILDER_H
#define ZRGRAPHBUILDER_H

#include <zrlib/config.h>

typedef struct ZRGraphBuilderS ZRGraphBuilder;
typedef struct ZRGraphBuilderStrategyS ZRGraphBuilderStrategy;
typedef ZRGraphNode ZRGraphBuilderNode;

#define ZRGBNODE(N) ((ZRGraphBuilderNode*)(N))
#define ZRGBNODE_GNODE(GBN) ((ZRGraphNode*)(GBN))

// ============================================================================

struct ZRGraphBuilderStrategyS
{
	ZRGraphStrategy graph;
	size_t (*fstrategySize)(void);

	ZRGraph* (*fnew)(ZRGraphBuilder *builder, void **nodes, size_t nbNodes);

	ZRGraphBuilderNode* (*fnode)(ZRGraphBuilder *builder, void *nodeData);
	void (*fedge)(ZRGraphBuilder *builder, ZRGraphBuilderNode *a, ZRGraphBuilderNode *b, void *edgeData);
};

#define ZRGBSTRATEGY_GSTRATEGY(GBSTRAT) (&(GBSTRAT)->graph)

struct ZRGraphBuilderS
{
	ZRGraph graph;
};

#define ZRGRAPHBUILDER(GB) ((ZRGraphBuilder*)(GB))
#define ZRGB_GRAPH(GB) (&(GB)->graph)
#define ZRGB_STRATEGY(GB) ((ZRGraphBuilderStrategy*)ZRGB_GRAPH(GB)->strategy)

// ============================================================================

ZRMUSTINLINE
static inline ZRGraph* ZRGRAPHBUILDER_NEW(ZRGraphBuilder *builder, void **nodes, size_t nbNodes)
{
	return ZRGB_STRATEGY(builder)->fnew(builder, nodes, nbNodes);
}

ZRMUSTINLINE
static inline ZRGraphBuilderNode* ZRGRAPHBUILDER_NODE(ZRGraphBuilder *builder, void *nodeData)
{
	return ZRGB_STRATEGY(builder)->fnode(builder, nodeData);
}

ZRMUSTINLINE
static inline void ZRGRAPHBUILDER_EDGE(ZRGraphBuilder *builder, ZRGraphBuilderNode *a, ZRGraphBuilderNode *b, void *edgeData)
{
	ZRGB_STRATEGY(builder)->fedge(builder, a, b, edgeData);
}

// ============================================================================
// HELP
// ============================================================================

ZRMUSTINLINE
static inline ZRGraphBuilderNode* ZRGRAPHBUILDER_EDGENODE(ZRGraphBuilder *builder, ZRGraphBuilderNode *node, void *nodeData, void *edgeData)
{
	ZRGraphBuilderNode *nodeb = ZRGRAPHBUILDER_NODE(builder, nodeData);
	ZRGRAPHBUILDER_EDGE(builder, node, nodeb, edgeData);
	return nodeb;
}

// ============================================================================


#endif

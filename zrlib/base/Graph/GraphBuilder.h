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
typedef void ZRGraphBuilderNode;

// ============================================================================

struct ZRGraphBuilderStrategyS
{
	size_t (*fstrategySize)(void);

	ZRGraph* (*fnew)(ZRGraphBuilder *builder, void **nodes, size_t nbNodes);

	ZRGraphBuilderNode* (*fnode)(ZRGraphBuilder *builder, void *nodeData);
	void (*fedge)(ZRGraphBuilder *builder, ZRGraphBuilderNode *a, ZRGraphBuilderNode *b, void *edgeData);

	void (*fdone)(__ ZRGraphBuilder*);
	void (*fdestroy)(ZRGraphBuilder*);
};

struct ZRGraphBuilderS
{
	ZRGraphBuilderStrategy *strategy;
};

// ============================================================================

ZRMUSTINLINE
static inline ZRGraph* ZRGRAPHBUILDER_NEW(ZRGraphBuilder *builder, void **nodes, size_t nbNodes)
{
	return builder->strategy->fnew(builder, nodes, nbNodes);
}

ZRMUSTINLINE
static inline ZRGraphBuilderNode* ZRGRAPHBUILDER_NODE(ZRGraphBuilder *builder, void *nodeData)
{
	return builder->strategy->fnode(builder, nodeData);
}

ZRMUSTINLINE
static inline void ZRGRAPHBUILDER_EDGE(ZRGraphBuilder *builder, ZRGraphBuilderNode *a, ZRGraphBuilderNode *b, void *edgeData)
{
	builder->strategy->fedge(builder, a, b, edgeData);
}

ZRMUSTINLINE
static inline void ZRGRAPHBUILDER_DONE(ZRGraphBuilder *builder)
{
	builder->strategy->fdone(builder);
}

ZRMUSTINLINE
static inline void ZRGRAPHBUILDER_DESTROY(ZRGraphBuilder *builder)
{
	builder->strategy->fdestroy(builder);
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

void ZRGraphBuilder_done(ZRGraphBuilder *builder);
void ZRGraphBuilder_destroy(ZRGraphBuilder *builder);

#endif

/**
 * @author zuri
 * @date mercredi 4 décembre 2019, 23:53:30 (UTC+0100)
 */

#include <stdalign.h>

#include <zrlib/base/struct.h>

typedef struct ZRSimpleTreeS ZRSimpleTree;
typedef struct ZRSimpleTreeStrategyS ZRSimpleTreeStrategy;

struct ZRSimpleTreeStrategyS
{
	ZRTreeStrategy tree;
};

struct ZRSimpleTreeS
{
	ZRTree tree;

	size_t nbNodes;

	ZRAllocator *allocator;

	ZRSimpleTreeNode *nodes;

	void *nodeObjs;
	void *edgeObjs;
};

#define ZRSTREE(STREE) ((ZRSimpleTree*)(STREE))
#define ZRSTREE_TREE(STREE)  (&(STREE)->tree)
#define ZRSTREE_GRAPH(STREE) (&ZRSTREE_TREE(STREE)->graph)

// ============================================================================
// Node

typedef struct ZRSimpleTreeNodeS ZRSimpleTreeNode;

struct ZRSimpleTreeNodeS
{
	ZRTreeNode node;
	size_t nbAscendants;
	size_t nbDescendants;
	size_t nbChilds;
	ZRSimpleTreeNode *parent;
	ZRSimpleTreeNode *childs;
	void *edgeObj;
};

#define ZRSTNODE(N) ((ZRSimpleTreeNode*)(N))
#define ZRSTNODE_TNODE(STN) (&(STN)->node)

#define ZRSIMPLETREENODE_INFOS_NB 5

typedef enum
{
	ZRSimpleTreeNodeInfos_base,
	ZRSimpleTreeNodeInfos_nodes,
	ZRSimpleTreeNodeInfos_nodeObjs,
	ZRSimpleTreeNodeInfos_edgeObjs,
	ZRSimpleTreeNodeInfos_struct,
} ZRSimpleTreeNodeInfos;

static void ZRSimpleTreeInfos(ZRObjAlignInfos *out, size_t nbNodes,
	size_t nodeObjSize, size_t nodeObjAlignment,
	size_t edgeObjSize, size_t edgeObjAlignment
	)
{
	size_t const nbEdges = nbNodes ? nbNodes - 1 : 0;
	out[0] = (ZRObjAlignInfos ) { 0, alignof(ZRSimpleTree), sizeof(ZRSimpleTree) };
	out[1] = (ZRObjAlignInfos ) { 0, alignof(ZRSimpleTreeNode), nbNodes * sizeof(ZRSimpleTreeNode) };
	out[2] = (ZRObjAlignInfos ) { 0, nodeObjAlignment, nbNodes * nodeObjSize };
	out[3] = (ZRObjAlignInfos ) { 0, edgeObjAlignment, nbEdges * edgeObjSize };
	out[4] = (ZRObjAlignInfos ) { };
	ZRStruct_bestOffsetsPos(ZRSIMPLETREENODE_INFOS_NB - 1, out, 1);
}

ZRTree* ZRSimpleTree_create(size_t nbNodes,
	size_t nodeObjSize, size_t nodeObjAlignment,
	size_t edgeObjSize, size_t edgeObjAlignment,
	ZRAllocator *allocator
	);

// ============================================================================
// Simple Tree Builder
// ============================================================================

typedef struct ZRSimpleTreeBuilderStrategyS ZRSimpleTreeBuilderStrategy;
typedef struct ZRSimpleTreeBuilderNodeS ZRSimpleTreeBuilderNode;
typedef struct ZRSimpleTreeBuilderS ZRSimpleTreeBuilder;

struct ZRSimpleTreeBuilderStrategyS
{
	ZRTreeBuilderStrategy treeBuilder;
};

#define ZRSTBSTRATEGY(S) ((ZRSimpleTreeBuilderStrategy*)(S))
#define ZRSTBSTRATEGY_TB(STB) (&(STB)->treeBuilder)
#define ZRSTBSTRATEGY_T(STB) ZRTBSTRATEGY_T(ZRSTBSTRATEGY_TB(STB))
#define ZRSTBSTRATEGY_G(STB) ZRTBSTRATEGY_G(ZRSTBSTRATEGY_TB(STB))

struct ZRSimpleTreeBuilderNodeS
{
	ZRTreeBuilderNode tbNode;
	ZRSimpleTreeBuilderNode *parent;
	ZRVector *childs;
	alignas(max_align_t) char obj[];
};

#define ZRSTBNODE(N) ((ZRSimpleTreeBuilderNode*)(N))
#define ZRSTBNODE_TBNODE(STBN) (&(STBN)->tbNode)

struct ZRSimpleTreeBuilderS
{
	ZRTreeBuilder treeBuilder;

	size_t nodeObjSize;
	size_t nodeObjAlignment;

	size_t edgeObjSize;
	size_t edgeObjAlignment;

	size_t nbNodes;
	size_t build_nbNodes;

	ZRAllocator *allocator;
	ZRVector *nodeStack;
	ZRSimpleTreeBuilderNode *root;

	alignas(max_align_t) char rootSpace[];
};

#define ZRSTB(TB) ((ZRSimpleTreeBuilder*)(TB))
#define ZRSTB_TB(STB) (&(STB)->treeBuilder)


// Node&Edge size
#define ZRSTREEBUILDERNODE_SIZE(OBJSIZE) ((OBJSIZE) + sizeof(ZRSimpleTreeBuilderNode))

#define ZRSTREEBUILDER_TREEBUILDER(STREEBUILDER) (&STREEBUILDER->treeBuilder)
#define ZRSTREEBUILDER_STRATEGY(STREEBUILDER)    (ZRSTREEBUILDER_TREEBUILDER(STREEBUILDER)->strategy)
#define ZRSTREEBUILDER_NODESIZE(STREEBUILDER)    ZRSTREEBUILDERNODE_SIZE((STREEBUILDER)->nodeObjSize + (STREEBUILDER)->edgeObjSize)

#define ZRSTREEBUILDER_CURRENTNODE(STREEBUILDER) *(void**)ZRVECTOR_GET((STREEBUILDER)->nodeStack, ZRVECTOR_NBOBJ((STREEBUILDER)->nodeStack) - 1)

void fBuilder_build(ZRTreeBuilder *tbuilder, ZRTree *tree);

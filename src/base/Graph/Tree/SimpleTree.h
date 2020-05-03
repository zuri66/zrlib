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
	size_t nbAscendants;
	size_t nbDescendants;
	size_t nbChilds;
	ZRSimpleTreeNode *parent;
	ZRSimpleTreeNode *childs;
	void *obj;
	void *edgeObj;
};

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

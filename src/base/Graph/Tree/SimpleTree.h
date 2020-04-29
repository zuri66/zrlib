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

	void *objs;
};

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
};


#define ZRSIMPLETREENODE_INFOS_NB 4

typedef enum
{
	ZRSimpleTreeNodeInfos_base, ZRSimpleTreeNodeInfos_nodes, ZRSimpleTreeNodeInfos_objs, ZRSimpleTreeNodeInfos_struct
} ZRSimpleTreeNodeInfos;

static void ZRSimpleTreeInfos(ZRObjAlignInfos *out, size_t objSize, size_t nbObj, size_t objAlignment)
{
	out[0] = (ZRObjAlignInfos ) { 0, alignof(ZRSimpleTree), sizeof(ZRSimpleTree) };
	out[1] = (ZRObjAlignInfos ) { 0, alignof(ZRSimpleTreeNode), nbObj * sizeof(ZRSimpleTreeNode) };
	out[2] = (ZRObjAlignInfos ) { 0, objAlignment, nbObj * objSize };
	out[3] = (ZRObjAlignInfos ) { };
	ZRStruct_bestOffsetsPos(ZRSIMPLETREENODE_INFOS_NB - 1, out, 1);
}

ZRTree* ZRSimpleTree_create(size_t objSize, size_t nbObjs, size_t objAlignment, ZRAllocator *allocator);

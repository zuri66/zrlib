/**
 * @author zuri
 * @date mercredi 4 décembre 2019, 23:53:30 (UTC+0100)
 */

#include <stdalign.h>

#include <zrlib/base/macro.h>

typedef struct ZRSimpleTreeS ZRSimpleTree;
typedef struct ZRSimpleTreeStrategyS ZRSimpleTreeStrategy;

struct ZRSimpleTreeStrategyS
{
	ZRTreeStrategy tree;
};

struct ZRSimpleTreeS
{
	ZRTree tree;

	ZRAllocator *allocator;

	ZRVector *nodes;
};

#define ZRSTREE_TREE(STREE)  (&(STREE)->tree)
#define ZRSTREE_GRAPH(STREE) ZRTREE_GRAPH(ZRSTREE_TREE(STREE))

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
	alignas(max_align_t) char obj[];
};

#define ZRSTREENODE_SIZE(OBJSIZE) ZRSTRUCTSIZE_FAM_PAD(OBJSIZE + sizeof(ZRSimpleTreeNode), alignof(ZRSimpleTreeNode))
#define ZRSTREE_NODESIZE(STREE)   ZRSTREENODE_SIZE(ZRSTREE_GRAPH(STREE)->objSize)


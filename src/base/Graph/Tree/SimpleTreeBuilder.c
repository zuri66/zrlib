/**
 * @author zuri
 * @date dimanche 1 décembre 2019, 15:36:56 (UTC+0100)
 */

#include <zrlib/base/Graph/Tree/Tree.h>
#include <zrlib/base/Graph/Tree/SimpleTree.h>
#include <zrlib/base/Vector/Vector2SideStrategy.h>

#include <assert.h>
#include <stdalign.h>

#include "SimpleTree.h"

typedef struct
{
	ZRTreeBuilderStrategy treeBuilder;
} ZRSimpleTreeBuilderStrategy;

typedef struct
{
	void *parent;
	ZRVector *childs;
	alignas(max_align_t) char obj[];
}
ZRSimpleTreeBuilder_node;

typedef struct
{
	ZRTreeBuilder treeBuilder;
	size_t nodeSize;
	size_t objSize;
	size_t nbNodes;
	size_t build_nbNodes;
	ZRAllocator *allocator;
	ZRVector *nodeStack;
	ZRSimpleTreeBuilder_node *root;
	alignas(max_align_t) char rootSpace[];
} ZRSimpleTreeBuilder;

#define ZRSTREEBUILDERNODE_SIZE(OBJSIZE)    ((OBJSIZE) + sizeof(ZRSimpleTreeBuilder_node))

#define ZRSTREEBUILDER_TREEBUILDER(STREEBUILDER) (&STREEBUILDER->treeBuilder)
#define ZRSTREEBUILDER_STRATEGY(STREEBUILDER)    ((ZRSimpleTreeBuilderStrategy*)ZRSTREEBUILDER_TREEBUILDER(STREEBUILDER)->strategy)
#define ZRSTREEBUILDER_NODESIZE(STREEBUILDER)    ZRTREEBUILDERNODE_SIZE((STREEBUILDER)->objSize)

// ============================================================================

static void ZRSimpleTreeBuilder_nodeParent(ZRSimpleTreeBuilder *builder, void *data, ZRSimpleTreeBuilder_node *parent)
{
	size_t const objSize = builder->objSize;
	size_t const stackNb = ZRVECTOR_NBOBJ(builder->nodeStack);
	ZRAllocator *const allocator = builder->allocator;

	ZRSimpleTreeBuilder_node *node;

	if (stackNb == 0)
	{
		node = (ZRSimpleTreeBuilder_node*)builder->rootSpace;
		builder->root = (ZRSimpleTreeBuilder_node*)node;
	}
	else
	{
		ZRSimpleTreeBuilder_node *current = *(void**)ZRVECTOR_GET(builder->nodeStack, stackNb - 1);
		size_t const nodePos = ZRVECTOR_NBOBJ(current->childs);

		ZRVECTOR_RESERVE(current->childs, nodePos, 1);
		node = ZRVECTOR_GET(current->childs, nodePos);
	}
	node->parent = parent;
	node->childs = ZRVector2SideStrategy_createDynamic(512, builder->nodeSize, allocator);

	if (data != NULL)
		memcpy(node->obj, data, builder->objSize);

	ZRVECTOR_ADD(builder->nodeStack, &node);
	builder->nbNodes++;
}

static void fBuilder_node(ZRTreeBuilder *tbuilder, void *data)
{
	ZRSimpleTreeBuilder *const builder = (ZRSimpleTreeBuilder*)tbuilder;
	ZRSimpleTreeBuilder_nodeParent(builder, data, NULL);
}

static void* fBuilder_currentNode(ZRTreeBuilder *tbuilder)
{
	ZRSimpleTreeBuilder *const builder = (ZRSimpleTreeBuilder*)tbuilder;
	return *(void**)ZRVECTOR_GET(builder->nodeStack, ZRVECTOR_NBOBJ(builder->nodeStack) - 1);
}

static void* fBuilder_currentObj(ZRTreeBuilder *tbuilder)
{
	ZRSimpleTreeBuilder *const builder = (ZRSimpleTreeBuilder*)tbuilder;
	ZRSimpleTreeBuilder_node **current = ZRVECTOR_GET(builder->nodeStack, ZRVECTOR_NBOBJ(builder->nodeStack) - 1);
	return (*current)->obj;
}

static void fBuilder_end(ZRTreeBuilder *tbuilder)
{
	ZRSimpleTreeBuilder *const builder = (ZRSimpleTreeBuilder*)tbuilder;
	assert(ZRVECTOR_NBOBJ(builder->nodeStack) > 0);
	ZRVECTOR_DEC(builder->nodeStack);
}

static void treeNodeFromBNode(ZRSimpleTreeBuilder *builder, ZRSimpleTreeBuilder_node *currentBNodes, ZRSimpleTree *tree, ZRSimpleTreeNode *parent, ZRSimpleTreeNode *treeNode_out)
{
	*treeNode_out = (ZRSimpleTreeNode ) { //
		.nbChilds = ZRVECTOR_NBOBJ(currentBNodes->childs), //
		.parent = parent, //
		};
	memcpy(treeNode_out->obj, currentBNodes->obj, builder->objSize);
}

static size_t fBuilder_build_rec(ZRSimpleTreeBuilder *builder, ZRSimpleTreeBuilder_node *currentBNode, ZRSimpleTree *tree, ZRSimpleTreeNode *currentTreeNode, size_t level)
{
	ZRVector *const treeNodes = tree->nodes;
	size_t const nbChilds = ZRVECTOR_NBOBJ(currentBNode->childs);
	size_t const nbNodes = builder->build_nbNodes;
	size_t const levelAdd1 = level + 1;
	size_t nbDescendants = 0;
	size_t i;

	for (i = 0; i < nbChilds; i++)
	{
		ZRSimpleTreeBuilder_node *const bchild = ZRVECTOR_GET(currentBNode->childs, i);
		ZRSimpleTreeNode *const treeChild = ZRVECTOR_GET(treeNodes, nbNodes + i);
		treeNodeFromBNode(builder, bchild, tree, currentTreeNode, treeChild);
		treeChild->nbAscendants = levelAdd1;
	}
	nbDescendants += nbChilds;
	builder->build_nbNodes += nbChilds;

	for (i = 0; i < nbChilds; i++)
	{
		ZRSimpleTreeBuilder_node *const child = ZRVECTOR_GET(currentBNode->childs, i);
		ZRSimpleTreeNode *const treeChild = ZRVECTOR_GET(treeNodes, nbNodes + i);

		treeChild->childs = ZRVECTOR_GET(treeNodes, builder->build_nbNodes);
		size_t const nodeNbDescendants = fBuilder_build_rec(builder, child, tree, treeChild, levelAdd1);
		treeChild->nbAscendants = levelAdd1;
		treeChild->nbDescendants = nodeNbDescendants;
		nbDescendants += nodeNbDescendants;
	}
	return nbDescendants;
}

static void fBuilder_build(ZRTreeBuilder *tbuilder, ZRTree *tree)
{
	ZRSimpleTreeBuilder *const builder = (ZRSimpleTreeBuilder*)tbuilder;
	ZRSimpleTree *const stree = (ZRSimpleTree*)tree;
	size_t const nodeSize = ZRSTREENODE_SIZE(builder->objSize);
	ZRVector *const treeNodes = ZRVector2SideStrategy_createFixed(builder->nbNodes, nodeSize, builder->allocator);

	stree->nodes = treeNodes;
	ZRVECTOR_RESERVE(treeNodes, 0, builder->nbNodes);

	ZRSimpleTreeNode *const treeRoot = ZRVECTOR_GET(treeNodes, 0);
	stree->tree.root = (ZRTreeNode*)treeRoot;

	treeNodeFromBNode(builder, builder->root, stree, NULL, treeRoot);
	treeRoot->childs = ZRVECTOR_GET(treeNodes, 1);
	builder->build_nbNodes = 1;
	((ZRSimpleTreeNode*)(stree->tree.root))->nbDescendants = fBuilder_build_rec(builder, builder->root, stree, treeRoot, 0);

	stree->tree.graph.nbNodes = builder->nbNodes;
	stree->tree.graph.nbEdges = builder->nbNodes ? builder->nbNodes - 1 : 0;
}

static ZRTree* fBuilder_new(ZRTreeBuilder *tbuilder)
{
	ZRSimpleTreeBuilder *const builder = (ZRSimpleTreeBuilder*)tbuilder;
	ZRSimpleTree *stree = (ZRSimpleTree*)ZRSimpleTree_create(builder->objSize, builder->allocator, NULL);
	fBuilder_build(ZRSTREEBUILDER_TREEBUILDER(builder), ZRSTREE_TREE(stree));
	return ZRSTREE_TREE(stree);
}

static void fBuilder_done(ZRTreeBuilder *tbuilder)
{
	ZRSimpleTreeBuilder *const builder = (ZRSimpleTreeBuilder*)tbuilder;
	ZRAllocator *allocator = builder->allocator;
	ZRFREE(allocator, builder->nodeStack);
}

// ============================================================================
// BUILDER HELP
// ============================================================================

static void ZRSimpleTreeBuilderS_init(ZRSimpleTreeBuilderStrategy *strategy)
{
	*strategy = (ZRSimpleTreeBuilderStrategy ) { //
		.treeBuilder = (ZRTreeBuilderStrategy ) { //
			.fcurrentNode = fBuilder_currentNode, //
			.fcurrentObj = fBuilder_currentObj, //
			.fnode = fBuilder_node, //
			.fend = fBuilder_end, //
			.fnew = fBuilder_new, //
			.fdone = fBuilder_done, //
			} , //
		};
}

static void ZRSimpleTreeBuilder_init(ZRSimpleTreeBuilder *builder, ZRSimpleTreeBuilderStrategy *strategy, size_t objSize, ZRAllocator *allocator)
{
	size_t const nodeSize = objSize + sizeof(ZRSimpleTreeBuilder_node);

	*builder = (ZRSimpleTreeBuilder ) { //
		.treeBuilder = (ZRTreeBuilder ) { //
			.strategy = (ZRTreeBuilderStrategy*)strategy, //
			},//
		.nbNodes = 0, //
		.objSize = objSize, //
		.nodeSize = nodeSize, //
		.allocator = allocator, //
		.nodeStack = ZRVector2SideStrategy_createDynamic(512, sizeof(void*), allocator), //
		.root = NULL , //
		};
}

static void ZRSimpleTreeBuilder_fromSimpleTreeRec(ZRSimpleTreeBuilder *builder, ZRSimpleTreeBuilder_node *bnodeParent, ZRTree *tree, ZRTreeNode *currentTNode, ZRTreeNode *currentForStack)
{
	fBuilder_node(ZRSTREEBUILDER_TREEBUILDER(builder), ZRTreeNode_getObj(tree, currentTNode));

	ZRSimpleTreeBuilder_node *currentBNode = fBuilder_currentNode(ZRSTREEBUILDER_TREEBUILDER(builder));
	size_t const nbChilds = ZRTreeNode_getNbChilds(tree, currentTNode);
	size_t i;

	for (i = 0; i < nbChilds; i++)
		ZRSimpleTreeBuilder_fromSimpleTreeRec(builder, currentBNode, tree, ZRTreeNode_getChild(tree, currentTNode, i), currentForStack);

	/*
	 * The current node is add to the first place of the stack.
	 * We can use it at the end of the recursion.
	 */
	if (currentForStack == currentTNode)
		ZRVECTOR_ADDFIRST(builder->nodeStack, &currentBNode);

	fBuilder_end(ZRSTREEBUILDER_TREEBUILDER(builder));
}

ZRTreeBuilder* ZRSimpleTreeBuilder_fromTree(ZRTree *tree, ZRTreeNode *currentForStack, size_t objSize, ZRAllocator *allocator)
{
	ZRSimpleTreeBuilderStrategy *strategy = ZRALLOC(allocator, sizeof(ZRSimpleTreeBuilderStrategy));
	ZRSimpleTreeBuilderS_init(strategy);
	size_t const bnodeSize = ZRSTREEBUILDERNODE_SIZE(objSize);
	ZRSimpleTreeBuilder *builder = ZRALLOC(allocator, sizeof(ZRSimpleTreeBuilder) + bnodeSize);
	ZRSimpleTreeBuilder_init(builder, strategy, objSize, allocator);
	ZRSimpleTreeBuilder_fromSimpleTreeRec(builder, NULL, tree, tree->root, currentForStack);

	// Get all the node from currentForStack to the root
	ZRSimpleTreeBuilder_node *current = *(void**)ZRVECTOR_GET(builder->nodeStack, 0);

	while (NULL != (current = current->parent))
		ZRVECTOR_ADD(builder->nodeStack, &current);

	ZRARRAYOP_REVERSE(builder->nodeStack->array, builder->nodeStack->objSize, ZRVECTOR_NBOBJ(builder->nodeStack));
	return (ZRTreeBuilder*)builder;
}

ZRTreeBuilder* ZRSimpleTreeBuilder_fromSimpleTree(ZRSimpleTree *tree, ZRSimpleTreeNode *currentForStack)
{
	return ZRSimpleTreeBuilder_fromTree((ZRTree*)tree, (ZRTreeNode*)currentForStack, tree->tree.graph.objSize, tree->allocator);
}

ZRTreeBuilder* ZRSimpleTreeBuilder_create(size_t objSize, ZRAllocator *allocator)
{
	ZRSimpleTreeBuilderStrategy *strategy = ZRALLOC(allocator, sizeof(ZRSimpleTreeBuilderStrategy));
	ZRSimpleTreeBuilderS_init(strategy);

	size_t const bnodeSize = ZRSTREEBUILDERNODE_SIZE(objSize);
	ZRSimpleTreeBuilder *builder = ZRALLOC(allocator, sizeof(ZRSimpleTreeBuilder) + bnodeSize);
	ZRSimpleTreeBuilder_init(builder, strategy, objSize, allocator);
	return (ZRTreeBuilder*)builder;
}

void ZRSimpleTreeBuilder_done(ZRTreeBuilder *builder)
{
	ZRVector_destroy(((ZRSimpleTreeBuilder*)builder)->nodeStack);
}

static void ZRSimpleTreeBuilder_destroyNode(ZRSimpleTreeBuilder *sbuilder, ZRSimpleTreeBuilder_node *node)
{
	size_t i;
	size_t const nb = node->childs->nbObj;

	for (i = 0; i < nb; i++)
		ZRSimpleTreeBuilder_destroyNode(sbuilder, ZRVECTOR_GET(node->childs, i));

	ZRVector_destroy(node->childs);
}

void ZRSimpleTreeBuilder_destroy(ZRTreeBuilder *builder)
{
	ZRSimpleTreeBuilder *sbuilder = (ZRSimpleTreeBuilder*)builder;
	ZRAllocator *allocator = sbuilder->allocator;

	ZRSimpleTreeBuilder_destroyNode(sbuilder, sbuilder->root);
	ZRSimpleTreeBuilder_done(builder);

	ZRFREE(allocator, builder->strategy);
	ZRFREE(allocator, builder);
}

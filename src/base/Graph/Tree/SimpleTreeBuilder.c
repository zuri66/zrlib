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

typedef struct ZRSimpleTreeBuilderStrategyS ZRSimpleTreeBuilderStrategy;
typedef struct ZRSimpleTreeBuilderNodeS ZRSimpleTreeBuilderNode;

struct ZRSimpleTreeBuilderStrategyS
{
	ZRTreeBuilderStrategy treeBuilder;
};

struct ZRSimpleTreeBuilderNodeS
{
	ZRSimpleTreeBuilderNode *parent;
	ZRVector *childs;
	alignas(max_align_t) char obj[];
};

typedef struct
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
} ZRSimpleTreeBuilder;

// Node&Edge size
#define ZRSTREEBUILDERNODE_SIZE(OBJSIZE) ((OBJSIZE) + sizeof(ZRSimpleTreeBuilderNode))

#define ZRSTREEBUILDER_TREEBUILDER(STREEBUILDER) (&STREEBUILDER->treeBuilder)
#define ZRSTREEBUILDER_STRATEGY(STREEBUILDER)    (ZRSTREEBUILDER_TREEBUILDER(STREEBUILDER)->strategy)
#define ZRSTREEBUILDER_NODESIZE(STREEBUILDER)    ZRSTREEBUILDERNODE_SIZE((STREEBUILDER)->nodeObjSize + (STREEBUILDER)->edgeObjSize)

#define ZRSTREEBUILDER_CURRENTNODE(STREEBUILDER) *(void**)ZRVECTOR_GET((STREEBUILDER)->nodeStack, ZRVECTOR_NBOBJ((STREEBUILDER)->nodeStack) - 1)

// ============================================================================

ZRMUSTINLINE
static inline void fbuilder_cpyData(ZRSimpleTreeBuilder *tbuilder, ZRSimpleTreeBuilderNode *bnode, void *nodeData, void *edgeData)
{
	if (nodeData != NULL)
		memcpy(bnode->obj, nodeData, tbuilder->nodeObjSize);

	if (edgeData != NULL)
		memcpy((char*)bnode->obj + tbuilder->nodeObjSize, edgeData, tbuilder->edgeObjSize);
}

ZRMUSTINLINE
static inline void fbuilder_cpyData2treeNode(ZRSimpleTreeBuilder *tbuilder, ZRSimpleTreeBuilderNode *bnode, ZRSimpleTreeNode *tnode)
{
	if (tbuilder->nodeObjSize)
		memcpy(tnode->obj, bnode->obj, tbuilder->nodeObjSize);

	if (tbuilder->edgeObjSize)
		memcpy(tnode->edgeObj, bnode->obj + tbuilder->nodeObjSize, tbuilder->edgeObjSize);
}

static void fBuilderNode(ZRTreeBuilder *tbuilder, void *nodeData, void *edgeData)
{
	ZRSimpleTreeBuilder *const builder = (ZRSimpleTreeBuilder*)tbuilder;
	size_t const stackNb = ZRVECTOR_NBOBJ(builder->nodeStack);
	ZRAllocator *const allocator = builder->allocator;

	ZRSimpleTreeBuilderNode *node;

	if (stackNb == 0)
	{
		node = (ZRSimpleTreeBuilderNode*)builder->rootSpace;
		builder->root = node;
		node->parent = NULL;
	}
	else
	{
		ZRSimpleTreeBuilderNode *current = ZRSTREEBUILDER_CURRENTNODE(builder);
		size_t const nodePos = ZRVECTOR_NBOBJ(current->childs);

		ZRVECTOR_RESERVE(current->childs, nodePos, 1);
		node = ZRVECTOR_GET(current->childs, nodePos);
		node->parent = current;
	}
	node->childs = ZRVector2SideStrategy_createDynamic(128, ZRSTREEBUILDER_NODESIZE(builder), allocator);

	fbuilder_cpyData(builder, node, nodeData, edgeData);

	ZRVECTOR_ADD(builder->nodeStack, (void*)&node);
	builder->nbNodes++;
}

static void* fBuilder_currentNode(ZRTreeBuilder *tbuilder)
{
	ZRSimpleTreeBuilder *const builder = (ZRSimpleTreeBuilder*)tbuilder;
	return *(void**)ZRVECTOR_GET(builder->nodeStack, ZRVECTOR_NBOBJ(builder->nodeStack) - 1);
}

static void* fBuilder_currentObj(ZRTreeBuilder *tbuilder)
{
	ZRSimpleTreeBuilder *const builder = (ZRSimpleTreeBuilder*)tbuilder;
	ZRSimpleTreeBuilderNode *current = ZRSTREEBUILDER_CURRENTNODE(builder);
	return current->obj;
}

static void fBuilder_end(ZRTreeBuilder *tbuilder)
{
	ZRSimpleTreeBuilder *const builder = (ZRSimpleTreeBuilder*)tbuilder;
	assert(ZRVECTOR_NBOBJ(builder->nodeStack) > 0);
	ZRVECTOR_DEC(builder->nodeStack);
}

static size_t fBuilder_build_rec(ZRSimpleTreeBuilder *builder, ZRSimpleTreeBuilderNode *currentBNode, ZRSimpleTree *tree, ZRSimpleTreeNode *currentTreeNode, size_t level)
{
	ZRSimpleTreeNode *const treeNodes = tree->nodes;
	size_t const nbChilds = ZRVECTOR_NBOBJ(currentBNode->childs);
	size_t const nbNodes = builder->build_nbNodes;
	size_t const levelAdd1 = level + 1;
	size_t nbDescendants = 0;
	size_t i;

	// Create each tree node
	for (i = 0; i < nbChilds; i++)
	{
		ZRSimpleTreeBuilderNode *const bchild = ZRVECTOR_GET(currentBNode->childs, i);
		ZRSimpleTreeNode *const treeChild = &treeNodes[nbNodes + i];
		*treeChild = (ZRSimpleTreeNode ) { //
			.nbAscendants = levelAdd1, //
			.nbChilds ___ = ZRVECTOR_NBOBJ(bchild->childs), //
			.parent _____ = currentTreeNode, //
			.obj ________ = (char*)tree->nodeObjs + ZRSTREE_GRAPH(tree)->nodeObjSize * (nbNodes + i), //
			.edgeObj ____ = (char*)tree->edgeObjs + ZRSTREE_GRAPH(tree)->edgeObjSize * (nbNodes + i), //
			};
		fbuilder_cpyData2treeNode(builder, bchild, treeChild);
	}
	nbDescendants += nbChilds;
	builder->build_nbNodes += nbChilds;

	// Recursive call for each child
	for (i = 0; i < nbChilds; i++)
	{
		ZRSimpleTreeBuilderNode *const child = ZRVECTOR_GET(currentBNode->childs, i);
		ZRSimpleTreeNode *const treeChild = &treeNodes[nbNodes + i];

// Beginning of the array of childs
		treeChild->childs = &treeNodes[builder->build_nbNodes];
		size_t const nodeNbDescendants = fBuilder_build_rec(builder, child, tree, treeChild, levelAdd1);
		treeChild->nbDescendants = nodeNbDescendants;
		nbDescendants += nodeNbDescendants;
	}
	return nbDescendants;
}

static void fBuilder_build(ZRTreeBuilder *tbuilder, ZRTree *tree)
{
	ZRSimpleTreeBuilder *const builder = (ZRSimpleTreeBuilder*)tbuilder;
	ZRSimpleTree *const stree = (ZRSimpleTree*)tree;
	ZRSimpleTreeNode *const treeRoot = stree->nodes;

	stree->tree.root = (ZRTreeNode*)treeRoot;

	// Make the root
	*treeRoot = (ZRSimpleTreeNode ) { //
		.parent = NULL, //
		.nbChilds ___ = ZRVECTOR_NBOBJ(builder->root->childs), //
		.childs = &stree->nodes[1], //
		.obj = stree->nodeObjs, //
		.edgeObj = stree->edgeObjs, //
		};
	fbuilder_cpyData2treeNode(builder, builder->root, treeRoot);
	builder->build_nbNodes = 1;
	treeRoot->nbDescendants = fBuilder_build_rec(builder, builder->root, stree, treeRoot, 0);

	stree->tree.graph.nbNodes = builder->nbNodes;
	stree->tree.graph.nbEdges = builder->nbNodes ? builder->nbNodes - 1 : 0;
}

static ZRTree* fBuilder_new(ZRTreeBuilder *tbuilder)
{
	ZRSimpleTreeBuilder *const builder = (ZRSimpleTreeBuilder*)tbuilder;
	// TODO alignment input
	ZRSimpleTree *stree = (ZRSimpleTree*)ZRSimpleTree_create(builder->nbNodes,
		builder->nodeObjSize, builder->nodeObjAlignment,
		builder->edgeObjSize, builder->edgeObjAlignment,
		builder->allocator);
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

static void ZRSimpleTreeBuilder_destroy(ZRTreeBuilder *builder);

static void ZRSimpleTreeBuilderS_init(ZRSimpleTreeBuilderStrategy *strategy)
{
	*strategy = (ZRSimpleTreeBuilderStrategy ) { //
		.treeBuilder = (ZRTreeBuilderStrategy ) { //
			.fcurrentNode = fBuilder_currentNode, //
			.fcurrentObj = fBuilder_currentObj, //
			.fnode = fBuilderNode, //
			.fend = fBuilder_end, //
			.fnew = fBuilder_new, //
			.fdone = fBuilder_done, //
			} , //
		};
}

static void ZRSimpleTreeBuilder_init(ZRSimpleTreeBuilder *builder, ZRSimpleTreeBuilderStrategy *strategy,
	size_t nodeObjSize, size_t nodeObjAlignment,
	size_t edgeObjSize, size_t edgeObjAlignment,
	ZRAllocator *allocator
	)
{
	*builder = (ZRSimpleTreeBuilder ) { //
		.treeBuilder = (ZRTreeBuilder ) { //
			.strategy = (ZRTreeBuilderStrategy*)strategy, //
			},//
		.nbNodes = 0, //
		.nodeObjSize = nodeObjSize, //
		.nodeObjAlignment = nodeObjAlignment, //
		.edgeObjSize = edgeObjSize, //
		.edgeObjAlignment = edgeObjAlignment, //
		.allocator = allocator, //
		.nodeStack = ZRVector2SideStrategy_createDynamic(512, sizeof(void*), allocator), //
		.root = NULL , //
		};
}

static void ZRSimpleTreeBuilder_fromTreeRec(ZRSimpleTreeBuilder *builder, ZRSimpleTreeBuilderNode *bnodeParent, ZRTree *tree, ZRTreeNode *currentTNode, ZRTreeNode *currentForStack, ZRSimpleTreeBuilderNode **stackBNode)
{
	ZRGraphEdge edge;
	void *edgeData;

	if (ZRGRAPHNODE_CPYNEDGES(ZRTREE_GRAPH(tree), currentTNode, &edge, 0, 1, ZRGraphEdge_selectIN))
		edgeData = edge.obj;
	else
		edgeData = NULL;

	fBuilderNode(ZRSTREEBUILDER_TREEBUILDER(builder), ZRGRAPHNODE_GETOBJ(ZRTREE_GRAPH(tree), currentTNode), edgeData);

	ZRSimpleTreeBuilderNode *currentBNode = fBuilder_currentNode(ZRSTREEBUILDER_TREEBUILDER(builder));
	size_t const nbChilds = ZRGRAPHNODE_GETNBCHILDS(ZRTREE_GRAPH(tree), currentTNode);
	size_t i;

	for (i = 0; i < nbChilds; i++)
		ZRSimpleTreeBuilder_fromTreeRec(builder, currentBNode, tree, ZRGRAPHNODE_GETCHILD(ZRTREE_GRAPH(tree), currentTNode, i), currentForStack, stackBNode);

	/*
	 * The current node is add to the first place of the stack.
	 * We can use it at the end of the recursion.
	 */
	if (currentForStack == currentTNode)
		*stackBNode = currentBNode;

	fBuilder_end(ZRSTREEBUILDER_TREEBUILDER(builder));
}

ZRTreeBuilder* ZRSimpleTreeBuilder_fromTree(
	ZRTree *tree, ZRTreeNode *currentForStack,
	size_t nodeObjSize, size_t nodeObjAlignment,
	size_t edgeObjSize, size_t edgeObjAlignment,
	ZRAllocator *allocator
	)
{
	ZRSimpleTreeBuilderStrategy *strategy = ZRALLOC(allocator, sizeof(ZRSimpleTreeBuilderStrategy));
	ZRSimpleTreeBuilderS_init(strategy);
	size_t const bnodeSize = ZRSTREEBUILDERNODE_SIZE(nodeObjSize + edgeObjSize);
	ZRSimpleTreeBuilder *builder = ZRALLOC(allocator, sizeof(ZRSimpleTreeBuilder) + bnodeSize);

	strategy->treeBuilder.fdestroy = ZRSimpleTreeBuilder_destroy;
	ZRSimpleTreeBuilder_init(builder, strategy, nodeObjSize, nodeObjAlignment, edgeObjSize, edgeObjAlignment, allocator);

	ZRSimpleTreeBuilderNode *stackNode = NULL;
	ZRSimpleTreeBuilder_fromTreeRec(builder, NULL, tree, tree->root, currentForStack, &stackNode);

	// Get all the node from currentForStack to the root

	if (currentForStack)
	{
		assert(stackNode != NULL);
		ZRVECTOR_DELETE_ALL(builder->nodeStack);

		do
		{
			ZRVECTOR_INSERT(builder->nodeStack, 0, (void*)&stackNode);
		} while (NULL != (stackNode = stackNode->parent));
	}
	return (ZRTreeBuilder*)builder;
}

ZRTreeBuilder* ZRSimpleTreeBuilder_fromSimpleTree(ZRSimpleTree *tree, ZRSimpleTreeNode *currentForStack)
{
	return ZRSimpleTreeBuilder_fromTree(
		(ZRTree*)tree, (ZRTreeNode*)currentForStack,
		ZRSTREE_GRAPH(tree)->nodeObjSize, ZRSTREE_GRAPH(tree)->nodeObjAlignment,
		ZRSTREE_GRAPH(tree)->edgeObjSize, ZRSTREE_GRAPH(tree)->edgeObjAlignment,
		tree->allocator
		);
}

ZRTreeBuilder* ZRSimpleTreeBuilder_create(
	size_t nodeObjSize, size_t nodeObjAlignment,
	size_t edgeObjSize, size_t edgeObjAlignment,
	ZRAllocator *allocator
	)
{
	ZRSimpleTreeBuilderStrategy *strategy = ZRALLOC(allocator, sizeof(ZRSimpleTreeBuilderStrategy));
	ZRSimpleTreeBuilderS_init(strategy);
	strategy->treeBuilder.fdestroy = ZRSimpleTreeBuilder_destroy;

	size_t const bnodeSize = ZRSTREEBUILDERNODE_SIZE(nodeObjSize + edgeObjSize);

	// Add the root space
	ZRSimpleTreeBuilder *builder = ZRALLOC(allocator, sizeof(ZRSimpleTreeBuilder) + bnodeSize);
	ZRSimpleTreeBuilder_init(builder, strategy, nodeObjSize, nodeObjAlignment, edgeObjSize, edgeObjAlignment, allocator);
	return (ZRTreeBuilder*)builder;
}

void ZRSimpleTreeBuilder_done(ZRTreeBuilder *builder)
{
	ZRVector_destroy(((ZRSimpleTreeBuilder*)builder)->nodeStack);
}

static void ZRSimpleTreeBuilder_destroyNode(ZRSimpleTreeBuilder *sbuilder, ZRSimpleTreeBuilderNode *node)
{
	size_t i;
	size_t const nb = node->childs->nbObj;

	for (i = 0; i < nb; i++)
		ZRSimpleTreeBuilder_destroyNode(sbuilder, ZRVECTOR_GET(node->childs, i));

	ZRVector_destroy(node->childs);
}

static void ZRSimpleTreeBuilder_destroy(ZRTreeBuilder *builder)
{
	ZRSimpleTreeBuilder *sbuilder = (ZRSimpleTreeBuilder*)builder;
	ZRAllocator *allocator = sbuilder->allocator;

	ZRSimpleTreeBuilder_destroyNode(sbuilder, sbuilder->root);
	ZRSimpleTreeBuilder_done(builder);

	ZRFREE(allocator, builder->strategy);
	ZRFREE(allocator, builder);
}

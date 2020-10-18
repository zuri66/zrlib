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

// ============================================================================
// BUILDER FUNCTIONS
// ============================================================================

ZRMUSTINLINE
static inline void sbnode_cpyData(ZRSimpleTreeBuilder *sbuilder, ZRSimpleTreeBuilderNode *bnode, void *nodeData, void *edgeData)
{
	if (nodeData != NULL)
	{
		memcpy(bnode->obj, nodeData, sbuilder->nodeObjSize);
		ZRSTBNODE_TB(bnode)->obj = bnode->obj;
	}
	else
		ZRSTBNODE_TB(bnode)->obj = NULL;

	if (edgeData != NULL)
		memcpy((char*)bnode->obj + sbuilder->nodeObjSize, edgeData, sbuilder->edgeObjSize);
}

ZRMUSTINLINE
static inline void fbuilder_cpyData2treeNode(ZRSimpleTreeBuilder *tbuilder, ZRSimpleTreeBuilderNode *bnode, ZRSimpleTreeNode *tnode)
{
	if (tbuilder->nodeObjSize)
		memcpy(ZRSTNODE_G(tnode)->obj, ZRSTBNODE_G(bnode)->obj, tbuilder->nodeObjSize);

	if (tbuilder->edgeObjSize)
		memcpy(tnode->edgeObj, bnode->obj + tbuilder->nodeObjSize, tbuilder->edgeObjSize);
}

void sbnode_add(ZRSimpleTreeBuilder *sbuilder, ZRSimpleTreeBuilderNode *sbnode, ZRSimpleTreeBuilderNode *parent, void *nodeData, void *edgeData)
{
	ZRVECTOR_ADD(sbuilder->nodeStack, &sbnode);

	*sbnode = (ZRSimpleTreeBuilderNode ) { //
		.tbNode = (ZRGraphNode ) { //
			.id = 0,
			.obj = sbnode->obj,
			},
		.parent = parent,
		.childs = ZRVector2SideStrategy_createDynamic(128, ZROBJINFOS_DEF(alignof(ZRSimpleTreeBuilderNode), ZRSTREEBUILDER_NODESIZE(sbuilder)), sbuilder->allocator),
		.edgeObj = ZRARRAYOP_GET(sbnode->obj, sbuilder->nodeObjSize, 1),
		};
	sbnode_cpyData(sbuilder, sbnode, nodeData, edgeData);
}

static void fBuilder_node(ZRTreeBuilder *tbuilder, void *nodeData, void *edgeData)
{
	ZRSimpleTreeBuilder *const builder = (ZRSimpleTreeBuilder*)tbuilder;
	size_t const stackNb = ZRVECTOR_NBOBJ(builder->nodeStack);
	ZRAllocator *const allocator = builder->allocator;

	ZRSimpleTreeBuilderNode *parent;
	ZRSimpleTreeBuilderNode *node;

	if (stackNb == 0)
	{
		node = (ZRSimpleTreeBuilderNode*)builder->rootSpace;
		builder->root = node;
		parent = NULL;
	}
	else
	{
		ZRSimpleTreeBuilderNode *parent = ZRSTB_CURRENTNODE(builder);
		size_t const nodePos = ZRVECTOR_NBOBJ(parent->childs);

		ZRVECTOR_RESERVE(parent->childs, nodePos, 1);
		node = ZRVECTOR_GET(parent->childs, nodePos);
	}
	sbnode_add(builder, node, parent, nodeData, edgeData);
	builder->nbNodes++;
}

static ZRTreeBuilderNode* fBuilder_currentNode(ZRTreeBuilder *tbuilder)
{
	ZRSimpleTreeBuilder *const builder = (ZRSimpleTreeBuilder*)tbuilder;
	return *(void**)ZRVECTOR_GET(builder->nodeStack, ZRVECTOR_NBOBJ(builder->nodeStack) - 1);
}

static void* fBuilder_currentObj(ZRTreeBuilder *tbuilder)
{
	return ZRSTBNODE_OBJ(ZRSTB_CURRENTNODE(ZRSTB(tbuilder)));
}

static void fBuilder_end(ZRTreeBuilder *tbuilder)
{
	ZRSimpleTreeBuilder *const builder = (ZRSimpleTreeBuilder*)tbuilder;
	assert(ZRVECTOR_NBOBJ(builder->nodeStack) > 0);
	ZRVECTOR_DEC(builder->nodeStack);
}

// ============================================================================
// BUILDING FUNCTIONS
// ============================================================================

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
			.node = (ZRTreeNode ) { //
				.id = nbNodes, //
				.obj ________ = (char*)tree->nodeObjs + ZRSTREE_GRAPH(tree)->nodeObjSize * (nbNodes + i), //
				},//
			.nbAscendants = levelAdd1, //
			.nbChilds ___ = ZRVECTOR_NBOBJ(bchild->childs), //
			.parent _____ = currentTreeNode, //
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

void fBuilder_build(ZRTreeBuilder *tbuilder, ZRTree *tree)
{
	ZRSimpleTreeBuilder *const builder = (ZRSimpleTreeBuilder*)tbuilder;
	ZRSimpleTree *const stree = (ZRSimpleTree*)tree;
	ZRSimpleTreeNode *const treeRoot = stree->nodes;

	stree->tree.root = (ZRTreeNode*)treeRoot;

	// Make the root
	*treeRoot = (ZRSimpleTreeNode ) { //
		.node = (ZRTreeNode ) { //
			.id = 0, //
			.obj = stree->nodeObjs, //
			},//
		.parent = NULL, //
		.nbChilds ___ = ZRVECTOR_NBOBJ(builder->root->childs), //
		.childs = &stree->nodes[1], //
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

// ============================================================================
// TREEFUNCTIONS
// ============================================================================

// ============================================================================
// GRAPH FUNCTIONS
// ============================================================================

void fgraph_done(ZRGraph *graph)
{
	ZRVECTOR_DESTROY(ZRSTB(graph)->nodeStack);
}

static void ZRSimpleTreeBuilder_destroyNode(ZRSimpleTreeBuilder *sbuilder, ZRSimpleTreeBuilderNode *node)
{
	size_t i;
	size_t const nb = ZRVECTOR_NBOBJ(node->childs);

	for (i = 0; i < nb; i++)
		ZRSimpleTreeBuilder_destroyNode(sbuilder, ZRVECTOR_GET(node->childs, i));

	ZRVector_destroy(node->childs);
}

static void fgraph_destroy(ZRGraph *graph)
{
	ZRSimpleTreeBuilder *sbuilder = ZRSTB(graph);
	ZRAllocator *allocator = sbuilder->allocator;

	ZRSimpleTreeBuilder_destroyNode(sbuilder, sbuilder->root);
	ZRGRAPH_DONE(graph);

	ZRFREE(allocator, ZRSTB_STRATEGY(sbuilder));
	ZRFREE(allocator, sbuilder);
}

// ============================================================================
// BUILDER HELP
// ============================================================================

static void ZRSimpleTreeBuilderS_init(ZRSimpleTreeBuilderStrategy *strategy)
{
	*strategy = (ZRSimpleTreeBuilderStrategy ) { //
		.treeBuilder = (ZRTreeBuilderStrategy ) { //
			.tree = (ZRTreeStrategy ) { //
				.graph = (ZRGraphStrategy ) { //
					.fdone = fgraph_done, //
					} , //
				},
			.fcurrentNode = fBuilder_currentNode, //
			.fcurrentObj = fBuilder_currentObj, //
			.fnode = fBuilder_node, //
			.fend = fBuilder_end, //
			.fnew = fBuilder_new, //
			} , //
		};
}

static void ZRSimpleTreeBuilder_init(ZRSimpleTreeBuilder *builder, ZRSimpleTreeBuilderStrategy *strategy,
	size_t nodeObjSize, size_t nodeObjAlignment,
	size_t edgeObjSize, size_t edgeObjAlignment,
	ZRAllocator *allocator
	)
{
	ZRVector *nodeStack = ZRVector2SideStrategy_createDynamic(512, ZRTYPE_SIZE_ALIGNMENT(void*), allocator);

	*builder = (ZRSimpleTreeBuilder ) { //
		.treeBuilder = (ZRTreeBuilder ) { //
			.tree = (ZRTree ) { //
				.graph = (ZRGraph ) { //
					.strategy = ZRSTBSTRATEGY_G(strategy),
					} ,
				} ,
			},
		.nbNodes = 0, //
		.nodeObjSize = nodeObjSize, //
		.nodeObjAlignment = nodeObjAlignment, //
		.edgeObjSize = edgeObjSize, //
		.edgeObjAlignment = edgeObjAlignment, //
		.allocator = allocator, //
		.nodeStack = nodeStack, //
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

	fBuilder_node(ZRSTREEBUILDER_TREEBUILDER(builder), ZRGRAPHNODE_GETOBJ(currentTNode), edgeData);

	ZRSimpleTreeBuilderNode *currentBNode = (ZRSimpleTreeBuilderNode*)fBuilder_currentNode(ZRSTREEBUILDER_TREEBUILDER(builder));
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

	ZRSTBSTRATEGY_G(strategy)->fdestroy = fgraph_destroy;
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
	ZRSTBSTRATEGY_G(strategy)->fdestroy = fgraph_destroy;

	size_t const bnodeSize = ZRSTREEBUILDERNODE_SIZE(nodeObjSize + edgeObjSize);

// Add the root space
	ZRSimpleTreeBuilder *builder = ZRALLOC(allocator, sizeof(ZRSimpleTreeBuilder) + bnodeSize);
	ZRSimpleTreeBuilder_init(builder, strategy,
		nodeObjSize, nodeObjAlignment,
		edgeObjSize, edgeObjAlignment,
		allocator
		);
	return ZRSTB_TB(builder);
}

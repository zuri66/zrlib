/**
 * @author zuri
 * @date dimanche 1 décembre 2019, 15:36:56 (UTC+0100)
 */

#ifndef ZRSIMPLETREE_H
#define ZRSIMPLETREE_H

#include "Tree.h"

#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/Vector/Vector.h>

typedef struct ZRSimpleTreeS ZRSimpleTree;
typedef struct ZRSimpleTreeNodeS ZRSimpleTreeNode;


ZRTreeBuilder* ZRSimpleTreeBuilder_fromTree(_____ ZRTree _____ *tree, ZRTreeNode *currentForStack, size_t objSize, size_t objAlignment, ZRAllocator *allocator);
ZRTreeBuilder* ZRSimpleTreeBuilder_fromSimpleTree(ZRSimpleTree *tree, ZRSimpleTreeNode *currentForStack);

ZRTreeBuilder* ZRSimpleTreeBuilder_create(size_t objSize, size_t objAlignment, ZRAllocator *allocator);

#endif

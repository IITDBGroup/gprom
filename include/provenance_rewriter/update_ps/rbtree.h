/*
 * rbtree.h
 *
 */

#ifndef INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_RBTREE_H_
#define INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_RBTREE_H_

#include "model/node/nodetype.h"

// color;
#define RBTREE_RED 0
#define RBTREE_BLACK 1

// parent;
#define RBTREE_PARENT(n) ((n)->parent)
#define RBTREE_SET_PARENT(n, p) ((n)->parent = (p))

// child
#define RBTREE_LCHILD(n) ((n)->left)
#define RBTREE_RCHILD(n) ((n)->right)

// color
#define RBTREE_COLOR(n) ((n)->color)
#define RBTREE_SET_COLOR(n, c) ((n)->color = (c))
#define RBTREE_IS_RED(n) ((n)->color == RBTREE_RED)
#define RBTREE_SET_RED(n) ((n)->color == RBTREE_RED)
#define RBTREE_IS_BLACK(n) ((n)->color == RBTREE_BLACK)
#define RBTREE_SET_BLACK(n) ((n)->color == RBTREE_BLACK)

// treenode;
typedef struct RBTreeNode
{
    Node *key;                  // key;
    struct RBTreeNode *left;    // left child;
    struct RBTreeNode *right;   // right child;
    struct RBTreeNode *parent;  // parent ;
    unsigned char color;        // color: RBTREE_RED or RBTREE_BLACK;
} RBTreeNode;

// tree;
typedef struct RBTree
{
    RBTNode *root;
} RBTree;

extern void RBTreeInsert(RBTreeNode **root, RBTreeNode *node);
extern void RBTreeDelete(RBTreeNode **root, RBTreeNode *node);
#endif /* INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_RBTREE_H_ */

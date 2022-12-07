/*
 * rbtree.h
 *
 */

#ifndef INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_RBTREE_H_
#define INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_RBTREE_H_

#include "model/node/nodetype.h"

// TNIL
#define TNIL NULL

// color;
#define RBT_RED 0
#define RBT_BLACK 1

// parent;
#define RBT_PARENT(n) ((n)->parent)
#define RBT_SET_PARENT(n, p) ((n)->parent = (p))

// child
#define RBT_LCHILD(n) ((n)->left)
#define RBT_RCHILD(n) ((n)->right)

// color
#define RBT_COLOR(n) ((n)->color)
#define RBT_SET_COLOR(n, c) ((n)->color = (c))
#define RBT_IS_RED(n) ((n)->color == RBTREE_RED)
#define RBT_SET_RED(n) ((n)->color = RBTREE_RED)
#define RBT_IS_BLACK(n) ((n)->color == RBTREE_BLACK)
#define RBT_SET_BLACK(n) ((n)->color = RBTREE_BLACK)



// TRY: SKIP LIST for limit oeprator;
// db: min heap, for top k, then compare new value wit min-heap,
// TODO: if applicable of RB tree, replace RB tree with heap in min/max;

// treenode;
typedef struct RBTNode
{
    Node *key;               // key;
    struct RBTNode *left;    // left child;
    struct RBTNode *right;   // right child;
    struct RBTNode *parent;  // parent ;
    unsigned char color;     // color: RBTREE_RED or RBTREE_BLACK;
} RBTNode;

// tree;
typedef struct RBTRoot
{
    RBTNode *root;
} RBTRoot;

extern RBTRoot *makeRBT();
extern RBTNode *makeRBTNode(Node *node);
extern void RBTInsert(RBTRoot *root, RBTNode *node, int (*cmp) (const void **, const void **) cmp);
extern void RBTDelete(RBTRoot *root, RBTNode *node, int (*cmp) (const void **, const void **) cmp);
extern RBTNode *RBTSearch(RBTRoot *root, Node *key, int (*cmp) (const void **, const void **) cmp);
extern List* RBTGetFirstK(RBTRoot *root, int k, int (*cmp) (const void **, const void **) cmp);
extern Node *RBTGetMin(RBTRoot *root);
extern Node *RBTGetMax(RBTRoot *root);
#endif /* INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_RBTREE_H_ */

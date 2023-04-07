/*
 * rbtree.h
 *
 */

#ifndef INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_RBTREE_H_
#define INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_RBTREE_H_

#include "model/node/nodetype.h"
#include "model/set/vector.h"
#include "model/node/nodetype.h"
#include "model/set/hashmap.h"
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
#define RBT_IS_RED(n) ((n)->color == RBT_RED)
#define RBT_IS_BLACK(n) ((n)->color == RBT_BLACK)
#define RBT_SET_RED(n) ((n)->color = RBT_RED)
#define RBT_SET_BLACK(n) ((n)->color = RBT_BLACK)

// db: min heap, for top k, then compare new value wit min-heap,
// TODO: if applicable of RB tree, replace RB tree with heap in min/max;

typedef enum RBTTYpe
{
    RBT_MIN_HEAP,
    RBT_MAX_HEAP,
    RBT_ORDER_BY
} RBTType;


// treenode;
typedef struct RBTNode
{
    Node *key;               // key;
    struct RBTNode *left;    // left child;
    struct RBTNode *right;   // right child;
    struct RBTNode *parent;  // parent ;
    unsigned char color;     // color: RBT_RED or RBT_BLACK;
} RBTNode;

// tree;
typedef struct RBTRoot
{
    RBTNode *root;
    RBTType type;
    int size;
    HashMap *metadata;       // store some prove sketch info for order by operator;
} RBTRoot;

// for makeRBT: isMetadataNeeded to indicate if need some data, sometimes, is order by one attribute, we can treat orderby as min/max heap but we need to know some info about the ps;
extern RBTRoot *makeRBT(RBTType type, boolean isMetadataNeeded);
extern RBTNode *makeRBTNode(Node *node);
// extern void RBTInsert(RBTRoot *root, RBTNode *node, int (*cmp) (const void **, const void **) cmp);
// extern void RBTDelete(RBTRoot *root, RBTNode *node, int (*cmp) (const void **, const void **) cmp);
// extern RBTNode *RBTSearch(RBTRoot *root, Node *key, int (*cmp) (const void **, const void **) cmp);

// get top K: return a vector size <= K;
extern Vector *RBTGetTopK(RBTRoot *root, int k);
extern Node *RBTGetMin(RBTRoot *root);
extern Node *RBTGetMax(RBTRoot *root);
extern void RBTInsert(RBTRoot *root, Node *node);
extern void RBTDelete(RBTRoot *root, Node *node);
#endif /* INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_RBTREE_H_ */

#ifndef _RBTREE_H_
#define	_RBTREE_H_

#include "model/node/nodetype.h"
#include "model/set/hashmap.h"
#include "model/set/vector.h"

#define DEBUG_RBTREE_BEATIFY_LOG(root) debug_rbtree_beatify_log((RBTRoot *) root)

typedef enum RBTType
{
	RBT_MIN_HEAP,
	RBT_MAX_HEAP,
	RBT_ORDER_BY
} RBTType;

typedef struct RBTNode {
	struct RBTNode   *parent;
	struct RBTNode   *left;
	struct RBTNode   *right;
	Node      		 *key;
	Node      		 *val;
	uint8_t	         color;
} RBTNode;

/** The nullpointer, points to empty node */
#define	RBTREE_NULL &rbtree_null_node
/** the global empty node */
extern	RBTNode	rbtree_null_node;

/** An entire red black tree */
// typedef struct RBTRoot RBTRoot;
/** definition for tree struct */
typedef struct RBTRoot {
	RBTNode    *root;
	size_t      size;
	RBTType treeType;
	HashMap *metadata;
} RBTRoot;

RBTRoot *rbtree_create(int (*cmpf)(const void *, const void *));
void rbtree_init(RBTRoot *rbtree, int (*cmpf)(const void *, const void *));

extern RBTRoot *makeRBT(RBTType treeType, boolean isMetadataNeeded);
extern RBTNode *makeRBTNode(Node *key);
extern void RBTInsert(RBTRoot *root, Node *key, Node *val);
extern void RBTDelete(RBTRoot *root, Node *key, Node *val);
extern Vector *RBTGetTopK(RBTRoot *root, int K);
extern RBTNode *RBTGetMin(RBTRoot *root);
extern RBTNode *RBTGetMax(RBTRoot *root);
extern Vector *RBTInorderTraverse(RBTRoot *root);
#define RBTREE_FOR(node, type, rbtree) \
	for(node=(type)rbtree_first(rbtree); \
		(RBTNode*)node != RBTREE_NULL; \
		node = (type)rbtree_next((RBTNode*)node))
extern void traverse_postorder(RBTRoot* tree, void (*func)(RBTNode*, void*),
	void* arg);
extern RBTNode *rbtree_next(RBTNode *rbtree);
extern RBTNode *rbtree_previous(RBTNode *rbtree);
extern void debug_rbtree_beatify_log(RBTRoot *root);
#endif /* _RBTREE_H_ */

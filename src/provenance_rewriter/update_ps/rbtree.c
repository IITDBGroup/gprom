#include "provenance_rewriter/update_ps/rbtree.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"
#include "log/logger.h"
#include "provenance_rewriter/update_ps/update_ps_build_state.h"
#include "model/set/hashmap.h"
#include "mem_manager/mem_mgr.h"

// #include "config.h"
// #include "log.h"
// #include "fptr_wlist.h"

/** Node colour black */
#define	BLACK	0
/** Node colour red */
#define	RED	1

/** the NULL node, global alloc */
RBTNode	rbtree_null_node = {
	T_Invalid,       /* it should be T_RBTNODE node type but in the NetBSD, the RBTNode == NULL should be the first to make null node*/
	RBTREE_NULL,		/* Parent.  */
	RBTREE_NULL,		/* Left.  */
	RBTREE_NULL,		/* Right.  */
	NULL,			/* Key.  */
	NULL,			/* Val */
	BLACK			/* Color.  */
};
static RBTNode *rbtree_delete(RBTRoot *rbtree, Node *key);
static RBTNode *rbtree_insert(RBTRoot *rbtree, RBTNode *data);
static RBTNode *rbtree_search(RBTRoot *rbtree, Node *key);
static void rbtree_rotate_left(RBTRoot *rbtree, RBTNode *node);
static void rbtree_rotate_right(RBTRoot *rbtree, RBTNode *node);
static void rbtree_insert_fixup(RBTRoot *rbtree, RBTNode *node);
static void rbtree_delete_fixup(RBTRoot* rbtree, RBTNode* child, RBTNode* child_parent);
static RBTNode *rbtree_first(RBTRoot *rbtree);
static RBTNode *rbtree_last(RBTRoot *rbtree);
// static RBTNode *rbtree_next(RBTNode *rbtree);
// static RBTNode *rbtree_previous(RBTNode *rbtree);
static int rbtree_find_less_equal(RBTRoot *rbtree, Node *key, RBTNode **result);
// static void traverse_postorder(RBTRoot* tree, void (*func)(RBTNode*, void*),void* arg);
static int compareTwoNodes(RBTRoot *root, Node *node1, Node *node2);
static void inorderTraverseNodes(RBTNode *node, Vector *v);

#define RBTREE_FOR(node, type, rbtree) \
	for(node=(type)rbtree_first(rbtree); \
		(RBTNode*)node != RBTREE_NULL; \
		node = (type)rbtree_next((RBTNode*)node))

RBTRoot *
makeRBT(RBTType treeType, boolean isMetadataNeeded)
{
	RBTRoot *tree = makeNode(RBTRoot);
	// Node *node = CALLOC(sizeof(RBTRoot), 1);
	// node->type = T_RBTRoot;
	// RBTRoot *tree = (RBTRoot *) node;
	tree->treeType = treeType;
	tree->root = RBTREE_NULL;
	tree->size = 0;
	tree->metadata = NULL;
	if (isMetadataNeeded) {
		tree->metadata = NEW_MAP(Constant, Node);
	}

	return tree;
}

RBTNode *
makeRBTNode(Node *key)
{
	RBTNode *node = makeNode(RBTNode);
	node->key = key;
	node->val = NULL;
	node->left = NULL;
	node->right = NULL;
	node->parent = NULL;
	node->color = BLACK;

	return node;
}

void
RBTInsert(RBTRoot *root, Node *key, Node *val)
{
	if (root->treeType == RBT_MAX_HEAP || root->treeType == RBT_MAX_HEAP) {

	} else {
		RBTNode *data = rbtree_search(root, key);
		// if (data != RBTREE_NULL) {
		if (data != NULL) {
			Constant *cnt = (Constant *) getMap((HashMap *) data->val, val);
			if (cnt != NULL) {
				incrConst(cnt);
			} else {
				addToMap((HashMap *) data->val, val, (Node *) createConstInt(1));
			}
		} else {
			data = makeRBTNode(key);
			data->val = (Node *) NEW_MAP(Node, Node);
			addToMap((HashMap *) data->val, val, (Node *) createConstInt(1));
			rbtree_insert(root, data);
		}
	}
}

void
RBTDelete(RBTRoot *root, Node *key, Node *val)
{
	RBTNode *node = rbtree_search(root, key);
	if (node != NULL) {
		if (root->treeType == RBT_MAX_HEAP || root->treeType == RBT_MIN_HEAP) {

		} else {
			Constant *cnt = (Constant *) getMap((HashMap *) node->val, val);
			if (INT_VALUE(cnt) > 1) {
				INT_VALUE(cnt) = INT_VALUE(cnt) - 1;
			} else {
				removeMapElem((HashMap *) node->val, val);
				if (mapSize((HashMap *) node->val) < 1) {
					rbtree_delete(root, key);
				}
			}
		}
	} else {
		INFO_LOG("NULL NODE");
	}
}

Vector *
RBTGetTopK(RBTRoot *root, int K)
{
	if (root->root == RBTREE_NULL) {
		return NULL;
	}
	INFO_LOG("HAS TOP K");
	// Use a vector as stack;
    Vector *stack = makeVector(VECTOR_NODE, T_Vector);
    int stackSize = 0;

	// Store output values in a vector;
    Vector *topK = makeVector(VECTOR_NODE, T_Vector);
    int topKSize = 0;

	RBTNode *curr = root->root;

	while (curr != RBTREE_NULL || stackSize > 0) {
        while (curr != RBTREE_NULL) {
            vecAppendNode(stack, (Node *) curr);
            stackSize++;
            curr = curr->left;
        }
		// current minimum node;
        curr = (RBTNode *) popVecNode(stack);
        stackSize--;
        // vecAppendNode(topK, curr->key);
        if (root->treeType == RBT_ORDER_BY) {
            HashMap *val = (HashMap *) curr->val;
            FOREACH_HASH_KEY(Node, n, val) {
                int cnt = INT_VALUE((Constant *) getMap(val, n));
                for (int i = 0; i < cnt; i++) {
                    vecAppendNode(topK, n);
                    topKSize++;
                    if (topKSize == K) {
                        break;
                    }
                }
                if (topKSize == K) {
                    break;
                }
            }
        } else {
            vecAppendNode(topK, (Node *) curr);
            topKSize++;
        }

        // pruning
        if (topKSize == K) {
            break;
        }
        curr = curr->right;
    }
    return topK;
}

RBTNode *
RBTGetMin(RBTRoot *root)
{
	return rbtree_first(root);
}

RBTNode *
RBTGetMax(RBTRoot *root)
{
	return rbtree_last(root);
}

Vector *
RBTInorderTraverse(RBTRoot *root)
{
	if (root->root == RBTREE_NULL) {
        return NULL;
    }

    Vector *vector = makeVector(VECTOR_NODE, T_Vector);
    inorderTraverseNodes(root->root, vector);
    return vector;
}

static void
inorderTraverseNodes(RBTNode *node, Vector *v)
{
    if (node == RBTREE_NULL) {
        return;
    }
    vecAppendNode(v, (Node *) node);
    inorderTraverseNodes(node->left, v);
    inorderTraverseNodes(node->right, v);
}

/*
 * Creates a new red black tree, initializes and returns a pointer to it.
 *
 * Return NULL on failure.
 *
 */

RBTRoot *
rbtree_create (int (*cmpf)(const void *, const void *))
{
	RBTRoot *rbtree;

	/* Allocate memory for it */
	rbtree = (RBTRoot *) MALLOC (sizeof(RBTRoot));
	if (!rbtree) {
		return NULL;
	}

	/* Initialize it */
	rbtree_init(rbtree, cmpf);

	return rbtree;
}

void
rbtree_init(RBTRoot *rbtree, int (*cmpf)(const void *, const void *))
{
	/* Initialize it */
	rbtree->root = RBTREE_NULL;
	rbtree->size = 0;
	// rbtree-> = cmpf;
}

/*
 * Rotates the node to the left.
 *
 */
static void
rbtree_rotate_left(RBTRoot *rbtree, RBTNode *node)
{
	RBTNode *right = node->right;
	node->right = right->left;
	if (right->left != RBTREE_NULL)
		right->left->parent = node;

	right->parent = node->parent;

	if (node->parent != RBTREE_NULL) {
		if (node == node->parent->left) {
			node->parent->left = right;
		} else  {
			node->parent->right = right;
		}
	} else {
		rbtree->root = right;
	}
	right->left = node;
	node->parent = right;
}

/*
 * Rotates the node to the right.
 *
 */
static void
rbtree_rotate_right(RBTRoot *rbtree, RBTNode *node)
{
	RBTNode *left = node->left;
	node->left = left->right;
	if (left->right != RBTREE_NULL)
		left->right->parent = node;

	left->parent = node->parent;

	if (node->parent != RBTREE_NULL) {
		if (node == node->parent->right) {
			node->parent->right = left;
		} else  {
			node->parent->left = left;
		}
	} else {
		rbtree->root = left;
	}
	left->right = node;
	node->parent = left;
}

static void
rbtree_insert_fixup(RBTRoot *rbtree, RBTNode *node)
{
	RBTNode	*uncle;

	/* While not at the root and need fixing... */
	while (node != rbtree->root && node->parent->color == RED) {
		/* If our parent is left child of our grandparent... */
		if (node->parent == node->parent->parent->left) {
			uncle = node->parent->parent->right;

			/* If our uncle is red... */
			if (uncle->color == RED) {
				/* Paint the parent and the uncle black... */
				node->parent->color = BLACK;
				uncle->color = BLACK;

				/* And the grandparent red... */
				node->parent->parent->color = RED;

				/* And continue fixing the grandparent */
				node = node->parent->parent;
			} else {				/* Our uncle is black... */
				/* Are we the right child? */
				if (node == node->parent->right) {
					node = node->parent;
					rbtree_rotate_left(rbtree, node);
				}
				/* Now we're the left child, repaint and rotate... */
				node->parent->color = BLACK;
				node->parent->parent->color = RED;
				rbtree_rotate_right(rbtree, node->parent->parent);
			}
		} else {
			uncle = node->parent->parent->left;

			/* If our uncle is red... */
			if (uncle->color == RED) {
				/* Paint the parent and the uncle black... */
				node->parent->color = BLACK;
				uncle->color = BLACK;

				/* And the grandparent red... */
				node->parent->parent->color = RED;

				/* And continue fixing the grandparent */
				node = node->parent->parent;
			} else {				/* Our uncle is black... */
				/* Are we the right child? */
				if (node == node->parent->left) {
					node = node->parent;
					rbtree_rotate_right(rbtree, node);
				}
				/* Now we're the right child, repaint and rotate... */
				node->parent->color = BLACK;
				node->parent->parent->color = RED;
				rbtree_rotate_left(rbtree, node->parent->parent);
			}
		}
	}
	rbtree->root->color = BLACK;
}


/*
 * Inserts a node into a red black tree.
 *
 * Returns NULL on failure or the pointer to the newly added node
 * otherwise.
 */


static RBTNode *
rbtree_insert (RBTRoot *rbtree, RBTNode *data)
{

	/* XXX Not necessary, but keeps compiler quiet... */
	int r = 0;


	/* We start at the root of the tree */
	RBTNode	*node = rbtree->root;
	RBTNode	*parent = RBTREE_NULL;

	// fptr_ok(fptr_whitelist_rbtree_cmp(rbtree->cmp));
	/* Lets find the new parent... */
	while (node != RBTREE_NULL) {
		/* Compare two keys, do we have a duplicate? */
		// if ((r = rbtree->cmp(data->key, node->key)) == 0) {
		if ((r = compareTwoNodes(rbtree, data->key, node->key)) == 0) {
			return NULL;
		}
		parent = node;

		if (r < 0) {
			node = node->left;
		} else {
			node = node->right;
		}
	}

	/* Initialize the new node */
	data->parent = parent;
	data->left = data->right = RBTREE_NULL;
	data->color = RED;
	rbtree->size++;

	/* Insert it into the tree... */
	if (parent != RBTREE_NULL) {
		if (r < 0) {
			parent->left = data;
		} else {
			parent->right = data;
		}
	} else {
		rbtree->root = data;
	}

	/* Fix up the red-black properties... */
	rbtree_insert_fixup(rbtree, data);

	return data;
}

static RBTNode *
rbtree_search (RBTRoot *rbtree, Node *key)
{
	RBTNode *node;

	if (rbtree_find_less_equal(rbtree, key, &node)) {
		return node;
	} else {
		return NULL;
	}
}

/** helpers for delete: swap node colours */
static void swap_int8(uint8_t* x, uint8_t* y)
{
	uint8_t t = *x; *x = *y; *y = t;
}

/** helpers for delete: swap node pointers */
static void swap_np(RBTNode** x, RBTNode** y)
{
	RBTNode* t = *x; *x = *y; *y = t;
}

/** Update parent pointers of child trees of 'parent' */
static void change_parent_ptr(RBTRoot* rbtree, RBTNode* parent,
	RBTNode* old, RBTNode* new)
{
	if(parent == RBTREE_NULL)
	{
		// log_assert(rbtree->root == old);
		if(rbtree->root == old) rbtree->root = new;
		return;
	}
	// log_assert(parent->left == old || parent->right == old
		// || parent->left == new || parent->right == new);
	if(parent->left == old) parent->left = new;
	if(parent->right == old) parent->right = new;
}
/** Update parent pointer of a node 'child' */
static void change_child_ptr(RBTNode* child, RBTNode* old,
	RBTNode* new)
{
	if(child == RBTREE_NULL) return;
	// log_assert(child->parent == old || child->parent == new);
	if(child->parent == old) child->parent = new;
}

static RBTNode*
rbtree_delete(RBTRoot *rbtree, Node *key)
{
	RBTNode *to_delete;
	RBTNode *child;
	if((to_delete = rbtree_search(rbtree, key)) == 0) return 0;
	rbtree->size--;

	/* make sure we have at most one non-leaf child */
	if(to_delete->left != RBTREE_NULL && to_delete->right != RBTREE_NULL)
	{
		/* swap with smallest from right subtree (or largest from left) */
		RBTNode *smright = to_delete->right;
		while(smright->left != RBTREE_NULL)
			smright = smright->left;
		/* swap the smright and to_delete elements in the tree,
		 * but the RBTNode is first part of user data struct
		 * so cannot just swap the keys and data pointers. Instead
		 * readjust the pointers left,right,parent */

		/* swap colors - colors are tied to the position in the tree */
		swap_int8(&to_delete->color, &smright->color);

		/* swap child pointers in parents of smright/to_delete */
		change_parent_ptr(rbtree, to_delete->parent, to_delete, smright);
		if(to_delete->right != smright)
			change_parent_ptr(rbtree, smright->parent, smright, to_delete);

		/* swap parent pointers in children of smright/to_delete */
		change_child_ptr(smright->left, smright, to_delete);
		change_child_ptr(smright->left, smright, to_delete);
		change_child_ptr(smright->right, smright, to_delete);
		change_child_ptr(smright->right, smright, to_delete);
		change_child_ptr(to_delete->left, to_delete, smright);
		if(to_delete->right != smright)
			change_child_ptr(to_delete->right, to_delete, smright);
		if(to_delete->right == smright)
		{
			/* set up so after swap they work */
			to_delete->right = to_delete;
			smright->parent = smright;
		}

		/* swap pointers in to_delete/smright nodes */
		swap_np(&to_delete->parent, &smright->parent);
		swap_np(&to_delete->left, &smright->left);
		swap_np(&to_delete->right, &smright->right);

		/* now delete to_delete (which is at the location where the smright previously was) */
	}
	// log_assert(to_delete->left == RBTREE_NULL || to_delete->right == RBTREE_NULL);

	if(to_delete->left != RBTREE_NULL) child = to_delete->left;
	else child = to_delete->right;

	/* unlink to_delete from the tree, replace to_delete with child */
	change_parent_ptr(rbtree, to_delete->parent, to_delete, child);
	change_child_ptr(child, to_delete, to_delete->parent);

	if(to_delete->color == RED)
	{
		/* if node is red then the child (black) can be swapped in */
	}
	else if(child->color == RED)
	{
		/* change child to BLACK, removing a RED node is no problem */
		if(child!=RBTREE_NULL) child->color = BLACK;
	}
	else rbtree_delete_fixup(rbtree, child, to_delete->parent);

	/* unlink completely */
	to_delete->parent = RBTREE_NULL;
	to_delete->left = RBTREE_NULL;
	to_delete->right = RBTREE_NULL;
	to_delete->color = BLACK;
	return to_delete;
}

static void rbtree_delete_fixup(RBTRoot* rbtree, RBTNode* child,
	RBTNode* child_parent)
{
	RBTNode* sibling;
	int go_up = 1;

	/* determine sibling to the node that is one-black short */
	if(child_parent->right == child) sibling = child_parent->left;
	else sibling = child_parent->right;

	while(go_up)
	{
		if(child_parent == RBTREE_NULL)
		{
			/* removed parent==black from root, every path, so ok */
			return;
		}

		if(sibling->color == RED)
		{	/* rotate to get a black sibling */
			child_parent->color = RED;
			sibling->color = BLACK;
			if(child_parent->right == child)
				rbtree_rotate_right(rbtree, child_parent);
			else	rbtree_rotate_left(rbtree, child_parent);
			/* new sibling after rotation */
			if(child_parent->right == child) sibling = child_parent->left;
			else sibling = child_parent->right;
		}

		if(child_parent->color == BLACK
			&& sibling->color == BLACK
			&& sibling->left->color == BLACK
			&& sibling->right->color == BLACK)
		{	/* fixup local with recolor of sibling */
			if(sibling != RBTREE_NULL)
				sibling->color = RED;

			child = child_parent;
			child_parent = child_parent->parent;
			/* prepare to go up, new sibling */
			if(child_parent->right == child) sibling = child_parent->left;
			else sibling = child_parent->right;
		}
		else go_up = 0;
	}

	if(child_parent->color == RED
		&& sibling->color == BLACK
		&& sibling->left->color == BLACK
		&& sibling->right->color == BLACK)
	{
		/* move red to sibling to rebalance */
		if(sibling != RBTREE_NULL)
			sibling->color = RED;
		child_parent->color = BLACK;
		return;
	}
	// log_assert(sibling != RBTREE_NULL);
	// ASSERT(sibling != RBTREE_NULL);

	/* get a new sibling, by rotating at sibling. See which child
	   of sibling is red */
	if(child_parent->right == child
		&& sibling->color == BLACK
		&& sibling->right->color == RED
		&& sibling->left->color == BLACK)
	{
		sibling->color = RED;
		sibling->right->color = BLACK;
		rbtree_rotate_left(rbtree, sibling);
		/* new sibling after rotation */
		if(child_parent->right == child) sibling = child_parent->left;
		else sibling = child_parent->right;
	}
	else if(child_parent->left == child
		&& sibling->color == BLACK
		&& sibling->left->color == RED
		&& sibling->right->color == BLACK)
	{
		sibling->color = RED;
		sibling->left->color = BLACK;
		rbtree_rotate_right(rbtree, sibling);
		/* new sibling after rotation */
		if(child_parent->right == child) sibling = child_parent->left;
		else sibling = child_parent->right;
	}

	/* now we have a black sibling with a red child. rotate and exchange colors. */
	sibling->color = child_parent->color;
	child_parent->color = BLACK;
	if(child_parent->right == child)
	{
		// log_assert(sibling->left->color == RED);
		// ASSERT(sibling->left->color == RED);
		sibling->left->color = BLACK;
		rbtree_rotate_right(rbtree, child_parent);
	}
	else
	{
		ASSERT(sibling->right->color == RED);
		sibling->right->color = BLACK;
		rbtree_rotate_left(rbtree, child_parent);
	}
}

static int
rbtree_find_less_equal(RBTRoot *rbtree, Node *key,
	RBTNode **result)
{
	int r;
	RBTNode *node;

	// ASSERT(result);

	/* We start at root... */
	node = rbtree->root;

	*result = NULL;
	// fptr_ok(fptr_whitelist_rbtree_cmp(rbtree->cmp));

	/* While there are children... */
	while (node != RBTREE_NULL) {
		// r = rbtree->cmp(key, node->key);
		r = compareTwoNodes(rbtree, key, node->key);
		if (r == 0) {
			/* Exact match */
			*result = node;
			return 1;
		}
		if (r < 0) {
			node = node->left;
		} else {
			/* Temporary match */
			*result = node;
			node = node->right;
		}
	}
	return 0;
}

static RBTNode *
rbtree_first (RBTRoot *rbtree)
{
	RBTNode *node;

	for (node = rbtree->root; node->left != RBTREE_NULL; node = node->left);
	return node;
}

static RBTNode *
rbtree_last (RBTRoot *rbtree)
{
	RBTNode *node;

	for (node = rbtree->root; node->right != RBTREE_NULL; node = node->right);
	return node;
}

RBTNode *
rbtree_next (RBTNode *node)
{
	RBTNode *parent;

	if (node->right != RBTREE_NULL) {
		/* One right, then keep on going left... */
		for (node = node->right; node->left != RBTREE_NULL; node = node->left);
	} else {
		parent = node->parent;
		while (parent != RBTREE_NULL && node == parent->right) {
			node = parent;
			parent = parent->parent;
		}
		node = parent;
	}
	return node;
}

RBTNode *
rbtree_previous(RBTNode *node)
{
	RBTNode *parent;

	if (node->left != RBTREE_NULL) {
		/* One left, then keep on going right... */
		for (node = node->left; node->right != RBTREE_NULL; node = node->right);
	} else {
		parent = node->parent;
		while (parent != RBTREE_NULL && node == parent->left) {
			node = parent;
			parent = parent->parent;
		}
		node = parent;
	}
	return node;
}

/** recursive descent traverse */
static void
traverse_post(void (*func)(RBTNode*, void*), void* arg, RBTNode* node)
{
	if(!node || node == RBTREE_NULL)
		return;
	/* recurse */
	traverse_post(func, arg, node->left);
	traverse_post(func, arg, node->right);
	/* call user func */
	(*func)(node, arg);
}

void
traverse_postorder(RBTRoot* tree, void (*func)(RBTNode*, void*),
	void* arg)
{
	traverse_post(func, arg, tree->root);
}


static int
compareTwoNodes(RBTRoot *root, Node *node1, Node *node2)
{
    int res = 0;
    if (root->treeType == RBT_MIN_HEAP || root->treeType == RBT_MAX_HEAP) {
        DataType dataType = ((Constant *) node1)->constType;
        switch (dataType) {
            case DT_INT:
            {
                res = INT_VALUE(node1) - INT_VALUE(node2) ;
            }
                break;
            case DT_LONG:
            {
                gprom_long_t val = LONG_VALUE(node1) - LONG_VALUE(node2);
                res = (val == 0 ? 0 : (val < 0 ? -1 : 1));
            }
                break;
            case DT_BOOL:
            {
                res = BOOL_VALUE(node1) - BOOL_VALUE(node2);
            }
                break;
            case DT_FLOAT:
            {
                double val = FLOAT_VALUE(node1) - FLOAT_VALUE(node2);
                res = (val == 0 ? 0 : (val < 0 ? -1 : 1));
            }
                break;
            case DT_STRING:
            case DT_VARCHAR2:
            {
                res = strcmp(STRING_VALUE(node1), STRING_VALUE(node2));
            }
                break;
        }
    } else {
        int len = ((Vector *) node1)->length;
        Vector *orderByASC = (Vector *) MAP_GET_STRING(root->metadata, ORDER_BY_ASCS);
        // int res = 0;
        for (int i = 0; i < len; i++) {
            Constant *c1 = (Constant *) getVecNode((Vector *) node1, i);
            Constant *c2 = (Constant *) getVecNode((Vector *) node2, i);
            DataType dt = c1->constType;
            switch(dt) {
                case DT_INT:
                {
                    res = INT_VALUE(c1) - INT_VALUE(c2);
                }
                break;
                case DT_BOOL:
                {
                    res = BOOL_VALUE(c1) - BOOL_VALUE(c2);
                }
                break;
                case DT_FLOAT:
                {
                    res = FLOAT_VALUE(c1) - FLOAT_VALUE(c2);
                }
                break;
                case DT_LONG:
                {
                    res = LONG_VALUE(c1) - LONG_VALUE(c2);
                }
                break;
                case DT_STRING:
                case DT_VARCHAR2:
                {
                    res = strcmp(STRING_VALUE(c1), STRING_VALUE(c2));
                }
                break;
            }
            if (res != 0) {
                return res * getVecInt(orderByASC, i);
            }
        }

    }
    return res;
}

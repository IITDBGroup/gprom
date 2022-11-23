#include "provenance_rewriter/update_ps/rbtree.h"

RBTreeNode *
RBTreeInsert(RBTreeNode *root, RBTreeNode *node, int (*cmp) (const void **, const void **) cmp)
{
    RBTreeNode *y = NULL;
    RBTreeNode *x = root;

    // traverse tree to find the position to insert;
    while (x != NULL) {
        y = x;
        if (cmp(node->key, x->key) < 0) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    // set parent of "node" to y;
    RBTREE_SET_PARENT(node, y);

    if (y == NULL) {
        if (cmp(node->key, y->key) < 0) {
            y->left = node;                 // case 2: node < y,
        } else {
            y->right = node;                // case 3: node >=y,
        }
    } else {                                // y is null, set node as root;
        root = node;
    }

    // set color to red;
    RBTREE_SET_RED(node);

    // rotate to a BST;
    RBTreeInsertFixup(root, node);
}

void
RBTreeDelete(RBTreeNode **root, RBTreeNode *node)
{

}

static void
RBTreeInsertFixup(RBTreeNode *root, RBTreeNode *node)
{
    RBTreeNode *parent = RBTREE_PARENT(node);
    RBTreeNode *gParent;

    // parent exists, and parent's color is red;
    while (parent && RBTREE_IS_RED(parent)) {
        gParent = RBTREE_PARENT(parent);

        // parent is left of grand parent;
        if (parent == gParent->left) {
            RBTreeNode *uncle = RBTREE_RCHILD(gParent);

            // 1. uncle is red;
            if (uncle && RBTREE_IS_RED(uncle)) {
                RBTREE_SET_BLACK(uncle);
                RBTREE_SET_BLACK(parent);
                RBTREE_SET_RED(gParent);
                node = gParent;
                continue;
            }
        }
    }
}
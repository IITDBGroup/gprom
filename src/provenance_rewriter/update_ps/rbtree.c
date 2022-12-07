#include "provenance_rewriter/update_ps/rbtree.h"

static void RBTLeftRotate(RBTRoot *root, RBTNode *node);
static void RBTRightRotate(RBTRoot *root, RBTNode *node);
static void RBTInsertFixup(RBTRoot *root, RBTNode *node);
static void RBTDeleteFixup(RBTRoot *root, RBTNode *node);



RBTRoot *
makeRBT()
{
    RBTRoot *tree = NEW(RBTRoot);
    tree->root = TNIL;
    return tree;
}

RBTNode *
makeRBTNode(Node *key, RBTNode *left, RBTNode *right, RBTNode *parent)
{
    RBTNode *node = NEW(RBTNode);

    node->key = key;
    node->left = left;
    node->right = right;
    node->parent = parent;
    node->color = RBT_BLACK; // default value is BLACK;

    return node;
}

// inorder traverse;
RBTNode *
RBTSearch(RBTNode *root, Node *key, int (*cmp) (const void **, const void **) cmp)
{
    if (root->root == TNIL) {
        return TNIL;
    }

    /* recursive */
    // int cmpreturn = cmp((void *) key, (void *) root);
    // if (cmpRes == 0) {
    //     return root;
    // } else if (cmpRes < 0) {
    //     return RBTSearch(root->left, key, cmp);
    // } else {
    //     return RBTSearch(root->right, key, cmp);
    // }

    // iterative;
    RBTNode *res = root;

    while(res != TNIL) {
        int cmpV = cmp(key, res);
        if (cmpV == 0) {
            return res;
        } else if (cmpV > 0) {
            res = res->left;
        } else {
            res = res->right;
        }
    }

    return res;
}

void
RBTInsert(RBTRoot *root, RBTNode *node, int (*cmp) (const void **, const void **) cmp)
{
    RBTNode *y = TNIL;
    RBTNode *x = root->root;

    // traverse tree to find the position to insert;
    while (x != TNIL) {
        y = x;
        if (cmp(node->key, x->key) < 0) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    // set parent of "node" to y;
    RBT_SET_PARENT(node, y);

    if (y != NULL) {
        if (cmp(node->key, y->key) < 0) {
            y->left = node;                 // case 2: node < y, set node as left child of y,
        } else {
            y->right = node;                // case 3: node >=y, set node as right child of y,
        }
    } else {                                // y is null, set node as root;
        root->root = node;
    }

    // set node color to red;
    RBT_SET_RED(node);

    // fixup to a BST;
    RBTInsertFixup(root, node);
}

void
RBTreeDelete(RBTRoot *root, RBTNode *node)
{
    RBTNode *child, *parent;
    unsigned char color = node->color;

    // both children of deleted node exist
    if ((node->left != TNIL) && (node->right != TNIL)) {

        return;
    }

    if (node->left == TNIL) {
        child = node->right;
    } else {
        child = node->left;
    }

    parent = node->parent;
    color = node->color;

    if (child) {
        child->parent = parent;
    }

    // transplant as INTRO_TO_ALGORITHMS
    if (parent) {
        if (node == parent->left) {
            parent->left = child;
        } else {
            parent->right = child;
        }
    } else {
        root->root = child;
    }

    // delete fixup;
    if (color == RBT_BLACK) {
        RBTDeleteFixup(root, child);
    }
}

Node *
RBTGetMin(RBTRoot *root)
{
    if (root == TNIL || root->root == TNIL) {
        return TNIL;
    }

    RBTNode *node = root->root;
    while (node->left != TNIL) {
        node = node->left;
    }

    return node->key;
}

Node *
RBTGetMax(RBTRoot *root)
{
    if (root == TNIL || root->root == TNIL) {
        return TNIL;
    }

    RBTNode *node = root->root;
    while (node->right != TNIL) {
        node = node->right;
    }

    return node->key;
}

static void
RBTInsertFixup(RBTRoot *root, RBTNode *node)
{
    RBTNode *parent, *gParent;

    // parent exists, and parent's color is red;
    while (node && (parent = RBT_PARENT(node)) && RBT_IS_RED(parent)) {
        gParent = RBT_PARENT(parent);

        // parent is left of grand parent;
        if (gParent && parent == gParent->left) {
            RBTNode *uncle = RBTREE_RCHILD(gParent);

            // 1. uncle is red;
            if (uncle && RBT_IS_RED(uncle)) {
                RBT_SET_BLACK(uncle);
                RBT_SET_BLACK(parent);
                RBT_SET_RED(gParent);
                node = gParent;
                continue;
            }

            // 2. uncle is black, and node is right child
            if (node == RBT_LCHILD(parent)) {
                RBTLeftRotate(root, parent);
                RBTNode *tmp = parent;
                parent = node;
                node = tmp;
            }

            // 3. uncle is black, and node is left child;
            RBT_SET_BLACK(parent);
            RBT_SET_RED(gParent);
            RBTRightRotate(root, gParent);
        } else { // parent is right child of grand parent; symmetric of above case
            RBTNode *uncle = RBT_LCHILD(gParent);

            if (uncle && RBT_IS_RED(uncle)) {
                RBT_SET_BLACK(uncle);
                RBT_SET_BLACK(parent);
                RBT_SET_RED(gParent);
                node = gParent;
            }

            if (node == RBT_LCHILD(parent)) {
                RBTRightRotate(root, parent);
                RBTNode *tmp = parent;
                parent = node;
                node = tmp;
            }

            RBT_SET_BLACK(parent);
            RBT_SET_RED(gParent);
            RBTLeftRotate(root, gParent);
        }

        RBT_SET_BLACK(root->root);
    }
}

static void
RBTDeleteFixup(RBTRoot *root, RBTNode *node)
{
    while ((!node || RBT_IS_BLACK(node)) && node != root->root) {
        RBTNode *parent = node->parent;
        if(node == parent->left) {
            RBTNode *tmp = parent->right;
            if (RBT_IS_RED(tmp)) {
                RBT_SET_BLACK(tmp);
                RBT_SET_RED(parent);
                RBTLeftRotate(root, parent);
                tmp = parent->right;
            }

            if ((tmp->left && RBT_IS_BLACK(tmp->left))
            && (tmp->right && RBT_IS_BLACK(tmp->right))) {
                RBT_SET_RED(tmp);
                node = parent;
            } else {
                if (tmp->right && RBT_IS_BLACK(tmp->right)) {
                    RBT_SET_BLACK(tmp->left);
                    RBT_SET_RED(tmp);
                    RBTRightRotate(root, tmp);
                    tmp = parent->right;
                }

                RBT_SET_COLOR(tmp, RBT_COLOR(parent));
                RBT_SET_BLACK(parent);
                RBT_SET_BLACK(tmp->right);
                RBTLeftRotate(root, parent);
                node = root->root;
            }

        } else {
            RBTNode *tmp = parent->left;
            if(RBT_IS_RED(tmp)) {
                RBT_SET_BLACK(tmp);
                RBT_SET_RED(parent);
                RBTRightRotate(root, parent);
                tmp = parent->left;
            }

            if ((tmp->left && RBT_IS_BLACK(tmp->left)
            && (tmp->right && RBT_IS_BLACK(tmp->right))) {
                RBT_SET_RED(tmp);
                node = parent;
            } else {
                if (tmp->left && RBT_IS_BLACK(tmp->left)) {
                    RBT_SET_BLACK(tmp->right);
                    RBT_SET_RED(tmp);
                    RBTLeftRotate(root, tmp);
                    tmp = parent->left;
                }
                RBT_SET_COLOR(tmp, RBT_COLOR(parent));
                RBT_SET_BLACK(parent);
                RBT_SET_BLACK(tmp->left);
                RBTRightRotate(root, parent);
            }
        }
    }

    if (node) {
        RBT_SET_BLACK(node);
    }
}

static void
RBTLeftRotate(RBTRoot *root, RBTNode *node)
{
    // y: right child of "node";
    RBTNode *y = RBT_RCHILD(node);

    node->right = y->left;

    // set "node" as parent of y->left;
    if (y->left != TNIL) {
        RBT_SET_PARENT(y->left, node);
    }

    RBT_SET_PARENT(y, RBT_PARENT(node));

    if (!RBT_PARENT(node)) {
        root->root = y;
    } else {
        if (RBT_LCHILD(RBT_PARENT(node)) == node) {
            node->parent->left = y;
        } else {
            node->parent->right = y;
        }
    }

    y->left = node;
    RBT_SET_PARENT(node, y);
}

/* right rotate is symmetric of left rotate*/
static void
RBTRightRotate(RBTRoot *root, RBTNode *node)
{
    RBTNode *y = RBT_LCHILD(node);

    node->left = y->right;

    if (!RBT_RCHILD(node)) {
        RBT_SET_PARENT(y->right, node);
    }

    RBT_SET_PARENT(y, RBT_PARENT(node));

    if (!RBT_PARENT(node)) {
        root->root = y;
    } else {
        if (RBT_RCHILD(RBT_PARENT(node)) == node) {
            node->parent->right = y;
        } else {
            node->parent->left = y;
        }
    }

    y->right = node;
    RBT_SET_PARENT(node, y);
}
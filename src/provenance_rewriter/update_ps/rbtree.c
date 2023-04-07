#include "provenance_rewriter/update_ps/rbtree.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"


static void RBTLeftRotate(RBTRoot *root, RBTNode *node);
static void RBTRightRotate(RBTRoot *root, RBTNode *node);
static void RBTInsertNode(RBTRoot *root, RBTNode *node);
static void RBTDeleteNode(RBTRoot *root, RBTNode *node);
static void RBTInsertFixup(RBTRoot *root, RBTNode *node);
static void RBTDeleteFixup(RBTRoot *root, RBTNode *node);
static RBTNode *RBTSearchNode(RBTRoot *root, Node *node);
static int compareTwoNodes(RBTType type, Node *node1, Node *node2);

RBTRoot *
makeRBT(RBTType type, boolean isMetadataNeeded)
{
    RBTRoot *tree = makeNode(RBTRoot);

    tree->type = type;
    tree->root = NULL;
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
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->color = RBT_BLACK;

    return node;
}

void
RBTInsert(RBTRoot *root, Node *node)
{
    // TODO: what should do if two nodes has same key??
    // TODO: for a ps, there can be two nodes with same value but different fragments info
    RBTNode *insertedNode = makeRBTNode(node);
    RBTInsertNode(root, insertedNode);
    root->size++;
}

void
RBTDelete(RBTRoot *root, Node *node)
{
    RBTNode *nodeInTree = RBTSearchNode(root, node);
    if (nodeInTree != NULL) {
        RBTDeleteNode(root, nodeInTree);
        root->size--;
    }
}

Node *
RBTGetMin(RBTRoot *root)
{
    if (root == NULL || root->root == NULL) {
        return NULL;
    }

    // iterate to the left most tree node;
    RBTNode *res = root->root;
    while (res->left != NULL) {
        res = res->left;
    }

    return res->key;
}

Node *
RBTGetMax(RBTRoot *root)
{
    if (root == NULL || root->root == NULL) {
        return NULL;
    }

    RBTNode *res = root->root;
    while (res->right != NULL) {
        res = res->right;
    }

    return res->key;
}

// inorder traverse
Vector *
RBTGetTopK(RBTRoot *root, int K)
{
    // use vector as a stack;
    Vector *stack = makeVector(VECTOR_NODE, T_Vector);
    int stackSize = 0;

    // return vector;
    Vector *topK = makeVector(VECTOR_NODE, T_Vector);
    int topKSize = 0;

    RBTNode *curr = root->root;
    // inorder iterative all nodes in rbtree
    while (curr != NULL || stackSize > 0) {
        while (curr != NULL) {
            vecAppendNode(stack, (Node *) curr);
            stackSize++;
            curr = curr->left;
        }
        // current minimum node;
        curr = (RBTNode *) popVecNode(stack);
        stackSize--;

        vecAppendNode(topK, curr->key);
        topKSize++;
        // pruning
        if (topKSize == K) {
            break;
        }
        curr = curr->right;
    }
    // the returned vector size <= K;
    return topK;
}

// inorder traverse;
// static RBTNode *
// RBTSearchNode(RBTNode *root, Node *key, int (*cmp) (const void **, const void **) cmp)
static RBTNode *
RBTSearchNode(RBTRoot *root, Node *node)
{
    if (root->root == NULL) {
        return NULL;
    }

    RBTType type = root->type;
    // iterative;
    RBTNode *res = root->root;

    while(res != NULL) {
        int cmpV = compareTwoNodes(type, res->key, node); // TODO: need revising
        if (cmpV == 0) {
            return res;
        } else if (cmpV < 0) {
            res = res->left;
        } else {
            res = res->right;
        }
    }

    return res;
}

static int
compareTwoNodes(RBTType type, Node *node1, Node *node2)
{
    int res = 0;
    if (type == RBT_MIN_HEAP || type == RBT_MAX_HEAP) {
        DataType dataType = ((Constant *) node1)->constType;
        switch (dataType) {
            case DT_INT:
                res = INT_VALUE(node1) - INT_VALUE(node2) ;
                break;
            case DT_LONG:
                gprom_long_t val = LONG_VALUE(node1) - LONG_VALUE(node2);
                res = (val == 0 ? 0 : (val < 0 ? -1 : 1));
                break;
            case DT_BOOL:
                res = BOOL_VALUE(node1) - BOOL_VALUE(node2);
                break;
            case DT_FLOAT:
                double val = FLOAT_VALUE(node1) - FLOAT_VALUE(node2);
                res = (val == 0 ? 0 : (val < 0 ? -1 : 1));
                break;
            case DT_STRING:
            case DT_VARCHAR2:
                res = strcmp(STRING_VALUE(node1), STRING_VALUE(node2));
                break;
        }
        // in a tree, left node is smaller than node, for max heap, left value is "logically" smaller than node;
        if (type == RBT_MAX_HEAP) {
            res *= (-1);
        }
    } else {

    }
    return res;
}

// RBTInsert(RBTRoot *root, RBTNode *node, int (*cmp) (const void **, const void **) cmp)
static void
RBTInsertNode(RBTRoot *root, RBTNode *node)
{
    RBTNode *y = NULL;
    RBTNode *x = root->root;
    RBTType type = root->type;
    // traverse tree to find the position to insert;
    while (x != NULL) {
        y = x;
        int compV = compareTwoNodes(type, node->key, x->key);
        if (compV < 0) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    // set parent of "node" to y;
    RBT_SET_PARENT(node, y);

    if (y != NULL) {
        int compV = compareTwoNodes(type, node->key, y->key);
        if (compV < 0) {
            y->left = node;                 // case 2: node < y, set node as left child of y,
        } else {
            y->right = node;                // case 3: node >=y, set node as right child of y,
        }
    } else {
        root->root = node;                  // case 1: y is null, set node as root;
    }

    // set node color to red;
    RBT_SET_RED(node);

    // fixup to a BST;
    RBTInsertFixup(root, node);
}

static void
RBTDeleteNode(RBTRoot *root, RBTNode *node)
{
    RBTNode *child, *parent;
    unsigned char color = node->color;

    // both children of deleted node exist
    if ((node->left != NULL) && (node->right != NULL)) {
        // get the "successor" of the deleted "node" and replace "node" with "successor"
        RBTNode *successor = node;
        successor = successor->right;
        while (successor->left != NULL) {
            successor = successor->left;
        }

        // node is not root(only root does not have parent)
        if (RBT_PARENT(node) != NULL) {
            if (RBT_PARENT(node)->left == node) {
                RBT_PARENT(node)->left = successor;
            } else {
                RBT_PARENT(node)->right = successor;
            }
        } else {
            // "node" is root, now update root to "successor";
            root->root = successor;
        }

        // "child" is right child of "successor", adjust
        // note: "successor" does exist left child.
        child = successor->right;
        parent = RBT_PARENT(successor);

        // store the color of "successor";
        color = RBT_COLOR(successor);

        if (parent == node) {
            parent = successor;
        } else {
            if (child != NULL) {
                RBT_SET_COLOR(child, RBT_COLOR(parent));
            }

            parent->left = child;
            successor->right = node->right;
            RBT_SET_PARENT(node->right, successor);
        }

        successor->parent = node->parent;
        successor->color = node->color;
        successor->left = node->left;
        RBT_SET_PARENT(node->left->parent, successor);

        if (color == RBT_BLACK) {
            RBTDeleteFixup(root, child);
            // RBTDeleteFixup(root, child, parent);
        }
        return;
    }

    if (node->left == NULL) {
        child = node->right;
    } else {
        child = node->left;
    }

    parent = node->parent;
    color = node->color;

    if (child != NULL) {
        child->parent = parent;
    }

    // transplant as INTRO_TO_ALGORITHMS
    if (parent != NULL) {
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
        // RBTDeleteFixup(root, child, parent);
    }
}

static void
RBTInsertFixup(RBTRoot *root, RBTNode *node)
{
    RBTNode *parent, *gParent;

    // parent exists, and parent's color is red;
    while (node && (parent = RBT_PARENT(node)) && RBT_IS_RED(parent)) {
        gParent = RBT_PARENT(parent);

        // parent is left of grand parent;
        if (parent == gParent->left) {
            RBTNode *uncle = RBT_RCHILD(gParent);

            // 1. uncle is red;
            if (uncle && RBT_IS_RED(uncle)) {
                RBT_SET_BLACK(uncle);
                RBT_SET_BLACK(parent);
                RBT_SET_RED(gParent);
                node = gParent;
                continue;
            }

            // 2. uncle is black(case 1 not hold), and node is right child
            if (node == RBT_RCHILD(parent)) {
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
            // case 1: uncle is red;
            if (uncle && RBT_IS_RED(uncle)) {
                RBT_SET_BLACK(uncle);
                RBT_SET_BLACK(parent);
                RBT_SET_RED(gParent);
                node = gParent;
                continue;
            }

            // case 2: uncle is black, and node is left child;
            if (node == RBT_LCHILD(parent)) {
                RBTRightRotate(root, parent);
                RBTNode *tmp = parent;
                parent = node;
                node = tmp;
            }

            // case 3: uncle is black, and node is right child;
            RBT_SET_BLACK(parent);
            RBT_SET_RED(gParent);
            RBTLeftRotate(root, gParent);
        }
    }
    // set root color to black;
    RBT_SET_BLACK(root->root);
}

static void
RBTDeleteFixup(RBTRoot *root, RBTNode *node)
// RBTDeleteFixup(RBTRoot *root, RBTNode *node, RBTNode *parent)
{
    while (node && RBT_IS_BLACK(node) && node != root->root) {
        // RBTNode *parent = node->parent;
        if(node == RBT_LCHILD(RBT_PARENT(node))) {
            RBTNode *sibling = RBT_RCHILD(RBT_PARENT(node));
            // case 1: node's sibling is red;
            if (sibling && RBT_IS_RED(sibling)) {
                RBT_SET_BLACK(sibling);
                RBT_SET_RED(RBT_PARENT(node));
                RBTLeftRotate(root, RBT_PARENT(node));
                sibling = RBT_PARENT(node)->right;
            }

            if (sibling
            && (RBT_LCHILD(sibling) && RBT_IS_BLACK(RBT_LCHILD(sibling)))
            && (RBT_RCHILD(sibling) && RBT_IS_BLACK(RBT_RCHILD(sibling)))) {
                // case 2: sibling color is black and two children are black;
                RBT_SET_RED(sibling);
                node = RBT_PARENT(node);
                // parent = RBT_PARENT(node);
            } else {
                if (sibling
                && RBT_RCHILD(sibling)
                && RBT_IS_BLACK(RBT_RCHILD(sibling))) {
                    RBT_SET_BLACK(RBT_LCHILD(sibling));
                    RBT_SET_RED(sibling);
                    RBTRightRotate(root, sibling);
                    sibling = RBT_RCHILD(RBT_PARENT(node));
                }

                RBT_SET_COLOR(sibling, RBT_COLOR(RBT_PARENT(node)));
                RBT_SET_BLACK(RBT_PARENT(node));
                RBT_SET_BLACK(RBT_RCHILD(sibling));
                RBTLeftRotate(root, RBT_PARENT(node));
                node = root->root;
            }

        } else {
            RBTNode *sibling = RBT_LCHILD(RBT_PARENT(node));
            // RBTNode *tmp = parent->left;
            if(sibling && RBT_IS_RED(sibling)) {
                RBT_SET_BLACK(sibling);
                RBT_SET_RED(RBT_PARENT(node));
                RBTRightRotate(root, RBT_PARENT(node));
                sibling = RBT_LCHILD(RBT_PARENT(node));
            }

            if (sibling
            && (RBT_LCHILD(sibling) && RBT_IS_BLACK(RBT_LCHILD(sibling)))
            && (RBT_RCHILD(sibling) && RBT_IS_BLACK(RBT_RCHILD(sibling)))) {
                RBT_SET_RED(sibling);
                node = RBT_PARENT(node);
            } else {
                if (sibling
                && RBT_LCHILD(sibling)
                && RBT_IS_BLACK(RBT_LCHILD(sibling))) {
                    RBT_SET_BLACK(RBT_RCHILD(sibling));
                    RBT_SET_RED(sibling);
                    RBTLeftRotate(root, sibling);
                    sibling = RBT_LCHILD(RBT_PARENT(node));
                }
                RBT_SET_COLOR(sibling, RBT_COLOR(RBT_PARENT(node)));
                RBT_SET_BLACK(RBT_PARENT(node));
                RBT_SET_BLACK(RBT_LCHILD(sibling));
                RBTRightRotate(root, RBT_PARENT(node));
            }
        }
    }

    if (node) {
        RBT_SET_BLACK(node);
    }
}

/*
        |      Left X        |
        X      ------>       Y
       / \                  / \
      a   Y    <------     X   c
         / \   Right Y    / \
        b   c            a   b
    Left rotate on X : to make X as a left child.
    Right rotate on Y: to make Y as a right child.
*/
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

    if (RBT_PARENT(node) == NULL) {
        root->root = y;
    } else {
        if (RBT_LCHILD(RBT_PARENT(node)) == node) {
            RBT_PARENT(node)->left = y;
            // node->parent->left = y;
        } else {
            RBT_PARENT(node)->right = y;
            // node->parent->right = y;
        }
    }

    y->left = node;
    RBT_SET_PARENT(node, y);
}

/* right rotate is symmetric of left rotate*/
static void
RBTRightRotate(RBTRoot *root, RBTNode *node)
{
    RBTNode *x = RBT_LCHILD(node);
    node->left = x->right;

    if (x->right != NULL) {
        x->right->parent = node;
    }

    x->parent = node->parent;
    if (node->parent == NULL) {
        root->root = x;
    } else {
        if (node == node->parent->left) {
            node->parent->left = x;
        } else {
            node->parent->right = x;
        }
    }

    x->parent = node;
    RBT_SET_PARENT(node, x);
}

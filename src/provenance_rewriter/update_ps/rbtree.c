#include "provenance_rewriter/update_ps/rbtree.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"
#include "log/logger.h"
#include "provenance_rewriter/update_ps/update_ps_build_state.h"

static void RBTLeftRotate(RBTRoot *root, RBTNode *node);
static void RBTRightRotate(RBTRoot *root, RBTNode *node);
static void RBTInsertNode(RBTRoot *root, RBTNode *node);
static void RBTDeleteNode(RBTRoot *root, RBTNode *node);
static void RBTInsertFixup(RBTRoot *root, RBTNode *node);
static void RBTDeleteFixup(RBTRoot *root, RBTNode *node, RBTNode *parent);
static RBTNode *RBTSearchNode(RBTRoot *root, Node *node);
static int compareTwoNodes(RBTRoot *root, Node *node1, Node *node2);
static void inorderTraverseNodes(RBTNode *node, Vector *v);

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
    node->val = NULL;
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->color = RBT_BLACK;

    return node;
}

void
RBTInsert(RBTRoot *root, Node *key, Node *val)
{
    if (root->type == RBT_MIN_HEAP || root->type == RBT_MAX_HEAP) {
        RBTNode *treeNode = RBTSearchNode(root, key);
        if (treeNode != NULL) {
            incrConst((Constant *) treeNode->val);
        } else {
            treeNode = makeRBTNode(key);
            treeNode->val = (Node *) createConstInt(1);
            RBTInsertNode(root, treeNode);
            root->size++;
        }
    } else {
        RBTNode *treeNode = RBTSearchNode(root, key);
        if (treeNode != NULL) {
            Node *n = getMap((HashMap *) treeNode->val, val);
            if (n != NULL) {
                incrConst((Constant *) n);
            } else {
                addToMap((HashMap *) treeNode->val, (Node *) val, (Node *) createConstInt(1));
            }
        } else {
            RBTNode *insertedNode = makeRBTNode(key);
            insertedNode->val = (Node *) NEW_MAP(Node, Node);
            addToMap((HashMap *) insertedNode->val, (Node *) val, (Node *) createConstInt(1));
            RBTInsertNode(root, insertedNode);
            root->size++;
        }
    }
}

void
RBTDelete(RBTRoot *root, Node *key, Node *val)
{
    RBTNode *nodeInTree = RBTSearchNode(root, key);

    if (nodeInTree != NULL) {
        if (root->type == RBT_MIN_HEAP || root->type == RBT_MAX_HEAP) {
            if (INT_VALUE(nodeInTree->val) > 1) {
                // (*((int *) ((Constant *) nodeInTree->val))) -= 1;
                INT_VALUE((Constant *) nodeInTree->val) = INT_VALUE((Constant *) nodeInTree->val) - 1;
            } else {
                RBTDeleteNode(root, nodeInTree);
                root->size--;
            }
        } else {
            Node *n = (Node *) getMap((HashMap *) nodeInTree->val, val);
            if (INT_VALUE((Constant *) n) > 1) {
                INT_VALUE((Constant *) n) = INT_VALUE((Constant *) n) - 1;
            } else {
                removeMapElem((HashMap *) nodeInTree->val, val);
                if (mapSize((HashMap *) nodeInTree->val) < 1) {
                    RBTDeleteNode(root, nodeInTree);
                    root->size--;
                }
            }
        }
    } else {
        INFO_LOG("NULL NODE");
    }
}

RBTNode *
RBTGetMin(RBTRoot *root)
{
    if (root == NULL || root->root == NULL) {
        return NULL;
    }

    // Loop to the left most tree node;
    RBTNode *res = root->root;
    while (res->left != NULL) {
        res = res->left;
    }

    return res;
}

RBTNode *
RBTGetMax(RBTRoot *root)
{
    if (root == NULL || root->root == NULL) {
        return NULL;
    }

    // Loop to the right most tree node;
    RBTNode *res = root->root;
    while (res->right != NULL) {
        res = res->right;
    }

    return res;
}

/*  Inorder traverse to get top K  */
Vector *
RBTGetTopK(RBTRoot *root, int K)
{
    if (root->root == NULL) {
        return NULL;
    }
    // Use a vector as stack;
    Vector *stack = makeVector(VECTOR_NODE, T_Vector);
    int stackSize = 0;

    // Store output values in a vector;
    Vector *topK = makeVector(VECTOR_NODE, T_Vector);
    int topKSize = 0;

    RBTNode *curr = root->root;
    while (curr != NULL || stackSize > 0) {
        while (curr != NULL) {
            vecAppendNode(stack, (Node *) curr);
            stackSize++;
            curr = curr->left;
        }

        // current minimum node;
        curr = (RBTNode *) popVecNode(stack);
        stackSize--;
        // vecAppendNode(topK, curr->key);
        if (root->type == RBT_ORDER_BY) {
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

Vector *
RBTInorderTraverse(RBTRoot *root)
{
    if (root->root == NULL) {
        return NULL;
    }

    // return RBTGetTopK(root, 100000);
    Vector *vector = makeVector(VECTOR_NODE, T_Vector);
    inorderTraverseNodes(root->root, vector);
    return vector;
}

static void
inorderTraverseNodes(RBTNode *node, Vector *v)
{
    if (node == NULL) {
        return;
    }
    vecAppendNode(v, (Node *) node);
    inorderTraverseNodes(node->left, v);
    inorderTraverseNodes(node->right, v);
}

static RBTNode *
RBTSearchNode(RBTRoot *root, Node *node)
{
    if (root->root == NULL) {
        return NULL;
    }

    // RBTType type = root->type;
    RBTNode *res = root->root;
    while(res != NULL) {
        int cmpV = compareTwoNodes(root, node, res->key);
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
compareTwoNodes(RBTRoot *root, Node *node1, Node *node2)
{
    int res = 0;
    if (root->type == RBT_MIN_HEAP || root->type == RBT_MAX_HEAP) {
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
                    res = BOOL_VALUE(c1) - BOOL_VALUE(c1);
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

static void
RBTInsertNode(RBTRoot *root, RBTNode *node)
{
    RBTNode *y = NULL;
    RBTNode *x = root->root;
    // RBTType type = root->type;
    // traverse tree to find the position to insert;
    while (x != NULL) {
        y = x;
        int compV = compareTwoNodes(root, node->key, x->key);
        if (compV < 0) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    // set parent of "node" to y;
    RBT_SET_PARENT(node, y);

    if (y != NULL) {
        int compV = compareTwoNodes(root, node->key, y->key);
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
    if ((node->left) && (node->right)) {
        // get the "successor" of the deleted "node" and replace "node" with "successor"
        RBTNode *successor = node;
        successor = successor->right;
        // while (successor->left) {
        while(RBT_LCHILD(successor)) {
            successor = successor->left;
        }

        // node is not root(only root does not have parent)
        if (RBT_PARENT(node)) {
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
        // note: "successor" does exist left child because if exists, it cannot be the successor
        child = successor->right;
        parent = RBT_PARENT(successor);

        // store the color of "successor";
        color = RBT_COLOR(successor);

        if (parent == node) {
            parent = successor;
        } else {
            if (child != NULL) {
                // RBT_SET_COLOR(child, RBT_COLOR(parent));
                RBT_SET_PARENT(child, parent);
            }

            parent->left = child;
            successor->right = node->right;
            RBT_SET_PARENT(node->right, successor);
        }

        successor->parent = node->parent;
        successor->color = node->color;
        successor->left = node->left;
        // RBT_SET_PARENT(node->left->parent, successor);
        node->left->parent = successor;

        if (color == RBT_BLACK) {
            // RBTDeleteFixup(root, child);
            RBTDeleteFixup(root, child, parent);
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

    if (child) {
        child->parent = parent;
    }

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
        // RBTDeleteFixup(root, child);
        RBTDeleteFixup(root, child, parent);
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
RBTDeleteFixup(RBTRoot *root, RBTNode *node, RBTNode *parent)
{
    RBTNode *sibling;
    while (node && RBT_IS_BLACK(node) && node != root->root) {
        if (node == RBT_PARENT(node)->left) {
            sibling = RBT_RCHILD(RBT_PARENT(node));
            if (RBT_IS_RED(sibling)) {
                RBT_SET_BLACK(sibling);
                RBT_SET_RED(parent);
                RBTLeftRotate(root, parent);
                sibling = parent->right;
            }

            if ((!sibling->left || RBT_IS_BLACK(sibling->left))
            && (!sibling->right || RBT_IS_BLACK(sibling->right))) {
                RBT_SET_RED(sibling);
                node = parent;
                parent = RBT_PARENT(node);
            } else {
                if (!sibling->right || RBT_IS_BLACK(sibling->right)) {
                    RBT_SET_BLACK(sibling->left);
                    RBT_SET_RED(sibling);
                    RBTRightRotate(root, sibling);
                    sibling = parent->right;
                }

                RBT_SET_COLOR(sibling, RBT_COLOR(parent));
                RBT_SET_BLACK(parent);
                RBT_SET_BLACK(sibling->right);
                node = root->root;
                break;
            }
        } else {
            sibling = RBT_LCHILD(RBT_PARENT(node));
            if (RBT_IS_RED(sibling)) {
                RBT_SET_BLACK(sibling);
                RBT_SET_RED(parent);
                RBTRightRotate(root, parent);
                sibling = parent->left;
            }

            if ((!sibling->left || RBT_IS_BLACK(sibling->left))
            && (!sibling->right || RBT_IS_BLACK(sibling->right))) {
                RBT_SET_RED(sibling);
                node = parent;
                parent = RBT_PARENT(node);
            } else {
                if (!sibling->left || RBT_IS_BLACK(sibling->left)) {
                    RBT_SET_BLACK(sibling->right);
                    RBT_SET_RED(sibling);
                    RBTLeftRotate(root, sibling);
                    sibling = parent->left;
                }
                RBT_SET_COLOR(sibling, RBT_COLOR(parent));
                RBT_SET_BLACK(parent);
                RBT_SET_BLACK(sibling->left);
                RBTRightRotate(root, parent);
                node = root->root;
                break;
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
    if (y->left != NULL) {
        RBT_SET_PARENT(y->left, node);
    }

    // y->parent = node->parent;
    RBT_SET_PARENT(y, RBT_PARENT(node));

    if (RBT_PARENT(node) == NULL) {
        root->root = y;
    } else {
        if (node->parent->left == node) {
            RBT_PARENT(node)->left = y;
        } else {
            RBT_PARENT(node)->right = y;
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
        RBT_SET_PARENT(RBT_RCHILD(x), node);
    }

    RBT_SET_PARENT(x, RBT_PARENT(node));
    if (node->parent == NULL) {
        root->root = x;
    } else {
        if (node == node->parent->left) {
            node->parent->left = x;
        } else {
            node->parent->right = x;
        }
    }

    x->right = node;
    RBT_SET_PARENT(node, x);
}

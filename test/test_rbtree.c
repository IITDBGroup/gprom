#include "test_main.h"
#include "log/logger.h"
#include "model/set/hashmap.h"
#include "model/set/vector.h"
#include "model/bitset/bitset.h"
#include "model/expression/expression.h"
#include "model/relation/relation.h"
#include "provenance_rewriter/update_ps/rbtree.h"
#include "metadata_lookup/metadata_lookup_postgres.h"

typedef enum DT_TEST
{
    DT_TEST_INT,
    DT_TEST_LONG,
    DT_TEST_FLOAT,
    DT_TEST_STRING
} DT_TEST;

static rc test_init_tree(void);
static rc test_tree_dt(Vector *DTs);

static boolean SHOWDETAIL = FALSE;

static int totS = 10000;
static int dupCnt = 100;


rc
testRBTREE(void)
{
    RUN_TEST(test_init_tree(), ">>>>>> (INIT) TREE TEST");
    Vector *DTs = makeVector(VECTOR_INT, T_Vector);

    // single int
    vecAppendInt(DTs, (int) DT_TEST_INT);
    RUN_TEST(test_tree_dt(DTs), " >>>>>> TYPES: int");

    // single long
    DTs = makeVector(VECTOR_INT, T_Vector);
    vecAppendInt(DTs, (int) DT_TEST_LONG);
    RUN_TEST(test_tree_dt(DTs), " >>>>>> TYPES: long");

    // single String
    DTs = makeVector(VECTOR_INT, T_Vector);
    vecAppendInt(DTs, (int) DT_TEST_STRING);
    RUN_TEST(test_tree_dt(DTs), " >>>>>> TYPES: string");

    // int + long
    DTs = makeVector(VECTOR_INT, T_Vector);
    vecAppendInt(DTs, (int) DT_TEST_INT);
    vecAppendInt(DTs, (int) DT_TEST_LONG);
    RUN_TEST(test_tree_dt(DTs), " >>>>>> TYPES: int + long");

    // int + string
    DTs = makeVector(VECTOR_INT, T_Vector);
    vecAppendInt(DTs, (int) DT_TEST_INT);
    vecAppendInt(DTs, (int) DT_TEST_STRING);
    RUN_TEST(test_tree_dt(DTs), " >>>>>> TYPES: int + string");

    // long + string
    DTs = makeVector(VECTOR_INT, T_Vector);
    vecAppendInt(DTs, (int) DT_TEST_LONG);
    vecAppendInt(DTs, (int) DT_TEST_STRING);
    RUN_TEST(test_tree_dt(DTs), " >>>>>> TYPES: long + string");

    // int + long + string
    DTs = makeVector(VECTOR_INT, T_Vector);
    vecAppendInt(DTs, (int) DT_TEST_STRING);
    vecAppendInt(DTs, (int) DT_TEST_LONG);
    vecAppendInt(DTs, (int) DT_TEST_INT);
    RUN_TEST(test_tree_dt(DTs), " >>>>>> TYPES: string + long + int");

    // single float;
    DTs = makeVector(VECTOR_INT, T_Vector);
    vecAppendInt(DTs, (int) DT_TEST_FLOAT);
    RUN_TEST(test_tree_dt(DTs), " >>>>>> TYPES: float");
    return PASS;
}

static rc
test_init_tree()
{
    // ASSERT_EQUALS_P(RBTREE_NULL, 0, "NULL NODE CHECK");
    RBTRoot *rbtree = makeRBT(RBT_ORDER_BY, TRUE);
    // ASSERT_EQUALS_INT(sizeof(*rbtree), 32, "SIZE");

    ASSERT_TRUE(T_Invalid == 0, "T_INVALID == 0");
    ASSERT_TRUE(T_Invalid == NULL, "T_INVALID == 0");
    ASSERT_FALSE(rbtree == NULL, "TREE NOT NULL");
    ASSERT_TRUE(rbtree->root == RBTREE_NULL, "TREE ROOT IS NULL NODE");
    ASSERT_TRUE(rbtree->size == 0, "EMPTY TREE");
    ASSERT_TRUE(rbtree->metadata->type == T_HashMap, "META DATA DS");
    ASSERT_TRUE(mapSize(rbtree->metadata) == 0, "EMPTY METADATA DS");
    ASSERT_TRUE(rbtree->treeType == RBT_ORDER_BY, "RBT FOR ORDER");
    ASSERT_EQUALS_INT(nodeTag(rbtree), T_RBTRoot, "NODETAG CHECK");
    return PASS;
}

static rc
test_tree_dt(Vector *DTs) {
    RBTRoot *rbtree = makeRBT(RBT_ORDER_BY, TRUE);
    Vector *orderbyacs = makeVector(VECTOR_INT, T_Vector);
    for (int i = 0; i < DTs->length; i++) {
        vecAppendInt(orderbyacs, 1);
    }
    vecAppendInt(orderbyacs, 1);
    addToMap(rbtree->metadata, (Node *) createConstString("ORDER_BY_ASCs"), (Node *) orderbyacs);


    for (int i = 0; i < totS; i++) {
        int k = i / dupCnt;
        Vector *key = makeVector(VECTOR_NODE, T_Vector);
        Vector *val = makeVector(VECTOR_NODE, T_Vector);
        FOREACH_VEC_INT(dt, DTs) {
            switch(dt) {
                case DT_TEST_INT: {
                    vecAppendNode(key, (Node *) createConstInt(k));
                    vecAppendNode(val, (Node *) createConstInt(k));
                    vecAppendNode(val, (Node *) createConstInt(k));
                }
                break;
                case DT_TEST_LONG: {
                    vecAppendNode(key, (Node *) createConstLong((gprom_long_t) k));
                    vecAppendNode(val, (Node *) createConstLong((gprom_long_t) k));
                    vecAppendNode(val, (Node *) createConstLong((gprom_long_t) k));
                }
                break;
                case DT_TEST_FLOAT: {
                    vecAppendNode(key, (Node *) createConstFloat((double) k + (double) 0.3));
                    vecAppendNode(val, (Node *) createConstFloat((double) k + (double) 0.3));
                    vecAppendNode(val, (Node *) createConstFloat((double) k + (double) 0.3));
                }
                break;
                case DT_TEST_STRING: {
                    vecAppendNode(key, (Node *) createConstString(gprom_itoa(k)));
                    vecAppendNode(val, (Node *) createConstString(gprom_itoa(k)));
                    vecAppendNode(val, (Node *) createConstString(gprom_itoa(k)));
                }
                break;
            }
        }

        HashMap * map = NEW_MAP(Constant, Node);
        BitSet *bits = stringToBitset("101010");
        addToMap(map, (Node *) createConstString("PS_a"), (Node *) bits);
        bits = stringToBitset("10101010");
        addToMap(map, (Node *) createConstString("PS_b"), (Node *) bits);
        vecAppendNode(val, (Node *) bits);
        RBTInsert(rbtree, (Node *) key, (Node *) val);
    }

    ASSERT_EQUALS_INT(rbtree->size, totS / dupCnt, "AFTER INSERT, TREE SIZE CHECK");

    Vector *allNodes = RBTInorderTraverse(rbtree);
    ASSERT_EQUALS_INT(allNodes->length, totS / dupCnt, "AFTER INSERT, FETCH ALL NODES CHECK");

    int errorCnt = 0;
    FOREACH_VEC(RBTNode, node, allNodes) {
        HashMap *nodeVal = (HashMap *) node->val;
        FOREACH_HASH_KEY(Node, key, nodeVal) {
            Constant *c = (Constant *) getMap(nodeVal, key);
            if (INT_VALUE(c) != dupCnt) {
                errorCnt++;
            }
            if (SHOWDETAIL)
                ASSERT_EQUALS_INT(INT_VALUE(c), dupCnt, "EACH VALUE COUNT SHOULD BE SAME");
        }
    }

    ASSERT_EQUALS_INT(errorCnt, 0, "CHECK INSIDE TREE FOR KEY AND VAL");


    for (int i = 0; i < totS; i++) {
        int k = i / dupCnt;
        Vector *key = makeVector(VECTOR_NODE, T_Vector);
        Vector *val = makeVector(VECTOR_NODE, T_Vector);
        FOREACH_VEC_INT(dt, DTs) {
            switch(dt) {
                case DT_TEST_INT: {
                    vecAppendNode(key, (Node *) createConstInt(k));
                    vecAppendNode(val, (Node *) createConstInt(k));
                    vecAppendNode(val, (Node *) createConstInt(k));
                }
                break;
                case DT_TEST_LONG: {
                    vecAppendNode(key, (Node *) createConstLong((gprom_long_t) k));
                    vecAppendNode(val, (Node *) createConstLong((gprom_long_t) k));
                    vecAppendNode(val, (Node *) createConstLong((gprom_long_t) k));
                }
                break;
                case DT_TEST_FLOAT: {
                    vecAppendNode(key, (Node *) createConstFloat((double) k + (double) 0.3));
                    vecAppendNode(val, (Node *) createConstFloat((double) k + (double) 0.3));
                    vecAppendNode(val, (Node *) createConstFloat((double) k + (double) 0.3));
                }
                break;
                case DT_TEST_STRING: {
                    vecAppendNode(key, (Node *) createConstString(gprom_itoa(k)));
                    vecAppendNode(val, (Node *) createConstString(gprom_itoa(k)));
                    vecAppendNode(val, (Node *) createConstString(gprom_itoa(k)));
                }
                break;
            }
        }
        HashMap * map = NEW_MAP(Constant, Node);
        BitSet *bits = stringToBitset("101010");
        addToMap(map, (Node *) createConstString("PS_a"), (Node *) bits);
        bits = stringToBitset("10101010");
        addToMap(map, (Node *) createConstString("PS_b"), (Node *) bits);
        vecAppendNode(val, (Node *) bits);
        RBTDelete(rbtree, (Node *) key, (Node *) val);
    }
    DEBUG_NODE_BEATIFY_LOG("RBTREE", rbtree);
    ASSERT_EQUALS_INT(rbtree->size, 0, "AFTER DELETE ALL, CHECK TREE SIZE");
    return PASS;
}


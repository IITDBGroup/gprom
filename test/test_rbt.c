#include "test_main.h"
#include "log/logger.h"
#include "model/set/hashmap.h"
#include "model/set/vector.h"
#include "model/bitset/bitset.h"
#include "model/expression/expression.h"
#include "model/relation/relation.h"
#include "provenance_rewriter/update_ps/rbtree.h"
#include "metadata_lookup/metadata_lookup_postgres.h"


#if HAVE_POSTGRES_BACKEND
#include "libpq-fe.h"
#endif

#define ASSERT_OK(res,mes) ASSERT_TRUE(PQresultStatus(res) == PGRES_COMMAND_OK, mes)
#define EXEC_CHECK(c,query,mes) \
    do { \
        PGresult *res_ = PQexec(c,query); \
        if (PQresultStatus(res_) != PGRES_COMMAND_OK) \
            DEBUG_LOG("error was: %s", PQresultErrorMessage(res_)); \
        ASSERT_OK(res_,mes); \
        PQclear(res_); \
    } while(0)


// >>>> test functions;
static rc testHashCompletedKey(void);
static rc testFetchData(void);
static rc testStoreData(void);
static rc testDataFetchedToMap(void);
static PGconn *getPostgresConn(void);
static rc testTreeINT(void);
static rc testTreeLONG(void);
static rc testTreeFLOAT(void);
static rc testTreeSTRING(void);


static char *testTableName = "TESTCASETABLE";
static int tuplesNum = 4;
static boolean SHOWDETAIL = FALSE;
// static floatType = "float8"
// <<<<



rc
testRBT(void)
{
    RUN_TEST(testHashCompletedKey(), "Hash With Completed Key");
    RUN_TEST(testStoreData(), "Store data in db");
    RUN_TEST(testFetchData(), "Fetch data from db");
    RUN_TEST(testDataFetchedToMap(), "Put fetched data into map");
    RUN_TEST(testTreeINT(), "Test tree INT");
    RUN_TEST(testTreeLONG(), "Test tree LONG");
    RUN_TEST(testTreeSTRING(), "Test tree STRING");
    RUN_TEST(testTreeFLOAT(), "Test tree FLOAT");

    return PASS;
}

static rc
testHashCompletedKey(void)
{
    /*
        a hashmap:
            key: vec(int, long, float, string, hashmap)
            val: count the content of vec;
    */

    HashMap *map = NEW_MAP(Node, Node);
    int valNumber = 3;
	for (int i = 0; i < valNumber; i++) {
		Vector *key = makeVector(VECTOR_NODE, T_Vector);
        // key 1: int;
		vecAppendNode(key, (Node *) createConstInt(1));
        // key 2: long;
        vecAppendNode(key, (Node *) createConstLong(100));
        // key 3: double;
		vecAppendNode(key, (Node *) createConstFloat(1.1));
        // key 4: string;
		vecAppendNode(key, (Node *) createConstString("aaa"));
        // key 5: hashmap;
		BitSet *bitset = newBitSet(10);
		setBit(bitset, 0, TRUE);
		HashMap *submap = NEW_MAP(Constant, Node);
		addToMap(submap, (Node *) createConstString("PS_NAME_A"), (Node *) bitset);
		vecAppendNode(key, (Node *) submap);

		Constant * c = (Constant *) getMap(map, (Node *) key);
		if (c == NULL) {
			addToMap(map, (Node *) key, (Node *) createConstInt(1));
		} else {
			incrConst(c);
		}
	}

    int mapSizes = mapSize(map);

    ASSERT_EQUALS_INT(1, mapSizes, "first sizeof map should be 1");
    FOREACH_HASH_KEY(Node, n, map) {
        Constant *cnt = (Constant *) getMap(map, n);
        ASSERT_EQUALS_INT(valNumber, INT_VALUE(cnt), "this key in map should be the same");
    }
    return PASS;
}

static PGconn *
getPostgresConn(void)
{
    PGconn *conn;
    initMetadataLookupPlugins();
    chooseMetadataLookupPlugin(METADATA_LOOKUP_PLUGIN_POSTGRES);
    initMetadataLookupPlugin();
    conn = getPostgresConnection();
    ASSERT_FALSE(conn == NULL, "CONNECTION IS READY");

    return conn;
}

static rc
testStoreData(void)
{
    PGconn *conn = getPostgresConn();

    // check table existance;
    EXEC_CHECK(conn, CONCAT_STRINGS("DROP TABLE IF EXISTS ", testTableName), "CHECK TABLE EXISTS");
    // create data table;
    EXEC_CHECK(conn, CONCAT_STRINGS("CREATE TABLE ", testTableName, " (a int, b bigint, c float, d float8, e varchar, f varchar, g varchar);"), "CREATE TABLE FOR DATA");


    // create the same tuples multiple times;

    for (int i = 0; i < tuplesNum; i++) {
        int intVal = 1;
        gprom_long_t longVal = (gprom_long_t) 10;
        double doubleVal1 = (double) 1.1;
        double doubleVal2 = (double) 1.2;
        char *str = MALLOC(4);
        memcpy(str, "abc", 3);
        str[3] = '\0';

        char *dummyPSName = MALLOC(6);
        memcpy(dummyPSName, "psname", 5);
        dummyPSName[5] = '\0';

        char *dummyPSVal = MALLOC(5);
        memcpy(dummyPSVal, "1000", 4);
        dummyPSVal[4] = '\0';

        StringInfo tup = makeStringInfo();
        appendStringInfo(tup, "insert into %s values (%s, %s, %s, %s, '%s', '%s', '%s');", testTableName, gprom_itoa(intVal), gprom_ltoa(longVal), gprom_ftoa(doubleVal1), gprom_ftoa(doubleVal2), str, dummyPSName, dummyPSVal);
        EXEC_CHECK(conn, tup->data, "INSERT ONE TUPLE");
    }

    return PASS;
}
static rc
testFetchData(void)
{
    // conn
    // PGconn *conn = getPostgresConn();
    StringInfo dataSql = makeStringInfo();
    appendStringInfo(dataSql, "select * from %s ;", testTableName);
    Relation *rel = postgresExecuteQuery(CONCAT_STRINGS("SELECT * FROM ", testTableName));
    ASSERT_EQUALS_INT(rel->tuples->length, tuplesNum , "SAME LENGTH EQUAL");

    return PASS;
}


static rc
testDataFetchedToMap(void)
{
    Relation *rel = postgresExecuteQuery(CONCAT_STRINGS("SELECT * FROM ", testTableName));

    HashMap *map = NEW_MAP(Node, Node);
    HashMap *mapbc = NEW_MAP(Node, Node);
    FOREACH_VEC(Vector, tup, rel->tuples) {
        ASSERT_EQUALS_INT(tup->length, 7, "TUPLE LENGTH CHECK");
        int intVal = atoi((char *) getVecString(tup, 0));
        int longVal = atol((char *) getVecString(tup, 1));
        double floatVal1 = atof((char *) getVecString(tup, 2));
        double floatVal2 = atof((char *) getVecString(tup, 3));
        char *str = (char *) getVecString(tup, 4);
        char *dummyPSName = (char *) getVecString(tup, 5);
        char *dummyPSVal = (char *) getVecString(tup, 6);
        BitSet *psVal = stringToBitset(dummyPSVal);

        HashMap *submap = NEW_MAP(Constant, Node);
        addToMap(submap, (Node *) createConstString(dummyPSName), (Node *) psVal);
        Vector *key = makeVector(VECTOR_NODE, T_Vector);
        vecAppendNode(key, (Node *) createConstInt(intVal));
        vecAppendNode(key, (Node *) createConstLong(longVal));
        vecAppendNode(key, (Node *) createConstFloat(floatVal1));
        vecAppendNode(key, (Node *) createConstFloat(floatVal2));
        vecAppendNode(key, (Node *) createConstString(str));
        vecAppendNode(key, (Node *) submap);

        Constant * cnt = (Constant *) getMap(map, (Node *) key) ;
        if (cnt == NULL) {
            addToMap(map, (Node *) key, (Node *) createConstInt(1));
        } else {
            incrConst(cnt);
        }

        HashMap *submap2 = NEW_MAP(Constant, Node);
        addToMap(submap2, (Node *) createConstString(dummyPSName), (Node *) psVal);
        Vector *key2 = makeVector(VECTOR_NODE, T_Vector);
        vecAppendNode(key2, (Node *) createConstInt(intVal));
        vecAppendNode(key2, (Node *) createConstLong(longVal));
        vecAppendNode(key2, (Node *) createConstFloat(floatVal1));
        vecAppendNode(key2, (Node *) createConstFloat(floatVal2));
        vecAppendNode(key2, (Node *) createConstString(str));
        vecAppendNode(key2, (Node *) submap2);

        Constant *cntbc = (Constant *) getMap(mapbc, (Node *) key2);
        if (cnt == NULL) {
            addToMap(mapbc, (Node *) key2, (Node *) createConstInt(1));
        } else {
            incrConst(cntbc);
        }



    }
    ASSERT_EQUALS_INT(mapSize(map), 1, "MAP SIZE CHECK");
    ASSERT_EQUALS_INT(mapSize(mapbc), 1, "MAP SICE CHECK BACKUP");

    FOREACH_HASH_KEY(Node, n, map) {
        Constant * cnt = (Constant *) getMap(map, n);
        ASSERT_EQUALS_INT(INT_VALUE(cnt), tuplesNum, "KEY COUNT CHECK");
    }

    FOREACH_HASH_KEY(Node, n, mapbc) {
        Constant * cnt = (Constant *) getMap(map, n);
        ASSERT_EQUALS_INT(INT_VALUE(cnt), tuplesNum, "KEY COUNT CHECK BACKUP");
    }

    // manipulate each data ;
    FOREACH_HASH_KEY(Node, n, map) {
        Vector *v = (Vector *) n;

        /*
         *   MODIFY INT IN VECTOR;
         */
        Constant *intC = (Constant *) getVecNode(v, 0);
        // increase this number;
        incrConst(intC);
        Constant *tupCnt = (Constant *) getMap(mapbc, (Node *) v);
        ASSERT_FALSE(tupCnt, "INT VALUE CHANGED: SHOULD BE FALSE");
        // decrease back to original number;
        INT_VALUE(intC) = INT_VALUE(intC) - 1;
        tupCnt = (Constant *) getMap(mapbc, (Node *) v);
        ASSERT_TRUE(tupCnt, "INT VALUE CHANGED BACK: SHOULD BE TRUE");
        ASSERT_EQUALS_INT(INT_VALUE(tupCnt), tuplesNum, "TWO COUNT SHOULD BE THE SAME");

        /*
         * MODIFY LONG IN VECTOR;
         */
        Constant *longC = (Constant *) getVecNode(v, 1);
        //increase long;
        incrConst(longC);
        tupCnt = (Constant *) getMap(mapbc, (Node *) v);
        ASSERT_FALSE(tupCnt, "LONG VALUE CHANGED: SHOULD BE FALSE");
        LONG_VALUE(longC) = LONG_VALUE(longC) - 1;
        tupCnt = (Constant *) getMap(mapbc, (Node *) v);
        ASSERT_TRUE(tupCnt, "LONG VALUE CHANGED BACK: SHOULD BE TRUE");
        ASSERT_EQUALS_INT(INT_VALUE(tupCnt), tuplesNum, "TWO COUNT SHOULD BE THE SAME");

        /*
         *  MODIFY STRING IN VECTOR;
         *  pos: 4, ori: abc,
         */
        Constant *strC = (Constant *) getVecNode(v, 4);
        strC->value = MALLOC(4);
        memcpy((char *) strC->value, "cba", 3);
        ((char *) strC->value)[3] = '\0';

        tupCnt = (Constant *) getMap(mapbc, (Node *) v);
        ASSERT_FALSE(tupCnt, "STRING VALUE CHANGED: SHOULD BE FALSE");
        memcpy((char *) strC->value, "abc", 3);
        tupCnt = (Constant *) getMap(mapbc, (Node *) v);
        ASSERT_TRUE(tupCnt, "STRING VALUE CHANGED BACK: SHOULD BE TRUE");



        /*
         *  MODIFY FLOAT IN VECTOR;
         */
        Vector *floatMod = (Vector *) copyObject(v);
        Vector *floatMod2 = (Vector *) copyObject(v);
        Constant *newC = (Constant *) getMap(mapbc, (Node *) floatMod);
        ASSERT_FALSE(floatMod == v, "TWO VECTOR NOT THE SAME: TRUE");
        ASSERT_TRUE(newC, "COPIED VECTOR: SHOULD BE IN MAP: TRUE");
        newC = (Constant *) getMap(mapbc, (Node *) floatMod2);
        ASSERT_FALSE(floatMod == v, "TWO VECTOR NOT THE SAME: TRUE");
        ASSERT_TRUE(newC, "COPIED VECTOR: SHOULD BE IN MAP: TRUE");

        Constant *floatC;
        // multiple by int;
        if (FALSE) {
            floatC = (Constant *) getVecNode(v, 2);
            FLOAT_VALUE(floatC) = FLOAT_VALUE(floatC) * 2;
            tupCnt = (Constant *) getMap(mapbc, (Node *) v);
            ASSERT_FALSE(tupCnt, "FLOAT VALUE CHANGED by int: SHOULD BE FALSE");
            FLOAT_VALUE(floatC) = FLOAT_VALUE(floatC) / 2;
            ASSERT_TRUE(tupCnt, "FLOAT VALUE CHANGED by int BACK: SHOULD BE TRUE");
            ASSERT_EQUALS_INT(INT_VALUE(tupCnt), tuplesNum, "TWO COUNT SHOULD BE THE SAME");
        }

        // multiple by long;
        if (FALSE) {
            floatC = (Constant *) getVecNode(floatMod, 2);
            FLOAT_VALUE(floatC) = FLOAT_VALUE(floatC) * (gprom_long_t) 2;
            tupCnt = (Constant *) getMap(mapbc, (Node *) floatMod);
            ASSERT_FALSE(tupCnt, "FLOAT VALUE CHANGED by long: SHOULD BE FALSE");
            FLOAT_VALUE(floatC) = FLOAT_VALUE(floatC) / (gprom_long_t) 2;
            ASSERT_TRUE(tupCnt, "FLOAT VALUE CHANGED by long BACK: SHOULD BE TRUE");
        }

        if (FALSE) {
            floatC = (Constant *) getVecNode(floatMod2, 2);
            FLOAT_VALUE(floatC) = FLOAT_VALUE(floatC) * (double) 2;
            tupCnt = (Constant *) getMap(mapbc, (Node *) floatMod);
            ASSERT_FALSE(tupCnt, "FLOAT VALUE CHANGED by float: SHOULD BE FALSE");
            FLOAT_VALUE(floatC) = FLOAT_VALUE(floatC) / (double) 2;
            ASSERT_TRUE(tupCnt, "FLOAT VALUE CHANGED by float BACK: SHOULD BE TRUE");
        }
    }
    return PASS;
}

static rc
testTreeINT()
{
    RBTRoot *root = makeRBT(RBT_ORDER_BY, TRUE);
    Vector *orderbyacs = makeVector(VECTOR_INT, T_Vector);
    vecAppendInt(orderbyacs, 1);
    addToMap(root->metadata, (Node *) createConstString("ORDER_BY_ASCs"), (Node *) orderbyacs);
    for (int i = 0; i < 10000; i++) {
        int k = i / 10;
        Vector *key = makeVector(VECTOR_NODE, T_Vector);
        vecAppendNode(key, (Node *) createConstInt(k));
        Vector *val = makeVector(VECTOR_NODE, T_Vector);
        vecAppendNode(val, (Node *) createConstInt(k));
        vecAppendNode(val, (Node *) createConstInt(k));
        RBTInsert(root, (Node *) key, (Node *) val);
    }

    ASSERT_EQUALS_INT(1000, root->size, "SIZES EQUAL");

    Vector *allNodes = RBTInorderTraverse(root);
    ASSERT_EQUALS_INT(allNodes->length, 1000, "IN ORDER TRAVERSE CHECK");

    FOREACH_VEC(RBTNode, node, allNodes) {
        HashMap *nodeVal = (HashMap *) node->val;
        FOREACH_HASH_KEY(Node, key, nodeVal) {
            Constant *c = (Constant *) getMap(nodeVal, key);
            if (SHOWDETAIL)
                ASSERT_EQUALS_INT(INT_VALUE(c), 10, "EACH VALUE COUNT SHOULD BE SAME");
        }
    }

    for (int i = 0; i < 10000; i++) {
        int k = i / 10;
        Vector *key = makeVector(VECTOR_NODE, T_Vector);
        vecAppendNode(key, (Node *) createConstInt(k));
        Vector *val = makeVector(VECTOR_NODE, T_Vector);
        vecAppendNode(val, (Node *) createConstInt(k));
        vecAppendNode(val, (Node *) createConstInt(k));
        RBTDelete(root, (Node *) key, (Node *) val);
    }
    ASSERT_EQUALS_INT(0, root->size, "INT AFTER DELETION CHECK");
    return PASS;
}

static rc
testTreeLONG(void)
{
    RBTRoot *root = makeRBT(RBT_ORDER_BY, TRUE);
    Vector *orderbyacs = makeVector(VECTOR_INT, T_Vector);
    vecAppendInt(orderbyacs, 1);
    addToMap(root->metadata, (Node *) createConstString("ORDER_BY_ASCs"), (Node *) orderbyacs);
    for (int i = 0; i < 10000; i++) {
        gprom_long_t k = (gprom_long_t) i / 10;
        Vector *key = makeVector(VECTOR_NODE, T_Vector);
        vecAppendNode(key, (Node *) createConstLong(k));
        Vector *val = makeVector(VECTOR_NODE, T_Vector);
        vecAppendNode(val, (Node *) createConstLong(k));
        vecAppendNode(val, (Node *) createConstLong(k));
        RBTInsert(root, (Node *) key, (Node *) val);
    }

    ASSERT_EQUALS_INT(1000, root->size, "SIZES EQUAL");

    Vector *allNodes = RBTInorderTraverse(root);
    ASSERT_EQUALS_INT(allNodes->length, 1000, "IN ORDER TRAVERSE CHECK");

    FOREACH_VEC(RBTNode, node, allNodes) {
        HashMap *nodeVal = (HashMap *) node->val;
        FOREACH_HASH_KEY(Node, key, nodeVal) {
            Constant *c = (Constant *) getMap(nodeVal, key);
            if (SHOWDETAIL)
                ASSERT_EQUALS_INT(INT_VALUE(c), 10, "EACH VALUE COUNT SHOULD BE SAME");
        }
    }

    for (int i = 0; i < 10000; i++) {
        gprom_long_t k = (gprom_long_t) i / 10;
        Vector *key = makeVector(VECTOR_NODE, T_Vector);
        vecAppendNode(key, (Node *) createConstLong(k));
        Vector *val = makeVector(VECTOR_NODE, T_Vector);
        vecAppendNode(val, (Node *) createConstLong(k));
        vecAppendNode(val, (Node *) createConstLong(k));
        RBTDelete(root, (Node *) key, (Node *) val);
    }

    ASSERT_EQUALS_INT(0, root->size, "SIZES EQUAL");
    return PASS;
}

static rc
testTreeFLOAT(void)
{
    RBTRoot *root = makeRBT(RBT_ORDER_BY, TRUE);
    Vector *orderbyacs = makeVector(VECTOR_INT, T_Vector);
    vecAppendInt(orderbyacs, 1);
    addToMap(root->metadata, (Node *) createConstString("ORDER_BY_ASCs"), (Node *) orderbyacs);
    for (int i = 0; i < 10000; i++) {
        int k = i / 10;
        Vector *key = makeVector(VECTOR_NODE, T_Vector);
        vecAppendNode(key, (Node *) createConstFloat((double) k));
        Vector *val = makeVector(VECTOR_NODE, T_Vector);
        vecAppendNode(val, (Node *) createConstFloat((double) k));
        vecAppendNode(val, (Node *) createConstFloat((double) k));
        RBTInsert(root, (Node *) key, (Node *) val);
    }

    ASSERT_EQUALS_INT(1000, root->size, "SIZES EQUAL");

    Vector *allNodes = RBTInorderTraverse(root);
    ASSERT_EQUALS_INT(allNodes->length, 1000, "IN ORDER TRAVERSE CHECK");

    FOREACH_VEC(RBTNode, node, allNodes) {
        HashMap *nodeVal = (HashMap *) node->val;
        FOREACH_HASH_KEY(Node, key, nodeVal) {
            Constant *c = (Constant *) getMap(nodeVal, key);
            if (SHOWDETAIL)
                ASSERT_EQUALS_INT(INT_VALUE(c), 10, "EACH VALUE COUNT SHOULD BE SAME");
        }
    }

    for (int i = 0; i < 10000; i++) {
        int k = i / 10;
        Vector *key = makeVector(VECTOR_NODE, T_Vector);
        vecAppendNode(key, (Node *) createConstFloat((double) k));
        Vector *val = makeVector(VECTOR_NODE, T_Vector);
        vecAppendNode(val, (Node *) createConstFloat((double) k));
        vecAppendNode(val, (Node *) createConstFloat((double) k));
        RBTDelete(root, (Node *) key, (Node *) val);
    }
    ASSERT_EQUALS_INT(0, root->size, "SIZES EQUAL");
    return PASS;
}

static rc
testTreeSTRING(void)
{
    RBTRoot *root = makeRBT(RBT_ORDER_BY, TRUE);
    Vector *orderbyacs = makeVector(VECTOR_INT, T_Vector);
    vecAppendInt(orderbyacs, 1);
    addToMap(root->metadata, (Node *) createConstString("ORDER_BY_ASCs"), (Node *) orderbyacs);
    for (int i = 0; i < 10000; i++) {
        int kk = i / 10;
        char *k = gprom_itoa(kk);
        Vector *key = makeVector(VECTOR_NODE, T_Vector);
        vecAppendNode(key, (Node *) createConstString(k));
        Vector *val = makeVector(VECTOR_NODE, T_Vector);
        vecAppendNode(val, (Node *) createConstString(k));
        vecAppendNode(val, (Node *) createConstString(k));
        RBTInsert(root, (Node *) key, (Node *) val);
    }

    ASSERT_EQUALS_INT(1000, root->size, "SIZES EQUAL");

    Vector *allNodes = RBTInorderTraverse(root);
    ASSERT_EQUALS_INT(allNodes->length, 1000, "IN ORDER TRAVERSE CHECK");

    FOREACH_VEC(RBTNode, node, allNodes) {
        HashMap *nodeVal = (HashMap *) node->val;
        FOREACH_HASH_KEY(Node, key, nodeVal) {
            Constant *c = (Constant *) getMap(nodeVal, key);
            if (SHOWDETAIL)
                ASSERT_EQUALS_INT(INT_VALUE(c), 10, "EACH VALUE COUNT SHOULD BE SAME");
        }
    }

    // for (int i = 0; i < 10000; i++) {
    //     int kk = i / 10;
    //     char *k = gprom_itoa(kk);
    //     Vector *key = makeVector(VECTOR_NODE, T_Vector);
    //     vecAppendNode(key, (Node *) createConstString(k));
    //     Vector *val = makeVector(VECTOR_NODE, T_Vector);
    //     vecAppendNode(val, (Node *) createConstString(k));
    //     vecAppendNode(val, (Node *) createConstString(k));
    //     RBTDelete(root, (Node *) key, (Node *) val);
    // }
    // ASSERT_EQUALS_INT(0, root->size, "SIZES EQUAL");
    return PASS;
}

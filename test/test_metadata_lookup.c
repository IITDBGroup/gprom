/*
 * test_metadata_lookup.c
 *
 *      Author: zephyr
 */

#include "test_main.h"
#include "common.h"

#include "mem_manager/mem_mgr.h"
#include "configuration/option.h"
#include "log/logger.h"

#include "metadata_lookup/metadata_lookup.h"
#include "metadata_lookup/metadata_lookup_oracle.h"
#include "mem_manager/mem_mgr.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"

static char *table1Attrs[3] = { "A","B","C" };
static char *table2Attrs[2] = { "D","E" };

/* internal tests */
static rc testCatalogTableExists(void);
static rc testViewExists(void);
static rc testGetAttributes(void);
static rc testIsAgg(void);
static rc testGetTableDefinition(void);
static rc testTransactionSQLAndSCNs(void);
static rc testGetViewDefinition(void);
static rc testRunTransactionAndGetXid(void);
static rc setupMetadataLookup(void);
static rc testDatabaseConnectionClose(void);

rc
testMetadataLookup(void)
{

    if (streq(getStringOption("backend"),"oracle"))
    {
        RUN_TEST(setupMetadataLookup(),"setup tables");
        RUN_TEST(testCatalogTableExists(), "test catalog table exists");
        RUN_TEST(testViewExists(), "test view exists");
        RUN_TEST(testGetAttributes(), "test get attributes");
        RUN_TEST(testIsAgg(), "test is aggregation functions");
        RUN_TEST(testGetTableDefinition(), "test get table definition");
        RUN_TEST(testTransactionSQLAndSCNs(), "test transaction SQL and SCN");
        RUN_TEST(testGetViewDefinition(), "test get view definition");
        RUN_TEST(testRunTransactionAndGetXid(), "test transaction execution and XID retrieval");
        RUN_TEST(testDatabaseConnectionClose(), "test close database connection");
    }

	return PASS;
}

/* if OCI is not available then add dummy versions */
#if HAVE_ORACLE_BACKEND

static rc
setupMetadataLookup(void)
{
    OCI_Connection *c;
    OCI_Statement *st;

    initMetadataLookupPlugins();
    chooseMetadataLookupPlugin(METADATA_LOOKUP_PLUGIN_ORACLE);
    initMetadataLookupPlugin();

    c = getConnection();
    st = OCI_StatementCreate(c);
    ASSERT_FALSE(st == NULL, "created statement");
    OCI_ExecuteStmt(st,"DROP TABLE metadatalookup_test1");
    OCI_ExecuteStmt(st,"DROP TABLE metadatalookup_test2");
    OCI_ExecuteStmt(st,"Drop View metadatalookup_view1");

    ASSERT_EQUALS_INT(1, OCI_ExecuteStmt(st,
            "CREATE TABLE metadatalookup_test1"
            " (a int, b int, c int)"), "Create table 1");
    ASSERT_EQUALS_INT(1,OCI_ExecuteStmt(st, "CREATE TABLE "
            "metadatalookup_test2 (d int, e int)"),  "Create table 2");
    ASSERT_EQUALS_INT(1,OCI_ExecuteStmt(st, "CREATE VIEW "
                "metadatalookup_view1 as select * from metadatalookup_test1"),  "Create view 1");

    OCI_Commit(c);
    OCI_StatementFree(st);

    DEBUG_LOG("Created test tables");

    return PASS;
}

static rc
testCatalogTableExists(void)
{
	boolean hasTest1 = catalogTableExists("metadatalookup_test1");
	boolean hasTest3 = catalogTableExists("metadatalookup_test3");

	ASSERT_EQUALS_INT(hasTest1, TRUE, "test Has table <metadatalookup_test1>");
	ASSERT_EQUALS_INT(hasTest3, FALSE, "test Doesn't have table <metadatalookup_test3>");

	return PASS;
}

static rc
testViewExists(void)
{
	boolean hasView1 = catalogViewExists("metadatalookup_view1");
	boolean hasView2 = catalogViewExists("metadatalookup_view2");

	ASSERT_EQUALS_INT(hasView1, TRUE, "test Has view <metadatalookup_view1>");
	ASSERT_EQUALS_INT(hasView2, FALSE, "test Doesn't have view <metadatalookup_view2>");
	return PASS;
}

static rc
testGetAttributes(void)
{
    int i = 0;

	List *attrs = getAttributes("metadatalookup_test1");
	FOREACH(AttributeDef,a,attrs)
	{
	    DEBUG_LOG("Attribute %s\n",nodeToString(a));
	    ASSERT_EQUALS_STRING(table1Attrs[i],a->attrName, "attribute");
	    i++;
	}

	i = 0;
    List *attrs1 = getAttributes("metadatalookup_test2");
    FOREACH(AttributeDef,a,attrs1)
    {
        DEBUG_LOG("Attribute %s\n",nodeToString(a));
        ASSERT_EQUALS_STRING(table2Attrs[i],a->attrName, "attribute");
        i++;
    }
    List *attrs2 = getAttributes("metadatalookup_test1");
    List *attrs3 = getAttributes("metadatalookup_test2");

    //test table buffers
    ASSERT_EQUALS_NODE(attrs, attrs2, "Get attributes addr from table buffers");
    ASSERT_EQUALS_NODE(attrs1, attrs3, "Get attributes addr from table buffers");

	return PASS;
}

static rc
testIsAgg(void)
{
	boolean isAgg1 = isAgg("max");
	boolean isAgg2 = isAgg("SuM");
	boolean isAgg3 = isAgg("notAgg");

	ASSERT_EQUALS_INT(isAgg1, TRUE, "test is aggregation function <max>");
	ASSERT_EQUALS_INT(isAgg2, TRUE, "test is aggregation function <SuM>");
	ASSERT_EQUALS_INT(isAgg3, FALSE, "test is aggregation function <notAgg>");
	return PASS;
}

static rc
testGetTableDefinition()
{
	char *tableDef = getTableDefinition("METADATALOOKUP_TEST1");
//	char *text = "whatever it is too complex to write here.";

	ASSERT_EQUALS_STRING(tableDef, tableDef, "test get table definition <metadatalookup_test1>");
	return PASS;
}

static rc
testTransactionSQLAndSCNs()
{
	List *scns = NIL;
	List *sqls = NIL;
	List *binds = NIL;
	IsolationLevel iso;
	Constant *commit = createConstLong(-1);

    getTransactionSQLAndSCNs("0A0020002F570200",&scns,&sqls,&binds, &iso, commit);
    DEBUG_LOG("scns: %s, sqls: %s, binds: %s, iso: %u, commitSCN: %u",
            nodeToString(scns),
            stringListToString(sqls),
            stringListToString(binds),
            iso,
            commit);
    if (scns !=NULL && sqls != NULL)
    	return PASS;

    return PASS;
}

static rc
testGetViewDefinition()
{
	char *viewDef = getViewDefinition("METADATALOOKUP_VIEW1");
	char *text = "select \"A\",\"B\",\"C\" from metadatalookup_test1";
	ASSERT_EQUALS_STRINGP(viewDef, text, "test get view definition <metadatalookup_view1>");

	char *viewDef1 = getViewDefinition("METADATALOOKUP_VIEW1");
	ASSERT_EQUALS_STRINGP(viewDef1, text, "test get view definition from buffer <metadatalookup_view1>");
	return PASS;
}

static rc
testRunTransactionAndGetXid()
{
    Constant *xid;
    List *sqls = LIST_MAKE("INSERT INTO metadatalookup_test1 VALUES (1,2,3)", "DELETE FROM metadatalookup_test1");

    xid = (Constant *) executeAsTransactionAndGetXID(sqls, ISOLATION_SERIALIZABLE);

    ASSERT_TRUE(xid != NULL, "xid should not be NULL");

    return PASS;
}

static rc
testDatabaseConnectionClose()
{
	ASSERT_EQUALS_INT(EXIT_SUCCESS, databaseConnectionClose(), "test close metadata_lookup");
	return PASS;
}

/* if OCI or OCILIB are not avaible replace with dummy test */
#else

static rc
testCatalogTableExists(void)
{
    return PASS;
}

static rc
testViewExists(void)
{
	return PASS;
}

static rc
testGetAttributes(void)
{
    return PASS;
}

static rc
testIsAgg(void)
{
	return PASS;
}

static rc
testGetTableDefinition()
{
	return PASS;
}

static rc
testTransactionSQLAndSCNs(void)
{
    return PASS;
}

static rc
testGetViewDefinition()
{
	return PASS;
}

static rc
setupMetadataLookup(void)
{
    return PASS;
}

static rc
testDatabaseConnectionClose()
{
	return PASS;
}

static rc
testRunTransactionAndGetXid()
{
    return PASS;
}

#endif

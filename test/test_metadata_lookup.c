/*
 * test_metadata_lookup.c
 *
 *      Author: zephyr
 */

#include <string.h>

#include "test_main.h"
#include "mem_manager/mem_mgr.h"
#include "common.h"
#include "configuration/option.h"
#include "metadata_lookup/metadata_lookup.h"
#include "mem_manager/mem_mgr.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"
#include "log/logger.h"

static char *table1Attrs[3] = { "a","b","c" };
static char *table2Attrs[2] = { "d","e" };

/* internal tests */
static rc testCatalogTableExists(void);
static rc testGetAttributes(void);
static rc setupMetadataLookup(void);

rc
testMetadataLookup(void)
{
    RUN_TEST(setupMetadataLookup(),"setup tables");
	RUN_TEST(testCatalogTableExists(), "test catalog table exists");
	RUN_TEST(testGetAttributes(), "test get attributes");

	return PASS;
}

static rc
setupMetadataLookup(void)
{
    OCI_Connection *c;
    OCI_Statement *st;

    c = getConnection();
    st = OCI_StatementCreate(c);
    ASSERT_FALSE(st == NULL, "created statement");
    OCI_ExecuteStmt(st,"DROP TABLE metadatalookup_test1");
    OCI_ExecuteStmt(st,"DROP TABLE metadatalookup_test2");
    ASSERT_EQUALS_INT(1, OCI_ExecuteStmt(st,
            "CREATE TABLE metadatalookup_test1"
            " (a int, b int, c int)"), "Create table 1");
    ASSERT_EQUALS_INT(1,OCI_ExecuteStmt(st, "CREATE TABLE "
            "metadatalookup_test2 (d int, e int)"),  "Create table 2");
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
testGetAttributes(void)
{
    int i = 0;
	List* attrs;

	attrs = getAttributes("metadatalookup_test1");
	FOREACH(AttributeReference,a,attrs)
	{
	    DEBUG_LOG("Attribute %s\n",nodeToString(a));
	    ASSERT_EQUALS_STRING(table1Attrs[i],a->name, "attribute");
	    i++;
	}

	i = 0;
    attrs = getAttributes("metadatalookup_test2");
    FOREACH(AttributeReference,a,attrs)
    {
        DEBUG_LOG("Attribute %s\n",nodeToString(a));
        ASSERT_EQUALS_STRING(table2Attrs[i],a->name, "attribute");
        i++;
    }

	return PASS;
}

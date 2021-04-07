/*-----------------------------------------------------------------------------
 *
 * test_parameter.c
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "analysis_and_translate/analyzer.h"
#include "analysis_and_translate/translator.h"
#include "metadata_lookup/metadata_lookup_sqlite.h"
#include "test_main.h"
#include "configuration/option.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"
#include "analysis_and_translate/parameter.h"
#include "parser/parser.h"
#include "analysis_and_translate/analyze_oracle.h"
#include "metadata_lookup/metadata_lookup.h"
#include "metadata_lookup/metadata_lookup_oracle.h"
#include "metadata_lookup/metadata_lookup_postgres.h"

#ifdef HAVE_POSTGRES_BACKEND
#include "libpq-fe.h"
#endif

static rc setupParameterDB (void);
static rc shutdownPlugin(void);
static rc testParseBinds (void);
static rc testSetParameterValues (void);
static rc testTemplatizeQuery (void);

rc
testParameter(void)
{
    RUN_TEST(setupParameterDB(), "setup tables for parameter tests");
    RUN_TEST(testParseBinds(), "test parse binds");
    RUN_TEST(testSetParameterValues(), "test setting values for parameters");
	RUN_TEST(testTemplatizeQuery(), "test templetizing queries");

    return PASS;
}

static rc
setupParameterDB (void)
{
#if HAVE_SQLITE_BACKEND
	if (strpleq(getStringOption("backend"),"sqlite"))
	{
		initMetadataLookupPlugins();
		chooseMetadataLookupPlugin(METADATA_LOOKUP_PLUGIN_SQLITE);
		initMetadataLookupPlugin();

		sqliteExecuteQueryIgnoreResults("DROP TABLE IF EXISTS param_test1");
		sqliteExecuteQueryIgnoreResults("CREATE TABLE param_test1 (a int, b int)");

		DEBUG_LOG("Created test tables");
	}
#endif
#if HAVE_ORACLE_BACKEND
    if (strpleq(getStringOption("backend"),"oracle"))
    {
        OCI_Connection *c;
        OCI_Statement *st;

        initMetadataLookupPlugins();
        chooseMetadataLookupPlugin(METADATA_LOOKUP_PLUGIN_ORACLE);
        initMetadataLookupPlugin();

        c = getConnection();
        st = OCI_StatementCreate(c);
        ASSERT_FALSE(st == NULL, "created statement");
        OCI_ExecuteStmt(st,"DROP TABLE param_test1");

        ASSERT_EQUALS_INT(1, OCI_ExecuteStmt(st,
                "CREATE TABLE param_test1"
                " (a int, b int)"), "Create param_test1");

        OCI_Commit(c);
        OCI_StatementFree(st);

        DEBUG_LOG("Created test tables");
    }
#endif
#if HAVE_POSTGRES_BACKEND

#define ASSERT_OK(res,mes) ASSERT_TRUE(PQresultStatus(res) == PGRES_COMMAND_OK, mes)
#define EXEC_CHECK(c,query,mes) \
    do { \
        PGresult *res_ = PQexec(c,query); \
        if (PQresultStatus(res_) != PGRES_COMMAND_OK) \
            DEBUG_LOG("error was: %s", PQresultErrorMessage(res_)); \
        ASSERT_OK(res_,mes); \
        PQclear(res_); \
    } while(0)

    if (strpleq(getStringOption("backend"),"postgres"))
    {
        PGconn *c;

        initMetadataLookupPlugins();
        chooseMetadataLookupPlugin(METADATA_LOOKUP_PLUGIN_POSTGRES);
        initMetadataLookupPlugin();

        c = getPostgresConnection();
        ASSERT_FALSE(c == NULL, "have working connection");
        EXEC_CHECK(c,"DROP TABLE IF EXISTS param_test1 CASCADE;", "drop param_test1");

        EXEC_CHECK(c, "CREATE TABLE param_test1"
                " (a int, b int);", "Create param_test1");

        DEBUG_LOG("Created test tables");
   }
#endif

    return PASS;
}

static rc
testParseBinds (void)
{
    char *binds = " #1(3):123 #2(5):abcde";
    List *consts = NIL;
    List *expected = LIST_MAKE(createConstLong(123), createConstString("abcde"));

    consts = oracleBindToConsts(binds);
    ASSERT_EQUALS_NODE(expected, consts, "parsed parameters should be [123,abcde]");

    return PASS;
}

static rc
testSetParameterValues (void)
{
#if HAVE_SQLITE_BACKEND
	if (strpleq(getStringOption("backend"),"sqlite"))
	{
        char *expSQL = "SELECT a FROM param_test1 WHERE a = '5';";
        char *inSQL = "SELECT a FROM param_test1 WHERE a = :param;";
        Node *expParse = parseFromString(expSQL);
        Node *inParse = parseFromString(inSQL);
        Node *val = (Node *) createConstString("5");
        SQLParameter *p = (SQLParameter *) getNthOfListP(findParameters(inParse), 0);

        analyzeQueryBlockStmt(expParse, NIL);
        analyzeQueryBlockStmt(inParse, NIL);
        p->position = 1;

        inParse = setParameterValues(inParse, singleton(val));

        ASSERT_EQUALS_NODE(expParse, inParse, "setting parameter is '5'");

        shutdownMetadataLookupPlugin();
	}
#endif

// only if a backend DB library is available
#if HAVE_ORACLE_BACKEND
    if (strpleq(getStringOption("backend"),"oracle"))
    {
        char *expSQL = "SELECT a FROM param_test1 WHERE a = '5';";
        char *inSQL = "SELECT a FROM param_test1 WHERE a = :param;";
        Node *expParse = parseFromString(expSQL);
        Node *inParse = parseFromString(inSQL);
        Node *val = (Node *) createConstString("5");
        SQLParameter *p = (SQLParameter *) getNthOfListP(findParameters(inParse), 0);

        analyzeQueryBlockStmt(expParse, NIL);
        analyzeQueryBlockStmt(inParse, NIL);
        p->position = 1;

        inParse = setParameterValues(inParse, singleton(val));

        ASSERT_EQUALS_NODE(expParse, inParse, "setting parameter is '5'");

        shutdownMetadataLookupPlugin();
    }
#endif

#ifdef HAVE_POSTGRES_BACKEND
    if (strpleq(getStringOption("backend"),"postgres"))
    {
        char *expSQL = "SELECT \"a\" FROM \"param_test1\" WHERE \"a\" = '5';";
        char *inSQL = "SELECT \"a\" FROM \"param_test1\" WHERE \"a\" = :param;";
        Node *expParse = parseFromString(expSQL);
        Node *inParse = parseFromString(inSQL);
        Node *val = (Node *) createConstString("5");
        SQLParameter *p = (SQLParameter *) getNthOfListP(findParameters(inParse), 0);

        analyzeQueryBlockStmt(expParse, NIL);
        analyzeQueryBlockStmt(inParse, NIL);
        p->position = 1;

        inParse = setParameterValues(inParse, singleton(val));

        ASSERT_EQUALS_NODE(expParse, inParse, "setting parameter is '5'");

        shutdownMetadataLookupPlugin();
    }
#endif

    return PASS;
}

static rc
testTemplatizeQuery (void)
{
// only if a backend DB library is available
#if HAVE_ORACLE_BACKEND
    if (strpleq(getStringOption("backend"),"oracle"))
    {
        char *inSQL = "SELECT a FROM param_test1 WHERE a = '5';";
        char *expSQL = "SELECT a FROM param_test1 WHERE a = :param;";
        Node *expQO = translateParse(analyzeParseModel(parseFromString(expSQL)));
        Node *inQO = translateParse(analyzeParseModel(parseFromString(inSQL)));



        SQLParameter *p = (SQLParameter *) getNthOfListP(findParameters(inParse), 0);


        analyzeQueryBlockStmt(expParse, NIL);
        analyzeQueryBlockStmt(inParse, NIL);
        p->position = 1;

        inParse = setParameterValues(inParse, singleton(val));

        ASSERT_EQUALS_NODE(expParse, inParse, "setting parameter is '5'");

        shutdownMetadataLookupPlugin();
    }
#endif

#ifdef HAVE_POSTGRES_BACKEND
    if (strpleq(getStringOption("backend"),"postgres"))
    {
        char *expSQL = "SELECT \"a\" FROM \"param_test1\" WHERE \"a\" = '5';";
        char *inSQL = "SELECT \"a\" FROM \"param_test1\" WHERE \"a\" = :param;";
        Node *expParse = parseFromString(expSQL);
        Node *inParse = parseFromString(inSQL);
        Node *val = (Node *) createConstString("5");
        SQLParameter *p = (SQLParameter *) getNthOfListP(findParameters(inParse), 0);

        analyzeQueryBlockStmt(expParse, NIL);
        analyzeQueryBlockStmt(inParse, NIL);
        p->position = 1;

        inParse = setParameterValues(inParse, singleton(val));

        ASSERT_EQUALS_NODE(expParse, inParse, "setting parameter is '5'");

        shutdownMetadataLookupPlugin();
    }
#endif

	return PASS;
}

static rc
shutdownPlugin(void)
{
	shutdownMetadataLookupPlugin();
}

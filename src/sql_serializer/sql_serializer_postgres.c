/*-----------------------------------------------------------------------------
 *
 * sql_serializer_postgres.c
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "instrumentation/timing_instrumentation.h"
#include "mem_manager/mem_mgr.h"
#include "configuration/option.h"
#include "log/logger.h"

#include "model/expression/expression.h"
#include "sql_serializer/sql_serializer_common.h"
#include "sql_serializer/sql_serializer_postgres.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/set/hashmap.h"

#include "analysis_and_translate/translator.h"

#include "utility/string_utils.h"

/* vars */
static SerializeClausesAPI *api = NULL;

/* methods */
static void createAPI(void);
static boolean addNullCasts(Node *n, Set *visited, void **parentPointer);
static char*serializeDMLandDDLPostgres(QueryOperator *q);
static void serializeJoinOperator(StringInfo from, QueryOperator* fromRoot, JoinOperator* j,
        int* curFromItem, int* attrOffset, FromAttrsContext *fac, SerializeClausesAPI *api);
static List *serializeProjectionAndAggregation (QueryBlockMatch *m, StringInfo select,
												StringInfo having, StringInfo groupBy, List *fromAttrs, boolean materialize, SerializeClausesAPI *api);
static void serializeConstRel(StringInfo from, ConstRelOperator* t, FromAttrsContext *fac,
        int* curFromItem,  SerializeClausesAPI *api);
static void serializeTableAccess(StringInfo from, TableAccessOperator* t, int* curFromItem,
		FromAttrsContext *fac, int* attrOffset, SerializeClausesAPI *api);
static List *serializeSetOperator(QueryOperator *q, StringInfo str, FromAttrsContext *fac, SerializeClausesAPI *api);

/* serialize functions for create table, delete, insert and update*/
static void
serializeCreateTable(StringInfo str, CreateTable *c);
static void
serializeDelete(StringInfo str, Delete *d);
static void
serializeInsert(StringInfo str, Insert *i);
static void
serializeUpdate(StringInfo str, Update *u);

char*
serializeOperatorModelPostgres(Node *q)
{
	StringInfo str = makeStringInfo();
	char *result = NULL;

	// create the api
	createAPI();

	// quote idents for postgres

	if (!getBoolOption(OPTION_PS_POST_TO_ORACLE))
		genQuoteAttributeNames(q);

	DEBUG_OP_LOG("after attr quoting", q);

	// add casts to null constants to make postgres aware of their types

	visitWithPointers(q, addNullCasts, (void**) &q, PSET());

	// serialize query
	if (IS_OP(q)) {
		appendStringInfoString(str, serializeQueryPostgres((QueryOperator*) q));
		appendStringInfoChar(str, ';');
	} else if (isA(q, List))
		FOREACH(QueryOperator,o,(List *) q)
		{
			appendStringInfoString(str, serializeQueryPostgres(o));
			appendStringInfoString(str, ";\n\n");
		}
	else
		FATAL_LOG("cannot serialize non-operator to SQL: %s", nodeToString(q));

	result = str->data;
	FREE(str);
	return result;
}

static boolean
addNullCasts(Node *n, Set *visited, void **parentPointer)
{
	if (n == NULL)
		return TRUE;

	if (hasSetElem(visited, n))
		return TRUE;

	addToSet(visited, n);

	if (isA(n, Constant)) {
		Constant *c = (Constant*) n;
		CastExpr *cast;

		if (c->isNull) {
			cast = createCastExpr((Node*) c, c->constType);
			*parentPointer = cast;
		}

		return TRUE;
	}

	return visitWithPointers(n, addNullCasts, parentPointer, visited);
}

char*
serializeQueryPostgres(QueryOperator *q)
{
	StringInfo str;
	StringInfo viewDef;
	char *result;
	createAPI();

	NEW_AND_ACQUIRE_MEMCONTEXT("SQL_SERIALIZER");
	str = makeStringInfo();
	viewDef = makeStringInfo();

	// DML and DDL statements
	if (isA(q, DLMorDDLOperator)) {
		result = serializeDMLandDDLPostgres(q);
		FREE_MEM_CONTEXT_AND_RETURN_STRING_COPY(result);
//		return serializeDMLandDDLPostgres(q);
	}

	// initialize basic structures and then call the worker
	api->tempViewMap = NEW_MAP(Constant, Node);
	api->viewCounter = 0;

	// initialize FromAttrsContext structure
	struct FromAttrsContext *fac = initializeFromAttrsContext();

	// call main entry point for translation
	api->serializeQueryOperator(q, str, NULL, fac, api);

	/*
	 *  prepend the temporary view definition to create something like
	 *      WITH a AS (q1), b AS (q2) ... SELECT ...
	 */
	if (mapSize(api->tempViewMap) > 0) {
		appendStringInfoString(viewDef, "WITH ");

		// loop through temporary views we have defined
		FOREACH_HASH(HashMap,view,api->tempViewMap)
		{
			appendStringInfoString(viewDef, TVIEW_GET_DEF(view));
			if (FOREACH_HASH_HAS_MORE(view))
				appendStringInfoString(viewDef, ",\n");
		}

		// prepend to query translation
		DEBUG_LOG("views are:\n\n%s", viewDef->data);
		result = CONCAT_STRINGS(viewDef->data, str->data);
	} else
		result = str->data;

	// copy result to callers memory context and clean up
	FREE_MEM_CONTEXT_AND_RETURN_STRING_COPY(result);
}

char*
quoteIdentifierPostgres(char *ident)
{
	int i = 0;
	boolean needsQuotes = FALSE;
	boolean containsQuotes = FALSE;

	// already quoted
	if (ident[0] == '"')
		return ident;

	// certain characters need to be quoted
	// also upper case needs to be quoted to preserve the case
    for(i = 0; i < strlen(ident); i++)
    {
        switch(ident[i])
        {
            case '$':
            case '#':
            case '_':
                break;
            case ' ':
            case '(':
            case ')':
                needsQuotes = TRUE;
                break;
            case '"':
                needsQuotes = TRUE;
                containsQuotes = TRUE;
                break;
            default:
                if (!isupper(ident[i]))
                    needsQuotes = TRUE;
                break;
        }
        if (needsQuotes)
            break;
    }

	if (containsQuotes)
		ident = replaceSubstr(ident, "\"", "\"\"");

	if (needsQuotes)
		ident = CONCAT_STRINGS("\"", ident, "\"");

	return ident;
}

static void
createAPI(void)
{
	if (api == NULL) {
		api = createAPIStub();
		api->serializeProjectionAndAggregation =
				serializeProjectionAndAggregation;
		api->serializeSetOperator = serializeSetOperator;
		api->serializeTableAccess = serializeTableAccess;
		api->serializeConstRel = serializeConstRel;
		api->serializeJoinOperator = serializeJoinOperator;
	}
}

static char*
serializeDMLandDDLPostgres(QueryOperator *q)
{
	DLMorDDLOperator *o = (DLMorDDLOperator*) q;
	StringInfo str = makeStringInfo();
	Node *stmt = o->stmt;

	switch (stmt->type) {
	case T_Update: {
		serializeUpdate(str, (Update*) stmt);
		break;
	}
		break;
	case T_Insert: {
		// this is current to support: insert into r values(xxxx);
		serializeInsert(str, (Insert*) stmt);
		// TODO need to support insert into r select xxxx;
		break;
	}
	case T_Delete:
		serializeDelete(str, (Delete*) stmt);
		break;
	case T_CreateTable:
		serializeCreateTable(str, (CreateTable*) stmt);
		break;
	case T_AlterTable:
		break;
	default:
		FATAL_LOG("should only pass DML and DDL nodes to this function.");
	}

	return str->data;
}

static void
serializeCreateTable(StringInfo str, CreateTable *c)
{
	appendStringInfo(str, "create table %s", c->tableName);
	appendStringInfo(str, "%s;", exprToSQL((Node*) c->tableElems, NULL, FALSE));
}

static void
serializeDelete(StringInfo str, Delete *d)
{
	appendStringInfo(str, "delete from %s %s;", d->deleteTableName,
					 (d->cond ? CONCAT_STRINGS("where ", exprToSQL(d->cond, NULL, FALSE)) : ""));
}

static void
serializeInsert(StringInfo str, Insert *i)
{
	if (isA(i->query, QueryBlock)) {
		// TODO this is for insert into tbl (a query);
//		appendStringInfo(str, "insert into %s ", i->insertTableName);
//		appendStringInfo(str, "select %s", exprToSQL((Node*) (((QueryBlock* )(i->query))->selectClause), NULL ));
//		appendStringInfoString(str, "from (");

	} else {
		appendStringInfo(str, "insert into %s values %s ;", i->insertTableName,
						 exprToSQL(i->query, NULL, FALSE));
	}
}

static void
serializeUpdate(StringInfo str, Update *u)
{
	INFO_LOG("SERIALIZE UPDATE\n");

	appendStringInfo(str, "update %s set", u->updateTableName);
	//Since exprToSQL add () for each time called, and this cause "update xxx set ((xx = xx), (xx = xx))..." error
	//iterate the selectClause list and exprToSQL() each item.
	// serialize left
	// =
	// serialize right
	int len = getListLength(u->selectClause);
	for (int i = 0; i < len; i++) {
		Operator *op = (Operator*) getNthOfListP(u->selectClause, i);
		appendStringInfo(str, " %s",
						 exprToSQL((Node*) getNthOfListP(op->args, 0), NULL, FALSE));
		appendStringInfo(str, " =");
		appendStringInfo(str, " %s",
						 exprToSQL((Node*) getNthOfListP(op->args, 1), NULL, FALSE));
		if (i != len - 1) {
			appendStringInfo(str, " ,");
		}
	}

	appendStringInfo(str, " %s;",
					 u->cond ? CONCAT_STRINGS("where ", exprToSQL(u->cond, NULL, FALSE)) : "");
}

static void
serializeJoinOperator(StringInfo from, QueryOperator *fromRoot, JoinOperator *j,
		int *curFromItem, int *attrOffset, FromAttrsContext *fac,
		SerializeClausesAPI *api)
{
	int rOffset;
	appendStringInfoString(from, "(");
	//left child
	api->serializeFromItem(fromRoot, OP_LCHILD(j), from, curFromItem,
			attrOffset, fac, api);

	fac->fromAttrsList = removeFromHead(fac->fromAttrsList);

	// join
	switch (j->joinType) {
	case JOIN_INNER:
		appendStringInfoString(from, " JOIN ");
		break;
	case JOIN_CROSS:
		appendStringInfoString(from, " CROSS JOIN ");
		break;
	case JOIN_LEFT_OUTER:
		appendStringInfoString(from, " LEFT OUTER JOIN ");
		break;
	case JOIN_RIGHT_OUTER:
		appendStringInfoString(from, " RIGHT OUTER JOIN ");
		break;
	case JOIN_FULL_OUTER:
		appendStringInfoString(from, " FULL OUTER JOIN ");
		break;
	}
	// right child
	rOffset = *curFromItem;
	api->serializeFromItem(fromRoot, OP_RCHILD(j), from, curFromItem,
			attrOffset, fac, api);
	// join condition
	if (j->cond)
		appendStringInfo(from, " ON (%s)",
				exprToSQLWithNamingScheme(copyObject(j->cond), rOffset, fac));
	appendStringInfoString(from, ")");
}



/*
 * Serialize a set operation UNION/EXCEPT/INTERSECT
 */
static List*
serializeSetOperator(QueryOperator *q, StringInfo str, FromAttrsContext *fac,
		SerializeClausesAPI *api)
{
	SetOperator *setOp = (SetOperator*) q;
	List *resultAttrs;

	//TODO be smarter to use UNION / INTERSECT / EXCEPT when these are implemented in rel algebra as duplicate elimination

    // output left child
	appendStringInfoString(str, "(");
    resultAttrs = api->serializeQueryOperator(OP_LCHILD(q), str, q, fac, api);

    // output set operation
    switch(setOp->setOpType)
    {
        case SETOP_UNION:
            appendStringInfoString(str, " UNION ALL ");
            break;
        case SETOP_INTERSECTION:
            appendStringInfoString(str, " INTERSECT ALL ");
            break;
        case SETOP_DIFFERENCE:
            appendStringInfoString(str, " EXCEPT ALL ");
            break;
    }

    // output right child
    api->serializeQueryOperator(OP_RCHILD(q), str, q, fac, api);
	appendStringInfoString(str, ")");

	return resultAttrs;
}

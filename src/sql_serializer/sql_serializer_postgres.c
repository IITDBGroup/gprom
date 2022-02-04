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
												StringInfo having, StringInfo groupBy, FromAttrsContext *fac, boolean materialize, SerializeClausesAPI *api);
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
 * Create the SELECT, GROUP BY, and HAVING clause
 */
static List *
serializeProjectionAndAggregation (QueryBlockMatch *m, StringInfo select,
        StringInfo having, StringInfo groupBy, FromAttrsContext *fac, boolean materialize, SerializeClausesAPI *api)
{
    int pos = 0;
    List *firstProjs = NIL;
    List *aggs = NIL;
    List *groupBys = NIL;
    List *windowFs = NIL;
//    List *secondProjs = NIL;
    List *resultAttrs = NIL;

    // either window funtions or regular aggregations but not both
    ASSERT(!m->windowRoot || !m->aggregation);

    AggregationOperator *agg = (AggregationOperator *) m->aggregation;
    WindowOperator *winR = (WindowOperator *) m->windowRoot;
    UpdateAggAndGroupByAttrState *state = NULL;

    appendStringInfoString(select, "\nSELECT ");
    if (materialize)
        appendStringInfoString(select, "/*+ materialize */ ");
    if (m->distinct)
        appendStringInfoString(select, " DISTINCT "); //TODO deal with distinct on attributes

    // Projection for aggregation inputs and group-by
    if (m->secondProj != NULL && (agg != NULL || winR != NULL))
    {
        FOREACH(Node,n,m->secondProj->projExprs)
        {
            updateAttributeNames(n, fac);
            firstProjs = appendToTailOfList(firstProjs, exprToSQL(n, NULL, FALSE));
        }
        DEBUG_LOG("second projection (aggregation and group by or window inputs) is %s",
                stringListToString(firstProjs));
    }


    // aggregation if need be
    if (agg != NULL)
    {
        DEBUG_LOG("deal with aggregation function calls");

        // aggregation
        FOREACH(Node,expr,agg->aggrs)
        {
            UPDATE_ATTR_NAME((m->secondProj == NULL), expr, fac, firstProjs);
//            if (m->secondProj == NULL)
//                updateAttributeNames(expr, fromAttrs);
//            else
//                updateAttributeNamesSimple(expr, firstProjs);
            aggs = appendToTailOfList(aggs, exprToSQL(expr, NULL, FALSE));
        }
        DEBUG_LOG("aggregation attributes are %s", stringListToString(aggs));

        // group by
        FOREACH(Node,expr,agg->groupBy)
        {
            char *g;
            if (pos++ == 0)
                appendStringInfoString (groupBy, "\nGROUP BY ");
            else
                appendStringInfoString (groupBy, ", ");

            UPDATE_ATTR_NAME((m->secondProj == NULL), expr, fac, firstProjs);
//            if (m->secondProj == NULL)
//                updateAttributeNames(expr, fromAttrs);
//            else
//                updateAttributeNamesSimple(expr, firstProjs);
            g = exprToSQL(expr, NULL, FALSE);

            groupBys = appendToTailOfList(groupBys, g);
            appendStringInfo(groupBy, "%s", strdup(g));
        }
        DEBUG_LOG("group by attributes are %s", stringListToString(groupBys));

        state = NEW(UpdateAggAndGroupByAttrState);
        state->aggNames = aggs;
        state->groupByNames = groupBys;
    }
    // window functions
    if (winR != NULL)
    {
        QueryOperator *curOp = (QueryOperator *) winR;
        List *inAttrs = (m->secondProj) ? firstProjs : getQueryOperatorAttrNames(m->fromRoot);
        DEBUG_LOG("deal with window function calls");

        windowFs = NIL;

        while(isA(curOp,WindowOperator))
        {
            WindowOperator *wOp = (WindowOperator *) curOp;
            Node *expr = wOp->f;

            DEBUG_LOG("BEFORE: window function = %s",
					  exprToSQL((Node *) winOpGetFunc((WindowOperator *) curOp), NULL, FALSE));

            UPDATE_ATTR_NAME((m->secondProj == NULL), expr, fac, firstProjs);
//            if (m->secondProj == NULL)
//                updateAttributeNames(expr, fromAttrs);
//            else
//                updateAttributeNamesSimple(expr, firstProjs);
            UPDATE_ATTR_NAME((m->secondProj == NULL), wOp->partitionBy, fac, firstProjs);
            UPDATE_ATTR_NAME((m->secondProj == NULL), wOp->orderBy, fac, firstProjs);
            UPDATE_ATTR_NAME((m->secondProj == NULL), wOp->frameDef, fac, firstProjs);

            if(HAS_STRING_PROP(curOp, PROP_SETBITS_CAST_NUM_WINDOW_MARK))
            {
            	int numFrags = INT_VALUE(GET_STRING_PROP(curOp, PROP_SETBITS_CAST_NUM_WINDOW_MARK));
                windowFs = appendToHeadOfList(windowFs,
    										  exprToSQL((Node *) createCastExprOtherDT((Node *) winOpGetFunc((WindowOperator *) curOp),"bit", numFrags, DT_STRING), NULL, FALSE));
            }
            else
            	windowFs = appendToHeadOfList(windowFs,
										  exprToSQL((Node *) winOpGetFunc((WindowOperator *) curOp), NULL, FALSE));


            DEBUG_LOG("AFTER: window function = %s", exprToSQL((Node *) winOpGetFunc((WindowOperator *) curOp), NULL, FALSE));

            curOp = OP_LCHILD(curOp);
        }

        windowFs = CONCAT_LISTS(deepCopyStringList(inAttrs), windowFs);

        state = NEW(UpdateAggAndGroupByAttrState);
        state->aggNames = windowFs;
        state->groupByNames = NIL;

        DEBUG_LOG("window function translated, %s", stringListToString(windowFs));
    }

    // having
    if (m->having != NULL)
    {
        SelectionOperator *sel = (SelectionOperator *) m->having;
        DEBUG_LOG("having condition %s", nodeToString(sel->cond));
        updateAggsAndGroupByAttrs(sel->cond, state);
        appendStringInfo(having, "\nHAVING %s", exprToSQL(sel->cond, NULL, FALSE));
        DEBUG_LOG("having translation %s", having->data);
    }

    // second level of projection either if no aggregation or using aggregation
    if ((m->secondProj != NULL && !agg && !winR ) || (m->firstProj != NULL && agg) || (m->firstProj != NULL && winR))
    {
        int pos = 0;
        ProjectionOperator *p = (agg || winR) ? m->firstProj : m->secondProj;
        List *attrNames = getAttrNames(p->op.schema);
        // create result attribute names
//        List *resultAttrs = NIL;

        DEBUG_LOG("outer projection");

        FOREACH(Node,a,p->projExprs)
        {
            char *attrName = (char *) getNthOfListP(attrNames, pos);
            if (pos++ != 0)
                appendStringInfoString(select, ", ");

            // is projection over aggregation
            if (agg)
                updateAggsAndGroupByAttrs(a, state); //TODO check that this method is still valid
            // is projection over window functions
            else if (winR)
                updateAggsAndGroupByAttrs(a, state);
            // is projection in query without aggregation
            else
                updateAttributeNames(a, fac);
            appendStringInfo(select, "%s%s", exprToSQL(a, NULL, FALSE), attrName ? CONCAT_STRINGS(" AS ", attrName) : "");
        }

        resultAttrs = attrNames;
        DEBUG_LOG("second projection expressions %s", select->data);
    }
    // else if window operator get the attributes from top-most window operator
    else if (winR)
    {
        int pos = 0;
        char *name;
        resultAttrs = getQueryOperatorAttrNames((QueryOperator *) winR);

        FOREACH(char,a,windowFs)
        {
            name = getNthOfListP(resultAttrs, pos);
            if (pos++ != 0)
                appendStringInfoString(select, ", ");
            appendStringInfo(select, "%s AS %s", a, name);
        }

        DEBUG_LOG("window functions results as projection expressions %s", select->data);
    }
    // get aggregation result attributes
    else if (agg)
    {
        int pos = 0;
        char *name;
        resultAttrs = getQueryOperatorAttrNames((QueryOperator *) agg);

        FOREACH(char,a,aggs)
        {
            name = getNthOfListP(resultAttrs, pos);
            if (pos++ != 0)
                appendStringInfoString(select, ", ");
            appendStringInfo(select, "%s AS %s", a, name);
        }
        FOREACH(char,gb,groupBys)
        {
            name = getNthOfListP(resultAttrs, pos);
            if (pos++ != 0)
                appendStringInfoString(select, ", ");
            appendStringInfo(select,  "%s AS %s", gb, name);
        }

        DEBUG_LOG("aggregation result as projection expressions %s", select->data);
    }
    // get attributes from FROM clause root
    else
    {
        List *inAttrs = NIL;
        int fromItem = 0;

        // attribute aliases are determined by the fromRoot operator's schema
        resultAttrs = getQueryOperatorAttrNames(m->fromRoot);//TODO
        // construct list of from clause attribute names with from clause item aliases
//        FOREACH(List, attrs, fromAttrs)
//        {
//            FOREACH(char,name,attrs)
//                 inAttrs = appendToTailOfList(inAttrs, CONCAT_STRINGS("F", gprom_itoa(fromItem), ".", name));
//            fromItem++;
//        }
        FOREACH(List, attrs, fac->fromAttrs)
        {
            FOREACH(char,name,attrs)
                 inAttrs = appendToTailOfList(inAttrs, CONCAT_STRINGS("F", gprom_itoa(fromItem), "_", gprom_itoa(LIST_LENGTH(fac->fromAttrsList) - 1), ".", name));
            fromItem++;
        }

        // construct select clause
        FORBOTH(char,outName,inName,resultAttrs,inAttrs)
        {
            if (pos++ != 0)
                appendStringInfoString(select, ", ");
            appendStringInfo(select, "%s AS %s", inName, outName);
        }

        DEBUG_LOG("FROM root attributes as projection expressions %s", select->data);
    }

    if (state)
        FREE(state);

    return resultAttrs;
}

static void
serializeConstRel(StringInfo from, ConstRelOperator* t, FromAttrsContext *fac,
        int* curFromItem, SerializeClausesAPI *api)
{
    int pos = 0;
    List* attrNames = getAttrNames(((QueryOperator*) t)->schema);
    //*fromAttrs = appendToTailOfList(*fromAttrs, attrNames);
    fac->fromAttrs = appendToTailOfList(fac->fromAttrs, attrNames);
    fac->fromAttrsList = appendToHeadOfList(fac->fromAttrsList, copyList(fac->fromAttrs));
    appendStringInfoString(from, "(SELECT ");
    FOREACH(char,attrName,attrNames)
    {
        Node *value;
        if (pos != 0)
            appendStringInfoString(from, ", ");
        value = getNthOfListP(t->values, pos++);
        appendStringInfo(from, "%s AS %s", exprToSQL(value, NULL, FALSE), attrName);

    }

    appendStringInfo(from, ") F%u_%u", (*curFromItem)++, LIST_LENGTH(fac->fromAttrsList) - 1);
}

//static void
//serializeTableAccess(StringInfo from, TableAccessOperator* t, int* curFromItem,
//        List** fromAttrs, int* attrOffset, SerializeClausesAPI *api)
static void
serializeTableAccess(StringInfo from, TableAccessOperator* t, int* curFromItem,
		FromAttrsContext *fac, int* attrOffset, SerializeClausesAPI *api)
{
    char* asOf = NULL;

    // use history join to prefilter updated rows
    if (HAS_STRING_PROP(t, PROP_USE_HISTORY_JOIN))
    {
        List* scnsAndXid = (List*) GET_STRING_PROP(t, PROP_USE_HISTORY_JOIN);
        Constant *startScn, *commitScn, *commitMinusOne;
        Constant* xid;
        StringInfo attrNameStr = makeStringInfo();
        List* attrNames = getAttrNames(((QueryOperator*) t)->schema);
        int i = 0;
        xid = (Constant*) getNthOfListP(scnsAndXid, 0);
        startScn = (Constant*) getNthOfListP(scnsAndXid, 1);
        commitScn = (Constant*) getNthOfListP(scnsAndXid, 2);
        commitMinusOne = createConstLong(LONG_VALUE(commitScn) - 1);
        FOREACH(char,a,attrNames)
        {
            appendStringInfo(attrNameStr, "%s%s", (i++ == 0) ? "" : ", ", a);
        }
        // read committed?
        if (HAS_STRING_PROP(t, PROP_IS_READ_COMMITTED))
        {
            appendStringInfo(from, "(SELECT %s \nFROM\n", attrNameStr->data);
            appendStringInfo(from, "\t(SELECT ROWID AS rid , %s",
                    attrNameStr->data);
            appendStringInfo(from,
                    "\tFROM %s VERSIONS BETWEEN SCN %u AND %u) F0",
                    t->tableName, LONG_VALUE(commitMinusOne),
                    LONG_VALUE(commitMinusOne));
            appendStringInfoString(from, "\n JOIN ");
            appendStringInfo(from,
                    "\t(SELECT ROWID AS rid FROM %s VERSIONS BETWEEN SCN %u AND %u F1 ",
                    t->tableName, LONG_VALUE(commitScn), LONG_VALUE(commitScn));
            appendStringInfo(from, "WHERE VERSIONS_XID = HEXTORAW('%s')) F1",
                    STRING_VALUE(xid));
            appendStringInfo(from, " ON (F0.rid = F1.rid)) F%u",
                    (*curFromItem)++);
        }
        else
        {
            appendStringInfo(from, "(SELECT %s \nFROM\n", attrNameStr->data);
            appendStringInfo(from, "\t(SELECT ROWID AS rid , %s",
                    attrNameStr->data);
            appendStringInfo(from, "\tFROM %s AS OF SCN %u) F0", t->tableName,
                    LONG_VALUE(startScn));
            appendStringInfoString(from, "\n JOIN ");
            appendStringInfo(from,
                    "\t(SELECT ROWID AS rid FROM %s VERSIONS BETWEEN SCN %u AND %u F1 ",
                    t->tableName, LONG_VALUE(commitScn), LONG_VALUE(commitScn));
            appendStringInfo(from, "WHERE VERSIONS_XID = HEXTORAW('%s')) F1",
                    STRING_VALUE(xid));
            appendStringInfo(from, " ON (F0.rid = F1.rid)) F%u",
                    (*curFromItem)++);
        }
        //*fromAttrs = appendToTailOfList(*fromAttrs, attrNames);
        fac->fromAttrs = appendToTailOfList(fac->fromAttrs, attrNames);
        fac->fromAttrsList = appendToHeadOfList(fac->fromAttrsList, copyList(fac->fromAttrs));
    }
    else
    {
        // add list of attributes as list to fromAttrs
        *attrOffset = 0;
        if (t->asOf)
        {
            if (isA(t->asOf, Constant))
            {
                Constant* c = (Constant*) t->asOf;
                if (c->constType == DT_LONG)
                    asOf = CONCAT_STRINGS(" AS OF SCN ", exprToSQL(t->asOf, NULL, FALSE));
                else
                    asOf = CONCAT_STRINGS(" AS OF TIMESTAMP to_timestamp(",
                            exprToSQL(t->asOf, NULL, FALSE), ")");
            }
            else
            {
                List* scns = (List*) t->asOf;
                Node* begin = (Node*) getNthOfListP(scns, 0);
                Node* end = (Node*) getNthOfListP(scns, 1);
                asOf = CONCAT_STRINGS(" VERSIONS BETWEEN SCN ",
                        exprToSQL(begin, NULL, FALSE), " AND ", exprToSQL(end, NULL, FALSE));
            }
        }

        List* attrNames = getAttrNames(((QueryOperator*) t)->schema);
        //*fromAttrs = appendToTailOfList(*fromAttrs, attrNames);
        fac->fromAttrs = appendToTailOfList(fac->fromAttrs, attrNames);
        DEBUG_LOG("table access append fac->fromAttrsList");
        //append fromAttrs into fromAttrsList, e.g., fromAttrs: ((A,B)), fromAttrsList: ( ((A,B)) )
        fac->fromAttrsList = appendToHeadOfList(fac->fromAttrsList, copyList(fac->fromAttrs));
        printFromAttrsContext(fac);

        //for temporal database coalesce
        if(HAS_STRING_PROP(t,PROP_TEMP_TNTAB))
        {
            QueryOperator *inp = (QueryOperator *) LONG_VALUE(GET_STRING_PROP(t,PROP_TEMP_TNTAB));
            StringInfo tabName = makeStringInfo();
            QueryOperator *inpParent = (QueryOperator *) getHeadOfListP(inp->parents);
            api->createTempView(inp, tabName,inpParent, fac, api);
            appendStringInfo(from, "generate_series(1,(SELECT MAX(NUMOPEN) FROM (%s) F0)) F%u(n)",
                    tabName->data, (*curFromItem)++);
//            appendStringInfo(from, " ((SELECT ROWNUM N FROM DUAL CONNECT BY LEVEL <= (SELECT MAX(NUMOPEN) FROM ((%s) F0))) F%u)",
//                  tabName->data, (*curFromItem)++);
        }
        else
        {
//            appendStringInfo(from, "%s%s AS F%u",
//                    quoteIdentifierPostgres(t->tableName), asOf ? asOf : "",
//                    (*curFromItem)++);
        	if(!getBoolOption(OPTION_PS_POST_TO_ORACLE))
        	{
    			appendStringInfo(from, "%s%s F%u_%u",
    					quoteIdentifierPostgres(t->tableName), asOf ? asOf : "",
    					(*curFromItem)++, LIST_LENGTH(fac->fromAttrsList) - 1);
        	}
        	else
        	{
    			appendStringInfo(from, "%s%s F%u_%u",
    					t->tableName, asOf ? asOf : "",
    					(*curFromItem)++, LIST_LENGTH(fac->fromAttrsList) - 1);
        	}
        }


    }
}

/*
 * Serialize a set operation UNION/EXCEPT/INTERSECT
 */
static List *
serializeSetOperator (QueryOperator *q, StringInfo str, FromAttrsContext *fac, SerializeClausesAPI *api)
{
    SetOperator *setOp = (SetOperator *) q;
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

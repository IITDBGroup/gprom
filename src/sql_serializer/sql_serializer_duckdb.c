/*-----------------------------------------------------------------------------
 *
 * sql_serializer_sqlite.c
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

#include "log/logger.h"

#include "sql_serializer/sql_serializer_common.h"
#include "sql_serializer/sql_serializer_duckdb.h"
#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/set/hashmap.h"

#include "utility/string_utils.h"

/* vars */
static SerializeClausesAPI *api = NULL;

/* methods */
static boolean replaceFunctionsWithEquivalent(Node *node, void *context);
static boolean replaceBoolWithInt (Node *node, void *context);
static void createAPI (void);
static void serializeJoinOperator(StringInfo from, QueryOperator* fromRoot, JoinOperator* j,
        int* curFromItem, int* attrOffset, FromAttrsContext *fac, SerializeClausesAPI *api);
static List *serializeProjectionAndAggregation (QueryBlockMatch *m, StringInfo select,
        StringInfo having, StringInfo groupBy, FromAttrsContext *fac, boolean materialize, SerializeClausesAPI *api);
static void serializeConstRel(StringInfo from, ConstRelOperator* t, FromAttrsContext *fac,
        int* curFromItem, SerializeClausesAPI *api);
static void serializeTableAccess(StringInfo from, TableAccessOperator* t, int* curFromItem,
		FromAttrsContext *fac, int* attrOffset, SerializeClausesAPI *api);
static List *serializeSetOperator(QueryOperator *q, StringInfo str, FromAttrsContext *fac, SerializeClausesAPI *api);

char *
serializeOperatorModelDuckDB(Node *q) // TODO
{
    StringInfo str = makeStringInfo();
    char *result = NULL;

    createAPI();
    // shorten attribute names to confrom with Oracle limits
    if (IS_OP(q))
    {
        appendStringInfoString(str, serializeQueryDuckDB((QueryOperator *) q));
        appendStringInfoChar(str,';');
    }
    else if (isA(q, List))
        FOREACH(QueryOperator,o,(List *) q)
        {
            appendStringInfoString(str, serializeQueryDuckDB(o));
            appendStringInfoString(str,";\n\n");
        }
    else
        FATAL_LOG("cannot serialize non-operator to SQL: %s", nodeToString(q));

    result = str->data;
    FREE(str);
    return result;
}

char *
serializeQueryDuckDB(QueryOperator *q)
{
    StringInfo str;
    StringInfo viewDef;
    char *result;

    createAPI();

    NEW_AND_ACQUIRE_MEMCONTEXT("SQL_SERIALIZER");
    str = makeStringInfo();
    viewDef = makeStringInfo();

    // replace boolean with ints
    replaceBoolWithInt((Node *) q, NULL);

    // initialize basic structures and then call the worker
    api->tempViewMap = NEW_MAP(Constant, Node);
    api->viewCounter = 0;

    // simulate non Oracle conformant data types and expressions (boolean)
    genQuoteAttributeNames((Node *) q);

    // initialize FromAttrsContext structure
  	struct FromAttrsContext *fac = initializeFromAttrsContext();

	// replace functions not supported by SQLite with equivalent alternatives
	replaceFunctionsWithEquivalent((Node *) q, NULL);

    // call main entry point for translation
    api->serializeQueryOperator (q, str, NULL, fac, api);

    /*
     *  prepend the temporary view definition to create something like
     *      WITH a AS (q1), b AS (q2) ... SELECT ...
     */
    if (mapSize(api->tempViewMap) > 0)
    {
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
    }
    else
        result = str->data;

    // copy result to callers memory context and clean up
    FREE_MEM_CONTEXT_AND_RETURN_STRING_COPY(result);
}

char *
quoteIdentifierSQLite (char *ident)
{
    int i = 0;
    boolean needsQuotes = FALSE;
    boolean containsQuotes = FALSE;

    // already quoted
    if (ident[0] == '"')
        return ident;

    // sqlite completely ignores case no matter whether the identifier is quoted or not
    // so upper/lower case does not indicate whether we need to escape
    for(i = 0; i < strlen(ident); i++)
    {
        switch(ident[i])
        {
            case '$':
            case '#':
            case '_':
                break;
            case '(':
            case ')':
            case '+':
            case ' ':
                needsQuotes = TRUE;
            break;
            case '"':
                needsQuotes = TRUE;
                containsQuotes = TRUE;
            break;
            default:
                break;
        }
        if (needsQuotes)
            break;
    }

    if (containsQuotes)
        ident = replaceSubstr(ident, "\"", "\"\"");

    if (needsQuotes)
        ident = CONCAT_STRINGS("\"",ident,"\"");

    return ident;
}

static boolean
replaceFunctionsWithEquivalent(Node *node, void *context)
{
	if (node == NULL)
		return TRUE;

	if (isA(node, FunctionCall))
	{
		FunctionCall *f = (FunctionCall *) node;
		if (streq(f->functionname, LEAST_FUNC_NAME))
			f->functionname = strdup(MIN_FUNC_NAME);
		if (streq(f->functionname, GREATEST_FUNC_NAME))
			f->functionname = strdup(GREATEST_FUNC_NAME);
	}

	return visit(node, replaceFunctionsWithEquivalent, context);
}

static boolean
replaceBoolWithInt (Node *node, void *context)
{
    if (node == NULL)
        return TRUE;

    // replace boolean constants with 1/0
    if (isA(node,Constant))
    {
        Constant *c = (Constant *) node;

        if (c->constType == DT_BOOL)
        {
            boolean val = BOOL_VALUE(c);
            c->constType = DT_INT;
            c->value = NEW(int);
            if (val)
                INT_VALUE(c) = 1;
            else
                INT_VALUE(c) = 0;

        }
        return TRUE;
    }

    return visit(node, replaceBoolWithInt, context);
}


static void
createAPI (void)
{
    if (api == NULL)
    {
        api = createAPIStub();
        api->serializeProjectionAndAggregation = serializeProjectionAndAggregation;
        api->serializeSetOperator = serializeSetOperator;
        api->serializeTableAccess = serializeTableAccess;
        api->serializeConstRel = serializeConstRel;
        api->serializeJoinOperator = serializeJoinOperator;
    }
}

static void
serializeJoinOperator(StringInfo from, QueryOperator* fromRoot, JoinOperator* j,
        int* curFromItem, int* attrOffset, FromAttrsContext *fac, SerializeClausesAPI *api)
{
    int rOffset;
    appendStringInfoString(from, "(");
    //left child
    api->serializeFromItem(fromRoot, OP_LCHILD(j), from, curFromItem, attrOffset,
            fac, api);

    //fac->fromAttrsList = removeFromHead(fac->fromAttrsList);

    // join
    switch (j->joinType)
    {
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
    api->serializeFromItem(fromRoot, OP_RCHILD(j), from, curFromItem, attrOffset,
    		fac, api);
    // join condition
    if (j->cond)
        appendStringInfo(from, " ON (%s)",
                exprToSQLWithNamingScheme(copyObject(j->cond), rOffset,
                		fac));
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
            aggs = appendToTailOfList(aggs, exprToSQL(expr, NULL, FALSE)); //TODO
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

            DEBUG_LOG("BEFORE: window function = %s", exprToSQL((Node *) winOpGetFunc((WindowOperator *) curOp), NULL, FALSE));

            UPDATE_ATTR_NAME((m->secondProj == NULL), expr, fac, firstProjs);
//            if (m->secondProj == NULL)
//                updateAttributeNames(expr, fromAttrs);
//            else
//                updateAttributeNamesSimple(expr, firstProjs);
            UPDATE_ATTR_NAME((m->secondProj == NULL), wOp->partitionBy, fac, firstProjs);
            UPDATE_ATTR_NAME((m->secondProj == NULL), wOp->orderBy, fac, firstProjs);
            UPDATE_ATTR_NAME((m->secondProj == NULL), wOp->frameDef, fac, firstProjs);

            windowFs = appendToHeadOfList(windowFs, exprToSQL((Node *) winOpGetFunc(
																  (WindowOperator *) curOp), NULL, FALSE));

            DEBUG_LOG("AFTER: window function = %s", exprToSQL((Node *) winOpGetFunc(
																   (WindowOperator *) curOp), NULL, FALSE));

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
				inAttrs = appendToTailOfList(inAttrs, CONCAT_STRINGS("F", gprom_itoa(fromItem), "_", gprom_itoa(LIST_LENGTH(fac->fromAttrsList)), ".", name)); //FIXME minux outer levels up?
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
    //fac->fromAttrsList = appendToHeadOfList(fac->fromAttrsList, copyList(fac->fromAttrs));
    appendStringInfoString(from, "(SELECT ");
    FOREACH(char,attrName,attrNames)
    {
        Node *value;
        if (pos != 0)
            appendStringInfoString(from, ", ");
        value = getNthOfListP(t->values, pos++);
        appendStringInfo(from, "%s AS %s", exprToSQL(value, NULL, FALSE), attrName);

    }
    appendStringInfo(from, ") F%u", (*curFromItem)++);
}

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
        //fac->fromAttrsList = appendToHeadOfList(fac->fromAttrsList, copyList(fac->fromAttrs));
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
       // *fromAttrs = appendToTailOfList(*fromAttrs, attrNames);
        fac->fromAttrs = appendToTailOfList(fac->fromAttrs, attrNames);
        DEBUG_LOG("table access append fac->fromAttrsList");
        //append fromAttrs into fromAttrsList, e.g., fromAttrs: ((A,B)), fromAttrsList: ( ((A,B)) )
        //fac->fromAttrsList = appendToHeadOfList(fac->fromAttrsList, copyList(fac->fromAttrs));
        printFromAttrsContext(fac);
//        appendStringInfo(from, "%s%s AS F%u",
//                quoteIdentifierSQLite(t->tableName), asOf ? asOf : "",
//                (*curFromItem)++);
		appendStringInfo(from, "%s%s F%u_%u",
				quoteIdentifierSQLite(t->tableName), asOf ? asOf : "",
				(*curFromItem)++, LIST_LENGTH(fac->fromAttrsList));
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

	// wrap set operation into a block to ensure that we can preserve evaluation order (SQLite does not support parenthesis around set operations)
	appendStringInfoString(str, "SELECT * FROM (");

    // output left child
    resultAttrs = api->serializeQueryOperator(OP_LCHILD(q), str, q, fac, api);

    // output set operation
    switch(setOp->setOpType)
    {
        case SETOP_UNION:
            appendStringInfoString(str, " UNION ALL ");
            break;
			//TODO SQLite does not support bag intersect and difference, have to implement workaround query
        case SETOP_INTERSECTION:
            appendStringInfoString(str, " INTERSECT ");
            break;
        case SETOP_DIFFERENCE:
            appendStringInfoString(str, " EXCEPT ");
            break;
    }

    // output right child
    api->serializeQueryOperator(OP_RCHILD(q), str, q, fac, api);
	appendStringInfoString(str, ")");

    return resultAttrs;
}

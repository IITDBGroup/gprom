/*-----------------------------------------------------------------------------
 *
 * sql_serializer_oracle.c
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

#include "sql_serializer/sql_serializer_oracle.h"
#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/list/list.h"
#include "model/set/set.h"



/* data structures */
typedef enum MatchState {
    MATCH_START,
    MATCH_DISTINCT,
    MATCH_FIRST_PROJ,
    MATCH_HAVING,
    MATCH_AGGREGATION,
    MATCH_SECOND_PROJ,
    MATCH_WHERE,
    MATCH_WINDOW,
    MATCH_ORDER,
    MATCH_NEXTBLOCK
} MatchState;

#define OUT_MATCH_STATE(_state) \
    (_state == MATCH_START ? "MATCH_START" : \
     _state == MATCH_DISTINCT ? "MATCH_DISTINCT" : \
     _state == MATCH_FIRST_PROJ ? "MATCH_FIRST_PROJ" : \
     _state == MATCH_HAVING ? "MATCH_HAVING" : \
     _state == MATCH_AGGREGATION ? "MATCH_AGGREGATION" : \
     _state == MATCH_SECOND_PROJ ? "MATCH_SECOND_PROJ" : \
     _state == MATCH_WHERE ? "MATCH_WHERE" : \
     _state == MATCH_WINDOW ? "MATCH_WINDOW" : \
     _state == MATCH_WINDOW ? "MATCH_ORDER" : \
             "MATCH_NEXTBLOCK" \
     )

typedef struct QueryBlockMatch {
    DuplicateRemoval *distinct;
    ProjectionOperator *firstProj;
    SelectionOperator *having;
    AggregationOperator *aggregation;
    ProjectionOperator *secondProj;
    SelectionOperator *where;
    QueryOperator *fromRoot;
    WindowOperator *windowRoot;
    OrderOperator *orderBy;
} QueryBlockMatch;

#define OUT_BLOCK_MATCH(_level,_m,_message) \
    do { \
        _level ## _LOG ("MATCH INFO: %s", _message); \
        _level ## _LOG ("distinct: %s", operatorToOverviewString((Node *) _m->distinct)); \
        _level ## _LOG ("firstProj: %s", operatorToOverviewString((Node *) _m->firstProj)); \
        _level ## _LOG ("having: %s", operatorToOverviewString((Node *) _m->having)); \
        _level ## _LOG ("aggregation: %s", operatorToOverviewString((Node *) _m->aggregation)); \
        _level ## _LOG ("secondProj: %s", operatorToOverviewString((Node *) _m->secondProj)); \
        _level ## _LOG ("where: %s", operatorToOverviewString((Node *) _m->where)); \
        _level ## _LOG ("fromRoot: %s", operatorToOverviewString((Node *) _m->fromRoot)); \
        _level ## _LOG ("windowRoot: %s", operatorToOverviewString((Node *) _m->windowRoot)); \
        _level ## _LOG ("orderBy: %s", operatorToOverviewString((Node *) _m->orderBy)); \
    } while(0)

typedef struct TemporaryViewMap {
    QueryOperator *viewOp; // the key
    char *viewName;
    char *viewDefinition;
    List *attrNames;
    UT_hash_handle hh;
} TemporaryViewMap;

typedef struct UpdateAggAndGroupByAttrState {
    List *aggNames;
    List *groupByNames;
} UpdateAggAndGroupByAttrState;

typedef struct JoinAttrRenameState {
    int rightFromOffsets;
    List *fromAttrs;
} JoinAttrRenameState;

/* macros */
#define OPEN_PARENS(str) appendStringInfoChar(str, '(')
#define CLOSE_PARENS(str) appendStringInfoChar(str, ')')
#define WITH_PARENS(str,operation) \
    do { \
        OPEN_PARENS(str); \
        operation; \
        CLOSE_PARENS(str); \
    } while(0)

#define ORACLE_IDENT_LIMIT 30

/* variables */
static TemporaryViewMap *viewMap;
static int viewNameCounter;

/* method declarations */
static boolean quoteAttributeNames (Node *node, void *context);

static void  makeDTOracleConformant(QueryOperator *q);
static boolean replaceNonOracleDTs (Node *node, void *context);

static List *serializeQueryOperator (QueryOperator *q, StringInfo str);
static List *serializeQueryBlock (QueryOperator *q, StringInfo str);
static List *serializeSetOperator (QueryOperator *q, StringInfo str);

static void serializeFrom (QueryOperator *q, StringInfo from, List **fromAttrs);
static void serializeFromItem (QueryOperator *q, StringInfo from,
        int *curFromItem, int *attrOffset, List **fromAttrs);

static void serializeWhere (SelectionOperator *q, StringInfo where, List *fromAttrs);
static boolean updateAttributeNames(Node *node, List *fromAttrs);
static boolean updateAttributeNamesSimple(Node *node, List *attrNames);

static List *serializeProjectionAndAggregation(QueryBlockMatch *m, StringInfo select,
        StringInfo having, StringInfo groupBy, List *fromAttrs, boolean materialize);

static char *exprToSQLWithNamingScheme(Node *expr, int rOffset, List *fromAttrs);
static boolean renameAttrsVisitor(Node *node, JoinAttrRenameState *state);

static char *createAttrName(char *name, int fItem);
//static char *createFromNames(int *attrOffset, int count);

static List *createTempView(QueryOperator *q, StringInfo str);
static char *createViewName(void);
static void shortenAttributeNames(QueryOperator *q);
static inline char *getShortAttr(char *newName, int id, boolean quoted);
static void fixAttrReferences (QueryOperator *q);


char *
serializeOperatorModelOracle(Node *q)
{
    StringInfo str = makeStringInfo();
    char *result = NULL;



    // quote ident names if necessary
    quoteAttributeNames(q, NULL);

    if (IS_OP(q))
    {
        // shorten attribute names to oracle's 30 char limit
        shortenAttributeNames((QueryOperator *) q);
        appendStringInfoString(str, serializeQueryOracle((QueryOperator *) q));
        appendStringInfoChar(str,';');
    }
    else if (isA(q, List))
        FOREACH(QueryOperator,o,(List *) q)
        {
            // shorten attribute names to oracle's 30 char limit
            shortenAttributeNames(o);
            appendStringInfoString(str, serializeQueryOracle(o));
            appendStringInfoString(str,";\n\n");
        }
    else
        FATAL_LOG("cannot serialize non-operator to SQL: %s", nodeToString(q));

    result = str->data;
    FREE(str);
    return result;
}

static void
shortenAttributeNames(QueryOperator *q)
{
    Set *newAttrNames = STRSET();
    List *attrs = q->schema->attrDefs;

    FOREACH(QueryOperator,child,q->inputs)
        shortenAttributeNames(child);

    fixAttrReferences(q);

    FOREACH(AttributeDef,a,attrs)
    {
        char *name = a->attrName;
        boolean isQuoted = (name[0] == '"');
        int addQuotLen = (isQuoted ? 2 : 0);

        // deal with quoted identifiers (quotes are not part of the limit)
        if (strlen(name) > (ORACLE_IDENT_LIMIT + addQuotLen))
        {
            char *newName = MALLOC(ORACLE_IDENT_LIMIT + 1 + addQuotLen);
            memcpy(newName,name,ORACLE_IDENT_LIMIT  + 1 + addQuotLen);
            newName[ORACLE_IDENT_LIMIT  + addQuotLen] = '\0';
            if (isQuoted)
                newName[ORACLE_IDENT_LIMIT + 1] = '"';
            int dup = 0;

            //TODO make more efficient
            while(hasSetElem(newAttrNames,getShortAttr(newName, dup++, isQuoted)))
                ;

            DEBUG_LOG("shorten attr <%s> to <%s>", a->attrName, newName);

            addToSet(newAttrNames, newName);
            a->attrName = strdup(newName);
        }
    }
}

static inline char*
getShortAttr(char *newName, int id, boolean quoted)
{
    char *idStr = itoa(id);
    int idLen = strlen(idStr) + 1;
    int offset = ORACLE_IDENT_LIMIT  - idLen + (quoted ? 2 : 0);
    memcpy(newName + offset, idStr, idLen - 1);
    newName[offset - 1] = '#';

    DEBUG_LOG("shortened attr  <%s>", newName);

    return newName;
}

static void
fixAttrReferences (QueryOperator *q)
{
    List *attRefs = getAttrRefsInOperator (q);

    FOREACH(AttributeReference, a, attRefs)
    {
        QueryOperator *child = getNthOfListP(q->inputs, a->fromClauseItem);
        char *newName = getAttrNameByPos(child,a->attrPosition);
        if (!streq(newName, a->name))
            a->name = strdup(newName);
    }
}

static boolean
quoteAttributeNames (Node *node, void *context)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, AttributeReference))
    {
        AttributeReference *a = (AttributeReference *) node;
        a->name = quoteIdentifierOracle(a->name);
    }
    if (isA(node, SelectItem))
    {
        SelectItem *a = (SelectItem *) node;
        a->alias = quoteIdentifierOracle(a->alias);
    }
    if (isA(node, AttributeDef))
    {
        AttributeDef *a = (AttributeDef *) node;
        a->attrName = quoteIdentifierOracle(a->attrName);
    }

    return visit(node, quoteAttributeNames, context);
}


char *
serializeQueryOracle(QueryOperator *q)
{
    StringInfo str;
    StringInfo viewDef;
    char *result;

    NEW_AND_ACQUIRE_MEMCONTEXT("SQL_SERIALIZER");

    str = makeStringInfo();
    viewDef = makeStringInfo();

    // initialize basic structures and then call the worker
    viewMap = NULL;
    viewNameCounter = 0;

    // simulate non Oracle conformant data types and expressions (boolean)
    makeDTOracleConformant(q);

    // call main entry point for translation
    serializeQueryOperator (q, str);

    /*
     *  prepend the temporary view definition to create something like
     *      WITH a AS (q1), b AS (q2) ... SELECT ...
     */
    if (HASH_COUNT(viewMap) > 0)
    {
        appendStringInfoString(viewDef, "WITH ");

        // loop through temporary views we have defined
        for(TemporaryViewMap *view = viewMap; view != NULL; view = view->hh.next)
        {
            appendStringInfoString(viewDef, view->viewDefinition);
            if (view->hh.next != NULL)
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

/*
 * Replace boolean data types
 */
static void
makeDTOracleConformant(QueryOperator *q)
{
    replaceNonOracleDTs((Node *) q, NULL);
}

static boolean
replaceNonOracleDTs (Node *node, void *context)
{
    if (node == NULL)
        return TRUE;

    //TODO keep context
    //TODO take care of boolean expressions that are used where Oracle expects
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
    }

    return visit(node, replaceNonOracleDTs, context);
}

/*
 * Main entry point for serialization.
 */
static List *
serializeQueryOperator (QueryOperator *q, StringInfo str)
{
    // operator with multiple parents
    if (LIST_LENGTH(q->parents) > 1 || HAS_STRING_PROP(q,PROP_MATERIALIZE))
        return createTempView (q, str);
    else if (isA(q, SetOperator))
        return serializeSetOperator(q, str);
    else
        return serializeQueryBlock(q, str);
}

/*
 * Serialize a SQL query block (SELECT ... FROM ... WHERE ...)
 */
static List *
serializeQueryBlock (QueryOperator *q, StringInfo str)
{
    QueryBlockMatch *matchInfo = NEW(QueryBlockMatch);
    StringInfo fromString = makeStringInfo();
    StringInfo whereString = makeStringInfo();
    StringInfo selectString = makeStringInfo();
    StringInfo groupByString = makeStringInfo();
    StringInfo havingString = makeStringInfo();
    MatchState state = MATCH_START;
    QueryOperator *cur = q;
    List *attrNames = getAttrNames(q->schema);
    boolean topMaterialize = HAS_STRING_PROP(cur,PROP_MATERIALIZE);

    // do the matching
    while(state != MATCH_NEXTBLOCK && cur != NULL)
    {
        DEBUG_LOG("STATE: %s", OUT_MATCH_STATE(state));
        DEBUG_LOG("Operator %s", operatorToOverviewString((Node *) cur));
        // first check that cur does not have more than one parent
        if (HAS_STRING_PROP(cur,PROP_MATERIALIZE) || LIST_LENGTH(cur->parents) > 1)
        {
            if (cur != q)
            {
                matchInfo->fromRoot = cur;
                state = MATCH_NEXTBLOCK;
                cur = OP_LCHILD(cur);
                continue;
            }
        }
        // if cur has not more than one parent and should not be materialized
        switch(cur->type)
        {
            case T_JoinOperator:
            case T_TableAccessOperator:
            case T_ConstRelOperator :
            case T_SetOperator:
            case T_JsonTableOperator:
                matchInfo->fromRoot = cur;
                state = MATCH_NEXTBLOCK;
                cur = OP_LCHILD(cur);
                continue;
                break;
            default:
                break;
        }
        switch(state)
        {
            /* START state */
            case MATCH_START:
            case MATCH_DISTINCT:
            case MATCH_ORDER:
            {
                switch(cur->type)
                {
                    case T_SelectionOperator:
                    {
                        QueryOperator *child = OP_LCHILD(cur);
                        /* HAVING */
                        if (isA(child,AggregationOperator))
                        {
                            matchInfo->having = (SelectionOperator *) cur;
                            state = MATCH_HAVING;
                        }
                        /* WHERE */
                        else
                        {
                            matchInfo->where = (SelectionOperator *) cur;
                            state = MATCH_WHERE;
                        }
                    }
                    break;
                    case T_ProjectionOperator:
                    {
                        QueryOperator *child = OP_LCHILD(cur);
                        QueryOperator *grandChild = (child ? OP_LCHILD(child) : NULL);

                        // is first projection?
                        if (isA(child,AggregationOperator)
                                || (isA(child,SelectionOperator)
                                        && isA(grandChild,AggregationOperator))
                                || (isA(child,WindowOperator)))
                        {
                            matchInfo->firstProj = (ProjectionOperator *) cur;
                            state = MATCH_FIRST_PROJ;
                        }
                        else
                        {
                            matchInfo->secondProj = (ProjectionOperator *) cur;
                            state = MATCH_SECOND_PROJ;
                        }
                    }
                    break;
                    case T_DuplicateRemoval:
                        if (state == MATCH_START || state == MATCH_ORDER)
                        {
                            matchInfo->distinct = (DuplicateRemoval *) cur;
                            state = MATCH_DISTINCT;
                        }
                        else
                        {
                            matchInfo->fromRoot = cur;
                            state = MATCH_NEXTBLOCK;
                        }
                        break;
                    case T_OrderOperator:
                        if (state == MATCH_START)
                        {
                            matchInfo->orderBy = (OrderOperator *) cur;
                            state = MATCH_ORDER;
                        }
                        else
                        {
                            matchInfo->fromRoot = cur;
                            state = MATCH_NEXTBLOCK;
                        }
                        break;
                    case T_AggregationOperator:
                        matchInfo->aggregation = (AggregationOperator *) cur;
                        state = MATCH_AGGREGATION;
                        break;
                    case T_WindowOperator:
                        matchInfo->windowRoot = (WindowOperator *) cur;
                        state = MATCH_WINDOW;
                        break;
                    default:
                        matchInfo->fromRoot = cur;
                        state = MATCH_NEXTBLOCK;
                        break;
                }
            }
            break;
            case MATCH_FIRST_PROJ:
            {
                switch(cur->type)
                {
                    case T_SelectionOperator:
                    {
                        QueryOperator *child = OP_LCHILD(cur);
                        if (child->type == T_AggregationOperator)
                        {
                            matchInfo->having = (SelectionOperator *) cur;
                            state = MATCH_HAVING;
                        }
                    }
                    break;
                    case T_AggregationOperator:
                        matchInfo->aggregation= (AggregationOperator *) cur;
                        state = MATCH_AGGREGATION;
                        break;
                    case T_WindowOperator:
                        matchInfo->windowRoot = (WindowOperator *) cur;
                        state = MATCH_WINDOW;
                        break;
                    default:
                        FATAL_LOG("After matching first projection we should "
                                "match selection or aggregation and not %s",
                                nodeToString(cur));
                    break;
                }
            }
            break;
            case MATCH_HAVING:
            {
                switch(cur->type)
                {
                    case T_AggregationOperator:
                    {
                        matchInfo->aggregation = (AggregationOperator *) cur;
                        state = MATCH_AGGREGATION;
                    }
                    break;
                    default:
                           FATAL_LOG("after matching having we should match "
                                   "aggregation and not %s", nodeToString(cur));
                    break;
                }
            }
            break;
            case MATCH_AGGREGATION:
            {
                switch(cur->type)
                {
                    case T_SelectionOperator:
                    {
                        matchInfo->where = (SelectionOperator *) cur;
                        state = MATCH_WHERE;
                    }
                    break;
                    case T_ProjectionOperator:
                        matchInfo->secondProj = (ProjectionOperator *) cur;
                        state = MATCH_SECOND_PROJ;
                        break;
                    default:
                        matchInfo->fromRoot = cur;
                        state = MATCH_NEXTBLOCK;
                    break;
                }
            }
            break;
            case MATCH_SECOND_PROJ:
            {
                switch(cur->type)
                {
                    case T_SelectionOperator:
                    {
                        matchInfo->where = (SelectionOperator *) cur;
                        state = MATCH_WHERE;
                    }
                    break;
                    default:
                        matchInfo->fromRoot = cur;
                        state = MATCH_NEXTBLOCK;
                    break;
                }
            }
            break;
            case MATCH_WHERE:
            {
                matchInfo->fromRoot = cur;
                state = MATCH_NEXTBLOCK;
            }
            break;
            case MATCH_WINDOW:
            {
                switch(cur->type)
                {
                    case T_WindowOperator:
                        // do nothing
                    break;
                    case T_SelectionOperator:
                    {
                        matchInfo->where = (SelectionOperator *) cur;
                        state = MATCH_WHERE;
                    }
                    break;
                   case T_ProjectionOperator:
                   {
                       matchInfo->secondProj = (ProjectionOperator *) cur;
                       state = MATCH_SECOND_PROJ;
                   }
                   break;
                   default:
                       matchInfo->fromRoot = cur;
                       state = MATCH_NEXTBLOCK;
                   break;
                }
            }
            break;
            case MATCH_NEXTBLOCK:
                FATAL_LOG("should not end up here because we already"
                        " have reached MATCH_NEXTBLOCK state");
                break;
        }

        // go to child of cur
        cur = OP_LCHILD(cur);
    }

    OUT_BLOCK_MATCH(DEBUG,matchInfo, "query block full match");

    // translate each clause
    DEBUG_LOG("serializeFrom");
    List *fromAttrs = NIL;
    serializeFrom(matchInfo->fromRoot, fromString, &fromAttrs);

    DEBUG_LOG("serializeWhere");
    if(matchInfo->where != NULL)
        serializeWhere(matchInfo->where, whereString, fromAttrs);

    DEBUG_LOG("serialize projection + aggregation + groupBy +  having + window functions");
    serializeProjectionAndAggregation(matchInfo, selectString, havingString,
            groupByString, fromAttrs, topMaterialize);

    // put everything together
    DEBUG_LOG("mergePartsTogether");
    //TODO DISTINCT
    if (STRINGLEN(selectString) > 0)
        appendStringInfoString(str, selectString->data);
    else
        appendStringInfoString(str, "\nSELECT *");

    appendStringInfoString(str, fromString->data);

    if (STRINGLEN(whereString) > 0)
        appendStringInfoString(str, whereString->data);

    if (STRINGLEN(groupByString) > 0)
            appendStringInfoString(str, groupByString->data);

    if (STRINGLEN(havingString) > 0)
            appendStringInfoString(str, havingString->data);

    FREE(matchInfo);

    return attrNames; //TODO return list of attribute names
}

/*
 * Translate a FROM clause
 */
static void
serializeFrom (QueryOperator *q, StringInfo from, List **fromAttrs)
{
    int curFromItem = 0, attrOffset = 0;

    appendStringInfoString(from, "\nFROM ");
    serializeFromItem (q, from, &curFromItem, &attrOffset, fromAttrs);
}

static void
ConstructNestedJsonColItems (JsonColInfoItem *col,StringInfo *from,int *nestedcount)
{
	FOREACH(JsonColInfoItem, col1, col->nested)
	{
		if (col1->nested)
		{
			(*nestedcount) ++;
			if(col1->forOrdinality)
			{
				appendStringInfoString(*from, col1->forOrdinality);
				appendStringInfoString(*from, " FOR ORDINALITY,");
			}

			appendStringInfoString(*from, " NESTED PATH");
			appendStringInfo(*from, " '%s'", col1->path);
			appendStringInfoString(*from, " COLUMNS");
			appendStringInfoString(*from, "(");

			ConstructNestedJsonColItems(col1, from, nestedcount);
		}
		else
		{
			appendStringInfo(*from, "%s", col1->attrName);
			appendStringInfo(*from, " %s", col1->attrType);

			if (col1->format)
			{
				appendStringInfoString(*from, " FORMAT");
				appendStringInfo(*from, " %s", col1->format);
			}
			if (col1->wrapper)
			{
				appendStringInfo(*from, " %s", col1->wrapper);
				appendStringInfo(*from, " WRAPPER");
			}
			appendStringInfoString(*from, " PATH");
			appendStringInfo(*from, " '%s'", col1->path);
			appendStringInfoString(*from, ",");
		}
	}
}

static void
serializeFromItem (QueryOperator *q, StringInfo from, int *curFromItem,
        int *attrOffset, List **fromAttrs)
{
    if (!(LIST_LENGTH(q->parents) > 1 || HAS_STRING_PROP(q, PROP_MATERIALIZE)))
    {
        switch(q->type)
        {
            // Join expressions
            case T_JoinOperator:
            {
                JoinOperator *j = (JoinOperator *) q;
                int rOffset;
                appendStringInfoString(from, "(");

                //left child
                serializeFromItem(OP_LCHILD(j), from, curFromItem, attrOffset, fromAttrs);

                // join
                switch(j->joinType)
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
                serializeFromItem(OP_RCHILD(j), from, curFromItem, attrOffset, fromAttrs);

                // join condition
                if (j->cond)
                    appendStringInfo(from, " ON (%s)", exprToSQLWithNamingScheme(
                            copyObject(j->cond), rOffset, *fromAttrs));

                //we don't need the alias part now
                appendStringInfo(from, ")");
            }
            break;
            // JSON TABLE OPERATOR
            case T_JsonTableOperator:
            {
            	JsonTableOperator *jt = (JsonTableOperator *) q;

            	QueryOperator *child = OP_LCHILD(jt);
            	// Serialize left child
            	serializeFromItem(child, from, curFromItem, attrOffset, fromAttrs);

            	// TODO  Get the attributes of JSON TABLE operator
            	List *jsonAttrNames = getAttrNames(((QueryOperator *) jt)->schema);

            	// Get attributes of child
            	List *childAttrNames = getAttrNames(((QueryOperator *) child)->schema);

            	// Remove childAttrNames from jsonAttrNames for
            	// updateAttributeNames to work correctly and identify the
            	// correct fromclause item
            	List *attrNames = NIL;
            	boolean flag = FALSE;

            	FOREACH(char, jsonAttr, jsonAttrNames)
            	{
            		flag = FALSE;
            		FOREACH(char, childAttr, childAttrNames)
            		{
            			if(streq(childAttr, jsonAttr))
            			{
            				flag = TRUE;
            				break;
            			}
            		}
            		if(flag == FALSE)
            		{
            			attrNames = appendToTailOfList(attrNames, jsonAttr);
            		}
            	}

            	// Add it to list of fromAttrs
            	*fromAttrs = appendToTailOfList(*fromAttrs, attrNames);

            	appendStringInfoString(from, ",");
            	appendStringInfo(from, " JSON_TABLE");
            	appendStringInfoString(from, "(");

            	// Call updateAtrrNames on jsonColumn and then serialize
            	updateAttributeNames((Node*)jt->jsonColumn, *fromAttrs);

            	appendStringInfo(from, exprToSQL((Node*)jt->jsonColumn));
            	appendStringInfoString(from, ",");
            	appendStringInfo(from, " '%s'", jt->documentcontext);
            	appendStringInfoString(from, " COLUMNS");
            	appendStringInfoString(from, "(");

            	int nestedcount = 0;
            	FOREACH(JsonColInfoItem, col, jt->columns)
            	{
            		if (col->nested)
            		{
            			if(col->forOrdinality)
            			{
            				appendStringInfoString(from, col->forOrdinality);
            				appendStringInfoString(from, " FOR ORDINALITY, ");
            			}

            			if (nestedcount++ > 0)
            				appendStringInfoString(from, ",");

            			appendStringInfoString(from, " NESTED PATH");
            			appendStringInfo(from, " '%s'", col->path);
            			appendStringInfoString(from, " COLUMNS");
            			appendStringInfoString(from, "(");

            			ConstructNestedJsonColItems (col,&from,&nestedcount);

            		}
            		else
            		{
            			appendStringInfo(from, "%s", col->attrName);
            			appendStringInfo(from, " %s", col->attrType);

            			if (col->format)
            			{
            				appendStringInfoString(from, " FORMAT");
            				appendStringInfo(from, " %s", col->format);
            			}
            			if (col->wrapper)
            			{
            				appendStringInfo(from, " %s", col->wrapper);
            				appendStringInfo(from, " WRAPPER");
            			}
            			appendStringInfoString(from, " PATH");
            			appendStringInfo(from, " '%s'", col->path);
            			appendStringInfoString(from, ",");
            		}
            	}

            	// Remove the last unnecessary comma
            	from->data[from->len - 1] = ' ';
            	appendStringInfoString(from, ")");
            	appendStringInfoString(from, ")");

            	for(int i=0; i<nestedcount; i++)
            		appendStringInfoString(from, ")");
            	appendStringInfoString(from, " AS ");
            	appendStringInfo(from, "F%u", (*curFromItem)++);
            }
            break;
            // Table Access
            case T_TableAccessOperator:
            {
            	TableAccessOperator *t = (TableAccessOperator *) q;
            	char *asOf = NULL;
            	//                ProvenanceComputation *op;
            	//                Constant *xid = getTranactionSQLAndSCNs(xid);
            	// use history join to prefilter updated rows
            	if (HAS_STRING_PROP(t, PROP_USE_HISTORY_JOIN))
            	{
            		List *scnsAndXid = (List *) GET_STRING_PROP(t, PROP_USE_HISTORY_JOIN);
            		Constant *startScn, *commitScn, *commitMinusOne;
            		Constant *xid;
            		StringInfo attrNameStr = makeStringInfo();
            		List *attrNames = getAttrNames(((QueryOperator *) t)->schema);
            		int i = 0;

            		xid = (Constant *) getNthOfListP(scnsAndXid,0);
            		startScn = (Constant *) getNthOfListP(scnsAndXid,1);
            		commitScn = (Constant *) getNthOfListP(scnsAndXid,2);
            		commitMinusOne = createConstLong(LONG_VALUE(commitScn) - 1);

            		FOREACH(char,a,attrNames)
            		{
            			appendStringInfo(attrNameStr, "%s%s",
            					(i++ == 0) ? "" : ", ",
            							a);
            			// append to string info
            			// appendStringInfo(from, "%s %s %s %s", a, attrNames);

            		}

                    // read committed?
                    if (HAS_STRING_PROP(t, PROP_IS_READ_COMMITTED))
                    {
                        appendStringInfo(from, "(SELECT %s \nFROM\n", attrNameStr->data);
                        appendStringInfo(from, "\t(SELECT ROWID AS rid , %s", attrNameStr->data);
                        appendStringInfo(from, "\tFROM %s VERSIONS BETWEEN SCN %u AND %u) F0",t->tableName,
                                LONG_VALUE(commitMinusOne),
                                LONG_VALUE(commitMinusOne));
                        appendStringInfoString(from, "\n JOIN ");
                        appendStringInfo(from, "\t(SELECT ROWID AS rid FROM %s VERSIONS BETWEEN SCN %u AND %u F1 ",
                                t->tableName, LONG_VALUE(commitScn), LONG_VALUE(commitScn));
                        appendStringInfo(from, "WHERE VERSIONS_XID = HEXTORAW('%s')) F1", STRING_VALUE(xid));
                        appendStringInfo(from, " ON (F0.rid = F1.rid)) F%u",  (*curFromItem)++);
                    }
                    else
                    {
                        appendStringInfo(from, "(SELECT %s \nFROM\n", attrNameStr->data);
                        appendStringInfo(from, "\t(SELECT ROWID AS rid , %s", attrNameStr->data);
                        appendStringInfo(from, "\tFROM %s AS OF SCN %u) F0",t->tableName, LONG_VALUE(startScn));
                        appendStringInfoString(from, "\n JOIN ");
                        appendStringInfo(from, "\t(SELECT ROWID AS rid FROM %s VERSIONS BETWEEN SCN %u AND %u F1 ",
                                t->tableName, LONG_VALUE(commitScn), LONG_VALUE(commitScn));
                        appendStringInfo(from, "WHERE VERSIONS_XID = HEXTORAW('%s')) F1", STRING_VALUE(xid));
                        appendStringInfo(from, " ON (F0.rid = F1.rid)) F%u",  (*curFromItem)++);
                    }

                    *fromAttrs = appendToTailOfList(*fromAttrs, attrNames);
                }
                else
                {
                    // add list of attributes as list to fromAttrs
                    *attrOffset = 0;
                    if (t->asOf)
                    {
                        if (isA(t->asOf, Constant))
                        {
                            Constant *c = (Constant *) t->asOf;
                            if (c->constType == DT_LONG)
                                asOf = CONCAT_STRINGS(" AS OF SCN ", exprToSQL(t->asOf));
                            else
                                asOf = CONCAT_STRINGS(" AS OF TIMESTAMP to_timestamp(", exprToSQL(t->asOf), ")");
                        }
                        else
                        {
                            List *scns = (List *) t->asOf;
                            Node *begin = (Node *) getNthOfListP(scns, 0);
                            Node *end = (Node *) getNthOfListP(scns, 1);

                            asOf = CONCAT_STRINGS(" VERSIONS BETWEEN SCN ", exprToSQL(begin), " AND ", exprToSQL(end));
                        }
                    }

                    List *attrNames = getAttrNames(((QueryOperator *) t)->schema);
                    *fromAttrs = appendToTailOfList(*fromAttrs, attrNames);
                    appendStringInfo(from, "(%s%s F%u)", quoteIdentifierOracle(t->tableName),
                            asOf ? asOf : "", (*curFromItem)++);
                }
            }
            break;
            // A constant relation, turn into (SELECT ... FROM dual) subquery
            case T_ConstRelOperator:
                   {
                       ConstRelOperator *t = (ConstRelOperator *) q;
                       int pos = 0;
                       List *attrNames = getAttrNames(((QueryOperator *) t)->schema);

                       *fromAttrs = appendToTailOfList(*fromAttrs, attrNames);

                       appendStringInfoString(from, "(SELECT ");
                       FOREACH(char,attrName,attrNames)
                       {
                           Node *value;
                           if (pos != 0)
                               appendStringInfoString(from, ", ");
                           value = getNthOfListP(t->values, pos++);
                           appendStringInfo(from, "%s AS %s", exprToSQL(value), attrName);

                       }
                       appendStringInfo(from, "\nFROM dual) F%u", (*curFromItem)++);
                   }
            break;
            default:
            {
                List *attrNames;

                appendStringInfoString(from, "((");
                attrNames = serializeQueryOperator(q, from);
                *fromAttrs = appendToTailOfList(*fromAttrs, attrNames);
                appendStringInfo(from, ") F%u)", (*curFromItem)++);
            }
            break;
        }
    }
    else
    {
        // A materialization point or WITH
        {
            List *attrNames;

            appendStringInfoString(from, "((");
            attrNames = serializeQueryOperator(q, from);
            *fromAttrs = appendToTailOfList(*fromAttrs, attrNames);
            appendStringInfo(from, ") F%u)", (*curFromItem)++);
        }
    }
}

static char *
exprToSQLWithNamingScheme (Node *expr, int rOffset, List *fromAttrs)
{
    JoinAttrRenameState *state = NEW(JoinAttrRenameState);

    state->rightFromOffsets = rOffset;
    state->fromAttrs = fromAttrs;
    renameAttrsVisitor(expr, state);

    FREE(state);
    return exprToSQL(expr);
}

static boolean
renameAttrsVisitor (Node *node, JoinAttrRenameState *state)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, AttributeReference))
    {
        AttributeReference *a = (AttributeReference *) node;
        boolean isRight = (a->fromClauseItem == 0) ? FALSE : TRUE;
        int pos = 0, fPos = 0;
        int rOffset = state->rightFromOffsets;
        ListCell *lc;
        char *name;
        List *from = NIL;

        // if right join input find first from item from right input
        if (isRight)
            for(lc = getHeadOfList(state->fromAttrs); fPos < rOffset; lc = lc->next, fPos++)
                ;
        else
            lc = getHeadOfList(state->fromAttrs);

        // find from position and attr name
        for(; lc != NULL; lc = lc->next)
        {
            List *attrs = (List *) LC_P_VAL(lc);
            pos += LIST_LENGTH(attrs);
            if (pos > a->attrPosition)
            {
                from = attrs;
                break;
            }
            fPos++;
        }

        pos = a->attrPosition - pos + LIST_LENGTH(from);
        name = getNthOfListP(from, pos);

        a->name = createAttrName(name, fPos);

        return TRUE;
    }

    return visit(node, renameAttrsVisitor, state);
}

//static char *
//createFromNames (int *attrOffset, int count)
//{
//    char *result = NULL;
//    StringInfo str = makeStringInfo();
//
//    for(int i = *attrOffset; i < count + *attrOffset; i++)
//        appendStringInfo(str, "%sA%u", (i == *attrOffset)
//                ? "" : ", ", i);
//
//    *attrOffset += count;
//    result = str->data;
//    FREE(str);
//
//    return result;
//}

static char *
createAttrName (char *name, int fItem)
{
   StringInfo str = makeStringInfo();
   char *result = NULL;

   appendStringInfo(str, "F%u.%s", fItem, name);
   result = str->data;
   FREE(str);

   return result;
}

/*
 * Translate a selection into a WHERE clause
 */
static void
serializeWhere (SelectionOperator *q, StringInfo where, List *fromAttrs)
{
    appendStringInfoString(where, "\nWHERE ");
    updateAttributeNames((Node *) q->cond, (List *) fromAttrs);
    appendStringInfoString(where, exprToSQL(q->cond));
}

static boolean
updateAttributeNames(Node *node, List *fromAttrs)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, AttributeReference))
    {
        AttributeReference *a = (AttributeReference *) node;
        char *newName;
        List *outer = NIL;
        int fromItem = -1;
        int attrPos = 0;

        // LOOP THROUGH fromItems (outer list)
        FOREACH(List, attrs, fromAttrs)
        {
            attrPos += LIST_LENGTH(attrs);
            fromItem++;
            if (attrPos > a->attrPosition)
            {
                outer = attrs;
                break;
            }
        }
        attrPos = a->attrPosition - attrPos + LIST_LENGTH(outer);
        newName = getNthOfListP(outer, attrPos);
        a->name = CONCAT_STRINGS("F", itoa(fromItem), ".", newName);;
    }

    return visit(node, updateAttributeNames, fromAttrs);
}

static boolean
updateAttributeNamesSimple(Node *node, List *attrNames)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, AttributeReference))
    {
        AttributeReference *a = (AttributeReference *) node;
        char *newName = getNthOfListP(attrNames, a->attrPosition);
        a->name = strdup(newName);
    }

    return visit(node, updateAttributeNamesSimple, attrNames);
}

static boolean
updateAggsAndGroupByAttrs(Node *node, UpdateAggAndGroupByAttrState *state)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, AttributeReference))
    {
        AttributeReference *a = (AttributeReference *) node;
        char *newName;
        int attrPos = a->attrPosition;

        // is aggregation function
        if (attrPos < LIST_LENGTH(state->aggNames))
            newName = strdup(getNthOfListP(state->aggNames, attrPos));
        else
        {
            attrPos -= LIST_LENGTH(state->aggNames);
            newName = strdup(getNthOfListP(state->groupByNames, attrPos));
        }
        DEBUG_LOG("attr <%d> is <%s>", a->attrPosition, newName);

        a->name = newName;
    }

    return visit(node, updateAggsAndGroupByAttrs, state);
}

#define UPDATE_ATTR_NAME(cond,expr,falseAttrs,trueAttrs) \
    do { \
        Node *_localExpr = (Node *) (expr); \
        if (m->secondProj == NULL) \
            updateAttributeNames(_localExpr, falseAttrs); \
        else \
            updateAttributeNamesSimple(_localExpr, trueAttrs); \
    } while(0)

/*
 * Create the SELECT, GROUP BY, and HAVING clause
 */
static List *
serializeProjectionAndAggregation (QueryBlockMatch *m, StringInfo select,
        StringInfo having, StringInfo groupBy, List *fromAttrs, boolean materialize)
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
            updateAttributeNames(n, fromAttrs);
            firstProjs = appendToTailOfList(firstProjs, exprToSQL(n));
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
            UPDATE_ATTR_NAME((m->secondProj == NULL), expr, fromAttrs, firstProjs);
//            if (m->secondProj == NULL)
//                updateAttributeNames(expr, fromAttrs);
//            else
//                updateAttributeNamesSimple(expr, firstProjs);
            aggs = appendToTailOfList(aggs, exprToSQL(expr));
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

            UPDATE_ATTR_NAME((m->secondProj == NULL), expr, fromAttrs, firstProjs);
//            if (m->secondProj == NULL)
//                updateAttributeNames(expr, fromAttrs);
//            else
//                updateAttributeNamesSimple(expr, firstProjs);
            g = exprToSQL(expr);

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

            DEBUG_LOG("BEFORE: window function = %s", exprToSQL((Node *) winOpGetFunc(
                                (WindowOperator *) curOp)));

            UPDATE_ATTR_NAME((m->secondProj == NULL), expr, fromAttrs, firstProjs);
//            if (m->secondProj == NULL)
//                updateAttributeNames(expr, fromAttrs);
//            else
//                updateAttributeNamesSimple(expr, firstProjs);
            UPDATE_ATTR_NAME((m->secondProj == NULL), wOp->partitionBy, fromAttrs, firstProjs);
            UPDATE_ATTR_NAME((m->secondProj == NULL), wOp->orderBy, fromAttrs, firstProjs);
            UPDATE_ATTR_NAME((m->secondProj == NULL), wOp->frameDef, fromAttrs, firstProjs);

            windowFs = appendToHeadOfList(windowFs, exprToSQL((Node *) winOpGetFunc(
                    (WindowOperator *) curOp)));

            DEBUG_LOG("AFTER: window function = %s", exprToSQL((Node *) winOpGetFunc(
                    (WindowOperator *) curOp)));

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
        appendStringInfo(having, "\nHAVING %s", exprToSQL(sel->cond));
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
                updateAttributeNames(a, fromAttrs);
            appendStringInfo(select, "%s%s", exprToSQL(a), attrName ? CONCAT_STRINGS(" AS ", attrName) : "");
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
        resultAttrs = getQueryOperatorAttrNames(m->fromRoot);//TODO

        FOREACH(char,name,resultAttrs)
        {
            if (pos++ != 0)
                appendStringInfoString(select, ", ");
            appendStringInfoString(select, name);
        }

        DEBUG_LOG("FROM root attributes as projection expressions %s", select->data);
    }

    if (state)
        FREE(state);

    return resultAttrs;
}


/*
 * Serialize a set operation UNION/MINUS/INTERSECT
 */
static List *
serializeSetOperator (QueryOperator *q, StringInfo str)
{
    SetOperator *setOp = (SetOperator *) q;
    List *resultAttrs;

    // output left child
    WITH_PARENS(str,resultAttrs = serializeQueryOperator(OP_LCHILD(q), str));

    // output set operation
    switch(setOp->setOpType)
    {
        case SETOP_UNION:
            appendStringInfoString(str, " UNION ALL ");
            break;
        case SETOP_INTERSECTION:
            appendStringInfoString(str, " INTERSECT ");
            break;
        case SETOP_DIFFERENCE:
            appendStringInfoString(str, " MINUS ");
            break;
    }

    // output right child
    WITH_PARENS(str,serializeQueryOperator(OP_RCHILD(q), str));

    return resultAttrs;
}

/*
 * Create a temporary view
 */
static List *
createTempView (QueryOperator *q, StringInfo str)
{
    StringInfo viewDef = makeStringInfo();
    char *viewName = createViewName();
    TemporaryViewMap *view;
    List *resultAttrs;

    // check whether we already have create a view for this op
    HASH_FIND_PTR(viewMap, &q, view);
    if (view != NULL)
    {
        appendStringInfoString(str, strdup(view->viewName));
        return deepCopyStringList(view->attrNames);
    }

    // create sql code to create view
    appendStringInfo(viewDef, "%s AS (", viewName);
    if (isA(q, SetOperator))
        resultAttrs = serializeSetOperator(q, viewDef);
    else
        resultAttrs = serializeQueryBlock(q, viewDef);

    appendStringInfoString(viewDef, ")");

    DEBUG_LOG("created view definition:\n%s", viewDef->data);

    // add reference to view
    appendStringInfoString(str, strdup(viewName));

    // add to view table
    view = NEW(TemporaryViewMap);
    view->viewName = viewName;
    view->viewOp = q;
    view->viewDefinition = viewDef->data;
    view->attrNames = resultAttrs;
    HASH_ADD_PTR(viewMap, viewOp, view);

    return resultAttrs;
}

static char *
createViewName (void)
{
    StringInfo str = makeStringInfo();

    appendStringInfo(str, "temp_view_of_%u", viewNameCounter++);

    return str->data;
}

/*
 * quote identifier if necessary
 */
char *
quoteIdentifierOracle (char *ident)
{
    int i = 0;
    boolean needsQuotes = FALSE;

    // already quoted
    if (ident[0] == '"')
        return ident;

    if (!isupper(ident[0]))
        needsQuotes = TRUE;

    for(i = 0; i < strlen(ident); i++)
    {
        switch(ident[i])
        {
            case '$':
            case '#':
            case '_':
                break;
            default:
                if (!isupper(ident[i]))
                    needsQuotes = TRUE;
                break;
        }
        if (needsQuotes)
            break;
    }

    if (needsQuotes)
        ident = CONCAT_STRINGS("\"",ident,"\"");

    return ident;
}

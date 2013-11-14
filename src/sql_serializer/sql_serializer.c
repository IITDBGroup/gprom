/*-----------------------------------------------------------------------------
 *
 * sql_serializer.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "sql_serializer/sql_serializer.h"
#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"
#include "model/list/list.h"



/* data structures */
typedef enum MatchState {
    MATCH_START,
    MATCH_DISTINCT,
    MATCH_FIRST_PROJ,
    MATCH_HAVING,
    MATCH_AGGREGATION,
    MATCH_SECOND_PROJ,
    MATCH_WHERE,
    MATCH_NEXTBLOCK
} MatchState;

typedef struct QueryBlockMatch {
    QueryOperator *firstProj;
    QueryOperator *having;
    QueryOperator *aggregation;
    QueryOperator *secondProj;
    QueryOperator *where;
    QueryOperator *fromRoot;
} QueryBlockMatch;

typedef struct TemporaryViewMap {
    QueryOperator *viewOp; // the key
    char *viewName;
    char *viewDefinition;
    UT_hash_handle hh;
} TemporaryViewMap;

/* macros */
#define OPEN_PARENS(str) appendStringInfoChar(str, '(')
#define CLOSE_PARENS(str) appendStringInfoChar(str, ')')
#define WITH_PARENS(str,operation) \
    do { \
        OPEN_PARENS(str); \
        operation; \
        CLOSE_PARENS(str); \
    } while(0)

/* variables */
static TemporaryViewMap *viewMap;
static int viewNameCounter;

/* method declarations */
static void serializeQueryOperator (QueryOperator *q, StringInfo str);
static void serializeQueryBlock (QueryOperator *q, StringInfo str);
static void serializeSetOperator (QueryOperator *q, StringInfo str);

static void serializeFrom (QueryOperator *q, StringInfo from);
static void serializeWhere (QueryOperator *q, StringInfo where);
static void serializeSelect (QueryOperator *q, StringInfo select);

static void createTempView (QueryOperator *q, StringInfo str);
static char *createViewName (void);

char *
serializeQuery(QueryOperator *q)
{
    StringInfo str = makeStringInfo();
    MemContext *memC = NEW_MEM_CONTEXT("SQL_SERIALZIER");
    ACQUIRE_MEM_CONTEXT(memC);

    // initialize basic structures and then call the worker
    viewMap = NULL;
    viewNameCounter = 0;

    // call main entry point for translation
    serializeQueryOperator (q, str);

    /*
     *  prepend the temporary view definition to create something like
     *      WITH a AS (q1), b AS (q2) ... SELECT ...
     */
    if (HASH_COUNT(viewMap) > 0)
    {
        StringInfo viewDef = makeStringInfo();
        appendStringInfoString(viewDef, "WITH ");

        // loop through temporary views we have defined
        for(TemporaryViewMap *view = viewMap; view != NULL; view = view->hh.next)
        {
            appendStringInfoString(viewDef, view->viewDefinition);
            if (view->hh.next != NULL)
                appendStringInfoString(str, ",\n\n");
        }

        // prepend to query translation
        appendStringInfoString(str, "\n\n");
        prependStringInfo(str, "%s", viewDef->data);
    }

    // copy result to callers memory context and clean up
    RELEASE_MEM_CONTEXT();
    char *result = strdup(str->data);

    FREE_MEM_CONTEXT(memC);

    return result;
}

/*
 * Main entry point for serialization.
 */
static void
serializeQueryOperator (QueryOperator *q, StringInfo str)
{
    // operator with multiple parents
    if (LIST_LENGTH(q->parents) > 1)
        createTempView (q, str);
    else if (isA(q, SetOperator))
        serializeSetOperator(q, str);
    else
        serializeQueryBlock(q, str);
}

/*
 * Serialize a SQL query block (SELECT ... FROM ... WHERE ...)
 */
static void
serializeQueryBlock (QueryOperator *q, StringInfo str)
{
    QueryBlockMatch *matchInfo = NEW(QueryBlockMatch);
    StringInfo fromString = makeStringInfo();
    StringInfo whereString = makeStringInfo();
    StringInfo selectString = makeStringInfo();
    MatchState state = MATCH_START;
    QueryOperator *cur = q;

    // do the matching
    while(state != MATCH_NEXTBLOCK && cur != NULL)
    {
        // first check that cur does not have more than one parent

        switch(state)
        {
            /* START state */
            case MATCH_START:
                {
                    switch(cur->type)
                    {
                        case T_SelectionOperator:
                        {
                            QueryOperator *child = OP_LCHILD(cur);
                            if (child->type == T_AggregationOperator)
                            {

                            }
                            /* WHERE */
                            else
                            {
                                matchInfo->where = cur;
                                state = MATCH_WHERE;
                            }
                        }
                            break;
                        case T_JoinOperator:
                            matchInfo->fromRoot = cur;
                            state = MATCH_NEXTBLOCK;
                            break;
                        default: //TODO add other cases
                            break;
                    }
                }
                break;
            case MATCH_DISTINCT:
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
                                matchInfo->having = cur;
                                state = MATCH_HAVING;
                            }  
                        }
                            break;

                        case T_JoinOperator:
                            matchInfo->firstProj = cur;
                            state = MATCH_NEXTBLOCK;
                            break;
                        default: //TODO add other cases
                            break;
                    }
                }
                break;
             case MATCH_HAVING:
                {
                    switch(cur->type)
                    {
                        case T_SelectionOperator:
                        {
                            QueryOperator *child = OP_LCHILD(cur);
                            if (child->type == T_AggregationOperator)
                            {
                                matchInfo->aggregation = cur;
                                state = MATCH_AGGREGATION;
                            }  
                        }
                            break;
                        default: //TODO add other cases
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
                            QueryOperator *child = OP_LCHILD(cur);
                            if (child->type == T_AggregationOperator)
                            {
                                matchInfo->where = cur;
                                state = MATCH_WHERE;
                            }
                        }
                            break;
                        case T_JoinOperator:
                            matchInfo->aggregation = cur;
                            state = MATCH_NEXTBLOCK;
                            break;
                       
                        default: //TODO add other cases
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
                            QueryOperator *child = OP_LCHILD(cur);
                            if (child->type == T_AggregationOperator)
                            {
                                matchInfo->where = cur;
                                state = MATCH_WHERE;
                            }
                        }
                            break;
                        case T_JoinOperator:
                            matchInfo->secondProj = cur;
                            state = MATCH_NEXTBLOCK;
                            break;
                       
                        default: //TODO add other cases
                            break;
                    }
                }
                break;
             case MATCH_WHERE:
                {
                    switch(cur->type)
                    {
                        case T_SelectionOperator:
                        {
                            QueryOperator *child = OP_LCHILD(cur);
                            if (child->type == T_AggregationOperator)
                            {
                                matchInfo->where = cur;
                                state = MATCH_NEXTBLOCK;
                            }
                        }
                            break;
                        case T_JoinOperator:
                            matchInfo->where = cur;
                            state = MATCH_NEXTBLOCK;
                            break;
                       
                        default: //TODO add other cases
                            break;
                    }
                }
                break;
            default: //TODO remove once all cases are handled
                break;
        }

        // go to child of cur
        cur = OP_LCHILD(cur);
    }

    // translate FROM
    serializeFrom(matchInfo->fromRoot, fromString);

    if(matchInfo->where != NULL)
        serializeWhere(matchInfo->where, whereString);

    if(matchInfo->secondProj != NULL)
        serializeSelect(matchInfo->secondProj, selectString);
    else
        appendStringInfoString(selectString, "*");

    // put everything together
    appendStringInfoString(str, "\nSELECT ");
    appendStringInfoString(str, selectString->data);

    appendStringInfoString(str, "\nFROM ");
    appendStringInfoString(str, fromString->data);

    appendStringInfoString(str, "\nWHERE ");
    appendStringInfoString(str, whereString->data);


    FREE(matchInfo);
}

/*
 * Translate a FROM clause
 */
static void
serializeFrom (QueryOperator *q, StringInfo from)
{
    appendStringInfoString(str, "\nFROM ");
    appendStringInfoString(str, fromString->data);
}

/*
 * Translate a selection into a WHERE clause
 */
static void
serializeWhere (QueryOperator *q, StringInfo where)
{
    if(matchInfo->where !=NULL)
    serializeWhere(matchInfo->where, whereString);
    appendStringInfoString(str, "\nWHERE ");
    appendStringInfoString(str, whereString->data);
}

/*
 * Create the SELECT clause
 */
static void
serializeSelect (QueryOperator *q, StringInfo select)
{
    if(matchInfo->aggregation !=NULL)
    serializeSelect(matchInfo->aggregation, selectString);
    appendStringInfoString(str, "\nSELECT ");
    appendStringInfoString(str, selectString->data);
}


/*
 * Serialize a set operation UNION/MINUS/INTERSECT
 */
static void
serializeSetOperator (QueryOperator *q, StringInfo str)
{
    SetOperator *setOp = (SetOperator *) q;

    // output left child
    WITH_PARENS(str,serializeQueryOperator(OP_LCHILD(q), str));

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
}

/*
 * Create a temporary view
 */
static void
createTempView (QueryOperator *q, StringInfo str)
{
    StringInfo viewDef = makeStringInfo();
    char *viewName = createViewName();
    TemporaryViewMap *view;

    // create sql code to create view
    appendStringInfo(viewDef, "%s AS (", viewName);
    serializeQueryOperator(q, viewDef);
    appendStringInfoString(viewDef, ")\n\n");

    // add to view table
    view = NEW(TemporaryViewMap);
    view->viewName = viewName;
    view->viewOp = q;
    view->viewDefinition = viewDef->data;
    HASH_ADD_PTR(viewMap, viewName, view);
}

static char *
createViewName (void)
{
    StringInfo str = makeStringInfo();

    appendStringInfo(str, "temp_view_of_%u", viewNameCounter++);

    return str->data;
}

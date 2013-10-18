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
typedef struct QueryBlockMatch {
    Node *firstProj;
    Node *having;
    Node *aggregation;
    Node *secondProj;
    Node *where;
    Node *fromRoot;
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
    StringInfo fromString;

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
    appendStringInfo(viewDef, "WITH %s AS (", viewName);
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

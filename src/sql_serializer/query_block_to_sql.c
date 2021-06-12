/*-----------------------------------------------------------------------------
 *
 * query_block_to_sql.c
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "log/logger.h"

#include "sql_serializer/query_block_to_sql.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "model/expression/expression.h"

// functions
static void nodeToSQL(StringInfo str, Node *node);
static void blockToSQL(StringInfo str, QueryBlock *q);
static void setToSQL(StringInfo str, SetQuery *s);
static void fromToSQL(StringInfo str, List *from);
static void fromItemToSQL(StringInfo str, FromItem *f);

// create SQL code from query block model
char *
qbToSQL (Node *node)
{
    StringInfo str = makeStringInfo();

    nodeToSQL(str, node);

    return str->data;
}

static void
nodeToSQL(StringInfo str, Node *node)
{
    switch(node->type)
    {
        case T_QueryBlock:
            blockToSQL(str, (QueryBlock *) node);
            break;
        case T_SetQuery:
            setToSQL(str, (SetQuery *) node);
            break;
        default:
            FATAL_LOG("node type not supports: %u", node->type);
    }
}


static void
blockToSQL(StringInfo str, QueryBlock *q)
{
    appendStringInfo(str, "SELECT %s", exprToSQL((Node *) q->selectClause, NULL, FALSE));
    if (q->fromClause)
        fromToSQL(str, q->fromClause);
    if (q->whereClause)
        appendStringInfo(str, " WHERE %s", exprToSQL(q->whereClause, NULL, FALSE));
    if (q->groupByClause)
        appendStringInfo(str, " GROUP BY %s", exprToSQL((Node *) q->groupByClause, NULL, FALSE));
    if (q->havingClause)
        appendStringInfo(str, " HAVING %s", exprToSQL(q->havingClause, NULL, FALSE));
    if (q->orderByClause)
        appendStringInfo(str, " ORDER BY %s", exprToSQL((Node *) q->orderByClause, NULL, FALSE));
}

static void
setToSQL(StringInfo str, SetQuery *s)
{
    appendStringInfoString(str, "(");
    nodeToSQL(str, s->lChild);
    appendStringInfoString(str, ")");

    switch(s->setOp)
    {
        case SETOP_UNION:
            appendStringInfo(str, " UNION %s", s->all ? " ALL " : "");
        break;
        case SETOP_INTERSECTION:
            appendStringInfo(str, " INTERSECT %s", s->all ? " ALL " : "");
        break;
        case SETOP_DIFFERENCE:
            appendStringInfo(str, " MINUS %s", s->all ? " ALL " : "");
        break;
    }

    appendStringInfoString(str, "(");
    nodeToSQL(str, s->lChild);
    appendStringInfoString(str, ")");
}

static void
fromToSQL(StringInfo str, List *from)
{
    appendStringInfoString(str, " FROM ");
    FOREACH(FromItem,f, from)
        fromItemToSQL(str, f);
}

static void
fromItemToSQL(StringInfo str, FromItem *f)
{
    switch(f->type)
    {
        case T_FromTableRef:
        {
            FromTableRef *r = (FromTableRef *) f;
            appendStringInfo(str, "%s", r->tableId);
        }
        break;
        case T_FromSubquery:
        {
            FromSubquery *q = (FromSubquery *) f;

            appendStringInfoString(str, "(");
            nodeToSQL(str, q->subquery);
            appendStringInfoString(str, ")");
        }
        break;
        default:
            FATAL_LOG("not supported yet: %u", f->type);
    }
    if (f->name)
        appendStringInfo(str, " %s", f->name);
}

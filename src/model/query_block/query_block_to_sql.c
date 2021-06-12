/*
 *------------------------------------------------------------------------------
 *
 * query_block_to_sql.c - Translate a query block model to SQL text.
 *
 *     Functions to translate the query block representation of a query produced
 *     by the parser into SQL text.
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2021-04-04
 *        SUBDIR: src/model/query_block/
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "log/logger.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"

static void parseBackQueryBlockInternal(Node *stmt, StringInfo str);
static void parseBackFromItem(FromItem *f, StringInfo str);


char *
parseBackQueryBlock(Node *stmt)
{
	StringInfo result = makeStringInfo();

	parseBackQueryBlockInternal(stmt, result);

    DEBUG_LOG("RESULT OF PARSING BACK QUERY IS:\n%s", result->data);

	return result->data;
}

static void
parseBackQueryBlockInternal(Node *stmt, StringInfo str)
{
    switch(stmt->type)
    {
	case T_QueryBlock:
	{
		QueryBlock *qb = (QueryBlock *) stmt;

		// SELECT clause
		appendStringInfoString(str, "SELECT ");
		FOREACH(SelectItem,s,qb->selectClause)
		{
			appendStringInfo(str, "%s%s",
							 exprToSQL(s->expr, NULL, FALSE),
							 s->alias ? CONCAT_STRINGS(" AS ", s->alias) : "");
			if(FOREACH_HAS_MORE(s))
			{
				appendStringInfoString(str, ", ");
			}
		}
		// FROM clause
		if (qb->fromClause)
		{
			appendStringInfoString(str, "\nFROM ");
			FOREACH(FromItem,f,qb->fromClause)
			{
				parseBackFromItem(f, str);
			}
		}
		// WHERE clause
		if (qb->whereClause)
		{
			appendStringInfo(str, "\nWHERE %s ", exprToSQL(qb->whereClause, NULL, FALSE)); //TODO deal with nested subqueries
		}
		// GROUP BY clause
		if (qb->groupByClause)
		{
			appendStringInfoString(str, "\nGROUP BY ");
			FOREACH(Node,gb,qb->groupByClause)
			{
				appendStringInfo(str, "%s", exprToSQL(gb, NULL, FALSE));
				if(FOREACH_HAS_MORE(gb))
				{
					appendStringInfoString(str, ", ");
				}
			}
		}
		// HAVING clause
		if (qb->havingClause)
		{
			appendStringInfo(str, "\nHAVING %s", exprToSQL(qb->havingClause, NULL, FALSE));
		}
		// ORDER BY clause
		if (qb->orderByClause)
		{
			appendStringInfoString(str, "ORDER BY ");
			FOREACH(Node,o,qb->orderByClause)
			{
				appendStringInfo(str, "%s", exprToSQL(o, NULL, FALSE));
				if(FOREACH_HAS_MORE(o))
				{
					appendStringInfoString(str, ", ");
				}
			}
		}
		// LIMIT clause
		if (qb->limitClause)
		{
			appendStringInfo(str, "\nLIMIT %s", exprToSQL(qb->limitClause, NULL, FALSE));
		}
		// OFFSET clause
		if (qb->offsetClause)
		{
			appendStringInfo(str, "\nOFFSET %s", exprToSQL(qb->offsetClause, NULL, FALSE));
		}
	}
	break;
	case T_SetQuery:
	{
		SetQuery *s = (SetQuery *) stmt;

		parseBackQueryBlockInternal(s->lChild, str);

		// output set operation
		switch(s->setOp)
		{
        case SETOP_UNION:
            appendStringInfoString(str, " UNION ");
            break;
        case SETOP_INTERSECTION:
            appendStringInfoString(str, " INTERSECT ");
            break;
        case SETOP_DIFFERENCE:
            appendStringInfoString(str, " EXCEPT ");
            break;
		}

		if (s->all)
		{
			appendStringInfoString(str, "ALL ");
		}

		parseBackQueryBlockInternal(s->rChild, str);
	}
	break;
	case T_ProvenanceStmt: //TODO
		break;
	case T_List:
		break;
	case T_Insert:
	{
		Insert *i = (Insert *) stmt;

		appendStringInfo(str, "INSERT INTO %s ",
						 i->insertTableName);

		if(i->attrList)
		{
			appendStringInfoString(str, "(");
			FOREACH(char,a,i->attrList)
			{
				appendStringInfoString(str, a);
				if(FOREACH_HAS_MORE(a))
				{
					appendStringInfoString(str, ",");
				}
			}
			appendStringInfoString(str, ")");
		}

		// VALUES
		if(isA(i->query, List))
		{
			appendStringInfoString(str, " VALUES (");
			FOREACH(Node,n,(List *) i->query)
			{
				appendStringInfo(str, "%s", exprToSQL(n, NULL, FALSE));
				if(FOREACH_HAS_MORE(n))
				{
					appendStringInfoString(str, ",");
				}
			}
			appendStringInfoString(str, ")");
		}
		// INSERT INTO SELECT ...
		else
		{
			parseBackQueryBlockInternal((Node *) i->query, str);
		}
	}
	break;
	case T_Delete:
	{
		Delete *d = (Delete *) stmt;

		appendStringInfo(str, "DELETE FROM %s WHERE %s",
						 d->deleteTableName,
						 d->cond);
	}
	break;
	case T_Update:
	{
		Update *u = (Update *) stmt;

		appendStringInfo(str, "UPDATE %s SET ", u->updateTableName);
		FOREACH(Node,n,u->selectClause)
		{
			appendStringInfo(str, "%s", exprToSQL(n, NULL, FALSE));
			if(FOREACH_HAS_MORE(n))
			{
				appendStringInfoString(str, ",");
			}
		}
		if (u->cond)
		{
			appendStringInfo(str, " WHERE %s", exprToSQL(u->cond, NULL, FALSE));
		}
	}
	break;
	case T_WithStmt:
		break;
	case T_CreateTable:
		break;
	case T_AlterTable:
		break;
	case T_ExecQuery:
		break;
	case T_PreparedQuery:
		break;
	default:
		break;
    }

}

static void
parseBackFromItem(FromItem *f, StringInfo str)
{
	switch(f->type)
	{
	case T_FromTableRef:
	{
		FromTableRef *t = (FromTableRef *) f;
		appendStringInfo(str, "%s", t->tableId);
	}
	break;
	case T_FromSubquery:
	{
		FromSubquery *q = (FromSubquery *) f;
		appendStringInfo(str, "(");
		parseBackQueryBlockInternal(q->subquery, str);
		appendStringInfo(str, ")");
	}
	break;
	case T_FromJoinExpr:
	{
		FromJoinExpr *j = (FromJoinExpr *) f;

		parseBackFromItem(j->left, str);

		// join
		if(j->joinCond == JOIN_COND_NATURAL)
		{
			appendStringInfoString(str, " NATURAL ");
		}

		switch (j->joinType)
		{
        case JOIN_INNER:
			appendStringInfoString(str, " JOIN ");
            break;
        case JOIN_CROSS:
			appendStringInfoString(str, " CROSS JOIN ");
            break;
        case JOIN_LEFT_OUTER:
			appendStringInfoString(str, " LEFT OUTER JOIN ");
            break;
        case JOIN_RIGHT_OUTER:
			appendStringInfoString(str, " RIGHT OUTER JOIN ");
            break;
        case JOIN_FULL_OUTER:
			appendStringInfoString(str, " FULL OUTER JOIN ");
           break;
		}

		parseBackFromItem(j->right, str);

		// join condition
		if (j->cond && j->joinCond == JOIN_COND_ON)
		{
			appendStringInfo(str, " ON (%s)",
                exprToSQL(copyObject(j->cond), NULL, FALSE));
		}

		if (j->cond && j->joinCond == JOIN_COND_USING)
		{
			appendStringInfoString(str, " USING (");
			FOREACH(char,a,(List *) j->cond)
			{
				appendStringInfoString(str, a);
				if(FOREACH_HAS_MORE(a))
				{
					appendStringInfo(str, ", ");
				}
			}
			appendStringInfoString(str, ")");
		}
	}
	break;
	default:
		FATAL_NODE_BEATIFY_LOG("not support for parse back: ", f);
	}

	appendStringInfo(str, " AS %s (", f->name);
	FOREACH(char,a,f->attrNames)
	{
		appendStringInfo(str, "%s", a);
		if(FOREACH_HAS_MORE(a))
		{
			appendStringInfo(str, ", ");
		}
	}
	appendStringInfoString(str, ")");

}

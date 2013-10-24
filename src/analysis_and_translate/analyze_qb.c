/*-----------------------------------------------------------------------------
 *
 * analyze_qb.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "analysis_and_translate/analyse_qb.h"

#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "model/list/list.h"
#include "model/expression/expression.h"

static void analyzeQueryBlock (QueryBlock *qb);
static void analyzeJoin (FromJoinExpr *j);

static void analyzeFromTableRef(FromTableRef *f);

void
analyzeQueryBlockStmt (Node *stmt)
{
    switch(stmt->type)
    {
        case T_QueryBlock:
            analyzeQueryBlock((QueryBlock *) stmt);
            break;
        case T_SetQuery:
            analyzeSetQuery((SetQuery *) stmt);
            break;
        case T_ProvenanceStmt:
            analyzeProvenanceStmt((ProvenanceStmt *) stmt);
            break;
    }
}

static void
analyzeQueryBlock (QueryBlock *qb)
{
    List *attrRefs = NIL;

    // figuring out attributes of from clause items
    FOREACH(FromItem,f,qb->fromClause)
    {
        switch(f->type)
        {
            case T_FromTableRef:
                analyzeFromTableRef((FromTableRef *) f);
                break;
            case T_FromSubquery:
            {
                FromSubquery *sq = (FromSubquery *) f;
                analyzeQueryBlockStmt(sq->subquery);

            }
            break;
            case T_FromJoinExpr:
            {
                analyzeJoin((FromJoinExpr *) f);
            }
            break;
            default:
            	break;
        }
    }
    // collect attribute references

    // adapt attribute references
    FOREACH(AttributeReference,a,attrRefs)
    {
        // split name on each "."
        // look name in from clause attributes
        // set
    }
}

static void
analyzeJoin (FromJoinExpr *j)
{
    FromItem *left;
    FromItem *right;

    switch(left->type)
    {
        case T_FromTableRef:
            break;
        case T_FromJoinExpr:
            analyzeJoin(left);
            break;
        case T_FromSubquery:
        {
            FromSubquery *sq = (FromSubquery *) left;
            analyzeQueryBlockStmt(sq->subquery);
        }
        break;
        default:
            break;
    }

    if (j->joinCond == JOIN_COND_NATURAL)
    {

    }
    else
    {
        j->from.attrNames = copyList(left->attrNames);
        j->from.attrNames = concatTwoLists(j->from.attrNames, copyList(right->attrNames));
    }
    //JOIN_COND_USING
    //JOIN_COND_ON
}

static void analyzeFromTableRef(FromTableRef *f)
{

}

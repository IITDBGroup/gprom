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
static void analyzeSetQuery (SetQuery *q);
static void analyzeProvenanceStmt (ProvenanceStmt *q);
static void analyzeJoin (FromJoinExpr *j);
static boolean findAttrReferences (Node *node, List **state);
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
        default:
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
    findAttrReferences((Node *) qb, &attrRefs);
    // adapt attribute references
    FOREACH(AttributeReference,a,attrRefs)
    {
        // split name on each "."
        // look name in from clause attributes
        // set
    }
}

static boolean
findAttrReferences (Node *node, List **state)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, AttributeReference))
    {
        *state = appendToTailOfList(*state, node);
    }

    if (isA(node, FromJoinExpr)) //other from items
        return visit(node, findAttrReferences, state);
    else
        return TRUE;
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
            analyzeJoin((FromJoinExpr *) left);
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

static void
analyzeSetQuery (SetQuery *q)
{

}

static void
analyzeProvenanceStmt (ProvenanceStmt *q)
{

}

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
#include "common.h"
#include "analysis_and_translate/analyze_qb.h"
#include "mem_manager/mem_mgr.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "model/list/list.h"
#include "model/expression/expression.h"
#include "log/logger.h"

static void analyzeQueryBlock (QueryBlock *qb);
static void analyzeSetQuery (SetQuery *q);
static void analyzeProvenanceStmt (ProvenanceStmt *q);
static void analyzeJoin (FromJoinExpr *j);
static boolean findAttrReferences (Node *node, List **state);
static void analyzeFromTableRef(FromTableRef *f);
static void analyzeFromSubquery(FromSubquery *sq);
static List *analyzeNaturalJoinRef(FromTableRef *left, FromTableRef *right);
// real attribute name fetching
static char *getAttrNameFromDot(char *dotName);
static char *getAttrNameFromNameWithBlank(char *blankName);

void
analyzeQueryBlockStmt (Node *stmt)
{
    switch(stmt->type)
    {
        case T_QueryBlock:
        	FATAL_LOG("Go here type: qb");
            analyzeQueryBlock((QueryBlock *) stmt);
            break;
        case T_SetQuery:
        	FATAL_LOG("Go here type: sq");
            analyzeSetQuery((SetQuery *) stmt);
            break;
        case T_ProvenanceStmt:
        	FATAL_LOG("Go here type: ps");
            analyzeProvenanceStmt((ProvenanceStmt *) stmt);
            break;
        default:
        	FATAL_LOG("Go here type: def");
            break;
    }
}

static void
analyzeQueryBlock (QueryBlock *qb)
{
    List *attrRefs = NIL;
    List *fromTables = NIL;

    // figuring out attributes of from clause items
    FOREACH(FromItem,f,qb->fromClause)
    {
        switch(f->type)
        {
            case T_FromTableRef:
                analyzeFromTableRef((FromTableRef *) f);
                fromTables = appendToTailOfList(fromTables, f);
                break;
            case T_FromSubquery:
            	analyzeFromSubquery((FromSubquery *) f);
            	fromTables = appendToTailOfList(fromTables, f);
            	break;
            case T_FromJoinExpr:
                analyzeJoin((FromJoinExpr *) f);
                fromTables = appendToTailOfList(fromTables, f);
                break;
            default:
            	break;
        }
    }
    // collect attribute references
    findAttrReferences((Node *) qb->distinct, &attrRefs);
    findAttrReferences((Node *) qb->groupByClause, &attrRefs);
    findAttrReferences((Node *) qb->havingClause, &attrRefs);
    findAttrReferences((Node *) qb->limitClause, &attrRefs);
    findAttrReferences((Node *) qb->orderByClause, &attrRefs);
    findAttrReferences((Node *) qb->selectClause, &attrRefs);
    findAttrReferences((Node *) qb->whereClause, &attrRefs);
    //TODO do we need to search into fromClause because there will be attributes in the subquery?

    // adapt attribute references
    FOREACH(AttributeReference,a,attrRefs)
    {
    	// split name on each "."
    	char *name = getAttrNameFromDot(a->name);
    	int fromPos = 0, attrPos;
    	boolean isFound;
        // look name in from clause attributes
    	FOREACH(FromTableRef, t, fromTables)
    	{
    		attrPos = 0;
    		isFound = FALSE;
    		FOREACH(AttributeReference, r, attrRefs)
			{
				if(strcmp(name, r->name) == 0)
				{
					isFound = TRUE;
					break;
				}
				attrPos++;
			}
    		if(isFound)
    			break;
    		fromPos++;
    	}
        // set the position
    	if(isFound)
    	{
    		a->fromClauseItem = fromPos;
			a->attrPosition = attrPos;
    	}
    	//TODO what if not found? throw syntax error?
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

    if (isQBQuery(node))
        return TRUE;

    return visit(node, findAttrReferences, state);
}

static void
analyzeJoin (FromJoinExpr *j)
{
    FromItem *left;
    FromItem *right;

    switch(left->type)
    {
        case T_FromTableRef:
        	analyzeFromTableRef((FromTableRef *)left);
            break;
        case T_FromJoinExpr:
            analyzeJoin((FromJoinExpr *)left);
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

    switch(right->type)
	{
		case T_FromTableRef:
			analyzeFromTableRef((FromTableRef *)right);
			break;
		case T_FromJoinExpr:
			analyzeJoin((FromJoinExpr *) right);
			break;
		case T_FromSubquery:
		{
			FromSubquery *sq = (FromSubquery *) right;
			analyzeQueryBlockStmt(sq->subquery);
		}
		break;
		default:
			break;
	}

    if (j->joinCond == JOIN_COND_NATURAL)
    {
    	j->from.attrNames = analyzeNaturalJoinRef((FromTableRef *)j->left, (FromTableRef *)j->right);
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
	f->from.attrNames = getAttributes(f->tableId);
	f->from.name = f->tableId;//TODO is it necessary?
}

static void analyzeFromSubquery(FromSubquery *sq)
{
	analyzeQueryBlockStmt(sq->subquery);
	switch(sq->subquery->type)
	{
		case T_QueryBlock:
		{
			QueryBlock *subQb = (QueryBlock *) sq->subquery;
			FOREACH(SelectItem,s,subQb->selectClause)
			{
				sq->from.attrNames = appendToTailOfList(sq->from.attrNames,s->alias);
				//TODO do we need to fill alias if it is null?
			}
		}
			break;
		default:
			break;
	}
}

static List *
analyzeNaturalJoinRef(FromTableRef *left, FromTableRef *right)
{
	if(left == NULL || right == NULL)
		return NIL;
	List *lList = left->from.attrNames;
	List *rList = right->from.attrNames;
	List *result = NIL;
	FOREACH(AttributeReference, l , lList)
	{
		FOREACH(AttributeReference, r, rList)
		{
			if(strcmp(l->name, r->name) == 0)
			{
				result = appendToTailOfList(result, l);
			}
		}
	}
	return result;
}

static char *
getAttrNameFromDot(char *dotName)
{
	dotName = getAttrNameFromNameWithBlank(dotName);
	//create a new attribute name from the original name with dot
	char *string = strdup(dotName);
	char *toFree = string;
	char *token;
	while((token = strsep(&string, ".")) != NULL);
	char *attrName = strdup(token);
	FREE(toFree);
	return attrName;
}

static char *
getAttrNameFromNameWithBlank(char *blankName)
{
	// filter out blank in string
	int i;
	for(i=0;i<strlen(blankName);i++)
	{
		if(blankName[i]==' ')
			memcpy(blankName+i,blankName+i+1,strlen(blankName)-i);
	}
	return blankName;
}

static void
analyzeSetQuery (SetQuery *q)
{

}

static void
analyzeProvenanceStmt (ProvenanceStmt *q)
{

}



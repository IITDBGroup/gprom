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
#include "metadata_lookup/metadata_lookup.h"

static void analyzeStmtList (List *l);
static void analyzeQueryBlock (QueryBlock *qb);
static void analyzeSetQuery (SetQuery *q);
static void analyzeProvenanceStmt (ProvenanceStmt *q);
static void analyzeJoin (FromJoinExpr *j);
static boolean findAttrReferences (Node *node, List **state);
static boolean findAttrRefInFrom (AttributeReference *a, List *fromItems);
static boolean findQualifiedAttrRefInFrom (List *nameParts, AttributeReference *a,  List *fromItems);
static void analyzeFromTableRef(FromTableRef *f);
static void analyzeFromSubquery(FromSubquery *sq);
static List *analyzeNaturalJoinRef(FromTableRef *left, FromTableRef *right);
// real attribute name fetching
//static char *getAttrNameFromDot(char *dotName);
static List *splitAttrOnDot (char *dotName);
static char *getAttrNameFromNameWithBlank(char *blankName);

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
        case T_List:
            analyzeStmtList ((List *) stmt);
            break;
        default:
            break;
    }
}

static void
analyzeStmtList (List *l)
{
    FOREACH(Node,n,l)
        analyzeQueryBlockStmt(n);
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

    ERROR_LOG("Figuring out attributes of from clause items done");
    ERROR_LOG("Found the following from tables: <%s>", nodeToString(fromTables));

//    FOREACH(FromTableRef, f, fromTables)
//    {
//    	ERROR_LOG("tableID: %s",f->tableId);
//    }
    // collect attribute references
    findAttrReferences((Node *) qb->distinct, &attrRefs);
    findAttrReferences((Node *) qb->groupByClause, &attrRefs);
    findAttrReferences((Node *) qb->havingClause, &attrRefs);
    findAttrReferences((Node *) qb->limitClause, &attrRefs);
    findAttrReferences((Node *) qb->orderByClause, &attrRefs);
    findAttrReferences((Node *) qb->selectClause, &attrRefs);
    findAttrReferences((Node *) qb->whereClause, &attrRefs);
    //TODO do we need to search into fromClause because there will be attributes in the subquery?

    ERROR_LOG("Collect attribute references done");
    INFO_LOG("Have the following attribute references: <%s>", nodeToString(attrRefs));

    // adapt attribute references
    FOREACH(AttributeReference,a,attrRefs)
    {
    	// split name on each "."
        boolean isFound = FALSE;
        List *nameParts = splitAttrOnDot(a->name);
        ERROR_LOG("attr split: %s", stringListToString(nameParts));

    	if (LIST_LENGTH(nameParts) == 1)
    	    isFound = findAttrRefInFrom(a, fromTables);
    	else if (LIST_LENGTH(nameParts) == 2)
    	    isFound = findQualifiedAttrRefInFrom(nameParts, a, fromTables);
    	else
    	    FATAL_LOG("right now attribute names should have at most two parts");

    	if (!isFound)
    	    FATAL_LOG("attribute <%s> does not exist in FROM clause", a->name);
    	//TODO what if not found? throw syntax error?
    }

    ERROR_LOG("Analysis done");
}

static boolean
findAttrRefInFrom (AttributeReference *a, List *fromItems)
{
    boolean isFound = FALSE;
    int fromPos = 0, attrPos;

    FOREACH(FromItem, f, fromItems)
    {
        attrPos = 0;
        FOREACH(char, r, f->attrNames)
        {
            if(strcmp(a->name, r) == 0)
            {
                // is ambigious?
                if (isFound)
                {
                    FATAL_LOG("Ambiguous attribute reference <%s>", a->name);
                    break;
                }
                // find occurance found
                else
                {
                    isFound = TRUE;
                    a->fromClauseItem = fromPos;
                    a->attrPosition = attrPos;
                }
            }
            attrPos++;
        }
        fromPos++;
    }

    return isFound;
}

static boolean
findQualifiedAttrRefInFrom (List *nameParts, AttributeReference *a, List *fromItems)
{
    boolean foundFrom;
    boolean foundAttr;
    int fromClauseItem = 0;
    int attrPos = 0;
    char *tabName = (char *) getNthOfList(nameParts, 0);
    char *attrName = (char *) getNthOfList(nameParts, 1);
    FromItem *fromItem = NULL;

    // find table name
    FOREACH(FromItem, f, fromItems)
    {
        if (strcmp(f->name, tabName) == 0)
        {
            if (foundFrom)
            {
                FATAL_LOG("from clause item name <%s> appears more than once", tabName);
                return FALSE;
            }
            else
            {
                fromItem = f;
                a->fromClauseItem = fromClauseItem;
                foundFrom = TRUE;
            }
        }
        fromClauseItem++;
    }

    // did we find from clause item
    if (!foundFrom)
    {
        FATAL_LOG("did not find from clause item named <%s>", tabName);
        return FALSE;
    }

    // find attribute name
    FOREACH(char,aName,fromItem->attrNames)
    {
        if (strcmp(aName, attrName) == 0)
        {
            if(foundAttr)
            {
                FATAL_LOG("ambigious attr name <%s> appears more than once in "
                        "from clause item <%s>", attrName, tabName);
                return FALSE;
            }
            else
            {
                a->attrPosition = attrPos;
                foundAttr = TRUE;
            }
        }
        attrPos++;
    }

    if (!foundAttr)
    {
        FATAL_LOG("did not find from clause item named <%s>", attrName);
        return FALSE;
    }

    return foundAttr;
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
    List *attrRefs = getAttributes(f->tableId);
    FOREACH(AttributeReference,a,attrRefs)
	    f->from.attrNames = appendToTailOfList(f->from.attrNames, a->name);

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

static List *
splitAttrOnDot (char *dotName)
{
    int start = 0, pos = 0;
    char *token, *string = dotName;
    List *result = NIL;

    while(string != NULL)
    {
        token = strsep(&string, ".");
        result = appendToTailOfList(result, strdup(token));
    }

//    while((c = dotName[pos++]) != '\0')
//        if (c == '.')
//        {
//            size_t len = (pos - start);
//            char *newSeg = CNEW(char, len + 1);
//
//            strncpy(newSeg, dotName + start, len);
//            result = appendToTailOfList(result, newSeg);
//
//            start = pos + 1;
//        }
//
//    size_t len = (pos - start);
//    char *newSeg = CNEW(char, len + 1);
//    strncpy(newSeg, dotName + start, len);
//    result = appendToTailOfList(result, newSeg);

    return result;
}

//static char *
//getAttrNameFromDot(char *dotName)
//{
//	dotName = getAttrNameFromNameWithBlank(dotName);
//	//create a new attribute name from the original name with dot
//	char *string = strdup(dotName);
//	char *toFree = string;
//	char *token = NULL;
//	while(string != NULL)
//		token = strsep(&string, ".");
//	char *attrName = strdup(token);
//	FREE(toFree);
//	return attrName;
//}

static char *
getAttrNameFromNameWithBlank(char *blankName)
{
	if(blankName == NULL)
		return NULL;

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



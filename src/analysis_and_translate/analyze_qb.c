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
static boolean findFunctionCall (Node *node, List **state);
static boolean findAttrRefInFrom (AttributeReference *a, List *fromItems);
static boolean findQualifiedAttrRefInFrom (List *nameParts, AttributeReference *a,  List *fromItems);
static void analyzeFromTableRef(FromTableRef *f);
static void analyzeFromSubquery(FromSubquery *sq);
static List *analyzeNaturalJoinRef(FromTableRef *left, FromTableRef *right);
static void analyzeFunctionCall(QueryBlock *qb);

// real attribute name fetching
static List *expandStarExpression (SelectItem *s, List *fromClause);
static List *splitAttrOnDot (char *dotName);
static char *generateAttrNameFromExpr(SelectItem *s);

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

    INFO_LOG("RESULT OF ANALYSIS IS:\n%s", beatify(nodeToString(stmt)));
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

    INFO_LOG("Figuring out attributes of from clause items done");
    DEBUG_LOG("Found the following from tables: <%s>", nodeToString(fromTables));

    // expand * expressions
    List *expandedSelectClause = NIL;
    FOREACH(SelectItem,s,qb->selectClause)
    {
        if (s->expr == NULL)
            expandedSelectClause = concatTwoLists(expandedSelectClause,
                    expandStarExpression(s,fromTables));
        else
            expandedSelectClause = appendToTailOfList(expandedSelectClause,s);
    }
    qb->selectClause = expandedSelectClause;
    INFO_LOG("Expanded select clause is: <%s>",nodeToString(expandedSelectClause));

    // create attribute names for unnamed attribute in select clause
    FOREACH(SelectItem,s,qb->selectClause)
    {
        if (s->alias == NULL)
        {
            char *newAlias = generateAttrNameFromExpr(s);
            s->alias = strdup(newAlias);
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

    INFO_LOG("Collect attribute references done");
    DEBUG_LOG("Have the following attribute references: <%s>", nodeToString(attrRefs));

    // adapt attribute references
    FOREACH(AttributeReference,a,attrRefs)
    {
    	// split name on each "."
        boolean isFound = FALSE;
        List *nameParts = splitAttrOnDot(a->name);
        DEBUG_LOG("attr split: %s", stringListToString(nameParts));

    	if (LIST_LENGTH(nameParts) == 1)
    	    isFound = findAttrRefInFrom(a, fromTables);
    	else if (LIST_LENGTH(nameParts) == 2)
    	    isFound = findQualifiedAttrRefInFrom(nameParts, a, fromTables);
    	else
    	    FATAL_LOG("right now attribute names should have at most two parts");

    	if (!isFound)
    	    FATAL_LOG("attribute <%s> does not exist in FROM clause", a->name);
    }

    // adapt function call (isAgg)
    analyzeFunctionCall(qb);

    INFO_LOG("Analysis done");
}

static void
analyzeFunctionCall(QueryBlock *qb)
{
	List *functionCallList = NIL;

	// collect function call
	findFunctionCall((Node *) qb->selectClause, &functionCallList);

	INFO_LOG("Collect function call done");
	DEBUG_LOG("Have the following function calls: <%s>", nodeToString(functionCallList));

	// adapt function call
	FOREACH(FunctionCall, c, functionCallList)
	{
		c->isAgg = isAgg(c->functionname);
	}
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
    boolean foundFrom = FALSE;
    boolean foundAttr = FALSE;
    int fromClauseItem = 0;
    int attrPos = 0;
    char *tabName = (char *) getNthOfListP(nameParts, 0);
    char *attrName = (char *) getNthOfListP(nameParts, 1);
    FromItem *fromItem = NULL;

    DEBUG_LOG("looking for attribute %s.%s", tabName, attrName);

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

    a->name = strdup(attrName);

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

static boolean
findFunctionCall (Node *node, List **state)
{
	if(node == NULL)
		return TRUE;

	if(isA(node, FunctionCall))
	{
		*state = appendToTailOfList(*state, node);
	}

	if(isQBQuery(node))
		return TRUE;

	return visit(node, findFunctionCall, state);
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

static void
analyzeFromTableRef(FromTableRef *f)
{
    List *attrRefs = getAttributes(f->tableId);
    FOREACH(AttributeReference,a,attrRefs)
	    f->from.attrNames = appendToTailOfList(f->from.attrNames, a->name);

    if(f->from.name == NULL)
    	f->from.name = f->tableId;
}

static void
analyzeFromSubquery(FromSubquery *sq)
{
	analyzeQueryBlockStmt(sq->subquery);
	switch(sq->subquery->type)
	{
		case T_QueryBlock:
		{
			QueryBlock *subQb = (QueryBlock *) sq->subquery;
			FOREACH(SelectItem,s,subQb->selectClause)
			{
                sq->from.attrNames = appendToTailOfList(sq->from.attrNames,
                        s->alias);
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
    char *token, *string = strdup(dotName);
    List *result = NIL;

    while(string != NULL)
    {
        token = strsep(&string, ".");
        result = appendToTailOfList(result, strdup(token));
    }

    TRACE_LOG("Split attribute reference <%s> into <%s>", dotName, stringListToString(result));

    return result;
}

static List *
expandStarExpression (SelectItem *s, List *fromClause)
{
    List *nameParts = splitAttrOnDot(s->alias);
    List *newSelectItems = NIL;

    assert(LIST_LENGTH(nameParts) == 1 || LIST_LENGTH(nameParts) == 2);

    // should be "*" select item -> expand to all attribute in from clause
    if (LIST_LENGTH(nameParts) == 1)
    {
        assert(strcmp((char *) getNthOfListP(nameParts,0),"*") == 0);
        FOREACH(FromItem,f,fromClause)
        {
            FOREACH(char,attr,f->attrNames)
            {
                newSelectItems = appendToTailOfList(newSelectItems,
                        createSelectItem(
                                strdup(attr),
                                (Node *) createAttributeReference(
                                		f->name? CONCAT_STRINGS(f->name,".",attr)
                                				: attr)
                        ));
            }
        }
    }
    /*
     * should be "R.*" for some from clause item named R, expand to all
     * attributes from R
     */
    else
    {
        boolean found = FALSE;
        char *tabName = (char *) getNthOfListP(nameParts,0);
        char *attrName = (char *) getNthOfListP(nameParts,1);
        assert(strcmp(attrName,"*") == 0);

        FOREACH(FromItem,f,fromClause)
        {
            if (strcmp(f->name,tabName) == 0)
            {
                if (found)
                    FATAL_LOG("Ambiguous from clause reference <%s> to from clause item <%s>", s->alias, tabName);
                else
                {
                    FOREACH(char,attr,f->attrNames)
                    {
                        newSelectItems = appendToTailOfList(newSelectItems,
                                createSelectItem(
                                       strdup(attr),
                                       (Node *) createAttributeReference(
                                               CONCAT_STRINGS(f->name,".",attr))
                                       ));
                    }
                }
            }
        }
    }

    DEBUG_LOG("Expanded a star expression into <%s>", nodeToString(newSelectItems));

    return newSelectItems;
}

static char *
generateAttrNameFromExpr(SelectItem *s)
{
    char *name = exprToSQL(s->expr);
    char c;
    StringInfo str = makeStringInfo();

    while((c = *name++) != '\0')
        if (c != ' ')
            appendStringInfoChar(str, toupper(c));

    return str->data;
}

static void
analyzeSetQuery (SetQuery *q)
{

}

static void
analyzeProvenanceStmt (ProvenanceStmt *q)
{

}



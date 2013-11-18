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
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "model/list/list.h"
#include "model/expression/expression.h"
#include "metadata_lookup/metadata_lookup.h"
#include "provenance_rewriter/prov_schema.h"

static void analyzeStmtList (List *l);
static void analyzeQueryBlock (QueryBlock *qb);
static void analyzeSetQuery (SetQuery *q);
static void analyzeProvenanceStmt (ProvenanceStmt *q);
static void analyzeJoin (FromJoinExpr *j);
static boolean findAttrReferences (Node *node, List **state);
static boolean findFunctionCall (Node *node, List **state);
static boolean findAttrRefInFrom (AttributeReference *a, List *fromItems);
static FromItem *findNamedFromItem (FromItem *fromItem, char *name);
static int findAttrInFromItem (FromItem *fromItem, AttributeReference *attr);
static boolean findQualifiedAttrRefInFrom (List *nameParts, AttributeReference *a,  List *fromItems);
static void analyzeFromTableRef(FromTableRef *f);
static void analyzeFromSubquery(FromSubquery *sq);
static List *analyzeNaturalJoinRef(FromTableRef *left, FromTableRef *right);
static void analyzeFunctionCall(QueryBlock *qb);

// real attribute name fetching
static List *expandStarExpression (SelectItem *s, List *fromClause);
static List *splitAttrOnDot (char *dotName);
//static char *getAttrNameFromNameWithBlank(char *blankName);
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

        DEBUG_LOG("analyzed from item <%s>", nodeToString(f));
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
        attrPos = findAttrInFromItem(f, a);

        if (attrPos != INVALID_ATTR)
        {
            if (isFound)
                FATAL_LOG("ambigious attribute reference %s", a->name);
            else
            {
                isFound = TRUE;
                a->fromClauseItem = fromPos;
                a->attrPosition = attrPos;
            }
        }
        fromPos++;
    }

    return isFound;
}

static FromItem *
findNamedFromItem (FromItem *fromItem, char *name)
{
    if (isA(fromItem, FromJoinExpr))
    {
        FromJoinExpr *join = (FromJoinExpr *) fromItem;
        FromItem *result;

        // if join has an alias do not recurse
        if (join->from.name != NULL)
        {
            if (strcmp(name, join->from.name) == 0)
                return fromItem;
            else
                return NULL;
        }

        result = findNamedFromItem (join->left, name);
        if (result != NULL)
            return result;
        return findNamedFromItem (join->right, name);
    }

    // is not a join
    if (strcmp(name, fromItem->name) == 0)
        return fromItem;

    return NULL;
}

static int
findAttrInFromItem (FromItem *fromItem, AttributeReference *attr)
{
    boolean isFound = FALSE;
    int attrPos = 0, foundAttr = INVALID_ATTR;

    // is not a join
    FOREACH(char, r, fromItem->attrNames)
    {
        if(strcmp(attr->name, r) == 0)
        {
            // is ambigious?
            if (isFound)
            {
                FATAL_LOG("Ambiguous attribute reference <%s>", attr->name);
                break;
            }
            // find occurance found
            else
            {
                isFound = TRUE;
                foundAttr = attrPos;
            }
        }
        attrPos++;
    }

    return foundAttr;
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
        FromItem *foundF = findNamedFromItem(f, tabName);

        if (foundF != NULL)
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
    FromItem *left = j->left;
    FromItem *right = j->right;

    // analyze inputs
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
            analyzeFromSubquery(sq);
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
			analyzeFromSubquery(sq);
		}
		break;
		default:
			break;
	}

    if (j->joinCond == JOIN_COND_NATURAL)
    {
        List *expectedAttrs = analyzeNaturalJoinRef((FromTableRef *)j->left,
                (FromTableRef *)j->right);
    	if (j->from.attrNames == NULL)
    	    j->from.attrNames = expectedAttrs;
    	assert(LIST_LENGTH(j->from.attrNames) == LIST_LENGTH(expectedAttrs));
    }
    //JOIN_COND_USING
    //JOIN_COND_ON
    else
    {
        List *expectedAttrs = concatTwoLists(
                deepCopyStringList(left->attrNames),
                deepCopyStringList(right->attrNames));
        if (j->from.attrNames == NULL)
            j->from.attrNames = expectedAttrs;
        assert(LIST_LENGTH(j->from.attrNames) == LIST_LENGTH(expectedAttrs));
    }
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
    List *expectedAttrs = NIL;

	analyzeQueryBlockStmt(sq->subquery);
	switch(sq->subquery->type)
	{
		case T_QueryBlock:
		{
			QueryBlock *subQb = (QueryBlock *) sq->subquery;
			FOREACH(SelectItem,s,subQb->selectClause)
			{
                 expectedAttrs = appendToTailOfList(expectedAttrs,
                        s->alias);
			}
		}
        break;
		case T_SetQuery:
		{
		    SetQuery *setQ = (SetQuery *) sq->subquery;
		    expectedAttrs = deepCopyStringList(setQ->selectClause);
		}
        break;
		case T_ProvenanceStmt:
		{
		    ProvenanceStmt *pStmt = (ProvenanceStmt *) sq->subquery;
		    expectedAttrs = deepCopyStringList(pStmt->selectClause);
		}
		break;
		default:
			break;
	}

	// if no attr aliases given
	if (!(sq->from.attrNames))
	    sq->from.attrNames = expectedAttrs;

	assert(LIST_LENGTH(sq->from.attrNames) == LIST_LENGTH(expectedAttrs));
}

static List *
analyzeNaturalJoinRef(FromTableRef *left, FromTableRef *right)
{
    List *lList = left->from.attrNames;
    List *rList = right->from.attrNames;
    List *result = deepCopyStringList(left->from.attrNames);

	// only add attributes from right input that are not in left input
	FOREACH(char, r, rList)
	{
	    boolean found = FALSE;
		FOREACH(char , l, lList)
		{
			if(strcmp(l, r) == 0)
			    found = TRUE;
		}
		if (!found)
            result = appendToTailOfList(result, r);
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
        int fromAliasCount = 0;
        assert(strcmp((char *) getNthOfListP(nameParts,0),"*") == 0);

        FOREACH(FromItem,f,fromClause)
        {
            // create alias for join
            if (!(f->name))
            {
                StringInfo s = makeStringInfo();
                appendStringInfo(s,"%u", fromAliasCount++);
                f->name = CONCAT_STRINGS("dummyFrom", s->data);
                FREE(s);
            }

            FOREACH(char,attr,f->attrNames)
            {
                AttributeReference *newA = createAttributeReference(
                          CONCAT_STRINGS(f->name,".",attr));

                newSelectItems = appendToTailOfList(newSelectItems,
                        createSelectItem(
                                strdup(attr),
                                (Node *) newA
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
    analyzeQueryBlockStmt(q->lChild);
    analyzeQueryBlockStmt(q->rChild);

    // get attributes from left child
    switch(q->lChild->type)
    {
        case T_QueryBlock:
        {
            QueryBlock *qb = (QueryBlock *) q->lChild;
            FOREACH(SelectItem,s,qb->selectClause)
            {
                q->selectClause = appendToTailOfList(q->selectClause,
                        strdup(s->alias));
            }
        }
        break;
        case T_SetQuery:
            q->selectClause = deepCopyStringList(
                    ((SetQuery *) q->lChild)->selectClause);
        break;
        case T_ProvenanceStmt:
            q->selectClause = deepCopyStringList(
                    ((ProvenanceStmt *) q->lChild)->selectClause);
        break;
        default:
        break;
    }
}

static void
analyzeProvenanceStmt (ProvenanceStmt *q)
{
    analyzeQueryBlockStmt(q->query);

    // get attributes from left child
    switch(q->query->type)
    {
        case T_QueryBlock:
        {
            QueryBlock *qb = (QueryBlock *) q->query;
            FOREACH(SelectItem,s,qb->selectClause)
            {
                q->selectClause = appendToTailOfList(q->selectClause,
                        strdup(s->alias));
            }
        }
        break;
        case T_SetQuery:
            q->selectClause = deepCopyStringList(
                    ((SetQuery *) q->query)->selectClause);
        break;
        case T_ProvenanceStmt:
            q->selectClause = deepCopyStringList(
                    ((ProvenanceStmt *) q->query)->selectClause);
        break;
        default:
        break;
    }

    q->selectClause = concatTwoLists(q->selectClause,
            getQBProvenanceAttrList(q));
}



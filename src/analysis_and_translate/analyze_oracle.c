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
#include "instrumentation/timing_instrumentation.h"

#include "analysis_and_translate/analyze_oracle.h"
#include "analysis_and_translate/parameter.h"
#include "configuration/option.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/expression/expression.h"
#include "metadata_lookup/metadata_lookup.h"
#include "provenance_rewriter/prov_schema.h"
#include "parser/parser.h"

static void analyzeStmtList (List *l, List *parentFroms);
static void analyzeQueryBlock (QueryBlock *qb, List *parentFroms);
static void analyzeSetQuery (SetQuery *q, List *parentFroms);
static void analyzeProvenanceStmt (ProvenanceStmt *q, List *parentFroms);
static void analyzeProvenanceOptions (ProvenanceStmt *prov);
static void analyzeWithStmt (WithStmt *w);

static void analyzeJoin (FromJoinExpr *j, List *parentFroms);

// search for attributes and other relevant node types
static void adaptAttrPosOffset(FromItem *f, FromItem *decendent, AttributeReference *a);
static void adaptAttributeRefs(List* attrRefs, List* parentFroms);
static boolean findAttrReferences (Node *node, List **state);
static void enumerateParameters (Node *stmt);
static boolean findFunctionCall (Node *node, List **state);
static boolean findAttrRefInFrom (AttributeReference *a, List *fromClauses);
static FromItem *findNamedFromItem (FromItem *fromItem, char *name);
static int findAttrInFromItem (FromItem *fromItem, AttributeReference *attr);
static boolean findQualifiedAttrRefInFrom (List *nameParts, AttributeReference *a,  List *fromClauses);

// analyze from item types
static void analyzeFromTableRef(FromTableRef *f);
static void analyzeInsert(Insert *f);
static void analyzeDelete(Delete *f);
static void analyzeUpdate(Update *f);
static void analyzeFromSubquery(FromSubquery *sq, List *parentFroms);
static List *analyzeNaturalJoinRef(FromTableRef *left, FromTableRef *right);
static void analyzeJoinCondAttrRefs(List *fromClause, List *parentFroms);

// analyze function calls and nested subqueries
static void analyzeFunctionCall(QueryBlock *qb);
static void analyzeNestedSubqueries(QueryBlock *qb, List *parentFroms);

// analyze FromJsonTable Item
static void analyzeFromJsonTable(FromJsonTable *f, List **state);

// real attribute name fetching
static List *expandStarExpression (SelectItem *s, List *fromClause);
static List *splitAttrOnDot (char *dotName);
//static char *getAttrNameFromNameWithBlank(char *blankName);
static List *getFromTreeLeafs (List *from);
static char *generateAttrNameFromExpr(SelectItem *s);
static List *splitTableName(char *tableName);
static List *getQBAttrNames (Node *qb);
static List *getQBAttrDTs (Node *qb);
static boolean setViewFromTableRefAttrs(Node *node, List *views);

/* str functions */
static inline char *
strToUpper(char *in)
{
    char *result = strdup(in);
    char *pos;
    for(pos = result; *++pos != '\0'; *pos = toupper(*pos));

    return result;
}


Node *
analyzeOracleModel (Node *stmt)
{
    analyzeQueryBlockStmt(stmt, NULL);

    return stmt;
}

void
analyzeQueryBlockStmt (Node *stmt, List *parentFroms)
{
    switch(stmt->type)
    {
        case T_QueryBlock:
            analyzeQueryBlock((QueryBlock *) stmt, parentFroms);
            DEBUG_LOG("analyzed QB");
            break;
        case T_SetQuery:
            analyzeSetQuery((SetQuery *) stmt, parentFroms);
            DEBUG_LOG("analyzed Set Query");
            break;
        case T_ProvenanceStmt:
            analyzeProvenanceStmt((ProvenanceStmt *) stmt, parentFroms);
            DEBUG_LOG("analyzed Provenance Stmt");
            break;
        case T_List:
            analyzeStmtList ((List *) stmt, parentFroms);
            DEBUG_LOG("analyzed List");
            break;
        case T_Insert:
            analyzeInsert((Insert *) stmt);
            break;
        case T_Delete:
            analyzeDelete((Delete *) stmt);
            break;
        case T_Update:
            analyzeUpdate((Update *) stmt);
            break;
        case T_WithStmt:
            analyzeWithStmt((WithStmt *) stmt);
            break;
        default:
            break;
    }

    if(isQBUpdate(stmt) || isQBQuery(stmt))
        enumerateParameters(stmt);

    INFO_LOG("RESULT OF ANALYSIS IS:\n%s", beatify(nodeToString(stmt)));
}

static void
enumerateParameters (Node *stmt)
{
    List *params = findParameters(stmt);
    int i = 1;

    FOREACH(SQLParameter,p,params)
        p->position = i++;
}

static void
analyzeStmtList (List *l, List *parentFroms)
{
    FOREACH(Node,n,l)
        analyzeQueryBlockStmt(n, parentFroms);
}

static void
adaptAttributeRefs(List* attrRefs, List* parentFroms)
{
    // adapt attribute references
    FOREACH(AttributeReference,a,attrRefs)
    {
        // split name on each "."
        boolean isFound = FALSE;
        List *nameParts = splitAttrOnDot(a->name);
        DEBUG_LOG("attr split: %s", stringListToString(nameParts));

        if (LIST_LENGTH(nameParts) == 1)
            isFound = findAttrRefInFrom(a, parentFroms);
        else if (LIST_LENGTH(nameParts) == 2)
            isFound = findQualifiedAttrRefInFrom(nameParts, a, parentFroms);
        else
            FATAL_LOG(
                    "right now attribute names should have at most two parts");

        if (!isFound)
            FATAL_LOG("attribute <%s> does not exist in FROM clause", a->name);
    }
}

static void
analyzeQueryBlock (QueryBlock *qb, List *parentFroms)
{
    List *attrRefs = NIL;

    FOREACH(FromItem,f,qb->fromClause)
    {
        switch(f->type)
        {
            case T_FromTableRef:
            {
                FromTableRef *tr = (FromTableRef *)f;
            	//check if it is a table or a view
            	if (!catalogTableExists(tr->tableId) && catalogViewExists(tr->tableId))
            	{
            	    char * view = getViewDefinition(((FromTableRef *)f)->tableId);
            	    char *newName = f->name ? f->name : tr->tableId; // if no alias then use view name
            	    DEBUG_LOG("view: %s", view);
            	    StringInfo s = makeStringInfo();
            	    appendStringInfoString(s,view);
            	    appendStringInfoString(s,";");
            	    view = s->data;
            	    Node * n1 = getHeadOfListP((List *) parseFromString((char *) view));
            	    FromItem * f1 = createFromSubquery(newName,NIL,(Node *) n1);

            	    DUMMY_LC(f)->data.ptr_value = f1;
            	}
            }
            break;
            default:
            	break;
        }
    }

    // figuring out attributes of from clause items
    FOREACH(FromItem,f,qb->fromClause)
    {
        switch(f->type)
        {
            case T_FromTableRef:
            	analyzeFromTableRef((FromTableRef *) f);
                break;           
            case T_FromSubquery:
            	analyzeFromSubquery((FromSubquery *) f, parentFroms);
            	break;
            case T_FromJoinExpr:
                analyzeJoin((FromJoinExpr *) f, parentFroms);
                break;
            case T_FromJsonTable:
	        analyzeFromJsonTable((FromJsonTable *)f, &attrRefs);
	        break;
            default:
            	break;
        }

        // analyze FromProvInfo if exists
        if (f->provInfo)
        {
            /* if the user provides a list of attributes (that store provenance
             * or should be duplicated as provenance attributes) then we need
             * to make sure these attributes exist. */
            if (f->provInfo->userProvAttrs)
            {
                FOREACH(char,name,f->provInfo->userProvAttrs)
                {
                    if(!searchListString(f->attrNames, name))
                    {
                        if (strcmp(name,"ROWID") == 0 || f->type == T_FromTableRef)
                        {
                            f->attrNames = appendToTailOfList(f->attrNames, strdup("ROWID"));
                        }
                        else
                            FATAL_LOG("did not find provenance attr %s in from "
                                "item attrs %s", name, stringListToString(f->attrNames));
                    }
                }
            }
        }

        /*
         * boolean baserel;
    boolean intermediateProv;
    List *userProvAttrs;
         */

        DEBUG_LOG("analyzed from item <%s>", nodeToString(f));
    }

    INFO_LOG("Figuring out attributes of from clause items done");
    DEBUG_LOG("Found the following from tables: <%s>", nodeToString(qb->fromClause));

    // expand * expressions
    List *expandedSelectClause = NIL;
    FOREACH(SelectItem,s,qb->selectClause)
    {
        if (s->expr == NULL)
            expandedSelectClause = concatTwoLists(expandedSelectClause,
                    expandStarExpression(s,qb->fromClause));
        else
            expandedSelectClause = appendToTailOfList(expandedSelectClause,s);
    }
    qb->selectClause = expandedSelectClause;
    INFO_LOG("Expanded select clause is: <%s>",nodeToString(expandedSelectClause));

    // analyze join conditions attribute references
    analyzeJoinCondAttrRefs(qb->fromClause, parentFroms);

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

    // expand list of from clause to use
    parentFroms = appendToHeadOfList(copyList(parentFroms), qb->fromClause);

    // adapt attribute references
    adaptAttributeRefs(attrRefs, parentFroms);

    // create attribute names for unnamed attribute in select clause
    FOREACH(SelectItem,s,qb->selectClause)
    {
        if (s->alias == NULL)
        {
            char *newAlias = generateAttrNameFromExpr(s);
            s->alias = strdup(newAlias);
        }
    }

    // adapt function call (isAgg)
    analyzeFunctionCall(qb);
    DEBUG_LOG("Analyzed functions");

    // find nested subqueries and analyze them
    analyzeNestedSubqueries(qb, parentFroms);
    DEBUG_LOG("Analyzed nested subqueries");

    INFO_LOG("Analysis done");
}

static void
analyzeNestedSubqueries(QueryBlock *qb, List *parentFroms)
{
    List *nestedSubqueries = NIL;

    // find nested subqueries
    findNestedSubqueries((Node *) qb->selectClause, &nestedSubqueries);
    findNestedSubqueries((Node *) qb->distinct, &nestedSubqueries);
    findNestedSubqueries((Node *) qb->fromClause, &nestedSubqueries);
    findNestedSubqueries((Node *) qb->whereClause, &nestedSubqueries);
    findNestedSubqueries((Node *) qb->groupByClause, &nestedSubqueries);
    findNestedSubqueries((Node *) qb->havingClause, &nestedSubqueries);
    findNestedSubqueries((Node *) qb->orderByClause, &nestedSubqueries);

    DEBUG_LOG("Current query <%s>\nhas nested subqueries\n%s",
            nodeToString(qb), nodeToString(nestedSubqueries));

    // analyze each subquery
    FOREACH(NestedSubquery,q,nestedSubqueries)
        analyzeQueryBlockStmt(q->query, parentFroms);
}

static void
analyzeFunctionCall(QueryBlock *qb)
{
    List *functionCallList = NIL;

    // collect function call
    findFunctionCall((Node *) qb->selectClause, &functionCallList);
    findFunctionCall((Node *) qb->havingClause, &functionCallList);

    INFO_LOG("Collect function call done");
    DEBUG_LOG("Have the following function calls: <%s>", nodeToString(functionCallList));

    // adapt function call
    FOREACH(Node, f, functionCallList) {
        if (isA(f, FunctionCall))
        {
            FunctionCall *c = (FunctionCall *) f;
            c->isAgg = isAgg(c->functionname);
        }
        // window function
        else
        {
            WindowFunction *w = (WindowFunction *) f;
            FunctionCall *c = (FunctionCall *) w->f;
            if(!isWindowFunction(c->functionname))
                FATAL_LOG("Function %s not supported as window function",
                        c->functionname);
        }
    }
}

static void
analyzeJoinCondAttrRefs(List *fromClause, List *parentFroms)
{
    List *stack = copyList(fromClause);

    while(!LIST_EMPTY(stack))
    {
        FromItem *cur = (FromItem *) popHeadOfListP(stack);

        DEBUG_LOG("analyze join:\n%s", beatify(nodeToString(cur)));

        // only interested in joins
        if (isA(cur,FromJoinExpr))
        {
            FromJoinExpr *j = (FromJoinExpr *) cur;
            List *aRefs = NIL;

            findAttrReferences(j->cond, &aRefs);

            // analyze children (if they are joins)
            if (isA(j->left, FromJoinExpr))
                stack = appendToTailOfList(stack, j->left);
            if (isA(j->right, FromJoinExpr))
                stack = appendToTailOfList(stack, j->right);

            DEBUG_LOG("join condition has attrs:\n%s",
                    beatify(nodeToString(aRefs)));

            FOREACH(AttributeReference,a,aRefs)
            {
                List *nameParts = splitAttrOnDot(a->name);
                boolean isFound = FALSE;
                List *newFroms = NIL;

                DEBUG_LOG("attr split: %s", stringListToString(nameParts));

                // no from item specified, check direct inputs
                if (LIST_LENGTH(nameParts) == 1)
                {
                    newFroms = copyList(parentFroms);
                    newFroms = appendToTailOfList(newFroms, LIST_MAKE(j->left, j->right));
                    isFound = findAttrRefInFrom(a, newFroms);
                }
                // is R.A, search for table in subtree
                else if (LIST_LENGTH(nameParts) == 2)
                {
                    List *leftLeafs = getFromTreeLeafs(singleton(j->left));
                    List *rightLeafs = getFromTreeLeafs(singleton(j->right));
                    char *fromItemName = (char *) getNthOfListP(nameParts,0);

                    DEBUG_LOG("search attr %s from FROM item %s",
                            getNthOfListP(nameParts,1), fromItemName);

                    // if named from item occurs in both subtree -> ambigious
                    if (findNamedFromItem(j->left, fromItemName) != NULL
                        &&
                        findNamedFromItem(j->right, fromItemName) != NULL)
                    {
                        FATAL_LOG("from item reference ambigious in join:\n%s",
                                beatify(nodeToString(j)));
                    }
                    // is in left subtree
                    else if (findNamedFromItem(j->left,fromItemName) != NULL)
                    {
                        newFroms = copyList(parentFroms);
                        newFroms = appendToTailOfList(newFroms, leftLeafs);
                        isFound = findQualifiedAttrRefInFrom(nameParts, a, newFroms);

                        DEBUG_LOG("is in left subtree");

                        if (isFound)
                        {
                            int offset = 0;
                            a->fromClauseItem = 0;

                            FOREACH(FromItem,leaf,leftLeafs)
                            {
                                if (streq(leaf->name,fromItemName))
                                {
                                    a->attrPosition +=offset;
                                    break;
                                }
                                offset += LIST_LENGTH(leaf->attrNames);
                            }
                        }
                    }
                    // else serach in right subtree
                    else if (findNamedFromItem(j->right,fromItemName) != NULL)
                    {
                        newFroms = copyList(parentFroms);
                        newFroms = appendToTailOfList(newFroms, rightLeafs);
                        isFound = findQualifiedAttrRefInFrom(nameParts, a, newFroms);

                        DEBUG_LOG("is in right subtree");

                        if (isFound)
                        {
                            int offset = 0;
                            a->fromClauseItem = 1;

                            FOREACH(FromItem,leaf,rightLeafs)
                            {
                                if (streq(leaf->name,fromItemName))
                                {
                                    a->attrPosition +=offset;
                                    break;
                                }
                                offset += LIST_LENGTH(leaf->attrNames);
                            }
                        }
                    }

                    if (!isFound)
                    {
                        FATAL_LOG("could not find attribute %s referenced in "
                                "condition of join:\n%s",
                                a->name,
                                beatify(nodeToString(j)));
                    }
                }
            }
        }
    }

    DEBUG_LOG("finished adapting attr refs in join conds::\n%s",
            beatify(nodeToString(fromClause)));
}

static boolean
findAttrRefInFrom (AttributeReference *a, List *fromClauses)
{
    boolean isFound = FALSE;
    int fromPos = 0, attrPos, levelsUp = 0;

    FOREACH(List,fClause,fromClauses)
    {
        FOREACH(FromItem, f, fClause)
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
                    a->outerLevelsUp = levelsUp;
                    a->attrType = getNthOfListInt(f->dataTypes, attrPos);
                }
            }
            fromPos++;
        }
        levelsUp++;
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

    // if it is a tableaccess then allow access to ROWID column
    if(strcmp(attr->name,"ROWID") == 0 && fromItem->type == T_FromTableRef)
    {
        isFound = TRUE;
        foundAttr = LIST_LENGTH(fromItem->attrNames);
        fromItem->attrNames = appendToTailOfList(fromItem->attrNames, strdup("ROWID"));
    }

    return foundAttr;
}


static boolean
findQualifiedAttrRefInFrom (List *nameParts, AttributeReference *a, List *fromClauses)
{
    boolean foundFrom = FALSE;
    boolean foundAttr = FALSE;
    int fromClauseItem = 0;
    int attrPos = 0, levelsUp = 0;
    char *tabName = (char *) getNthOfListP(nameParts, 0);
    char *attrName = (char *) getNthOfListP(nameParts, 1);
    FromItem *fromItem = NULL;
    FromItem *leafItem = NULL;

    DEBUG_LOG("looking for attribute %s.%s", tabName, attrName);

    // find table name
    FOREACH(List,fromItems,fromClauses)
    {
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
                    leafItem = foundF;
                    a->fromClauseItem = fromClauseItem;
                    a->outerLevelsUp = levelsUp;
                    foundFrom = TRUE;
                }
            }
            fromClauseItem++;
        }
        levelsUp++;
    }

    // did we find from clause item
    if (!foundFrom)
    {
        FATAL_LOG("did not find from clause item named <%s>", tabName);
        return FALSE;
    }

    // find attribute name
    FOREACH(char,aName,leafItem->attrNames)
    {
        if (strcmp(aName, attrName) == 0)
        {
            if(foundAttr)
            {
                FATAL_LOG("ambigious attr name <%s> appears more than once in "
                        "from clause item <%s>:\n%s", attrName, tabName,
                        beatify(nodeToString(leafItem)));
                return FALSE;
            }
            else
            {
                a->attrPosition = attrPos;
                a->attrType = getNthOfListInt(leafItem->dataTypes, attrPos);
                foundAttr = TRUE;
            }
        }
        attrPos++;
    }

    if (!foundAttr)
    {
        FATAL_LOG("did not find any from clause item from attribute <%s>", attrName);
        return FALSE;
    }

    // map back attribute position and original from item
    adaptAttrPosOffset(fromItem, leafItem, a);
    a->name = strdup(attrName);

    return foundAttr;
}

static void
adaptAttrPosOffset(FromItem *f, FromItem *decendent, AttributeReference *a)
{
    List *leafs = getFromTreeLeafs(singleton(f));
    int offset = 0;

    FOREACH(FromItem,l,leafs)
    {
        if (streq(l->name, decendent->name))
        {
            a->attrPosition += offset;
            break;
        }
        offset += LIST_LENGTH(l->attrNames);
    }
}


boolean
hasNestedSubqueries (Node *node)
{
    List *nested = NIL;
    boolean result;

    findNestedSubqueries (node, &nested);
    result = LIST_LENGTH(nested) != 0;
    freeList(nested);

    return result;
}

boolean
findNestedSubqueries (Node *node, List **state)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, NestedSubquery))
    {
        *state = appendToTailOfList(*state, node);
        TRACE_LOG("found nested subquery <%s>", nodeToString(node));
        return TRUE;
    }

    if (isQBQuery(node))
        return TRUE;

    return visit(node, findNestedSubqueries, state);
}

static boolean
findFunctionCall (Node *node, List **state)
{
    if(node == NULL)
        return TRUE;

    if(isA(node, FunctionCall))
        *state = appendToTailOfList(*state, node);
    else if (isA(node, WindowFunction))
    {
        *state = appendToTailOfList(*state, node);
        return TRUE;
    }

    if(isQBQuery(node))
        return TRUE;

    return visit(node, findFunctionCall, state);
}

static void
analyzeJoin (FromJoinExpr *j, List *parentFroms)
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
            analyzeJoin((FromJoinExpr *)left, parentFroms);
            break;
        case T_FromSubquery:
        {
            FromSubquery *sq = (FromSubquery *) left;
            analyzeFromSubquery(sq, parentFroms);
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
			analyzeJoin((FromJoinExpr *) right, parentFroms);
			break;
		case T_FromSubquery:
		{
			FromSubquery *sq = (FromSubquery *) right;
			analyzeFromSubquery(sq, parentFroms);
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
    	ASSERT(LIST_LENGTH(j->from.attrNames) == LIST_LENGTH(expectedAttrs));
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
        ASSERT(LIST_LENGTH(j->from.attrNames) == LIST_LENGTH(expectedAttrs));
    }

    j->from.dataTypes = CONCAT_LISTS((List *) copyObject(left->dataTypes),
            (List *) copyObject(left->dataTypes));

    DEBUG_LOG("join analysis:\n%s", beatify(nodeToString(j)));
}

static void
analyzeFromTableRef(FromTableRef *f)
{
    // attribute names already set (view or temporary view for now)
    if (f->from.attrNames == NIL)
        f->from.attrNames = getAttributeNames(f->tableId);

    if(!(f->from.dataTypes))
        f->from.dataTypes = getAttributeDataTypes(f->tableId);


    if(f->from.name == NULL)
    	f->from.name = f->tableId;
}

static void
recursiveAppendAttrNames(JsonColInfoItem *attr, List **attrNames, List **attrTypes)
{
	if (attr->nested)
	{
		FOREACH(JsonColInfoItem, attr1, attr->nested)
        		{
			if(attr1->nested)
				recursiveAppendAttrNames(attr1, attrNames, attrTypes);
			else
			{
				*attrNames = appendToTailOfList(*attrNames, attr1->attrName);
				*attrTypes = appendToTailOfListInt(*attrTypes, 5);
			}
        		}
	}
	else
	{
		*attrNames = appendToTailOfList(*attrNames, attr->attrName);
		*attrTypes = appendToTailOfListInt(*attrTypes, 5);
	}
}

static void
analyzeFromJsonTable(FromJsonTable *f, List **state)
{
	// Populate the attrnames, datatypes from columnlist
	List *attrNames = NIL;
	List *attrTypes = NIL;

	FOREACH(JsonColInfoItem, attr1, f->columns)
	{
		recursiveAppendAttrNames(attr1, &attrNames, &attrTypes);

		//TODO Add if streq for other datatypes as well
		/*
        switch (attr->attrType)
        {
        case DT_INT:
        	attrTypes = appendToTailOfListInt(attrTypes, 0);
        	break;
        case DT_LONG:
        	attrTypes = appendToTailOfListInt(attrTypes, 1);
        	break;
        case DT_STRING:
        	attrTypes = appendToTailOfListInt(attrTypes, 2);
        	break;
        case DT_FLOAT:
        	attrTypes = appendToTailOfListInt(attrTypes, 3);
        	break;
        case DT_BOOL:
        	attrTypes = appendToTailOfListInt(attrTypes, 4);
        	break;
        case DT_VARCHAR2:
        	attrTypes = appendToTailOfListInt(attrTypes, 5);
        	break;
        }
		 */
	}

	if (f->from.attrNames == NIL)
		f->from.attrNames = attrNames;

	if (f->from.dataTypes == NIL)
		f->from.dataTypes = attrTypes;

	if(f->from.name == NULL)
		f->from.name = f->jsonTableIdentifier;

	//TODO JsonColumn can refer to column of JsonTable
	// Append jsonColumn to attributeRef list
	*state = appendToTailOfList(*state, f->jsonColumn);
}

static void
analyzeInsert(Insert * f)
{
    List *attrNames = getAttributeNames(f->tableName);
//    List *dataTypes = getAttributeDataTypes(f->tableName);
    List *attrRefs = getAttributes(f->tableName);
    HashMap *attrPos = NULL;
    Set *attrNameSet = makeStrSetFromList(attrNames);

    // if user has given no attribute list, then get it from table definition
    if (f->attrList == NULL)
        f->attrList = deepCopyStringList(attrNames);
    // else use the user provided one and prepare a map from attribute name to position
    else
    {
        int i = 0;
        attrPos = NEW_MAP(Constant,Constant);

        FOREACH(char,name,f->attrList)
        {
            MAP_ADD_STRING_KEY(attrPos,name,createConstInt(i++));

            // if attribute is not an attribute of table then fail
            if (!hasSetElem(attrNameSet,name))
                FATAL_LOG("INSERT mentions attribute <%s> that is not an "
                        "attribute of table %s:<%s>",
                        name, f->tableName, stringListToString(attrNames));
        }
    }

    // is a VALUES clause
    if (isA(f->query,List))
    {
        if (LIST_LENGTH(f->attrList) != attrNames->length)
        {
//            int pos = 0;
            List *newValues = NIL;
            List *oldValues = (List *) f->query;
            INFO_LOG("The number of values are not equal to the number "
                    "attributes in the table");
            //TODO add NULL or DEFAULT values for remaining attributes
            FOREACH(AttributeDef,a,attrRefs)
            {
                Node *val = NULL;

                if (MAP_HAS_STRING_KEY(attrPos,a->attrName))
                {
                    val = getNthOfListP(oldValues,
                            INT_VALUE(MAP_GET_STRING(attrPos,a->attrName)));
                    // TODO sanity check value (e.g., no attribute references) tackle also corner cases
                }
                else
                {
                    List *nameParts = splitTableName(f->tableName);
                    Node *def = getAttributeDefaultVal(
                            (char *) getNthOfListP(nameParts, 0),
                            (char *) getNthOfListP(nameParts, 1),
                            a->attrName); //TODO get schema

                    if (def == NULL)
                        val = (Node *) createNullConst(a->dataType);
                    else
                        val = def;
                }
                newValues = appendToTailOfList(newValues, val);
            }

            f->query = (Node *) newValues;
        }
    }
    // is an INSERT INTO R (SELECT ...)
    else
    {
        QueryBlock *q = (QueryBlock *) f->query;
        analyzeQueryBlockStmt(f->query, NIL);
        //TODO check query data types
        //TODO even more important add query block for missing attributes if necessary
        if (LIST_LENGTH(f->attrList) != attrNames->length)
        {
            QueryBlock *wrap = createQueryBlock();
            List *selectClause = NIL;

            FOREACH(AttributeDef,a,attrRefs)
            {
                Node *val = NULL;
                //	            SelectItem *subItem = NULL;
                SelectItem *newItem = NULL;

                if (MAP_HAS_STRING_KEY(attrPos,a->attrName))
                {
                    //                    subItem = (SelectItem *) getNthOfListP(q->selectClause,
                    //                            INT_VALUE(MAP_GET_STRING(attrPos,a->attrName)));
                    val = (Node *) createFullAttrReference(strdup(a->attrName),
                            0,
                            INT_VALUE(MAP_GET_STRING(attrPos,a->attrName)),
                            INVALID_ATTR,
                            a->dataType);
                    // TODO sanity check DT
                }
                else
                {
                    List *nameParts = splitTableName(f->tableName);
                    Node *def = getAttributeDefaultVal(
                            (char *) getNthOfListP(nameParts, 0),
                            (char *) getNthOfListP(nameParts, 1),
                            a->attrName); //TODO get schema

                    if (def == NULL)
                        val = (Node *) createNullConst(a->dataType);
                    else
                        val = def;
                }
                newItem = createSelectItem(strdup(a->attrName),val);
                selectClause = appendToTailOfList(selectClause, newItem);
            }

            wrap->selectClause = selectClause;
            wrap->fromClause = singleton(createFromSubquery(strdup("origInsertQuery"),
                    getQBAttrNames((Node *) q), (Node *) q));
            f->query = (Node *) wrap;
        }
    }
}

static void analyzeDelete(Delete * f) {
	List *attrRefs = NIL;
	List *subqueries = NIL;
	List *attrDef = getAttributes(f->nodeName);
	List *attrNames = NIL;
	List *dataTypes = getAttributeDataTypes(f->nodeName);
	FromTableRef *fakeTable;
	List *fakeFrom = NIL;

	FOREACH(AttributeDef,a,attrDef)
		attrNames = appendToTailOfList(attrNames, strdup(a->attrName));

	fakeTable = (FromTableRef *) createFromTableRef(strdup(f->nodeName), attrNames,
			strdup(f->nodeName), dataTypes);
	fakeFrom = singleton(singleton(fakeTable));

	int attrPos = 0;

	findAttrReferences((Node *) f->cond, &attrRefs);
	FOREACH(AttributeReference,a,attrRefs) {
		boolean isFound = FALSE;

		attrPos = findAttrInFromItem((FromItem *) fakeTable, a);

		if (attrPos != INVALID_ATTR) {
			if (isFound)
				DEBUG_LOG("ambigious attribute reference %s", a->name);
			else {
				isFound = TRUE;
				a->fromClauseItem = 0;
				a->attrPosition = attrPos;
				a->outerLevelsUp = 0;
				a->attrType = getNthOfListInt(dataTypes, attrPos);
			}
		}

		if (!isFound)
			FATAL_LOG("do not find attribute %s", a->name);
	}

	// search for nested subqueries
	findNestedSubqueries(f->cond, &subqueries);

	// analyze each nested subqueries
	FOREACH(NestedSubquery,nq,subqueries)
	    analyzeQueryBlockStmt(nq->query, fakeFrom);

}

static void
analyzeUpdate(Update* f) {
	List *attrRefs = NIL;
	List *attrDef = getAttributes(f->nodeName);
	List *dataTypes = getAttributeDataTypes(f->nodeName);
	List *attrNames = NIL;
	List *subqueries = NIL;
	FromTableRef *fakeTable;
	List *fakeFrom = NIL;

	FOREACH(AttributeDef,a,attrDef)
		attrNames = appendToTailOfList(attrNames, strdup(a->attrName));

	fakeTable = (FromTableRef *) createFromTableRef(strdup(f->nodeName), attrNames,
			strdup(f->nodeName), dataTypes);
	fakeFrom = singleton(singleton(fakeTable));

//	boolean isFound = FALSE;
	int attrPos = 0;

	// find attributes
	findAttrReferences((Node *) f->cond, &attrRefs);
	findAttrReferences((Node *) f->selectClause, &attrRefs);

	// adapt attributes
	FOREACH(AttributeReference,a,attrRefs) {
		boolean isFound = FALSE;

		attrPos = findAttrInFromItem((FromItem *) fakeTable, a);

		if (attrPos != INVALID_ATTR) {
			if (isFound)
				DEBUG_LOG("ambigious attribute reference %s", a->name);
			else {
				isFound = TRUE;
				a->fromClauseItem = 0;
				a->attrPosition = attrPos;
				a->outerLevelsUp = 0;
				a->attrType = getNthOfListInt(dataTypes, attrPos);
			}
		}

		if (!isFound)
			FATAL_LOG("do not find attribute %s", a->name);
	}

	// search for nested subqueries
	findNestedSubqueries(f->cond, &subqueries);

    // analyze each nested subqueries
	FOREACH(NestedSubquery,nq,subqueries)
		analyzeQueryBlockStmt(nq->query, fakeFrom);
}

static void
analyzeFromSubquery(FromSubquery *sq, List *parentFroms)
{
    List *expectedAttrs;

	analyzeQueryBlockStmt(sq->subquery, parentFroms);
	expectedAttrs = getQBAttrNames(sq->subquery);

	// if no attr aliases given
	if (!(sq->from.attrNames))
	    sq->from.attrNames = expectedAttrs;
	sq->from.dataTypes = getQBAttrDTs(sq->subquery);

	ASSERT(LIST_LENGTH(sq->from.attrNames) == LIST_LENGTH(expectedAttrs));
}

static List *
getQBAttrDTs (Node *qb)
{
    List *DTs = NIL;

    switch(qb->type)
    {
        case T_QueryBlock:
        {
            QueryBlock *subQb = (QueryBlock *) qb;
            FOREACH(SelectItem,s,subQb->selectClause)
            {
                DTs = appendToTailOfListInt(DTs,
                        (int) typeOf(s->expr));
            }
        }
        break;
        case T_SetQuery:
        {
            SetQuery *setQ = (SetQuery *) qb;
            DTs = getQBAttrDTs(setQ->lChild);
        }
        break;
        case T_ProvenanceStmt:
        {
//            ProvenanceStmt *pStmt = (ProvenanceStmt *) qb;
            DTs = NIL; //TODO
        }
        break;
        default:
            FATAL_LOG("unexpected node type as FROM clause item: %s", beatify(nodeToString(qb)));
            break;
    }

    return DTs;
}

static List *
getQBAttrNames (Node *qb)
{
    List *attrs = NIL;

    switch(qb->type)
    {
        case T_QueryBlock:
        {
            QueryBlock *subQb = (QueryBlock *) qb;
            FOREACH(SelectItem,s,subQb->selectClause)
            {
                 attrs = appendToTailOfList(attrs,
                        s->alias);
            }
        }
        break;
        case T_SetQuery:
        {
            SetQuery *setQ = (SetQuery *) qb;
            attrs = deepCopyStringList(setQ->selectClause);
        }
        break;
        case T_ProvenanceStmt:
        {
            ProvenanceStmt *pStmt = (ProvenanceStmt *) qb;
            attrs = deepCopyStringList(pStmt->selectClause);
        }
        break;
        default:
            break;
    }

    return attrs;
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
//    int start = 0, pos = 0;
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
    List *leafItems = getFromTreeLeafs(fromClause);
    ASSERT(LIST_LENGTH(nameParts) == 1 || LIST_LENGTH(nameParts) == 2);

    // should be "*" select item -> expand to all attribute in from clause
    if (LIST_LENGTH(nameParts) == 1)
    {
        int fromAliasCount = 0;
        ASSERT(strcmp((char *) getNthOfListP(nameParts,0),"*") == 0);

        FOREACH(FromItem,f,leafItems)
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
                // do not expand ROWID column
                if (!(f->type == T_FromTableRef && strcmp(attr,"ROWID") == 0))
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
        ASSERT(strcmp(attrName,"*") == 0);

        FOREACH(FromItem,f,leafItems)
        {
            if (strcmp(f->name,tabName) == 0)
            {
                if (found)
                    FATAL_LOG("Ambiguous from clause reference <%s> to from clause item <%s>", s->alias, tabName);
                else
                {
                    FOREACH(char,attr,f->attrNames)
                    {
                        // do not expand ROWID column
                        if (!(f->type == T_FromTableRef && strcmp(attr,"ROWID") == 0))
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
    }

    DEBUG_LOG("Expanded a star expression into <%s>", nodeToString(newSelectItems));

    return newSelectItems;
}

static List *
getFromTreeLeafs (List *from)
{
    List *result = NIL;

    FOREACH(FromItem,f,from)
    {
        switch(f->type)
        {
            case T_FromJoinExpr:
            {
                FromJoinExpr *j = (FromJoinExpr *) f;
                result = CONCAT_LISTS(result,
                        getFromTreeLeafs(LIST_MAKE(j->left, j->right)));
            }
            break;
            case T_FromSubquery:
            case T_FromTableRef:
                result = appendToTailOfList(result, f);
                break;
            case T_FromJsonTable:
	        result = appendToTailOfList(result, f);
	        break;
            default:
                FATAL_LOG("expected a FROM clause item not: %s",
                        NodeTagToString(f->type));
        }
    }

    DEBUG_LOG("from leaf items are:\n%s", beatify(nodeToString(result)));

    return result;
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

static List *
splitTableName(char *tableName)
{
    List *result = NIL;
    StringInfo split = makeStringInfo();
    char *pos = tableName - 1;
    int len = strlen(tableName);
    boolean inString = FALSE;

    while(pos++ != (tableName + len))
    {
        char c = *pos;
        switch(c)
        {
            case '.':
                if (!inString)
                {
                    result = appendToTailOfList(result, strdup(split->data));
                    resetStringInfo(split);
                }
                appendStringInfoChar(split,*pos);
                break;
            case '"':
                if (inString)
                    inString = FALSE;
                else
                    inString = TRUE;
                break;
            case '\\':
                if (inString)
                    pos++;
                break;
            default:
                appendStringInfoChar(split,*pos);
                break;
        }
    }
    result = appendToTailOfList(result, strdup(split->data));

    // if no schema is given, use connection user part
    if (LIST_LENGTH(result) == 1)
        result = appendToHeadOfList(result,
                strToUpper(getStringOption("connection.user")));
    FREE(split->data);

    return result;
}



static void
analyzeSetQuery (SetQuery *q, List *parentFroms)
{
    analyzeQueryBlockStmt(q->lChild, parentFroms);
    analyzeQueryBlockStmt(q->rChild, parentFroms);

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

/*
 * Analyze a provenance computation. The main part is to figure out the attributes
 */

static void
analyzeProvenanceStmt (ProvenanceStmt *q, List *parentFroms)
{

    switch (q->inputType) {
        case PROV_INPUT_TRANSACTION:
        {
            //TODO need to know updates at this point
        }
        break;
        case PROV_INPUT_UPDATE:
        {

        }
        break;
        case PROV_INPUT_QUERY:
        {
            analyzeQueryBlockStmt(q->query, parentFroms);

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
        break;
        case PROV_INPUT_TIME_INTERVAL:
            break;
        case PROV_INPUT_UPDATE_SEQUENCE:
            break;
        default:
            break;
    }

	analyzeProvenanceOptions(q);
}

static void
analyzeProvenanceOptions (ProvenanceStmt *prov)
{
    /* loop through options */
    FOREACH(KeyValue,kv,prov->options)
    {
        char *key = STRING_VALUE(kv->key);
        char *value = STRING_VALUE(kv->value);

        /* provenance type */
        if (!strcmp(key, "TYPE"))
        {
            if (!strcmp(value, "PICS"))
                prov->provType = PROV_PI_CS;
            else if (!strcmp(value, "TRANSFORMATION"))
                prov->provType = PROV_TRANSFORMATION;
            else
                FATAL_LOG("Unkown provenance type: <%s>", value);
        }
    }
}

static void
analyzeWithStmt (WithStmt *w)
{
    Set *viewNames = STRSET();
    List *analyzedViews = NIL;

    // check that no two views have the same name
    FOREACH(KeyValue,v,w->withViews)
    {
        char *vName = STRING_VALUE(v->key);
        if (hasSetElem(viewNames, vName))
            FATAL_LOG("view <%s> defined more than once in with stmt:\n\n%s",
                    vName, nodeToString(w));
        else
            addToSet(viewNames, vName);
    }

    // analyze each view, but make sure to set attributes of dummy views upfront
    FOREACH(KeyValue,v,w->withViews)
    {
        setViewFromTableRefAttrs(v->value, analyzedViews);
        DEBUG_LOG("did set view table refs:\n%s", beatify(nodeToString(v->value)));
        analyzeQueryBlockStmt(v->value, NIL);
        analyzedViews = appendToTailOfList(analyzedViews, v);
    }

    setViewFromTableRefAttrs(w->query, analyzedViews);
    DEBUG_LOG("did set view table refs:\n%s", beatify(nodeToString(w->query)));
    analyzeQueryBlockStmt(w->query, NIL);

    DEBUG_LOG("analyzed view is:\n%s", beatify(nodeToString(w->query)));
}

static boolean
setViewFromTableRefAttrs(Node *node, List *views)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, FromTableRef))
    {
        FromTableRef *f = (FromTableRef *) node;
        char *name = f->tableId;

        FOREACH(KeyValue,v,views)
        {
            char *vName = STRING_VALUE(v->key);

            // found view, set attr names
            if (strcmp(name, vName) == 0)
            {
                ((FromItem *) f)->attrNames = getQBAttrNames(v->value);
                ((FromItem *) f)->dataTypes = getQBAttrDTs  (v->value);
            }
        }

        return TRUE;
    }

    return visit(node, setViewFromTableRefAttrs, views);
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

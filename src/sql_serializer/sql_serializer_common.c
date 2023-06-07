/*-----------------------------------------------------------------------------
 *
 * sql_serializer_common.c
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
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"

#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "utility/string_utils.h"

#include "sql_serializer/sql_serializer_common.h"
#include "sql_serializer/sql_serializer.h"

#define TEMP_VIEW_NAME_PATTERN "_temp_view_%u"

static boolean quoteAttributeNamesVisitQO (QueryOperator *op, void *context);
static boolean quoteAttributeNames (Node *node, void *context);
static char *createViewName (SerializeClausesAPI *api);
static boolean renameAttrsVisitor (Node *node, JoinAttrRenameState *state);
static char *createAttrName (char *name, int fItem);

/*
 * create API struct
 */

SerializeClausesAPI *
createAPIStub (void)
{
    SerializeClausesAPI *api = NEW(SerializeClausesAPI);

    api->serializeQueryOperator = genSerializeQueryOperator;
    api->serializeQueryBlock = genSerializeQueryBlock;
    api->serializeProjectionAndAggregation = NULL;
    api->serializeFrom = genSerializeFrom;
    api->serializeWhere = genSerializeWhere;
    api->serializeSetOperator = NULL;
    api->serializeFromItem = genSerializeFromItem;
    api->serializeTableAccess = NULL;
    api->serializeConstRel = NULL;
    api->serializeJoinOperator = NULL;
	api->serializeLimitOperator = genSerializeLimitOperator;
	api->serializeOrderByOperator = genSerializeOrderByOperator;
    api->createTempView = genCreateTempView;
    api->tempViewMap = NEW_MAP(Constant, Node);
    api->viewCounter = 0;

    return api;
}

void
genQuoteAttributeNames (Node *q)
{
    // quote ident names if necessary
    ASSERT(IS_OP(q) || isA(q,List));
    if (isA(q,List))
    {
        FOREACH(QueryOperator,el, (List *) q)
            visitQOGraph((QueryOperator *) el, TRAVERSAL_PRE, quoteAttributeNamesVisitQO, NULL);
    }
    else
        visitQOGraph((QueryOperator *) q, TRAVERSAL_PRE, quoteAttributeNamesVisitQO, NULL);
}

static boolean
quoteAttributeNamesVisitQO (QueryOperator *op, void *context)
{
    return quoteAttributeNames((Node *) op, op);
}

static boolean
quoteAttributeNames (Node *node, void *context)
{
     if (node == NULL)
        return TRUE;

    // do not traverse into query operator nodes to avoid repeated traversal of paths in the graph
    if (node != context && IS_OP(node))
        return TRUE;

    if (isA(node, AttributeReference))
    {
        AttributeReference *a = (AttributeReference *) node;
        a->name = quoteIdentifier(a->name);
    }
    if (isA(node, SelectItem))
    {
        SelectItem *a = (SelectItem *) node;
        a->alias = quoteIdentifier(a->alias);
    }
    if (isA(node, AttributeDef))
    {
        AttributeDef *a = (AttributeDef *) node;
        a->attrName = quoteIdentifier(a->attrName);
    }

    return visit(node, quoteAttributeNames, context);
}

/*
 * Serialize a SQL query block (SELECT ... FROM ... WHERE ...)
 */
List *
genSerializeQueryBlock (QueryOperator *q, StringInfo str, SerializeClausesAPI *api)
{
    QueryBlockMatch *matchInfo = NEW(QueryBlockMatch);
    StringInfo fromString = makeStringInfo();
    StringInfo whereString = makeStringInfo();
    StringInfo selectString = makeStringInfo();
    StringInfo groupByString = makeStringInfo();
    StringInfo havingString = makeStringInfo();
	StringInfo orderString = makeStringInfo();
	StringInfo limitOffsetString = makeStringInfo();
    MatchState state = MATCH_START;
    QueryOperator *cur = q;
    List *attrNames = getAttrNames(q->schema);
    boolean topMaterialize = HAS_STRING_PROP(cur,PROP_MATERIALIZE);

    // do the matching
    while(state != MATCH_NEXTBLOCK && cur != NULL)
    {
        DEBUG_LOG("STATE: %s", OUT_MATCH_STATE(state));
        DEBUG_LOG("Operator %s", operatorToOverviewString((Node *) cur));
        // first check that cur does not have more than one parent
        if (HAS_STRING_PROP(cur,PROP_MATERIALIZE) || LIST_LENGTH(cur->parents) > 1)
        {
            if (cur != q)
            {
                matchInfo->fromRoot = cur;
                state = MATCH_NEXTBLOCK;
                cur = OP_LCHILD(cur);
                continue;
            }
        }
        // if cur has not more than one parent and should not be materialized
        switch(cur->type)
        {
            case T_JoinOperator:
            case T_TableAccessOperator:
            case T_ConstRelOperator :
            case T_SetOperator:
            case T_RecursiveOperator: //Not sure about this one
            case T_JsonTableOperator:
                matchInfo->fromRoot = cur;
                state = MATCH_NEXTBLOCK;
                cur = OP_LCHILD(cur);
                continue;
                break;
            default:
                break;
        }
        switch(state)
        {
            /* START state */
            case MATCH_START:
            case MATCH_DISTINCT:
            case MATCH_ORDER:
		    case MATCH_LIMIT:
            {
                switch(cur->type)
                {
                    case T_SelectionOperator:
                    {
                        QueryOperator *child = OP_LCHILD(cur);
                        /* HAVING */
                        if (isA(child,AggregationOperator))
                        {
                            matchInfo->having = (SelectionOperator *) cur;
                            state = MATCH_HAVING;
                        }
                        /* WHERE */
                        else
                        {
                            matchInfo->where = (SelectionOperator *) cur;
                            state = MATCH_WHERE;
                        }
                    }
                    break;
                    case T_ProjectionOperator:
                    {
                        QueryOperator *child = OP_LCHILD(cur);
                        QueryOperator *grandChild = (child ? OP_LCHILD(child) : NULL);

                        // is first projection?
                        if (isA(child,AggregationOperator)
                                || (isA(child,SelectionOperator)
                                        && isA(grandChild,AggregationOperator))
                                || (isA(child,WindowOperator)))
                        {
                            matchInfo->firstProj = (ProjectionOperator *) cur;
                            state = MATCH_FIRST_PROJ;
                        }
                        else
                        {
                            matchInfo->secondProj = (ProjectionOperator *) cur;
                            state = MATCH_SECOND_PROJ;
                        }
                    }
                    break;
                    case T_DuplicateRemoval:
                        if (state == MATCH_START || state == MATCH_ORDER || state == MATCH_LIMIT)
                        {
                            matchInfo->distinct = (DuplicateRemoval *) cur;
                            state = MATCH_DISTINCT;
                        }
                        else
                        {
                            matchInfo->fromRoot = cur;
                            state = MATCH_NEXTBLOCK;
                        }
                        break;
                    case T_OrderOperator:
                        if (state == MATCH_START || state == MATCH_LIMIT)
                        {
                            matchInfo->orderBy = (OrderOperator *) cur;
                            state = MATCH_ORDER;
                        }
                        else
                        {
                            matchInfo->fromRoot = cur;
                            state = MATCH_NEXTBLOCK;
                        }
                        break;
                    case T_AggregationOperator:
                        matchInfo->aggregation = (AggregationOperator *) cur;
                        state = MATCH_AGGREGATION;
                        break;
                    case T_WindowOperator:
                        matchInfo->windowRoot = (WindowOperator *) cur;
                        state = MATCH_WINDOW;
                        break;
				    case T_LimitOperator:
						if (state == MATCH_START)
                        {
							matchInfo->limitOffset = (LimitOperator *) cur;
							state = MATCH_LIMIT;
                        }
                        else
                        {
                            matchInfo->fromRoot = cur;
                            state = MATCH_NEXTBLOCK;
                        }
						break;
                    default:
                        matchInfo->fromRoot = cur;
                        state = MATCH_NEXTBLOCK;
                        break;
                }
            }
            break;
            case MATCH_FIRST_PROJ:
            {
                switch(cur->type)
                {
                    case T_SelectionOperator:
                    {
                        QueryOperator *child = OP_LCHILD(cur);
                        if (child->type == T_AggregationOperator)
                        {
                            matchInfo->having = (SelectionOperator *) cur;
                            state = MATCH_HAVING;
                        }
                    }
                    break;
                    case T_AggregationOperator:
                        matchInfo->aggregation= (AggregationOperator *) cur;
                        state = MATCH_AGGREGATION;
                        break;
                    case T_WindowOperator:
                        matchInfo->windowRoot = (WindowOperator *) cur;
                        state = MATCH_WINDOW;
                        break;
                    default:
                        FATAL_LOG("After matching first projection we should "
                                "match selection or aggregation and not %s",
                                nodeToString(cur));
                    break;
                }
            }
            break;
            case MATCH_HAVING:
            {
                switch(cur->type)
                {
                    case T_AggregationOperator:
                    {
                        matchInfo->aggregation = (AggregationOperator *) cur;
                        state = MATCH_AGGREGATION;
                    }
                    break;
                    default:
                           FATAL_LOG("after matching having we should match "
                                   "aggregation and not %s", nodeToString(cur));
                    break;
                }
            }
            break;
            case MATCH_AGGREGATION:
            {
                switch(cur->type)
                {
                    case T_SelectionOperator:
                    {
                        matchInfo->where = (SelectionOperator *) cur;
                        state = MATCH_WHERE;
                    }
                    break;
                    case T_ProjectionOperator:
                        matchInfo->secondProj = (ProjectionOperator *) cur;
                        state = MATCH_SECOND_PROJ;
                        break;
                    default:
                        matchInfo->fromRoot = cur;
                        state = MATCH_NEXTBLOCK;
                    break;
                }
            }
            break;
            case MATCH_SECOND_PROJ:
            {
                switch(cur->type)
                {
                    case T_SelectionOperator:
                    {
                        matchInfo->where = (SelectionOperator *) cur;
                        state = MATCH_WHERE;
                    }
                    break;
                    default:
                        matchInfo->fromRoot = cur;
                        state = MATCH_NEXTBLOCK;
                    break;
                }
            }
            break;
            case MATCH_WHERE:
            {
                matchInfo->fromRoot = cur;
                state = MATCH_NEXTBLOCK;
            }
            break;
            case MATCH_WINDOW:
            {
                switch(cur->type)
                {
                    case T_WindowOperator:
                        // do nothing
                    break;
                    case T_SelectionOperator:
                    {
                        matchInfo->where = (SelectionOperator *) cur;
                        state = MATCH_WHERE;
                    }
                    break;
                   case T_ProjectionOperator:
                   {
                       matchInfo->secondProj = (ProjectionOperator *) cur;
                       state = MATCH_SECOND_PROJ;
                   }
                   break;
                   default:
                       matchInfo->fromRoot = cur;
                       state = MATCH_NEXTBLOCK;
                   break;
                }
            }
            break;
            case MATCH_NEXTBLOCK:
                FATAL_LOG("should not end up here because we already"
                        " have reached MATCH_NEXTBLOCK state");
                break;
        }

        // go to child of cur
        cur = OP_LCHILD(cur);
    }

    OUT_BLOCK_MATCH(DEBUG,matchInfo, "query block full match");

    // translate each clause
    DEBUG_LOG("serializeFrom");
    List *fromAttrs = NIL;
    api->serializeFrom(matchInfo->fromRoot, fromString, &fromAttrs, api);

    DEBUG_LOG("serializeWhere");
    if(matchInfo->where != NULL)
        api->serializeWhere(matchInfo->where, whereString, fromAttrs, api);

    DEBUG_LOG("serialize projection + aggregation + groupBy +  having + window functions");
    api->serializeProjectionAndAggregation(matchInfo, selectString, havingString,
            groupByString, fromAttrs, topMaterialize, api);

	DEBUG_LOG("serializeOrder");
	if (matchInfo->orderBy != NULL)
		api->serializeOrderByOperator(matchInfo->orderBy, orderString, fromAttrs, api);

	DEBUG_LOG("serializeLimit");
	if (matchInfo->limitOffset != NULL)
		api->serializeLimitOperator(matchInfo->limitOffset, limitOffsetString, api);

    // put everything together
    DEBUG_LOG("mergePartsTogether");
    //TODO DISTINCT
    if (STRINGLEN(selectString) > 0)
	{
        appendStringInfoString(str, selectString->data);
	}
    else
	{
        appendStringInfoString(str, "\nSELECT *");
	}

    appendStringInfoString(str, fromString->data);

    if (STRINGLEN(whereString) > 0)
	{
        appendStringInfoString(str, whereString->data);
	}

    if (STRINGLEN(groupByString) > 0)
	{
        appendStringInfoString(str, groupByString->data);
	}

    if (STRINGLEN(havingString) > 0)
	{
        appendStringInfoString(str, havingString->data);
	}

	if (STRINGLEN(orderString) > 0)
	{
		appendStringInfoString(str, orderString->data);
	}

	if (STRINGLEN(limitOffsetString) > 0)
	{
		appendStringInfoString(str, limitOffsetString->data);
	}

    FREE(matchInfo);

    return attrNames;
}

void
genSerializeFrom (QueryOperator *q, StringInfo from, List **fromAttrs, SerializeClausesAPI *api)
{
    int curFromItem = 0, attrOffset = 0;

    appendStringInfoString(from, "\nFROM ");
    api->serializeFromItem (q, q, from, &curFromItem, &attrOffset, fromAttrs, api);
}

void
genSerializeFromItem (QueryOperator *fromRoot, QueryOperator *q, StringInfo from, int *curFromItem,
        int *attrOffset, List **fromAttrs, SerializeClausesAPI *api)
{
    // if operator has more than one parent then it will be represented as a CTE
    // however, when create the code for a CTE (q==fromRoot) then we should create SQL for this op)
    if (!(LIST_LENGTH(q->parents) > 1 || HAS_STRING_PROP(q, PROP_MATERIALIZE)) || q == fromRoot)
    {
        switch(q->type)
        {
            // Join expressions
            case T_JoinOperator:
            {
                JoinOperator *j = (JoinOperator *) q;
                api->serializeJoinOperator(from, fromRoot, j, curFromItem,
                        attrOffset, fromAttrs, api);
            }
            break;
            // Table Access
            case T_TableAccessOperator:
            {
                TableAccessOperator *t = (TableAccessOperator *) q;
                api->serializeTableAccess(from, t, curFromItem, fromAttrs,
                        attrOffset, api);
            }
            break;
            // A constant relation, turn into (SELECT ... FROM dual) subquery
            case T_ConstRelOperator:
            {
                ConstRelOperator *t = (ConstRelOperator *) q;
                api->serializeConstRel(from, t, fromAttrs, curFromItem, api);
            }
            break;
            default:
            {
                List *attrNames;

                appendStringInfoString(from, "(");
                attrNames = api->serializeQueryOperator(q, from, (QueryOperator *) getNthOfListP(q->parents,0), api); //TODO ok to use first?
                *fromAttrs = appendToTailOfList(*fromAttrs, attrNames);
                appendStringInfo(from, ") F%u", (*curFromItem)++);
            }
            break;
        }
    }
    else
    {
        // A materialization point or WITH
        {
            List *attrNames;

            appendStringInfoString(from, "(");
            attrNames = api->serializeQueryOperator(q, from, (QueryOperator *) getNthOfListP(q->parents,0), api); //TODO ok to use first?
            *fromAttrs = appendToTailOfList(*fromAttrs, attrNames);
            appendStringInfo(from, ") F%u", (*curFromItem)++);
        }
    }
}

/*
 * Translate a selection into a WHERE clause
 */
void
genSerializeWhere (SelectionOperator *q, StringInfo where, List *fromAttrs, SerializeClausesAPI *api)
{
    appendStringInfoString(where, "\nWHERE ");
    updateAttributeNames((Node *) q->cond, (List *) fromAttrs);
    appendStringInfoString(where, exprToSQL(q->cond, NULL));
}

void
genSerializeLimitOperator (LimitOperator *q, StringInfo limit, SerializeClausesAPI *api)
{
	if (q->limitExpr != NULL)
	{
		appendStringInfoString(limit, "\nLIMIT ");
		appendStringInfo(limit, "%s", exprToSQL(q->limitExpr, NULL));
	}
	if (q->offsetExpr != NULL)
	{
		appendStringInfoString(limit, "\nOFFSET ");
		appendStringInfo(limit, "%s", exprToSQL(q->offsetExpr, NULL));
	}
}

void
genSerializeOrderByOperator (OrderOperator *q, StringInfo order, List *fromAttrs,
							 SerializeClausesAPI *api) //TODO check since copied from Oracle
{
	appendStringInfoString(order, "\nORDER BY ");
    updateAttributeNames((Node *) q->orderExprs, (List *) fromAttrs);

    char *ordExpr = replaceSubstr(exprToSQL((Node *) q->orderExprs, NULL),"(","");
    ordExpr = replaceSubstr(ordExpr,")","");
    ordExpr = replaceSubstr(ordExpr,"'","");
    appendStringInfoString(order, ordExpr);
}


boolean
updateAttributeNames(Node *node, List *fromAttrs)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, AttributeReference))
    {
        AttributeReference *a = (AttributeReference *) node;
        char *newName;
        List *outer = NIL;
        int fromItem = -1;
        int attrPos = 0;

        // LOOP THROUGH fromItems (outer list)
        FOREACH(List, attrs, fromAttrs)
        {
            attrPos += LIST_LENGTH(attrs);
            fromItem++;
            if (attrPos > a->attrPosition)
            {
                outer = attrs;
                break;
            }
        }
        attrPos = a->attrPosition - attrPos + LIST_LENGTH(outer);
        newName = getNthOfListP(outer, attrPos);
        a->name = CONCAT_STRINGS("F", gprom_itoa(fromItem), ".", newName);;
    }

    return visit(node, updateAttributeNames, fromAttrs);
}

/*
 * Main entry point for serialization.
 */
List *
genSerializeQueryOperator (QueryOperator *q, StringInfo str, QueryOperator *parent, SerializeClausesAPI *api)
{
    // operator with multiple parents
    if (LIST_LENGTH(q->parents) > 1 || HAS_STRING_PROP(q,PROP_MATERIALIZE))
        return api->createTempView (q, str, parent, api);
    else if (isA(q, SetOperator))
        return api->serializeSetOperator(q, str, api);
    else
        return api->serializeQueryBlock(q, str, api);
}

/*
 * Create a temporary view
 */
List *
genCreateTempView (QueryOperator *q, StringInfo str, QueryOperator *parent, SerializeClausesAPI *api)
{
    StringInfo viewDef = makeStringInfo();
    char *viewName = createViewName(api);
    HashMap *tempViewMap = api->tempViewMap;
    List *resultAttrs;
    HashMap *view;

    // check whether we already have create a view for this op

    if (MAP_HAS_POINTER(tempViewMap, q))
    {
        view = (HashMap *) MAP_GET_POINTER(tempViewMap, q);
        char *name = strdup(TVIEW_GET_NAME(view));

//        if (isA(parent, SetOperator))
            appendStringInfo(str, "SELECT * FROM %s", name);
//        else
//            appendStringInfoString(str, name);

        return deepCopyStringList(TVIEW_GET_ATTRNAMES(view));
    }

    // create sql code to create view
    appendStringInfo(viewDef, "%s AS (", viewName);
    if (isA(q, SetOperator))
        resultAttrs = api->serializeSetOperator(q, viewDef, api);
    else
        resultAttrs = api->serializeQueryBlock(q, viewDef, api);

    appendStringInfoString(viewDef, ")");

    DEBUG_LOG("created view definition:\n%s for %p", viewDef->data, q);

    // add reference to view
//    if (isA(parent, SetOperator))
        appendStringInfo(str, "SELECT * FROM %s", strdup(viewName));
//    else
//        appendStringInfoString(str, strdup(viewName));

    // add to view table
    view = NEW_MAP(Constant,Node);
    TVIEW_SET_NAME(view, strdup(viewName));
    TVIEW_SET_DEF(view, strdup(viewDef->data));
    TVIEW_SET_ATTRNAMES(view, resultAttrs);
    MAP_ADD_POINTER(tempViewMap, q, view);

    return resultAttrs;
}

static char *
createViewName (SerializeClausesAPI *api)
{
    StringInfo str = makeStringInfo();

    appendStringInfo(str, TEMP_VIEW_NAME_PATTERN, api->viewCounter++);

    return str->data;
}

char *
exprToSQLWithNamingScheme (Node *expr, int rOffset, List *fromAttrs)
{
    JoinAttrRenameState *state = NEW(JoinAttrRenameState);

    state->rightFromOffsets = rOffset;
    state->fromAttrs = fromAttrs;
    renameAttrsVisitor(expr, state);

    FREE(state);
    return exprToSQL(expr, NULL);
}

static boolean
renameAttrsVisitor (Node *node, JoinAttrRenameState *state)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, AttributeReference))
    {
        AttributeReference *a = (AttributeReference *) node;
        boolean isRight = (a->fromClauseItem == 0) ? FALSE : TRUE;
        int pos = 0, fPos = 0;
        int rOffset = state->rightFromOffsets;
        ListCell *lc;
        char *name;
        List *from = NIL;

        // if right join input find first from item from right input
        if (isRight)
            for(lc = getHeadOfList(state->fromAttrs); fPos < rOffset; lc = lc->next, fPos++)
                ;
        else
            lc = getHeadOfList(state->fromAttrs);

        // find from position and attr name
        for(; lc != NULL; lc = lc->next)
        {
            List *attrs = (List *) LC_P_VAL(lc);
            pos += LIST_LENGTH(attrs);
            if (pos > a->attrPosition)
            {
                from = attrs;
                break;
            }
            fPos++;
        }

        pos = a->attrPosition - pos + LIST_LENGTH(from);
        name = getNthOfListP(from, pos);

        a->name = createAttrName(name, fPos);

        return TRUE;
    }

    return visit(node, renameAttrsVisitor, state);
}

static char *
createAttrName (char *name, int fItem)
{
   StringInfo str = makeStringInfo();
   char *result = NULL;

   appendStringInfo(str, "F%u.%s", fItem, name);
   result = str->data;
   FREE(str);

   return result;
}

boolean
updateAggsAndGroupByAttrs(Node *node, UpdateAggAndGroupByAttrState *state)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, AttributeReference))
    {
        AttributeReference *a = (AttributeReference *) node;
        char *newName;
        int attrPos = a->attrPosition;

        // is aggregation function
        if (attrPos < LIST_LENGTH(state->aggNames))
            newName = strdup(getNthOfListP(state->aggNames, attrPos));
        else
        {
            attrPos -= LIST_LENGTH(state->aggNames);
            newName = strdup(getNthOfListP(state->groupByNames, attrPos));
        }
        DEBUG_LOG("attr <%d> is <%s>", a->attrPosition, newName);

        a->name = newName;
    }

    return visit(node, updateAggsAndGroupByAttrs, state);
}

boolean
updateAttributeNamesSimple(Node *node, List *attrNames)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, AttributeReference))
    {
        AttributeReference *a = (AttributeReference *) node;
        char *newName = getNthOfListP(attrNames, a->attrPosition);
        a->name = strdup(newName);
    }

    return visit(node, updateAttributeNamesSimple, attrNames);
}

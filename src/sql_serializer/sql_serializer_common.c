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
#include "configuration/option.h"
#include "metadata_lookup/metadata_lookup.h"
#include "model/expression/expression.h"
#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "utility/string_utils.h"

#include "sql_serializer/sql_serializer_common.h"
#include "sql_serializer/sql_serializer.h"


//#define TEMP_VIEW_NAME_PATTERN "_temp_view_%u"
#define TEMP_VIEW_NAME_PATTERN "temp_view_%u"


static boolean quoteAttributeNamesVisitQO (QueryOperator *op, void *context);
static boolean quoteAttributeNames (Node *node, void *context);
static char *createViewName (SerializeClausesAPI *api);
static boolean renameAttrsVisitor (Node *node, JoinAttrRenameState *state);
static char *createAttrName (char *name, int fItem, FromAttrsContext *fac);
static HashMap *getNestAttrMap(QueryOperator *op, FromAttrsContext *fac, SerializeClausesAPI *api);
static void setNestAttrMap(QueryOperator *op, HashMap **map, FromAttrsContext *fac, SerializeClausesAPI *api);

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
	api->serializeExecPreparedOperator = genSerializeExecPreparedOperator;
	api->serializePreparedStatment = genSerializePreparedStatement;
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

FromAttrsContext *
initializeFromAttrsContext ()
{
  	struct FromAttrsContext *fac;
  	fac =  CALLOC(sizeof(FromAttrsContext),1);
  	fac->fromAttrsList = NIL;
  	fac->fromAttrs = NIL;

  	return fac;
}

FromAttrsContext *
copyFromAttrsContext(FromAttrsContext *fac)
{
  	struct FromAttrsContext *result;
  	result =  CALLOC(sizeof(FromAttrsContext),1);
  	result->fromAttrsList = copyList(fac->fromAttrsList);
  	result->fromAttrs = copyList(fac->fromAttrs);

  	return result;
}

void
printFromAttrsContext(FromAttrsContext *fac)
{
     DEBUG_LOG("FromAttrsContext:");
     if(fac->fromAttrsList != NIL)
     {
     StringInfo s1 = makeStringInfo();
     appendStringInfo(s1,"Len: %d, FromAttrsContext->fromAttrsList: ", LIST_LENGTH(fac->fromAttrsList));
     FOREACH(List, l1, fac->fromAttrsList)
     {
     	 FOREACH(List, l2, l1)
		{
     	    appendStringInfo(s1, "(");
		 	 FOREACH(char, c, l2)
			    appendStringInfo(s1, " %s ", c);
			 	 //DEBUG_LOG("%s",c);
		 	appendStringInfo(s1, ")");
		}
     	appendStringInfo(s1, " , ");
     }
     DEBUG_LOG(" %s ", s1->data);
     }
     else
    	 	 DEBUG_LOG("FromAttrsContext->fromAttrsList: NULL");

     if(fac->fromAttrs != NIL)
     {
     StringInfo s2 = makeStringInfo();
     appendStringInfo(s2,"Len: %d, FromAttrsContext->fromAttrs: ",LIST_LENGTH(fac->fromAttrs));
     FOREACH(List, l1, fac->fromAttrs)
     {
    	    appendStringInfo(s2, "(");
 	 	 FOREACH(char, c, l1)
		 	 appendStringInfo(s2, " %s ", c);
 	 	appendStringInfo(s2, ")");
     }
     DEBUG_LOG(" %s ", s2->data);
     }
     else
    	 DEBUG_LOG("FromAttrsContext->fromAttrs: NULL");
}

static HashMap *
getNestAttrMap(QueryOperator *op, FromAttrsContext *fac, SerializeClausesAPI *api)
{
	HashMap *map = NEW_MAP(Constant, Constant);
	setNestAttrMap(op, &map, fac, api);

	return map;
}

static void
setNestAttrMap(QueryOperator *op, HashMap **map, FromAttrsContext *fac, SerializeClausesAPI *api)
{
	if(isA(op, NestingOperator))
	{
		NestingOperator *nest = (NestingOperator *) op;

		List *names = getAttrNames(op->schema);
		char *nestName = (char *) getTailOfListP(names);
		DEBUG_LOG("nestName %s", nestName);

		if(!hasMapStringKey(*map, nestName))
		{
			StringInfo s = makeStringInfo();
			if(nest->nestingType == NESTQ_EXISTS)
				appendStringInfoString(s, "EXISTS ");
			else if(nest->nestingType == NESTQ_ANY)
			{
				Operator *cond = (Operator *) nest->cond;
				Node *a = getHeadOfListP(cond->args);
				char *name = exprToSQL(a, *map, FALSE);
				appendStringInfo(s, "%s %s ANY ", name, cond->name);
				/* appendStringInfoString(s, name); */
				/* appendStringInfoString(s, cond->name); */
				/* appendStringInfoString(s, " ANY "); */
			}
			else if(nest->nestingType == NESTQ_ALL)
			{
				Operator *cond = (Operator *) nest->cond;
				Node *a = getHeadOfListP(cond->args);
				char *name = exprToSQL(a, *map, FALSE);
				appendStringInfo(s, "%s %s ALL ", name, cond->name);
				/* appendStringInfoString(s, name); */
				/* appendStringInfoString(s, cond->name); */
				/* appendStringInfoString(s, " ALL "); */
			}

			appendStringInfoString(s, "(");
			api->serializeQueryOperator(OP_RCHILD(op), s, NULL, fac, api);
			appendStringInfoString(s, ")");
			DEBUG_LOG("serialized nested subquery: %s", s->data);
			MAP_ADD_STRING_KEY(*map, strdup(nestName), createConstString(s->data));
		}
	}

	FOREACH(QueryOperator, o, op->inputs)
		setNestAttrMap(o, map, fac, api);
}

/*
 * Serialize a SQL query block (SELECT ... FROM ... WHERE ...)
 */
List *
genSerializeQueryBlock (QueryOperator *q, StringInfo str, FromAttrsContext *fac, SerializeClausesAPI *api)
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
            case T_JsonTableOperator:
            case T_NestingOperator:
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
    //List *fromAttrs = NIL;
    //sine want to use fac->fromAttrs as a local variable in serializeFrom
    FromAttrsContext *cfac = copyFromAttrsContext(fac);
    cfac->fromAttrs = NIL;
    api->serializeFrom(matchInfo->fromRoot, fromString, cfac, api);

    DEBUG_LOG("serializeWhere");
    if(matchInfo->where != NULL)
        api->serializeWhere(matchInfo->where, whereString, cfac, api);

    DEBUG_LOG("serialize projection + aggregation + groupBy +  having + window functions");
    api->serializeProjectionAndAggregation(matchInfo, selectString, havingString,
            groupByString, cfac, topMaterialize, api);

	DEBUG_LOG("serializeOrder");
	if (matchInfo->orderBy != NULL)
		api->serializeOrderByOperator(matchInfo->orderBy, orderString, cfac, api);

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
genSerializeFrom (QueryOperator *q, StringInfo from, FromAttrsContext *fac, SerializeClausesAPI *api)
{
    int curFromItem = 0, attrOffset = 0;

    appendStringInfoString(from, "\nFROM ");
    api->serializeFromItem (q, q, from, &curFromItem, &attrOffset, fac, api);
}

void
genSerializeFromItem (QueryOperator *fromRoot, QueryOperator *q, StringInfo from, int *curFromItem,
        int *attrOffset, FromAttrsContext *fac, SerializeClausesAPI *api)
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
                        attrOffset, fac, api);
            }
            break;
            // Table Access
            case T_TableAccessOperator:
            {
                TableAccessOperator *t = (TableAccessOperator *) q;
                api->serializeTableAccess(from, t, curFromItem, fac,
                        attrOffset, api);
            }
            break;
            // A constant relation, turn into (SELECT ... FROM dual) subquery
            case T_ConstRelOperator:
            {
                ConstRelOperator *t = (ConstRelOperator *) q;
                api->serializeConstRel(from, t, fac, curFromItem, api);
            }
            break;
            case T_NestingOperator:
            {
            		NestingOperator *nest = (NestingOperator *) q;

            		char *subAttr = getTailOfListP(getQueryOperatorAttrNames(q));
            		QueryOperator *input = OP_LCHILD(nest);
            		QueryOperator *subquery = OP_RCHILD(nest);
            		//List *subqueryNames = getQueryOperatorAttrNames(subquery);

            		api->serializeFromItem(fromRoot, input, from, curFromItem, attrOffset, fac, api);

            		fac->fromAttrs = appendToTailOfList(fac->fromAttrs, LIST_MAKE(strdup(subAttr)));

            		//fromAttrsList: ( ((C,D)) , ((A,B), (nesting)) ) -> format: (L1, L2)
            		//here (((A,B))) -> (((A,B), (nesting_eval_1)))
            		fac->fromAttrsList = removeFromHead(fac->fromAttrsList);
            		fac->fromAttrsList = appendToHeadOfList(fac->fromAttrsList, copyList(fac->fromAttrs));
            		printFromAttrsContext(fac);

            		AttributeDef *a = getTailOfListP(GET_OPSCHEMA(subquery)->attrDefs);
            		a->attrName = strdup(subAttr);
            		//api->serializeQueryOperator(subquery, from, (QueryOperator *) nest, fac, api);
            		//appendStringInfoString(from, ")");
            		//appendStringInfo(from, " F%u_0", (*curFromItem)++);
            }
            break;
            default:
            {
                List *attrNames;

                appendStringInfoString(from, "(");
                attrNames = api->serializeQueryOperator(q, from, (QueryOperator *) getNthOfListP(q->parents,0), fac, api); //TODO ok to use first?
                fac->fromAttrs = appendToTailOfList(fac->fromAttrs, attrNames);
                //fac->fromAttrsList = removeFromHead(fac->fromAttrsList);
                fac->fromAttrsList = appendToHeadOfList(fac->fromAttrsList, copyList(fac->fromAttrs));
                printFromAttrsContext(fac);
                appendStringInfo(from, ") F%u_%u", (*curFromItem)++, LIST_LENGTH(fac->fromAttrsList) - 1);
                //*fromAttrs = appendToTailOfList(*fromAttrs, attrNames);
                //appendStringInfo(from, ") F%u", (*curFromItem)++);
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
            attrNames = api->serializeQueryOperator(q, from, (QueryOperator *) getNthOfListP(q->parents,0), fac, api); //TODO ok to use first?
            //*fromAttrs = appendToTailOfList(*fromAttrs, attrNames);
            fac->fromAttrs = appendToTailOfList(fac->fromAttrs, attrNames);
            //fac->fromAttrsList = removeFromHead(fac->fromAttrsList);
            fac->fromAttrsList = appendToHeadOfList(fac->fromAttrsList, copyList(fac->fromAttrs));
            appendStringInfo(from, ") F%u_%u", (*curFromItem)++, LIST_LENGTH(fac->fromAttrsList) - 1);
        }
    }
}

/*
 * Translate a selection into a WHERE clause
 */
void
genSerializeWhere (SelectionOperator *q, StringInfo where, FromAttrsContext *fac, SerializeClausesAPI *api)
{
	HashMap *nestAttrMap = getNestAttrMap((QueryOperator *) q, fac, api);

	appendStringInfoString(where, "\nWHERE ");
	updateAttributeNames((Node *) q->cond, fac);
    appendStringInfoString(where, exprToSQL(q->cond, nestAttrMap, FALSE));
}

void
genSerializeLimitOperator (LimitOperator *q, StringInfo limit, SerializeClausesAPI *api)
{
	if (q->limitExpr != NULL)
	{
		appendStringInfoString(limit, "\nLIMIT ");
		appendStringInfo(limit, "%s", exprToSQL(q->limitExpr, NULL, FALSE));
	}
	if (q->offsetExpr != NULL)
	{
		appendStringInfoString(limit, "\nOFFSET ");
		appendStringInfo(limit, "%s", exprToSQL(q->offsetExpr, NULL, FALSE));
	}
}

void
genSerializePreparedStatement (QueryOperator *q, StringInfo prep, SerializeClausesAPI *api)
{
	if(HAS_STRING_PROP(q, PROP_PREPARED_QUERY_NAME))
	{
		char *name = GET_STRING_PROP_STRING_VAL(q, PROP_PREPARED_QUERY_NAME);
		appendStringInfo(prep, "PREPARE %s ", name);

		// explicit specificed data types
		if(HAS_STRING_PROP(q, PROP_PREPARED_QUERY_DTS))
		{
			List *dts = (List *) GET_STRING_PROP(q, PROP_PREPARED_QUERY_DTS);
			appendStringInfoString(prep, "(");
			FOREACH_INT(d,dts)
			{
				DataType dt = (DataType) d;
				appendStringInfoString(prep, backendDatatypeToSQL(dt));
				if(FOREACH_HAS_MORE(d))
				{
					appendStringInfoString(prep, ", ");
				}
			}
			appendStringInfoString(prep, ")");
		}
		appendStringInfo(prep, " AS ");
	}
}

void
genSerializeExecPreparedOperator (ExecPreparedOperator *q, StringInfo exec)
{
	appendStringInfo(exec, "EXEC %s", q->name);
	if(q->params)
	{
		appendStringInfoString(exec, "(");
		FOREACH(Constant,p,q->params)
		{
			appendStringInfo(exec, exprToSQL((Node *) p, NULL, FALSE));
		}
		appendStringInfoString(exec, ")");
	}
}

void
genSerializeOrderByOperator (OrderOperator *q, StringInfo order, FromAttrsContext *fac,
							 SerializeClausesAPI *api) //TODO check since copied from Oracle
{
	appendStringInfoString(order, "\nORDER BY ");
    //updateAttributeNames((Node *) q->orderExprs, (List *) fromAttrs);

    char *ordExpr = replaceSubstr(exprToSQL((Node *) q->orderExprs, NULL, FALSE),"(","");
    ordExpr = replaceSubstr(ordExpr,")","");
    ordExpr = replaceSubstr(ordExpr,"'","");
    appendStringInfoString(order, ordExpr);
}


boolean
updateAttributeNames(Node *node, FromAttrsContext *fac)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, AttributeReference))
    {
        AttributeReference *a = (AttributeReference *) node;
        DEBUG_LOG("a: %s",a->name);
        char *newName;
        List *outer = NIL;
        int fromItem = -1;
        int attrPos = 0;

//        // LOOP THROUGH fromItems (outer list)
//        FOREACH(List, attrs, fromAttrs)
//        {
//            attrPos += LIST_LENGTH(attrs);
//            fromItem++;
//            if (attrPos > a->attrPosition)
//            {
//                outer = attrs;
//                break;
//            }
//        }
        List *attrsList = NIL;
        if(a->outerLevelsUp >= 0)
        		attrsList = (List *) getNthOfListP(fac->fromAttrsList, a->outerLevelsUp);
        else
        		attrsList = (List *) getNthOfListP(fac->fromAttrsList, 0);

        FOREACH(List, attrs, attrsList)
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

        if(a->outerLevelsUp == -1)  //deal with nesting_eval_1 attribute which with outerLevelsUp = -1
         	a->name = CONCAT_STRINGS("F", gprom_itoa(fromItem), "_", gprom_itoa(LIST_LENGTH(fac->fromAttrsList)-1) , ".", newName);
        else
         	a->name = CONCAT_STRINGS("F", gprom_itoa(fromItem), "_", gprom_itoa(LIST_LENGTH(fac->fromAttrsList)-a->outerLevelsUp-1) , ".", newName);

//        attrPos = a->attrPosition - attrPos + LIST_LENGTH(outer);
//        if(LIST_LENGTH(outer) > attrPos)
//        {
//        		newName = getNthOfListP(outer, attrPos);
//        		a->name = CONCAT_STRINGS("F", gprom_itoa(fromItem), ".", newName);
//        }
    }

    return visit(node, updateAttributeNames, fac);
}

/*
 * Main entry point for serialization.
 */
List *
genSerializeQueryOperator (QueryOperator *q, StringInfo str, QueryOperator *parent, FromAttrsContext *fac, SerializeClausesAPI *api)
{
    // operator with multiple parents
	if (HAS_STRING_PROP(q, PROP_PREPARED_QUERY_NAME))
	{
	    api->serializePreparedStatment(q, str, api);
	}
	if (isA(q,ExecPreparedOperator))
	{
		api->serializeExecPreparedOperator((ExecPreparedOperator *) q, str);
		return NIL;
	}
    if (LIST_LENGTH(q->parents) > 1 || HAS_STRING_PROP(q,PROP_MATERIALIZE))
        return api->createTempView (q, str, parent, fac, api);
    else if (isA(q, SetOperator))
        return api->serializeSetOperator(q, str, fac, api);
    else
        return api->serializeQueryBlock(q, str, fac, api);
}

/*
 * Create a temporary view
 */
List *
genCreateTempView (QueryOperator *q, StringInfo str, QueryOperator *parent, FromAttrsContext *fac, SerializeClausesAPI *api)
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
        resultAttrs = api->serializeSetOperator(q, viewDef, fac, api);
    else
        resultAttrs = api->serializeQueryBlock(q, viewDef, fac, api);

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
exprToSQLWithNamingScheme (Node *expr, int rOffset,FromAttrsContext *fac)
{
    JoinAttrRenameState *state = NEW(JoinAttrRenameState);

    state->rightFromOffsets = rOffset;
    state->fac = fac;
    renameAttrsVisitor(expr, state);

    FREE(state);
    return exprToSQL(expr, NULL, FALSE);
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
            for(lc = getHeadOfList(state->fac->fromAttrs); fPos < rOffset; lc = lc->next, fPos++)
                ;
        else
            lc = getHeadOfList(state->fac->fromAttrs);

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

        a->name = createAttrName(name, fPos, state->fac);

        return TRUE;
    }

    return visit(node, renameAttrsVisitor, state);
}

static char *
createAttrName (char *name, int fItem, FromAttrsContext *fac)
{
   StringInfo str = makeStringInfo();
   char *result = NULL;

   appendStringInfo(str, "F%u_%u.%s", fItem, LIST_LENGTH(fac->fromAttrsList) - 1, name);
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

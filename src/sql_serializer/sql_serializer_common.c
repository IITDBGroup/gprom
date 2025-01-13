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
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"
#include "operator_optimizer/optimizer_prop_inference.h"
#include "provenance_rewriter/prov_utility.h"
#include <string.h>


#if HAVE_ORACLE_BACKEND
#include "ocilib.h"
#endif

#include "utility/string_utils.h"
#include "operator_optimizer/optimizer_prop_inference.h"
#include "configuration/option.h"

#include "sql_serializer/sql_serializer_common.h"
#include "sql_serializer/sql_serializer.h"


//#define TEMP_VIEW_NAME_PATTERN "_temp_view_%u"
#define TEMP_VIEW_NAME_PATTERN "temp_view_%u"


static boolean quoteAttributeNamesVisitQO (QueryOperator *op, void *context);
static boolean quoteAttributeNames (Node *node, void *context);
static char *createViewName (SerializeClausesAPI *api);
static boolean renameAttrsVisitor (Node *node, JoinAttrRenameState *state);
static char *createAttrName (char *name, int fItem, FromAttrsContext *fac);
static char *nestMapToString(HashMap *m);
static void setNestAttrMap(QueryOperator *op, FromAttrsContext *fac, SerializeClausesAPI *api, boolean where);
static boolean nestedSubqueryFirstUsedInProjection(QueryOperator *op, char *a, QueryBlockMatch *m);
static void findNestedSubqueryUsage(QueryOperator *op, char *a, boolean *inMatchSel, boolean *inNonMatchSel, QueryBlockMatch *m, boolean outOfFrom);
static void removeNestingAttributeFromOperators(QueryOperator *op, char *nestingAttr, SerializeClausesAPI *api);
static List *removeAllStringsFromList(List *l, HashMap *strs);
static boolean hasInlinedAttr(FromAttrsContext *fac);
static boolean updateAttributeNamesInternal(Node *node, FromAttrsContext *fac);
static boolean analyzeNestingVisitor(QueryOperator *q, void *api);
static void gatherCorrelatedAttrs(QueryOperator *q, List *nestScopes, SerializeClausesAPI *api);

/*
 * create API struct
 */

SerializeClausesAPI *
createAPIStub(void)
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
	api->getNestedSerLocations = genGetNestedSerializationLocations;
    api->createTempView = genCreateTempView;

    cleanAPIState(api);

    return api;
}

void
cleanAPIState(SerializeClausesAPI *api)
{
    api->tempViewMap = NEW_MAP(Constant, Node);
    api->nestAttrMap = NEW_MAP(Constant, Constant);
    api->inlinedNestResultAttrs = STRSET();
    api->nestingLeftSchemas = NEW_MAP(Constant,List);
    api->nestingCorrelatedReferences = NEW_MAP(Constant,List);
    api->viewCounter = 0;
}

char *
serLocationsToString(int serloc)
{
	StringInfo str = makeStringInfo();

	if(serloc & NEST_SER_SELECTION)
	{
		appendStringInfoString(str, "NEST_SER_SELECTION");
	}
	if(serloc & NEST_SER_FROM)
	{
		appendStringInfo(str, "%sNEST_SER_FROM", (STRINFO_LEN(str) == 0) ? "": ", ");
	}
	if(serloc & NEST_SER_SELECT)
	{
		appendStringInfo(str, "%sNEST_SER_SELECT", (STRINFO_LEN(str) == 0) ? "": ", ");
	}

	return str->data;
}

void
genQuoteAttributeNames(Node *q)
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
quoteAttributeNamesVisitQO(QueryOperator *op, void *context)
{
    return quoteAttributeNames((Node *) op, op);
}

static boolean
quoteAttributeNames(Node *node, void *context)
{
     if (node == NULL)
        return TRUE;

    // do not traverse into query operator nodes to avoid repeated traversal of paths in the graph
    if (node != context && IS_OP(node))
        return TRUE;

    // for nesting operators adjust result attribute names
    if(isA(node,NestingOperator))
    {
        NestingOperator *n = (NestingOperator *) node;
        List *attrNames = (List *) GET_STRING_PROP(n, PROP_NESTED_RESULT_ATTR);

        FOREACH(Constant,c,attrNames)
        {
            c->value = quoteIdentifier(STRING_VALUE(c));
        }
    }

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
initializeFromAttrsContext()
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
    result->api = fac->api; // this data structure is shared for the whole serialization process, do deep copy it

  	return result;
}

void
printFromAttrsContext(FromAttrsContext *fac)
{
    DEBUG_LOG_FAC("FAC\n\n%s", fac);
}

char *
fromAttrsContextToString(FromAttrsContext *fac)
{
    StringInfo str = makeStringInfo();

	appendStringInfo(str,"FromAttrsContext:\n");
	if(fac->fromAttrsList != NIL)
	{
		appendStringInfo(str,"Len: %d, FromAttrsContext->fromAttrsList: \n", LIST_LENGTH(fac->fromAttrsList));
		FOREACH(List, l1, fac->fromAttrsList)
		{
			appendStringInfoString(str, "[");
			FOREACH(List, l2, l1)
			{
				appendStringInfo(str, "(");
				FOREACH(char, c, l2)
				{
					appendStringInfo(str, " %s ", c);
				}
				appendStringInfo(str, ")%s", FOREACH_HAS_MORE(l2) ? ", " : "");
			}
			appendStringInfo(str, "]%s\n", FOREACH_HAS_MORE(l1) ? ", " : "");
		}
	}
	else
		appendStringInfo(str,"FromAttrsContext->fromAttrsList: NULL");

	if(fac->fromAttrs != NIL)
	{
		appendStringInfo(str,"\nLen: %d, FromAttrsContext->fromAttrs: ",LIST_LENGTH(fac->fromAttrs));
		FOREACH(List, l1, fac->fromAttrs)
		{
    	    appendStringInfo(str, "(");
			FOREACH(char, c, l1)
				appendStringInfo(str, " %s ", c);
			appendStringInfo(str, ")%s", FOREACH_HAS_MORE(l1) ? ", " : "");
		}
	}
	else
		appendStringInfo(str,"\nFromAttrsContext->fromAttrs: NULL");

	if(fac->api)
	{
        appendStringInfo(str, nestingOperatorBookkeepingToString(fac->api));
	}

    return str->data;
}

char *
nestingOperatorBookkeepingToString(SerializeClausesAPI *api)
{
    StringInfo str = makeStringInfo();

	if(api->nestAttrMap)
	{
	    appendStringInfo(str,
                         "\nAPI->NESTATTRMAP:\n%s\n\n",
                         nestMapToString(api->nestAttrMap));
	}
    if(api->inlinedNestResultAttrs)
    {
        appendStringInfo(str,
                         "\napi->inlinedNestResultAttrs:\n{");

        FOREACH_SET(char, s, api->inlinedNestResultAttrs)
        {
            appendStringInfo(str,
                             "%s%s",
                             s,
                             FOREACH_SET_HAS_MORE(s) ? ", ": "");
        }

        appendStringInfoString(str, "}\n");
    }
    if(api->nestingLeftSchemas)
    {
        appendStringInfo(str, "\napi->nestingLeftSchemas:\n");

        FOREACH_HASH_ENTRY(kv, api->nestingLeftSchemas)
        {
            appendStringInfo(str, "%s -> %s\n",
                             STRING_VALUE(kv->key),
                             stringListToString((List *) kv->value));
        }
    }
    if(api->nestingCorrelatedReferences && maxLevel >= LOG_DEBUG)//TODO hardcode here?
    {
        FOREACH_HASH_ENTRY(kv, api->nestingCorrelatedReferences)
        {
            appendStringInfo(str,
                             "\n\nCorrelations for <%s>:\n\n%s\n",
                             STRING_VALUE(kv->key),
                             beatify(nodeToString(kv->value)));
        }
    }

    return str->data;
}


static char *
nestMapToString(HashMap *m)
{
    StringInfo s = makeStringInfo();

    FOREACH_HASH_ENTRY(kv, m)
    {
        appendStringInfo(s, "%s -> %s\n\n", STRING_VALUE(kv->key), STRING_VALUE(kv->value));
    }

    return s->data;
}

HashMap *
getNestAttrMap(QueryOperator *op, FromAttrsContext *fac, SerializeClausesAPI *api, boolean where)
{
	/* HashMap *map = NEW_MAP(Constant, Constant); */
	FromAttrsContext *subqueryFac = copyFromAttrsContext(fac);

    setNestAttrMap(op, subqueryFac, api, where);
    DEBUG_LOG("inlined nested subqueries for %s:\n%s", (where ? "WHERE": "SELECT"), nestMapToString(api->nestAttrMap));

	return api->nestAttrMap;
}

static void
setNestAttrMap(QueryOperator *op, FromAttrsContext *fac, SerializeClausesAPI *api, boolean where)
{
	if(isA(op, NestingOperator))
	{
		NestingOperator *nest = (NestingOperator *) op;
		int nestingLocations = INT_VALUE(getStringProperty(op, PROP_NESTING_LOCATIONS));
        QueryOperator *lChild = OP_LCHILD(op); // cache left child before hand for traversal

        DEBUG_LOG("traverse nesting operator [%s] <%s>\n%s",
                  serLocationsToString(nestingLocations),
                  nestingOperatorGetNumResultAttrs(nest) == 1 ? getSingleNestingResultAttribute(nest) : "multiple",
                  singleOperatorToOverview(op));

		// only store nested subqueries that should be serialized into WHERE / HAVING
		if((where && (nestingLocations & NEST_SER_SELECTION))
		   || (!where && (nestingLocations & NEST_SER_SELECT)))
		{
			/* List *names = getAttrNames(op->schema); */
			char *nestName = (char *) getSingleNestingResultAttribute(nest);
			DEBUG_LOG("nesting attribute name %s for nested subquery in %s:\n%s", nestName, where ? "WHERE": "SELECT", singleOperatorToOverview(op));

			if(!(hasMapStringKey(api->nestAttrMap, nestName)))
 			{
				StringInfo s = makeStringInfo();
				if(nest->nestingType == NESTQ_EXISTS)
                {
					appendStringInfoString(s, "EXISTS ");
                }
				else if(nest->nestingType == NESTQ_ANY)
				{
					Operator *cond = (Operator *) nest->cond;
					Node *a = getHeadOfListP(cond->args);
					char *name = exprToSQL(a, api->nestAttrMap, FALSE);
					appendStringInfo(s, "%s %s ANY ", name, cond->name);
				}
				else if(nest->nestingType == NESTQ_ALL)
				{
					Operator *cond = (Operator *) nest->cond;
					Node *a = getHeadOfListP(cond->args);
					char *name = exprToSQL(a, api->nestAttrMap, FALSE);
					appendStringInfo(s, "%s %s ALL ", name, cond->name);
				}

                // subquery from clause starts empty but has current query's from clause as an "outer"
                FromAttrsContext *subqueryFac = copyFromAttrsContext(fac);
                subqueryFac->fromAttrs = NIL;
                subqueryFac->fromAttrsList = appendToHeadOfList(subqueryFac->fromAttrsList, copyList(fac->fromAttrs));
                //TODO this is where things break? fac = subqueryFac;

                DEBUG_LOG("going to serialize nested subquery for inlining <%s> for %s:\n\n%s",
                          strdup(nestName),
                          where ? "WHERE" : "SELECT",
                          operatorToOverviewString(op));

				appendStringInfoString(s, "(");
				api->serializeQueryOperator(OP_RCHILD(op), s, NULL, subqueryFac, api);
				appendStringInfoString(s, ")");
                if(maxLevel >= LOG_DEBUG)
                {
                    DEBUG_LOG("SERIALIZED NESTED SUBQUERY <%s> for %s:\n\n%s",
                              strdup(nestName),
                              where ? "WHERE" : "SELECT",
                              s->data);
                }
                else
                {
				    INFO_LOG("SERIALIZED NESTED SUBQUERY <%s> for %s: %s",
                             strdup(nestName),
                             where ? "WHERE" : "SELECT",
                             substr(s->data, 0, 40));
                }
				MAP_ADD_STRING_KEY(api->nestAttrMap, strdup(nestName),
								   createConstString(strdup(s->data)));

                // remove nesting operator
                switchSubtreeWithExisting(op, lChild);
                DEBUG_OP_LOG("query after nesting attr got removed", getFirstRoot(lChild));

                //TODO does not work as selection is now referring to non-existing nesting attr in its definition need to give check an options or
                //TODO too early have not adjusted everything yet
                /* if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING)) */
                /* { */
                /*     QueryOperator *root = getFirstRoot(lChild); */
                /*     //SET_BOOL_STRING_PROP(op, PROP_NO_MODEL_CHECKING_ROOT); */

                /*     ASSERT(checkModel(root)); */
                /* } */
			}
            else
            {
                DEBUG_NODE_BEATIFY_LOG("already in map", api->nestAttrMap);
            }
		}
        setNestAttrMap(lChild, fac, api, where);
        return;
	}

	FOREACH(QueryOperator, o, op->inputs)
    {
	    setNestAttrMap(o, fac, api, where);
    }
}

/*
 * Serialize a SQL query block (SELECT ... FROM ... WHERE ...)
 */
List *
genSerializeQueryBlock(QueryOperator *q, StringInfo str, FromAttrsContext *fac, SerializeClausesAPI *api)
{
    QueryBlockMatch *matchInfo = NEW(QueryBlockMatch);
    StringInfo fromString = makeStringInfo();
    StringInfo whereString = makeStringInfo();
    StringInfo selectString = makeStringInfo();
    StringInfo groupByString = makeStringInfo();
    StringInfo havingString = makeStringInfo();
	StringInfo orderString = makeStringInfo();
	StringInfo limitOffsetPrefixString = makeStringInfo();
	StringInfo limitOffsetSuffixString = makeStringInfo();
    MatchState state = MATCH_START;
    QueryOperator *cur = q;
    List *attrNames;
    boolean topMaterialize = HAS_STRING_PROP(cur,PROP_MATERIALIZE);

    // do the matching
    while(state != MATCH_NEXTBLOCK && cur != NULL)
    {
        DEBUG_LOG("STATE: %s", OUT_MATCH_STATE(state));
        DEBUG_LOG("Operator %s", singleOperatorToOverview((Node *) cur));
        // first check that cur does not have more than one parent
        if (!isRewriteOptionActivated(OPTION_ALWAYS_TREEIFY) && (HAS_STRING_PROP(cur,PROP_MATERIALIZE) || LIST_LENGTH(cur->parents) > 1))
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
                    {
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

	// determine for any nested subquery part of this block where it should be serialized to
	genMarkSubqueriesSerializationLocation(matchInfo, matchInfo->fromRoot, api);
	attrNames = getAttrNames(q->schema);

    // if from root is a nested subquery that got removed then change from root
    if (HAS_STRING_PROP(matchInfo->fromRoot, PROP_INLINED_NESTED_QUERY))
    {
        while(HAS_STRING_PROP(matchInfo->fromRoot, PROP_INLINED_NESTED_QUERY))
        {
            matchInfo->fromRoot = OP_LCHILD(matchInfo->fromRoot);
        }
        OUT_BLOCK_MATCH(DEBUG,matchInfo, "adjusted match after inlining nested subqueries into WHERE or SELECT");

        //FIXME fix from context and remove delete nesting result attributes
        removeInlinedNestingFromAttrsContext(fac);
    }

    // translate each clause
    DEBUG_LOG("serializeFrom");
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
		api->serializeLimitOperator(matchInfo->limitOffset, limitOffsetPrefixString, limitOffsetSuffixString, api);

    // put everything together
    DEBUG_LOG("mergePartsTogether");
    //TODO DISTINCT
	if(STRINGLEN(limitOffsetPrefixString) > 0)
	{
		appendStringInfoString(str, limitOffsetPrefixString->data);
	}

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

	if (STRINGLEN(limitOffsetSuffixString) > 0)
	{
		appendStringInfoString(str, limitOffsetSuffixString->data);
	}

    FREE(matchInfo);

    return attrNames;
}

/**
 * @brief Determines which clause nested subqueries should be serialized into.
 *
 * Given a query block match that assigns operators to SQL query block clauses, determines for each matched nesting operator what to do:
 *
 * 1) if the result attribute of the nesting operator is
 *      (i) if either of
            a) used only in the conditions of matched selection(s) (WHERE or HAVING)
            b) does not have correlations and is used in conditions non-matched selections
        AND
 *      (ii) the nesting operator can be serialized into a condition (e.g., LATERAL does not work)
 *	  then serialize the nested subquery into the WHERE or HAVING condition.
 * 2) if the nesting operator can be serialized into the FROM clause, then do that
 * 3) otherwise serialize into the SELECT (projection)
 *
 * for some systems 2) and 3) may require modifications to the nesting operator
 *
 * results are stored as a property PROP_NESTING_LOCATIONS of the operator
 *
 * @param qbMatch operators that will go into the current query block
 * @param op operator to check (skip if this is no nesting operator)
 */

void
genMarkSubqueriesSerializationLocation(QueryBlockMatch *qbMatch, QueryOperator *op, SerializeClausesAPI *api)
{
	//TODO use icols to determine whether the attribute is really needed and remove it otherwise
	if(isA(op,NestingOperator))
	{
		NestingOperator *n = (NestingOperator *) op;
		int serLocations = api->getNestedSerLocations(n, api);
        List *resultAttrs;

        // store result attributes for nesting operator
        resultAttrs = nestingOperatorGetResultAttributes(n);
        //resultAttrs = getNestingResultAttributeNames(n);

        DEBUG_LOG("Nesting operator locations <%s> result attrs: <%s>:\n\n%s",
                  serLocationsToString(serLocations),
                  stringListToString(resultAttrs),
                  singleOperatorToOverview(op));

		// check for case 1: operator can be serialized into WHERE or HAVING
		if(//TODO should allow serialization into ancestor block if possible? (qbMatch->where || qbMatch->having) // we have a condition clause to serialize into
           // &&
           (serLocations & NEST_SER_SELECTION)) // this type of subquery can be put into a condition
		{
			// determine whether the result attribute is only used in selections that are matched to the current query block
			boolean inMatchedSel = FALSE;
			boolean inNonSelOrNonMatched = FALSE;
			char *curTrackAtt = strdup(getSingleNestingResultAttribute(n));

			FOREACH(QueryOperator,p,op->parents)
			{
                //TODO check: nesting operator should be ok?
				findNestedSubqueryUsage(op,curTrackAtt, &inMatchedSel, &inNonSelOrNonMatched, qbMatch,
										!(isA(p,NestingOperator)
										  || isA(p,JoinOperator)));
			}
			// can serialize into WHERE, do that
			if (inMatchedSel && !inNonSelOrNonMatched) //TODO should we allow simultanous serialization to WHERE/HAVING and elsewhere?
			{
				serLocations = NEST_SER_SELECTION;
			}
			// serialization into WHERE is not possible
			else
			{
				serLocations &= ~NEST_SER_SELECTION; // cannot put into a selection
			}
		}
		// no WHERE or HAVING to serialize into remove selection as possible location
		else
		{
			serLocations &= ~NEST_SER_SELECTION;
		}

		// serialize into SELECT, remove nesting operator result attribute from intermediate operators
		if((serLocations & NEST_SER_SELECT)
		   && (qbMatch->firstProj || qbMatch->secondProj)
           && nestingOperatorGetNumResultAttrs((NestingOperator *) op) == 1)
		{
			char *curTrackAtt = strdup(getSingleNestingResultAttribute(n));

			// check that nested subquery result attribute is not used before projection
			if(nestedSubqueryFirstUsedInProjection(op, curTrackAtt, qbMatch))
			{
			    DEBUG_LOG("Serialize nested subquery into SELECT: <%s>", singleOperatorToOverview(op));
				serLocations = NEST_SER_SELECT;
			}
			else
			{
				serLocations &= ~NEST_SER_SELECT;
			}
		}
		else
		{
			serLocations &= ~NEST_SER_SELECT;
		}

		// serialize into WHERE, remove nesting operator result attribute from intermediate operators
		if(serLocations == NEST_SER_SELECTION)
		{
            char *nestResultAttr = getSingleNestingResultAttribute(n);

            DEBUG_LOG("Serialize nested subquery into WHERE as <%s>: <%s>",
                      nestResultAttr,
                      singleOperatorToOverview(op));
			// remove the attribute from other operators to avoid problems later
            SET_BOOL_STRING_PROP(op, PROP_INLINED_NESTED_QUERY);
            addToSet(api->inlinedNestResultAttrs, strdup(nestResultAttr));
			removeNestingAttributeFromOperators(op, nestResultAttr, api);

            DEBUG_OP_LOG("After removing nested subquery attribute from parent operators", getFirstRoot(op));
		}
		// serialize into SELECT, mark nesting operator to be inlined
		if(serLocations == NEST_SER_SELECT)
		{
			DEBUG_LOG("Serialize nested subquery into SELECT: <%s>", singleOperatorToOverview(op));
            SET_BOOL_STRING_PROP(op, PROP_INLINED_NESTED_QUERY);
			//TODO
			//ASSERT_WITH_MESSAGE(FALSE, "SERIALIZE INTO SELECT NOT SUPPORTED YET", "");
		}

		setStringProperty(op, PROP_NESTING_LOCATIONS, (Node *) createConstInt(serLocations));
		DEBUG_LOG("Subquery: %s\nshould be serialized into %s", singleOperatorToOverview(op), serLocationsToString(serLocations));
		//ASSERT(serLocations != NEST_SER_SELECT); //TODO implement serialization into SELECT
	}
	// from clause operators to traverse into
	if(isA(op,NestingOperator)
	   || isA(op,JoinOperator))
	{
		FOREACH(QueryOperator,c,op->inputs)
		{
			genMarkSubqueriesSerializationLocation(qbMatch, c, api);
		}
	}
}



/**
 * @brief Remove nested subquery result attribute from parents of a nesting operator.
 *
 * This is called when we want to serialize the nested subquery into WHERE or
 * HAVING, so the subquery result will not be available to other operator.
 *
 * @param op the operator whose parents we want to remove the attribute from
 * @param nestingAttr the attribute to be removed
 */

static void
removeNestingAttributeFromOperators(QueryOperator *op, char *nestingAttr, SerializeClausesAPI *api)
{
    Set *nestResultAttr = api->inlinedNestResultAttrs;
	AttributeDef *ad = getAttrDefByName(op, nestingAttr);
	int pos;
	char *newAttr = nestingAttr;

    // have nesting attribute, we still need to fix the operator itself as its child has changed
	if(ad == NULL)
    {
        FOREACH(QueryOperator,c,op->inputs)
        {
            // adjust attribute positions
            // if selection, then keep the nesting operator result attr in the
            // condition, but adjust other attribute reference to
	        if(isA(op,SelectionOperator))
	        {
		        resetPosOfAttrRefBaseOnBelowLayerSchema(op, c, nestResultAttr);
	        }
            else
            {
                resetPosOfAttrRefBaseOnBelowLayerSchema(op, c, NULL);
            }
        }
		return;
    }

	pos = getAttrPos(op, nestingAttr);

	if(isA(op,ProjectionOperator))
	{
	    ProjectionOperator *p = (ProjectionOperator *) op;

		p->projExprs = removeListElemAtPos(p->projExprs, pos);
	}

	// delete the attribute
	deleteAttrFromSchemaByName(op, nestingAttr, TRUE); //FIXME need to adjust also for nesting operators if there multiple ones !isA(op, NestingOperator));

    // record inlined attributes for selection operator
	if(isA(op,SelectionOperator))
	{
        SET_STRING_PROP(op, PROP_NESTED_INLINED_NESTED_QUERY, copyObject(nestResultAttr));
    }

    FOREACH(QueryOperator,c,op->inputs)
    {
        // adjust attribute positions
        // if selection, then keep the nesting operator result attr in the
        // condition, but adjust other attribute reference to
	    if(isA(op,SelectionOperator))
	    {
		    resetPosOfAttrRefBaseOnBelowLayerSchema(op, c, nestResultAttr);
	    }
        else
        {
            resetPosOfAttrRefBaseOnBelowLayerSchema(op, c, NULL);
        }
    }

    if(isA(op,NestingOperator))
    {
        List *newAttrNames = getQueryOperatorAttrNames(OP_LCHILD(op));
        char *id = getNestingOperatorId((NestingOperator *) op);
        List *correlations = (List *) ((MAP_HAS_STRING_KEY(api->nestingCorrelatedReferences,id)) ? (List *) MAP_GET_STRING(api->nestingCorrelatedReferences, id) : NIL);

        DEBUG_LOG("NestingOperator left input schema has changed, adjust correlated references:\n%s\n%s",
                  singleOperatorToOverview(op),
                  beatify(nodeToString(api->nestingCorrelatedReferences)));

        MAP_ADD_STRING_KEY(api->nestingLeftSchemas, id, newAttrNames);

        FOREACH(AttributeReference,a,correlations)
        {
            int newpos = listPosString(newAttrNames, a->name);
            DEBUG_LOG("a: %s: %u -> %u", a->name, a->attrPosition, newpos);
            a->attrPosition = newpos;
        }
    }

	// remove attribute from parents
	FOREACH(QueryOperator,p,op->parents)
	{
		removeNestingAttributeFromOperators(p, newAttr, api);
	}

    if(isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
    {
        ASSERT(checkSingleOperator(op));
    }
}

void
removeInlinedNestingFromAttrsContext(FromAttrsContext *fac)
{
    HashMap *inlinedA = fac->api->nestAttrMap;
    List *newFromAttrsList = NIL;

    fac->fromAttrs = removeAllStringsFromList(fac->fromAttrs, inlinedA);

    FOREACH(List,outer,fac->fromAttrsList)
    {
        List *newOuter = NIL;
        FOREACH(List, attrs, outer)
        {
            newOuter = appendToTailOfList(newOuter,
                                          removeAllStringsFromList(attrs, inlinedA));
        }
        newFromAttrsList = appendToTailOfList(newFromAttrsList, newOuter);
    }

    fac->fromAttrsList = newFromAttrsList;
}

static List *
removeAllStringsFromList(List *l, HashMap *strs)
{
    List *result = NIL;

    FOREACH(char,s,l)
    {
        if(!MAP_HAS_STRING_KEY(strs, s))
        {
            result = appendToTailOfList(result, s);
        }
        else
        {
            DEBUG_LOG("removed attribute from FromAttrsContext %s", s);
        }
    }

    return result;
}

static boolean
nestedSubqueryFirstUsedInProjection(QueryOperator *op, char *a, QueryBlockMatch *m)
{
	/* boolean secondProj = FALSE; */

	// no projections, can not serialize into SELECT
	if(!(m->firstProj || m->secondProj))
	{
		return FALSE;
	}

	// only one projection using the nested subquery or first of two projections using nested subquery
    if(!(m->firstProj && m->secondProj && opReferencesAttr((QueryOperator *) m->firstProj, a))
       && !(m->secondProj && opReferencesAttr((QueryOperator *) m->secondProj, a)))
    {
        return FALSE;
    }
    DEBUG_LOG("Used subquery in inner projection, may be able to serialize into SELECT.");

	// check that the nested subquery result attribute is not used in WHERE
	if(m->where)
	{
		if(opReferencesAttr((QueryOperator *) m->where, a))
		{
			return FALSE;
		}
	}
	if(m->having)
	{
		if(opReferencesAttr((QueryOperator *) m->having, a))
		{
			return FALSE;
		}
	}

	return TRUE;
}

/**
 * @brief Find operators which use the result attribute of NestingOperator.
 *
 * @param op check whether this operator uses the result attribute a of the nesting operator
 * @param a the result attribute of the nesting operator
 * @param inMatchSel record whether the nesting result is used in a selection of the query query block (m)
 * @param inNonMatchSel record whether the nesting result is used in a selection belonging to another query block
 * @param m the query block to which the nesting operator belongs to
 * @param outOfFrom is op inside the from clause of the query block m
 *
 */

static void
findNestedSubqueryUsage(QueryOperator *op, char *a, boolean *inMatchSel, boolean *inNonMatchSel, QueryBlockMatch *m, boolean outOfFrom)
{
	char *newA = a;
	boolean attrExistsInOutput = TRUE;

	switch (op->type)
	{
    	case T_NestingOperator:
    	{
    		// if nesting operator references attribute in condition then for now we
    		// do not consider serialization into WHERE/ HAVING
    		//TODO in some cases this would still be possible, e.g., WHERE (SELECT count(*) FROM R) = (SELECT count(*) FROM S)
    		if(nestingOpUsesAttrInCond((NestingOperator *) op, a))
    		{
    			*inNonMatchSel = TRUE;
    		}
    	}
    	break;
    	case T_ProjectionOperator:
    	{
    		ProjectionOperator *p = (ProjectionOperator *) op;
    		// for projection we need to check whether the attribute is only referenced or also used in an expression.
    		if(opReferencesAttr(op, a))
    		{
    			List *resultNames = NIL;
    			int pos = 0;

    			FOREACH(Node,e,p->projExprs)
    			{
    				if(isA(e,AttributeReference))
    				{
    					AttributeReference *ar = (AttributeReference *) e;
    					if(streq(ar->name, a))
    					{
                            DEBUG_LOG("Nested subquery inline, projection projects on attr <%s> in %s", a, beatify(nodeToString(ar)));
    						resultNames = appendToTailOfList(resultNames, getAttrNameByPos(op, pos));
    					}
    				}
    				// an expression referencing the attribute, we cannot serialize into WHERE / HAVING
    				else
    				{
    					if(doesExprReferenceAttribute(e, a))
    					{
                            DEBUG_LOG("Nested subquery inline, projection uses attr <%s> in EXPRESSION %s", a, beatify(nodeToString(e)));
    						*inNonMatchSel = TRUE;
    					}
    				}
    				pos++;
    			}
    			//TODO currently do not consider putting this into WHERE / HAVING if we are projecting the attribute out more than once
    			switch(LIST_LENGTH(resultNames))
    			{
    			    case 0://TODO should be dead code now, remove?
    			    {
                        FATAL_LOG("Should never end up here");
                        DEBUG_LOG("Nested subquery inlining, projection removes nesting result attr <%s>", a);
    				    attrExistsInOutput = FALSE;
    			    }
    			    break;
    			    case 1:
    			    {
    				    // we postpone making a decision
                        DEBUG_LOG("Nested subquery inlining, projection projects ones on nesting result attr <%s>", a);
    				    newA = getNthOfListP(resultNames, 0);
    			    }
                    break;
    			    // more than one reference
    			    default:
    			    {
                        DEBUG_LOG("Nested subquery inlining, projection removes nesting result attr <%s>", a);
    				    *inNonMatchSel = TRUE;
    			    }
                    break;
    			}
    			outOfFrom = TRUE;
    		}
    		// no references, attribute no longer there
    		else
    		{
    			attrExistsInOutput = FALSE;
    		}
    	}
    	break;
    	// if selection operator
    	case T_SelectionOperator:
    	{
    		outOfFrom = TRUE;
    		boolean parentNeedsNestingAttr = FALSE;

    		FOREACH(QueryOperator,p,op->parents)
    		{
    			parentNeedsNestingAttr |= isAttrRequired(p,a);
    		}

            DEBUG_LOG("nesting operator inlining: in selection and parents need the nesting attribute? %s\n\n%s",
                      parentNeedsNestingAttr ? "YES": "NO",
                      singleOperatorToOverview(op));

    		// does selection operator reference subquery
    		if(opReferencesAttr(op, a))
    		{
    			// is this a matched operator or from a different query block
    			if(!parentNeedsNestingAttr)
                    //TODO we should not limit serialization to the same query block as long as there are no uses between the nesting operator and the selection that cannot be removed
                    //&& (((QueryOperator *) m->where) == op || ((QueryOperator *) m->having) == op))
    			{
                    DEBUG_LOG("can materialize into WHERE");

    				*inMatchSel = TRUE;
    			}
    			// not a matched operator
    			else
    			{
    				*inNonMatchSel = TRUE;
    			}
    		}
    	}
    	break;
    	case T_JoinOperator:
    	{
    		QueryOperator *l = OP_LCHILD(op);
    		QueryOperator *r = OP_RCHILD(op);
    		int pos;

    		//TODO technically we could serialize into joins, but let's not do that
    		if(opReferencesAttr(op, a))
    		{
    			*inNonMatchSel = TRUE;
    		}

    		pos = getAttrPos(l, a);
    		if(pos == -1)
    		{
    			pos = getAttrPos(r, a) + LIST_LENGTH(l->schema->attrDefs);
    		}

    		newA = getAttrNameByPos(op, pos);
    	}
    	break;
    	default: //TODO there may be some other cases where the nesting operator
    			//result attribute is in the result schema but could be ommitted
    	{
            DEBUG_LOG("Nested subquery inling, other operator %s", singleOperatorToOverview(op));
    		outOfFrom = TRUE;
    		// does  operator reference subquery, for now only
    		if(opReferencesAttr(op, a))
    		{
                DEBUG_LOG("Other operator references nesting result attribute <%s>:\n%s\n\nCANNOT SERAILIZE INTO WHERE",
                          a,
                          singleOperatorToOverview(op));
    			*inNonMatchSel = TRUE;
    		}
    	}
    	break;
	}

	// check for usage in parents if the attribute still exists in the result of the operator
	if(attrExistsInOutput)
	{
		FOREACH(QueryOperator,p,op->parents)
		{
			findNestedSubqueryUsage(p, newA, inMatchSel, inNonMatchSel, m, outOfFrom);
		}
	}
}


int
genGetNestedSerializationLocations(NestingOperator *n, SerializeClausesAPI *api)
{
	if(n->nestingType == NESTQ_LATERAL)
	{
		return NEST_SER_FROM;
	}
	return NEST_SER_SELECTION | NEST_SER_SELECT | NEST_SER_FROM;
}

void
genSerializeFrom(QueryOperator *q, StringInfo from, FromAttrsContext *fac, SerializeClausesAPI *api)
{
    int curFromItem = 0, attrOffset = 0;

    appendStringInfoString(from, "\nFROM ");
    api->serializeFromItem (q, q, from, &curFromItem, &attrOffset, fac, api);
}

#define CLOSE_FROM_ITEM() appendStringInfo(from, ") F%u_%u", (*curFromItem)++, LIST_LENGTH(fac->fromAttrsList))

void
genSerializeFromItem(QueryOperator *fromRoot, QueryOperator *q, StringInfo from, int *curFromItem,
					  int *attrOffset, FromAttrsContext *fac, SerializeClausesAPI *api)
{
    // if operator has more than one parent then it will be represented as a CTE
    // however, when create the code for a CTE (q==fromRoot) then we should create SQL for this op)
	// also do not materialized if the user forced a tree structions
    if (!(LIST_LENGTH(q->parents) > 1 || HAS_STRING_PROP(q, PROP_MATERIALIZE))
        || q == fromRoot
        || isRewriteOptionActivated(OPTION_ALWAYS_TREEIFY)
        || !HAS_STRING_PROP(q, PROP_DO_NOT_MATERIALIZE))
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
		case T_NestingOperator: //FIXME fix serialization
		{
			NestingOperator *nest = (NestingOperator *) q;
			int location = INT_VALUE(getStringProperty(q, PROP_NESTING_LOCATIONS));

			DEBUG_LOG("subquery %s in from, will be serialized into %s",
					  singleOperatorToOverview(q),
					  serLocationsToString(location));

			// only serialize to FROM clause if we do not serialize elsewhere
			if((location & NEST_SER_FROM) && !(location & NEST_SER_SELECTION) && !(location & NEST_SER_SELECT))
			{
				QueryOperator *outer = OP_LCHILD(nest);
				QueryOperator *subquery = OP_RCHILD(nest);
				FromAttrsContext *subqueryFac;
				List *subqueryNames;

				//TODO translate into lateral when necessary (currently only scalar which does not require any changes)
				ASSERT(nest->nestingType == NESTQ_LATERAL || nest->nestingType == NESTQ_SCALAR);

				// serialize left input (the outer query)
				api->serializeFromItem(fromRoot, outer, from, curFromItem, attrOffset, fac, api);

				// create fac with previous from clause items as context for correlations in LATERAL
				subqueryFac = copyFromAttrsContext(fac);
				subqueryFac->fromAttrsList = appendToHeadOfList(subqueryFac->fromAttrsList, copyList(fac->fromAttrs));
				subqueryFac->fromAttrs = NIL;

				// serialize the nested subquery
				appendStringInfoString(from, ", LATERAL (");
				subqueryNames = api->serializeQueryOperator(subquery, from, q, subqueryFac, api);
				CLOSE_FROM_ITEM();

				// add result attributes to current from clasue
				fac->fromAttrs = appendToTailOfList(fac->fromAttrs, subqueryNames);
				printFromAttrsContext(fac);

			}
			else // otherwise attributes from the left input still have to be added to from clause
			{
				QueryOperator *input = OP_LCHILD(nest);
				api->serializeFromItem(fromRoot, input, from, curFromItem, attrOffset, fac, api);
			}
		}
		break;
		default://CHECK not sure that the fromAttrsList handling here is correct
		{
			List *attrNames;

			appendStringInfoString(from, "(");
			attrNames = api->serializeQueryOperator(q, from, (QueryOperator *) getNthOfListP(q->parents,0), fac, api); //TODO ok to use first?
			fac->fromAttrs = appendToTailOfList(fac->fromAttrs, attrNames);
			printFromAttrsContext(fac);
			CLOSE_FROM_ITEM();
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
            fac->fromAttrs = appendToTailOfList(fac->fromAttrs, attrNames);
            CLOSE_FROM_ITEM();
        }
    }
}

/*
 * Translate a selection into a WHERE clause
 */
void
genSerializeWhere(SelectionOperator *q, StringInfo where, FromAttrsContext *fac, SerializeClausesAPI *api)
{
	HashMap *nestAttrMap = getNestAttrMap((QueryOperator *) q, fac, api, TRUE);
    char *condStr;

	appendStringInfoString(where, "\nWHERE ");
    removeInlinedNestingFromAttrsContext(fac);
	updateAttributeNames((Node *) q->cond, fac);
    condStr = exprToSQL(q->cond, nestAttrMap, FALSE);
    DEBUG_LOG("translated where clause condition: <%s>", condStr);
    appendStringInfoString(where, condStr);
}

void
genSerializeLimitOperator(LimitOperator *q, StringInfo limitPrefix, StringInfo limitSuffix, SerializeClausesAPI *api)
{
	if (q->limitExpr != NULL)
	{
		appendStringInfoString(limitSuffix, "\nLIMIT ");
		appendStringInfo(limitSuffix, "%s", exprToSQL(q->limitExpr, NULL, FALSE));
	}
	if (q->offsetExpr != NULL)
	{
		appendStringInfoString(limitSuffix, "\nOFFSET ");
		appendStringInfo(limitSuffix, "%s", exprToSQL(q->offsetExpr, NULL, FALSE));
	}
}

void
genSerializePreparedStatement(QueryOperator *q, StringInfo prep, SerializeClausesAPI *api)
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
    removeInlinedNestingFromAttrsContext(fac);
    if(hasInlinedAttr(fac))
    {
        printFromAttrsContext(fac);
        FATAL_LOG("Should not have attribute for inlined nested subquery <%s> in FromAttrsContext", beatify(nodeToString(fac->api->nestAttrMap)));
    }
    return updateAttributeNamesInternal(node, fac);
}

static boolean
updateAttributeNamesInternal(Node *node, FromAttrsContext *fac)
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

		if(!fac->api || !fac->api->nestAttrMap || !MAP_HAS_STRING_KEY(fac->api->nestAttrMap, a->name))
		{
			List *attrsList = NIL;
			if(a->outerLevelsUp > 0) // outer query correlated attributes
            { //TODO double check
                int pos = a->outerLevelsUp - 1;
                ASSERT(pos >=0 && pos < LIST_LENGTH(fac->fromAttrsList));
				attrsList = (List *) getNthOfListP(fac->fromAttrsList, pos);
            }
			else // attribute from current query block
            {
				attrsList = (List *) fac->fromAttrs;
            }

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

            // if we have not found outer the there is something wrong
            if(outer == NIL)
            {
                printFromAttrsContext(fac);
                FATAL_LOG("Cannot update attribute name, reference to non-existing attribute %s", beatify(nodeToString(node)));
            }

            // we find inlined attributes in schema: fix that
            if(hasInlinedAttr(fac))
            {
                FATAL_LOG("found inlined subquery attribute in FromAttrsContext");
                printFromAttrsContext(fac);
                removeInlinedNestingFromAttrsContext(fac);
            }

			attrPos = a->attrPosition - attrPos + LIST_LENGTH(outer);
			newName = getNthOfListP(outer, attrPos);

            //FIXME this does no longer happen?
			if(a->outerLevelsUp == -1)  //deal with nesting_eval_1 attribute which with outerLevelsUp = -1
            {
                DEBUG_LOG("update attribute names for nested orig name <%s> to <%s>", a->name, newName);
				a->name = CONCAT_STRINGS("F", gprom_itoa(fromItem), "_",
                                         gprom_itoa(LIST_LENGTH(fac->fromAttrsList)),
                                         ".",
                                         newName);
            }
			else
            {
                char *updatedName = CONCAT_STRINGS("F", gprom_itoa(fromItem), "_",
                                                   gprom_itoa(LIST_LENGTH(fac->fromAttrsList)-(a->outerLevelsUp)),
                                                   ".",
                                                   newName);

                if(!streq(a->name,updatedName) && !streq(a->name, newName))
                {
                    ERROR_LOG_FAC("Messed up attributes:\n\n%s", fac);
                    ERROR_LOG("WARNING: different attribute name used <%s> new name <%s> <%s>",
                              a->name,
                              newName,
                              updatedName
                              );
                }

                a->name = updatedName;
            }
		}
    }

    return visit(node, updateAttributeNames, fac);
}

static boolean
hasInlinedAttr(FromAttrsContext *fac)
{
    HashMap *inlinedA = fac->api->nestAttrMap;

    FOREACH(char,a,fac->fromAttrs)
    {
        if(MAP_HAS_STRING_KEY(inlinedA, a))
        {
            return TRUE;
        }
    }

    FOREACH(List,fromItem,fac->fromAttrsList)
    {
        FOREACH(List,attrs, fromItem)
        {
            FOREACH(char,a,attrs)
            {
                if(MAP_HAS_STRING_KEY(inlinedA, a))
                {
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
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
    if (!isRewriteOptionActivated(OPTION_ALWAYS_TREEIFY) &&
		(LIST_LENGTH(q->parents) > 1 || HAS_STRING_PROP(q,PROP_MATERIALIZE)) &&
        !HAS_STRING_PROP(q, PROP_DO_NOT_MATERIALIZE))
    {
        return api->createTempView(q, str, parent, fac, api);
    }
    else if (isA(q, SetOperator))
    {
        return api->serializeSetOperator(q, str, fac, api);
    }
    else
    {
        return api->serializeQueryBlock(q, str, fac, api);
    }
}

/*
 * Create a temporary view
 */
List *
genCreateTempView(QueryOperator *q, StringInfo str, QueryOperator *parent, FromAttrsContext *fac, SerializeClausesAPI *api)
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

    DEBUG_LOG("created view definition:\n%s:\n\n %s", singleOperatorToOverview(q), viewDef->data);

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

   appendStringInfo(str, "F%u_%u.%s", fItem, LIST_LENGTH(fac->fromAttrsList), name);
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

void
analyzeNesting(QueryOperator *q, SerializeClausesAPI *api)
{
    // gather result attributes to
    visitQOGraph(q, TRAVERSAL_PRE, analyzeNestingVisitor, api);
    // store correlated attribute references
    gatherCorrelatedAttrs(q, NIL, api);
}

static boolean
analyzeNestingVisitor(QueryOperator *q, void *context)
{
    SerializeClausesAPI *api = (SerializeClausesAPI *) context;

    if(isA(q,NestingOperator))
    {
        NestingOperator *n = (NestingOperator *) q;

        List *leftResultAttrs = getQueryOperatorAttrNames(q);
        char *resultAttr = getSingleNestingResultAttribute(n);
        MAP_ADD_STRING_KEY(api->nestingLeftSchemas, resultAttr, leftResultAttrs);
    }

    return TRUE;
}

#define LATERAL_DUMMY_ID "LATERAL_DUMMY"

static void
gatherCorrelatedAttrs(QueryOperator *q, List *nestScopes, SerializeClausesAPI *api)
{
    List *corrAttrs = getCorrelatedAttrRefsInOperator(q);
    List *newScope = nestScopes;

    // if this is a nesting operator, add it to the scope
    if(isA(q,NestingOperator))
    {
        NestingOperator *n = (NestingOperator *) q;
        char *id = getNestingOperatorId(n);

        gatherCorrelatedAttrs(OP_LCHILD(q), newScope, api);

        newScope = appendToTailOfList(newScope, strdup(id));

        gatherCorrelatedAttrs(OP_RCHILD(q), newScope, api);
    }
    else
    {
        FOREACH(QueryOperator,c,q->inputs)
        {
            gatherCorrelatedAttrs(c, newScope, api);
        }
    }

    FOREACH(AttributeReference,a,corrAttrs)
    {
        int scopepos = a->outerLevelsUp - 1;
        ASSERT(LIST_LENGTH(nestScopes) >= a->outerLevelsUp);
        char *scope = getNthOfListP(nestScopes, scopepos);

        MAP_ADD_STRING_KEY_TO_VALUE_LIST(api->nestingCorrelatedReferences, scope, a, FALSE);
    }
}

#include "common.h"
#include "configuration/option.h"
#include "log/logger.h"

#include "metadata_lookup/metadata_lookup_postgres.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"
#include "model/expression/expression.h"
#include "metadata_lookup/metadata_lookup.h"

#include "provenance_rewriter/prov_utility.h"
#include "utility/enum_magic.h"
#include "utility/string_utils.h"

NEW_ENUM_WITH_ONLY_TO_STRING(Semiring,
                             SEMIRING_CUSTOM,
                             SEMIRING_N,
                             SEMIRING_TROPICAL,
                             SEMIRING_VITERBI,
                             SEMIRING_FUZZY,
                             SEMIRING_WHICH,
                             SEMIRING_NX);

#define DUMMY_LEFT_ATTR backendifyIdentifier("__prov_left_input")
#define DUMMY_RIGHT_ATTR backendifyIdentifier("__prov_right_input")

/* function declarations */
//static Node *deepReplaceAttrRef(Node * expr, Node *af);//replace all attributeReferences to af
//QueryOperator * addSemiringCombiner(QueryOperator * result);
static void getAttributeReferencesForSC(Node *expr, AttributeReference **leftName,
        AttributeReference **rightName, boolean twoInputs);
static Node *deepReplaceAttrRefMutator(Node *node,  void *state);
static boolean addCombinerExprIsOK(Node *node, void *state);
static boolean onlySemiringSupportedOpsInternal(QueryOperator *op);


static Node *
deepReplaceAttrRefMutator(Node *node, void *state)
{
    HashMap *context = (HashMap *) state;

    if (node == NULL)
        return NULL;

    if (isA(node, AttributeReference))
    {
        Node *repl;
        repl = getMap(context, node);
        return copyObject(repl);
    }

    return mutate(node, deepReplaceAttrRefMutator, context);
}

static Semiring
userStringToSemiring(char *str)
{
    if(streq(str,"N"))
    {
        return SEMIRING_N;
    }
    if(streq(str,"NX"))
    {
        return SEMIRING_NX;
    }
    if(streq(str,"TROPICAL"))
    {
        return SEMIRING_TROPICAL;
    }
    if(streq(str,"VITERBI"))
    {
        return SEMIRING_VITERBI;
    }
    if(streq(str, "FUZZY"))
    {
        return SEMIRING_FUZZY;
    }
    if(streq(str, "WHICH"))
    {
        return SEMIRING_WHICH;
    }

    return SEMIRING_CUSTOM;
}

//
//static Node *
//deepReplaceAttrRef(Node * expr, Node *af)
//{
//	switch(expr->type){
//		case T_AttributeReference: {
//			return (Node *)copyObject(af);
//		}
//		case T_Operator: {
//			FOREACH_LC(lc,((Operator *)expr)->args){
//				lc->data.ptr_value = (Node *)deepReplaceAttrRef((Node *)(lc->data.ptr_value), af);
//			}
//			break;
//		}
//		default: {
//			return expr;
//		}
//	}
//	return expr;
//}

boolean
isSemiringCombinerActivatedOp(QueryOperator * op)
{
    return HAS_STRING_PROP((QueryOperator *)op,PROP_PC_SEMIRING_COMBINER);
}

boolean
isSemiringCombinerActivatedPs(ProvenanceStmt *stmt)
{
	FOREACH(KeyValue,kv,stmt->options){
		//INFO_LOG(STRING_VALUE(kv->key));
		if(strcmp(STRING_VALUE(kv->key),PROP_PC_SEMIRING_COMBINER)==0)
			return TRUE;
	}
	return FALSE;
}



/**
 * @brief Given a parsed semiring specification, return a scalar expression for multiplication in the semiring.
 *
 * @param p the parsed semiring specification, either a string constants (the semiring name) or an expression provided by the user.
 * @return an expression over two dummy attribute references implementing the multiplication in the semiring.
 */

static Node *
semiringMultExpr(Node *p)
{
	switch(p->type){
		case T_Constant:
		{
            Semiring K = userStringToSemiring(STRING_VALUE(p));
            Node *l, *r;
            DEBUG_LOG("Mult expression for semiring %s", SemiringToString(K));

            switch(K)
            {
                case SEMIRING_N:
                {
                    // l * r
                    l = (Node *) createFullAttrReference(strdup(DUMMY_LEFT_ATTR),0,0,0,DT_LONG);
                    r = (Node *) createFullAttrReference(strdup(DUMMY_RIGHT_ATTR),0,0,0,DT_LONG);
			        return (Node *) createOpExpr(OPNAME_MULT,
                                                 LIST_MAKE(l,r));
                }
                case SEMIRING_NX:
                {
                    // "(" || l || " * " || r || ")"
                    List *operands;
                    l = (Node *) createFullAttrReference(strdup(DUMMY_LEFT_ATTR),0,0,0,DT_STRING);
                    r = (Node *) createFullAttrReference(strdup(DUMMY_RIGHT_ATTR),0,0,0,DT_STRING);
                    operands = LIST_MAKE(createConstString("("),
                                         l,
                                         createConstString(" * "),
                                         r,
                                         createConstString(")"));
			        return (Node *) concatExprList(operands);
                }
                case SEMIRING_TROPICAL:
                {
                    // l + r
                    l = (Node *) createFullAttrReference(strdup(DUMMY_LEFT_ATTR),0,0,0,DT_FLOAT);
                    r = (Node *) createFullAttrReference(strdup(DUMMY_RIGHT_ATTR),0,0,0,DT_FLOAT);
			        return (Node *) createOpExpr(OPNAME_ADD,
                                                 LIST_MAKE(l,r));
                }
                case SEMIRING_VITERBI:
                {
                    // l * r
                    l = (Node *) createFullAttrReference(strdup(DUMMY_LEFT_ATTR),0,0,0,DT_FLOAT);
                    r = (Node *) createFullAttrReference(strdup(DUMMY_RIGHT_ATTR),0,0,0,DT_FLOAT);
			        return (Node *) createOpExpr(OPNAME_MULT,
                                                 LIST_MAKE(l,r));
                }
                case SEMIRING_FUZZY:
                {
                    // min(l,r)
                    l = (Node *) createFullAttrReference(strdup(DUMMY_LEFT_ATTR),0,0,0,DT_FLOAT);
                    r = (Node *) createFullAttrReference(strdup(DUMMY_RIGHT_ATTR),0,0,0,DT_FLOAT);
			        return (Node *) createFunctionCall(LEAST_FUNC_NAME,
                                                       LIST_MAKE(l,r));
                }
                case SEMIRING_WHICH:
                {
                    BackendType backend = getBackend();

                    // l || r: only for backends that support arrays and concatenation
                    if(backend == BACKEND_POSTGRES || backend == BACKEND_DUCKDB)
                    {
                        l = (Node *) createFullAttrReference(strdup(DUMMY_LEFT_ATTR),0,0,0,DT_STRING);
                        r = (Node *) createFullAttrReference(strdup(DUMMY_RIGHT_ATTR),0,0,0,DT_STRING);

                        if(backend == BACKEND_POSTGRES)
                        {
                            char *dt = postgresExtensionInstalled(PGEXT_INTARRAY) ? "int4[]": "int8[]";
                            Node *arraycat;

                            l = (Node *) createCastExprOtherDT((Node *) l, dt, -1, DT_STRING);
                            r = (Node *) createCastExprOtherDT((Node *) r, dt, -1, DT_STRING);
                            arraycat = (Node *) createCastExprOtherDT((Node *) createOpExpr(OPNAME_ARRAY_CONCAT,
                                                                                            LIST_MAKE(l,r)),
                                                                      strdup(dt),
                                                                      0,
                                                                      DT_STRING);

                            return arraycat;
                        }
                        else
                        {
                            l = (Node *) createCastExprOtherDT((Node *) l, strdup("int32[]"), -1, DT_STRING);
                            r = (Node *) createCastExprOtherDT((Node *) r, strdup("int32[]"), -1, DT_STRING);
                            return (Node *) createOpExpr(OPNAME_ARRAY_CONCAT,
                                                         LIST_MAKE(l,r));
                        }
                    }
                    else
                    {
                        THROW(SEVERITY_RECOVERABLE,
                              "semiring WHICH not supported on backend %s",
                              BackendTypeToString(getBackend()));
                    }
                }
                default:
                    THROW(SEVERITY_RECOVERABLE,"unknown semiring %s specified", STRING_VALUE(p));
                break;
            }
            return NULL;
		}
		case T_List:
		{
			return (Node *) getNthOfListP((List *) p,1);
		}
		default:
		{
			FATAL_LOG("unknown expression type for SC option: %s", nodeToString(p));
		}
	}
	return NULL;
}

/**
 * @brief Given semiring specification provided by the user, return an aggregation function implementing addition.
 *
 * @param p the parsed semiring specification, either a String constant (semiring name) or an expression for custom semrirings.
 * @return an expression with the aggregation implementing semiring addition over a dummy attribute reference.
 */

static Node *
semiringAddExpr(Node *p)
{
    BackendType b = getBackend();

	switch(p->type)
	{
	    // user has specified a semiring name (Should be one of Semiring enum)
		case T_Constant:
		{
            Semiring K = userStringToSemiring(STRING_VALUE(p));
            AttributeReference *a;
            DEBUG_LOG("Add expression for semiring %s", SemiringToString(K));

            switch(K)
            {
                case SEMIRING_N:
                {
                    // l + r => sum(a)
                    a = createFullAttrReference(strdup(DUMMY_LEFT_ATTR),0,0,0,DT_FLOAT);
			        return (Node *) createFunctionCall(SUM_FUNC_NAME,
                                                       singleton(a));
                }
                case SEMIRING_TROPICAL:
                {
                    // min(l,r) => min(a)
                    a = createFullAttrReference(strdup(DUMMY_LEFT_ATTR),0,0,0,DT_FLOAT);
                    return (Node *) createFunctionCall(MIN_FUNC_NAME,
                                                       singleton(a));
                }
                case SEMIRING_VITERBI:
                {
                    // max(l,r) => max(a)
                    a = createFullAttrReference(strdup(DUMMY_LEFT_ATTR),0,0,0,DT_FLOAT);
                    return (Node *) createFunctionCall(MAX_FUNC_NAME,
                                                       singleton(a));
                }
                case SEMIRING_FUZZY:
                {
                    // max(l,r) => max(a)
                    a = createFullAttrReference(strdup(DUMMY_LEFT_ATTR),0,0,0,DT_FLOAT);
                    return (Node *) createFunctionCall(MAX_FUNC_NAME,
                                                       singleton(a));
                }
                case SEMIRING_NX:
                {
                    // 'l + r' => string_agg(a, ' + ')
                    // TODO check who supports it
                    a = createFullAttrReference(strdup(DUMMY_LEFT_ATTR),0,0,0,DT_STRING);
			        return (Node *) createFunctionCall(STRINGAGG_FUNC_NAME,
                                                       LIST_MAKE(a,createConstString(" + ")));
                }
                case SEMIRING_WHICH:
                {
                    // l={a,...} u r={b,...} => array_agg(a)
                    if(b == BACKEND_POSTGRES)
                    {
                        a = createFullAttrReference(strdup(DUMMY_LEFT_ATTR),0,0,0,DT_STRING);

                        if(postgresExtensionInstalled(PGEXT_INTARRAY))
                        {
                            Node *arrayagg, *sort, *unique;

                            arrayagg = (Node *) createFunctionCall(POSTGRES_ARRAY_CONCAT_AGG_FUNC,
                                                          singleton(a));
                            sort = (Node *) createFunctionCall(POSTGRES_INTARRAY_SORT_FUNC,
                                                               singleton(arrayagg));
                            unique = (Node *) createFunctionCall(POSTGRES_INTARRAY_UNIQ_FUNC,
                                                                 singleton(sort));
                            return unique;
                        }
                        else
                        {
                            return (Node *) createFunctionCall(POSTGRES_ARRAY_CONCAT_AGG_FUNC,
                                                               singleton(a));
                        }
                    }
                    // => list_flatten(list(a))
                    else if(b == BACKEND_DUCKDB)
                    {
                        a = createFullAttrReference(strdup(DUMMY_LEFT_ATTR),0,0,0,DT_STRING);
                        return (Node *) createFunctionCall(DUCKDB_LIST_FLATTEN_FUNC,
                                                           singleton(createFunctionCall(DUCKDB_LIST_AGG_FUNC,
                                                                                        singleton(a))));
                    }
                    else
                    {
                        THROW(SEVERITY_RECOVERABLE,
                              "semiring WHICH not supported for Backend %s yet",
                              BackendTypeToString(b));
                    }
                }
                default:
                    THROW(SEVERITY_RECOVERABLE,"unknown semiring %s specified", STRING_VALUE(p));
                break;
            }
            return NULL;
		}
		case T_List:
		{
			return getNthOfListP((List *) p,0);
		}
		default:
		{
			FATAL_LOG("unknown expression type for SC option: %s", nodeToString(p));
		}
	}
	return NULL;
}


Node *
getSemiringCombinerMultExpr(QueryOperator *op)
{
	Node *p = getStringProperty(op, PROP_PC_SEMIRING_COMBINER);
    return semiringMultExpr(p);
}

Node *
getSemiringCombinerAddExpr(QueryOperator *op){
	Node *p = getStringProperty(op, PROP_PC_SEMIRING_COMBINER);
    return semiringAddExpr(p);
}


/**
 * @brief return datatype for a particular semiring
 *
 *
 * @param stmt provenance statement that uses semiring combiner
 * @param dts the input data types to the expression
 * @return result datatype for semiring expressions
 */

DataType
getSemiringCombinerDatatype(ProvenanceStmt *stmt, List *dts)
{
    DataType multType = DT_INT;
    DataType addType = DT_INT;

	FOREACH(KeyValue,kv,stmt->options)
    {
		if(streq(STRING_VALUE(kv->key),PROP_PC_SEMIRING_COMBINER))
		{
			INFO_NODE_BEATIFY_LOG("get datatypes for semiring combiner expressions:",kv->value);
			Node *addExpr = semiringAddExpr(kv->value);
			Node *multExpr = semiringMultExpr(kv->value);

            //			boolean exists = FALSE;
			if (!addCombinerExprIsOK(addExpr, NEW(boolean)))
			    FATAL_NODE_BEATIFY_LOG("expression for addition semiring combiner can only use attribute references within aggregation function calls:\n\n", addExpr);


			// replace user provided attribute references with actual ones
			AttributeReference *multLeftAttr = NULL;
			AttributeReference *multRightAttr = NULL;
			AttributeReference *multActualLeft = NULL;
			AttributeReference *multActualRight = NULL;
			AttributeReference *addInputAttr = NULL;
			AttributeReference *multResultAttr = NULL;

			getAttributeReferencesForSC(multExpr, &multLeftAttr, &multRightAttr, TRUE);
			getAttributeReferencesForSC(addExpr, &addInputAttr, &addInputAttr, FALSE);
			multResultAttr = copyObject(addInputAttr);
			multActualLeft = copyObject(multLeftAttr);
			multActualLeft->attrType = getNthOfListInt(dts, 0);
			multActualRight = copyObject(multRightAttr);
			multActualRight->attrType = LIST_LENGTH(dts) == 1 ? multActualLeft->attrType : getNthOfListInt(dts, 1);

			multExpr = copyObject(multExpr);
			multType = typeOf(multExpr);
			multResultAttr->attrType = multType;

			addExpr = copyObject(addExpr);
			addType = typeOf(addExpr);

			return addType;
//			DEBUG_LOG("SC: getting SC datatype: %d.", dtFuncIn);
		}
	}

	FATAL_LOG("No semiring combiner info in provenance options.");
}

/**
 * @brief Copies semiring combiner property from one operator to another.
 *
 * @param op copy from this operator
 * @param to copy to this operator
 */

extern void
addSCOptionToChild(QueryOperator *op, QueryOperator *to)
{
	if(HAS_STRING_PROP(op,PROP_PC_SC_AGGR_OPT))
	{
		SET_STRING_PROP(to, PROP_PC_SC_AGGR_OPT, GET_STRING_PROP(op,PROP_PC_SC_AGGR_OPT));
	}
}

extern boolean
onlySemiringSupportedOps(QueryOperator *op)
{
    return onlySemiringSupportedOpsInternal(op);
}


/**
 * @brief Returns true if query only uses operators for which we support semiring operations.
 *
 * @param op the root of the subtree to tests
 * @return true, if only supported operators are used in subtree rooted at op
 */

static boolean
onlySemiringSupportedOpsInternal(QueryOperator *op)
{
    boolean res = TRUE;

    FOREACH(QueryOperator,child,op->inputs)
    {
        res &= onlySemiringSupportedOpsInternal(child);
    }

    if(!(isA(op,ProjectionOperator)
         || isA(op,SelectionOperator)
         || isA(op,ProvenanceComputation)
         || isA(op,TableAccessOperator)
         || isA(op,JoinOperator)
         || isA(op,ConstRelOperator)))
    {
        res = FALSE;
    }

    // no outer joins
    if(isA(op,JoinOperator))
    {
        JoinOperator *j = (JoinOperator *) op;
        if(!(j->joinType == JOIN_CROSS
             || j->joinType == JOIN_INNER))
        {
            res = FALSE;
        }
    }

    // only union set operator
    if(isA(op,SetOperator))
    {
        SetOperator *s = (SetOperator *) op;
        res &= (s->setOpType == SETOP_UNION);
    }

    return res;
}


/**
 * @brief Add operators on top of operator result that implement semiring multiplication and addition.
 * Projection is used for multiplication, multiplying all input provenance attributes and then using aggregation to sum up the results.
 *
 * @param result the query operator on top of which we need to introduce semiring addition and multiplication.
 * @param addExpr expression implementing semiring addition, that contains and aggregation function over a dummy attribtue
 * @param multExpr expression implementing semiring multiplication as a scalar expression over two dummy attribute references
 * @return the root of the operator graph implementing addition on top of result
 */

QueryOperator *
addSemiringCombiner(QueryOperator * result, Node *addExpr, Node *multExpr)
{
    AttributeReference *leftAttr = NULL;
    AttributeReference *rightAttr = NULL;
    HashMap *replMap = NEW_MAP(AttributeReference,Node);
    Node *expre = NULL;

	if(addExpr==NULL || multExpr==NULL)
	{
		FATAL_LOG("SC: addition and multiplication expressions cannot be NULL.");
	}
	List *attrNames = getNormalAttrNames((QueryOperator *)result);
	List *projExprs = getNormalAttrProjectionExprs((QueryOperator *)result);
	List *provExprs = getProvAttrProjectionExprs((QueryOperator *)result);

	// create projection expression multiplying all input provenance attributes
	getAttributeReferencesForSC(multExpr, &leftAttr, &rightAttr, TRUE);

	FOREACH(Node,nd,provExprs) {
        if(!expre)
            expre = copyObject(nd);
        else
        {
            addToMap(replMap, (Node *) rightAttr, nd);
            addToMap(replMap, (Node *) leftAttr, expre);
            expre = deepReplaceAttrRefMutator(copyObject(multExpr), replMap);
            DEBUG_NODE_BEATIFY_LOG("SC has constructed expression so far:", expre);
        }
    }

	// projection for multiplication
	appendToTailOfList(projExprs,expre);
	appendToTailOfList(attrNames,replaceSubstr(exprToSQL(expre, NULL, FALSE), " ", ""));
	QueryOperator *proj = (QueryOperator *)createProjectionOp(projExprs, (QueryOperator *)result, NIL, attrNames);
	proj->provAttrs = singletonInt(getListLength(attrNames)-1);
	switchSubtrees(result, proj);
	result->parents = singleton(proj);

	// compute addition expression
	getAttributeReferencesForSC(addExpr, &leftAttr, &rightAttr, FALSE);
	replMap = NEW_MAP(AttributeReference,Node);
	addToMap(replMap, (Node *) leftAttr, (Node *)  getHeadOfListP(getProvAttrProjectionExprs(proj)));
    expre = deepReplaceAttrRefMutator(copyObject(addExpr), replMap);

    //TODO deal with arithemtics and such in this type of expression, e.g., SUM(X * X) + MAX(X) which requires projection on top and below

    // add aggregation implementing addition
	List *gby = getNormalAttrProjectionExprs(proj);
	attrNames = removeFromTail(attrNames);
	appendToHeadOfList(attrNames,strdup("_PROV"));
	QueryOperator *aggr = (QueryOperator *)createAggregationOp(singleton(expre),gby,proj,NIL,attrNames);
	aggr->provAttrs = singletonInt(0);
	switchSubtrees(proj, aggr);
	proj->parents = singleton(aggr);

	// add final projection
	attrNames = appendToTailOfList(getNormalAttrNames(aggr),"PROV");
	projExprs = concatTwoLists(getNormalAttrProjectionExprs(aggr),getProvAttrProjectionExprs(aggr));
	QueryOperator *proj2 = (QueryOperator *)createProjectionOp(projExprs, (QueryOperator *)aggr, NIL, attrNames);
	proj2->provAttrs = singletonInt(getListLength(attrNames)-1);
	switchSubtrees(aggr, proj2);
	aggr->parents = singleton(proj2);
	result = proj2;

	return result;
}

static void
getAttributeReferencesForSC(Node *expr, AttributeReference **leftName, AttributeReference **rightName, boolean twoInputs)
{
    List *attrRefs = getAttrReferences(expr);
    *leftName = NULL;
    *rightName = NULL;

    FOREACH(AttributeReference,a,attrRefs)
    {
        if (*leftName == NULL)
        {
            *leftName = copyObject(a);
        }
        else if (!twoInputs)
        {
            if (!streq((**leftName).name, a->name))
            {
                 FATAL_LOG("expression for ADD SEMIRING combiner should reference exactly one attribute: %s",
                         beatify(nodeToString(expr)));
            }
        }
        else if (*rightName == NULL)
        {
            if (!streq((**leftName).name, a->name))
            {
                *rightName = copyObject(a);
            }
        }
        else
        {
            if (!streq((**leftName).name, a->name) && !streq((**rightName).name, a->name))
            {
                 FATAL_LOG("expression for MULT SEMIRING combiner should reference exactly two attributes: %s",
                         beatify(nodeToString(expr)));
            }
        }
    }

    if (*leftName == NULL || (*rightName == NULL && twoInputs))
    {
        FATAL_LOG("expression for MULT SEMIRING combiner should reference exactly %s attribute(s): %s",
                        (twoInputs ? "two"  : "one"), beatify(nodeToString(expr)));
    }
}


/**
 * @brief Return true if semiring addition expression is valid.
 *
 * We require this to be an aggregation expression over a dummy attribute
 * reference, e.g., sum(a * 2) + 3 + avg(a) is ok, but a + sum(1) is not
 * (attribute used outside of an aggregation's scope).
 *
 * @param node the expression implementing semiring addition (as an aggregation expression)
 * @param state pointer to boolean recording whether we are withing an aggregation
 * @return true, if the expression is valid
 */

static boolean
addCombinerExprIsOK(Node *node, void *state)
{
    boolean *inAgg = (boolean *) state;

    if (node == NULL)
        return TRUE;

    if (isA(node, FunctionCall))
    {
        FunctionCall *f = (FunctionCall*) node;
        if (isAgg(f->functionname))
        {
            inAgg = NEW(boolean);
            *inAgg = TRUE;
        }
    }

    if (isA(node, AttributeReference) && !(*inAgg))
    {
        return FALSE;
    }

    return visit (node, addCombinerExprIsOK, inAgg);
}

#include "common.h"
#include "log/logger.h"

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
                             SEMIRING_NX);

#define DUMMY_LEFT_ATTR backendifyIdentifier("__prov_left_input")
#define DUMMY_RIGHT_ATTR backendifyIdentifier("__prov_right_input")

/* function declarations */
//static Node *deepReplaceAttrRef(Node * expr, Node *af);//replace all attributeReferences to af
//QueryOperator * addSemiringCombiner(QueryOperator * result);
static void getAttributeReferencesForSC(Node *expr, AttributeReference **leftName,
        AttributeReference **rightName, boolean twoInputs);
static Node *deepReplaceAttrRefMutator(Node *node, HashMap *context);
static boolean addCombinerExprIsOK(Node *node, boolean *inAgg);

static Node *
deepReplaceAttrRefMutator(Node *node, HashMap *context)
{
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


static Node *
semiringMultExpr(Node *p)
{
	switch(p->type){
		case T_Constant:
		{
            Semiring K = userStringToSemiring(STRING_VALUE(p));
            AttributeReference *l, *r;
            DEBUG_LOG("Mult expression for semiring %s", SemiringToString(K));

            switch(K)
            {
                case SEMIRING_N:
                {
                    // l * r
                    l = createFullAttrReference(strdup(DUMMY_LEFT_ATTR),0,0,0,DT_LONG);
                    r = createFullAttrReference(strdup(DUMMY_RIGHT_ATTR),0,0,0,DT_LONG);
			        return (Node *) createOpExpr(OPNAME_MULT,
                                                 LIST_MAKE(l,r));
                }
                case SEMIRING_NX:
                {
                    // "(" || l || " * " || r || ")"
                    List *operands;
                    l = createFullAttrReference(strdup(DUMMY_LEFT_ATTR),0,0,0,DT_STRING);
                    r = createFullAttrReference(strdup(DUMMY_RIGHT_ATTR),0,0,0,DT_STRING);
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
                    l = createFullAttrReference(strdup(DUMMY_LEFT_ATTR),0,0,0,DT_FLOAT);
                    r = createFullAttrReference(strdup(DUMMY_RIGHT_ATTR),0,0,0,DT_FLOAT);
			        return (Node *) createOpExpr(OPNAME_ADD,
                                                 LIST_MAKE(l,r));
                }
                case SEMIRING_VITERBI:
                {
                    // l * r
                    l = createFullAttrReference(strdup(DUMMY_LEFT_ATTR),0,0,0,DT_FLOAT);
                    r = createFullAttrReference(strdup(DUMMY_RIGHT_ATTR),0,0,0,DT_FLOAT);
			        return (Node *) createOpExpr(OPNAME_MULT,
                                                 LIST_MAKE(l,r));
                }
                case SEMIRING_FUZZY:
                {
                    l = createFullAttrReference(strdup(DUMMY_LEFT_ATTR),0,0,0,DT_FLOAT);
                    r = createFullAttrReference(strdup(DUMMY_RIGHT_ATTR),0,0,0,DT_FLOAT);
			        return (Node *) createFunctionCall(LEAST_FUNC_NAME,
                                                       LIST_MAKE(l,r));
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

static Node *
semiringAddExpr(Node *p)
{
	switch(p->type)
	{
	    // user has specified a semiring name (currently only N and N[X])
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
                    a = createFullAttrReference(strdup(DUMMY_LEFT_ATTR),0,0,0,DT_FLOAT);
                    return (Node *) createFunctionCall(MAX_FUNC_NAME,
                                                       singleton(a));

                }
                case SEMIRING_FUZZY:
                {
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
			    FATAL_NODE_BEATIFY_LOG("expression for addition semirign combiner can only use attribute references within aggregation function calls:\n\n", addExpr);


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

extern void
addSCOptionToChild(QueryOperator *op, QueryOperator *to)
{
	if(HAS_STRING_PROP(op,PROP_PC_SC_AGGR_OPT))
	{
		SET_STRING_PROP(to, PROP_PC_SC_AGGR_OPT, GET_STRING_PROP(op,PROP_PC_SC_AGGR_OPT));
	}
}

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


static boolean
addCombinerExprIsOK(Node *node, boolean *inAgg)
{
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

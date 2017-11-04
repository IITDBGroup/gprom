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
#include "utility/string_utils.h"

/* function declarations */
Node *deepReplaceAttrRef(Node * expr, Node *af);//replace all attributeReferences to af
//QueryOperator * addSemiringCombiner(QueryOperator * result);

Node *deepReplaceAttrRef(Node * expr, Node *af){
	switch(expr->type){
		case T_AttributeReference: {
			return (Node *)copyObject(af);
		}
		case T_Operator: {
			FOREACH_LC(lc,((Operator *)expr)->args){
				lc->data.ptr_value = (Node *)deepReplaceAttrRef((Node *)(lc->data.ptr_value), af);
			}
			break;
		}
		default: {
			return expr;
		}
	}
	return expr;
}

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

char *
getSemiringCombinerFuncName(QueryOperator *op){
	Node *p = getStringProperty(op, PROP_PC_SEMIRING_COMBINER);
	switch(p->type){
		case T_Constant: {
			return "sum";
		}
		case T_List: {
			return ((FunctionCall *)getNthOfListP((List *)p,0))->functionname;
		}
		default: {
			FATAL_LOG("unknown expression type for SC option: %s", nodeToString(p));
		}
	}
	return NULL;
}

Node *
getSemiringCombinerExpr(QueryOperator *op){
	Node *p = getStringProperty(op, PROP_PC_SEMIRING_COMBINER);
	switch(p->type){
		case T_Constant: {
			return (Node *)createOpExpr("*", appendToTailOfList(singleton(createAttributeReference("x")),createAttributeReference("y")));
		}
		case T_List: {
			return (Node *)((Operator *)getNthOfListP((List *)p,1));
		}
		default: {
			FATAL_LOG("unknown expression type for SC option: %s", nodeToString(p));
		}
	}
	return NULL;
}

DataType
getSemiringCombinerDatatype(ProvenanceStmt *stmt, List *dts) {
	FOREACH(KeyValue,kv,stmt->options){
		if(streq(STRING_VALUE(kv->key),PROP_PC_SEMIRING_COMBINER)) {
			INFO_LOG(nodeToString(kv->value));
			char * scop = NULL;
			char * funcn = NULL;
			switch(kv->value->type){
				case T_Constant: {
					scop = "*";
					funcn = "sum";
					break;
				}
				case T_List: {
					scop = ((Operator *)getNthOfListP(((List *)kv->value),1))->name;
					funcn = ((FunctionCall *)getNthOfListP(((List *)kv->value),0))->functionname;
					break;
				}
				default: {
					FATAL_LOG("unknown expression type for SC option: %s", nodeToString(kv->value));
				}
			}
			boolean exists = FALSE;
			DataType dtFuncIn = getOpReturnType(scop,sublist(dts,0,1), &exists);
			//INFO_LOG("SC: getting SC datatype: %d.", dtFuncIn);
			return getFuncReturnType(funcn,singletonInt(dtFuncIn), &exists);
		}
	}
	FATAL_LOG("No semiring combiner info in provenance options.");
}

extern void addSCOptionToChild(QueryOperator *op, QueryOperator *to) {
	if(HAS_STRING_PROP(op,PROP_PC_SC_AGGR_OPT)){
		SET_STRING_PROP(to, PROP_PC_SC_AGGR_OPT, GET_STRING_PROP(op,PROP_PC_SC_AGGR_OPT));
	}
}

QueryOperator *
addSemiringCombiner(QueryOperator * result, char * funcN, Node * expr) {
	if(funcN==NULL || expr==NULL){
		FATAL_LOG("SC: function and expression can not be null.");
	}
	List *attrNames = getNormalAttrNames((QueryOperator *)result);
	List *projExprs = getNormalAttrProjectionExprs((QueryOperator *)result);
	List *provExprs = getProvAttrProjectionExprs((QueryOperator *)result);
	Node *expre = NULL;
	char *opN = ((Operator *)expr)->name;
	Node *singExpr = (Node *)getNthOfListP(((Operator *)expr)->args,0);
	FOREACH(Node,nd,provExprs) {
		if(!expre)
			expre = deepReplaceAttrRef((Node *)copyObject(singExpr),nd);
		else
			expre = (Node *)createOpExpr(opN,appendToTailOfList(singleton(expre),deepReplaceAttrRef((Node *)copyObject(singExpr),nd)));
	}
	appendToTailOfList(projExprs,expre);
	appendToTailOfList(attrNames,replaceSubstr(exprToSQL(expre), " ", ""));
	QueryOperator *proj = (QueryOperator *)createProjectionOp(projExprs, (QueryOperator *)result, NIL, attrNames);
	proj->provAttrs = singletonInt(getListLength(attrNames)-1);
	switchSubtrees(result, proj);
	result->parents = singleton(proj);
	//add aggregation
	Node *fc = (Node *)createFunctionCall(funcN,getProvAttrProjectionExprs(proj));
	List *gby = getNormalAttrProjectionExprs(proj);
	attrNames = removeFromTail(attrNames);
	appendToHeadOfList(attrNames,exprToSQL(fc));
	QueryOperator *aggr = (QueryOperator *)createAggregationOp(singleton(fc),gby,proj,NIL,attrNames);
	aggr->provAttrs = singletonInt(0);
	switchSubtrees(proj, aggr);
	proj->parents = singleton(aggr);
	//add projection
	attrNames = appendToTailOfList(getNormalAttrNames(aggr),"PROV");
	projExprs = concatTwoLists(getNormalAttrProjectionExprs(aggr),getProvAttrProjectionExprs(aggr));
	QueryOperator *proj2 = (QueryOperator *)createProjectionOp(projExprs, (QueryOperator *)aggr, NIL, attrNames);
	proj2->provAttrs = singletonInt(getListLength(attrNames)-1);
	switchSubtrees(aggr, proj2);
	aggr->parents = singleton(proj2);
	result = proj2;
	return result;
}

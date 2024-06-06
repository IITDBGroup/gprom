#include "common.h"
#include "log/logger.h"

#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/expression/expression.h"
#include "metadata_lookup/metadata_lookup.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/zonotope_rewrites/zonotope_rewriter.h"
#include "utility/string_utils.h"

/* bound based uncertainty */
#define ATTR_ZONO_PFX backendifyIdentifier("Z_")

static QueryOperator * rewrite_ZonoSelection(QueryOperator *op, boolean attrLevel);

char *
getZonoString(char *in)
{
	StringInfo str = makeStringInfo();
	appendStringInfo(str, "%s", ATTR_ZONO_PFX);
	appendStringInfo(str, "%s", in);
	return str->data;
}

// static void
// addZonoAttrToSchema(HashMap *hmp, QueryOperator *target, Node * aRef)
// {
// 	addAttrToSchema(target, getZonoString(((AttributeReference *)aRef)->name), DT_INT);
// 	((AttributeReference *)aRef)->outerLevelsUp = 0;
// 	ADD_TO_MAP(hmp, createNodeKeyValue(aRef, (Node *)getTailOfListP(getProjExprsForAllAttrs(target))));
// }

QueryOperator *
rewriteZono(QueryOperator * op)
{
	QueryOperator *rewrittenOp;
    INFO_LOG("REWRITE UNCERTAIN");
    INFO_LOG("TEST");

	switch(op->type)
	{
		case T_SelectionOperator:
        {
			rewrittenOp = rewrite_ZonoSelection(op, TRUE);
			INFO_OP_LOG("Uncertainty Rewrite Selection:", rewrittenOp);
        }
        break;
        default:
        {
            rewrittenOp = op;
        }
        break;
	}

	return rewrittenOp;
}

static QueryOperator *
rewrite_ZonoSelection(QueryOperator *op, boolean attrLevel)
{

	ASSERT(OP_LCHILD(op));

    // INFO_LOG("REWRITE-ZONO - Selection (%s)", attrLevel ? "ATTRIBUTE LEVEL" : "TUPLE LEVEL");
	DEBUG_LOG("Operator tree \n%s", nodeToString(op));

	// // Rewrite child first
	// if (attrLevel)
	// {
	// 	rewriteZono(OP_LCHILD(op));
	// }

    // HashMap *hmp = NEW_MAP(Node, Node);

	// if(attrLevel)
	// {
	// 	List *attrExpr = getProjExprsForAllAttrs(op);
	// 	FOREACH(Node, nd, attrExpr)
    //     {
    //     	addZonoAttrToSchema(hmp, op, nd);
	// 	}
	// }
	// addZonoAttrToSchema(hmp, op, (Node *)createAttributeReference(backendifyIdentifier("Z")));
	// setStringProperty(op, "ZONO_MAPPING", (Node *)hmp);

    // // Create projection to calculate row uncertainty
    // QueryOperator *proj = (QueryOperator *)createProjectionOp(getProjExprsForAllAttrs(op), op, NIL, getQueryOperatorAttrNames(op));
    // switchSubtrees(op, proj);
    // op->parents = singleton(proj);

    // setStringProperty(proj, "ZONO_MAPPING", (Node *) copyObject(hmp));

	// LOG_RESULT("ZONOTOPE: Rewritten Operator tree [SELECTION]", op);

	// return proj;

    return op;
}


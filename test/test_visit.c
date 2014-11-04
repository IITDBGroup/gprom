#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "configuration/option.h"
#include "configuration/option_parser.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "parser/parser.h"
#include "rewriter.h"

/*
 * declarations.
 */
static boolean visitTheNode (Node *node, void *state);

int
main (int argc, char* argv[])
{
    Node *result;

    READ_OPTIONS_AND_INIT("testvisit", "Run visit function on input for testing.");

    // read from terminal
    if (getStringOption("input.sql") == NULL)
    {
        result = parseStream(stdin);

        DEBUG_LOG("Address of returned node is <%p>", result);
        ERROR_LOG("PARSE RESULT FROM STREAM IS <%s>", beatify(nodeToString(result)));
    }
    // parse input string
    else
    {
        result = parseFromString(getStringOption("input.sql"));

        DEBUG_LOG("Address of returned node is <%p>", result);
        ERROR_LOG("PARSE RESULT FROM STRING IS:\n%s", nodeToString(result));
        ERROR_LOG("PARSE RESULT FROM STRING IS:\n%s", beatify(nodeToString(result)));
    }


    void *state = NULL;
    visitTheNode(result, state);

    return shutdownApplication();
}

static boolean
visitTheNode(Node *node, void *state)
{
    if (node == NULL)
        return TRUE;

	switch (node->type) {
	case T_Invalid: printf("The type of the node is Invalid\n"); break;
	/* lists */

	case T_List: printf("The type of the node is List\n"); break;
	case T_IntList: printf("The type of the node is IntList\n"); break;

		    /* sets */
	case T_Set: printf("The type of the node is Set\n"); break;

		    /* expression nodes */
	case T_Constant: printf("The type of the node is Constant\n"); break;
	case T_AttributeReference: printf("The type of the node is AttributeReference\n"); break;
	case T_FunctionCall: printf("The type of the node is FunctionCall\n"); break;
	case T_Operator: printf("The type of the node is Operator\n"); break;

		    /* query block model nodes */
	case T_SetQuery: printf("The type of the node is SetQuery\n"); break;
	case T_ProvenanceStmt: printf("The type of the node is ProvenanceStmt\n"); break;
	case T_QueryBlock: printf("The type of the node is QueryBlock\n"); break;
	case T_SelectItem: printf("The type of the node is SelectItem\n"); break;
	case T_FromItem: printf("The type of the node is FromItem\n"); break;
	case T_FromProvInfo: printf("The type of the node is FromProvInfo\n"); break;
	case T_FromTableRef: printf("The type of the node is FromTableRef\n"); break;
	case T_FromSubquery: printf("The type of the node is FromSubquery\n"); break;
	case T_FromJoinExpr: printf("The type of the node is FromJoinExpr\n"); break;
	case T_DistinctClause: printf("The type of the node is DistinctClause\n"); break;
	case T_NestedSubquery: printf("The type of the node is NestedSubquery\n"); break;
	case T_Insert: printf("The type of the node is Insert\n"); break;
	case T_Delete: printf("The type of the node is Delete\n"); break;
	case T_Update: printf("The type of the node is Update\n"); break;
	case T_TransactionStmt: printf("The type of the node is TransactionStmt\n"); break;

		    /* query operator model nodes */
	case T_Schema: printf("The type of the node is Schema\n"); break;
	case T_AttributeDef: printf("The type of the node is AttributeDef\n"); break;
	case T_QueryOperator: printf("The type of the node is QueryOperator\n"); break;
	case T_SelectionOperator: printf("The type of the node is SelectionOperator\n"); break;
	case T_ProjectionOperator: printf("The type of the node is ProjectionOperator\n"); break;
	case T_JoinOperator: printf("The type of the node is JoinOperator\n"); break;
	case T_AggregationOperator: printf("The type of the node is AggregationOperator\n"); break;
	case T_ProvenanceComputation: printf("The type of the node is ProvenanceComputation\n"); break;
	case T_TableAccessOperator: printf("The type of the node is TableAccessOperator\n"); break;
	case T_SetOperator: printf("The type of the node is SetOperator\n"); break;
	case T_DuplicateRemoval: printf("The type of the node is DuplicateRemoval\n"); break;
	case T_ConstRelOperator: printf("The type of the node is ConstRelOperator\n"); break;
	case T_ProvenanceTransactionInfo: printf("The type of the node is ProvenanceTransactionInfo\n"); break;
	case T_KeyValue: printf("The type of the node is KeyValue\n"); break;
    case T_NestingOperator: printf("The type of the node is NestingOperator\n"); break;
    case T_SQLParameter: printf("The type of the node is SQLParameter\n"); break;
    default: printf("Nodetype not added to test yet"); break;
	}



	//printf("The type of the node is %d\n", node->type);
	return visit(node, visitTheNode, state);
}

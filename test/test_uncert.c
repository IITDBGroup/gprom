/*-----------------------------------------------------------------------------
 *
 * test_uncert.c
 *			  
 *		
 *		AUTHOR: Su Feng
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "configuration/option.h"
#include "configuration/option_parser.h"
#include "analysis_and_translate/translator.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "parser/parser.h"
#include "rewriter.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"
#include "model/expression/expression.h"
#include "metadata_lookup/metadata_lookup.h"

#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/uncertainty_rewrites/uncert_rewriter.h"
#include "utility/string_utils.h"
#include "execution/executor.h"
#include "sql_serializer/sql_serializer.h"

void testExpr(Node *result);
void testQuery(Node *result);
void testRemoveUncert(Node* result);

int
main (int argc, char* argv[])
{
	Node *result;

	READ_OPTIONS_AND_INIT("testparser", "Run parser stage only.");

	//parse expression
	if (getStringOption("input.sql") == NULL)
	    {
	        result = parseStream(stdin);
	     	//INFO_LOG("Address of returned node is <%p>", result);
	     	//INFO_LOG("PARSE RESULT FROM STREAM IS <%s>", beatify(nodeToString(result)));
	    }
	    // parse input string
	    else
	    {
	        result = parseFromString(getStringOption("input.sql"));

	        //INFO_LOG("Address of returned node is <%p>", result);
	        //ERROR_LOG("PARSE RESULT FROM STRING IS:\n%s", nodeToString(result));
	        //INFO_LOG("PARSE RESULT FROM STRING IS:\n%s", beatify(nodeToString(result)));
	    }
	//testing expression uncertainty propagation
	//testExpr(result);
	//testRemoveUncert(result);
	testQuery(result);

	    shutdownApplication();

	    return EXIT_SUCCESS;
		//test end
}

void testExpr(Node *result){
	HashMap * hmp = NEW_MAP(Node, Node);
						ADD_TO_MAP(hmp, createNodeKeyValue((Node *)createAttributeReference("A"), (Node *)createAttributeReference("U_A")));
						ADD_TO_MAP(hmp, createNodeKeyValue((Node *)createAttributeReference("B"), (Node *)createAttributeReference("U_B")));
						ADD_TO_MAP(hmp, createNodeKeyValue((Node *)createAttributeReference("C"), (Node *)createAttributeReference("U_C")));
						ADD_TO_MAP(hmp, createNodeKeyValue((Node *)createAttributeReference("D"), (Node *)createAttributeReference("U_D")));
		if(isA(result, List)) {
			result = (Node *)getHeadOfListP((List *)result);
		}
		if(isA(result, Operator) || isA(result, CaseExpr)) {
			//result = result
		}
		else if(isA(result, QueryBlock)){
			result = ((QueryBlock *)result)->whereClause;
		}
		else {
			ERROR_LOG("Invalid input: %s", nodeToString(result));
		}
		INFO_LOG("expression in: %s\n", exprToSQL(result, NULL));
			Node * retexp = getUncertaintyExpr(result, hmp);
			INFO_LOG("expression out: %s\n", exprToSQL(retexp, NULL));
}

void testRemoveUncert(Node* result){
	if(isA(result, List)) {
				result = (Node *)getHeadOfListP((List *)result);
			}
			if(isA(result, Operator) || isA(result, CaseExpr)) {
				//result = result
			}
			else if(isA(result, QueryBlock)){
				result = ((QueryBlock *)result)->whereClause;
			}
			else {
				ERROR_LOG("Invalid input: %s", nodeToString(result));
			}
			INFO_LOG("expression in: %s\n", exprToSQL(result, NULL));
			Node * retexp = removeUncertOpFromExpr(result);
			INFO_LOG("expression out: %s\n", exprToSQL(retexp, NULL));
}

void testQuery(Node *result) {
	//testing query operator
	Node *qoModel = translateParse(result);
	INFO_LOG("TRANSLATION RESULT FROM STRING IS:\n%s", nodeToString(qoModel));

	QueryOperator *ret = rewriteUncert((QueryOperator *)getHeadOfListP((List *)qoModel));
	INFO_OP_LOG("Rewritten query: ", ret);

	char *plan = serializeOperatorModel((Node *)ret);
	INFO_LOG("Rewritten SQL: %s", plan);

	execute(plan);
}

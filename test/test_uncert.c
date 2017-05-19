/*-----------------------------------------------------------------------------
 *
 * test_uncert.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
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
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "parser/parser.h"
#include "rewriter.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"
#include "model/expression/expression.h"
#include "metadata_lookup/metadata_lookup.h"

#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/semiring_combiner/sc_main.h"
#include "utility/string_utils.h"

int
main (int argc, char* argv[])
{
	Node *result;

	READ_OPTIONS_AND_INIT("testparser", "Run parser stage only.");

	//parse expression
	if (getStringOption("input.sql") == NULL)
	    {
	        result = parseStream(stdin);
	     	INFO_LOG("Address of returned node is <%p>", result);
	     	INFO_LOG("PARSE RESULT FROM STREAM IS <%s>", beatify(nodeToString(result)));
	    }
	    // parse input string
	    else
	    {
	        result = parseFromString(getStringOption("input.sql"));

	        INFO_LOG("Address of returned node is <%p>", result);
	        //ERROR_LOG("PARSE RESULT FROM STRING IS:\n%s", nodeToString(result));
	        INFO_LOG("PARSE RESULT FROM STRING IS:\n%s", beatify(nodeToString(result)));
	    }
	//testing expression uncertainty propagation
	HashMap * hmp = NEW_MAP(Node, Node);
					ADD_TO_MAP(hmp, createNodeKeyValue((Node *)createAttributeReference("A"), (Node *)createAttributeReference("U_A")));
					ADD_TO_MAP(hmp, createNodeKeyValue((Node *)createAttributeReference("B"), (Node *)createAttributeReference("U_B")));
					ADD_TO_MAP(hmp, createNodeKeyValue((Node *)createAttributeReference("C"), (Node *)createAttributeReference("U_C")));
					ADD_TO_MAP(hmp, createNodeKeyValue((Node *)createAttributeReference("D"), (Node *)createAttributeReference("U_D")));
	Set *st = PSET();
	if(isA(result, List)) {
		result = (Node *)getHeadOfListP((List *)result);
	}
	if(isA(result, Operator)) {
		//result = result
	}
	else if(isA(result, QueryBlock)){
		result = ((QueryBlock *)result)->whereClause;
	}
	else {
		ERROR_LOG("Invalid input: %s", nodeToString(result));
	}
	INFO_LOG("expression in: %s\n", exprToSQL(result));
		Node * retexp = getUncertaintyExpr(result, hmp, st);
		INFO_LOG("expression out: %s\n", exprToSQL(retexp));

	    shutdownApplication();

	    return EXIT_SUCCESS;
		//test end
}

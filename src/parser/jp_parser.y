%{
#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/datalog/datalog_model.h"
#include "model/query_block/query_block.h"
#include "parser/parse_internal_jp.h"
#include "log/logger.h"
#include "model/query_operator/operator_property.h"

#define RULELOG(grule) \
    { \
        TRACE_LOG("Parsing grammer rule <%s>", #grule); \
    }
    
#undef free
#undef malloc

Node *jpParseResult = NULL;
%}

%name-prefix "jp"

%union {
    /* 
     * Declare some C structure those will be used as data type
     * for various tokens used in grammar rules.
     */
     Node *node;
     List *list;
     char *stringVal;
     int intVal;
     double floatVal;
}

/*
 * Declare tokens for name and literal values
 * Declare tokens for user variables
 */
%token <stringVal> IDENTWITHSYMBOL
%token <stringVal> VARIDENT
%token <stringVal> IDENT
/*
 * Tokens for in-built keywords
 *        Currently keywords related to basic query are considered.
 *        Later on other keywords will be added.
 */
%token <stringVal> NEGATION RULE_IMPLICATION ANS WHYPROV WHYNOTPROV GP

/* tokens for constant and idents */
%token <intVal> intConst
%token <floatVal> floatConst
%token <stringVal> stringConst

/* comparison and arithmetic operators */
%token <stringVal> comparisonOp arithmeticOp

/*
 * Declare token for operators specify their associativity and precedence
 */
%left NEGATION

%nonassoc '(' ')'


/*
 * Types of non-terminal symbols
 */
/* statements and their parts */ 
%type <list> actualpath path
%type <stringVal> pathstep


/* start symbol */
%start path

/*************************************************************/
/* RULE SECTION												 */
/*************************************************************/
%%

path:
		 '$'            {RULELOG("path::$");}
                 | '$' actualpath
		 { 
				RULELOG("path::$ actualpath");
                                $$ = $2;
				jpParseResult = (Node *) $2;
				DEBUG_LOG("parsed %s", nodeToString($$));
		}
		;	
actualpath:
		pathstep
                { 
                 RULELOG("actualpath::pathstep"); 
                 Node *n = (Node *) createJsonPath ($1);
                 $$ = singleton(n); 
                }

                | actualpath pathstep 
                {
                 RULELOG("actualpath::pathstep::actualpath"); 
                 Node *n = (Node *) createJsonPath ($2);
                 $$ = appendToTailOfList($1, n);
                }
                ;

pathstep:		
                '.' '*'
                { 
                 RULELOG("pathstep::.*"); $$ = "*"; 
                }

                | '.' IDENT
                {
                 RULELOG("pathstep::.IDENT"); $$ = $2;
                }

                | '[' '*' ']'
                {
                 RULELOG("pathstep::[*]"); $$ = "*";
                }

                | '[' intConst ']'
                {
                 RULELOG("pathstep::[*]"); $$ = $2;
                }
                ;
                

%{
#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/rpq/rpq_model.h"
#include "model/query_block/query_block.h"
#include "parser/parse_internal_rpq.h"
#include "log/logger.h"
#include "model/query_operator/operator_property.h"

#define RULELOG(grule) \
    { \
        TRACE_LOG("Parsing grammer rule <%s>", #grule); \
    }
    
#undef free
#undef malloc

Node *rpqParseResult = NULL;
%}

%define api.prefix {rpq}

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
%token <stringVal> PLUS OPTIONAL STAR DOT OR

/*
 * Declare token for operators specify their associativity and precedence
 */
%left OR DOT
%right PLUS OPTIONAL STAR 
%nonassoc '(' ')'


/*
 * Types of non-terminal symbols
 */
/* statements and their parts */ 
%type <node> rpq regexpr
%type <stringVal> label


/* start symbol */
%start rpq

/*************************************************************/
/* RULE SECTION												 */
/*************************************************************/
%%

rpq:
		regexpr
		{ 
				RULELOG("rpq:label");
         	    $$ = $1;
				rpqParseResult = (Node *) $1;
				DEBUG_LOG("parsed %s", nodeToString($$));
		}	
	;

regexpr:
		label { $$ = (Node *) makeRegexLabel($1); }
		| '(' regexpr ')' { $$ = $2; }
		| regexpr OR regexpr { $$ = (Node *) makeRegex(LIST_MAKE($1,$3), "|"); }
		| regexpr DOT regexpr { $$ = (Node *) makeRegex(LIST_MAKE($1,$3), "."); }
		| regexpr PLUS { $$ = (Node *) makeRegex(singleton($1), "+"); }
		| regexpr OPTIONAL { $$ = (Node *) makeRegex(singleton($1), "?"); }
		| regexpr STAR { $$ = (Node *) makeRegex(singleton($1), "*"); }
	;
	
label:
		IDENT { $$ = $1; }
	;

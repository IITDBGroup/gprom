%{
#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "parser/parse_internal_dl.h"
#include "log/logger.h"
#include "model/query_operator/operator_property.h"

#define RULELOG(grule) \
    { \
        TRACE_LOG("Parsing grammer rule <%s>", #grule); \
    }
    
#undef free

Node *dlParseResult = NULL;
%}

%name-prefix "dl"

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
 %token <stringVal> StringLiteral Identifier
  
/*
 * Tokens for in-built keywords
 *        Currently keywords related to basic query are considered.
 *        Later on other keywords will be added.
 */
%token <stringVal> IDENT

%token <intVal> intConst
%token <floatVal> floatConst
%token <stringVal> stringConst
%token <stringVal> identifier
%token <stringVal> parameter
%token <stringVal> '+' '-' '*' '/' '%' '^' '&' '|' '!' comparisonOps ')' '(' '='


/*
 * Declare token for operators specify their associativity and precedence
 *
%left MINUS

/* Logical operators *
%left '&'
%left '!'
%left ':-'

/* Comparison operator *
%left comparisonOps
%right NOT
%left AND OR

/* Arithmetic operators : FOR TESTING *
%nonassoc DUMMYEXPR
%left '+' '-'
%left '*' '/' '%'
%left '^'
%nonassoc '(' ')'

%left NATURAL JOIN CROSS LEFT FULL RIGHT INNER

/*
 * Types of non-terminal symbols
 */
/* simple types */
 
/* statements and their parts */ 
%type <list> stmtList
%type <node> statement 

%type <node> rule fact rulehead rulebody atomList atom constAtom argList constList arg variable constant

/* start symbol */
%start stmtList

/*************************************************************/
/* RULE SECTION 											 */
/*************************************************************/
%%

/* Rule for all types of statements */
stmtList: 
		statement ';'
			{ 
				RULELOG("stmtList::statement"); 
				$$ = singleton($1);
				dlParseResult = (Node *) $$;	 
			}
		| stmtList statement ';' 
			{
				RULELOG("stmtlist::stmtList::statement");
				$$ = appendToTailOfList($1, $2);	
				dlParseResult = (Node *) $$; 
			}
	;
	
statement:
		rule { $$ = NULL; }
		| fact { $$ = NULL; }
	;
	
rule:
		rulehead RULE_IMPLICATION rulebody '.' { $$ = NULL; }
	;
	
fact:
		constAtom { $$ = NULL; } /* do more elegant? */
	;

rulehead:
		atom  { $$ = NULL; }
	;
	
rulebody:
		atomList { $$ = NULL; }
	;
	
atomList:
		atomList atom { $$ = NULL; }
		| atom		  { $$ = NULL; }
	;

atom:
 		NEGATION ident '(' argList ')' { $$ = NULL; }
 		| ident '(' argList ')' { $$ = NULL; }
 	;

constAtom:
		ident '(' constList ')' { $$ = NULL; }
	;

argList:
 		argList arg { $$ = NULL; }
 		| arg		{ $$ = NULL; }
 	;
 	
constList:
		constList constant { $$ = NULL; }
		| constant { $$ = NULL; }
	;
 	
arg:
 		variable { $$ = NULL; }
 		| constant { $$ = NULL; }
 		//TODO skolem
	;
 		
variable:
		VARIDENT { $$ = NULL; }
	;
	
constant: 
		intConst { $$ = NULL; }
		| floatConst { $$ = NULL; }
		| stringConst { $$ = NULL; }
	;
	

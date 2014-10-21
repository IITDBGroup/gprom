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
%token <stringVal> NEGATION RULE_IMPLICATION

/* tokens for constant and idents */
%token <intVal> intConst
%token <floatVal> floatConst
%token <stringVal> stringConst
%token <stringVal> IDENT
%token <stringVal> VARIDENT
%token <stringVal> parameter

/* comparison and arithmetic operators */
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

%type <node> rule fact rulehead rulebody atomList atom argList  arg variable constant program

/* start symbol */
%start program

/*************************************************************/
/* RULE SECTION 											 */
/*************************************************************/
%%

program:
		stmtList 
			{ 
				RULELOG("program::stmtList");
				$$ = (Node *) createDLProgram ($1, NULL);
				dlParseResult = (Node *) $$; 
			}
		;

/* Rule for all types of statements */
stmtList: 
		statement ';'
			{ 
				RULELOG("stmtList::statement"); 
				$$ = singleton($1);
			}
		| stmtList statement ';' 
			{
				RULELOG("stmtlist::stmtList::statement");
				$$ = appendToTailOfList($1, $2); 
			}
	;
	
statement:
		rule { $$ = $1; }
		| fact { $$ = $1; }
	;
	
rule:
		rulehead RULE_IMPLICATION rulebody '.' 
			{ 
				$$ = (Node *) createDLRule($1,$3); 
			}
	;
	
fact:
		atom '.' { $$ = $1; } /* do more elegant? */
	;

rulehead:
		atom  { $$ = $1; }
	;
	
rulebody:
		atomList { $$ = $1; }
	;
	
atomList:
		atomList ',' atom { $$ = appendToTailOfList($1,$3); }
		| atom		  { $$ = singleton($1); }
	;

atom:
 		NEGATION IDENT '(' argList ')' { $$ = createDLAtom($2, $4, TRUE); }
 		| IDENT '(' argList ')' { $$ = createDLAtom($1, $3, FALSE); }
 	;

/*
constAtom:
		IDENT '(' constList ')' { $$ = createDLAtom($1,$3,FALSE); }
	;
*/

argList:
 		argList arg { $$ = appendToTailOfList($1,$2); }
 		| arg		{ $$ = singleton($1); }
 	;

/* 	
constList:
		constList ',' constant { $$ = appendToTailOfList($1,$2); }
		| constant { $$ = singleton($1); }
	;
*/

/* add skolem */ 	
arg:
 		variable { $$ = $1; }
 		| constant { $$ = $1; }
	;
 		
variable:
		VARIDENT { $$ = (Node *) createConstString($1); }
	;
	
constant: 
		intConst { $$ = (Node *) createConstInt($1); }
		| floatConst { $$ = (Node *) createConstFloat($1); }
		| stringConst { $$ = (Node *) createConstString($1); }
	;
	

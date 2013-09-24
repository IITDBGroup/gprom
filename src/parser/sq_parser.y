/*
 * Sql_Parser.y
 *     This is a bison file which contains grammar rules to parse SQLs
 */

%{
#include <stdio.h>
#include <conio.h>
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"

int yylex(void);
void yyerror (char const *);
%}

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
%token <node> expression
%token <stringVal> aliasName tableName
%token <intVal> intConst
%token <floatVal> floatConst
%token <stringVal> stringConst
%token <node> identifier		// Already node was declared as pointer so removed *
%token <node> argument			// Already node was declared as pointer so removed *

/*
 * Declare token for operators specify their associativity and precedence
 */
/* Logical Operators */
%left '||'
%left XOR
%left '&&'
%right ':='
%left '!'
/* Comparison operator */
%left '==' '<' '>' '<=' '>=' '<>' '!='

/* Arithmetic operators */
%left '+' '-'
%left '*' '/' '%'
%left '^'

/*
 * Tokens for in-built keywords
 *		Currently keywords related to basic query are considered.
 *		Later on other keywords will be added.
 */
%token SELECT
%token PROVENANCE OF
%token FROM
%token AS
%token WHERE
%token DISTINCT
%token ON
%token STARALL
%token AND OR LIKE NOT IN NULL BETWEEN

/*
 * Tokens for functions
*/
%token FUNCIDENTIFIER	
		// Added a token for all functions inbuilt and user defined functions.

%type <node> stmt provStmt dmlStmt
%type <queryBlockModel> selectQuery	deleteQuery updateQuery
		// Its a query block model that defines the structure of query.
%type <list> selectClause fromClause exprList // select and from clauses are lists
%type <node> selectItem optionalDistinct whereClause
%type <node> expression constant attributeRef operatorExpression sqlFunctionCall

%start stmt

%%

/* Rule for all types of statements */
stmt: 
		provStmt ';'
    	| dmlStmt ';'		// DML statement can be select, update, insert, delete
    ;

/* 
 * Rule to parse a query asking for provenance
 */
provStmt: 
 		PROVENANCE OF '(' dmlStmt ')' 	{ $$ = createProvStmt($4); }
 	;

/*
 * Rule to parse all DML queries.
 */
dmlStmt:
		selectQuery
		| deleteQuery
		| updateQuery
	;
/*
 * Rule to parse select query
 * Currently it will parse following type of select query:
 * 			'SELECT [DISTINCT clause] selectClause FROM fromClause WHERE whereClause'
 */
selectQuery: 
		SELECT optionalDistinct selectClause optionlFrom optionalWhere
			{
				$$ = createQueryBlock($2,$3,$4,$5);
			}
	;


/*
 * Rule to parse optional distinct clause.
 */ 
optionalDistinct: 
		/* empty */ 					{ $$ = NULL; }
		| DISTINCT						{ $$ = createDistinct(NULL); }
		| DISTINCT ON '(' exprList ')'	{ $$ = createDistinct($4); }
	;
						

/*
 * Rule to parse the select clause items.
 */
selectClause: 
		selectItem 						{ $$ = $1; }
		| selectClause ',' selectItem 
			{ 
				$$ = appendToTailOfList($1, $3); 
			}
	;

selectItem:
 		expression							
 			{ 
 				$$ = createProjectionNode($1, NULL); 
 			}
 		| expression AS aliasName 			
 			{ 
 				$$ = createProjectionNode($1, $3);
			}
	; 

/*
 * Rule to parse an expression list
 */
exprList: 
		expression						{ $$ = singleton($1); }		
		 | exprList ',' expression 		{ $$ = appendToTailOfList($1, $3); }
	;
		 

/*
 * Rule to parse expressions used in various lists
 */
expression: 
		constant
		| attributeRef 
		| operatorExpression
		| sqlFunctionCall
		| STARALL		/* this token is for '*' */
	;
			
/*
 * Constant parsing
 */
constant: 
		intConst			{ $$ = createIntConst($1); }
		| floatConst		{ $$ = createFloatConst($1); }
		| stringConst		{ $$ = createStringConst($1); }
	;
			
/*
 * Parse attribute reference
 */
attributeRef: 
		identifier 		{ $$ = createAttributeReference($1); }
	;

/*
 * Parse operator expression
 */
operatorExpression: 
		expression operator expression 		{ $$ = createOpExpression($1,$2,$3) }
	;

operator: 
		arithmaticOperator
		| logicalOperator
		| comparisonOperator
		| sqlOperators
	;
	
arithmaticOperator:
		'+' | '-' | '*' | '/' | '%' | '^' 		{ $$ = $1; }
	;

logicalOperator:
		'&&' | '||' | '!'		{ $$ = $1; }
	;
	
comparisonOperator:
		'==' | '<' | '>' | '<=' | '>=' | '!='		{ $$ = $1; }
	;
	
sqlOperators:
		AND | OR | NOT | IN | NULL | BETWEEN | LIKE 	{ $$ = $1; }
	;
	
/*
 * Rule to parse function calls
 */
sqlFunctionCall: 
		FUNCIDENTIFIER '(' exprList ')'  		
			{ 
				$$ = createFunctionCall($1, $3); 
			}
	;

/*
 * Rule to parse from clause
 *			Currently implemented for basic from clause.
 *			Later on other forms of from clause will be added.
 */
optionlFrom: 
		/* empty */ 		 	{ $$ = NULL; }
		| FROM fromClause		{ $$ = $2; }
	;
			
fromClause: 
		fromClauseItem					{ $$ = singeltonP($1); }
		| fromClause ',' fromClauseItem { $$ = appendToTailOfList($1, $3); }
	;
	
fromClauseItem:
		tableName optionalAlias { $$ = crateTableNode($1, $2); }
	;
	
optionalAlias: 
		aliasName				{ $$ = $1; }
		| AS aliasName			{ $$ = $2; }
	;
		  
/*
 * Rule to parse the where clause.
 */
optionalWhere: 
		/* empty */ 			{ $$ = NULL; }
		| WHERE expression			{ $$ = $1; } 
	;

%%
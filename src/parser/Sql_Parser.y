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

/******************************************************************************
 * UNION
 *****************************************************************************/

%union {
	/* 
	 * Declare some C structure those will be used as data type
     * for various tokens used in grammar rules.
	 */
	 Node *node;
	 List *list;
//	 Expression *expr; Expressions are just nodes
	 char *stringVal;
	 int intVal;
	 double floatVal;
}

/******************************************************************************
 * TOKENS AND PREFERENCE RULES
 *****************************************************************************/

/*
 * Declare tokens for name and literal values
 * Declare tokens for user variables
 */
%token <node> expression // you already have declared these things to be pointers
%token <stringVal> aliasName tableName // not sure whether these should both be tokes
%token <intVal> intConst
%token <floatVal> floatConst
%token <stringVal> stringConst
%token <node> indentifier
%token <node> argument

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
%token PROVENANCE
%token OF
%token FROM
%token AS
%token WHERE
%token DISTINCT
%token ON
%token STARALL

/*
 * Tokens for functions
 *		Currently few simple functions are considered.
 *		Later on other functions will be added too.
 */
%token COUNT //defining each function as a token  would be cumbersome
%token AVG
%token MIN
%token MAX

/******************************************************************************
 * TYPES OF NON-TERMINALS AND START SYMBOL
 *****************************************************************************/

%type <node> stmt provStmt // can be any type of node, but for now only mainSelectQuery
%type <queryBlockModel> mainSelectQuery			
		// Its a query block model that defines the structure of query.
%type <node> whereClause 
%type <list> selectClause fromClause exprList // select and from clauses are lists
%type <node> selectItem optionalDistinct
%type <node> expression constant attributeRef operatorExpression sqlFunctionCall

%start stmt

/******************************************************************************
 * GRAMMAR RULES
 *****************************************************************************/
%%

/******************************************************************************
 * SQL STATEMENTS
 *****************************************************************************/

/* 
 * Rule for all types of statements 
 */
stmt: 
		provStmt ';'
    	| mainSelectQuery ';'
    ;

/* 
 * Rule to parse a query asking for provenance
 */
 provStmt: 
 		PROVENANCE OF '(' mainSelectQuery ')' 	{ $$ = createProvStmt($1); }
 	;

/*
 * Rule to parse select query
 * Currently it will parse following type of select query:
 * 			'SELECT [DISTINCT clause] selectClause FROM fromClause WHERE whereClause'
 */
mainSelectQuery: 
		SELECT optionalDistinct selectClause optionalFrom optionalWhere
			{
				$$ = createQueryBlock();
				$$->distinct = $2;
				$$->selectClause = $3;
				$$->fromClause = $4;
				$$->whereClause = $5;
			}
	;

/******************************************************************************
 * SELECT CLAUSE
 *****************************************************************************/

/*
 * For parsing optional distinct clause.
 */ 
optionalDistinct: 
		/* empty */ 					{ $$ = NULL; }
		| DISTINCT						{ $$ = createDistinctClause(NULL); }
		| DISTINCT ON '(' exprList ')'	{ $$ = createDistinctClause($4); }
	;
						

/*
 * parse the select clause items.
 */
selectClause: 
		selectItem 						{ $$ = singleton($1); }
		| selectClause ',' selectItem 
			{ 
				$$ = appendToTailOfList($1, $3); 
			}
	;

/*
 *
 */
 selectItem:
 		expression							
 			{ 
 				$$ = createSelectItem($1, NULL); 
 			}
 		| expression AS aliasName 			
 			{ 
 				$$ = createSelectItem($1, $3);
			}
	; 

/******************************************************************************
 * QUERY FROM CLAUSE
 *****************************************************************************/		

/*
 * Parser to parse from clause
 *			Currently implemented for basic from clause.
 *			Later on other forms of from clause will be added.
 */
optionlFrom: 
		/* empty */ 		 	{ $$ = NULL; }
		| FROM fromClause		{ $$ = $2; }
	;
			
fromClause: 
		fromClauseItem					{ $$ = singleton($1); }
		| fromClause ',' fromClauseItem { $$ = appendToTailOfList($1, $3); }
	;
	
fromClauseItem:
		tableName optionalAlias { $$ = createFromTableRef($2, NULL, $1); }
	;
	
optionalAlias: 
		aliasName				{ $$ = $1; }
		| AS aliasName			{ $$ = $2; }
	;

/******************************************************************************
 * WHERE AND HAVING CLAUSES
 *****************************************************************************/
		  
/*
 * Rule to parse the where clause.
 * 			Currently only simple where clause is considered.
 *				e.g.   table1.col1 == table2.col1
 *			Later on, other arguments will be considered.
 */
optionalWhere: 
		/* empty */ 			{ $$ = NULL}
		| WHERE expression		{ $$ = $1 } 
	;
	
/******************************************************************************
 * EXPRESSIONS
 *****************************************************************************/

/*
 * Parse an expression list
 */
exprList: 
		expression						{ $$ = singletonP($1); }		
		 | exprList ',' expression 		{ $$ = appendToTailOfList($1, $3); }
	;
		 

/*
 * Parse expressions/attributes/functions used in the select clause.
 */
expression: 
		constant
		| attributeRef 
		| operatorExpression
		| sqlFunctionCall
		| STARALL		/* this token is for '*' */
	;
			
/*
 * constant parsing
 */
constant: 
		intConst			{ $$ = createIntConst($1); }
		| floatConst		{ $$ = createFloatConst($1); }
		| stringConst		{ $$ = createStringConst($1); }
	;
			
/*
 * parse attribute reference
 */
attributeRef: 
		identifier 		{ $$ = createAttributeReference($1); }
	;

/*
 * parse operator expression
 */
operatorExpression: 
		expression operator expression 		{ $$ = createOpExpression($2,$1,$3) }
	;

operator: 
		'+' | '-' | '*' | '/' | '%' | '||' | '&&' | '!' | '^' 		{$$ = $1}
	;
		
/*
 * Function call parser
 */
 
// This does not allow for UDFs. Similar to programming language parsers we need to have a token IDENT that can be used as a function name or attribute reference
sqlFunctionCall: 
		sqlFunctionName '(' exprList ')'  		{ $$ = createFunctionCall($1, $3); } /* Call a function that will create
											 * a function node.
											 */
	;

/* 
 * This grammar rule is a sample grammar rule to identify functions.
 * Later on other functions of Oracle will be added. 
 */
sqlFunctionName:  
		COUNT
		| AVG
		| MAX
		| MIN
	;
	
			
%%
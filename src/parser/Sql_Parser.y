/*
 * Sql_Parser.y
 *     This is a bison file which contains grammar rules to parse SQLs
 */

%{
#include <stdio.h>
#include <conio.h>
#include <exprssion.h>
#include <list.h>
#include <node.h>

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
	 Expression *expr;
	 char *stringVal;
	 int intVal;
	 double floatVal;
}

/*
 * Declare tokens for name and literal values
 * Declare tokens for user variables
 */
%token <*expr> expression
%token <*stringVal> aliasName
%token <*stringVal> tableName
%token <intVal> intConst
%token <floatVal> floatConst
%token <*stringVal> stringConst
%token <*node> indentifier
%token <*node> argument

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
%token COUNT
%token AVG
%token MIN
%token MAX

%type <*queryBlockModel> mainSelectQuery			
		// Its a query block model that defines the structure of query.
%type <*node> selectClause fromClause whereClause
%type <*list> exprList argList
%type <*node> selectItem whereExpression

%start mainSelectQuery

%%
/*
 * Rule to parse select query
 * Currently it will parse following type of select query:
 * 			'SELECT [DISTINCT clause] selectClause FROM fromClause WHERE whereClause;'
 * It will also parse queries asking provenance, like:
 * PROVENANCE OF (query);
 */
mainSelectQuery: optionalProvKeyWord SELECT optionalDistinctKeyWord selectClause \
			optionlFrom optionalWhere optinalClosingBracket ';'
			{$$ = callToFunctionThatDefineQueryBlockModel}
			;

/*
 * GR 2: Parse keyword 'Provenance of (' which is optional.
 */
optionalProvKeyWord: /* empty */
					| PROVENANCE OF '('
					;

/*
 * GR 3: For parsing optional distinct clause.
 */ 
optionalDistinctKeyWord: /* empty */
						| DISTINCT
						| DISTINCT ON '(' exprList ')'
						;
						
/*
 * GR 4: Parse the expression list of distinct clause.
 */
exprList: expression
		 | exprList ',' expression /* Alias used for the attribute/expression 
									* selected Leaf data type.
									*/
		 ;

/*
 * GR 5: parse the select clause items.
 */
selectClause: selectItem optionalAs {$$ = createProjectionNode($1, $2)}
			 | selectClause ',' selectItem optionalAs
			 {$$ = createProjectionNode($1, $3, $4)}
			 ;
			 
/*
 * GR 6: Parse the alias given for the selected attribute or expression.
 */
optionalAs: /* empty */
			| AS aliasName /* Alias used for the attribute/expression 
			                * selected Leaf data type.
							*/
			;

/*
 * GR 7: Parse expressions/attributes/functions used in the select clause.
 */
selectItem: constant
			| attributeRef 
			| operatorExpression
			| sqlFunctionCall
			| STARALL		/* this token is for '*' */
			;
			
/*
 * GR 8: constant parsing
 */
constant: intConst
		  | floatConst
		  | stringConst
		  ;
			
/*
 * GR 9: parse attribute reference
 */
attributeRef: identifier {$$ = $1};

/*
 * GR 10: parse operator expression
 */
operatorExpression: selectItem operator selectItem
					{$$ = createExpression($1,$2,$3)}
					;

operator: '+' | '-' | '*' | '/' | '%' | '||' | '&&' | '!' | '^'
		{$$ = $1}
		;
		
/*
 * GR 11: Function call parser
 */
sqlFunctionCall: sqlFunctionName '(' argList ')' 
				{$$ = functionCall($1, $3)} /* Call a function that will create
											 * a function node.
											 */
				;

/* 
 * This grammar rule is a sample grammar rule to identify functions.
 * Later on other functions of Oracle will be added. 
 */
sqlFunctionName: COUNT
				| AVG
				| MAX
				| MIN
				;
				
argList: /* empty */
		| argument
		| argList ',' argument
		| STARALL
		;

/*
 * GR 12: Parser to parse from clause
 *			Currently implemented for basic from clause.
 *			Later on other forms of from clause will be added.
 */
optionlFrom: /* empty */
			| FROM fromClause
			{$$ = createFromCluaseNode($1, $2)} /* Call a function that will
												 * create a from clause node.
												 */
			;
			
fromClause: tableName optionalAlias {$$ = crateTableNode($1, $2)}

optionalAlias: aliasName
			  | AS aliasName
			  ;
		  
/*
 * GR 13: Rule to parse the where clause.
 * 			Currently only simple where clause is considered.
 *				e.g.   table1.col1 == table2.col1
 *			Later on, other arguments will be considered.
 */
optionalWhere: /* empty */ {$$ = NULL}
			| WHERE whereClause
			{$$ = createWhereNode($1, $2)} /* Call a function that will create
											* where clause node.
											*/
			;
			
whereClause: whereExpression comparatorOperator whereExpression
			{$$ = compareExpressionFunction($1, $2, $3)}
			/* Call a function that will embed comparator in where clause
			 * in a structure node.
			 */

whereExpression: constant
				| attributeRef
				{$$ = $1}
				;
				
comparatorOperator: '==' | '<' | '>' | '<=' | '>=' | '!='
				  {$$ = $1}
				  ;
%%
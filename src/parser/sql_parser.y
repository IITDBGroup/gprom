/*
 * Sql_Parser.y
 *     This is a bison file which contains grammar rules to parse SQLs
 */

%{
#include <stdio.h>
#include "common.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "parser/parse_internal.h"
#include "log/logger.h"

#define RULELOG(grule) \
    { \
        TRACE_LOG("Parsing grammer rule <%s>", #grule); \
    }

Node *bisonParseResult = NULL;
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
%token <intVal> intConst
%token <floatVal> floatConst
%token <stringVal> stringConst
%token <stringVal> identifier
%token <stringVal> comparisonop '+' '-' '*' '/' '%' '^' '&' '|' '!'

/*
 * Tokens for in-built keywords
 *        Currently keywords related to basic query are considered.
 *        Later on other keywords will be added.
 */
%token <stringVal> SELECT
%token <stringVal> PROVENANCE OF
%token <stringVal> FROM
%token <stringVal> AS
%token <stringVal> WHERE
%token <stringVal> DISTINCT
%token <stringVal> ON
%token <stringVal> STARALL
%token <stringVal> UPDATE DELETE
%token <stringVal> AND OR LIKE NOT IN ISNULL BETWEEN
%token <stringVal> AMMSC NULLVAL ALL ANY BY IS

/*
 * Declare token for operators specify their associativity and precedence
 */

/* Logical operators */
%left '|'
%left XOR
%left '&'
%right ':='
%left '!'

/* Comparison operator */
%left comparisonop
%nonassoc AND OR NOT IN ISNULL BETWEEN LIKE

/* Arithmetic operators : FOR TESTING */
%left '+' '-'
%left '*' '/' '%'
%left '^'


/*
 * Types of non-terminal symbols
 */
%type <node> stmt provStmt dmlStmt
%type <node> selectQuery deleteQuery updateQuery
        // Its a query block model that defines the structure of query.
%type <list> selectClause optionalFrom fromClause exprList // select and from clauses are lists
%type <node> selectItem fromClauseItem optionalDistinct optionalWhere
%type <node> expression constant attributeRef sqlFunctionCall
%type <node> binaryOperatorExpression
/*
%type <stringVal> operator arithmaticOperator comparisonOperator sqlOperators logicalOperator
*/


%type <stringVal> optionalAlias

%start stmt

%%

/* Rule for all types of statements */
stmt: 
        dmlStmt ';'    // DML statement can be select, update, insert, delete
        {
            RULELOG(stmt);
            $$ = $1;
            bisonParseResult = (Node *) $$;
        }
/*	queryStms ';' */
    ;

/*
queryStmt:
	selectQuery
	| provStmt
	| setOperator

setOperator:
	queryStmt setOP queryStmt { createSetOp() }

joinExpr:
	fromItem joinOp fromItem optionalJoinCond

joinOP:
	optionalNatural joinType 

optionalJoinCond:
		{}
	ON ( expression ) {} 
	USING ( exprList ) {} 
*/

/* 
 * Rule to parse a query asking for provenance
 */
provStmt: 
        PROVENANCE OF '(' dmlStmt ')'
        {
            RULELOG(provStmt);
            $$ = (Node *) createProvenanceStmt($4);
        }
    ;


/*
 * Rule to parse all DML queries.
 */
dmlStmt:
        selectQuery
        | deleteQuery
        | updateQuery
        | provStmt
        {
            RULELOG(dmlStmt);
        }
    ;
    
/*
 * Rule to parse delete query
 */ 
deleteQuery: 
         DELETE         { RULELOG(deleteQuery); $$ = NULL; }
    ;
         
 /*
  * Rules to parse update query
  */
updateQuery:
        UPDATE        { RULELOG(updateQuery); $$ = NULL; }
    ;
 
/*
 * Rule to parse select query
 * Currently it will parse following type of select query:
 *             'SELECT [DISTINCT clause] selectClause FROM fromClause WHERE whereClause'
 */
selectQuery: 
        SELECT optionalDistinct selectClause optionalFrom optionalWhere
            {
                RULELOG(selectQuery);
                QueryBlock *q =  createQueryBlock();
                
                q->distinct = $2;
                q->selectClause = $3;
                q->fromClause = $4;
                q->whereClause = $5;
                
                $$ = (Node *) q; 
            }
    ;


/*
 * Rule to parse optional distinct clause.
 */ 
optionalDistinct: 
        /* empty */                     { RULELOG("optionalDistinct::NULL"); $$ = NULL; }
        | DISTINCT
            {
                RULELOG("optionalDistinct::DISTINCT");
                $$ = (Node *) createDistinctClause(NULL);
            }
        | DISTINCT ON '(' exprList ')'
            {
                RULELOG("optionalDistinct::DISTINCT::exprList");
                $$ = (Node *) createDistinctClause($4);
            }
    ;
                        

/*
 * Rule to parse the select clause items.
 */
selectClause: 
        selectItem
             {
                RULELOG("selectClause::selectItem"); $$ = singleton($1);
            }
        | selectClause ',' selectItem
            {
                RULELOG("selectClause::selectClause::selectItem");
                $$ = appendToTailOfList($1, $3); 
            }
    ;

selectItem:
         expression                            
             {
                 RULELOG("selectItem::expression"); 
                 $$ = (Node *) createSelectItem(NULL, $1); 
             }
         | expression AS identifier             
             {
                 RULELOG("selectItem::expression::identifier"); 
                 $$ = (Node *) createSelectItem($3, $1);
             }
    ; 

/*
 * Rule to parse an expression list
 */
exprList: 
        expression        { RULELOG("exprList"); $$ = singleton($1); }
        | exprList ',' expression
             {
                  RULELOG("exprList");
                  $$ = appendToTailOfList($1, $3);
             }
    ;
         

/*
 * Rule to parse expressions used in various lists
 */
expression: 
        constant        { RULELOG("expression::constant"); }
        | attributeRef         { RULELOG("expression::attributeRef"); }
        | binaryOperatorExpression        { RULELOG("expression::binaryOperatorExpression"); } 
        | sqlFunctionCall        { RULELOG("expression::sqlFunctionCall"); }
/*        | STARALL        { RULELOG("expression::STARALL"); } */
    ;
            
/*
 * Constant parsing
 */
constant: 
        intConst            { RULELOG("constant::INT"); $$ = (Node *) createConstInt($1); }
        | floatConst        { RULELOG("constant::FLOAT"); $$ = (Node *) createConstFloat($1); }
        | stringConst        { RULELOG("constant::STRING"); $$ = (Node *) createConstString($1); }
    ;
            
/*
 * Parse attribute reference
 */
attributeRef: 
        identifier         { RULELOG("attributeRef::IDENTIFIER"); $$ = (Node *) createAttributeReference($1); }
    ;

/*
 * Parse operator expression
 */
 
binaryOperatorExpression: 
        expression '+' expression
        {
             RULELOG("operatorExpression");
             List *expr = singleton($1);
             expr = appendToTailOfList(expr, $3);
             $$ = createOpExpr($2, expr);
        }
        | expression '-' expression
        | expression '*' expression
        | expression '/' expression
        | expression '%' expression
        | expression '^' expression
        | expression '&' expression
        | expression '|' expression
        | expression comparisonop expression
        {
             RULELOG("operatorExpression");
             List *expr = singleton($1);
             expr = appendToTailOfList(expr, $3);
             $$ = createOpExpr($2, expr);
        }
    ;

/*
operator: 
        arithmaticOperator
        | logicalOperator 
        | comparisonOperator
        | sqlOperators
    ;
    
arithmaticOperator:
        arithmeticop
    ;

logicalOperator:
        '&' | '|' | '!'
    ;

    
comparisonOperator:
        comparisonop
    ;
    
sqlOperators:
        AND | OR | NOT | IN | ISNULL | BETWEEN | LIKE
    ;
*/
    
/*
 * Rule to parse function calls
 */
sqlFunctionCall: 
        identifier '(' exprList ')'          
            {
                RULELOG("sqlFunctionCall::IDENTIFIER::exprList"); 
                $$ = (Node *) createFunctionCall($1, $3); 
            }
    ;

/*
 * Rule to parse from clause
 *            Currently implemented for basic from clause.
 *            Later on other forms of from clause will be added.
 */
optionalFrom: 
        /* empty */              { RULELOG("optionalFrom::NULL"); $$ = NULL; }
        | FROM fromClause        { RULELOG("optionalFrom::fromClause"); $$ = $2; }
    ;
            
fromClause: 
        fromClauseItem
            {
                RULELOG("fromClause::fromClauseItem");
                $$ = singleton($1);
            }
        | fromClause ',' fromClauseItem
            {
                RULELOG("fromClause::fromClause::fromClauseItem");
                $$ = appendToTailOfList($1, $3);
            }
    ;
    
fromClauseItem:
        identifier optionalAlias
            {
                RULELOG("fromClauseItem");
                $$ = (Node *) createFromTableRef($2, NIL, $1);
            }
    ;
    
optionalAlias:
        /* empty */                { RULELOG("optionalAlias::NULL"); $$ = NULL; }
        | identifier            { RULELOG("optionalAlias::identifier"); $$ = $1; }
        | AS identifier            { RULELOG("optionalAlias::identifier"); $$ = $2; }
    ;
          
/*
 * Rule to parse the where clause.
 */
optionalWhere: 
        /* empty */             { RULELOG("optionalWhere::NULL"); $$ = NULL; }
        | WHERE expression        { RULELOG("optionalWhere::WHERE::expression"); $$ = $2; }
    ;

%%

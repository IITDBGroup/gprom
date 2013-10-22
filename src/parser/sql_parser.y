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
%token <stringVal> '+' '-' '*' '/' '%' '^' '&' '|' '!' comparisonOps ')' '('

/*
 * Tokens for in-built keywords
 *        Currently keywords related to basic query are considered.
 *        Later on other keywords will be added.
 */
%token <stringVal> SELECT INSERT UPDATE DELETE
%token <stringVal> PROVENANCE OF
%token <stringVal> FROM
%token <stringVal> AS
%token <stringVal> WHERE
%token <stringVal> DISTINCT
%token <stringVal> STARALL
%token <stringVal> AND OR LIKE NOT IN ISNULL BETWEEN EXCEPT EXISTS
%token <stringVal> AMMSC NULLVAL ALL ANY BY IS SOME
%token <stringVal> UNION INTERSECT MINUS
%token <stringVal> INTO VALUES

/* Keywords for Join queries */
%token <stringVal> JOIN NATURAL LEFT RIGHT OUTER INNER CROSS ON USING FULL 

/*
 * Declare token for operators specify their associativity and precedence
 */
%left UNION INTERSECT MINUS

/* Logical operators */
%left '|'
%left XOR
%left '&'
/* what is that? %right ':=' */
%left '!'

/* Comparison operator */
%left comparisonOps
%nonassoc AND OR NOT IN ISNULL BETWEEN LIKE

/* Arithmetic operators : FOR TESTING */
%left '+' '-'
%left '*' '/' '%'
%left '^'
%nonassoc '(' ')'

/*
 * Types of non-terminal symbols
 */
%type <node> stmt provStmt dmlStmt queryStmt
%type <node> selectQuery deleteQuery updateQuery insertQuery subQuery setOperatorQuery
        // Its a query block model that defines the structure of query.
%type <list> selectClause optionalFrom fromClause exprList // select and from clauses are lists
%type <node> selectItem fromClauseItem optionalDistinct optionalWhere
%type <node> expression constant attributeRef sqlFunctionCall whereExpression
%type <node> binaryOperatorExpression unaryOperatorExpression
/*%type <node> optionalJoinClause optionalJoinCond*/
%type <stringVal> optionalAlias optionalAll nestedSubQueryOperator optionalNot 

%start stmt

%%

/* Rule for all types of statements */
stmt: 
        dmlStmt ';'    // DML statement can be select, update, insert, delete
        {
            RULELOG("stmt::dmlStmt");
            $$ = $1;
            bisonParseResult = (Node *) $$;
        }
	| queryStmt ';'
        {
            RULELOG("stmt::queryStmt");
            $$ = $1;
            bisonParseResult = (Node *) $$;
        }
    ;

/*
 * Rule to parse all DML queries.
 */
dmlStmt:
        insertQuery        { RULELOG("dmlStmt::insertQuery"); }
        | deleteQuery        { RULELOG("dmlStmt::deleteQuery"); }
        | updateQuery        { RULELOG("dmlStmt::updateQuery"); }
    ;

/*
 * Rule to parse all types projection queries.
 */
queryStmt:
	selectQuery        { RULELOG("queryStmt::selectQuery"); }
	| provStmt        { RULELOG("queryStmt::provStmt"); }
	| setOperatorQuery        { RULELOG("queryStmt::setOperatorQuery"); }
    ;

/* 
 * Rule to parse a query asking for provenance
 */
provStmt: 
        PROVENANCE OF '(' stmt ')'
        {
            RULELOG(provStmt);
            $$ = (Node *) createProvenanceStmt($4);
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
 * Rules to parse insert query
 */
insertQuery:
        INSERT 		  { RULELOG(insertQuery); $$ = NULL; } 
    ;

/*
 * Rules to parse set operator queries
 */

setOperatorQuery:     // Need to look into createFunction
        queryStmt INTERSECT queryStmt
            {
                RULELOG("setOperatorQuery::INTERSECT");
                $$ = (Node *) createSetQuery($2, FALSE, $1, $3);
            }
        | queryStmt MINUS queryStmt 
            {
                RULELOG("setOperatorQuery::MINUS");
                $$ = (Node *) createSetQuery($2, FALSE, $1, $3);
            }
        | queryStmt UNION optionalAll queryStmt
            {
                RULELOG("setOperatorQuery::UNION");
                $$ = (Node *) createSetQuery($2, ($3 != NULL), $1, $4);
            }
    ;

optionalAll:
        /* Empty */ { RULELOG("optionalAll::NULL"); $$ = NULL; }
        | ALL        { RULELOG("optionalAll::ALLTRUE"); $$ = $1; }
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
        expression        { RULELOG("exprList::SINGLETON"); $$ = singleton($1); }
        | exprList ',' expression
             {
                  RULELOG("exprList::exprList::expression");
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
        | unaryOperatorExpression        { RULELOG("expression::unaryOperatorExpression"); }
        | sqlFunctionCall        { RULELOG("expression::sqlFunctionCall"); }
/*        | '(' queryStmt ')'       { RULELOG ("expression::subQuery"); $$ = $2; } */
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
/* HELP HELP ??
       Need helper function support for attribute list in expression.
       For e.g.
           SELECT attr FROM tab
           WHERE
              (col1, col2) = (SELECT cl1, cl2 FROM tab2)
       SolQ: Can we use selectItem function here?????
*/
    ;

/*
 * Parse operator expression
 */
 
binaryOperatorExpression: 

    /* Arithmatic Operations */
        expression '+' expression
            {
                RULELOG("binaryOperatorExpression:: '+' ");
                List *expr = singleton($1);
                expr = appendToTailOfList(expr, $3);
                $$ = (Node *) createOpExpr($2, expr);
            }
        | expression '-' expression
            {
                RULELOG("binaryOperatorExpression:: '-' ");
                List *expr = singleton($1);
                expr = appendToTailOfList(expr, $3);
                $$ = (Node *) createOpExpr($2, expr);
            }
        | expression '*' expression
            {
                RULELOG("binaryOperatorExpression:: '*' ");
                List *expr = singleton($1);
                expr = appendToTailOfList(expr, $3);
                $$ = (Node *) createOpExpr($2, expr);
            }
        | expression '/' expression
            {
                RULELOG("binaryOperatorExpression:: '/' ");
                List *expr = singleton($1);
                expr = appendToTailOfList(expr, $3);
                $$ = (Node *) createOpExpr($2, expr);
            }
        | expression '%' expression
            {
                RULELOG("binaryOperatorExpression:: '%' ");
                List *expr = singleton($1);
                expr = appendToTailOfList(expr, $3);
                $$ = (Node *) createOpExpr($2, expr);
            }
        | expression '^' expression
            {
                RULELOG("binaryOperatorExpression:: '^' ");
                List *expr = singleton($1);
                expr = appendToTailOfList(expr, $3);
                $$ = (Node *) createOpExpr($2, expr);
            }

    /* Binary operators */
        | expression '&' expression
            {
                RULELOG("binaryOperatorExpression:: '&' ");
                List *expr = singleton($1);
                expr = appendToTailOfList(expr, $3);
                $$ = (Node *) createOpExpr($2, expr);
            }
        | expression '|' expression
            {
                RULELOG("binaryOperatorExpression:: '|' ");
                List *expr = singleton($1);
                expr = appendToTailOfList(expr, $3);
                $$ = (Node *) createOpExpr($2, expr);
            }

    /* Comparison Operators */
        | expression comparisonOps expression
            {
                RULELOG("binaryOperatorExpression::comparisonOps");
                List *expr = singleton($1);
                expr = appendToTailOfList(expr, $3);
                $$ = (Node *) createOpExpr($2, expr);
            }
    ;

unaryOperatorExpression:
        '!' expression
            {
                RULELOG("unaryOperatorExpression:: '!' ");
                List *expr = singleton($2);
                $$ = (Node *) createOpExpr($1, expr);
            }
    ;
    
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

subQuery:
        '(' queryStmt ')' optionalAlias
            {
                RULELOG("subQuery::queryStmt");
                $$ = (Node *) createFromSubquery($4, NULL, $2);
            }
    ;

fromClauseItem:
        identifier optionalAlias
            {
                RULELOG("fromClauseItem");
                $$ = (Node *) createFromTableRef($2, NIL, $1);
            }
        | subQuery
            {
                RULELOG("fromClauseItem::subQuery");
                $$ = $1;
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
        | WHERE whereExpression        { RULELOG("optionalWhere::whereExpression"); $$ = $2; }
    ;

whereExpression:
        expression        { RULELOG("whereExpression::expression"); $$ = $1; }
        | whereExpression AND whereExpression
            {
                RULELOG("whereExpression::AND");
                List *expr = singleton($1);
                expr = appendToTailOfList(expr, $3);
                $$ = (Node *) createOpExpr($2, expr);
            }
        | whereExpression OR whereExpression
            {
                RULELOG("whereExpression::AND");
                List *expr = singleton($1);
                expr = appendToTailOfList(expr, $3);
                $$ = (Node *) createOpExpr($2, expr);
            }
        | whereExpression LIKE whereExpression
            {
                RULELOG("whereExpression::AND");
                List *expr = singleton($1);
                expr = appendToTailOfList(expr, $3);
                $$ = (Node *) createOpExpr($2, expr);
            }
        | whereExpression BETWEEN whereExpression AND whereExpression
            {
                RULELOG("whereExpression::BETWEEN-AND");
                List *expr = singleton($1);
                expr = appendToTailOfList(expr, $3);
                expr = appendToTailOfList(expr, $5);
                $$ = (Node *) createOpExpr($2, expr);
            } 
        | expression comparisonOps nestedSubQueryOperator '(' queryStmt ')'
            {
                RULELOG("whereExpression::comparisonOps::nestedSubQueryOperator::Subquery");
                $$ = (Node *) createNestedSubquery($3, $1, $2, $5);
            }
        | expression comparisonOps '(' queryStmt ')'
            {
                RULELOG("whereExpression::comparisonOps::Subquery");
                List *expr = singleton($1);
                expr = appendToTailOfList(expr, $4);
                $$ = (Node *) createOpExpr($2, expr);
            }
        | expression optionalNot IN '(' queryStmt ')'
            {
                if ($2 == NULL)
                {
                    RULELOG("whereExpression::IN");
                    $$ = (Node *) createNestedSubquery("ANY", $1, "=", $5);
                }
                else
                {
                    RULELOG("whereExpression::NOT::IN");
                    $$ = (Node *) createNestedSubquery("ALL",$1, "<>", $5);
                }
            }
        | optionalNot EXISTS '(' queryStmt ')'
            {
                if ($1 == NULL)
                {
                    RULELOG("whereExpression::EXISTS");
                    $$ = (Node *) createNestedSubquery($2, NULL, $1, $4);
                }
                else
                {
                    RULELOG("whereExpression::EXISTS::NOT");
                    $$ = (Node *) createNestedSubquery($2, NULL, "<>", $4);
                }
                /* How should I call function for this? No provision for NOT */
            }
    ;

nestedSubQueryOperator:
        ANY { RULELOG("nestedSubQueryOperator::ANY"); $$ = $1; }
        | ALL { RULELOG("nestedSubQueryOperator::ALL"); $$ = $1; }
        | SOME { RULELOG("nestedSubQueryOperator::SOME"); $$ = "ANY"; }
    ;

optionalNot:
        /* Empty */    { RULELOG("optionalNot::NULL"); $$ = NULL; }
        | NOT    { RULELOG("optionalNot::NOT"); $$ = $1; }
    ;

%%

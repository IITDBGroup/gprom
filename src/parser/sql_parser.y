/*
 * Sql_Parser.y
 *     This is a bison file which contains grammar rules to parse SQLs
 */



%{
#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "parser/parse_internal.h"
#include "log/logger.h"
#include "model/query_operator/operator_property.h"

#define RULELOG(grule) \
    { \
        TRACE_LOG("Parsing grammer rule <%s> at line %d", #grule, yylineno); \
    }
    
#undef free
#undef malloc

Node *bisonParseResult = NULL;
%}

%error-verbose

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
%token <stringVal> identifier compositeIdentifier
%token <stringVal> parameter
%token <stringVal> '+' '-' '*' '/' '%' '^' '&' '|' '!' comparisonOps ')' '(' '='

/*
 * Tokens for in-built keywords
 *        Currently keywords related to basic query are considered.
 *        Later on other keywords will be added.
 */
%token <stringVal> SELECT INSERT UPDATE DELETE
%token <stringVal> PROVENANCE OF BASERELATION SCN TIMESTAMP HAS TABLE ONLY UPDATED SHOW INTERMEDIATE USE TUPLE VERSIONS STATEMENT ANNOTATIONS NO
%token <stringVal> FROM
%token <stringVal> AS
%token <stringVal> WHERE
%token <stringVal> DISTINCT
%token <stringVal> STARALL
%token <stringVal> AND OR LIKE NOT IN ISNULL BETWEEN EXCEPT EXISTS
%token <stringVal> AMMSC NULLVAL ROWNUM ALL ANY IS SOME
%token <stringVal> UNION INTERSECT MINUS
%token <stringVal> INTO VALUES HAVING GROUP ORDER BY LIMIT SET
%token <stringVal> INT BEGIN_TRANS COMMIT_TRANS ROLLBACK_TRANS
%token <stringVal> CASE WHEN THEN ELSE END
%token <stringVal> OVER_TOK PARTITION ROWS RANGE UNBOUNDED PRECEDING CURRENT ROW FOLLOWING
%token <stringVal> NULLS FIRST LAST ASC DESC
%token <stringVal> JSON_TABLE COLUMNS PATH FORMAT WRAPPER NESTED WITHOUT CONDITIONAL

%token <stringVal> DUMMYEXPR

/* Keywords for Join queries */
%token <stringVal> JOIN NATURAL LEFT RIGHT OUTER INNER CROSS ON USING FULL TYPE TRANSACTION WITH 

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
%right NOT
%left AND OR
%right ISNULL
%nonassoc  LIKE IN  BETWEEN

/* Arithmetic operators : FOR TESTING */
%nonassoc DUMMYEXPR
%left '+' '-'
%left '*' '/' '%'
%left '^'
%nonassoc '(' ')'

%left NATURAL JOIN CROSS LEFT FULL RIGHT INNER

/*
 * Types of non-terminal symbols
 */
%type <node> stmt provStmt dmlStmt queryStmt
%type <node> selectQuery deleteQuery updateQuery insertQuery subQuery setOperatorQuery
        // Its a query block model that defines the structure of query.
%type <list> selectClause optionalFrom fromClause exprList orderList 
			 optionalGroupBy optionalOrderBy setClause  stmtList //insertList 
			 identifierList optionalAttrAlias optionalProvWith provOptionList 
			 caseWhenList windowBoundaries optWindowPart withViewList jsonColInfo
//			 optInsertAttrList
%type <node> selectItem fromClauseItem fromJoinItem optionalFromProv optionalAlias optionalDistinct optionalWhere optionalLimit optionalHaving orderExpr insertContent
             //optionalReruning optionalGroupBy optionalOrderBy optionalLimit 
%type <node> expression constant attributeRef sqlParameter sqlFunctionCall whereExpression setExpression caseExpression caseWhen optionalCaseElse
%type <node> overClause windowSpec optWindowFrame windowBound
%type <node> jsonTable jsonColInfoItem 
%type <node> binaryOperatorExpression unaryOperatorExpression
%type <node> joinCond
%type <node> optionalProvAsOf provOption
%type <node> withView withQuery
%type <stringVal> optionalAll nestedSubQueryOperator optionalNot fromString optionalSortOrder optionalNullOrder
%type <stringVal> joinType transactionIdentifier delimIdentifier
%type <stringVal> optionalFormat optionalWrapper

%start stmtList

%%

/* Rule for all types of statements */
stmtList: 
		stmt ';'
			{ 
				RULELOG("stmtList::stmt"); 
				$$ = singleton($1);
				bisonParseResult = (Node *) $$;	 
			}
		| stmtList stmt ';' 
			{
				RULELOG("stmtlist::stmtList::stmt");
				$$ = appendToTailOfList($1, $2);	
				bisonParseResult = (Node *) $$; 
			}
	;

stmt: 
        dmlStmt    // DML statement can be select, update, insert, delete
        {
            RULELOG("stmt::dmlStmt");
            $$ = $1;
        }
		| queryStmt
        {
            RULELOG("stmt::queryStmt");
            $$ = $1;
        }
        | transactionIdentifier
        {
            RULELOG("stmt::transactionIdentifier");
            $$ = (Node *) createTransactionStmt($1);
        }
		| withQuery
		{ 
			RULELOG("stmt::withQuery"); 
			$$ = $1; 
		}
		| expression
		{
			RULELOG("stmt::expression");
			$$ = $1;
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
		'(' queryStmt ')'	{ RULELOG("queryStmt::bracketedQuery"); $$ = $2; }
		| selectQuery        { RULELOG("queryStmt::selectQuery"); }
		| provStmt        { RULELOG("queryStmt::provStmt"); }
		| setOperatorQuery        { RULELOG("queryStmt::setOperatorQuery"); }
    ;

withQuery:
		WITH withViewList queryStmt
		{
			RULELOG("withQuery::withViewList::queryStmt");
			$$ = (Node *) createWithStmt($2, $3);
		}
	;	
	
withViewList:
		withViewList ',' withView
		{
			RULELOG("withViewList::list::view");
			$$ = appendToTailOfList($1, $3);
		}
		| withView
		{
			RULELOG("withViewList::view");
			$$ = singleton($1);
		}
	;

withView:
		identifier AS '(' queryStmt ')'
		{
			RULELOG("withView::ident::AS:queryStmt");
			$$ = (Node *) createNodeKeyValue((Node *) createConstString($1), $4);
		}
	;

transactionIdentifier:
        BEGIN_TRANS        { RULELOG("transactionIdentifier::BEGIN"); $$ = strdup("TRANSACTION_BEGIN"); }
        | COMMIT_TRANS        { RULELOG("transactionIdentifier::COMMIT"); $$ = strdup("TRANSACTION_COMMIT"); }
        | ROLLBACK_TRANS        { RULELOG("transactionIdentifier::ROLLBACK"); $$ = strdup("TRANSACTION_ABORT"); }
    ;

/* 
 * Rule to parse a query asking for provenance
 */
provStmt: 
        PROVENANCE optionalProvAsOf optionalProvWith OF '(' stmt ')'
        {
            RULELOG("provStmt::stmt");
            Node *stmt = $6;
	    	ProvenanceStmt *p = createProvenanceStmt(stmt);
		    p->inputType = isQBUpdate(stmt) ? PROV_INPUT_UPDATE : PROV_INPUT_QUERY;
		    p->provType = PROV_PI_CS;
		    p->asOf = (Node *) $2;
		    p->options = $3;
            $$ = (Node *) p;
        }
		| PROVENANCE optionalProvAsOf optionalProvWith OF '(' stmtList ')'
		{
			RULELOG("provStmt::stmtlist");
			ProvenanceStmt *p = createProvenanceStmt((Node *) $6);
			p->inputType = PROV_INPUT_UPDATE_SEQUENCE;
			p->provType = PROV_PI_CS;
			p->asOf = (Node *) $2;
			p->options = $3;
			$$ = (Node *) p;
		}
		| PROVENANCE optionalProvAsOf optionalProvWith OF TRANSACTION stringConst
		{
			RULELOG("provStmt::transaction");
			ProvenanceStmt *p = createProvenanceStmt((Node *) createConstString($6));
			p->inputType = PROV_INPUT_TRANSACTION;
			p->provType = PROV_PI_CS;
			p->options = $3;
			$$ = (Node *) p;
		}
    ;
    
optionalProvAsOf:
		/* empty */			{ RULELOG("optionalProvAsOf::EMPTY"); $$ = NULL; }
		| AS OF SCN intConst
		{
			RULELOG("optionalProvAsOf::SCN");
			$$ = (Node *) createConstLong($4);
		}
		| AS OF TIMESTAMP stringConst
		{
			RULELOG("optionalProvAsOf::TIMESTAMP");
			$$ = (Node *) createConstString($4);
		}
	;
	
optionalProvWith:
		/* empty */			{ RULELOG("optionalProvWith::EMPTY"); $$ = NIL; }
		| WITH provOptionList
		{
			RULELOG("optionalProvWith::WITH");
			$$ = $2;
		}
	;
	
provOptionList:
		provOption	{ RULELOG("provOptionList::option"); $$ = singleton($1); }
		| provOptionList provOption 
		{ 
			RULELOG("provOptionList::list"); 
			$$ = appendToTailOfList($1,$2); 
		}
	;
	
provOption:
		TYPE stringConst 
		{ 
			RULELOG("provOption::TYPE"); 
			$$ = (Node *) createStringKeyValue("TYPE", $2); 
		}
		| TABLE identifier
		{
			RULELOG("provOption::TABLE");
			$$ = (Node *) createStringKeyValue("TABLE", $2);
		}
		| ONLY UPDATED
		{
			RULELOG("provOption::ONLY::UPDATED");
			$$ = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_ONLY_UPDATED), 
					(Node *) createConstBool(TRUE));
		}
		| SHOW INTERMEDIATE
		{
			RULELOG("provOption::SHOW::INTERMEDIATE");
			$$ = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_SHOW_INTERMEDIATE), 
					(Node *) createConstBool(TRUE));
		}
		| TUPLE VERSIONS
		{
			RULELOG("provOption::TUPLE::VERSIONS");
			$$ = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_TUPLE_VERSIONS),
					(Node *) createConstBool(TRUE));
		}
		| STATEMENT ANNOTATIONS
		{
			RULELOG("provOption::STATEMENT::ANNOTATIONS");
			$$ = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_STATEMENT_ANNOTATIONS),
					(Node *) createConstBool(TRUE));
		}
		| NO STATEMENT ANNOTATIONS
		{
			RULELOG("provOption::NO::STATEMENT::ANNOTATIONS");
			$$ = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_STATEMENT_ANNOTATIONS),
					(Node *) createConstBool(FALSE));
		}
	;
	
/*
 * Rule to parse delete query
 */ 
deleteQuery: 
         DELETE fromString identifier WHERE whereExpression /* optionalReturning */
         { 
             RULELOG("deleteQuery");
             $$ = (Node *) createDelete($3, $5);
         }
/* No provision made for RETURNING statements in delete clause */
    ;

fromString:
        /* Empty */        { RULELOG("fromString::NULL"); $$ = NULL; }
        | FROM        { RULELOG("fromString::FROM"); $$ = $1; }
    ;

         
//optionalReturning:
/*         Empty         { RULELOG("optionalReturning::NULL"); $$ = NULL; }
        | RETURNING expression INTO identifier
            { RULELOG("optionalReturning::RETURNING"); }
    ; */

/*
 * Rules to parse update query
 */
updateQuery:
        UPDATE identifier SET setClause optionalWhere
            { 
                RULELOG(updateQuery); 
                $$ = (Node *) createUpdate($2, $4, $5); 
            }
    ;

setClause:
        setExpression
            {
                RULELOG("setClause::setExpression");
                $$ = singleton($1);
            }
        | setClause ',' setExpression
            {
                RULELOG("setClause::setClause::setExpression");
                $$ = appendToTailOfList($1, $3);
            }
    ;

setExpression:
        attributeRef comparisonOps expression
            {
                if (!strcmp($2,"=")) {
                    RULELOG("setExpression::attributeRef::expression");
                    List *expr = singleton($1);
                    expr = appendToTailOfList(expr, $3);
                    $$ = (Node *) createOpExpr($2, expr);
                }
            }
        | attributeRef comparisonOps subQuery 
            {
                if (!strcmp($2, "=")) {
                    RULELOG("setExpression::attributeRef::queryStmt");
                    List *expr = singleton($1);
                    expr = appendToTailOfList(expr, $3);
                    $$ = (Node *) createOpExpr($2, expr);
                }
            }
    ;

/*
 * Rules to parse insert query
 */
insertQuery:
        INSERT INTO identifier insertContent
        { 
           	RULELOG("insertQuery::insertList"); 
           	$$ = (Node *) createInsert($3,(Node *) $4, NIL); 
        }
        | INSERT INTO identifier '(' identifierList ')' insertContent
        { 
           	RULELOG("insertQuery::insertList"); 
           	$$ = (Node *) createInsert($3,(Node *) $7, $5); 
     	} 
    ;
    
insertContent:
		VALUES '(' exprList ')' { $$ = (Node *) $3; }
		| queryStmt
	;
	

/* No Provision made for this type of insert statements */
/* generalize to expression instead of only constant */
    //;

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
        SELECT optionalDistinct selectClause optionalFrom optionalWhere optionalGroupBy optionalHaving optionalOrderBy optionalLimit
            {
                RULELOG(selectQuery);
                QueryBlock *q =  createQueryBlock();
                
                q->distinct = $2;
                q->selectClause = $3;
                q->fromClause = $4;
                q->whereClause = $5;
                q->groupByClause = $6;
                q->havingClause = $7;
                q->orderByClause = $8;
                q->limitClause = $9;
                
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
         | '*'              
			{ 
         		RULELOG("selectItem::*"); 
         		$$ = (Node *) createSelectItem(strdup("*"), NULL); 
     		}
         | identifier '.' '*' 
         	{ 
         		RULELOG("selectItem::*"); 
     			$$ = (Node *) createSelectItem(CONCAT_STRINGS($1,".*"), NULL); 
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
		'(' expression ')'				{ RULELOG("expression::bracked"); $$ = $2; } 
		| constant     				   	{ RULELOG("expression::constant"); }
        | attributeRef         		  	{ RULELOG("expression::attributeRef"); }
	    | sqlParameter					{ RULELOG("expression::sqlParameter"); }
        | binaryOperatorExpression		{ RULELOG("expression::binaryOperatorExpression"); } 
        | unaryOperatorExpression       { RULELOG("expression::unaryOperatorExpression"); }
        | sqlFunctionCall        		{ RULELOG("expression::sqlFunctionCall"); }
		| caseExpression				{ RULELOG("expression::case"); }
		| ROWNUM						{ RULELOG("expression::ROWNUM"); $$ = (Node *) makeNode(RowNumExpr); }
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
        identifier         
        { 
        	RULELOG("attributeRef::IDENTIFIER"); 
        	$$ = (Node *) createAttributeReference($1); 
        }
        | compositeIdentifier  
        { 
        	RULELOG("attributeRef::COMPOSITEIDENT"); 
        	$$ = (Node *) createAttributeReference($1); 
        }
	;
	
/*
 * SQL parameter
 */
sqlParameter:
		parameter		   { RULELOG("sqlParameter::PARAMETER"); $$ = (Node *) createSQLParameter($1); }
	;
	
/* HELP HELP ??
       Need helper function support for attribute list in expression.
       For e.g.
           SELECT attr FROM tab
           WHERE
              (col1, col2) = (SELECT cl1, cl2 FROM tab2)
       SolQ: Can we use selectItem function here?????
*/

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
        identifier '(' exprList ')' overClause          
            {
                RULELOG("sqlFunctionCall::IDENTIFIER::exprList");
				FunctionCall *f = createFunctionCall($1, $3);
				if ($5 != NULL)
					$$ = (Node *) createWindowFunction(f, (WindowDef *) $5);
				else  
                	$$ = (Node *) f; 
            }
		| AMMSC '(' exprList ')' overClause          
            {
                RULELOG("sqlFunctionCall::AMMSC::exprList");
				FunctionCall *f = createFunctionCall($1, $3);
				if ($5 != NULL)
					$$ = (Node *) createWindowFunction(f, (WindowDef *) $5);
				else  
                	$$ = (Node *) f; 
            }
        | AMMSC '(' '*' ')' overClause
            {
                RULELOG("sqlFunctionCall::COUNT::*");
				FunctionCall *f = createFunctionCall($1, singleton(createConstInt(1)));
				if (strcasecmp($1,"COUNT") != 0)
					yyerror("* can only be used as an input of the COUNT aggregate function, but no any other function.");
				if ($5 != NULL)
					$$ = (Node *) createWindowFunction(f, (WindowDef *) $5);
				else  
                	$$ = (Node *) f; 
            }
    ;

/*
 * Rule to parser CASE expressions
 */
caseExpression:
		CASE expression caseWhenList optionalCaseElse END
			{
				RULELOG("caseExpression::CASE::expression::whens:else:END");
				$$ = (Node *) createCaseExpr($2, $3, $4);
			}
		| CASE caseWhenList optionalCaseElse END
			{
				RULELOG("caseExpression::CASE::whens::else::END");
				$$ = (Node *) createCaseExpr(NULL, $2, $3);
			}
	;

caseWhenList:
		caseWhenList caseWhen
			{
				RULELOG("caseWhenList::list::caseWhen");
				$$ = appendToTailOfList($1, $2);
			}
		| caseWhen
			{
				RULELOG("caseWhenList::caseWhen");
				$$ = singleton($1);
			}
	;
	
caseWhen:
		WHEN expression THEN expression
			{
				RULELOG("caseWhen::WHEN::expression::THEN::expression");
				$$ = (Node *) createCaseWhen($2,$4);
			}
	;

optionalCaseElse:
		/* empty */				{ RULELOG("optionalCaseElse::NULL"); $$ = NULL; }
		| ELSE expression
			{
				RULELOG("optionalCaseElse::ELSE::expression");
				$$ = $2;
			}
	;
    
 /*
  * Rule to parser OVER clause for window functions
  */
overClause:
		/* empty */ 	{ RULELOG("overclause::NULL"); $$ = NULL; }
		| OVER_TOK windowSpec
			{
				RULELOG("overclause::window");
				$$ = $2;
			}
	;
	
windowSpec:
		'('  optWindowPart optionalOrderBy optWindowFrame ')'
			{
				RULELOG("window");
				$$ = (Node *) createWindowDef($2,$3, (WindowFrame *) $4);
			}
	;
	
optWindowPart:
		/* empty */ 			{ RULELOG("optWindowPart::NULL"); $$ = NIL; }
		| PARTITION BY exprList
			{
				RULELOG("optWindowPart::PARTITION:BY::expressionList");
				$$ = $3;
			}
	;

optWindowFrame:
		/* empty */				{ RULELOG("optWindowFrame::NULL"); $$ = NULL; }
		| ROWS windowBoundaries 
			{ 
				WindowBound *l, *u = NULL;
				RULELOG("optWindowFrame::ROWS::windoBoundaries");
				l = getNthOfListP($2, 0);
				if(LIST_LENGTH($2) > 1)
					u = getNthOfListP($2, 1);
				$$ = (Node *) createWindowFrame(WINFRAME_ROWS, l, u); 
			}
		| RANGE windowBoundaries 
			{
				WindowBound *l, *u = NULL; 
				RULELOG("optWindowFrame::RANGE::windoBoundaries"); 
				l = getNthOfListP($2, 0);
				if(LIST_LENGTH($2) > 1)
					u = getNthOfListP($2, 1);
				$$ = (Node *) createWindowFrame(WINFRAME_RANGE, l, u); 
			}
	;
	
windowBoundaries:
		BETWEEN windowBound AND windowBound
			{ 
				RULELOG("windowBoundaries::BETWEEN"); 
				$$ = LIST_MAKE($2, $4); 
			}	
		| windowBound 						
			{ 
				RULELOG("windowBoundaries::windowBound"); 
				$$ = singleton($1); 
			}
	;

windowBound:
		UNBOUNDED PRECEDING 
			{ 
				RULELOG("windowBound::UNBOUNDED::PRECEDING"); 
				$$ = (Node *) createWindowBound(WINBOUND_UNBOUND_PREC, NULL); 
			}
		| CURRENT ROW 
			{ 
				RULELOG("windowBound::CURRENT::ROW"); 
				$$ = (Node *) createWindowBound(WINBOUND_CURRENT_ROW, NULL); 
			}			
		| expression PRECEDING
			{ 
				RULELOG("windowBound::expression::PRECEDING"); 
				$$ = (Node *) createWindowBound(WINBOUND_EXPR_PREC, $1); 
			}
		| expression FOLLOWING
			{ 
				RULELOG("windowBound::expression::FOLLOWING"); 
				$$ = (Node *) createWindowBound(WINBOUND_EXPR_FOLLOW, $1); 
			}
	;

/*
 * Rule to parse JSON Functions
 */
jsonTable:
                /* empty */	{ RULELOG("jsonTable::NULL"); $$ = NULL; }
		| JSON_TABLE '(' attributeRef ',' stringConst COLUMNS '(' jsonColInfo ')' ')' AS identifier
			{
				RULELOG("jsonTable::jsonTable");
                                $$ = (Node *) createFromJsonTable((AttributeReference *) $3, $5, $8, $12);
			}
	;

jsonColInfo:
                jsonColInfoItem
                        {
                                RULELOG("jsonColInfo::jsonColInfoItem");
                                $$ = singleton($1);
                        }
                | jsonColInfo ',' jsonColInfoItem
                        {
                                RULELOG("jsonColInfo::jsonColInfoItem::jsonColInfoItem");
                                $$ = appendToTailOfList($1, $3);
                        }
        ;

jsonColInfoItem:
                /* empty */ { RULELOG("jsonColInfoItem::NULL"); }
                | identifier identifier optionalFormat optionalWrapper PATH stringConst
                        {
                                RULELOG("jsonColInfoItem::jsonColInfoItem");
                                JsonColInfoItem *c = createJsonColInfoItem ($1, $2, $6, $3, $4, NULL, NULL);
                                $$ = (Node *) c;
                        }
                | NESTED PATH stringConst COLUMNS '(' jsonColInfo ')'
                        {
                                RULELOG("jsonColInfoItem::jsonColInfoItem");
                                JsonColInfoItem *c = createJsonColInfoItem (NULL, NULL, $3, NULL, NULL, $6, NULL);
                                $$ = (Node *) c;
                        }
        ;

optionalFormat:
                /* empty */ { RULELOG("optionalFormat::NULL"); $$ = NULL; }
                | FORMAT identifier
                        {
                                RULELOG("optionalFormat::FORMAT");
                                $$ = $2;
                        }
        ;

optionalWrapper:
                /* empty */ { RULELOG("optionalWrapper::NULL"); $$ = NULL; }
                | WITH WRAPPER
                        {
                                RULELOG("optionalWrapper::WITH WRAPPER");
                                $$ = strdup("WITH");
                        }
                | WITHOUT WRAPPER
                        {
                                RULELOG("optionalWrapper::WITHOUT WRAPPER");
                                $$ = strdup("WITHOUT");
                        }
                | WITH CONDITIONAL WRAPPER
                        {
                                RULELOG("optionalWrapper::WITHOUT WRAPPER");
                                $$ = strdup("WITH CONDITIONAL");
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
        identifier optionalFromProv
            {
                RULELOG("fromClauseItem");
				FromItem *f = createFromTableRef(NULL, NIL, $1, NIL);
				f->provInfo = (FromProvInfo *) $2;
                $$ = (Node *) f;
            }
        | identifier optionalAlias
            {
                RULELOG("fromClauseItem");
                FromItem *f = createFromTableRef(((FromItem *) $2)->name, 
						((FromItem *) $2)->attrNames, $1, NIL);
				f->provInfo = ((FromItem *) $2)->provInfo;
                $$ = (Node *) f;
            }
            
        | subQuery optionalFromProv
            {
                RULELOG("fromClauseItem::subQuery");
                FromItem *f = (FromItem *) $1;
                f->provInfo = (FromProvInfo *) $2;
                $$ = $1;
            }
        | subQuery optionalAlias
            {
                RULELOG("fromClauseItem::subQuery");
                FromSubquery *s = (FromSubquery *) $1;
                s->from.name = ((FromItem *) $2)->name;
                s->from.attrNames = ((FromItem *) $2)->attrNames;
                s->from.provInfo = ((FromItem *) $2)->provInfo;
                $$ = (Node *) s;
            }
        | fromJoinItem
        	{
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) $1;
        		f->name = NULL;
        		$$ = (Node *) f;
        	}
       	 | '(' fromJoinItem ')' optionalAlias
        	{
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) $2;
        		f->name = ((FromItem *) $4)->name;
                f->attrNames = ((FromItem *) $4)->attrNames;
                f->provInfo = ((FromItem *) $4)->provInfo;
        		$$ = (Node *) f;
        	}
         | jsonTable
                {
                    RULELOG("fromClauseItem::jsonTable");
                    //FromJsonTable *jt = (FromJsonTable *) $1;
                    FromItem *jt = (FromItem *)$1;
                    $$ = (Node*) jt;
                }
    ;

subQuery:
        '(' queryStmt ')'
            {
                RULELOG("subQuery::queryStmt");
                $$ = (Node *) createFromSubquery(NULL, NULL, $2);
            }
    ;

identifierList:
		delimIdentifier { $$ = singleton($1); }
		| identifierList ',' delimIdentifier { $$ = appendToTailOfList($1, $3); }
	;
	
fromJoinItem:
		'(' fromJoinItem ')' 			{ $$ = $2; }
        | fromClauseItem NATURAL JOIN fromClauseItem 
			{
                RULELOG("fromJoinItem::NATURAL");
                $$ = (Node *) createFromJoin(NULL, NIL, (FromItem *) $1, 
						(FromItem *) $4, "JOIN_INNER", "JOIN_COND_NATURAL", 
						NULL);
          	}
		| fromClauseItem NATURAL joinType JOIN fromClauseItem 
			{
                RULELOG("fromJoinItem::NATURALjoinType");
                $$ = (Node *) createFromJoin(NULL, NIL, (FromItem *) $1, 
                		(FromItem *) $5, $3, "JOIN_COND_NATURAL", NULL);
          	}
     	| fromClauseItem CROSS JOIN fromClauseItem 
        	{
				RULELOG("fromJoinItem::CROSS JOIN");
                $$ = (Node *) createFromJoin(NULL, NIL, (FromItem *) $1, 
                		(FromItem *) $4, "JOIN_CROSS", "JOIN_COND_ON", NULL);
          	}
     	| fromClauseItem joinType JOIN fromClauseItem joinCond 
        	{
				RULELOG("fromJoinItem::JOIN::joinType::joinCond");
				char *condType = (isA($5,List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON";
                $$ = (Node *) createFromJoin(NULL, NIL, (FromItem *) $1, 
                		(FromItem *) $4, $2, condType, $5);
          	}
     	| fromClauseItem JOIN fromClauseItem joinCond
        	{
				RULELOG("fromJoinItem::JOIN::joinCond");
				char *condType = (isA($4,List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON"; 
                $$ = (Node *) createFromJoin(NULL, NIL, (FromItem *) $1, 
                		(FromItem *) $3, "JOIN_INNER", 
                		condType, $4);
          	}
     ;
     
joinType:
		LEFT 			{ RULELOG("joinType::LEFT"); $$ = "JOIN_LEFT_OUTER"; }
		| LEFT OUTER 	{ RULELOG("joinType::LEFT OUTER"); $$ = "JOIN_LEFT_OUTER"; }
		| RIGHT 		{ RULELOG("joinType::RIGHT "); $$ = "JOIN_RIGHT_OUTER"; }
		| RIGHT OUTER  	{ RULELOG("joinType::RIGHT OUTER"); $$ = "JOIN_RIGHT_OUTER"; }
		| FULL OUTER  	{ RULELOG("joinType::FULL OUTER"); $$ = "JOIN_FULL_OUTER"; }
		| FULL 	  		{ RULELOG("joinType::FULL"); $$ = "JOIN_FULL_OUTER"; }
		| INNER  		{ RULELOG("joinType::INNER"); $$ = "JOIN_INNER"; }
	;

joinCond:
		USING '(' identifierList ')' { $$ = (Node *) $3; }
		| ON whereExpression			 { $$ = $2; }
	;

optionalAlias:
        optionalFromProv identifier optionalAttrAlias      
			{
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem($2,$3);
 				f->provInfo = (FromProvInfo *) $1;
				$$ = (Node *) f;
			}
        | optionalFromProv AS identifier optionalAttrAlias       
			{ 
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem($3,$4);
 				f->provInfo = (FromProvInfo *) $1; 
				$$ = (Node *) f;
			}
    ;
    
optionalFromProv:
		/* empty */ { RULELOG("optionalFromProv::empty"); $$ = NULL; }
		| BASERELATION 
			{
				RULELOG("optionalFromProv::BASERELATION");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = TRUE;
				p->userProvAttrs = NIL;				 
				$$ = (Node *) p; 
			}
		| HAS PROVENANCE '(' identifierList ')'
			{
				RULELOG("optionalFromProv::userProvAttr");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = FALSE;
				p->userProvAttrs = $4;				 
				$$ = (Node *) p; 
			}
		| USE PROVENANCE '(' identifierList ')'
			{
				RULELOG("optionalFromProv::userProvDupAttr");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = TRUE;
				p->userProvAttrs = $4;
				$$ = (Node *) p;
			}
		| SHOW INTERMEDIATE PROVENANCE
			{
				RULELOG("optionalFromProv::intermediateProv");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->intermediateProv = TRUE;
				$$ = (Node *) p;
			}
		| SHOW INTERMEDIATE PROVENANCE '(' identifierList ')'
			{
				RULELOG("optionalFromProv::intermediateProv::attrList");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->intermediateProv = TRUE;
				p->userProvAttrs = $5;
				$$ = (Node *) p;
			}
	;
    
optionalAttrAlias:
		/* empty */ { RULELOG("optionalAttrAlias::empty"); $$ = NULL; }
		| '(' identifierList ')' 
			{ 
				RULELOG("optionalAttrAlias::identifierList"); $$ = $2; 
			}
    ;
    
/*
 * Rule to parse the where clause.
 */
optionalWhere: 
        /* empty */             { RULELOG("optionalWhere::NULL"); $$ = NULL; }
        | WHERE whereExpression        { RULELOG("optionalWhere::whereExpression"); $$ = $2; }
    ;

whereExpression:
		'(' whereExpression ')' { RULELOG("where::brackedWhereExpression"); $$ = $2; } %prec DUMMYEXPR
        | expression        { RULELOG("whereExpression::expression"); $$ = $1; } %prec '+'
        | NOT whereExpression
            {
                RULELOG("whereExpression::NOT");
                List *expr = singleton($2);
                $$ = (Node *) createOpExpr($1, expr);
            }
        | whereExpression AND whereExpression	
            {
                RULELOG("whereExpression::AND");
                List *expr = singleton($1);
                expr = appendToTailOfList(expr, $3);
                $$ = (Node *) createOpExpr($2, expr);
            }
        | whereExpression OR whereExpression
            {
                RULELOG("whereExpression::OR");
                List *expr = singleton($1);
                expr = appendToTailOfList(expr, $3);
                $$ = (Node *) createOpExpr($2, expr);
            }
		| whereExpression LIKE whereExpression
			{
				//if ($2 == NULL)
                //{
                	RULELOG("whereExpression::LIKE");
	                List *expr = singleton($1);
	                expr = appendToTailOfList(expr, $3);
	                $$ = (Node *) createOpExpr($2, expr);
                /* }
				else
				{   
                	RULELOG("whereExpression::NOT::LIKE");
                	List *expr = singleton($1);
                	expr = appendToTailOfList(expr, $4);
                	Node *like = (Node *) createOpExpr($3, expr);
                	$$ = (Node *) createOpExpr("NOT", singleton(like));
				}*/
            }
        | whereExpression BETWEEN expression AND expression
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
                RULELOG("whereExpression::Subquery");
                Node *q = (Node *) createNestedSubquery("SCALAR", NULL, NULL, $4); 
                List *expr = LIST_MAKE($1, q);
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
        | EXISTS '(' queryStmt ')'
            {
                RULELOG("whereExpression::EXISTS");
                $$ = (Node *) createNestedSubquery($1, NULL, NULL, $3);
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

optionalGroupBy:
        /* Empty */        { RULELOG("optionalGroupBy::NULL"); $$ = NULL; }
        | GROUP BY exprList      { RULELOG("optionalGroupBy::GROUPBY"); $$ = $3; }
    ;

optionalHaving:
        /* Empty */        { RULELOG("optionalOrderBy:::NULL"); $$ = NULL; }
        | HAVING whereExpression
            { 
                RULELOG("optionalHaving::HAVING"); 
                $$ = (Node *) $2;
            }
    ;

optionalOrderBy:
        /* Empty */        { RULELOG("optionalOrderBy:::NULL"); $$ = NULL; }
        | ORDER BY orderList       { RULELOG("optionalOrderBy::ORDERBY"); $$ = $3; }
    ;

optionalLimit:
        /* Empty */        { RULELOG("optionalLimit::NULL"); $$ = NULL; }
        | LIMIT constant        { RULELOG("optionalLimit::CONSTANT"); $$ = $2;}
    ;

orderList:
		 orderExpr
            {
                RULELOG("clauseList::orderExpr");
                $$ = singleton($1);
            }
        | orderList ',' orderExpr
            {
                RULELOG("orderList::orderList::orderExpr");
                $$ = appendToTailOfList($1, $3);
            }
    ;

orderExpr:
		expression optionalSortOrder optionalNullOrder
			{
				RULELOG("orderExpr::expr::sortOrder::nullOrder");
				SortOrder o = (strcmp($2,"ASC") == 0) ?  SORT_ASC : SORT_DESC;
				SortNullOrder n = (strcmp($3,"NULLS_FIRST") == 0) ? 
						SORT_NULLS_FIRST : 
						SORT_NULLS_LAST;
				$$ = (Node *) createOrderExpr($1, o, n);
			}
	;
	
optionalSortOrder:
		/* empty */ { RULELOG("optionalSortOrder::empty"); $$ = "ASC"; }
		| ASC
			{
				RULELOG("optionalSortOrder::ASC");
				$$ = "ASC";
			}
		| DESC
			{
				RULELOG("optionalSortOrder::DESC");
				$$ = "DESC";
			}
	;

optionalNullOrder:
		/* empty */ { RULELOG("optionalNullOrder::empty"); $$ = "NULLS_LAST"; }
		| NULLS FIRST
			{
				RULELOG("optionalNullOrder::NULLS FIRST");
				$$ = "NULLS_FIRST";
			}
		| NULLS LAST
			{
				RULELOG("optionalNullOrder::NULLS LAST");
				$$ = "NULLS_LAST";
			}
	;

delimIdentifier:
		identifier { RULELOG("identifierList::ident"); }
		| delimIdentifier '.' identifier 
		{ 
			RULELOG("identifierList::list::ident"); 
			$$ = CONCAT_STRINGS($1, ".", $3); //TODO 
		}  

	
%%



/* FUTURE WORK 

PRIORITIES
7)
4)
1)

EXHAUSTIVE LIST
2. Implement support for RETURNING statement in DELETE queries.
3. Implement support for column list like (col1, col2, col3). 
   Needed in insert queries, select queries where conditions etc.
4. Implement support for Transactions.
5. Implement support for Create queries.
7. Implement support for AS OF (timestamp) modifier of a table reference
8. Implement support for casting expressions
9. Implement support for IN array expressions like a IN (1,2,3,4,5)
10. Implement support for ASC, DESC, NULLS FIRST/LAST in ORDER BY
11. Add DDL statements
12. Add provenance storage policy language
13. Add provenance export options (also requires several other changes in the application)
*/

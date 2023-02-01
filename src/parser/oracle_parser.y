/*
 * Sql_Parser.y
 *     This is a bison file which contains grammar rules to parse Oracle-ish SQL
 */



%{
#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "parser/parse_internal_oracle.h"
#include "log/logger.h"
#include "model/query_operator/operator_property.h"
#include "utility/string_utils.h"

#define RULELOG(grule) \
    { \
        TRACE_LOG("Parsing grammer rule <%s> at line %d", #grule, oraclelineno); \
    }

#undef free
#undef malloc

Node *oracleParseResult = NULL;
%}

%define api.prefix {oracle}

%define parse.error verbose

%union {
    /*
     * Declare some C structure those will be used as data type
     * for various tokens used in grammar rules.
     */
     Node *node;
     List *list;
     char *stringVal;
     long intVal;
     double floatVal;
}

/*
 * Declare tokens for name and literal values
 * Declare tokens for user variables
 */
%token <intVal> intConst
%token <floatVal> floatConst
%token <stringVal> stringConst
%token <stringVal> boolConst
%token <stringVal> identifier compositeIdentifier
%token <stringVal> parameter
%token <stringVal> '+' '-' '*' '/' '%' '^' '&' '|' '!' comparisonOps ')' '(' '=' '[' ']' STRINGCONCAT POSTGRESCAST

/*
 * Tokens for in-built keywords
 *        Currently keywords related to basic query are considered.
 *        Later on other keywords will be added.
 */
%token <stringVal> SELECT INSERT UPDATE DELETE
%token <stringVal> SEQUENCED TEMPORAL TIME
%token <stringVal> PROVENANCE OF BASERELATION SCN TIMESTAMP HAS TABLE ONLY UPDATED SHOW INTERMEDIATE USE TUPLE VERSIONS STATEMENT ANNOTATIONS NO REENACT OPTIONS SEMIRING COMBINER MULT UNCERTAIN URANGE
%token <stringVal> TIP INCOMPLETE XTABLE RADB UADB
%token <stringVal> CAPTURE COARSE GRAINED FRAGMENT PAGE RANGESA RANGESB
%token <stringVal> FROM
%token <stringVal> ISOLATION LEVEL
%token <stringVal> AS
%token <stringVal> WHERE
%token <stringVal> DISTINCT
%token <stringVal> STARALL
%token <stringVal> AND OR LIKE NOT IN ISNULL BETWEEN EXCEPT EXISTS
%token <stringVal> AMMSC NULLVAL ROWNUM ALL ANY IS SOME
%token <stringVal> UNION INTERSECT MINUS
%token <stringVal> INTO VALUES HAVING GROUP ORDER BY LIMIT OFFSET SET
%token <stringVal> INT BEGIN_TRANS COMMIT_TRANS ROLLBACK_TRANS
%token <stringVal> CASE WHEN THEN ELSE END
%token <stringVal> OVER_TOK PARTITION ROWS RANGE UNBOUNDED PRECEDING CURRENT ROW FOLLOWING
%token <stringVal> NULLS FIRST LAST ASC DESC
%token <stringVal> JSON_TABLE COLUMNS PATH FORMAT WRAPPER NESTED WITHOUT CONDITIONAL JSON TRANSLATE
%token <stringVal> CAST
%token <stringVal> CREATE ALTER ADD REMOVE COLUMN
%token <stringVal> SUMMARIZED TO EXPLAIN SAMPLE TOP

%token <stringVal> DUMMYEXPR

/* Keywords for Join queries */
%token <stringVal> JOIN NATURAL LEFT RIGHT OUTER INNER CROSS ON USING FULL TYPE TRANSACTION WITH

/*
 * Declare token for operators specify their associativity and precedence
 */
%left UNION INTERSECT MINUS EXCEPT

/* Logical operators */
%left '|'
%left STRINGCONCAT
%left XOR
%left '&'
/* what is that? %right ':=' */
%left '!'

/* Comparison operator */
%left comparisonOps
%right NOT
%left AND OR
%right IS NULLVAL
%nonassoc  LIKE IN  BETWEEN

/* Arithmetic operators : FOR TESTING */
%nonassoc DUMMYEXPR
%right POSTGRESCAST
%left '+' '-'
%left '*' '/' '%'
%left '^'
%nonassoc '(' ')'
%nonassoc '[' ']'
						
%left NATURAL JOIN CROSS LEFT FULL RIGHT INNER

/*
 * Types of non-terminal symbols
 */
%type <node> stmt provStmt dmlStmt queryStmt ddlStmt reenactStmtWithOptions
%type <node> createTableStmt alterTableStmt alterTableCommand
%type <list> tableElemList optTableElemList attrElemList
%type <node> tableElement attr
%type <node> selectQuery deleteQuery updateQuery insertQuery subQuery setOperatorQuery
        // Its a query block model that defines the structure of query.
%type <list> selectClause optionalFrom fromClause exprList orderList
			 optionalGroupBy optionalOrderBy setClause  stmtList stmtWithReenactOptionsList //insertList
			 identifierList optionalAttrAlias optionalProvWith provOptionList optionalReenactOptions reenactOptionList
			 caseWhenList windowBoundaries optWindowPart withViewList jsonColInfo optionalTranslate
//			 optInsertAttrList
%type <node> selectItem fromClauseItem fromJoinItem optionalFromProv optionalAlias optionalDistinct optionalWhere optionalLimit optionalOffset optionalHaving orderExpr insertContent
             //optionalReruning optionalGroupBy optionalOrderBy optionalLimit
%type <node> optionalFromTIP optionalFromIncompleteTable optionalFromXTable optionalFromRADB optionalFromUADB
%type <node> expression constant attributeRef sqlParameter sqlFunctionCall whereExpression setExpression caseExpression caseWhen optionalCaseElse castExpression
%type <node> overClause windowSpec optWindowFrame windowBound
%type <node> jsonTable jsonColInfoItem
%type <node> binaryOperatorExpression unaryOperatorExpression
%type <node> joinCond
%type <node> optionalProvAsOf provAsOf provOption reenactOption semiringCombinerSpec coarseGrainedSpec optionalCoarseGrainedPara
%type <list> fragmentList pageList rangeAList rangeBList intConstList attrRangeList
%type <node> withView withQuery
%type <stringVal> optionalAll nestedSubQueryOperator optionalNot fromString optionalSortOrder optionalNullOrder
%type <stringVal> joinType transactionIdentifier delimIdentifier
%type <stringVal> optionalFormat optionalWrapper optionalstringConst
%type <node> optionalTopK optionalSumType optionalToExplain optionalSumSample
%type <list> optionalSummarization
%type <intVal>	optionalCountDistinct

%start stmtList

%%

/* Rule for all types of statements */
stmtList:
		stmt ';'
			{
				RULELOG("stmtList::stmt");
				$$ = singleton($1);
				oracleParseResult = (Node *) $$;
			}
		| stmtList stmt ';'
			{
				RULELOG("stmtlist::stmtList::stmt");
				$$ = appendToTailOfList($1, $2);
				oracleParseResult = (Node *) $$;
			}
		| '[' whereExpression ']'
		{
			RULELOG("stmt::expression");
			$$ = singleton($2);
			oracleParseResult = (Node *) $$;
		}
	;

stmt:
        ddlStmt		// DDL statments
        {
        	RULELOG("stmt::ddlStmt");
        	$$ = $1;
        }
        | dmlStmt    // DML statement can be select, update, insert, delete
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
 * Rule to parse all DDL statements.
 */
ddlStmt:
		createTableStmt
		{
			RULELOG("ddlStmt::createTable");
			$$ = $1;
		}
		| alterTableStmt
		{
			RULELOG("ddlStmt::alterTable");
			$$ = $1;
		}
	;

createTableStmt:
		CREATE TABLE identifier AS queryStmt
		{
			RULELOG("createTable::query");
			$$ = (Node *) createCreateTableQuery($3,$5);
		}
		| CREATE TABLE identifier '(' optTableElemList ')'
		{
			RULELOG("createTable::");
			$$ = (Node *) createCreateTable($3,$5);
		}
	;

optTableElemList:
		/* EMPTY */
		{
			RULELOG("optTableElemList::EMPTY");
			$$ = NIL;
		}
		| tableElemList
		{
			RULELOG("optTableElemList::tableElemList");
			$$ = $1;
		}
	;

tableElemList:
		tableElement
		{
			RULELOG("tableElemList::tableElement");
			$$ = singleton($1);
		}
		| tableElemList ',' tableElement
		{
			RULELOG("tableElemList::tableElement");
			$$ = appendToTailOfList($1,$3);
		}
	;

tableElement:
		identifier identifier
		{
			RULELOG("tableElement::columnDef");
			$$ = (Node *) createAttributeDef($1, SQLdataTypeToDataType($2));
		}
		/* TODO add constraints and make column def more general */
	;

alterTableStmt:
		ALTER TABLE identifier alterTableCommand
		{
			RULELOG("alterTable:");
			AlterTable *a = (AlterTable *) $4;
			a->tableName = $3;
			$$ = (Node *) a;
		}
	;

alterTableCommand:
		ADD COLUMN identifier identifier
		{
			RULELOG("alterTableCommand::addColumn");
			$$ = (Node *) createAlterTableAddColumn(NULL,$3,$4);
		}
		| REMOVE COLUMN identifier
		{
			RULELOG("alterTableCommand::addColumn");
			$$ = (Node *) createAlterTableRemoveColumn(NULL,$3);
		}
	;

/*
 * Rule to parse all DML statements.
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
        PROVENANCE optionalProvAsOf optionalProvWith OF '(' stmt ')' optionalTranslate
        {
            RULELOG("provStmt::stmt");
            Node *stmt = $6;
	    	ProvenanceStmt *p = createProvenanceStmt(stmt);
		    p->inputType = isQBUpdate(stmt) ? PROV_INPUT_UPDATE : PROV_INPUT_QUERY;
		    p->provType = PROV_PI_CS;
		    p->asOf = (Node *) $2;
            // p->options = $3;
            p->options = concatTwoLists($3, $8);
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
		| REENACT optionalProvAsOf optionalProvWith '(' stmtWithReenactOptionsList ')'
		{
			RULELOG("provStmt::stmtWithReenactOptionsList");
			ProvenanceStmt *p = createProvenanceStmt((Node *) $5);
			p->inputType = PROV_INPUT_REENACT;
			p->provType = PROV_NONE;
			p->asOf = (Node *) $2;
			p->options = $3;
			$$ = (Node *) p;
		}
		| REENACT optionalProvAsOf optionalProvWith TRANSACTION stringConst
		{
			RULELOG("provStmt::transaction");
			ProvenanceStmt *p = createProvenanceStmt((Node *) createConstString($5));
			p->inputType = PROV_INPUT_TRANSACTION;
			p->provType = PROV_NONE;
			p->options = $3;
			$$ = (Node *) p;
		}
		| CAPTURE PROVENANCE optionalProvAsOf optionalProvWith OF '(' stmt ')' optionalTranslate
        {
            RULELOG("provStmt::stmt");
            Node *stmt = $7;
	    		ProvenanceStmt *p = createProvenanceStmt(stmt);
		    p->inputType = isQBUpdate(stmt) ? PROV_INPUT_UPDATE : PROV_INPUT_QUERY;
		    p->provType = PROV_COARSE_GRAINED;
		    p->asOf = (Node *) $3;
            // p->options = $4;
            p->options = concatTwoLists($4, $9);
            $$ = (Node *) p;
        }
        | USE PROVENANCE optionalProvAsOf optionalProvWith OF '(' stmt ')' optionalTranslate
        {
            RULELOG("provStmt::stmt");
            Node *stmt = $7;
	    	ProvenanceStmt *p = createProvenanceStmt(stmt);
		    p->inputType = isQBUpdate(stmt) ? PROV_INPUT_UPDATE : PROV_INPUT_QUERY;
		    p->provType = USE_PROV_COARSE_GRAINED;
		    p->asOf = (Node *) $3;
            // p->options = $4;
            p->options = concatTwoLists($4, $9);
            $$ = (Node *) p;
        }
		| SEQUENCED TEMPORAL '(' stmt ')'
		{
			RULELOG("provStmt::temporal");
			ProvenanceStmt *p = createProvenanceStmt((Node *) $4);
			p->inputType = PROV_INPUT_TEMPORAL_QUERY;
			p->provType = PROV_NONE;
			p->asOf = NULL;
			p->options = NIL;
			$$ = (Node *) p;
		}
		| UNCERTAIN '(' stmt ')'
		{
			RULELOG("provStmt::uncertain");
			ProvenanceStmt *p = createProvenanceStmt((Node *) $3);
			p->inputType = PROV_INPUT_UNCERTAIN_QUERY;
			p->provType = PROV_NONE;
			p->asOf = NULL;
			p->options = NIL;
			$$ = (Node *) p;
		}
		| TUPLE UNCERTAIN '(' stmt ')'
		{
			RULELOG("provStmt::uncertain");
			ProvenanceStmt *p = createProvenanceStmt((Node *) $4);
			p->inputType = PROV_INPUT_UNCERTAIN_TUPLE_QUERY;
			p->provType = PROV_NONE;
			p->options = NULL;
			$$ = (Node *) p;
		}
		| URANGE '(' stmt ')'
		{
			RULELOG("provStmt::range");
			ProvenanceStmt *p = createProvenanceStmt((Node *) $3);
			p->inputType = PROV_INPUT_RANGE_QUERY;
			p->provType = PROV_NONE;
			p->asOf = NULL;
			p->options = NIL;
			$$ = (Node *) p;
		}
        | optionalTopK PROVENANCE optionalProvAsOf optionalProvWith OF '(' stmt ')' optionalTranslate optionalSummarization
        {
            RULELOG("provStmt::summaryStmt");
            Node *stmt = $7;
	    	ProvenanceStmt *p = createProvenanceStmt(stmt);
		    p->inputType = isQBUpdate(stmt) ? PROV_INPUT_UPDATE : PROV_INPUT_QUERY;
		    p->provType = PROV_PI_CS;
		    p->asOf = (Node *) $3;
            p->options = CONCAT_LISTS(singleton($1),$4,$9,$10,
									  singleton(createNodeKeyValue((Node *) createConstString(PROP_SUMMARIZATION_DOSUM),
																   (Node *) createConstBool(TRUE))));
           	/* p->sumOpts = appendToTailOfList(p->sumOpts,$1); */
           	/* p->sumOpts = appendToTailOfList(p->sumOpts,(Node *) $10); */
            $$ = (Node *) p;
        }
    ;


optionalTopK:
//		/* empty */			{ RULELOG("optionalTopK::EMPTY"); $$ = NULL; }
//		|
		TOP intConst
		{
			RULELOG("optionalTopK::topk");
			$$ = (Node *) createNodeKeyValue((Node *) createConstString(PROP_SUMMARIZATION_TOPK),(Node *) createConstInt($2));
		}
    ;


optionalSummarization:
		optionalSumType		{ RULELOG("optionalSumType::sumType"); $$ = singleton($1); }
		| optionalSummarization optionalSumType
        {
        	RULELOG("optionalSummarization::sumOpts");
			$$ = appendToTailOfList($1,$2);
        }
        | optionalSummarization optionalToExplain
        {
        	RULELOG("optionalSummarization::sumOpts");
			$$ = appendToTailOfList($1,$2);
        }
        | optionalSummarization optionalSumSample
        {
        	RULELOG("optionalSummarization::sumOpts");
			$$ = appendToTailOfList($1,$2);
        }
	;

stmtWithReenactOptionsList:
		reenactStmtWithOptions
		{
			RULELOG("stmtWithTimeList::stmt");
			$$ = singleton($1);
		}
		| stmtWithReenactOptionsList reenactStmtWithOptions
		{
			RULELOG("stmtWithTimeList::stmtWithTimeList::stmt");
			$$ = appendToTailOfList($1, $2);
		}
	;

reenactStmtWithOptions:
		optionalReenactOptions stmt ';'
		{
			RULELOG("reenactStmtWithOptions");
			KeyValue *kv = createNodeKeyValue((Node *) $2, (Node *) $1);
			$$ = (Node *) kv;
		}
	;

optionalReenactOptions:
		/* empty */
		{
			RULELOG("optionalReenactOptions:EMPTY");
			$$ = NIL;
		}
		| OPTIONS '(' reenactOptionList ')'
		{
			RULELOG("optionalReenactOptions:reenactOptionList");
			$$ = $3;
		}
	;

reenactOptionList:
		reenactOption
		{
			RULELOG("reenactOptionList:option");
			$$ = singleton($1);
		}
		| reenactOptionList reenactOption
		{
			RULELOG("reenactOptionList:list:option");
			$$ = CONCAT_LISTS($1, singleton($2));
		}
	;

reenactOption:
		provAsOf
		{
			RULELOG("reenactOption:provAsOf");
			$$ = (Node *) createNodeKeyValue((Node *) createConstString(PROP_REENACT_ASOF), $1);
		}
		| NO PROVENANCE
		{
			RULELOG("reenactOption:noProvenance");
			$$ = (Node *) createNodeKeyValue(
					(Node *) createConstString(PROP_REENACT_DO_NOT_TRACK_PROV),
					(Node *) createConstBool(TRUE));
		}
	;



optionalSumType:
		SUMMARIZED BY identifier
		{
			RULELOG("optionalSummarization::SumType");
			$$ = (Node *) createStringKeyValue(strdup(PROP_SUMMARIZATION_TYPE),strdup($3));
		}
	;


optionalToExplain:
		TO EXPLAIN '(' attrElemList ')'
		{
			RULELOG("optionalToExplain::ToExplain");
			$$ = (Node *) createNodeKeyValue((Node *) createConstString(PROP_SUMMARIZATION_TO_EXPLAIN),(Node *) $4);
		}
	;


optionalSumSample:
		WITH SAMPLE '(' intConst ')'
		{
			RULELOG("optionalSumSample::WithSample");
			$$ = (Node *) createNodeKeyValue((Node *) createConstString(PROP_SUMMARIZATION_SAMPLE),(Node *) createConstInt($4));
		}
 	;


optionalProvAsOf:
		/* empty */			{ RULELOG("optionalProvAsOf::EMPTY"); $$ = NULL; }
		| provAsOf
		{
			RULELOG("optionalProvAsOf::provAsOf");
			$$ = $1;
		}
	;

provAsOf:
		AS OF SCN intConst
		{
			RULELOG("provAsOf::SCN");
			$$ = (Node *) createConstLong($4);
		}
		| AS OF TIMESTAMP stringConst
		{
			RULELOG("provAsOf::TIMESTAMP");
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
			$$ = (Node *) createStringKeyValue(PROP_PC_PROV_TYPE, $2);
		}
		| TABLE identifier
		{
			RULELOG("provOption::TABLE");
			$$ = (Node *) createStringKeyValue(PROP_PC_TABLE, $2);
		}
		| COARSE GRAINED coarseGrainedSpec
		{
			RULELOG("provOption::COARSE");
            $$ = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_COARSE_GRAINED),
            									(Node *) $3);
		}
		| USE COARSE GRAINED coarseGrainedSpec
		{
			RULELOG("provOption::COARSE");
            $$ = (Node *) createNodeKeyValue((Node *) createConstString(USE_PROP_PC_COARSE_GRAINED),
            									(Node *) $4);
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
		| PROVENANCE
		{
			RULELOG("provOption::PROVENANCE");
			$$ = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_GEN_PROVENANCE),
					(Node *) createConstBool(TRUE));
		}
		| ISOLATION LEVEL identifier
		{
			RULELOG("provOption::ISOLATION::LEVEL");
			$$ = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_ISOLATION_LEVEL),
					(Node *) createConstString(strdup($3)));
		}
		| COMMIT_TRANS SCN intConst
		{
			RULELOG("provOption::COMMIT::SCN");
			$$ = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_COMMIT_SCN),
					(Node *) createConstLong($3));
		}
		| SEMIRING COMBINER semiringCombinerSpec
		{
			RULELOG("provOption::SEMIRING::COMBINER::semiringCombinerSpec");
            $$ = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_SEMIRING_COMBINER),
            									(Node *) $3);
		}
	;

coarseGrainedSpec:
		FRAGMENT '(' fragmentList ')'
		{
			RULELOG("coarse_grained::fragmentlist");
			$$ = (Node *) $3;
		}
		|
		PAGE '(' pageList ')'
		{
			RULELOG("coarse_grained::pagelist");
			$$ = (Node *) $3;
		}
		|
		RANGESA '(' rangeAList ')'
		{
			RULELOG("coarse_grained::rangelist");
			$$ = (Node *) $3;
		}
		|
		RANGESB '(' rangeBList ')'
		{
			RULELOG("coarse_grained::rangelist");
			$$ = (Node *) $3;
		}

	;


rangeAList:
       identifier '(' identifierList intConst intConst ')' intConst optionalCoarseGrainedPara
       {
            RULELOG("rangeList::identifier::intConst::intConst");
            List *l = NIL;
            KeyValue *k1 = createNodeKeyValue((Node *) createConstString("PTYPE"),
            									(Node *) createConstString("RANGEA"));
            KeyValue *k2 = createNodeKeyValue((Node *) createConstString("ATTRS"),
            									(Node *) stringListToConstList($3));
            KeyValue *k3 = createNodeKeyValue((Node *) createConstString("BEGIN"),
            									(Node *) createConstInt($4));
            KeyValue *k4 = createNodeKeyValue((Node *) createConstString("END"),
            									(Node *) createConstInt($5));
            KeyValue *k5 = createNodeKeyValue((Node *) createConstString("HVALUE"),
            									(Node *) createConstInt($7));
            if($8 == NULL)
            {
                l = LIST_MAKE(k1,k2,k3,k4,k5);
				//l = LIST_MAKE(createConstString($3),createConstInt($4), createConstInt($5), createConstInt($7));
			}
		    else
		    {
		        KeyValue *k6 = createNodeKeyValue((Node *) createConstString("UHVALUE"),
            									(Node *) $8);
				l = LIST_MAKE(k1,k2,k3,k4,k5,k6);
		        //l = LIST_MAKE(createConstString($3),createConstInt($4), createConstInt($5), createConstInt($7), $8);
		    }
            KeyValue *k = createNodeKeyValue((Node *) createConstString($1),
            									(Node *) l);
            $$ = singleton(k);
       }
       |
       rangeAList ',' identifier '(' identifierList intConst intConst ')' intConst optionalCoarseGrainedPara
       {
            RULELOG("rangeList::rangeList::rangeList ");
            List *l = NIL;
            KeyValue *k1 = createNodeKeyValue((Node *) createConstString("PTYPE"),
            									(Node *) createConstString("RANGEA"));
            KeyValue *k2 = createNodeKeyValue((Node *) createConstString("ATTRS"),
            									(Node *) stringListToConstList($5));
            KeyValue *k3 = createNodeKeyValue((Node *) createConstString("BEGIN"),
            									(Node *) createConstInt($6));
            KeyValue *k4 = createNodeKeyValue((Node *) createConstString("END"),
            									(Node *) createConstInt($7));
            KeyValue *k5 = createNodeKeyValue((Node *) createConstString("HVALUE"),
            									(Node *) createConstInt($9));
            if($10 == NULL)
                l = LIST_MAKE(k1,k2,k3,k4,k5);
		    else
		    {
		        KeyValue *k6 = createNodeKeyValue((Node *) createConstString("UHVALUE"),
            									(Node *) $10);
				l = LIST_MAKE(k1,k2,k3,k4,k5,k6);
		        //l = LIST_MAKE(createConstString($3),createConstInt($4), createConstInt($5), createConstInt($7), $8);
		    }

            //List *l = LIST_MAKE(createConstString($5),createConstInt($6), createConstInt($7), createConstInt($9));
            KeyValue *k = createNodeKeyValue((Node *) createConstString($3),
            									(Node *) l);
            $$ = appendToTailOfList($1, k);
       }
    ;


intConstList:
		intConst { $$ = singleton((Node *) createConstInt($1)); }
		| intConstList ',' intConst { $$ = appendToTailOfList($1, (Node *) createConstInt($3)); }
	;



attrRangeList:
         '(' delimIdentifier intConstList ')'
         {
         	KeyValue *k = createNodeKeyValue((Node *) createConstString($2),
            									(Node *) $3);
            $$ = singleton(k);
         }
         | attrRangeList '(' delimIdentifier intConstList ')'
         {
         	KeyValue *k = createNodeKeyValue((Node *) createConstString($3),
            									(Node *) $4);
            $$ = appendToTailOfList($1, k);
         }
	;

rangeBList:
       identifier '(' attrRangeList ')' optionalCoarseGrainedPara
       {
            RULELOG("rangeList::identifierList::intConstList");
            List *l = NIL;
            KeyValue *k1 = createNodeKeyValue((Node *) createConstString("PTYPE"),
            									(Node *) createConstString("RANGEB"));
            KeyValue *k2 = createNodeKeyValue((Node *) createConstString("ATTRSRANGES"),
            									(Node *) $3);
            //KeyValue *k3 = createNodeKeyValue((Node *) createConstString("RANGES"),
            	//								(Node *) $4);
            if($5 == NULL)
            {
                l = LIST_MAKE(k1,k2);
				//l = LIST_MAKE(createConstString($3),createConstInt($4), createConstInt($5), createConstInt($7));
			}
		    else
		    {
		        KeyValue *k3 = createNodeKeyValue((Node *) createConstString("UHVALUE"),
            									(Node *) $5);
				l = LIST_MAKE(k1,k2,k3);
		        //l = LIST_MAKE(createConstString($3),createConstInt($4), createConstInt($5), createConstInt($7), $8);
		    }
            KeyValue *k = createNodeKeyValue((Node *) createConstString($1),
            									(Node *) l);
            $$ = singleton(k);
       }
       |
       rangeBList ',' identifier '(' attrRangeList ')' optionalCoarseGrainedPara
       {
            RULELOG("rangeList::rangeList::rangeList ");
            List *l = NIL;
            KeyValue *k1 = createNodeKeyValue((Node *) createConstString("PTYPE"),
            									(Node *) createConstString("RANGEB"));
            KeyValue *k2 = createNodeKeyValue((Node *) createConstString("ATTRSRANGES"),
            									(Node *) $5);
            //KeyValue *k3 = createNodeKeyValue((Node *) createConstString("RANGES"),
            	//								(Node *) $6);
            if($7 == NULL)
                l = LIST_MAKE(k1,k2);
		    else
		    {
		        KeyValue *k3 = createNodeKeyValue((Node *) createConstString("UHVALUE"),
            									(Node *) $7);
				l = LIST_MAKE(k1,k2,k3);
		        //l = LIST_MAKE(createConstString($3),createConstInt($4), createConstInt($5), createConstInt($7), $8);
		    }

            //List *l = LIST_MAKE(createConstString($5),createConstInt($6), createConstInt($7), createConstInt($9));
            KeyValue *k = createNodeKeyValue((Node *) createConstString($3),
            									(Node *) l);
            $$ = appendToTailOfList($1, k);
       }
    ;


pageList:
       identifier intConst optionalCoarseGrainedPara
       {
            RULELOG("pageList::identifier::identifier");
            List *l = NIL;
            KeyValue *k1 = createNodeKeyValue((Node *) createConstString("PTYPE"),
            									(Node *) createConstString("PAGE"));
            KeyValue *k2 = createNodeKeyValue((Node *) createConstString("HVALUE"),
            									(Node *) createConstInt($2));
            if($3 == NULL)
            {
                 l = LIST_MAKE(k1,k2);
            		//l = singleton(createConstInt($2));
            	}
            else
            {
                //l = CONCAT_LISTS(singleton(createConstInt($2)), singleton($3));
                KeyValue *k3 = createNodeKeyValue((Node *) createConstString("UHVALUE"),
            									(Node *) $3);
                l = LIST_MAKE(k1,k2,k3);
            }
            KeyValue *k = createNodeKeyValue((Node *) createConstString($1),
            									(Node *) l);
            $$ = singleton(k);
       }
       |
       pageList ',' identifier intConst optionalCoarseGrainedPara
       {
            RULELOG("pageList::pageList::pageList");
            List *l = NIL;
            KeyValue *k1 = createNodeKeyValue((Node *) createConstString("PTYPE"),
            									(Node *) createConstString("PAGE"));
            KeyValue *k2 = createNodeKeyValue((Node *) createConstString("HVALUE"),
            									(Node *) createConstInt($4));
            if($5 == NULL)
                 l = LIST_MAKE(k1,k2);
            	else
            	{
            	    KeyValue *k3 = createNodeKeyValue((Node *) createConstString("UHVALUE"),
            									(Node *) $5);
                l = LIST_MAKE(k1,k2,k3);
            	}
            //List *l = singleton(createConstInt($4));
            KeyValue *k = createNodeKeyValue((Node *) createConstString($3),
            									(Node *) l);
            $$ = appendToTailOfList($1, k);
       }
    ;

fragmentList:
       identifier '(' identifierList ')' intConst optionalCoarseGrainedPara
       {
            RULELOG("fragmentList::identifier::identifierList::identifier");
            List *l = NIL;
            KeyValue *k1 = createNodeKeyValue((Node *) createConstString("PTYPE"),
            									(Node *) createConstString("FRAGMENT"));
            KeyValue *k2 = createNodeKeyValue((Node *) createConstString("ATTRS"),
            									(Node *) stringListToConstList($3));
            KeyValue *k3 = createNodeKeyValue((Node *) createConstString("HVALUE"),
            									(Node *) createConstInt($5));
            if($6 == NULL)
            {
             	//l = concatTwoLists(stringListToConstList($3),singleton(createConstInt($5)));
            		l = LIST_MAKE(k1,k2,k3);
             }
            else
            {
                //l = CONCAT_LISTS(stringListToConstList($3),singleton(createConstInt($5)), singleton($6));
                KeyValue *k4 = createNodeKeyValue((Node *) createConstString("UHVALUE"),
            									(Node *) $6);
            	    l = LIST_MAKE(k1,k2,k3,k4);
            }
            KeyValue *k = createNodeKeyValue((Node *) createConstString($1),
            									(Node *) l);
            $$ = singleton(k);
       }
       |
       fragmentList ',' identifier '(' identifierList ')' intConst optionalCoarseGrainedPara
       {
            RULELOG("fragmentList::fragmentList::fragmentList");
            List *l = NIL;
            KeyValue *k1 = createNodeKeyValue((Node *) createConstString("PTYPE"),
            									(Node *) createConstString("FRAGMENT"));
            KeyValue *k2 = createNodeKeyValue((Node *) createConstString("ATTRS"),
            									(Node *) stringListToConstList($5));
            KeyValue *k3 = createNodeKeyValue((Node *) createConstString("HVALUE"),
            									(Node *) createConstInt($7));
            if($8 == NULL)
            		l = LIST_MAKE(k1,k2,k3);
            	else
            	{
            	    KeyValue *k4 = createNodeKeyValue((Node *) createConstString("UHVALUE"),
            									(Node *) $8);
            	    l = LIST_MAKE(k1,k2,k3,k4);
            	}

            //List *l = concatTwoLists(stringListToConstList($5),singleton(createConstInt($7)));
            KeyValue *k = createNodeKeyValue((Node *) createConstString($3),
            									(Node *) l);
            $$ = appendToTailOfList($1, k);
       }
    ;

optionalCoarseGrainedPara:
         /* empty */ { RULELOG("optionalCoarseGrainedPara::EMPTY"); $$ = NULL;}
         |
         intConst
         {
         	$$ = (Node *) createConstLong($1);
         }

semiringCombinerSpec:
   		identifier
        {
            //$$ = createConstString($1);
            RULELOG("semiringCombinerSpec::identifier");
            $$ = (Node *)createConstString($1);
        }
        |
        ADD '(' expression ')'  MULT '(' expression ')'
        {
            RULELOG("semiringCombinerSpec::ADD::expression::MULT::expression");
            $$ = (Node *) LIST_MAKE($3, $7);
        }
;

optionalTranslate:
                /* empty */ { RULELOG("optionalTranslate::EMPTY"); $$ = NULL;}
                |
		TRANSLATE AS optionalstringConst
		{
			RULELOG("optionaltranslate::TRANSLATE::AS");
			$$ = singleton((Node *) createStringKeyValue(strdup("TRANSLATE AS"), $3));
		}
        ;

optionalstringConst:
		JSON
		{
			RULELOG("optionalstringConst::JSON");
                        $$ = strdup("JSON");
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
        queryStmt INTERSECT optionalAll queryStmt
            {
                RULELOG("setOperatorQuery::INTERSECT");
                $$ = (Node *) createSetQuery($2, ($3 != NULL), $1, $4);
            }
        | queryStmt MINUS optionalAll queryStmt
            {
                RULELOG("setOperatorQuery::EXCEPT");
                $$ = (Node *) createSetQuery($2, ($3 != NULL), $1, $4);
            }
		| queryStmt EXCEPT optionalAll queryStmt
            {
                RULELOG("setOperatorQuery::MINUS");
                $$ = (Node *) createSetQuery($2, ($3 != NULL), $1, $4);
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
        SELECT optionalDistinct selectClause optionalFrom optionalWhere optionalGroupBy optionalHaving optionalOrderBy optionalLimit optionalOffset
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
                q->offsetClause = $10;

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
        | castExpression				{ RULELOG("expression::castExpression"); }
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
        | stringConst       { RULELOG("constant::STRING"); $$ = (Node *) createConstString($1); }
        | boolConst			{ RULELOG("constant::BOOL"); $$ = (Node *) createConstBoolFromString($1); }
		| NULLVAL           { RULELOG("constant::NULL"); $$ = (Node *) createNullConst(DT_STRING); }
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
        | expression STRINGCONCAT expression
        	{
                RULELOG("binaryOperatorExpression:: '||' ");
                List *expr = singleton($1);
                expr = appendToTailOfList(expr, $3);
                $$ = (Node *) createOpExpr(strdup("||"), expr);
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
         | expression IS NULLVAL
         {
         	RULELOG("unaryOperatorExpression::IS NULL");
         	$$ = (Node *) createIsNullExpr($1);
         }
         | expression IS NOT NULLVAL
         {
         	RULELOG("unaryOperatorExpression::IS NOT NULL");
         	$$ = (Node *) createOpExpr(OPNAME_NOT, singleton(createIsNullExpr($1)));
         }
		 | expression POSTGRESCAST identifier
		 {
			 RULELOG("postgres castExpression");
             CastExpr *c = createCastExpr($1, strdup($3), SQLdataTypeToDataType($3));
			 $$ = (Node *) c;
		 }
    ;

/*
 * Rule to parse function calls
 */
sqlFunctionCall:
		identifier '(' ')' overClause
        {
			RULELOG("sqlFunctionCall::IDENTIFIER::exprList");
			FunctionCall *f = createFunctionCall($1, NIL);
			if ($4 != NULL)
				$$ = (Node *) createWindowFunction(f, (WindowDef *) $4);
			else
				$$ = (Node *) f;
		}
        | identifier '(' exprList ')' overClause
            {
                RULELOG("sqlFunctionCall::IDENTIFIER::exprList");
				FunctionCall *f = createFunctionCall($1, $3);
				if ($5 != NULL)
					$$ = (Node *) createWindowFunction(f, (WindowDef *) $5);
				else
                	$$ = (Node *) f;
            }
		| AMMSC '(' optionalCountDistinct exprList ')' overClause
            {
                RULELOG("sqlFunctionCall::AMMSC::exprList");
				FunctionCall *f = createFunctionCall($1, $4);
				if ($3 == 1)
				{
					f->isDistinct = TRUE;
				}
				if ($6 != NULL)
					$$ = (Node *) createWindowFunction(f, (WindowDef *) $6);
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

// optional distinct in count (distinct ...)
optionalCountDistinct:
	   /*  EMPTY */ { RULELOG("optionalDistinct::EMPTY"); $$ = 0; }
	   | DISTINCT { RULELOG("optionalDistinct::DISTINCT"); $$ = 1; }
    ;

/*
 * Rule for parsing CAST (expr AS type)
 */
castExpression:
		CAST '(' expression AS identifier ')'
			{
				RULELOG("castExpression");
				CastExpr *c = createCastExpr($3, strdup($5), SQLdataTypeToDataType($5));
				$$ = (Node *) c;
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
		WHEN whereExpression THEN expression
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
  $$ = (Node *) createFromJsonTable((AttributeReference *) $3, $5, $8, $12, NULL);
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
                | FORMAT JSON
                        {
                                RULELOG("optionalFormat::FORMAT");
                                $$ = strdup("JSON");
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

optionalFromTIP:
		IS TIP '(' identifier ')'
		{
			RULELOG("optionalFromTIP");
			FromProvInfo *p = makeNode(FromProvInfo);
			setStringProvProperty(p, PROV_PROP_TIP_ATTR, (Node *) createConstString($4));
			$$ = (Node *) p;
		}
;

optionalFromIncompleteTable:
		IS INCOMPLETE
		{
			RULELOG("optionalFromIncompleteTable");
			FromProvInfo *p = makeNode(FromProvInfo);
			setStringProvProperty(p, PROV_PROP_INCOMPLETE_TABLE, (Node *) createConstBool(1));
			$$ = (Node *) p;
		}
;
optionalFromRADB:
		IS RADB
		{
			RULELOG("optionalFromRADB");
			FromProvInfo *p = makeNode(FromProvInfo);
			setStringProvProperty(p, PROV_PROP_RADB, (Node *) createConstBool(1));
			$$ = (Node *) p;
		}
;
optionalFromUADB:
		IS UADB
		{
			RULELOG("optionalFromUADB");
			FromProvInfo *p = makeNode(FromProvInfo);
			setStringProvProperty(p, PROV_PROP_UADB, (Node *) createConstBool(1));
			$$ = (Node *) p;
		}
;
optionalFromXTable:
		IS XTABLE '(' identifier  ',' identifier ')'
		{
			RULELOG("optionalFromXTable");
			FromProvInfo *p = makeNode(FromProvInfo);
			setStringProvProperty(p, PROV_PROP_XTABLE_GROUPID, (Node *) createConstString($4));
			setStringProvProperty(p, PROV_PROP_XTABLE_PROB, (Node *) createConstString($6));
			$$ = (Node *) p;
		}
;



optionalFromProv:
		/* empty */ { RULELOG("optionalFromProv::empty"); $$ = NULL; }
		| optionalFromTIP {  $$ = $1; }
		| optionalFromRADB {  $$ = $1; }
		| optionalFromUADB {  $$ = $1; }
		| optionalFromIncompleteTable { $$ = $1; }
		| optionalFromXTable { $$ = $1; }
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
		| WITH TIME '(' identifierList ')'
		{
			RULELOG("optionalFromProv::userProvAttr");
			FromProvInfo *p = makeNode(FromProvInfo);
			p->baserel = FALSE;
			p->userProvAttrs = $4;
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
                	$$ = (Node *) createOpExpr(OPNAME_NOT, singleton(like));
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
        | expression optionalNot IN '(' exprList ')'
            {
                if ($2 == NULL)
                {
                    RULELOG("whereExpression::IN");
                    $$ = (Node *) createQuantifiedComparison("ANY", $1, OPNAME_EQ, $5);
                }
                else
                {
                    RULELOG("whereExpression::NOT::IN");
                    $$ = (Node *) createQuantifiedComparison("ALL",$1, "<>", $5);
                }
            }
        | expression optionalNot IN '(' queryStmt ')'
            {
                if ($2 == NULL)
                {
                    RULELOG("whereExpression::IN");
                    $$ = (Node *) createNestedSubquery("ANY", $1, OPNAME_EQ, $5);
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

optionalOffset:
        /* Empty */        { RULELOG("optionalOffset::NULL"); $$ = NULL; }
        | OFFSET constant        { RULELOG("optionalOffset::CONSTANT"); $$ = $2;}
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
	;

attrElemList:
		attr
            {
                RULELOG("userQuestList::identifier");
                $$ = singleton($1);
            }
        | attrElemList ',' attr
            {
                RULELOG("attrElemList::attrElemList::identifier");
                $$ = appendToTailOfList($1, $3);
            }
    ;

attr:
 		constant
 			{
 				RULELOG("arg:constant");
 				$$ = $1;
			}
		| '*'
			{
 				RULELOG("arg:star");
 				$$ = (Node *) createConstString(strdup("*"));
			}
	;

%%

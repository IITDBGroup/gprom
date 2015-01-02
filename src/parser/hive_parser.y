%{
#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "parser/parse_internal_hive.h"
#include "log/logger.h"
#include "model/query_operator/operator_property.h"

#define RULELOG(grule) \
    { \
        TRACE_LOG("Parsing grammer rule <%s>", #grule); \
    }
    
#undef free
#undef malloc

Node *hiveParseResult = NULL;
%}

%name-prefix "hive"

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
%token <stringVal> kwUser kwRole kwInner
%token <stringVal> KW_TRUE KW_FALSE KW_ALL KW_AND KW_OR KW_NOT KW_LIKE KW_IF KW_EXISTS 
%token <stringVal> KW_ASC KW_DESC KW_ORDER KW_GROUP KW_BY KW_HAVING KW_WHERE KW_FROM KW_AS KW_SELECT 
%token <stringVal> KW_DISTINCT KW_INSERT KW_OVERWRITE KW_OUTER KW_UNIQUEJOIN KW_PRESERVE KW_JOIN KW_LEFT 
%token <stringVal> KW_RIGHT KW_FULL KW_ON KW_PARTITION KW_PARTITIONS KW_TABLE KW_TABLES KW_INDEX 
%token <stringVal> KW_INDEXES KW_REBUILD KW_FUNCTIONS KW_SHOW KW_MSCK KW_REPAIR KW_DIRECTORY KW_LOCAL KW_TRANSFORM 
%token <stringVal> KW_USING KW_CLUSTER KW_DISTRIBUTE KW_SORT KW_UNION KW_LOAD KW_EXPORT KW_IMPORT KW_DATA KW_INPATH
%token <stringVal> KW_IS KW_NULL KW_CREATE KW_EXTERNAL KW_ALTER KW_CHANGE KW_COLUMN KW_FIRST 
%token <stringVal> KW_AFTER KW_DESCRIBE KW_DROP KW_RENAME KW_TO KW_COMMENT KW_BOOLEAN
%token <stringVal> KW_TINYINT KW_SMALLINT KW_INT KW_BIGINT KW_FLOAT KW_DOUBLE KW_DATE
%token <stringVal> KW_DATETIME KW_TIMESTAMP KW_STRING KW_ARRAY KW_STRUCT KW_MAP 
%token <stringVal> KW_UNIONTYPE KW_REDUCE KW_PARTITIONED KW_CLUSTERED KW_SORTED 
%token <stringVal> KW_INTO KW_BUCKETS KW_ROW KW_FORMAT KW_DELIMITED KW_FIELDS 
%token <stringVal> KW_TERMINATED KW_ESCAPED KW_COLLECTION KW_ITEMS KW_KEYS KW_KEY_TYPE
%token <stringVal> KW_LINES KW_STORED KW_FILEFORMAT KW_SEQUENCEFILE KW_TEXTFILE KW_RCFILE
%token <stringVal> KW_INPUTFORMAT KW_OUTPUTFORMAT KW_INPUTDRIVER KW_OUTPUTDRIVER 
%token <stringVal> KW_OFFLINE KW_ENABLE KW_DISABLE KW_READONLY KW_NO_DROP KW_LOCATION
%token <stringVal> KW_TABLESAMPLE KW_BUCKET KW_OUT KW_OF KW_PERCENT KW_CAST KW_ADD 
%token <stringVal> KW_REPLACE KW_COLUMNS KW_RLIKE KW_REGEXP KW_TEMPORARY KW_FUNCTION 
%token <stringVal> KW_EXPLAIN KW_EXTENDED KW_FORMATTED KW_SERDE KW_WITH KW_DEFERRED 
%token <stringVal> KW_SERDEPROPERTIES KW_DBPROPERTIES KW_LIMIT KW_SET KW_TBLPROPERTIES 
%token <stringVal> KW_IDXPROPERTIES KW_VALUE_TYPE KW_ELEM_TYPE	KW_CASE KW_WHEN KW_THEN
%token <stringVal> KW_ELSE KW_END KW_MAPJOIN KW_STREAMTABLE KW_HOLD_DDLTIME 
%token <stringVal> KW_CLUSTERSTATUS KW_UTC KW_UTCTIMESTAMP KW_LONG KW_DELETE KW_PLUS
%token <stringVal> KW_MINUS KW_FETCH KW_INTERSECT KW_VIEW KW_IN KW_DATABASE KW_DATABASES
%token <stringVal> KW_MATERIALIZED KW_SCHEMA KW_SCHEMAS KW_GRANT KW_REVOKE KW_SSL KW_UNDO
%token <stringVal> KW_LOCK KW_LOCKS KW_UNLOCK KW_SHARED KW_EXCLUSIVE KW_PROCEDURE 
%token <stringVal> KW_UNSIGNED KW_WHILE KW_READ KW_READS KW_PURGE KW_RANGE KW_ANALYZE 
%token <stringVal> KW_BEFORE KW_BETWEEN KW_BOTH KW_BINARY KW_CROSS KW_CONTINUE KW_CURSOR 
%token <stringVal> KW_TRIGGER KW_RECORDREADER KW_RECORDWRITER KW_SEMI KW_LATERAL KW_TOUCH 
%token <stringVal> KW_ARCHIVE KW_UNARCHIVE KW_COMPUTE KW_STATISTICS KW_USE KW_OPTION 
%token <stringVal> KW_CONCATENATE KW_SHOW_DATABASE KW_UPDATE KW_RESTRICT KW_CASCADE  

%token <stringVal> COMMA LPAREN RPAREN EQUAL

/*
 * Declare token for operators specify their associativity and precedence
 *
%left UNION INTERSECT MINUS

/* Logical operators *
%left '|'
%left XOR
%left '&'
/* what is that? %right ':=' *
%left '!'

/* Comparison operator *
%left comparisonOps
%right NOT
%left AND OR
%right ISNULL
%nonassoc  LIKE IN  BETWEEN

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
%type <node> keyValueProperty
 
/* statements and their parts */ 
%type <list> stmtList
%type <node> statement explainStatement execStatement

%type <stringVal> explainOptions

%type <node> queryStatementExpression loadStatement exportStatement importStatement ddlStatement

%type <stringVal> loadIsLocal loadIsOverwrite

%type <stringVal> importIsExternal
%type <node> importTableOrPartition tableOrPartition /* tableLocation */
%type <stringVal> importTableLocation

%type <stringVal> ifExists restrictOrCascade ifNotExists orReplace

%type <node> createDatabaseStatement optDbProperties dbProperties dbPropertiesList
%type <stringVal> createDatabaseOrSchema dbLocation

%type <node> switchDatabaseStatement dropDatabaseStatement 

%type <node> databaseComment

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
				hiveParseResult = (Node *) $$;	 
			}
		| stmtList statement ';' 
			{
				RULELOG("stmtlist::stmtList::statement");
				$$ = appendToTailOfList($1, $2);	
				hiveParseResult = (Node *) $$; 
			}
	;

statement:
		explainStatement ';'
		| execStatement ';'
	;

/* explain statement */
explainStatement:
		KW_EXPLAIN explainOptions execStatement
		{
			$$ = NULL;
		}
	;

explainOptions:
		/* empty */ { $$ = NULL; }
		| KW_EXTENDED
		| KW_FORMATTED
	;

/* executable statement */
execStatement:
		queryStatementExpression
    	| loadStatement
    	| exportStatement
    	| importStatement
    	| ddlStatement
    ;
		
/* load statement */
loadStatement: 
		KW_LOAD KW_DATA loadIsLocal KW_INPATH StringLiteral loadIsOverwrite KW_INTO KW_TABLE tableOrPartition
    	{
			$$ = NULL;
		}
    ;
    
loadIsLocal:
		/* empty */ { $$ = NULL; }
		| KW_LOCAL
	;
	
loadIsOverwrite:
		/* empty */ { $$ = NULL; }
		| KW_OVERWRITE
	;			

/* export statement */
exportStatement: 
		KW_EXPORT KW_TABLE tableOrPartition KW_TO StringLiteral
    	{
			$$ = NULL;
		}
    ;

/* import statement */
importStatement: 
		KW_IMPORT importIsExternal KW_TABLE importTableOrPartition KW_FROM StringLiteral importTableLocation
    	{
			$$ = NULL;
		}    	
    ;

importIsExternal:
		/* empty */ { $$ = NULL; }
		| KW_EXTERNAL
	;

importTableOrPartition:
		/* empty */ { $$ = NULL; }
		| tableOrPartition 
	;
		

importTableLocation:
		/* empty */ { $$ = NULL; }
/*		| tableLocation */
	;		

/* DDL statements */
ddlStatement:
		 createDatabaseStatement
    	| switchDatabaseStatement
    	| dropDatabaseStatement /*
    	| createTableStatement
    	| dropTableStatement
    	| alterStatement
    	| descStatement
    	| showStatement
    	| metastoreCheck
    	| createViewStatement
    	| dropViewStatement
    	| createFunctionStatement
    	| createIndexStatement
    	| dropIndexStatement
    	| dropFunctionStatement
    	| analyzeStatement
    	| lockStatement
    	| unlockStatement
    	| createRoleStatement
    	| dropRoleStatement
    	| grantPrivileges
    	| revokePrivileges
    	| showGrants
    	| showRoleGrants
    	| grantRole
    	| revokeRole */
    ;

/* */
ifExists:
		/* emtpy */ { $$ = NULL; }
		| KW_IF KW_EXISTS
    ;

restrictOrCascade:
		/* empty */ { $$ = NULL; }  
		| KW_RESTRICT
    	| KW_CASCADE
    ;

ifNotExists:
    	/* emtpy */ { $$ = NULL; }
		| KW_IF KW_NOT KW_EXISTS
    ;

orReplace:
		/* emtpy */ { $$ = NULL; }
		| KW_OR KW_REPLACE
    ;

/* createDatabase */
createDatabaseStatement:
   		KW_CREATE createDatabaseOrSchema ifNotExists Identifier databaseComment dbLocation optDbProperties
		{ 
			$$ = NULL; 
		}
    ;

createDatabaseOrSchema:
		KW_DATABASE
		| KW_SCHEMA
	;

dbLocation:
		/* empty */ { $$=NULL; }
      	| KW_LOCATION StringLiteral
    ;

optDbProperties:
		/*empty */ { $$ = NULL; }
		| KW_WITH KW_DBPROPERTIES dbProperties { $$ = $3; }
	;
	
dbProperties:
      	LPAREN dbPropertiesList RPAREN { $$ = NIL; }
    ;

dbPropertiesList:
      	keyValueProperty { $$ = singleton($1); }
		| dbPropertiesList COMMA keyValueProperty { $$ = CONCAT_LISTS($1,$3); }
    ;

/* switchDatabaseStatement */
switchDatabaseStatement:
		KW_USE Identifier { $$ = NULL; }
    ;

/* dropDatabaseStatement */
dropDatabaseStatement:
    	KW_DROP createDatabaseOrSchema ifExists Identifier restrictOrCascade { $$ = NULL; }
    ;

databaseComment:
		/* empty */ { $$ = NULL; }
		| KW_COMMENT StringLiteral { $$ = $2; }
    ;

/* tableStatement */
createTableStatement:
		KW_CREATE tableIsExternal KW_TABLE ifNotExists tableName KW_LIKE tableName tableLocation
		{
			$$ = NULL;
		}
		| optTableColumnList
         tableComment
         tablePartition
         tableBuckets
         tableRowFormat
         tableFileFormat
         tableLocation
         tablePropertiesPrefixed
		 optTableAsSelect
		{
			$$ = NULL;
		}
    ;

tableIsExternal:
		/* empty */ { $$ = NULL; }
		| KW_EXTERNAL
	;

optTableColumnList:
		LPAREN columnNameTypeList RPAREN { $$ = NULL; }
	;

optTableAsSelect:
		KW_AS selectStatement { $$ = $2; }
	;

createIndexStatement: 
		KW_CREATE KW_INDEX Identifier KW_ON KW_TABLE tableName LPAREN columnNameList RPAREN KW_AS StringLiteral
      	autoRebuild
      	indexPropertiesPrefixed
      	indexTblName
      	tableRowFormat
      	tableFileFormat
      	tableLocation
      	tablePropertiesPrefixed
      	indexComment
		{
			$$ = NULL;
		}
    ;

indexComment:
		KW_COMMENT StringLiteral { $$ = $2; }
	;

autoRebuild:
		/* empty */ 	{ $$ = NULL; }
    	| KW_WITH KW_DEFERRED KW_REBUILD
    ;

indexTblName:
		/* empty */ 	{ $$ = NULL; } 
		| KW_IN KW_TABLE tableName
    	{
			$$ = $3;
		}
    ;

indexPropertiesPrefixed:
        KW_IDXPROPERTIES! indexProperties
    ;

indexProperties:
		LPAREN indexPropertiesList RPAREN  { $$ = $2; }
    ;

indexPropertiesList:
      	keyValueProperty { $$ = singleton($1); }
		| indexPropertiesList COMMA keyValueProperty { $$ = CONCAT_LISTS($1,$3); }
    ;

dropIndexStatement: 
		KW_DROP KW_INDEX ifExists Identifier KW_ON tableName
    	{
			$$ = NULL;
		}
    ;

dropTableStatement: 
		KW_DROP KW_TABLE ifExists tableName
		{
			$$ = NULL;
		}
    ;

alterStatement: 
		KW_ALTER KW_TABLE alterTableStatementSuffix
		{
			$$ = NULL;
		}
        | KW_ALTER KW_VIEW alterViewStatementSuffix
		{
			$$ = NULL;
		}
        | KW_ALTER KW_INDEX alterIndexStatementSuffix
		{
			$$ = NULL;
		}
        | KW_ALTER KW_DATABASE alterDatabaseStatementSuffix
		{
			$$ = NULL;
		}
    ;

alterTableStatementSuffix: 
		alterStatementSuffixRename
    	| alterStatementSuffixAddCol
    	| alterStatementSuffixRenameCol
    	| alterStatementSuffixDropPartitions
    	| alterStatementSuffixAddPartitions
    	| alterStatementSuffixTouch
    	| alterStatementSuffixArchive
    	| alterStatementSuffixUnArchive
    	| alterStatementSuffixProperties
    	| alterTblPartitionStatement
    	| alterStatementSuffixClusterbySortby
    ;

alterViewStatementSuffix: 
		alterViewSuffixProperties
    	| alterStatementSuffixRename
	    | alterStatementSuffixAddPartitions
		| alterStatementSuffixDropPartitions
    ;

alterIndexStatementSuffix: 
		Identifier KW_ON Identifier partitionSpec KW_REBUILD
    	| Identifier KW_ON Identifier partitionSpec  KW_SET KW_IDXPROPERTIES indexProperties
    ;

alterDatabaseStatementSuffix: 
		alterDatabaseSuffixProperties
    ;

alterDatabaseSuffixProperties: 
		Identifier KW_SET KW_DBPROPERTIES dbProperties
    ;

alterStatementSuffixRename: 
		Identifier KW_RENAME KW_TO Identifier
    ;

alterStatementSuffixAddCol: 
		Identifier addOrReplace KW_COLUMNS LPAREN columnNameTypeList RPAREN
    ;
    
addOrReplace:
		KW_ADD
		| KW_REPLACE
	;

alterStatementSuffixRenameCol:
		Identifier KW_CHANGE alterColOptColumn Identifier Identifier colType optComment alterStatementChangeColPosition
    ;
    
alterColOptColumn:
		/* empty */ { $$ = NULL; }
		| KW_COLUMN
	;

optComment:
		/* empty */ { $$ = NULL; }
		| KW_COMMENT StringLiteral
	;
	
alterStatementChangeColPosition: 
		firstOrAfter Identifier
    ;

firstOrAfter:
		KW_FIRST
		| KW_AFTER
	;

alterStatementSuffixAddPartitions: 
		Identifier KW_ADD ifNotExists partitionSpecLocationList
    ;

partitionSpecLocationList:
		partitionSpecLocation
		| partitionSpecLocationList partitionSpecLocation
	;
	
partitionSpecLocation:
		partitionSpec
		| partitionSpec partitionLocation
	;

alterStatementSuffixTouch: 
		Identifier KW_TOUCH optPartitionSpecList
    ;


optPartitionSpecList:
		/* empty */ { $$ = NULL; }
		| partitionSpecList { $$ = $1; }
	;
	
partitionSpecList:
		partitionSpec
		| partitionSpecList COMMA partitionSpec
	;

alterStatementSuffixArchive: 
		Identifier KW_ARCHIVE optPartitionSpecList
    ;

alterStatementSuffixUnArchive: 
		Identifier KW_UNARCHIVE optPartitionSpecList
    ;

partitionLocation:
      	KW_LOCATION StringLiteral
    ;

alterStatementSuffixDropPartitions: 
		Identifier KW_DROP ifExists partitionSpecList
    ;

alterStatementSuffixProperties:
		Identifier KW_SET KW_TBLPROPERTIES tableProperties
    ;

alterViewSuffixProperties:
	    Identifier KW_SET KW_TBLPROPERTIES tableProperties
    ;

alterStatementSuffixSerdeProperties:
    	KW_SET KW_SERDE StringLiteral optWithSerdeProperties
       | KW_SET KW_SERDEPROPERTIES tableProperties
    ;
    
optWithSerdeProperties:
		KW_WITH KW_SERDEPROPERTIES tableProperties
	;

tablePartitionPrefix:
	  	Identifier optPartitionSpec
	;

alterTblPartitionStatement:
		tablePartitionPrefix alterTblPartitionStatementSuffix
  ;

alterTblPartitionStatementSuffix:
		alterStatementSuffixFileFormat
  		| alterStatementSuffixLocation
  		| alterStatementSuffixProtectMode
  		| alterStatementSuffixMergeFiles
  		| alterStatementSuffixSerdeProperties
		| alterStatementSuffixRenamePart
  	;

alterStatementSuffixFileFormat:
		KW_SET KW_FILEFORMAT fileFormat
	;

alterStatementSuffixLocation:
		KW_SET KW_LOCATION StringLiteral
	;

alterStatementSuffixProtectMode:
    	alterProtectMode
    ;

alterStatementSuffixRenamePart:
    	KW_RENAME KW_TO partitionSpec
    ;

alterStatementSuffixMergeFiles:
		W_CONCATENATE
    ;

alterProtectMode:
		KW_ENABLE alterProtectModeMode
    	| KW_DISABLE alterProtectModeMode
    ;

alterProtectModeMode:
    	KW_OFFLINE
    	| KW_NO_DROP
    	| KW_NO_DROP KW_CASCADE
    	| KW_READONLY
    ;


alterStatementSuffixClusterbySortby:
		tableBuckets
		| Identifier KW_NOT KW_CLUSTERED
	;

fileFormat:
    	KW_SEQUENCEFILE
    	| KW_TEXTFILE
    	| KW_RCFILE
    	| KW_INPUTFORMAT StringLiteral KW_OUTPUTFORMAT StringLiteral optFileFormatDriverSpec
	    | Identifier
    ;

optFileFormatDriverSpec:
		KW_INPUTDRIVER StringLiteral KW_OUTPUTDRIVER StringLiteral
	;
	
tabTypeExpr:
   	Identifier (DOT^ (Identifier | KW_ELEM_TYPE | KW_KEY_TYPE | KW_VALUE_TYPE))*
   ;

partTypeExpr:
    	tabTypeExpr optPartitionSpec
    ;

descStatement:
    describeOrDesc formatedOrExtended partTypeExpr
    | describeOrDesc KW_FUNCTION isExtended descFuncNames
    | describeOrDesc KW_DATABASE isExtended Identifier
    ;

optFormatedOrExtended:
		/* empty */
		| KW_FORMATTED
		| KW_EXTENDED
	;
		
describeOrDesc:
		KW_DESCRIBE
		| KW_DESC
	;
	
analyzeStatement:
    	KW_ANALYZE KW_TABLE tableOrPartition KW_COMPUTE KW_STATISTICS
    ;

showStatement:
    	KW_SHOW databaseOrSchema optLikeShowStatementIdentifier
    	| KW_SHOW KW_TABLES optFromInIdent optLikeSSIOrSSI
    	| KW_SHOW KW_FUNCTIONS optShowStmtIdentifier
    	| KW_SHOW KW_PARTITIONS Identifier optPartitionSpec
    	| KW_SHOW KW_TABLE KW_EXTENDED optFromInIdent KW_LIKE showStmtIdentifier optPartitionSpec
    	| KW_SHOW KW_LOCKS optPartTypeExpr isExtended
    	| KW_SHOW optFormated indexOrIndexes KW_ON showStmtIdentifier optFromInIdent
    ;

indexOrIndexes:
		KW_INDEX
		| KW_INDEXES
	;

optFormated:
		/* empty */
		| KW_FORMATTED
	;

optFromInIdent:
		/* empty */
		| fromInIdent
	;

fromInIdent:
		KW_FROM Identifier
		| KW_IN Identifier
	;
	
optLikeSSIOrSSI:
		optLikeShowStatementIdentifier
		| showStmtIdentifier
	;		

optLikeShowStatementIdentifier:
		/* empty */
		| KW_LIKE showStmtIdentifier
	;
	
lockStatement:
    	KW_LOCK KW_TABLE tableName optPartitionSpec lockMode
    ;

lockMode:
    	KW_SHARED 
		| KW_EXCLUSIVE
    ;

unlockStatement:
		KW_UNLOCK KW_TABLE tableName optPartitionSpec
    ;

createRoleStatement:
		KW_CREATE kwRole Identifier
    ;

dropRoleStatement:
		KW_DROP kwRole Identifier
    ;

grantPrivileges:
		KW_GRANT privilegeList
      	optPrivilegeObject
      	KW_TO principalSpecification
      	optWithOption
    ;

optWithOption:
		/* empty */
		KW_WITH withOption
	;

revokePrivileges:
		KW_REVOKE privilegeList optPrivilegeObject KW_FROM principalSpecification
    ;

grantRole:
		KW_GRANT kwRole identifierList KW_TO principalSpecification
    ;

identifierList:
		Identifier { $$ = singleton($1); }
		| identifierList COMMA Identifier { $$ = CONCAT_LISTS($1,$3); }
	;

revokeRole:
    	KW_REVOKE kwRole identifierList KW_FROM principalSpecification
    ;

showRoleGrants:
		KW_SHOW kwRole KW_GRANT principalName
    ;

showGrants:
		KW_SHOW KW_GRANT principalName optPrivilegeIncludeColObject
    ;

optPrivilegeIncludeColObject:
		/* empty */ { $$ = NULL; }
		| privilegeIncludeColObject
	;

privilegeIncludeColObject:
		KW_ON tableOrDatabase Identifier optColumnNameList optPartitionSpec
    ;

tableOrDatabase:
		KW_TABLE
		| KW_DATABASE
	;


privilegeObject:
		KW_ON tableOrDatabase Identifier optPartitionSpec
    ;

privilegeList:
    	privlegeDef	 { $$ = singleton($1); } 
		| privilegeList COMMA privlegeDef { $$ = CONCAT_LISTS($1,$3); }
    ;

privlegeDef:
		privilegeType optColumnNameList
    ;

privilegeType:
		KW_ALL
    	| KW_ALTER
    	| KW_UPDATE
    	| KW_CREATE
    	| KW_DROP
    	| KW_INDEX 
    	| KW_LOCK
    	| KW_SELECT
    	| KW_SHOW_DATABASE
    ;

principalSpecification:
    	principalName
		| principalSpecification COMMA principalName
    ;

principalName:
    	kwUser Identifier
    	| KW_GROUP Identifier
    	| kwRole Identifier
    ;

withOption:
		KW_GRANT KW_OPTION
    ;

metastoreCheck
		KW_MSCK (repair=KW_REPAIR)? (KW_TABLE table=Identifier partitionSpec? (COMMA partitionSpec)*)?
    -> ^(TOK_MSCK $repair? ($table partitionSpec*)?)
    ;
    
optRepair:
		/* empty */ { $$ = NULL; }
		| KW_REPAIR
	;

createFunctionStatement:
		KW_CREATE KW_TEMPORARY KW_FUNCTION Identifier KW_AS StringLiteral
    ;

dropFunctionStatement:
		KW_DROP KW_TEMPORARY KW_FUNCTION ifExists Identifier
    ;

createViewStatement:
		KW_CREATE orReplace KW_VIEW ifNotExists tableName
        optColumnNameCommentList tableCommen? optViewPartition
        tablePropertiesPrefixed?
        KW_AS
        selectStatement
    -> ^(TOK_CREATEVIEW $name orReplace?
         ifNotExists?
         columnNameCommentList?
         tableComment?
         viewPartition?
         tablePropertiesPrefixed?
         selectStatement
        )
    ;

viewPartition:
		W_PARTITIONED KW_ON LPAREN columnNameList RPAREN
    ;

dropViewStatement:
		KW_DROP KW_VIEW ifExists viewName
    ;

showStmtIdentifier:
		Identifier
    	| StringLiteral
	;

tableComment:
		KW_COMMENT StringLiteral
    ;

tablePartition:
		KW_PARTITIONED KW_BY LPAREN columnNameTypeList RPAREN
    ;

tableBuckets:
		KW_CLUSTERED KW_BY LPAREN columnNameList RPAREN optSortedByOrderColumnList KW_INTO Number KW_BUCKETS
    ;

optSortedByOrderColumnList:
		/* empty */ { $$ = NULL; }
		| KW_SORTED KW_BY LPAREN columnNameOrderList RPAREN
	;

rowFormat:
		rowFormatSerde
    	| rowFormatDelimited
    	| /* empty */
    ;

recordReader
		KW_RECORDREADER StringLiteral
    	| /* empty */
    ;

recordWriter:
		KW_RECORDWRITER StringLiteral
    	| /* empty */
    ;

rowFormatSerde:
		KW_ROW KW_FORMAT KW_SERDE StringLiteral optWithSerdeProperties
    ;

optWithSerdeProperties:
		/* empty */
		| KW_WITH KW_SERDEPROPERTIES tableProperties
	;
	
rowFormatDelimited:
		KW_ROW KW_FORMAT KW_DELIMITED tableRowFormatFieldIdentifier? tableRowFormatCollItemsIdentifier? tableRowFormatMapKeysIdentifier? tableRowFormatLinesIdentifier?
    ;

tableRowFormat:
		rowFormatDelimited
	    | rowFormatSerde
    ;

tablePropertiesPrefixed:
        KW_TBLPROPERTIES tableProperties
    ;

tableProperties
	      LPAREN tablePropertiesList RPAREN
    ;

tablePropertiesList:
		keyValueProperty
		| tablePropertiesList COMMA keyValueProperty
    ;

/* Literals */
keyValueProperty:
      	StringLiteral EQUAL StringLiteral { $$ = createStringKeyValue($1,$3); }
    ;


tableRowFormatFieldIdentifier:
		KW_FIELDS KW_TERMINATED KW_BY StringLiteral optEscapedBy
    ;

optEscapedBy:
		/* empty */ { $$ = NULL; }
		| KW_ESCAPED KW_BY StringLiteral
	;
	
tableRowFormatCollItemsIdentifier:
      	KW_COLLECTION KW_ITEMS KW_TERMINATED KW_BY StringLiteral
    ;

tableRowFormatMapKeysIdentifier:
		KW_MAP KW_KEYS KW_TERMINATED KW_BY StringLiteral
    ;

tableRowFormatLinesIdentifier:
		KW_LINES KW_TERMINATED KW_BY StringLiteral
    ;

tableFileFormat:
      	KW_STORED KW_AS KW_SEQUENCEFILE
      	| KW_STORED KW_AS KW_TEXTFILE
      	| KW_STORED KW_AS KW_RCFILE
      	| KW_STORED KW_AS KW_INPUTFORMAT StringLiteral KW_OUTPUTFORMAT StringLiteral
      	| KW_STORED KW_BY StringLiteral optSerdeProperties 
		| KW_STORED KW_AS genericSpec=Identifier
    ;

optDriver:
		/* empty */ { $$ = NULL; }
		| KW_INPUTDRIVER StringLiteral KW_OUTPUTDRIVER StringLiteral
	;
	
optSerdeProperties:
		/* empty */ { $$ = NULL; }
		| KW_WITH KW_SERDEPROPERTIES tableProperties
	;

tableLocation:
		KW_LOCATION StringLiteral
    ;

columnNameTypeList:
		columnNameType { $$ = singleton($1); }
		| columnNameTypeList COMMA columnNameType { $$ = CONCAT_LISTS($1,$3); }
    ;

columnNameColonTypeList:
		columnNameColonType
		| columnNameColonTypeList COMMA columnNameColonType
    ;

columnNameList:
		columnName { $$ = singleton($1); }
		| columnNameList COMMA columnName  { $$ = CONCAT_LISTS($1,$3); }
    ;

columnName:
		Identifier
    ;

columnNameOrderList:
		columnNameOrder { $$ = singleton($1); }
		| columnNameOrderList COMMA columnNameOrder { $$ = CONCAT_LISTS($1,$3); }
    ;

columnNameOrder:
		Identifier optAscOrDesc
    ;

optAscOrDesc:
		KW_ASC
		| KW_DESC
	;

columnNameCommentList:
		columnNameComment  { $$ = singleton($1); }
		| columnNameCommentList COMMA columnNameComment { $$ = CONCAT_LISTS($1,$3); }
    ;

columnNameComment:
		Identifier optComment
    ;
    
optComment:
		KW_COMMENT StringLiteral
	;

columnRefOrder:
		expression ascOrDesc
    ;

columnNameType:
		Identifier colType optComment
    ;

columnNameColonType:
		Identifier COLON colType optComment
    ;

colType:
		type
    ;

colTypeList:
		colType 
		| colTypeList COMMA colType
    ;

type:
		primitiveType
    	| listType
    	| structType
    	| mapType
    	| unionType
	;

primitiveType:
		KW_TINYINT
    	| KW_SMALLINT 
    	| KW_INT      
    	| KW_BIGINT   
    	| KW_BOOLEAN  
    	| KW_FLOAT    
    	| KW_DOUBLE   
	    | KW_DATE     
	    | KW_DATETIME 
	    | KW_TIMESTAMP
	    | KW_STRING   
	    | KW_BINARY   
    ;

listType:
		KW_ARRAY LESSTHAN type GREATERTHAN
    ;

structType:
		KW_STRUCT LESSTHAN columnNameColonTypeList GREATERTHAN
    ;

mapType:
		KW_MAP LESSTHAN primitiveType COMMA type GREATERTHAN
    ;

unionType:
		KW_UNIONTYPE LESSTHAN colTypeList GREATERTHAN
    ;

queryOperator:
		KW_UNION KW_ALL
    ;

// select statement select ... from ... where ... group by ... order by ...
queryStatementExpression:
		queryStatement
		| setQuery
    ;
    
setQuery:
		queryStatmentExpression queryOperator queryStatmentExpression
	;

queryStatement:
    	fromClause bodyList
    	| regular_body
    ;

regular_body:
   		insertClause
   		selectClause
   		fromClause
   		optWhereClause
   		optGroupByClause
   		optHavingClause
   		optOrderByClause
   		optClusterByClause
   		optDistributeByClause
   		optSortByClause
   		optLimitClause
   		| selectStatement
   ;

selectStatement:
   		selectClause
   		fromClause
   		optwhereClause
   		optgroupByClause
   		opthavingClause
   		optorderByClause
   		optclusterByClause
   		optdistributeByClause
   		optsortByClause
   		optlimitClause
   ;


body:
   		insertClause
		selectClause
   		optwhereClause
   		optgroupByClause
   		opthavingClause
   		optorderByClause
   		optclusterByClause
   		optdistributeByClause
   		optsortByClause
	   | selectClause
   		optwhereClause
   		optgroupByClause
		opthavingClause
   		optorderByClause
   		optclusterByClause
   		optdistributeByClause
   		optsortByClause
   		optlimitClause
   ;

insertClause:
		KW_INSERT KW_OVERWRITE destination
   		| KW_INSERT KW_INTO KW_TABLE tableOrPartition
   ;

destination:
     	KW_LOCAL KW_DIRECTORY StringLiteral
   		| KW_DIRECTORY StringLiteral
   		| KW_TABLE tableOrPartition
   ;

limitClause:
		KW_LIMIT Number
   ;

//----------------------- Rules for parsing selectClause -----------------------------
// select a,b,c ...
selectClause:
		KW_SELECT optHintClause optAllOrDistinct selectList
		| KW_SELECT optHintClause KW_TRANSFORM selectTrfmClause 
	    | trfmClause
	;

optHintClause:
		/* empty */ { $$ = NULL; }
		| hintClause 
	;

optAllOrDistinct:
		KW_ALL
		| KW_DISTINCT
	;

selectList:
		selectItem
		| selectList COMMA selectItem
    ;

selectTrfmClause:
	LPAREN selectExpressionList RPAREN
    inSerde=rowFormat inRec=recordWriter
    KW_USING StringLiteral
    ( KW_AS ((LPAREN (aliasList | columnNameTypeList) RPAREN) | (aliasList | columnNameTypeList)))?
    outSerde=rowFormat outRec=recordReader
    -> ^(TOK_TRANSFORM selectExpressionList $inSerde $inRec StringLiteral $outSerde $outRec aliasList? columnNameTypeList?)
    ;

hintClause
@init { msgs.push("hint clause"); }
@after { msgs.pop(); }
    :
    DIVIDE STAR PLUS hintList STAR DIVIDE -> ^(TOK_HINTLIST hintList)
    ;

hintList
@init { msgs.push("hint list"); }
@after { msgs.pop(); }
    :
    hintItem (COMMA hintItem)* -> hintItem+
    ;

hintItem
@init { msgs.push("hint item"); }
@after { msgs.pop(); }
    :
    hintName (LPAREN hintArgs RPAREN)? -> ^(TOK_HINT hintName hintArgs?)
    ;

hintName
@init { msgs.push("hint name"); }
@after { msgs.pop(); }
    :
    KW_MAPJOIN -> TOK_MAPJOIN
    | KW_STREAMTABLE -> TOK_STREAMTABLE
    | KW_HOLD_DDLTIME -> TOK_HOLD_DDLTIME
    ;

hintArgs
@init { msgs.push("hint arguments"); }
@after { msgs.pop(); }
    :
    hintArgName (COMMA hintArgName)* -> ^(TOK_HINTARGLIST hintArgName+)
    ;

hintArgName
@init { msgs.push("hint argument name"); }
@after { msgs.pop(); }
    :
    Identifier
    ;

selectItem
@init { msgs.push("selection target"); }
@after { msgs.pop(); }
    :
    ( selectExpression  ((KW_AS? Identifier) | (KW_AS LPAREN Identifier (COMMA Identifier)* RPAREN))?) -> ^(TOK_SELEXPR selectExpression Identifier*)
    ;

trfmClause
@init { msgs.push("transform clause"); }
@after { msgs.pop(); }
    :
    (   KW_MAP    selectExpressionList
      | KW_REDUCE selectExpressionList )
    inSerde=rowFormat inRec=recordWriter
    KW_USING StringLiteral
    ( KW_AS ((LPAREN (aliasList | columnNameTypeList) RPAREN) | (aliasList | columnNameTypeList)))?
    outSerde=rowFormat outRec=recordReader
    -> ^(TOK_TRANSFORM selectExpressionList $inSerde $inRec StringLiteral $outSerde $outRec aliasList? columnNameTypeList?)
    ;

selectExpression
@init { msgs.push("select expression"); }
@after { msgs.pop(); }
    :
    expression | tableAllColumns
    ;

selectExpressionList
@init { msgs.push("select expression list"); }
@after { msgs.pop(); }
    :
    selectExpression (COMMA selectExpression)* -> ^(TOK_EXPLIST selectExpression+)
    ;


//-----------------------------------------------------------------------------------

tableAllColumns
    : STAR
        -> ^(TOK_ALLCOLREF)
    | tableName DOT STAR
        -> ^(TOK_ALLCOLREF tableName)
    ;

// (table|column)
tableOrColumn
@init { msgs.push("table or column identifier"); }
@after { msgs.pop(); }
    :
    Identifier -> ^(TOK_TABLE_OR_COL Identifier)
    ;

expressionList
@init { msgs.push("expression list"); }
@after { msgs.pop(); }
    :
    expression (COMMA expression)* -> ^(TOK_EXPLIST expression+)
    ;

aliasList
@init { msgs.push("alias list"); }
@after { msgs.pop(); }
    :
    Identifier (COMMA Identifier)* -> ^(TOK_ALIASLIST Identifier+)
    ;

//----------------------- Rules for parsing fromClause ------------------------------
// from [col1, col2, col3] table1, [col4, col5] table2
fromClause
@init { msgs.push("from clause"); }
@after { msgs.pop(); }
    :
    KW_FROM joinSource -> ^(TOK_FROM joinSource)
    ;

joinSource
@init { msgs.push("join source"); }
@after { msgs.pop(); }
    : fromSource ( joinToken^ fromSource (KW_ON! expression)? )*
    | uniqueJoinToken^ uniqueJoinSource (COMMA! uniqueJoinSource)+
    ;

uniqueJoinSource
@init { msgs.push("join source"); }
@after { msgs.pop(); }
    : KW_PRESERVE? fromSource uniqueJoinExpr
    ;

uniqueJoinExpr
@init { msgs.push("unique join expression list"); }
@after { msgs.pop(); }
    : LPAREN e1+=expression (COMMA e1+=expression)* RPAREN
      -> ^(TOK_EXPLIST $e1*)
    ;

uniqueJoinToken
@init { msgs.push("unique join"); }
@after { msgs.pop(); }
    : KW_UNIQUEJOIN -> TOK_UNIQUEJOIN;

joinToken
@init { msgs.push("join type specifier"); }
@after { msgs.pop(); }
    :
      KW_JOIN                     -> TOK_JOIN
    | kwInner  KW_JOIN            -> TOK_JOIN
    | KW_LEFT  KW_OUTER KW_JOIN   -> TOK_LEFTOUTERJOIN
    | KW_RIGHT KW_OUTER KW_JOIN   -> TOK_RIGHTOUTERJOIN
    | KW_FULL  KW_OUTER KW_JOIN   -> TOK_FULLOUTERJOIN
    | KW_LEFT  KW_SEMI  KW_JOIN   -> TOK_LEFTSEMIJOIN
    ;

lateralView
@init {msgs.push("lateral view"); }
@after {msgs.pop(); }
	:
	KW_LATERAL KW_VIEW function tableAlias KW_AS Identifier (COMMA Identifier)* -> ^(TOK_LATERAL_VIEW ^(TOK_SELECT ^(TOK_SELEXPR function Identifier+ tableAlias)))
	;

tableAlias
@init {msgs.push("table alias"); }
@after {msgs.pop(); }
    :
    Identifier -> ^(TOK_TABALIAS Identifier)
    ;

fromSource
@init { msgs.push("from source"); }
@after { msgs.pop(); }
    :
    (tableSource | subQuerySource) (lateralView^)*
    ;

tableBucketSample
@init { msgs.push("table bucket sample specification"); }
@after { msgs.pop(); }
    :
    KW_TABLESAMPLE LPAREN KW_BUCKET (numerator=Number) KW_OUT KW_OF (denominator=Number) (KW_ON expr+=expression (COMMA expr+=expression)*)? RPAREN -> ^(TOK_TABLEBUCKETSAMPLE $numerator $denominator $expr*)
    ;

splitSample
@init { msgs.push("table split sample specification"); }
@after { msgs.pop(); }
    :
    KW_TABLESAMPLE LPAREN  (numerator=Number) KW_PERCENT RPAREN -> ^(TOK_TABLESPLITSAMPLE $numerator)
    ;

tableSample
@init { msgs.push("table sample specification"); }
@after { msgs.pop(); }
    :
    tableBucketSample |
    splitSample
    ;

tableSource
@init { msgs.push("table source"); }
@after { msgs.pop(); }
    : tabname=tableName (ts=tableSample)? (alias=Identifier)?
    -> ^(TOK_TABREF $tabname $ts? $alias?)
    ;

tableName
@init { msgs.push("table name"); }
@after { msgs.pop(); }
    : (db=Identifier DOT)? tab=Identifier
    -> ^(TOK_TABNAME $db? $tab)
    ;

viewName
@init { msgs.push("view name"); }
@after { msgs.pop(); }
    :
    (db=Identifier DOT)? view=Identifier
    -> ^(TOK_TABNAME $db? $view)
    ;

subQuerySource
@init { msgs.push("subquery source"); }
@after { msgs.pop(); }
    :
    LPAREN queryStatementExpression RPAREN Identifier -> ^(TOK_SUBQUERY queryStatementExpression Identifier)
    ;

//----------------------- Rules for parsing whereClause -----------------------------
// where a=b and ...
whereClause
@init { msgs.push("where clause"); }
@after { msgs.pop(); }
    :
    KW_WHERE searchCondition -> ^(TOK_WHERE searchCondition)
    ;

searchCondition
@init { msgs.push("search condition"); }
@after { msgs.pop(); }
    :
    expression
    ;

//-----------------------------------------------------------------------------------

// group by a,b
groupByClause
@init { msgs.push("group by clause"); }
@after { msgs.pop(); }
    :
    KW_GROUP KW_BY
    groupByExpression
    ( COMMA groupByExpression )*
    -> ^(TOK_GROUPBY groupByExpression+)
    ;

groupByExpression
@init { msgs.push("group by expression"); }
@after { msgs.pop(); }
    :
    expression
    ;

havingClause
@init { msgs.push("having clause"); }
@after { msgs.pop(); }
    :
    KW_HAVING havingCondition -> ^(TOK_HAVING havingCondition)
    ;

havingCondition
@init { msgs.push("having condition"); }
@after { msgs.pop(); }
    :
    expression
    ;

// order by a,b
orderByClause
@init { msgs.push("order by clause"); }
@after { msgs.pop(); }
    :
    KW_ORDER KW_BY
    columnRefOrder
    ( COMMA columnRefOrder)* -> ^(TOK_ORDERBY columnRefOrder+)
    ;

clusterByClause
@init { msgs.push("cluster by clause"); }
@after { msgs.pop(); }
    :
    KW_CLUSTER KW_BY
    expression
    ( COMMA expression )* -> ^(TOK_CLUSTERBY expression+)
    ;

distributeByClause
@init { msgs.push("distribute by clause"); }
@after { msgs.pop(); }
    :
    KW_DISTRIBUTE KW_BY
    expression (COMMA expression)* -> ^(TOK_DISTRIBUTEBY expression+)
    ;

sortByClause
@init { msgs.push("sort by clause"); }
@after { msgs.pop(); }
    :
    KW_SORT KW_BY
    columnRefOrder
    ( COMMA columnRefOrder)* -> ^(TOK_SORTBY columnRefOrder+)
    ;

// fun(par1, par2, par3)
function
@init { msgs.push("function specification"); }
@after { msgs.pop(); }
    :
    functionName
    LPAREN
      (
        (star=STAR)
        | (dist=KW_DISTINCT)? (expression (COMMA expression)*)?
      )
    RPAREN -> {$star != null}? ^(TOK_FUNCTIONSTAR functionName)
           -> {$dist == null}? ^(TOK_FUNCTION functionName (expression+)?)
                            -> ^(TOK_FUNCTIONDI functionName (expression+)?)
    ;

functionName
@init { msgs.push("function name"); }
@after { msgs.pop(); }
    : // Keyword IF is also a function name
    Identifier | KW_IF | KW_ARRAY | KW_MAP | KW_STRUCT | KW_UNIONTYPE
    ;

castExpression
@init { msgs.push("cast expression"); }
@after { msgs.pop(); }
    :
    KW_CAST
    LPAREN
          expression
          KW_AS
          primitiveType
    RPAREN -> ^(TOK_FUNCTION primitiveType expression)
    ;

caseExpression
@init { msgs.push("case expression"); }
@after { msgs.pop(); }
    :
    KW_CASE expression
    (KW_WHEN expression KW_THEN expression)+
    (KW_ELSE expression)?
    KW_END -> ^(TOK_FUNCTION KW_CASE expression*)
    ;

whenExpression
@init { msgs.push("case expression"); }
@after { msgs.pop(); }
    :
    KW_CASE
     ( KW_WHEN expression KW_THEN expression)+
    (KW_ELSE expression)?
    KW_END -> ^(TOK_FUNCTION KW_WHEN expression*)
    ;

constant
@init { msgs.push("constant"); }
@after { msgs.pop(); }
    :
    Number
    | StringLiteral
    | stringLiteralSequence
    | BigintLiteral
    | SmallintLiteral
    | TinyintLiteral
    | charSetStringLiteral
    | booleanValue
    ;

stringLiteralSequence
    :
    StringLiteral StringLiteral+ -> ^(TOK_STRINGLITERALSEQUENCE StringLiteral StringLiteral+)
    ;

charSetStringLiteral
@init { msgs.push("character string literal"); }
@after { msgs.pop(); }
    :
    csName=CharSetName csLiteral=CharSetLiteral -> ^(TOK_CHARSETLITERAL $csName $csLiteral)
    ;

expression
@init { msgs.push("expression specification"); }
@after { msgs.pop(); }
    :
    precedenceOrExpression
    ;

atomExpression
    :
    KW_NULL -> TOK_NULL
    | constant
    | function
    | castExpression
    | caseExpression
    | whenExpression
    | tableOrColumn
    | LPAREN! expression RPAREN!
    ;


precedenceFieldExpression
    :
    atomExpression ((LSQUARE^ expression RSQUARE!) | (DOT^ Identifier))*
    ;

precedenceUnaryOperator
    :
    PLUS | MINUS | TILDE
    ;

nullCondition
    :
    KW_NULL -> ^(TOK_ISNULL)
    | KW_NOT KW_NULL -> ^(TOK_ISNOTNULL)
    ;



/* dummy for now */
queryStatementExpression: { $$ = NULL; } 
tableOrPartition: { $$ = NULL; }

/*
// starting rule
execStatement
@init { msgs.push("statement"); }
@after { msgs.pop(); }
    : queryStatementExpression
    | loadStatement
    | exportStatement
    | importStatement
    | ddlStatement
    ;

loadStatement
@init { msgs.push("load statement"); }
@after { msgs.pop(); }
    : KW_LOAD KW_DATA (islocal=KW_LOCAL)? KW_INPATH (path=StringLiteral) (isoverwrite=KW_OVERWRITE)? KW_INTO KW_TABLE (tab=tableOrPartition)
    -> ^(TOK_LOAD $path $tab $islocal? $isoverwrite?)
    ;

exportStatement
@init { msgs.push("export statement"); }
@after { msgs.pop(); }
    : KW_EXPORT KW_TABLE (tab=tableOrPartition) KW_TO (path=StringLiteral)
    -> ^(TOK_EXPORT $tab $path)
    ;

importStatement
@init { msgs.push("import statement"); }
@after { msgs.pop(); }
	: KW_IMPORT ((ext=KW_EXTERNAL)? KW_TABLE (tab=tableOrPartition))? KW_FROM (path=StringLiteral) tableLocation?
    -> ^(TOK_IMPORT $path $tab? $ext? tableLocation?)
    ;

ddlStatement
@init { msgs.push("ddl statement"); }
@after { msgs.pop(); }
    : createDatabaseStatement
    | switchDatabaseStatement
    | dropDatabaseStatement
    | createTableStatement
    | dropTableStatement
    | alterStatement
    | descStatement
    | showStatement
    | metastoreCheck
    | createViewStatement
    | dropViewStatement
    | createFunctionStatement
    | createIndexStatement
    | dropIndexStatement
    | dropFunctionStatement
    | analyzeStatement
    | lockStatement
    | unlockStatement
    | createRoleStatement
    | dropRoleStatement
    | grantPrivileges
    | revokePrivileges
    | showGrants
    | showRoleGrants
    | grantRole
    | revokeRole
    ;

ifExists
@init { msgs.push("if exists clause"); }
@after { msgs.pop(); }
    : KW_IF KW_EXISTS
    -> ^(TOK_IFEXISTS)
    ;

restrictOrCascade
@init { msgs.push("restrict or cascade clause"); }
@after { msgs.pop(); }
    : KW_RESTRICT
    -> ^(TOK_RESTRICT)
    | KW_CASCADE
    -> ^(TOK_CASCADE)
    ;

ifNotExists
@init { msgs.push("if not exists clause"); }
@after { msgs.pop(); }
    : KW_IF KW_NOT KW_EXISTS
    -> ^(TOK_IFNOTEXISTS)
    ;

orReplace
@init { msgs.push("or replace clause"); }
@after { msgs.pop(); }
    : KW_OR KW_REPLACE
    -> ^(TOK_ORREPLACE)
    ;


createDatabaseStatement
@init { msgs.push("create database statement"); }
@after { msgs.pop(); }
    : KW_CREATE (KW_DATABASE|KW_SCHEMA)
        ifNotExists?
        name=Identifier
        databaseComment?
        dbLocation?
        (KW_WITH KW_DBPROPERTIES dbprops=dbProperties)?
    -> ^(TOK_CREATEDATABASE $name ifNotExists? dbLocation? databaseComment? $dbprops?)
    ;

dbLocation
@init { msgs.push("database location specification"); }
@after { msgs.pop(); }
    :
      KW_LOCATION locn=StringLiteral -> ^(TOK_DATABASELOCATION $locn)
    ;

dbProperties
@init { msgs.push("dbproperties"); }
@after { msgs.pop(); }
    :
      LPAREN dbPropertiesList RPAREN -> ^(TOK_DATABASEPROPERTIES dbPropertiesList)
    ;

dbPropertiesList
@init { msgs.push("database properties list"); }
@after { msgs.pop(); }
    :
      keyValueProperty (COMMA keyValueProperty)* -> ^(TOK_DBPROPLIST keyValueProperty+)
    ;


switchDatabaseStatement
@init { msgs.push("switch database statement"); }
@after { msgs.pop(); }
    : KW_USE Identifier
    -> ^(TOK_SWITCHDATABASE Identifier)
    ;

dropDatabaseStatement
@init { msgs.push("drop database statement"); }
@after { msgs.pop(); }
    : KW_DROP (KW_DATABASE|KW_SCHEMA) ifExists? Identifier restrictOrCascade?
    -> ^(TOK_DROPDATABASE Identifier ifExists? restrictOrCascade?)
    ;

databaseComment
@init { msgs.push("database's comment"); }
@after { msgs.pop(); }
    : KW_COMMENT comment=StringLiteral
    -> ^(TOK_DATABASECOMMENT $comment)
    ;

createTableStatement
@init { msgs.push("create table statement"); }
@after { msgs.pop(); }
    : KW_CREATE (ext=KW_EXTERNAL)? KW_TABLE ifNotExists? name=tableName
      (  like=KW_LIKE likeName=tableName
         tableLocation?
       | (LPAREN columnNameTypeList RPAREN)?
         tableComment?
         tablePartition?
         tableBuckets?
         tableRowFormat?
         tableFileFormat?
         tableLocation?
         tablePropertiesPrefixed?
         (KW_AS selectStatement)?
      )
    -> ^(TOK_CREATETABLE $name $ext? ifNotExists?
         ^(TOK_LIKETABLE $likeName?)
         columnNameTypeList?
         tableComment?
         tablePartition?
         tableBuckets?
         tableRowFormat?
         tableFileFormat?
         tableLocation?
         tablePropertiesPrefixed?
         selectStatement?
        )
    ;

createIndexStatement
@init { msgs.push("create index statement");}
@after {msgs.pop();}
    : KW_CREATE KW_INDEX indexName=Identifier
      KW_ON KW_TABLE tab=tableName LPAREN indexedCols=columnNameList RPAREN
      KW_AS typeName=StringLiteral
      autoRebuild?
      indexPropertiesPrefixed?
      indexTblName?
      tableRowFormat?
      tableFileFormat?
      tableLocation?
      tablePropertiesPrefixed?
      indexComment?
    ->^(TOK_CREATEINDEX $indexName $typeName $tab $indexedCols
        autoRebuild?
        indexPropertiesPrefixed?
        indexTblName?
        tableRowFormat?
        tableFileFormat?
        tableLocation?
        tablePropertiesPrefixed?
        indexComment?)
    ;

indexComment
@init { msgs.push("comment on an index");}
@after {msgs.pop();}
        :
                KW_COMMENT comment=StringLiteral  -> ^(TOK_INDEXCOMMENT $comment)
        ;

autoRebuild
@init { msgs.push("auto rebuild index");}
@after {msgs.pop();}
    : KW_WITH KW_DEFERRED KW_REBUILD
    ->^(TOK_DEFERRED_REBUILDINDEX)
    ;

indexTblName
@init { msgs.push("index table name");}
@after {msgs.pop();}
    : KW_IN KW_TABLE indexTbl=tableName
    ->^(TOK_CREATEINDEX_INDEXTBLNAME $indexTbl)
    ;

indexPropertiesPrefixed
@init { msgs.push("table properties with prefix"); }
@after { msgs.pop(); }
    :
        KW_IDXPROPERTIES! indexProperties
    ;

indexProperties
@init { msgs.push("index properties"); }
@after { msgs.pop(); }
    :
      LPAREN indexPropertiesList RPAREN -> ^(TOK_INDEXPROPERTIES indexPropertiesList)
    ;

indexPropertiesList
@init { msgs.push("index properties list"); }
@after { msgs.pop(); }
    :
      keyValueProperty (COMMA keyValueProperty)* -> ^(TOK_INDEXPROPLIST keyValueProperty+)
    ;

dropIndexStatement
@init { msgs.push("drop index statement");}
@after {msgs.pop();}
    : KW_DROP KW_INDEX ifExists? indexName=Identifier KW_ON tab=tableName
    ->^(TOK_DROPINDEX $indexName $tab ifExists?)
    ;

dropTableStatement
@init { msgs.push("drop statement"); }
@after { msgs.pop(); }
    : KW_DROP KW_TABLE ifExists? tableName -> ^(TOK_DROPTABLE tableName ifExists?)
    ;

alterStatement
@init { msgs.push("alter statement"); }
@after { msgs.pop(); }
    : KW_ALTER!
        (
            KW_TABLE! alterTableStatementSuffix
        |
            KW_VIEW! alterViewStatementSuffix
        |
            KW_INDEX! alterIndexStatementSuffix
        |
            KW_DATABASE! alterDatabaseStatementSuffix
        )
    ;

alterTableStatementSuffix
@init { msgs.push("alter table statement"); }
@after { msgs.pop(); }
    : alterStatementSuffixRename
    | alterStatementSuffixAddCol
    | alterStatementSuffixRenameCol
    | alterStatementSuffixDropPartitions
    | alterStatementSuffixAddPartitions
    | alterStatementSuffixTouch
    | alterStatementSuffixArchive
    | alterStatementSuffixUnArchive
    | alterStatementSuffixProperties
    | alterTblPartitionStatement
    | alterStatementSuffixClusterbySortby
    ;

alterViewStatementSuffix
@init { msgs.push("alter view statement"); }
@after { msgs.pop(); }
    : alterViewSuffixProperties
    | alterStatementSuffixRename
        -> ^(TOK_ALTERVIEW_RENAME alterStatementSuffixRename)
    | alterStatementSuffixAddPartitions
        -> ^(TOK_ALTERVIEW_ADDPARTS alterStatementSuffixAddPartitions)
    | alterStatementSuffixDropPartitions
        -> ^(TOK_ALTERVIEW_DROPPARTS alterStatementSuffixDropPartitions)
    ;

alterIndexStatementSuffix
@init { msgs.push("alter index statement"); }
@after { msgs.pop(); }
    : indexName=Identifier
      (KW_ON tableNameId=Identifier)
      partitionSpec?
    (
      KW_REBUILD
      ->^(TOK_ALTERINDEX_REBUILD $tableNameId $indexName partitionSpec?)
    |
      KW_SET KW_IDXPROPERTIES
      indexProperties
      ->^(TOK_ALTERINDEX_PROPERTIES $tableNameId $indexName indexProperties)
    )
    ;

alterDatabaseStatementSuffix
@init { msgs.push("alter database statement"); }
@after { msgs.pop(); }
    : alterDatabaseSuffixProperties
    ;

alterDatabaseSuffixProperties
@init { msgs.push("alter database properties statement"); }
@after { msgs.pop(); }
    : name=Identifier KW_SET KW_DBPROPERTIES dbProperties
    -> ^(TOK_ALTERDATABASE_PROPERTIES $name dbProperties)
    ;

alterStatementSuffixRename
@init { msgs.push("rename statement"); }
@after { msgs.pop(); }
    : oldName=Identifier KW_RENAME KW_TO newName=Identifier
    -> ^(TOK_ALTERTABLE_RENAME $oldName $newName)
    ;

alterStatementSuffixAddCol
@init { msgs.push("add column statement"); }
@after { msgs.pop(); }
    : Identifier (add=KW_ADD | replace=KW_REPLACE) KW_COLUMNS LPAREN columnNameTypeList RPAREN
    -> {$add != null}? ^(TOK_ALTERTABLE_ADDCOLS Identifier columnNameTypeList)
    ->                 ^(TOK_ALTERTABLE_REPLACECOLS Identifier columnNameTypeList)
    ;

alterStatementSuffixRenameCol
@init { msgs.push("rename column name"); }
@after { msgs.pop(); }
    : Identifier KW_CHANGE KW_COLUMN? oldName=Identifier newName=Identifier colType (KW_COMMENT comment=StringLiteral)? alterStatementChangeColPosition?
    ->^(TOK_ALTERTABLE_RENAMECOL Identifier $oldName $newName colType $comment? alterStatementChangeColPosition?)
    ;

alterStatementChangeColPosition
    : first=KW_FIRST|KW_AFTER afterCol=Identifier
    ->{$first != null}? ^(TOK_ALTERTABLE_CHANGECOL_AFTER_POSITION )
    -> ^(TOK_ALTERTABLE_CHANGECOL_AFTER_POSITION $afterCol)
    ;

alterStatementSuffixAddPartitions
@init { msgs.push("add partition statement"); }
@after { msgs.pop(); }
    : Identifier KW_ADD ifNotExists? partitionSpec partitionLocation? (partitionSpec partitionLocation?)*
    -> ^(TOK_ALTERTABLE_ADDPARTS Identifier ifNotExists? (partitionSpec partitionLocation?)+)
    ;

alterStatementSuffixTouch
@init { msgs.push("touch statement"); }
@after { msgs.pop(); }
    : Identifier KW_TOUCH (partitionSpec)*
    -> ^(TOK_ALTERTABLE_TOUCH Identifier (partitionSpec)*)
    ;

alterStatementSuffixArchive
@init { msgs.push("archive statement"); }
@after { msgs.pop(); }
    : Identifier KW_ARCHIVE (partitionSpec)*
    -> ^(TOK_ALTERTABLE_ARCHIVE Identifier (partitionSpec)*)
    ;

alterStatementSuffixUnArchive
@init { msgs.push("unarchive statement"); }
@after { msgs.pop(); }
    : Identifier KW_UNARCHIVE (partitionSpec)*
    -> ^(TOK_ALTERTABLE_UNARCHIVE Identifier (partitionSpec)*)
    ;

partitionLocation
@init { msgs.push("partition location"); }
@after { msgs.pop(); }
    :
      KW_LOCATION locn=StringLiteral -> ^(TOK_PARTITIONLOCATION $locn)
    ;

alterStatementSuffixDropPartitions
@init { msgs.push("drop partition statement"); }
@after { msgs.pop(); }
    : Identifier KW_DROP ifExists? partitionSpec (COMMA partitionSpec)*
    -> ^(TOK_ALTERTABLE_DROPPARTS Identifier partitionSpec+ ifExists?)
    ;

alterStatementSuffixProperties
@init { msgs.push("alter properties statement"); }
@after { msgs.pop(); }
    : name=Identifier KW_SET KW_TBLPROPERTIES tableProperties
    -> ^(TOK_ALTERTABLE_PROPERTIES $name tableProperties)
    ;

alterViewSuffixProperties
@init { msgs.push("alter view properties statement"); }
@after { msgs.pop(); }
    : name=Identifier KW_SET KW_TBLPROPERTIES tableProperties
    -> ^(TOK_ALTERVIEW_PROPERTIES $name tableProperties)
    ;

alterStatementSuffixSerdeProperties
@init { msgs.push("alter serdes statement"); }
@after { msgs.pop(); }
    : KW_SET KW_SERDE serdeName=StringLiteral (KW_WITH KW_SERDEPROPERTIES tableProperties)?
    -> ^(TOK_ALTERTABLE_SERIALIZER $serdeName tableProperties?)
    | KW_SET KW_SERDEPROPERTIES tableProperties
    -> ^(TOK_ALTERTABLE_SERDEPROPERTIES tableProperties)
    ;

tablePartitionPrefix
@init {msgs.push("table partition prefix");}
@after {msgs.pop();}
  :name=Identifier partitionSpec?
  ->^(TOK_TABLE_PARTITION $name partitionSpec?)
  ;

alterTblPartitionStatement
@init {msgs.push("alter table partition statement");}
@after {msgs.pop();}
  :  tablePartitionPrefix alterTblPartitionStatementSuffix
  -> ^(TOK_ALTERTABLE_PARTITION tablePartitionPrefix alterTblPartitionStatementSuffix)
  ;

alterTblPartitionStatementSuffix
@init {msgs.push("alter table partition statement suffix");}
@after {msgs.pop();}
  : alterStatementSuffixFileFormat
  | alterStatementSuffixLocation
  | alterStatementSuffixProtectMode
  | alterStatementSuffixMergeFiles
  | alterStatementSuffixSerdeProperties
  | alterStatementSuffixRenamePart
  ;

alterStatementSuffixFileFormat
@init {msgs.push("alter fileformat statement"); }
@after {msgs.pop();}
	: KW_SET KW_FILEFORMAT fileFormat
	-> ^(TOK_ALTERTABLE_FILEFORMAT fileFormat)
	;

alterStatementSuffixLocation
@init {msgs.push("alter location");}
@after {msgs.pop();}
  : KW_SET KW_LOCATION newLoc=StringLiteral
  -> ^(TOK_ALTERTABLE_LOCATION $newLoc)
  ;

alterStatementSuffixProtectMode
@init { msgs.push("alter partition protect mode statement"); }
@after { msgs.pop(); }
    : alterProtectMode
    -> ^(TOK_ALTERTABLE_ALTERPARTS_PROTECTMODE alterProtectMode)
    ;

alterStatementSuffixRenamePart
@init { msgs.push("alter table rename partition statement"); }
@after { msgs.pop(); }
    : KW_RENAME KW_TO partitionSpec
    ->^(TOK_ALTERTABLE_RENAMEPART partitionSpec)
    ;

alterStatementSuffixMergeFiles
@init { msgs.push(""); }
@after { msgs.pop(); }
    : KW_CONCATENATE
    -> ^(TOK_ALTERTABLE_ALTERPARTS_MERGEFILES)
    ;

alterProtectMode
@init { msgs.push("protect mode specification enable"); }
@after { msgs.pop(); }
    : KW_ENABLE alterProtectModeMode  -> ^(TOK_ENABLE alterProtectModeMode)
    | KW_DISABLE alterProtectModeMode  -> ^(TOK_DISABLE alterProtectModeMode)
    ;

alterProtectModeMode
@init { msgs.push("protect mode specification enable"); }
@after { msgs.pop(); }
    : KW_OFFLINE  -> ^(TOK_OFFLINE)
    | KW_NO_DROP KW_CASCADE? -> ^(TOK_NO_DROP KW_CASCADE?)
    | KW_READONLY  -> ^(TOK_READONLY)
    ;


alterStatementSuffixClusterbySortby
@init {msgs.push("alter cluster by sort by statement");}
@after{msgs.pop();}
	:name=Identifier tableBuckets
	->^(TOK_ALTERTABLE_CLUSTER_SORT $name tableBuckets)
	|
	name=Identifier KW_NOT KW_CLUSTERED
	->^(TOK_ALTERTABLE_CLUSTER_SORT $name)
	;

fileFormat
@init { msgs.push("file format specification"); }
@after { msgs.pop(); }
    : KW_SEQUENCEFILE  -> ^(TOK_TBLSEQUENCEFILE)
    | KW_TEXTFILE  -> ^(TOK_TBLTEXTFILE)
    | KW_RCFILE  -> ^(TOK_TBLRCFILE)
    | KW_INPUTFORMAT inFmt=StringLiteral KW_OUTPUTFORMAT outFmt=StringLiteral (KW_INPUTDRIVER inDriver=StringLiteral KW_OUTPUTDRIVER outDriver=StringLiteral)?
      -> ^(TOK_TABLEFILEFORMAT $inFmt $outFmt $inDriver? $outDriver?)
    | genericSpec=Identifier -> ^(TOK_FILEFORMAT_GENERIC $genericSpec)
    ;

tabTypeExpr
@init { msgs.push("specifying table types"); }
@after { msgs.pop(); }

   : Identifier (DOT^ (Identifier | KW_ELEM_TYPE | KW_KEY_TYPE | KW_VALUE_TYPE))*
   ;

partTypeExpr
@init { msgs.push("specifying table partitions"); }
@after { msgs.pop(); }
    :  tabTypeExpr partitionSpec? -> ^(TOK_TABTYPE tabTypeExpr partitionSpec?)
    ;

descStatement
@init { msgs.push("describe statement"); }
@after { msgs.pop(); }
    : (KW_DESCRIBE|KW_DESC) (descOptions=KW_FORMATTED|descOptions=KW_EXTENDED)? (parttype=partTypeExpr) -> ^(TOK_DESCTABLE $parttype $descOptions?)
    | (KW_DESCRIBE|KW_DESC) KW_FUNCTION KW_EXTENDED? (name=descFuncNames) -> ^(TOK_DESCFUNCTION $name KW_EXTENDED?)
    | (KW_DESCRIBE|KW_DESC) KW_DATABASE KW_EXTENDED? (dbName=Identifier) -> ^(TOK_DESCDATABASE $dbName KW_EXTENDED?)
    ;

analyzeStatement
@init { msgs.push("analyze statement"); }
@after { msgs.pop(); }
    : KW_ANALYZE KW_TABLE (parttype=tableOrPartition) KW_COMPUTE KW_STATISTICS -> ^(TOK_ANALYZE $parttype)
    ;

showStatement
@init { msgs.push("show statement"); }
@after { msgs.pop(); }
    : KW_SHOW (KW_DATABASES|KW_SCHEMAS) (KW_LIKE showStmtIdentifier)? -> ^(TOK_SHOWDATABASES showStmtIdentifier?)
    | KW_SHOW KW_TABLES ((KW_FROM|KW_IN) db_name=Identifier)? (KW_LIKE showStmtIdentifier|showStmtIdentifier)?  -> ^(TOK_SHOWTABLES (TOK_FROM $db_name)? showStmtIdentifier?)
    | KW_SHOW KW_FUNCTIONS showStmtIdentifier?  -> ^(TOK_SHOWFUNCTIONS showStmtIdentifier?)
    | KW_SHOW KW_PARTITIONS Identifier partitionSpec? -> ^(TOK_SHOWPARTITIONS Identifier partitionSpec?)
    | KW_SHOW KW_TABLE KW_EXTENDED ((KW_FROM|KW_IN) db_name=Identifier)? KW_LIKE showStmtIdentifier partitionSpec?
    -> ^(TOK_SHOW_TABLESTATUS showStmtIdentifier $db_name? partitionSpec?)
    | KW_SHOW KW_LOCKS (parttype=partTypeExpr)? (isExtended=KW_EXTENDED)? -> ^(TOK_SHOWLOCKS $parttype? $isExtended?)
    | KW_SHOW (showOptions=KW_FORMATTED)? (KW_INDEX|KW_INDEXES) KW_ON showStmtIdentifier ((KW_FROM|KW_IN) db_name=Identifier)?
    -> ^(TOK_SHOWINDEXES showStmtIdentifier $showOptions? $db_name?)
    ;

lockStatement
@init { msgs.push("lock statement"); }
@after { msgs.pop(); }
    : KW_LOCK KW_TABLE tableName partitionSpec? lockMode -> ^(TOK_LOCKTABLE tableName lockMode partitionSpec?)
    ;

lockMode
@init { msgs.push("lock mode"); }
@after { msgs.pop(); }
    : KW_SHARED | KW_EXCLUSIVE
    ;

unlockStatement
@init { msgs.push("unlock statement"); }
@after { msgs.pop(); }
    : KW_UNLOCK KW_TABLE tableName partitionSpec?  -> ^(TOK_UNLOCKTABLE tableName partitionSpec?)
    ;

createRoleStatement
@init { msgs.push("create role"); }
@after { msgs.pop(); }
    : KW_CREATE kwRole roleName=Identifier
    -> ^(TOK_CREATEROLE $roleName)
    ;

dropRoleStatement
@init {msgs.push("drop role");}
@after {msgs.pop();}
    : KW_DROP kwRole roleName=Identifier
    -> ^(TOK_DROPROLE $roleName)
    ;

grantPrivileges
@init {msgs.push("grant privileges");}
@after {msgs.pop();}
    : KW_GRANT privList=privilegeList
      privilegeObject?
      KW_TO principalSpecification
      (KW_WITH withOption)?
    -> ^(TOK_GRANT $privList principalSpecification privilegeObject? withOption?)
    ;

revokePrivileges
@init {msgs.push("revoke privileges");}
@afer {msgs.pop();}
    : KW_REVOKE privilegeList privilegeObject? KW_FROM principalSpecification
    -> ^(TOK_REVOKE privilegeList principalSpecification privilegeObject?)
    ;

grantRole
@init {msgs.push("grant role");}
@after {msgs.pop();}
    : KW_GRANT kwRole Identifier (COMMA Identifier)* KW_TO principalSpecification
    -> ^(TOK_GRANT_ROLE principalSpecification Identifier+)
    ;

revokeRole
@init {msgs.push("revoke role");}
@after {msgs.pop();}
    : KW_REVOKE kwRole Identifier (COMMA Identifier)* KW_FROM principalSpecification
    -> ^(TOK_REVOKE_ROLE principalSpecification Identifier+)
    ;

showRoleGrants
@init {msgs.push("show role grants");}
@after {msgs.pop();}
    : KW_SHOW kwRole KW_GRANT principalName
    -> ^(TOK_SHOW_ROLE_GRANT principalName)
    ;

showGrants
@init {msgs.push("show grants");}
@after {msgs.pop();}
    : KW_SHOW KW_GRANT principalName privilegeIncludeColObject?
    -> ^(TOK_SHOW_GRANT principalName privilegeIncludeColObject?)
    ;

privilegeIncludeColObject
@init {msgs.push("privilege object including columns");}
@after {msgs.pop();}
    : KW_ON (table=KW_TABLE|KW_DATABASE) Identifier (LPAREN cols=columnNameList RPAREN)? partitionSpec?
    -> ^(TOK_PRIV_OBJECT_COL Identifier $table? $cols? partitionSpec?)
    ;

privilegeObject
@init {msgs.push("privilege subject");}
@after {msgs.pop();}
    : KW_ON (table=KW_TABLE|KW_DATABASE) Identifier partitionSpec?
    -> ^(TOK_PRIV_OBJECT Identifier $table? partitionSpec?)
    ;

privilegeList
@init {msgs.push("grant privilege list");}
@after {msgs.pop();}
    : privlegeDef (COMMA privlegeDef)*
    -> ^(TOK_PRIVILEGE_LIST privlegeDef+)
    ;

privlegeDef
@init {msgs.push("grant privilege");}
@after {msgs.pop();}
    : privilegeType (LPAREN cols=columnNameList RPAREN)?
    -> ^(TOK_PRIVILEGE privilegeType $cols?)
    ;

privilegeType
@init {msgs.push("privilege type");}
@after {msgs.pop();}
    : KW_ALL -> ^(TOK_PRIV_ALL)
    | KW_ALTER -> ^(TOK_PRIV_ALTER_METADATA)
    | KW_UPDATE -> ^(TOK_PRIV_ALTER_DATA)
    | KW_CREATE -> ^(TOK_PRIV_CREATE)
    | KW_DROP -> ^(TOK_PRIV_DROP)
    | KW_INDEX -> ^(TOK_PRIV_INDEX)
    | KW_LOCK -> ^(TOK_PRIV_LOCK)
    | KW_SELECT -> ^(TOK_PRIV_SELECT)
    | KW_SHOW_DATABASE -> ^(TOK_PRIV_SHOW_DATABASE)
    ;

principalSpecification
@init { msgs.push("user/group/role name list"); }
@after { msgs.pop(); }
    : principalName (COMMA principalName)* -> ^(TOK_PRINCIPAL_NAME principalName+)
    ;

principalName
@init {msgs.push("user|group|role name");}
@after {msgs.pop();}
    : kwUser Identifier -> ^(TOK_USER Identifier)
    | KW_GROUP Identifier -> ^(TOK_GROUP Identifier)
    | kwRole Identifier -> ^(TOK_ROLE Identifier)
    ;

withOption
@init {msgs.push("grant with option");}
@after {msgs.pop();}
    : KW_GRANT KW_OPTION
    -> ^(TOK_GRANT_WITH_OPTION)
    ;

metastoreCheck
@init { msgs.push("metastore check statement"); }
@after { msgs.pop(); }
    : KW_MSCK (repair=KW_REPAIR)? (KW_TABLE table=Identifier partitionSpec? (COMMA partitionSpec)*)?
    -> ^(TOK_MSCK $repair? ($table partitionSpec*)?)
    ;

createFunctionStatement
@init { msgs.push("create function statement"); }
@after { msgs.pop(); }
    : KW_CREATE KW_TEMPORARY KW_FUNCTION Identifier KW_AS StringLiteral
    -> ^(TOK_CREATEFUNCTION Identifier StringLiteral)
    ;

dropFunctionStatement
@init { msgs.push("drop temporary function statement"); }
@after { msgs.pop(); }
    : KW_DROP KW_TEMPORARY KW_FUNCTION ifExists? Identifier
    -> ^(TOK_DROPFUNCTION Identifier ifExists?)
    ;

createViewStatement
@init {
    msgs.push("create view statement");
}
@after { msgs.pop(); }
    : KW_CREATE (orReplace)? KW_VIEW (ifNotExists)? name=tableName
        (LPAREN columnNameCommentList RPAREN)? tableComment? viewPartition?
        tablePropertiesPrefixed?
        KW_AS
        selectStatement
    -> ^(TOK_CREATEVIEW $name orReplace?
         ifNotExists?
         columnNameCommentList?
         tableComment?
         viewPartition?
         tablePropertiesPrefixed?
         selectStatement
        )
    ;

viewPartition
@init { msgs.push("view partition specification"); }
@after { msgs.pop(); }
    : KW_PARTITIONED KW_ON LPAREN columnNameList RPAREN
    -> ^(TOK_VIEWPARTCOLS columnNameList)
    ;

dropViewStatement
@init { msgs.push("drop view statement"); }
@after { msgs.pop(); }
    : KW_DROP KW_VIEW ifExists? viewName -> ^(TOK_DROPVIEW viewName ifExists?)
    ;

showStmtIdentifier
@init { msgs.push("Identifier for show statement"); }
@after { msgs.pop(); }
    : Identifier
    | StringLiteral
    ;

tableComment
@init { msgs.push("table's comment"); }
@after { msgs.pop(); }
    :
      KW_COMMENT comment=StringLiteral  -> ^(TOK_TABLECOMMENT $comment)
    ;

tablePartition
@init { msgs.push("table partition specification"); }
@after { msgs.pop(); }
    : KW_PARTITIONED KW_BY LPAREN columnNameTypeList RPAREN
    -> ^(TOK_TABLEPARTCOLS columnNameTypeList)
    ;

tableBuckets
@init { msgs.push("table buckets specification"); }
@after { msgs.pop(); }
    :
      KW_CLUSTERED KW_BY LPAREN bucketCols=columnNameList RPAREN (KW_SORTED KW_BY LPAREN sortCols=columnNameOrderList RPAREN)? KW_INTO num=Number KW_BUCKETS
    -> ^(TOK_TABLEBUCKETS $bucketCols $sortCols? $num)
    ;

rowFormat
@init { msgs.push("serde specification"); }
@after { msgs.pop(); }
    : rowFormatSerde -> ^(TOK_SERDE rowFormatSerde)
    | rowFormatDelimited -> ^(TOK_SERDE rowFormatDelimited)
    |   -> ^(TOK_SERDE)
    ;

recordReader
@init { msgs.push("record reader specification"); }
@after { msgs.pop(); }
    : KW_RECORDREADER StringLiteral -> ^(TOK_RECORDREADER StringLiteral)
    |   -> ^(TOK_RECORDREADER)
    ;

recordWriter
@init { msgs.push("record writer specification"); }
@after { msgs.pop(); }
    : KW_RECORDWRITER StringLiteral -> ^(TOK_RECORDWRITER StringLiteral)
    |   -> ^(TOK_RECORDWRITER)
    ;

rowFormatSerde
@init { msgs.push("serde format specification"); }
@after { msgs.pop(); }
    : KW_ROW KW_FORMAT KW_SERDE name=StringLiteral (KW_WITH KW_SERDEPROPERTIES serdeprops=tableProperties)?
    -> ^(TOK_SERDENAME $name $serdeprops?)
    ;

rowFormatDelimited
@init { msgs.push("serde properties specification"); }
@after { msgs.pop(); }
    :
      KW_ROW KW_FORMAT KW_DELIMITED tableRowFormatFieldIdentifier? tableRowFormatCollItemsIdentifier? tableRowFormatMapKeysIdentifier? tableRowFormatLinesIdentifier?
    -> ^(TOK_SERDEPROPS tableRowFormatFieldIdentifier? tableRowFormatCollItemsIdentifier? tableRowFormatMapKeysIdentifier? tableRowFormatLinesIdentifier?)
    ;

tableRowFormat
@init { msgs.push("table row format specification"); }
@after { msgs.pop(); }
    :
      rowFormatDelimited
    -> ^(TOK_TABLEROWFORMAT rowFormatDelimited)
    | rowFormatSerde
    -> ^(TOK_TABLESERIALIZER rowFormatSerde)
    ;

tablePropertiesPrefixed
@init { msgs.push("table properties with prefix"); }
@after { msgs.pop(); }
    :
        KW_TBLPROPERTIES! tableProperties
    ;

tableProperties
@init { msgs.push("table properties"); }
@after { msgs.pop(); }
    :
      LPAREN tablePropertiesList RPAREN -> ^(TOK_TABLEPROPERTIES tablePropertiesList)
    ;

tablePropertiesList
@init { msgs.push("table properties list"); }
@after { msgs.pop(); }
    :
      keyValueProperty (COMMA keyValueProperty)* -> ^(TOK_TABLEPROPLIST keyValueProperty+)
    ;

keyValueProperty
@init { msgs.push("specifying key/value property"); }
@after { msgs.pop(); }
    :
      key=StringLiteral EQUAL value=StringLiteral -> ^(TOK_TABLEPROPERTY $key $value)
    ;

tableRowFormatFieldIdentifier:
		KW_FIELDS KW_TERMINATED KW_BY StringLiteral optEscapedBy
    ;

optEscapedBy:
		/* empty */ { $$ = NULL; }
		| KW_ESCAPED KW_BY StringLiteral
	;
	
tableRowFormatCollItemsIdentifier
@init { msgs.push("table row format's column separator"); }
@after { msgs.pop(); }
    :
      KW_COLLECTION KW_ITEMS KW_TERMINATED KW_BY collIdnt=StringLiteral
    -> ^(TOK_TABLEROWFORMATCOLLITEMS $collIdnt)
    ;

tableRowFormatMapKeysIdentifier
@init { msgs.push("table row format's map key separator"); }
@after { msgs.pop(); }
    :
      KW_MAP KW_KEYS KW_TERMINATED KW_BY mapKeysIdnt=StringLiteral
    -> ^(TOK_TABLEROWFORMATMAPKEYS $mapKeysIdnt)
    ;

tableRowFormatLinesIdentifier
@init { msgs.push("table row format's line separator"); }
@after { msgs.pop(); }
    :
      KW_LINES KW_TERMINATED KW_BY linesIdnt=StringLiteral
    -> ^(TOK_TABLEROWFORMATLINES $linesIdnt)
    ;

tableFileFormat
@init { msgs.push("table file format specification"); }
@after { msgs.pop(); }
    :
      KW_STORED KW_AS KW_SEQUENCEFILE  -> TOK_TBLSEQUENCEFILE
      | KW_STORED KW_AS KW_TEXTFILE  -> TOK_TBLTEXTFILE
      | KW_STORED KW_AS KW_RCFILE  -> TOK_TBLRCFILE
      | KW_STORED KW_AS KW_INPUTFORMAT inFmt=StringLiteral KW_OUTPUTFORMAT outFmt=StringLiteral (KW_INPUTDRIVER inDriver=StringLiteral KW_OUTPUTDRIVER outDriver=StringLiteral)?
      -> ^(TOK_TABLEFILEFORMAT $inFmt $outFmt $inDriver? $outDriver?)
      | KW_STORED KW_BY storageHandler=StringLiteral
         (KW_WITH KW_SERDEPROPERTIES serdeprops=tableProperties)?
      -> ^(TOK_STORAGEHANDLER $storageHandler $serdeprops?)
      | KW_STORED KW_AS genericSpec=Identifier
      -> ^(TOK_FILEFORMAT_GENERIC $genericSpec)
    ;

tableLocation
@init { msgs.push("table location specification"); }
@after { msgs.pop(); }
    :
      KW_LOCATION locn=StringLiteral -> ^(TOK_TABLELOCATION $locn)
    ;

columnNameTypeList
@init { msgs.push("column name type list"); }
@after { msgs.pop(); }
    : columnNameType (COMMA columnNameType)* -> ^(TOK_TABCOLLIST columnNameType+)
    ;

columnNameColonTypeList
@init { msgs.push("column name type list"); }
@after { msgs.pop(); }
    : columnNameColonType (COMMA columnNameColonType)* -> ^(TOK_TABCOLLIST columnNameColonType+)
    ;

columnNameList
@init { msgs.push("column name list"); }
@after { msgs.pop(); }
    : columnName (COMMA columnName)* -> ^(TOK_TABCOLNAME columnName+)
    ;

columnName
@init { msgs.push("column name"); }
@after { msgs.pop(); }
    :
      Identifier
    ;

columnNameOrderList
@init { msgs.push("column name order list"); }
@after { msgs.pop(); }
    : columnNameOrder (COMMA columnNameOrder)* -> ^(TOK_TABCOLNAME columnNameOrder+)
    ;

columnNameOrder
@init { msgs.push("column name order"); }
@after { msgs.pop(); }
    : Identifier (asc=KW_ASC | desc=KW_DESC)?
    -> {$desc == null}? ^(TOK_TABSORTCOLNAMEASC Identifier)
    ->                  ^(TOK_TABSORTCOLNAMEDESC Identifier)
    ;

columnNameCommentList
@init { msgs.push("column name comment list"); }
@after { msgs.pop(); }
    : columnNameComment (COMMA columnNameComment)* -> ^(TOK_TABCOLNAME columnNameComment+)
    ;

columnNameComment
@init { msgs.push("column name comment"); }
@after { msgs.pop(); }
    : colName=Identifier (KW_COMMENT comment=StringLiteral)?
    -> ^(TOK_TABCOL $colName TOK_NULL $comment?)
    ;

columnRefOrder
@init { msgs.push("column order"); }
@after { msgs.pop(); }
    : expression (asc=KW_ASC | desc=KW_DESC)?
    -> {$desc == null}? ^(TOK_TABSORTCOLNAMEASC expression)
    ->                  ^(TOK_TABSORTCOLNAMEDESC expression)
    ;

columnNameType
@init { msgs.push("column specification"); }
@after { msgs.pop(); }
    : colName=Identifier colType (KW_COMMENT comment=StringLiteral)?
    -> {$comment == null}? ^(TOK_TABCOL $colName colType)
    ->                     ^(TOK_TABCOL $colName colType $comment)
    ;

columnNameColonType
@init { msgs.push("column specification"); }
@after { msgs.pop(); }
    : colName=Identifier COLON colType (KW_COMMENT comment=StringLiteral)?
    -> {$comment == null}? ^(TOK_TABCOL $colName colType)
    ->                     ^(TOK_TABCOL $colName colType $comment)
    ;

colType
@init { msgs.push("column type"); }
@after { msgs.pop(); }
    : type
    ;

colTypeList
@init { msgs.push("column type list"); }
@after { msgs.pop(); }
    : colType (COMMA colType)* -> ^(TOK_COLTYPELIST colType+)
    ;

type
    : primitiveType
    | listType
    | structType
    | mapType
    | unionType;

primitiveType
@init { msgs.push("primitive type specification"); }
@after { msgs.pop(); }
    : KW_TINYINT       ->    TOK_TINYINT
    | KW_SMALLINT      ->    TOK_SMALLINT
    | KW_INT           ->    TOK_INT
    | KW_BIGINT        ->    TOK_BIGINT
    | KW_BOOLEAN       ->    TOK_BOOLEAN
    | KW_FLOAT         ->    TOK_FLOAT
    | KW_DOUBLE        ->    TOK_DOUBLE
    | KW_DATE          ->    TOK_DATE
    | KW_DATETIME      ->    TOK_DATETIME
    | KW_TIMESTAMP     ->    TOK_TIMESTAMP
    | KW_STRING        ->    TOK_STRING
    | KW_BINARY        ->    TOK_BINARY
    ;

listType
@init { msgs.push("list type"); }
@after { msgs.pop(); }
    : KW_ARRAY LESSTHAN type GREATERTHAN   -> ^(TOK_LIST type)
    ;

structType
@init { msgs.push("struct type"); }
@after { msgs.pop(); }
    : KW_STRUCT LESSTHAN columnNameColonTypeList GREATERTHAN -> ^(TOK_STRUCT columnNameColonTypeList)
    ;

mapType
@init { msgs.push("map type"); }
@after { msgs.pop(); }
    : KW_MAP LESSTHAN left=primitiveType COMMA right=type GREATERTHAN
    -> ^(TOK_MAP $left $right)
    ;

unionType
@init { msgs.push("uniontype type"); }
@after { msgs.pop(); }
    : KW_UNIONTYPE LESSTHAN colTypeList GREATERTHAN -> ^(TOK_UNIONTYPE colTypeList)
    ;

queryOperator
@init { msgs.push("query operator"); }
@after { msgs.pop(); }
    : KW_UNION KW_ALL -> ^(TOK_UNION)
    ;

// select statement select ... from ... where ... group by ... order by ...
queryStatementExpression
    : queryStatement (queryOperator^ queryStatement)*
    ;

queryStatement
    :
    fromClause
    ( b+=body )+ -> ^(TOK_QUERY fromClause body+)
    | regular_body
    ;

regular_body
   :
   insertClause
   selectClause
   fromClause
   whereClause?
   groupByClause?
   havingClause?
   orderByClause?
   clusterByClause?
   distributeByClause?
   sortByClause?
   limitClause? -> ^(TOK_QUERY fromClause ^(TOK_INSERT insertClause
                     selectClause whereClause? groupByClause? havingClause? orderByClause? clusterByClause?
                     distributeByClause? sortByClause? limitClause?))
   |
   selectStatement
   ;

selectStatement
   :
   selectClause
   fromClause
   whereClause?
   groupByClause?
   havingClause?
   orderByClause?
   clusterByClause?
   distributeByClause?
   sortByClause?
   limitClause? -> ^(TOK_QUERY fromClause ^(TOK_INSERT ^(TOK_DESTINATION ^(TOK_DIR TOK_TMP_FILE))
                     selectClause whereClause? groupByClause? havingClause? orderByClause? clusterByClause?
                     distributeByClause? sortByClause? limitClause?))
   ;


body
   :
   insertClause
   selectClause
   whereClause?
   groupByClause?
   havingClause?
   orderByClause?
   clusterByClause?
   distributeByClause?
   sortByClause?
   limitClause? -> ^(TOK_INSERT insertClause?
                     selectClause whereClause? groupByClause? havingClause? orderByClause? clusterByClause?
                     distributeByClause? sortByClause? limitClause?)
   |
   selectClause
   whereClause?
   groupByClause?
   havingClause?
   orderByClause?
   clusterByClause?
   distributeByClause?
   sortByClause?
   limitClause? -> ^(TOK_INSERT ^(TOK_DESTINATION ^(TOK_DIR TOK_TMP_FILE))
                     selectClause whereClause? groupByClause? havingClause? orderByClause? clusterByClause?
                     distributeByClause? sortByClause? limitClause?)
   ;

insertClause
@init { msgs.push("insert clause"); }
@after { msgs.pop(); }
   :
     KW_INSERT KW_OVERWRITE destination -> ^(TOK_DESTINATION destination)
   | KW_INSERT KW_INTO KW_TABLE tableOrPartition
       -> ^(TOK_INSERT_INTO ^(tableOrPartition))
   ;

destination
@init { msgs.push("destination specification"); }
@after { msgs.pop(); }
   :
     KW_LOCAL KW_DIRECTORY StringLiteral -> ^(TOK_LOCAL_DIR StringLiteral)
   | KW_DIRECTORY StringLiteral -> ^(TOK_DIR StringLiteral)
   | KW_TABLE tableOrPartition -> ^(tableOrPartition)
   ;

limitClause
@init { msgs.push("limit clause"); }
@after { msgs.pop(); }
   :
   KW_LIMIT num=Number -> ^(TOK_LIMIT $num)
   ;

//----------------------- Rules for parsing selectClause -----------------------------
// select a,b,c ...
selectClause
@init { msgs.push("select clause"); }
@after { msgs.pop(); }
    :
    KW_SELECT hintClause? (((KW_ALL | dist=KW_DISTINCT)? selectList)
                          | (transform=KW_TRANSFORM selectTrfmClause))
     -> {$transform == null && $dist == null}? ^(TOK_SELECT hintClause? selectList)
     -> {$transform == null && $dist != null}? ^(TOK_SELECTDI hintClause? selectList)
     -> ^(TOK_SELECT hintClause? ^(TOK_SELEXPR selectTrfmClause) )
    |
    trfmClause  ->^(TOK_SELECT ^(TOK_SELEXPR trfmClause))
    ;

selectList
@init { msgs.push("select list"); }
@after { msgs.pop(); }
    :
    selectItem ( COMMA  selectItem )* -> selectItem+
    ;

selectTrfmClause
@init { msgs.push("transform clause"); }
@after { msgs.pop(); }
    :
    LPAREN selectExpressionList RPAREN
    inSerde=rowFormat inRec=recordWriter
    KW_USING StringLiteral
    ( KW_AS ((LPAREN (aliasList | columnNameTypeList) RPAREN) | (aliasList | columnNameTypeList)))?
    outSerde=rowFormat outRec=recordReader
    -> ^(TOK_TRANSFORM selectExpressionList $inSerde $inRec StringLiteral $outSerde $outRec aliasList? columnNameTypeList?)
    ;

hintClause
@init { msgs.push("hint clause"); }
@after { msgs.pop(); }
    :
    DIVIDE STAR PLUS hintList STAR DIVIDE -> ^(TOK_HINTLIST hintList)
    ;

hintList
@init { msgs.push("hint list"); }
@after { msgs.pop(); }
    :
    hintItem (COMMA hintItem)* -> hintItem+
    ;

hintItem
@init { msgs.push("hint item"); }
@after { msgs.pop(); }
    :
    hintName (LPAREN hintArgs RPAREN)? -> ^(TOK_HINT hintName hintArgs?)
    ;

hintName
@init { msgs.push("hint name"); }
@after { msgs.pop(); }
    :
    KW_MAPJOIN -> TOK_MAPJOIN
    | KW_STREAMTABLE -> TOK_STREAMTABLE
    | KW_HOLD_DDLTIME -> TOK_HOLD_DDLTIME
    ;

hintArgs
@init { msgs.push("hint arguments"); }
@after { msgs.pop(); }
    :
    hintArgName (COMMA hintArgName)* -> ^(TOK_HINTARGLIST hintArgName+)
    ;

hintArgName
@init { msgs.push("hint argument name"); }
@after { msgs.pop(); }
    :
    Identifier
    ;

selectItem
@init { msgs.push("selection target"); }
@after { msgs.pop(); }
    :
    ( selectExpression  ((KW_AS? Identifier) | (KW_AS LPAREN Identifier (COMMA Identifier)* RPAREN))?) -> ^(TOK_SELEXPR selectExpression Identifier*)
    ;

trfmClause
@init { msgs.push("transform clause"); }
@after { msgs.pop(); }
    :
    (   KW_MAP    selectExpressionList
      | KW_REDUCE selectExpressionList )
    inSerde=rowFormat inRec=recordWriter
    KW_USING StringLiteral
    ( KW_AS ((LPAREN (aliasList | columnNameTypeList) RPAREN) | (aliasList | columnNameTypeList)))?
    outSerde=rowFormat outRec=recordReader
    -> ^(TOK_TRANSFORM selectExpressionList $inSerde $inRec StringLiteral $outSerde $outRec aliasList? columnNameTypeList?)
    ;

selectExpression
@init { msgs.push("select expression"); }
@after { msgs.pop(); }
    :
    expression | tableAllColumns
    ;

selectExpressionList
@init { msgs.push("select expression list"); }
@after { msgs.pop(); }
    :
    selectExpression (COMMA selectExpression)* -> ^(TOK_EXPLIST selectExpression+)
    ;


//-----------------------------------------------------------------------------------

tableAllColumns
    : STAR
        -> ^(TOK_ALLCOLREF)
    | tableName DOT STAR
        -> ^(TOK_ALLCOLREF tableName)
    ;

// (table|column)
tableOrColumn
@init { msgs.push("table or column identifier"); }
@after { msgs.pop(); }
    :
    Identifier -> ^(TOK_TABLE_OR_COL Identifier)
    ;

expressionList
@init { msgs.push("expression list"); }
@after { msgs.pop(); }
    :
    expression (COMMA expression)* -> ^(TOK_EXPLIST expression+)
    ;

aliasList
@init { msgs.push("alias list"); }
@after { msgs.pop(); }
    :
    Identifier (COMMA Identifier)* -> ^(TOK_ALIASLIST Identifier+)
    ;

//----------------------- Rules for parsing fromClause ------------------------------
// from [col1, col2, col3] table1, [col4, col5] table2
fromClause
@init { msgs.push("from clause"); }
@after { msgs.pop(); }
    :
    KW_FROM joinSource -> ^(TOK_FROM joinSource)
    ;

joinSource
@init { msgs.push("join source"); }
@after { msgs.pop(); }
    : fromSource ( joinToken^ fromSource (KW_ON! expression)? )*
    | uniqueJoinToken^ uniqueJoinSource (COMMA! uniqueJoinSource)+
    ;

uniqueJoinSource
@init { msgs.push("join source"); }
@after { msgs.pop(); }
    : KW_PRESERVE? fromSource uniqueJoinExpr
    ;

uniqueJoinExpr
@init { msgs.push("unique join expression list"); }
@after { msgs.pop(); }
    : LPAREN e1+=expression (COMMA e1+=expression)* RPAREN
      -> ^(TOK_EXPLIST $e1*)
    ;

uniqueJoinToken
@init { msgs.push("unique join"); }
@after { msgs.pop(); }
    : KW_UNIQUEJOIN -> TOK_UNIQUEJOIN;

joinToken
@init { msgs.push("join type specifier"); }
@after { msgs.pop(); }
    :
      KW_JOIN                     -> TOK_JOIN
    | kwInner  KW_JOIN            -> TOK_JOIN
    | KW_LEFT  KW_OUTER KW_JOIN   -> TOK_LEFTOUTERJOIN
    | KW_RIGHT KW_OUTER KW_JOIN   -> TOK_RIGHTOUTERJOIN
    | KW_FULL  KW_OUTER KW_JOIN   -> TOK_FULLOUTERJOIN
    | KW_LEFT  KW_SEMI  KW_JOIN   -> TOK_LEFTSEMIJOIN
    ;

lateralView
@init {msgs.push("lateral view"); }
@after {msgs.pop(); }
	:
	KW_LATERAL KW_VIEW function tableAlias KW_AS Identifier (COMMA Identifier)* -> ^(TOK_LATERAL_VIEW ^(TOK_SELECT ^(TOK_SELEXPR function Identifier+ tableAlias)))
	;

tableAlias
@init {msgs.push("table alias"); }
@after {msgs.pop(); }
    :
    Identifier -> ^(TOK_TABALIAS Identifier)
    ;

fromSource
@init { msgs.push("from source"); }
@after { msgs.pop(); }
    :
    (tableSource | subQuerySource) (lateralView^)*
    ;

tableBucketSample
@init { msgs.push("table bucket sample specification"); }
@after { msgs.pop(); }
    :
    KW_TABLESAMPLE LPAREN KW_BUCKET (numerator=Number) KW_OUT KW_OF (denominator=Number) (KW_ON expr+=expression (COMMA expr+=expression)*)? RPAREN -> ^(TOK_TABLEBUCKETSAMPLE $numerator $denominator $expr*)
    ;

splitSample
@init { msgs.push("table split sample specification"); }
@after { msgs.pop(); }
    :
    KW_TABLESAMPLE LPAREN  (numerator=Number) KW_PERCENT RPAREN -> ^(TOK_TABLESPLITSAMPLE $numerator)
    ;

tableSample
@init { msgs.push("table sample specification"); }
@after { msgs.pop(); }
    :
    tableBucketSample |
    splitSample
    ;

tableSource
@init { msgs.push("table source"); }
@after { msgs.pop(); }
    : tabname=tableName (ts=tableSample)? (alias=Identifier)?
    -> ^(TOK_TABREF $tabname $ts? $alias?)
    ;

tableName
@init { msgs.push("table name"); }
@after { msgs.pop(); }
    : (db=Identifier DOT)? tab=Identifier
    -> ^(TOK_TABNAME $db? $tab)
    ;

viewName
@init { msgs.push("view name"); }
@after { msgs.pop(); }
    :
    (db=Identifier DOT)? view=Identifier
    -> ^(TOK_TABNAME $db? $view)
    ;

subQuerySource
@init { msgs.push("subquery source"); }
@after { msgs.pop(); }
    :
    LPAREN queryStatementExpression RPAREN Identifier -> ^(TOK_SUBQUERY queryStatementExpression Identifier)
    ;

//----------------------- Rules for parsing whereClause -----------------------------
// where a=b and ...
whereClause
@init { msgs.push("where clause"); }
@after { msgs.pop(); }
    :
    KW_WHERE searchCondition -> ^(TOK_WHERE searchCondition)
    ;

searchCondition
@init { msgs.push("search condition"); }
@after { msgs.pop(); }
    :
    expression
    ;

//-----------------------------------------------------------------------------------

// group by a,b
groupByClause
@init { msgs.push("group by clause"); }
@after { msgs.pop(); }
    :
    KW_GROUP KW_BY
    groupByExpression
    ( COMMA groupByExpression )*
    -> ^(TOK_GROUPBY groupByExpression+)
    ;

groupByExpression
@init { msgs.push("group by expression"); }
@after { msgs.pop(); }
    :
    expression
    ;

havingClause
@init { msgs.push("having clause"); }
@after { msgs.pop(); }
    :
    KW_HAVING havingCondition -> ^(TOK_HAVING havingCondition)
    ;

havingCondition
@init { msgs.push("having condition"); }
@after { msgs.pop(); }
    :
    expression
    ;

// order by a,b
orderByClause
@init { msgs.push("order by clause"); }
@after { msgs.pop(); }
    :
    KW_ORDER KW_BY
    columnRefOrder
    ( COMMA columnRefOrder)* -> ^(TOK_ORDERBY columnRefOrder+)
    ;

clusterByClause
@init { msgs.push("cluster by clause"); }
@after { msgs.pop(); }
    :
    KW_CLUSTER KW_BY
    expression
    ( COMMA expression )* -> ^(TOK_CLUSTERBY expression+)
    ;

distributeByClause
@init { msgs.push("distribute by clause"); }
@after { msgs.pop(); }
    :
    KW_DISTRIBUTE KW_BY
    expression (COMMA expression)* -> ^(TOK_DISTRIBUTEBY expression+)
    ;

sortByClause
@init { msgs.push("sort by clause"); }
@after { msgs.pop(); }
    :
    KW_SORT KW_BY
    columnRefOrder
    ( COMMA columnRefOrder)* -> ^(TOK_SORTBY columnRefOrder+)
    ;

// fun(par1, par2, par3)
function
@init { msgs.push("function specification"); }
@after { msgs.pop(); }
    :
    functionName
    LPAREN
      (
        (star=STAR)
        | (dist=KW_DISTINCT)? (expression (COMMA expression)*)?
      )
    RPAREN -> {$star != null}? ^(TOK_FUNCTIONSTAR functionName)
           -> {$dist == null}? ^(TOK_FUNCTION functionName (expression+)?)
                            -> ^(TOK_FUNCTIONDI functionName (expression+)?)
    ;

functionName
@init { msgs.push("function name"); }
@after { msgs.pop(); }
    : // Keyword IF is also a function name
    Identifier | KW_IF | KW_ARRAY | KW_MAP | KW_STRUCT | KW_UNIONTYPE
    ;

castExpression
@init { msgs.push("cast expression"); }
@after { msgs.pop(); }
    :
    KW_CAST
    LPAREN
          expression
          KW_AS
          primitiveType
    RPAREN -> ^(TOK_FUNCTION primitiveType expression)
    ;

caseExpression
@init { msgs.push("case expression"); }
@after { msgs.pop(); }
    :
    KW_CASE expression
    (KW_WHEN expression KW_THEN expression)+
    (KW_ELSE expression)?
    KW_END -> ^(TOK_FUNCTION KW_CASE expression*)
    ;

whenExpression
@init { msgs.push("case expression"); }
@after { msgs.pop(); }
    :
    KW_CASE
     ( KW_WHEN expression KW_THEN expression)+
    (KW_ELSE expression)?
    KW_END -> ^(TOK_FUNCTION KW_WHEN expression*)
    ;

constant
@init { msgs.push("constant"); }
@after { msgs.pop(); }
    :
    Number
    | StringLiteral
    | stringLiteralSequence
    | BigintLiteral
    | SmallintLiteral
    | TinyintLiteral
    | charSetStringLiteral
    | booleanValue
    ;

stringLiteralSequence
    :
    StringLiteral StringLiteral+ -> ^(TOK_STRINGLITERALSEQUENCE StringLiteral StringLiteral+)
    ;

charSetStringLiteral
@init { msgs.push("character string literal"); }
@after { msgs.pop(); }
    :
    csName=CharSetName csLiteral=CharSetLiteral -> ^(TOK_CHARSETLITERAL $csName $csLiteral)
    ;

expression
@init { msgs.push("expression specification"); }
@after { msgs.pop(); }
    :
    precedenceOrExpression
    ;

atomExpression
    :
    KW_NULL -> TOK_NULL
    | constant
    | function
    | castExpression
    | caseExpression
    | whenExpression
    | tableOrColumn
    | LPAREN! expression RPAREN!
    ;


precedenceFieldExpression
    :
    atomExpression ((LSQUARE^ expression RSQUARE!) | (DOT^ Identifier))*
    ;

precedenceUnaryOperator
    :
    PLUS | MINUS | TILDE
    ;

nullCondition
    :
    KW_NULL -> ^(TOK_ISNULL)
    | KW_NOT KW_NULL -> ^(TOK_ISNOTNULL)
    ;

precedenceUnaryPrefixExpression
    :
    (precedenceUnaryOperator^)* precedenceFieldExpression
    ;

precedenceUnarySuffixExpression
    : precedenceUnaryPrefixExpression (a=KW_IS nullCondition)?
    -> {$a != null}? ^(TOK_FUNCTION nullCondition precedenceUnaryPrefixExpression)
    -> precedenceUnaryPrefixExpression
    ;


precedenceBitwiseXorOperator
    :
    BITWISEXOR
    ;

precedenceBitwiseXorExpression
    :
    precedenceUnarySuffixExpression (precedenceBitwiseXorOperator^ precedenceUnarySuffixExpression)*
    ;


precedenceStarOperator
    :
    STAR | DIVIDE | MOD | DIV
    ;

precedenceStarExpression
    :
    precedenceBitwiseXorExpression (precedenceStarOperator^ precedenceBitwiseXorExpression)*
    ;


precedencePlusOperator
    :
    PLUS | MINUS
    ;

precedencePlusExpression
    :
    precedenceStarExpression (precedencePlusOperator^ precedenceStarExpression)*
    ;


precedenceAmpersandOperator
    :
    AMPERSAND
    ;

precedenceAmpersandExpression
    :
    precedencePlusExpression (precedenceAmpersandOperator^ precedencePlusExpression)*
    ;


precedenceBitwiseOrOperator
    :
    BITWISEOR
    ;

precedenceBitwiseOrExpression
    :
    precedenceAmpersandExpression (precedenceBitwiseOrOperator^ precedenceAmpersandExpression)*
    ;


// Equal operators supporting NOT prefix
precedenceEqualNegatableOperator
    :
    KW_LIKE | KW_RLIKE | KW_REGEXP
    ;

precedenceEqualOperator
    :
    precedenceEqualNegatableOperator | EQUAL | NOTEQUAL | LESSTHANOREQUALTO | LESSTHAN | GREATERTHANOREQUALTO | GREATERTHAN
    ;

precedenceEqualExpression
    :
    (left=precedenceBitwiseOrExpression -> $left)
    (
       (KW_NOT precedenceEqualNegatableOperator notExpr=precedenceBitwiseOrExpression)
       -> ^(KW_NOT ^(precedenceEqualNegatableOperator $precedenceEqualExpression $notExpr))
    | (precedenceEqualOperator equalExpr=precedenceBitwiseOrExpression)
       -> ^(precedenceEqualOperator $precedenceEqualExpression $equalExpr)
    | (KW_NOT KW_IN expressions)
       -> ^(KW_NOT ^(TOK_FUNCTION KW_IN $precedenceEqualExpression expressions))
    | (KW_IN expressions)
       -> ^(TOK_FUNCTION KW_IN $precedenceEqualExpression expressions)
    )*
    ;

expressions
    :
    LPAREN expression (COMMA expression)* RPAREN -> expression*
    ;

precedenceNotOperator
    :
    KW_NOT
    ;

precedenceNotExpression
    :
    (precedenceNotOperator^)* precedenceEqualExpression
    ;


precedenceAndOperator
    :
    KW_AND
    ;

precedenceAndExpression
    :
    precedenceNotExpression (precedenceAndOperator^ precedenceNotExpression)*
    ;


precedenceOrOperator
    :
    KW_OR
    ;

precedenceOrExpression
    :
    precedenceAndExpression (precedenceOrOperator^ precedenceAndExpression)*
    ;


booleanValue
    :
    KW_TRUE^ | KW_FALSE^
    ;

tableOrPartition
   :
   tableName partitionSpec? -> ^(TOK_TAB tableName partitionSpec?)
   ;

partitionSpec
    :
    KW_PARTITION
     LPAREN partitionVal (COMMA  partitionVal )* RPAREN -> ^(TOK_PARTSPEC partitionVal +)
    ;

partitionVal
    :
    Identifier (EQUAL constant)? -> ^(TOK_PARTVAL Identifier constant?)
    ;

sysFuncNames
    :
      KW_AND
    | KW_OR
    | KW_NOT
    | KW_LIKE
    | KW_IF
    | KW_CASE
    | KW_WHEN
    | KW_TINYINT
    | KW_SMALLINT
    | KW_INT
    | KW_BIGINT
    | KW_FLOAT
    | KW_DOUBLE
    | KW_BOOLEAN
    | KW_STRING
    | KW_BINARY
    | KW_ARRAY
    | KW_MAP
    | KW_STRUCT
    | KW_UNIONTYPE
    | EQUAL
    | NOTEQUAL
    | LESSTHANOREQUALTO
    | LESSTHAN
    | GREATERTHANOREQUALTO
    | GREATERTHAN
    | DIVIDE
    | PLUS
    | MINUS
    | STAR
    | MOD
    | DIV
    | AMPERSAND
    | TILDE
    | BITWISEOR
    | BITWISEXOR
    | KW_RLIKE
    | KW_REGEXP
    | KW_IN
    ;

descFuncNames
    :
      sysFuncNames
    | StringLiteral
    | Identifier
    ;

*/
	
%%



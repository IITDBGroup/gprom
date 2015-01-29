%option prefix="hive" 
%option outfile="hive_parser.lex.c"

/*
 * SQL lexer file for lex/flex defining basic tokens used in the oracle parser
 *
 */


/*******************************************************************************
 * options section 
 ******************************************************************************/

/* set lexer options */
%option noyywrap nodefault yylineno

/* includes */
%{
#include "common.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "parser/parse_internal_hive.h"
#include "hive_parser.tab.h"

#undef free
#undef malloc

#define TOKSELF() { \
	TRACE_LOG("Lexed TOKENSELF <%c> with VALUE <%c>", hivetext[0], hivetext[0]); \
	hivelval.stringVal = strdup(hivetext); \
	return hivetext[0]; \
	}
#define TOK(name) { \
	TRACE_LOG("Lexed TOKEN <%s> with VALUE <%s>", #name, hivetext); \
 	hivelval.stringVal = strdup(hivetext); \
 	return name; \
 	}
#define UPCASE_TOK(name) { \
	TRACE_LOG("Lexed TOKEN <%s> with VALUE <%s>", #name, hivetext); \
	char *result = strdup(hivetext); \
	hivelval.stringVal = result; \
	for(; *result != '\0'; (*(result) = toupper(*result)), result++); \
	return name; \
	}
#define TOKSAVE(name,field,function) { \
	TRACE_LOG("Lexed TOKEN <%s> of TYPE <%s>",  #name, #field); \
	hivelval.field = function(hivetext); \
	return name; \
	}
%}

/********************************************************************
 * lexer states 
 */

%s SQL

/********************************************************************
 * predefined regular expressions 
 */

/* whitespace */
space			[\t\f\n\r ]+
newline			[\n\r]
nonewline		[^\n\r]

comment			("--"{nonewline}*)

whitespace		({space}+|{comment})

/* numbers */
integer 		[0-9]+
decimal 		({integer}*\.{integer}+|{integer}+\.{integer}*)
float 			(({integer}|{decimal})([eE][+-]?{integer})?)

/* names (e.g., tables, attributes) */
identStart		[A-Za-z]
identContinue	[A-Za-z0-9_%?]
identDelim		[.]
doublequote     [\"]
quotedIdentifier		{doublequote}[^\"]+{doublequote}
identifiers		({identStart}({identDelim}?{identContinue}+)*)
parameters		[:]{identifiers}

/* operators */
/* comparisonOps	['!'|'<'|'>'|'=']{1,2} */ 
comparisonOps   ("="|"<>"|"!="|"<"|">"|"<="|">=")
/*shiftsOps		["<<"|">>"] */
notOps			("NOT"|"!")

/*****************************************************************/
/* HIVE ONES */
DOT						'.'
COLON					':' 
COMMA					',' 
SEMICOLON				';' 

LPAREN					'(' 
RPAREN					')' 
LSQUARE					'[' 
RSQUARE					']' 
LCURLY					'{'
RCURLY					'}'

EQUAL					("=" | "==")
NOTEQUAL				("<>" | "!=")
LESSTHANOREQUALTO		"<="
LESSTHAN				"<"
GREATERTHANOREQUALTO	">="
GREATERTHAN				">"

DIVIDE					'/'
PLUS					'+'
MINUS					'-'
STAR					'*'
MOD						'%'
DIV						"DIV"

AMPERSAND				'&'
TILDE					'~'
BITWISEOR				'|'
BITWISEXOR				'^'
QUESTION				'?'
DOLLAR					'$'

/* LITERAL ONES */
Letter					 [A-Za-z]
HexDigit 				 [A-Fa-f]
Digit					 [0-9]
Exponent				 ("e" | "E") ( {PLUS}|{MINUS} )? ({Digit})+

RegexComponent 			([A-Za-z0-9_] | {PLUS} | {STAR} | {QUESTION} | {MINUS} | {DOT} | {LPAREN} | {RPAREN} | {LSQUARE} | {RSQUARE} | {LCURLY} | {RCURLY} | {BITWISEXOR} | {BITWISEOR} | {DOLLAR})

StringLiteral ( '\'' ( ~('\''|'\\') | ('\\' .) )* '\'' | '\"' ( ~('\"'|'\\') | ('\\' .) )* '\"' )+

CharSetLiteral ({StringLiteral} | '0' 'X' ({HexDigit}|{Digit})+)

BigintLiteral 			({Digit})+ 'L'
SmallintLiteral			({Digit})+ 'S'
TinyintLiteral			({Digit})+ 'Y'
Number					({Digit})+ ( {DOT} ({Digit})* ({Exponent})? | {Exponent})?

Identifier				(Letter | Digit) (Letter | Digit | '_')* | '`' RegexComponent+ '`'
CharSetName 		'_' (Letter | Digit | '_' | '-' | '.' | ':' )+

WS    				(' '|'\r'|'\t'|'\n')

COMMENT '--' (~('\n'|'\r'))*

/*******************************************************************************
 * Token definitions and actions 
 ******************************************************************************/
%%

	/* ignore all whitespace */
{whitespace}	 { TRACE_LOG("Lexed whitespace <%s>", hivetext); /* ignore it */ }

	/* literal keyword tokens */
(?i:user)	TOK(kwUser)
(?i:role)	TOK(kwRole)
(?i:inner)	TOK(kwInner)

(?i:TRUE)	TOK(KW_TRUE)
(?i:FALSE)	TOK(KW_FALSE)
(?i:ALL)	TOK(KW_ALL)
(?i:AND)	TOK(KW_AND)
(?i:OR)		TOK(KW_OR)
{notOps} 	TOK(KW_NOT)
(?i:LIKE)	TOK(KW_LIKE)

(?i:IF)	TOK(KW_IF)
(?i:EXISTS)	TOK(KW_EXISTS)

(?i:ASC)	TOK(KW_ASC)
(?i:DESC)	TOK(KW_DESC)
(?i:ORDER)	TOK(KW_ORDER)
(?i:GROUP)	TOK(KW_GROUP)
(?i:BY)	TOK(KW_BY)
(?i:HAVING)	TOK(KW_HAVING)
(?i:WHERE)	TOK(KW_WHERE)
(?i:FROM)	TOK(KW_FROM)
(?i:AS)	TOK(KW_AS)
(?i:SELECT)	TOK(KW_SELECT)
(?i:DISTINCT)	TOK(KW_DISTINCT)
(?i:INSERT)	TOK(KW_INSERT)
(?i:OVERWRITE)	TOK(KW_OVERWRITE)
(?i:OUTER)	TOK(KW_OUTER)
(?i:UNIQUEJOIN)	TOK(KW_UNIQUEJOIN)
(?i:PRESERVE)	TOK(KW_PRESERVE)
(?i:JOIN)	TOK(KW_JOIN)
(?i:LEFT)	TOK(KW_LEFT)
(?i:RIGHT)	TOK(KW_RIGHT)
(?i:FULL)	TOK(KW_FULL)
(?i:ON)	TOK(KW_ON)
(?i:PARTITION)	TOK(KW_PARTITION)
(?i:PARTITIONS)	TOK(KW_PARTITIONS)
(?i:TABLE) TOK(KW_TABLE)
(?i:TABLES) TOK(KW_TABLES)
(?i:INDEX) TOK(KW_INDEX)
(?i:INDEXES) TOK(KW_INDEXES)
(?i:REBUILD) TOK(KW_REBUILD)
(?i:FUNCTIONS) TOK(KW_FUNCTIONS)
(?i:SHOW) TOK(KW_SHOW)
(?i:MSCK) TOK(KW_MSCK)
(?i:REPAIR) TOK(KW_REPAIR)
(?i:DIRECTORY) TOK(KW_DIRECTORY)
(?i:LOCAL) TOK(KW_LOCAL)
(?i:TRANSFORM)	TOK(KW_TRANSFORM) 
(?i:USING) TOK(KW_USING);
(?i:CLUSTER) TOK(KW_CLUSTER);
(?i:DISTRIBUTE) TOK(KW_DISTRIBUTE);
(?i:SORT) TOK(KW_SORT);
(?i:UNION) TOK(KW_UNION);
(?i:LOAD) TOK(KW_LOAD);
(?i:EXPORT) TOK(KW_EXPORT);
(?i:IMPORT) TOK(KW_IMPORT);
(?i:DATA) TOK(KW_DATA);
(?i:INPATH) TOK(KW_INPATH);
(?i:IS) TOK(KW_IS);
(?i:NULL) TOK(KW_NULL);
(?i:CREATE) TOK(KW_CREATE);
(?i:EXTERNAL) TOK(KW_EXTERNAL);
(?i:ALTER) TOK(KW_ALTER);
(?i:CHANGE) TOK(KW_CHANGE);
(?i:COLUMN) TOK(KW_COLUMN);
(?i:FIRST) TOK(KW_FIRST);
(?i:AFTER) TOK(KW_AFTER);
(?i:DESCRIBE) TOK(KW_DESCRIBE);
(?i:DROP) TOK(KW_DROP);
(?i:RENAME) TOK(KW_RENAME);
(?i:TO) TOK(KW_TO);
(?i:COMMENT) TOK(KW_COMMENT);
(?i:BOOLEAN) TOK(KW_BOOLEAN);
(?i:TINYINT) TOK(KW_TINYINT);
(?i:SMALLINT) TOK(KW_SMALLINT);
(?i:INT) TOK(KW_INT);
(?i:BIGINT) TOK(KW_BIGINT);
(?i:FLOAT) TOK(KW_FLOAT);
(?i:DOUBLE) TOK(KW_DOUBLE);
(?i:DATE) TOK(KW_DATE);
(?i:DATETIME) TOK(KW_DATETIME);
(?i:TIMESTAMP) TOK(KW_TIMESTAMP);
(?i:STRING) TOK(KW_STRING);
(?i:ARRAY) TOK(KW_ARRAY);
(?i:STRUCT) TOK(KW_STRUCT);
(?i:MAP) TOK(KW_MAP);
(?i:UNIONTYPE) TOK(KW_UNIONTYPE);
(?i:REDUCE) TOK(KW_REDUCE);
(?i:PARTITIONED) TOK(KW_PARTITIONED);
(?i:CLUSTERED) TOK(KW_CLUSTERED);
(?i:SORTED) TOK(KW_SORTED);
(?i:INTO) TOK(KW_INTO);
(?i:BUCKETS) TOK(KW_BUCKETS);
(?i:ROW) TOK(KW_ROW);
(?i:FORMAT) TOK(KW_FORMAT);
(?i:DELIMITED) TOK(KW_DELIMITED);
(?i:FIELDS) TOK(KW_FIELDS);
(?i:TERMINATED) TOK(KW_TERMINATED);
(?i:ESCAPED) TOK(KW_ESCAPED);
(?i:COLLECTION) TOK(KW_COLLECTION);
(?i:ITEMS) TOK(KW_ITEMS);
(?i:KEYS) TOK(KW_KEYS);
(?i:$KEY$) TOK(KW_KEY_TYPE);
(?i:LINES) TOK(KW_LINES);
(?i:STORED) TOK(KW_STORED);
(?i:FILEFORMAT) TOK(KW_FILEFORMAT);
(?i:SEQUENCEFILE) TOK(KW_SEQUENCEFILE);
(?i:TEXTFILE) TOK(KW_TEXTFILE);
(?i:RCFILE) TOK(KW_RCFILE);
(?i:INPUTFORMAT) TOK(KW_INPUTFORMAT);
(?i:OUTPUTFORMAT) TOK(KW_OUTPUTFORMAT);
(?i:INPUTDRIVER) TOK(KW_INPUTDRIVER);
(?i:OUTPUTDRIVER) TOK(KW_OUTPUTDRIVER);
(?i:OFFLINE) TOK(KW_OFFLINE);
(?i:ENABLE) TOK(KW_ENABLE);
(?i:DISABLE) TOK(KW_DISABLE);
(?i:READONLY) TOK(KW_READONLY);
(?i:NO_DROP) TOK(KW_NO_DROP);
(?i:LOCATION) TOK(KW_LOCATION);
(?i:TABLESAMPLE) TOK(KW_TABLESAMPLE);
(?i:BUCKET) TOK(KW_BUCKET);
(?i:OUT) TOK(KW_OUT);
(?i:OF) TOK(KW_OF);
(?i:PERCENT) TOK(KW_PERCENT);
(?i:CAST) TOK(KW_CAST);
(?i:ADD) TOK(KW_ADD);
(?i:REPLACE) TOK(KW_REPLACE);
(?i:COLUMNS) TOK(KW_COLUMNS);
(?i:RLIKE) TOK(KW_RLIKE);
(?i:REGEXP) TOK(KW_REGEXP);
(?i:TEMPORARY) TOK(KW_TEMPORARY);
(?i:FUNCTION) TOK(KW_FUNCTION);
(?i:EXPLAIN) TOK(KW_EXPLAIN);
(?i:EXTENDED) TOK(KW_EXTENDED);
(?i:FORMATTED) TOK(KW_FORMATTED);
(?i:SERDE) TOK(KW_SERDE);
(?i:WITH) TOK(KW_WITH);
(?i:DEFERRED) TOK(KW_DEFERRED);
(?i:SERDEPROPERTIES) TOK(KW_SERDEPROPERTIES);
(?i:DBPROPERTIES) TOK(KW_DBPROPERTIES);
(?i:LIMIT) TOK(KW_LIMIT);
(?i:SET) TOK(KW_SET);
(?i:TBLPROPERTIES) TOK(KW_TBLPROPERTIES);
(?i:IDXPROPERTIES) TOK(KW_IDXPROPERTIES);
(?i:$VALUE$) TOK(KW_VALUE_TYPE);
(?i:$ELEM$) TOK(KW_ELEM_TYPE);
(?i:CASE) TOK(KW_CASE);
(?i:WHEN) TOK(KW_WHEN);
(?i:THEN) TOK(KW_THEN);
(?i:ELSE) TOK(KW_ELSE);
(?i:END) TOK(KW_END);
(?i:MAPJOIN) TOK(KW_MAPJOIN);
(?i:STREAMTABLE) TOK(KW_STREAMTABLE);
(?i:HOLD_DDLTIME) TOK(KW_HOLD_DDLTIME);
(?i:CLUSTERSTATUS) TOK(KW_CLUSTERSTATUS);
(?i:UTC) TOK(KW_UTC);
(?i:UTC_TMESTAMP) TOK(KW_UTCTIMESTAMP);
(?i:LONG) TOK(KW_LONG);
(?i:DELETE) TOK(KW_DELETE);
(?i:PLUS) TOK(KW_PLUS);
(?i:MINUS) TOK(KW_MINUS);
(?i:FETCH) TOK(KW_FETCH);
(?i:INTERSECT) TOK(KW_INTERSECT);
(?i:VIEW) TOK(KW_VIEW);
(?i:IN) TOK(KW_IN);
(?i:DATABASE) TOK(KW_DATABASE);
(?i:DATABASES) TOK(KW_DATABASES);
(?i:MATERIALIZED) TOK(KW_MATERIALIZED);
(?i:SCHEMA) TOK(KW_SCHEMA);
(?i:SCHEMAS) TOK(KW_SCHEMAS);
(?i:GRANT) TOK(KW_GRANT);
(?i:REVOKE) TOK(KW_REVOKE);
(?i:SSL) TOK(KW_SSL);
(?i:UNDO) TOK(KW_UNDO);
(?i:LOCK) TOK(KW_LOCK);
(?i:LOCKS) TOK(KW_LOCKS);
(?i:UNLOCK) TOK(KW_UNLOCK);
(?i:SHARED) TOK(KW_SHARED);
(?i:EXCLUSIVE) TOK(KW_EXCLUSIVE);
(?i:PROCEDURE) TOK(KW_PROCEDURE);
(?i:UNSIGNED) TOK(KW_UNSIGNED);
(?i:WHILE) TOK(KW_WHILE);
(?i:READ) TOK(KW_READ);
(?i:READS) TOK(KW_READS);
(?i:PURGE) TOK(KW_PURGE);
(?i:RANGE) TOK(KW_RANGE);
(?i:ANALYZE) TOK(KW_ANALYZE);
(?i:BEFORE) TOK(KW_BEFORE);
(?i:BETWEEN) TOK(KW_BETWEEN);
(?i:BOTH) TOK(KW_BOTH);
(?i:BINARY) TOK(KW_BINARY);
(?i:CROSS) TOK(KW_CROSS);
(?i:CONTINUE) TOK(KW_CONTINUE);
(?i:CURSOR) TOK(KW_CURSOR);
(?i:TRIGGER) TOK(KW_TRIGGER);
(?i:RECORDREADER) TOK(KW_RECORDREADER);
(?i:RECORDWRITER) TOK(KW_RECORDWRITER);
(?i:SEMI) TOK(KW_SEMI);
(?i:LATERAL) TOK(KW_LATERAL);
(?i:TOUCH) TOK(KW_TOUCH);
(?i:ARCHIVE) TOK(KW_ARCHIVE);
(?i:UNARCHIVE) TOK(KW_UNARCHIVE);
(?i:COMPUTE) TOK(KW_COMPUTE);
(?i:STATISTICS) TOK(KW_STATISTICS);
(?i:USE) TOK(KW_USE);
(?i:OPTION) TOK(KW_OPTION);
(?i:CONCATENATE) TOK(KW_CONCATENATE);
(?i:SHOW_DATABASE) TOK(KW_SHOW_DATABASE);
(?i:UPDATE) TOK(KW_UPDATE);
(?i:RESTRICT) TOK(KW_RESTRICT);
(?i:CASCADE) TOK(KW_CASCADE);


	/* failure - no token matched */
.		{ hiveerror("Unknown symbol"); }	/* random non-SQL text */

%%

/*
	
{identifiers}	UPCASE_TOK(identifier)
{quotedIdentifier}   { 		
		hivelval.stringVal = MALLOC(strlen(hivetext) - 1);
		memcpy(hivelval.stringVal, hivetext + 1, strlen(hivetext) - 2);
		hivelval.stringVal[strlen(hivetext) - 2] = '\0';
		TRACE_LOG("Lexed TOKEN <quotedIdentifier> with VALUE <%s>", hivetext);
		return identifier;
	}	

{parameters} {
		hivelval.stringVal = MALLOC(strlen(hivetext));
		memcpy(hivelval.stringVal, hivetext + 1, strlen(hivetext) - 1);
		hivelval.stringVal[strlen(hivetext) - 1] = '\0';
		TRACE_LOG("Lexed TOKEN <parameter> with VALUE <:%s>", hivetext);
		return parameter;
	}

	
[(),.;] TOKSELF()

	
{comparisonOps}		TOK(comparisonOps)
[-+*%/^~&|!]		TOKSELF()
	


	
{integer}	TOKSAVE(intConst,intVal,atoi)
{float}	TOKSAVE(floatConst,floatVal,atof)

	
'[^'\n]*'	{
		hivelval.stringVal = MALLOC(strlen(hivetext) - 1);
		memcpy(hivelval.stringVal, hivetext + 1, strlen(hivetext) - 2);
		hivelval.stringVal[strlen(hivetext) - 2] = '\0';
		TRACE_LOG("Lexed TOKEN <stringConst> with VALUE <%s>", hivetext);
		return stringConst;
	}
}
		
'[^'\n]*$	{hiveerror("Unterminated string"); }
*/

/*******************************************************************************
 * Literal C code to include
 ******************************************************************************/

/* use logging framework in the future */
void hiveerror(char *s)
{
	ERROR_LOG("%s at %s\n", s, hivetext);
}

void hiveSetupStringInput(char *input)
{
	hive_scan_string(input);
}

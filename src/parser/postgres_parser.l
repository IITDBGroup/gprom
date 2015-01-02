%option prefix="postgres" 
%option outfile="postgres_parser.lex.c"

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
#include "parser/parse_internal_postgres.h"
#include "postgres_parser.tab.h"

#undef free
#undef malloc

#define TOKSELF() { \
	TRACE_LOG("Lexed TOKENSELF <%c> with VALUE <%c>", postgrestext[0], postgrestext[0]); \
	postgreslval.stringVal = strdup(postgrestext); \
	return postgrestext[0]; \
	}
#define TOK(name) { \
	TRACE_LOG("Lexed TOKEN <%s> with VALUE <%s>", #name, postgrestext); \
 	postgreslval.stringVal = strdup(postgrestext); \
 	return name; \
 	}
#define DOWNCASE_TOK(name) { \
	TRACE_LOG("Lexed TOKEN <%s> with VALUE <%s>", #name, postgrestext); \
	char *result = strdup(postgrestext); \
	postgreslval.stringVal = result; \
	for(; *result != '\0'; (*(result) = tolower(*result)), result++); \
	return name; \
	}
#define TOKSAVE(name,field,function) { \
	TRACE_LOG("Lexed TOKEN <%s> of TYPE <%s>",  #name, #field); \
	postgreslval.field = function(postgrestext); \
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


/*******************************************************************************
 * Token definitions and actions 
 ******************************************************************************/
%%

	/* ignore all whitespace */
{whitespace}	 { TRACE_LOG("Lexed whitespace <%s>", postgrestext); /* ignore it */ }

	/* literal keyword tokens */
(?i:SELECT)       	TOK(SELECT)
(?i:INSERT)			TOK(INSERT)
(?i:UPDATE)			TOK(UPDATE)
(?i:DELETE)			TOK(DELETE)
(?i:SET)			TOK(SET)
(?i:PROVENANCE)		TOK(PROVENANCE)
(?i:OF)				TOK(OF)
(?i:FROM)			TOK(FROM)
(?i:AS)				TOK(AS)
(?i:WHERE)			TOK(WHERE)
(?i:DISTINCT)		TOK(DISTINCT)
(?i:ON)				TOK(ON)
STARLL				TOK(STARALL)
(?i:ALL)		 	TOK(ALL)
(?i:AND)		 	TOK(AND)
(?i:OR)             TOK(OR)
(?i:AVG)		 	TOK(AMMSC)
(?i:MIN)		 	TOK(AMMSC)
(?i:MAX)		 	TOK(AMMSC)
(?i:SUM)		 	TOK(AMMSC)
(?i:COUNT)		 	TOK(AMMSC)
(?i:ANY)		 	TOK(ANY)
(?i:SOME)           TOK(SOME)
(?i:BETWEEN)	 	TOK(BETWEEN)
(?i:BY)			 	TOK(BY)
(?i:IS)		     	TOK(IS)
(?i:NULL)	     	TOK(NULLVAL)
(?i:UNION)        	TOK(UNION)
(?i:INTERSECT)      TOK(INTERSECT)
(?i:MINUS)        	TOK(MINUS)
(?i:JOIN)        	TOK(JOIN)
(?i:LEFT)        	TOK(LEFT)
(?i:RIGHT)        	TOK(RIGHT)
(?i:OUTER)        	TOK(OUTER)
(?i:INNER)        	TOK(INNER)
(?i:NATURAL)		TOK(NATURAL)
(?i:USING)        	TOK(USING)
(?i:CROSS)        	TOK(CROSS)
(?i:INTO)			TOK(INTO)
(?i:VALUES)			TOK(VALUES)
(?i:IN)        		TOK(IN)
(?i:EXISTS)        	TOK(EXISTS)
(?i:LIKE)        	TOK(LIKE)
(?i:GROUP)        	TOK(GROUP)
(?i:NOT)        	TOK(NOT)
(?i:HAVING)        	TOK(HAVING)
(?i:LIMIT)        	TOK(LIMIT)
(?i:ORDER)        	TOK(ORDER)
(?i:BEGIN)        	TOK(BEGIN_TRANS)
(?i:COMMIT)       	TOK(COMMIT_TRANS)
(?i:ROLLBACK)     	TOK(ROLLBACK_TRANS)
(?i:ABORT)		 	TOK(ROLLBACK_TRANS)
(?i:BASERELATION) 	TOK(BASERELATION)
(?i:SCN)			TOK(SCN)
(?i:TIMESTAMP)		TOK(TIMESTAMP)
(?i:TYPE)			TOK(TYPE)
(?i:TRANSACTION) 	TOK(TRANSACTION)
(?i:WITH)			TOK(WITH)
(?i:HAS)			TOK(HAS)
(?i:CASE)			TOK(CASE)
(?i:WHEN)			TOK(WHEN)
(?i:THEN)			TOK(THEN)
(?i:END)			TOK(END)
(?i:ELSE)			TOK(ELSE)
(?i:OVER)			TOK(OVER_TOK)
(?i:PARTITION)		TOK(PARTITION)
(?i:ROWS)			TOK(ROWS)
(?i:RANGE)			TOK(RANGE)
(?i:UNBOUNDED)		TOK(UNBOUNDED)
(?i:PRECEDING)		TOK(PRECEDING)
(?i:CURRENT)		TOK(CURRENT)
(?i:ROW)			TOK(ROW)
(?i:FOLLOWING)		TOK(FOLLOWING)
(?i:TABLE)			TOK(TABLE)
(?i:ONLY)			TOK(ONLY)
(?i:UPDATED)		TOK(UPDATED)
(?i:SHOW)			TOK(SHOW)
(?i:INTERMEDIATE)	TOK(INTERMEDIATE)
(?i:USE)			TOK(USE)
(?i:ASC)			TOK(ASC)
(?i:DESC)			TOK(DESC)
(?i:NULLS)			TOK(NULLS)
(?i:FIRST)			TOK(FIRST)
(?i:LAST)			TOK(LAST)
(?i:TUPLE)			TOK(TUPLE)
(?i:VERSIONS)		TOK(VERSIONS)

	/* names */
{identifiers}	DOWNCASE_TOK(identifier)
{quotedIdentifier}   { 		
		postgreslval.stringVal = MALLOC(strlen(postgrestext) - 1);
		memcpy(postgreslval.stringVal, postgrestext + 1, strlen(postgrestext) - 2);
		postgreslval.stringVal[strlen(postgrestext) - 2] = '\0';
		TRACE_LOG("Lexed TOKEN <quotedIdentifier> with VALUE <%s>", postgrestext);
		return identifier;
	}	

{parameters} {
		postgreslval.stringVal = MALLOC(strlen(postgrestext));
		memcpy(postgreslval.stringVal, postgrestext + 1, strlen(postgrestext) - 1);
		postgreslval.stringVal[strlen(postgrestext) - 1] = '\0';
		TRACE_LOG("Lexed TOKEN <parameter> with VALUE <:%s>", postgrestext);
		return parameter;
	}

	/* punctuation */
[(),.;] TOKSELF()

	/* operators */
{comparisonOps}		TOK(comparisonOps)
[-+*%/^~&|!]		TOKSELF()
	


	/* numbers */
{integer}	TOKSAVE(intConst,intVal,atoi)
{float}	TOKSAVE(floatConst,floatVal,atof)

	/* strings */
'[^'\n]*'	{
		postgreslval.stringVal = MALLOC(strlen(postgrestext) - 1);
		memcpy(postgreslval.stringVal, postgrestext + 1, strlen(postgrestext) - 2);
		postgreslval.stringVal[strlen(postgrestext) - 2] = '\0';
		TRACE_LOG("Lexed TOKEN <stringConst> with VALUE <%s>", postgrestext);
		return stringConst;
	}

		
'[^'\n]*$	{postgreserror("Unterminated string"); }

	/* failure - no token matched */
.		{ postgreserror("Unknown symbol"); }	/* random non-SQL text */

%%

/*******************************************************************************
 * Literal C code to include
 ******************************************************************************/

/* use logging framework in the future */
void postgreserror(char *s)
{
	ERROR_LOG("%s at %s\n", s, postgrestext);
}

void postgresSetupStringInput(char *input)
{
	postgres_scan_string(input);
}

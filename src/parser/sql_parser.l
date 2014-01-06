/*
 *This is the lexer file which defines tokens
 *
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
#include "parser/parse_internal.h"
#include "sql_parser.tab.h"

#undef free

#define TOKSELF() { \
	TRACE_LOG("Lexed TOKENSELF <%c> with VALUE <%c>", yytext[0], yytext[0]); \
	yylval.stringVal = strdup(yytext); \
	return yytext[0]; \
	}
#define TOK(name) { \
	TRACE_LOG("Lexed TOKEN <%s> with VALUE <%s>", #name, yytext); \
 	yylval.stringVal = strdup(yytext); \
 	return name; \
 	}
#define UPCASE_TOK(name) { \
	TRACE_LOG("Lexed TOKEN <%s> with VALUE <%s>", #name, yytext); \
	char *result = strdup(yytext); \
	yylval.stringVal = result; \
	for(; *result != '\0'; (*(result) = toupper(*result)), result++); \
	return name; \
	}
#define TOKSAVE(name,field,function) { \
	TRACE_LOG("Lexed TOKEN <%s> of TYPE <%s>",  #name, #field); \
	yylval.field = function(yytext); \
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

/* operators */
/* comparisonOps	['!'|'<'|'>'|'=']{1,2} */ 
comparisonOps   ("="|"<>"|"!="|"<"|">"|"<="|">=")
/*shiftsOps		["<<"|">>"] */

/*******************************************************************************
 * Token definitions and actions 
 ******************************************************************************/
%%

	/* ignore all whitespace */
{whitespace}	 { TRACE_LOG("Lexed whitespace <%s>", yytext); /* ignore it */ }

	/* literal keyword tokens */
SELECT      TOK(SELECT)
INSERT      TOK(INSERT)
UPDATE      TOK(UPDATE)
DELETE      TOK(DELETE)
SET         TOK(SET)
PROVENANCE  TOK(PROVENANCE)
OF          TOK(OF)
FROM        TOK(FROM)
AS          TOK(AS)
WHERE       TOK(WHERE)
DISTINCT    TOK(DISTINCT)
ON          TOK(ON)
STARLL      TOK(STARALL)
ALL		 TOK(ALL)
AND		 TOK(AND)
OR               TOK(OR)
AVG		 TOK(AMMSC)
MIN		 TOK(AMMSC)
MAX		 TOK(AMMSC)
SUM		 TOK(AMMSC)
COUNT		 TOK(AMMSC)
ANY		 TOK(ANY)
SOME             TOK(SOME)
BETWEEN	 TOK(BETWEEN)
BY			 TOK(BY)
UPDATE		 TOK(UPDATE)
DELETE	     TOK(DELETE)
IS		     TOK(IS)
NULL	     TOK(NULLVAL)
UNION        TOK(UNION)
INTERSECT        TOK(INTERSECT)
MINUS        TOK(MINUS)
JOIN        TOK(JOIN)
LEFT        TOK(LEFT)
RIGHT        TOK(RIGHT)
OUTER        TOK(OUTER)
INNER        TOK(INNER)
NATURAL		TOK(NATURAL)
USING        TOK(USING)
CROSS        TOK(CROSS)
INTO		TOK(INTO)
VALUES		TOK(VALUES)
IN        TOK(IN)
EXISTS        TOK(EXISTS)
LIKE        TOK(LIKE)
NOT        TOK(NOT)
GROUP        TOK(GROUP)
HAVING        TOK(HAVING)
LIMIT        TOK(LIMIT)
ORDER        TOK(ORDER)
BEGIN        TOK(BEGIN_TRANS)
COMMIT       TOK(COMMIT_TRANS)
ROLLBACK     TOK(ROLLBACK_TRANS)
ABORT		 TOK(ROLLBACK_TRANS)
BASERELATION TOK(BASERELATION)
SCN			TOK(SCN)
TIMESTAMP	TOK(TIMESTAMP)
TYPE		TOK(TYPE)
TRANSACTION TOK(TRANSACTION)
WITH		TOK(WITH)

	/* names */
{identifiers}	UPCASE_TOK(identifier)
{quotedIdentifier}   { 		
		yylval.stringVal = MALLOC(strlen(yytext) - 1);
		memcpy(yylval.stringVal, yytext + 1, strlen(yytext) - 2);
		yylval.stringVal[strlen(yytext) -1] = '\0';
		TRACE_LOG("Lexed TOKEN <quotedIdentifier> with VALUE <%s>", yytext);
		return identifier;
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
		yylval.stringVal = MALLOC(strlen(yytext) - 1);
		memcpy(yylval.stringVal, yytext + 1, strlen(yytext) - 2);
		yylval.stringVal[strlen(yytext) -1] = '\0';
		TRACE_LOG("Lexed TOKEN <stringConst> with VALUE <%s>", yytext);
		return stringConst;
	}

		
'[^'\n]*$	{yyerror("Unterminated string"); }

	/* failure - no token matched */
.		{ yyerror("Unknown symbol"); }	/* random non-SQL text */

%%

/*******************************************************************************
 * Literal C code to include
 ******************************************************************************/

/* use logging framework in the future */
void yyerror(char *s)
{
	ERROR_LOG("%s at %s\n", s, yytext);
}

void setupStringInput(char *input)
{
	yy_scan_string(input);
}

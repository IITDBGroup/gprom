%option prefix="dl"
%option outfile="dl_parser.lex.c"

/*
 * SQL lexer file for lex/flex defining basic tokens used in the datalog parser
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
#include "parser/parse_internal_dl.h"
#include "dl_parser.tab.h"

#undef free
#undef malloc

#define TOKSELF() { \
	TRACE_LOG("Lexed TOKENSELF <%c> with VALUE <%c>", dltext[0], dltext[0]); \
	dllval.stringVal = strdup(dltext); \
	return dltext[0]; \
	}
#define TOK(name) { \
	TRACE_LOG("Lexed TOKEN <%s> with VALUE <%s>", #name, dltext); \
 	dllval.stringVal = strdup(dltext); \
 	return name; \
 	}
#define UPCASE_TOK(name) { \
	TRACE_LOG("Lexed TOKEN <%s> with VALUE <%s>", #name, dltext); \
	char *result = strdup(dltext); \
	dllval.stringVal = result; \
	for(; *result != '\0'; (*(result) = toupper(*result)), result++); \
	return name; \
	}
#define TOKSAVE(name,field,function) { \
	TRACE_LOG("Lexed TOKEN <%s> of TYPE <%s>",  #name, #field); \
	dllval.field = function(dltext); \
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
decimal 		({integer}*\.{integer}+|{integer}+\.{integer}+)
float 			(({integer}|{decimal})([eE][+-]?{integer})?)

/* names (e.g., tables, attributes) */
variableStart	[A-Z]
identStart		[A-Za-z]
constStart		[a-z]
identContinue	[A-Za-z0-9_%?]
identDelim		[.]
doublequote     [\"]
identifier		{identStart}{identContinue}*
quotedIdentifier		{doublequote}[^\"]+{doublequote}
variable		{variableStart}{identContinue}*

/* operators */
dot						'.'
colon					':'
comma					','
semicolon				';'

IMPLIES					":-"

DIVIDE					'/'
PLUS					'+'
MINUS					'-'
STAR					'*'
MOD						'%'
STRCONCAT				"||"

comparisonOps   ("="|"<>"|"!="|"<"|">"|"<="|">="|"!=")
LOG_IMPLIES             "->"

/* LITERAL ONES */

/*******************************************************************************
 * Token definitions and actions
 ******************************************************************************/
%%

	/* ignore all whitespace */
{whitespace}	 { TRACE_LOG("Lexed whitespace <%s>", dltext); /* ignore it */ }

	/* literal keyword tokens */
(?i:LIKE)           TOK(LIKE)
{IMPLIES}			TOK(RULE_IMPLICATION)
{LOG_IMPLIES}       TOK(IMPLIES)
(?i:FD)                TOK(KEYWORD_FD)
(?i:ANS)				TOK(ANS)
(?i:GP)				TOK(GP)
(?i:WHY)				TOK(WHYPROV)
(?i:WHYNOT)			TOK(WHYNOTPROV)
(?i:LINEAGE)           TOK(LINEAGE)
(?i:FOR)               TOK(FOR)
(?i:RPQ)				TOK(RPQ)
(?i:FORMAT)			TOK(FORMAT)
(?i:OF)				TOK(OF)
(?i:IS)				TOK(IS)
(?i:USERDOMAIN)		TOK(USERDOMAIN)
(?i:TOP)				TOK(TOP)
(?i:FAILURE)			TOK(FAILURE)
(?i:SUMMARIZED)		TOK(SUMMARIZED)
(?i:BY)				TOK(BY)
(?i:WITH)				TOK(WITH)
(?i:SAMPLE)			TOK(SAMPLE)
(?i:SCORE)				TOK(SCORE)
(?i:AS)				TOK(AS)
(?i:THRESHOLDS)		TOK(THRESHOLDS)
(?i:RESULTS)           TOK(RESULTS)
(?i:FROM)              TOK(FROM)
(?i:CASE)			UPCASE_TOK(CASE)
(?i:WHEN)			UPCASE_TOK(WHEN)
(?i:THEN)			UPCASE_TOK(THEN)
(?i:END)			UPCASE_TOK(END)
(?i:ELSE)			UPCASE_TOK(ELSE)
(?i:NOT)			TOK(NEGATION)
(?i:CAST)           UPCASE_TOK(CAST)

	/* aggregation functions */
(?i:AVG)	TOK(AMMSC)
(?i:MIN)	TOK(AMMSC)
(?i:MAX)	TOK(AMMSC)
(?i:SUM)	TOK(AMMSC)
(?i:COUNT)	TOK(AMMSC)

	/* comparisons */
{comparisonOps}		TOK(comparisonOp)

	/* arithmetic operator */
[-+*%/]				TOKSELF()
{STRCONCAT}		TOK(STRCONCAT)

	/* identifier types */
{identifier}	TOK(IDENT)

{quotedIdentifier}   TOK(IDENT)

[(),.;:] 			TOKSELF()

	/* constants */
{integer}	TOKSAVE(intConst,intVal,atoi)
{float}		TOKSAVE(floatConst,floatVal,atof)


'[^'\n]*'	{
		dllval.stringVal = MALLOC(strlen(dltext) - 1);
		memcpy(dllval.stringVal, dltext + 1, strlen(dltext) - 2);
		dllval.stringVal[strlen(dltext) - 2] = '\0';
		TRACE_LOG("Lexed TOKEN <stringConst> with VALUE <%s>", dltext);
		return stringConst;
	}


'[^'\n]*$	{ dlerror("Unterminated string"); }

. 			{ dlerror("not matching any token"); }

%%

/*******************************************************************************
 * Literal C code to include
 ******************************************************************************/

	/* use logging framework in the future */
void dlerror(char *s)
{
	ERROR_LOG("%s at %s\n", s, dltext);
}

void dlSetupStringInput(char *input)
{
	dl_scan_string(input);
}

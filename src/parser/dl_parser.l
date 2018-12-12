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

comparisonOps   ("="|"<>"|"!="|"<"|">"|"<="|">=")

/* KEYWORDS */
ANS					"ANS"
WHY					"WHY"
WHYNOT				"WHYNOT"
GP					"GP"
RPQ					"RPQ"
FORMAT				"FORMAT"
OF					"OF"
IS					"IS"
USERDOMAIN			"USERDOMAIN"
TOP					"TOP"
FOR					"FOR"
FAILURE				"FAILURE"
SUMMARIZED			"SUMMARIZED"
BY					"BY"
WITH				"WITH"
SAMPLE				"SAMPLE"
SCORE				"SCORE"
AS					"AS"
THRESHOLDS			"THRESHOLDS"

/* LITERAL ONES */

/*******************************************************************************
 * Token definitions and actions 
 ******************************************************************************/
%%

	/* ignore all whitespace */
{whitespace}	 { TRACE_LOG("Lexed whitespace <%s>", dltext); /* ignore it */ }

	/* literal keyword tokens */
(?i:not)			TOK(NEGATION)
{IMPLIES}			TOK(RULE_IMPLICATION)
{ANS}				TOK(ANS)
{GP}				TOK(GP)
{WHY}				TOK(WHYPROV)
{WHYNOT}			TOK(WHYNOTPROV)
{RPQ}				TOK(RPQ)
{FORMAT}			TOK(FORMAT)
{OF}				TOK(OF)
{IS}				TOK(IS)
{USERDOMAIN}		TOK(USERDOMAIN)
{TOP}				TOK(TOP)
{FOR}				TOK(FOR)
{FAILURE}			TOK(FAILURE)
{SUMMARIZED}		TOK(SUMMARIZED)
{BY}				TOK(BY)
{WITH}				TOK(WITH)
{SAMPLE}			TOK(SAMPLE)
{SCORE}				TOK(SCORE)
{AS}				TOK(AS)
{THRESHOLDS}		TOK(THRESHOLDS)


	/* aggregation functions */
(?i:AVG)	TOK(AMMSC)
(?i:MIN)	TOK(AMMSC)
(?i:MAX)	TOK(AMMSC)
(?i:SUM)	TOK(AMMSC)
(?i:COUNT)	TOK(AMMSC)

	/* identifier types */
{identifier}	UPCASE_TOK(IDENT)
	/* {variable}	 	UPCASE_TOK(VARIDENT)*/

{quotedIdentifier}   { 		
		dllval.stringVal = MALLOC(strlen(dltext) - 1);
		memcpy(dllval.stringVal, dltext + 1, strlen(dltext) - 2);
		dllval.stringVal[strlen(dltext) - 2] = '\0';
		TRACE_LOG("Lexed TOKEN <quotedIdentifier> with VALUE <%s>", dltext);
		return IDENT;
	}	

[(),.;:] 			TOKSELF()

	/* comparisons */
{comparisonOps}		TOK(comparisonOp)

	/* arithmetic operator */
[-+*%/]				TOKSELF()
{STRCONCAT}		TOK(STRCONCAT)

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

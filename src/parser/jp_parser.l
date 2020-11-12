%option prefix="jp" 
%option outfile="jp_parser.lex.c"

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
#include "parser/parse_internal_jp.h"
#include "jp_parser.tab.h"

#undef free
#undef malloc

#define TOKSELF() { \
	TRACE_LOG("Lexed TOKENSELF <%c> with VALUE <%c>", jptext[0], jptext[0]); \
	jplval.stringVal = strdup(jptext); \
	return jptext[0]; \
	}
#define TOK(name) { \
	TRACE_LOG("Lexed TOKEN <%s> with VALUE <%s>", #name, jptext); \
 	jplval.stringVal = strdup(jptext); \
 	return name; \
 	}
#define UPCASE_TOK(name) { \
	TRACE_LOG("Lexed TOKEN <%s> with VALUE <%s>", #name, jptext); \
	char *result = strdup(jptext); \
	jplval.stringVal = result; \
	for(; *result != '\0'; (*(result) = toupper(*result)), result++); \
	return name; \
	}
#define TOKSAVE(name,field,function) { \
	TRACE_LOG("Lexed TOKEN <%s> of TYPE <%s>",  #name, #field); \
	jplval.field = function(jptext); \
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
variableStart	[A-Z]
identStart		[A-Za-z]
constStart		[a-z]
identContinue	[A-Za-z0-9_%?]
identDelim		[.]
doublequote     [\"]
identifier		{identStart}{identContinue}*
quotedIdentifier		{doublequote}[^\"]+{doublequote}


/* operators */
dot					'.'
colon					':' 
comma					',' 
semicolon				';' 

IMPLIES					":-"

DIVIDE					'/'
PLUS					'+'
MINUS					'-'
STAR					'*'
MOD					'%'


comparisonOps   ("="|"<>"|"!="|"<"|">"|"<="|">=")
arithmeticOps	({PLUS}|{MINUS}|{STAR}|{MOD})

/* KEYWORDS */


/* LITERAL ONES */

/*******************************************************************************
 * Token definitions and actions 
 ******************************************************************************/
%%

	/* ignore all whitespace */
{whitespace}	 { TRACE_LOG("Lexed whitespace <%s>", jptext); /* ignore it */ }


	/* literal keyword tokens */

{identifier}            TOK(IDENT)
{integer}	TOKSAVE(intConst,intVal,atoi)


[(),.;*:$\[\]] 			TOKSELF()




	
'[^'\n]*'	{
		jplval.stringVal = MALLOC(strlen(jptext) - 1);
		memcpy(jplval.stringVal, jptext + 1, strlen(jptext) - 2);
		jplval.stringVal[strlen(jptext) - 2] = '\0';
		TRACE_LOG("Lexed TOKEN <stringConst> with VALUE <%s>", jptext);
		return stringConst;
	}

		
'[^'\n]*$	{ jperror("Unterminated string"); }

. 			{ jperror("not matching any token"); }


%%

/*******************************************************************************
 * Literal C code to include
 ******************************************************************************/

	/* use logging framework in the future */
void jperror(char *s)
{
	ERROR_LOG("%s at %s\n", s, jptext);
}

void jpSetupStringInput(char *input)
{
	jp_scan_string(input);
}

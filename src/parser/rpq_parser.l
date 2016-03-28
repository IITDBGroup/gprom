%option prefix="rpq" 
%option outfile="rpq_parser.lex.c"

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
#include "parser/parse_internal_rpq.h"
#include "rpq_parser.tab.h"

#undef free
#undef malloc

#define TOKSELF() { \
	TRACE_LOG("Lexed TOKENSELF <%c> with VALUE <%c>", rpqtext[0], rpqtext[0]); \
	rpqlval.stringVal = strdup(rpqtext); \
	return rpqtext[0]; \
	}
#define TOK(name) { \
	TRACE_LOG("Lexed TOKEN <%s> with VALUE <%s>", #name, rpqtext); \
 	rpqlval.stringVal = strdup(rpqtext); \
 	return name; \
 	}
#define UPCASE_TOK(name) { \
	TRACE_LOG("Lexed TOKEN <%s> with VALUE <%s>", #name, rpqtext); \
	char *result = strdup(rpqtext); \
	rpqlval.stringVal = result; \
	for(; *result != '\0'; (*(result) = toupper(*result)), result++); \
	return name; \
	}
#define TOKSAVE(name,field,function) { \
	TRACE_LOG("Lexed TOKEN <%s> of TYPE <%s>",  #name, #field); \
	rpqlval.field = function(rpqtext); \
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
identContinue	[A-Za-z0-9_%]
doublequote     [\"]
identifier		{identStart}{identContinue}*
quotedIdentifier	{doublequote}[^\"]+{doublequote}


/* operators */
DOT						[.]
PLUS					[+]
STAR					[*]
OR						[|]
OPTIONAL				[?]

/* KEYWORDS */


/* LITERAL ONES */

/*******************************************************************************
 * Token definitions and actions 
 ******************************************************************************/
%%

	/* ignore all whitespace */
{whitespace}	 { TRACE_LOG("Lexed whitespace <%s>", rpqtext); /* ignore it */ }


	/* literal keyword tokens */

{identifier}            TOK(IDENT)
{integer}				TOKSAVE(intConst,intVal,atoi)
{DOT}					TOK(DOT)
{PLUS}					TOK(PLUS)
{STAR}					TOK(STAR)
{OR}					TOK(OR)
{OPTIONAL}				TOK(OPTIONAL)

[()] 			TOKSELF()

	
'[^'\n]*'	{
		rpqlval.stringVal = MALLOC(strlen(rpqtext) - 1);
		memcpy(rpqlval.stringVal, rpqtext + 1, strlen(rpqtext) - 2);
		rpqlval.stringVal[strlen(rpqtext) - 2] = '\0';
		TRACE_LOG("Lexed TOKEN <stringConst> with VALUE <%s>", rpqtext);
		return stringConst;
	}

		
'[^'\n]*$	{ rpqerror("Unterminated string"); }

. 			{ rpqerror("not matching any token"); }


%%

/*******************************************************************************
 * Literal C code to include
 ******************************************************************************/

	/* use logging framework in the future */
void rpqerror(char *s)
{
	ERROR_LOG("%s at %s\n", s, rpqtext);
}

void rpqSetupStringInput(char *input)
{
	rpq_scan_string(input);
}

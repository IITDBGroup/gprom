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
#include <stdio.h>
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "parser/parse_internal.h"
#include "sql_parser.tab.h"

#define TOK(name) { yylval.stringVal = yytext; return name; }
#define TOKSAVE(name,field,function) { yylval.field = function(yytext); return name; }
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
identContinue	[A-Za-z0-9_]
identifiers		({identStart}{identContinue}+)

/* operators */
comparisonops	("="|"<>"|"!="|"<"|">"|"<="|">=")
arithmeticops	("+"|"-"|"/"|"*"|"%"|"&"|"|"|"^")

/*******************************************************************************
 * Token definitions and actions 
 ******************************************************************************/
%%

EXEC[ \t]+SQL	{ BEGIN SQL; }

	/* ignore all whitespace */
{whitespace}	 { /* ignore it */ }

	/* literal keyword tokens */
<SQL>SELECT      TOK(SELECT)
<SQL>PROVENANCE  TOK(PROVENANCE)
<SQL>OF          TOK(OF)
<SQL>FROM        TOK(FROM)
<SQL>AS          TOK(AS)
<SQL>WHERE       TOK(WHERE)
<SQL>DISTINCT    TOK(DISTINCT)
<SQL>ON          TOK(ON)
<SQL>STARLL      TOK(STARALL)
<SQL>ALL		 TOK(ALL)
<SQL>AND		 TOK(AND)
<SQL>AVG		 TOK(AMMSC)
<SQL>MIN		 TOK(AMMSC)
<SQL>MAX		 TOK(AMMSC)
<SQL>SUM		 TOK(AMMSC)
<SQL>COUNT		 TOK(AMMSC)
<SQL>ANY		 TOK(ANY)
<SQL>BETWEEN	 TOK(BETWEEN)
<SQL>BY			 TOK(BY)
<SQL>UPDATE		 TOK(UPDATE)
<SQL>DELETE	     TOK(DELETE)
<SQL>IS		     TOK(IS)
<SQL>NULL	     TOK(NULLVAL)

	/* punctuation */
<SQL>[(),.;] TOK(yytext[0])

	/* operators */
<SQL>{comparisonops}		TOK(comparisonop)
<SQL>{arithmeticops}		TOK(arithmeticop)
	
	/* names */
<SQL>{identifiers}	TOK(identifier)

	/* numbers */
<SQL>{integer}	TOKSAVE(intConst,intVal,atoi)
<SQL>{float}	TOKSAVE(floatConst,floatVal,atof)

	/* strings */
<SQL>'[^'\n]*'	{
		int c = input();
		unput(c);	
		if(c != '\'') {
			yylval.stringVal = strdup(yytext);
			return stringConst;
		} else
			yymore();
	}

		
<SQL>'[^'\n]*$	{yyerror("Unterminated string"); }

	/* failure - no token matched */
.		ECHO;	/* random non-SQL text */

%%

/*******************************************************************************
 * Literal C code to include
 ******************************************************************************/


/* */
int yywarp(void)
{
	return 1;
}

/* use logging framework in the future */
void yyerror(char *s)
{
	printf("%s at %s\n", s, yytext);
}

/* dummy main method for testing */
int main()
{
	if(!yyparse())
		fprintf(stderr, "SQL parse worked\n");
	else
		fprintf(stderr, "SQL parse failed\n");
	return 0;
}
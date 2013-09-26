/*
 *This is the lexer file which defines tokens
 *
 *
*/

%{

#include <stdio.h>
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"

int lineno = 1;
void yyerror(char *s);
int yywarp(void);
int yylex(void);

#define SV save_str(yytext)
#define TOK(name) { return name; }
#define TOKSAVE(name,field) { yylval.field = yytext; return name; }
%}

/*******************************************************************************
 * options section 
 ******************************************************************************/

%s SQL

/********************************************************************
 * lexer states 
 */

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
identifier		({identStart}{identContinue}+)

/*******************************************************************************
 * Token definitions and actions 
 ******************************************************************************/
%%

EXEC[ \t]+SQL	{BEGIN SQL; yylval.clear(); start_save();}

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

	/* punctuation */

<SQL>"="	|
<SQL>"<>" 	|
<SQL>"<"	|
<SQL>">"	|
<SQL>"<="	|
<SQL>">="		TOK(COMPARISON)

<SQL>[-+*/(),.;] TOK(yytext[0])

	/* names */
<SQL>identifier	TOK(IDENTIFIER)

/* numbers */

<SQL>{integer}	TOK(intConst)

<SQL>{float}	TOK(floatConst)

/* strings */

<SQL>'[^'\n]*'	{
		int c = input();
		unput(c);	
		if(c != '\'') {
			 SV;
			yylval.strval = strdup(yytext);
			return STRING;
		} else
			yymore();
	}

		
<SQL>'[^'\n]*$	{yyerror("Unterminated string"); }

<SQL>\n		{ save_str(" ");lineno++; }
\n		{ lineno++; ECHO; }

<SQL>[ \t\r]+	save_str(" ");	/* white space */

/* failure - no token matched */
.		ECHO;	/* random non-SQL text */

/*******************************************************************************
 * Literal C code to include
 ******************************************************************************/
%%

int yywarp(void)
{
	return 1;
}


void yyerror(char *s)
{
	printf("%d: %s at %s\n", lineno, s, yytext);
}

int main()
{
	if(!yyparse())
		fprintf(stderr, "SQL parse worked\n");
	else
		fprintf(stderr, "SQL parse failed\n");
	return 0;
} /* main */

/* leave SQL lexing mode */
un_sql()
{
	BEGIN INITIAL;
} /* un_sql */
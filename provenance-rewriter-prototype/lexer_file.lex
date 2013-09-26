/*
 *This is the lexer file which defines tokens
 *
*/

%{
#include "express.h"
#include "list.h"
#include "expression.h"	
#include "list.h"
#include <string.h>
#include <stdio.h>

int lineno = 1;
void yyerror(char *s);
int yywarp(void);
int yylex(void);

	/* macro to save the text of a SQL token */
#define SV save_str(yytext)

	/* macro to save the text and return a token */
#define TOK(name) { SV;return name; }
%}

%s SQL
%%

EXEC[ \t]+SQL	{BEGIN SQL; yylval.clear(); start_save();}

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
<SQL>[A-Za-z][A-Za-z0-9_]*	TOK(tableName)

	/* numbers */

<SQL>[0-9]+	|
<SQL>[0-9]+"."[0-9]* |
<SQL>"."[0-9]*		TOK(intConst)

<SQL>[0-9]+[eE][+-]?[0-9]+	|
<SQL>[0-9]+"."[0-9]*[eE][+-]?[0-9]+ |
<SQL>"."[0-9]*[eE][+-]?[0-9]+	TOK(floatConst)

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

<SQL>"--".*	;	/* comment */

.		ECHO;	/* random non-SQL text */
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
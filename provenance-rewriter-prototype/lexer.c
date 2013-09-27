/*
 *This is the lexer file which defines tokens
 *
 *
*/

%{
#include <stdio.h>
#include <conio.h>
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
int lineno = 1;
void yyerror(char *s);
int yywarp(void);
int yylex(void);
#define SV save_str(yytext)
#define TOK(name) { SV; return name; }
%}

%s SQL
%%

EXEC[ \t]+SQL	{BEGIN SQL;}

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
<SQL>[A-Za-z][A-Za-z0-9_]*	 {yyval.strval = strdup(yytext); return tablename}

	/* numbers */

<SQL>[0-9]+	|
<SQL>[0-9]+"."[0-9]* |
<SQL>"."[0-9]* {yylval.intval = atoi(yytext); return intConst}

<SQL>[0-9]+[eE][+-]?[0-9]+	|
<SQL>[0-9]+"."[0-9]*[eE][+-]?[0-9]+ |
<SQL>"."[0-9]*[eE][+-]?[0-9]+	{yylval.floatval = atof(yyteext); return floatConst}

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
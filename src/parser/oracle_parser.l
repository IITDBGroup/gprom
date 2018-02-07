%option prefix="oracle" 
%option outfile="oracle_parser.lex.c"

/*
 * SQL lexer file for lex/flex defining basic tokens used in the oracle parser
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
#include "parser/parse_internal_oracle.h"
#include "oracle_parser.tab.h"

#undef free
#undef malloc

#define TOKSELF() { \
	TRACE_LOG("Lexed TOKENSELF <%c> with VALUE <%c>", oracletext[0], oracletext[0]); \
	oraclelval.stringVal = strdup(oracletext); \
	return oracletext[0]; \
	}
#define TOK(name) { \
	TRACE_LOG("Lexed TOKEN <%s> with VALUE <%s>", #name, oracletext); \
 	oraclelval.stringVal = strdup(oracletext); \
 	return name; \
 	}
#define UPCASE_TOK(name) { \
	TRACE_LOG("Lexed TOKEN <%s> with VALUE <%s>", #name, oracletext); \
	char *result = strdup(oracletext); \
	oraclelval.stringVal = result; \
	for(; *result != '\0'; (*(result) = toupper(*result)), result++); \
	TRACE_LOG("Upcased TOKEN <%s>", oraclelval.stringVal); \
	return name; \
	}
#define TOKSAVE(name,field,function) { \
	TRACE_LOG("Lexed TOKEN <%s> of TYPE <%s>",  #name, #field); \
	oraclelval.field = function(oracletext); \
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
bool			("TRUE"|"FALSE")
/* names (e.g., tables, attributes) */
identStart		[A-Za-z]
identContinue	[A-Za-z0-9_%?]
identDelim		[.]
doublequote     [\"]
quotedIdentifier		{doublequote}[^\"]+{doublequote}
identifiers				({identStart}{identContinue}*)
identifierPart			({identifiers}|{quotedIdentifier})
compositeIdentifier		({identifierPart}({identDelim}{identifierPart})+)
parameters		[:]({identifiers}|{integer})

/* operators */
/* comparisonOps	['!'|'<'|'>'|'=']{1,2} */ 
comparisonOps   ("="|"<>"|"!="|"<"|">"|"<="|">=")
/*shiftsOps		("<<"|">>") */
/* string concatenation */
strconcat		"||"

/*******************************************************************************
 * Token definitions and actions 
 ******************************************************************************/
%%

	/* ignore all whitespace */
{whitespace}	 { TRACE_LOG("Lexed whitespace <%s>", oracletext); /* ignore it */ }

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
(?i:EXCEPT)        	TOK(EXCEPT)
(?i:JOIN)        	TOK(JOIN)
(?i:LEFT)        	TOK(LEFT)
(?i:RIGHT)        	TOK(RIGHT)
(?i:OUTER)        	TOK(OUTER)
(?i:FULL)			TOK(FULL)
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
(?i:JSON)           TOK(JSON)
(?i:TRANSLATE)      TOK(TRANSLATE)
(?i:JSON_TABLE)     TOK(JSON_TABLE)
(?i:COLUMNS)        TOK(COLUMNS)
(?i:PATH)           TOK(PATH)
(?i:FORMAT)         TOK(FORMAT)
(?i:WITHOUT)        TOK(WITHOUT)
(?i:WRAPPER)        TOK(WRAPPER)
(?i:NESTED)         TOK(NESTED)
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
(?i:ROWNUM)			TOK(ROWNUM)
(?i:STATEMENT)		TOK(STATEMENT)
(?i:ANNOTATIONS)	TOK(ANNOTATIONS)
(?i:NO)				TOK(NO)
(?i:CAST)			TOK(CAST)
(?i:ALTER)			TOK(ALTER)
(?i:ADD)			TOK(ADD)
(?i:REMOVE)			TOK(REMOVE)
(?i:COLUMN)			TOK(COLUMN)
(?i:CREATE)			TOK(CREATE)
(?i:REENACT)		TOK(REENACT)
(?i:SUMMARIZED)		TOK(SUMMARIZED)
(?i:TO)				TOK(TO)
(?i:EXPLAIN)		TOK(EXPLAIN)
(?i:SAMPLE)			TOK(SAMPLE)
(?i:TOP)			TOK(TOP)
(?i:ISOLATION)		TOK(ISOLATION)
(?i:LEVEL)			TOK(LEVEL)
(?i:OPTIONS)		TOK(OPTIONS)
(?i:SEQUENCED)		TOK(SEQUENCED)
(?i:TEMPORAL)		TOK(TEMPORAL)
(?i:TIME)			TOK(TIME)
(?i:SEMIRING)		TOK(SEMIRING)
(?i:COMBINER)		TOK(COMBINER)
(?i:MULT)           TOK(MULT)
(?i:UNCERTAIN)		TOK(UNCERTAIN)
(?i:CAPTURE)		TOK(CAPTURE)
(?i:COARSE)			TOK(COARSE)
(?i:GRAINED)		TOK(GRAINED)
(?i:FRAGMENT)		TOK(FRAGMENT)
(?i:PAGE)		TOK(PAGE)
(?i:RANGESA)		TOK(RANGESA)
(?i:RANGESB)		TOK(RANGESB)

	/* booleans */
{bool}			TOK(boolConst)

	/* names */
{identifiers}	UPCASE_TOK(identifier)
{compositeIdentifier}	UPCASE_TOK(compositeIdentifier)

{quotedIdentifier}   { 		
		oraclelval.stringVal = MALLOC(strlen(oracletext) - 1);
		memcpy(oraclelval.stringVal, oracletext + 1, strlen(oracletext) - 2);
		oraclelval.stringVal[strlen(oracletext) - 2] = '\0';
		TRACE_LOG("Lexed TOKEN <quotedIdentifier> with VALUE <%s>", oracletext);
		return identifier;
	}	

{parameters} {
		oraclelval.stringVal = MALLOC(strlen(oracletext));
		memcpy(oraclelval.stringVal, oracletext + 1, strlen(oracletext) - 1);
		oraclelval.stringVal[strlen(oracletext) - 1] = '\0';
		TRACE_LOG("Lexed TOKEN <parameter> with VALUE <:%s>", oracletext);
		return parameter;
	}

	/* punctuation */
[(),.;] TOKSELF()

	/* operators */
{comparisonOps}		TOK(comparisonOps)
{strconcat}			TOK(STRINGCONCAT)
[-+*%/^~&|!]		TOKSELF()



	/* numbers */
{integer}	TOKSAVE(intConst,intVal,atoi)
{float}		TOKSAVE(floatConst,floatVal,atof)

	/* strings */
'[^'\n]*'	{
		oraclelval.stringVal = MALLOC(strlen(oracletext) - 1);
		memcpy(oraclelval.stringVal, oracletext + 1, strlen(oracletext) - 2);
		oraclelval.stringVal[strlen(oracletext) - 2] = '\0';
		TRACE_LOG("Lexed TOKEN <stringConst> (quoted) with VALUE <%s>", oracletext);
		return stringConst;
	}

		
'[^'\n]*$	{ oracleerror("Unterminated string"); }

	/* failure - no token matched */
.		{ oracleerror("Unknown symbol"); }	/* random non-SQL text */

%%

/*******************************************************************************
 * Literal C code to include
 ******************************************************************************/

/* use logging framework in the future */
void oracleerror(char *s)
{
	FATAL_LOG("%s at %s\n", s, oracletext);
}

void oracleSetupStringInput(char *input)
{
	oracle_scan_string(input);
}

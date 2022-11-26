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
#define TOK(name) {													  \
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
postgrescast    "::"

/*******************************************************************************
 * Token definitions and actions
 ******************************************************************************/
%%

	/* ignore all whitespace */
{whitespace}	 { TRACE_LOG("Lexed whitespace <%s>", oracletext); /* ignore it */ }

	/* literal keyword tokens */
(?i:SELECT)       	UPCASE_TOK(SELECT)
(?i:INSERT)			UPCASE_TOK(INSERT)
(?i:UPDATE)			UPCASE_TOK(UPDATE)
(?i:DELETE)			UPCASE_TOK(DELETE)
(?i:SET)			UPCASE_TOK(SET)
(?i:PROVENANCE)		UPCASE_TOK(PROVENANCE)
(?i:OF)				UPCASE_TOK(OF)
(?i:FROM)			UPCASE_TOK(FROM)
(?i:AS)				UPCASE_TOK(AS)
(?i:WHERE)			UPCASE_TOK(WHERE)
(?i:DISTINCT)		UPCASE_TOK(DISTINCT)
(?i:ON)				UPCASE_TOK(ON)
(?i:ALL)		 	UPCASE_TOK(ALL)
(?i:AND)		 	UPCASE_TOK(AND)
(?i:OR)             UPCASE_TOK(OR)
(?i:AVG)		 	UPCASE_TOK(AMMSC)
(?i:MIN)		 	UPCASE_TOK(AMMSC)
(?i:MAX)		 	UPCASE_TOK(AMMSC)
(?i:SUM)		 	UPCASE_TOK(AMMSC)
(?i:COUNT)		 	UPCASE_TOK(AMMSC)
(?i:ANY)		 	UPCASE_TOK(ANY)
(?i:SOME)           UPCASE_TOK(SOME)
(?i:BETWEEN)	 	UPCASE_TOK(BETWEEN)
(?i:BY)			 	UPCASE_TOK(BY)
(?i:IS)		     	UPCASE_TOK(IS)
(?i:NULL)	     	UPCASE_TOK(NULLVAL)
(?i:UNION)        	UPCASE_TOK(UNION)
(?i:INTERSECT)      UPCASE_TOK(INTERSECT)
(?i:MINUS)        	UPCASE_TOK(MINUS)
(?i:EXCEPT)        	UPCASE_TOK(EXCEPT)
(?i:JOIN)        	UPCASE_TOK(JOIN)
(?i:LEFT)        	UPCASE_TOK(LEFT)
(?i:RIGHT)        	UPCASE_TOK(RIGHT)
(?i:OUTER)        	UPCASE_TOK(OUTER)
(?i:FULL)			UPCASE_TOK(FULL)
(?i:INNER)        	UPCASE_TOK(INNER)
(?i:NATURAL)		UPCASE_TOK(NATURAL)
(?i:USING)        	UPCASE_TOK(USING)
(?i:CROSS)        	UPCASE_TOK(CROSS)
(?i:INTO)			UPCASE_TOK(INTO)
(?i:VALUES)			UPCASE_TOK(VALUES)
(?i:IN)        		UPCASE_TOK(IN)
(?i:EXISTS)        	UPCASE_TOK(EXISTS)
(?i:LIKE)        	UPCASE_TOK(LIKE)
(?i:GROUP)        	UPCASE_TOK(GROUP)
(?i:NOT)        	UPCASE_TOK(NOT)
(?i:HAVING)        	UPCASE_TOK(HAVING)
(?i:LIMIT)        	UPCASE_TOK(LIMIT)
(?i:OFFSET)        	UPCASE_TOK(OFFSET)
(?i:ORDER)        	UPCASE_TOK(ORDER)
(?i:BEGIN)        	UPCASE_TOK(BEGIN_TRANS)
(?i:COMMIT)       	UPCASE_TOK(COMMIT_TRANS)
(?i:ROLLBACK)     	UPCASE_TOK(ROLLBACK_TRANS)
(?i:ABORT)		 	UPCASE_TOK(ROLLBACK_TRANS)
(?i:BASERELATION) 	UPCASE_TOK(BASERELATION)
(?i:SCN)			UPCASE_TOK(SCN)
(?i:TIMESTAMP)		UPCASE_TOK(TIMESTAMP)
(?i:TYPE)			UPCASE_TOK(TYPE)
(?i:TRANSACTION) 	UPCASE_TOK(TRANSACTION)
(?i:WITH)			UPCASE_TOK(WITH)
(?i:HAS)			UPCASE_TOK(HAS)
(?i:CASE)			UPCASE_TOK(CASE)
(?i:WHEN)			UPCASE_TOK(WHEN)
(?i:THEN)			UPCASE_TOK(THEN)
(?i:END)			UPCASE_TOK(END)
(?i:ELSE)			UPCASE_TOK(ELSE)
(?i:OVER)			UPCASE_TOK(OVER_TOK)
(?i:PARTITION)		UPCASE_TOK(PARTITION)
(?i:ROWS)			UPCASE_TOK(ROWS)
(?i:RANGE)			UPCASE_TOK(RANGE)
(?i:UNBOUNDED)		UPCASE_TOK(UNBOUNDED)
(?i:PRECEDING)		UPCASE_TOK(PRECEDING)
(?i:CURRENT)		UPCASE_TOK(CURRENT)
(?i:ROW)			UPCASE_TOK(ROW)
(?i:FOLLOWING)		UPCASE_TOK(FOLLOWING)
(?i:JSON)           UPCASE_TOK(JSON)
(?i:TRANSLATE)      UPCASE_TOK(TRANSLATE)
(?i:JSON_TABLE)     UPCASE_TOK(JSON_TABLE)
(?i:COLUMNS)        UPCASE_TOK(COLUMNS)
(?i:PATH)           UPCASE_TOK(PATH)
(?i:FORMAT)         UPCASE_TOK(FORMAT)
(?i:WITHOUT)        UPCASE_TOK(WITHOUT)
(?i:WRAPPER)        UPCASE_TOK(WRAPPER)
(?i:NESTED)         UPCASE_TOK(NESTED)
(?i:TABLE)			UPCASE_TOK(TABLE)
(?i:ONLY)			UPCASE_TOK(ONLY)
(?i:UPDATED)		UPCASE_TOK(UPDATED)
(?i:SHOW)			UPCASE_TOK(SHOW)
(?i:INTERMEDIATE)	UPCASE_TOK(INTERMEDIATE)
(?i:USE)			UPCASE_TOK(USE)
(?i:ASC)			UPCASE_TOK(ASC)
(?i:DESC)			UPCASE_TOK(DESC)
(?i:NULLS)			UPCASE_TOK(NULLS)
(?i:FIRST)			UPCASE_TOK(FIRST)
(?i:LAST)			UPCASE_TOK(LAST)
(?i:TUPLE)			UPCASE_TOK(TUPLE)
(?i:VERSIONS)		UPCASE_TOK(VERSIONS)
(?i:ROWNUM)			UPCASE_TOK(ROWNUM)
(?i:STATEMENT)		UPCASE_TOK(STATEMENT)
(?i:ANNOTATIONS)	UPCASE_TOK(ANNOTATIONS)
(?i:NO)				UPCASE_TOK(NO)
(?i:CAST)			UPCASE_TOK(CAST)
(?i:ALTER)			UPCASE_TOK(ALTER)
(?i:ADD)			UPCASE_TOK(ADD)
(?i:REMOVE)			UPCASE_TOK(REMOVE)
(?i:COLUMN)			UPCASE_TOK(COLUMN)
(?i:CREATE)			UPCASE_TOK(CREATE)
(?i:REENACT)		UPCASE_TOK(REENACT)
(?i:SUMMARIZED)		UPCASE_TOK(SUMMARIZED)
(?i:TO)				UPCASE_TOK(TO)
(?i:EXPLAIN)		UPCASE_TOK(EXPLAIN)
(?i:SAMPLE)			UPCASE_TOK(SAMPLE)
(?i:TOP)			UPCASE_TOK(TOP)
(?i:ISOLATION)		UPCASE_TOK(ISOLATION)
(?i:LEVEL)			UPCASE_TOK(LEVEL)
(?i:OPTIONS)		UPCASE_TOK(OPTIONS)
(?i:SEQUENCED)		UPCASE_TOK(SEQUENCED)
(?i:TEMPORAL)		UPCASE_TOK(TEMPORAL)
(?i:TIME)			UPCASE_TOK(TIME)
(?i:SEMIRING)		UPCASE_TOK(SEMIRING)
(?i:COMBINER)		UPCASE_TOK(COMBINER)
(?i:MULT)           UPCASE_TOK(MULT)
(?i:UNCERTAIN)		UPCASE_TOK(UNCERTAIN)
(?i:TIP)			UPCASE_TOK(TIP)
(?i:INCOMPLETE)		UPCASE_TOK(INCOMPLETE)
(?i:XTABLE) 		UPCASE_TOK(XTABLE)
(?i:URANGE)			UPCASE_TOK(URANGE)
(?i:RADB)			UPCASE_TOK(RADB)
(?i:UADB)			UPCASE_TOK(UADB)
(?i:VTABLE)			UPCASE_TOK(VTABLE)
(?i:CAPTURE)		UPCASE_TOK(CAPTURE)
(?i:COARSE)			UPCASE_TOK(COARSE)
(?i:GRAINED)		UPCASE_TOK(GRAINED)
(?i:FRAGMENT)		UPCASE_TOK(FRAGMENT)
(?i:PAGE)		    UPCASE_TOK(PAGE)
(?i:RANGESA)		UPCASE_TOK(RANGESA)
(?i:RANGESB)		UPCASE_TOK(RANGESB)
(?i:HASH)		    UPCASE_TOK(HASH)
(?i:CAPTUREUSE)		UPCASE_TOK(CAPTUREUSE)
(?i:BIND)		    UPCASE_TOK(BIND)
(?i:FOR)		    UPCASE_TOK(FOR)
(?i:CANUSE)		    UPCASE_TOK(CANUSE)
(?i:LATERAL)        UPCASE_TOK(LATERAL)
(?i:EXECUTE)        UPCASE_TOK(EXECUTE)
(?i:PREPARE)        UPCASE_TOK(PREPARE)

	/* booleans */
{bool}			TOK(boolConst)

	/* names */
{identifiers}			TOK(identifier)
{compositeIdentifier}	TOK(compositeIdentifier)

{quotedIdentifier}  		TOK(identifier)

{parameters} {
		oraclelval.stringVal = MALLOC(strlen(oracletext));
		memcpy(oraclelval.stringVal, oracletext + 1, strlen(oracletext) - 1);
		oraclelval.stringVal[strlen(oracletext) - 1] = '\0';
		TRACE_LOG("Lexed TOKEN <parameter> with VALUE <:%s>", oracletext);
		return parameter;
	}

	/* punctuation */
[(),.;\[\]] TOKSELF()

	/* operators */
{comparisonOps}		TOK(comparisonOps)
{strconcat}			TOK(STRINGCONCAT)
{postgrescast}      TOK(POSTGRESCAST)
[-+*%/^~&|![\]]		TOKSELF()



	/* numbers */
{integer}	TOKSAVE(intConst,intVal,atoll)
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
void oracleerror(const char *s)
{
	FATAL_LOG("%s at %s\n", s, oracletext);
}

void oracleSetupStringInput(char *input)
{
	oracle_scan_string(input);
}

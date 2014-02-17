/* A Bison parser, made by GNU Bison 3.0.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_SQL_PARSER_TAB_H_INCLUDED
# define YY_YY_SQL_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    intConst = 258,
    floatConst = 259,
    stringConst = 260,
    identifier = 261,
    parameter = 262,
    comparisonOps = 263,
    SELECT = 264,
    INSERT = 265,
    UPDATE = 266,
    DELETE = 267,
    PROVENANCE = 268,
    OF = 269,
    BASERELATION = 270,
    SCN = 271,
    TIMESTAMP = 272,
    HAS = 273,
    TABLE = 274,
    FROM = 275,
    AS = 276,
    WHERE = 277,
    DISTINCT = 278,
    STARALL = 279,
    AND = 280,
    OR = 281,
    LIKE = 282,
    NOT = 283,
    IN = 284,
    ISNULL = 285,
    BETWEEN = 286,
    EXCEPT = 287,
    EXISTS = 288,
    AMMSC = 289,
    NULLVAL = 290,
    ALL = 291,
    ANY = 292,
    IS = 293,
    SOME = 294,
    UNION = 295,
    INTERSECT = 296,
    MINUS = 297,
    INTO = 298,
    VALUES = 299,
    HAVING = 300,
    GROUP = 301,
    ORDER = 302,
    BY = 303,
    LIMIT = 304,
    SET = 305,
    INT = 306,
    BEGIN_TRANS = 307,
    COMMIT_TRANS = 308,
    ROLLBACK_TRANS = 309,
    CASE = 310,
    WHEN = 311,
    THEN = 312,
    ELSE = 313,
    END = 314,
    OVER_TOK = 315,
    PARTITION = 316,
    ROWS = 317,
    RANGE = 318,
    UNBOUNDED = 319,
    PRECEDING = 320,
    CURRENT = 321,
    ROW = 322,
    FOLLOWING = 323,
    DUMMYEXPR = 324,
    JOIN = 325,
    NATURAL = 326,
    LEFT = 327,
    RIGHT = 328,
    OUTER = 329,
    INNER = 330,
    CROSS = 331,
    ON = 332,
    USING = 333,
    FULL = 334,
    TYPE = 335,
    TRANSACTION = 336,
    WITH = 337,
    XOR = 338
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 28 "sql_parser.y" /* yacc.c:1909  */

    /* 
     * Declare some C structure those will be used as data type
     * for various tokens used in grammar rules.
     */
     Node *node;
     List *list;
     char *stringVal;
     int intVal;
     double floatVal;

#line 150 "sql_parser.tab.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SQL_PARSER_TAB_H_INCLUDED  */

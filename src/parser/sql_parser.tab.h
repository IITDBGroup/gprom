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
    ONLY = 275,
    UPDATED = 276,
    SHOW = 277,
    INTERMEDIATE = 278,
    FROM = 279,
    AS = 280,
    WHERE = 281,
    DISTINCT = 282,
    STARALL = 283,
    AND = 284,
    OR = 285,
    LIKE = 286,
    NOT = 287,
    IN = 288,
    ISNULL = 289,
    BETWEEN = 290,
    EXCEPT = 291,
    EXISTS = 292,
    AMMSC = 293,
    NULLVAL = 294,
    ALL = 295,
    ANY = 296,
    IS = 297,
    SOME = 298,
    UNION = 299,
    INTERSECT = 300,
    MINUS = 301,
    INTO = 302,
    VALUES = 303,
    HAVING = 304,
    GROUP = 305,
    ORDER = 306,
    BY = 307,
    LIMIT = 308,
    SET = 309,
    INT = 310,
    BEGIN_TRANS = 311,
    COMMIT_TRANS = 312,
    ROLLBACK_TRANS = 313,
    CASE = 314,
    WHEN = 315,
    THEN = 316,
    ELSE = 317,
    END = 318,
    OVER_TOK = 319,
    PARTITION = 320,
    ROWS = 321,
    RANGE = 322,
    UNBOUNDED = 323,
    PRECEDING = 324,
    CURRENT = 325,
    ROW = 326,
    FOLLOWING = 327,
    DUMMYEXPR = 328,
    JOIN = 329,
    NATURAL = 330,
    LEFT = 331,
    RIGHT = 332,
    OUTER = 333,
    INNER = 334,
    CROSS = 335,
    ON = 336,
    USING = 337,
    FULL = 338,
    TYPE = 339,
    TRANSACTION = 340,
    WITH = 341,
    XOR = 342
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

#line 154 "sql_parser.tab.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SQL_PARSER_TAB_H_INCLUDED  */

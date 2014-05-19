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
    USE = 279,
    FROM = 280,
    AS = 281,
    WHERE = 282,
    DISTINCT = 283,
    STARALL = 284,
    AND = 285,
    OR = 286,
    LIKE = 287,
    NOT = 288,
    IN = 289,
    ISNULL = 290,
    BETWEEN = 291,
    EXCEPT = 292,
    EXISTS = 293,
    AMMSC = 294,
    NULLVAL = 295,
    ALL = 296,
    ANY = 297,
    IS = 298,
    SOME = 299,
    UNION = 300,
    INTERSECT = 301,
    MINUS = 302,
    INTO = 303,
    VALUES = 304,
    HAVING = 305,
    GROUP = 306,
    ORDER = 307,
    BY = 308,
    LIMIT = 309,
    SET = 310,
    INT = 311,
    BEGIN_TRANS = 312,
    COMMIT_TRANS = 313,
    ROLLBACK_TRANS = 314,
    CASE = 315,
    WHEN = 316,
    THEN = 317,
    ELSE = 318,
    END = 319,
    OVER_TOK = 320,
    PARTITION = 321,
    ROWS = 322,
    RANGE = 323,
    UNBOUNDED = 324,
    PRECEDING = 325,
    CURRENT = 326,
    ROW = 327,
    FOLLOWING = 328,
    NULLS = 329,
    FIRST = 330,
    LAST = 331,
    ASC = 332,
    DESC = 333,
    DUMMYEXPR = 334,
    JOIN = 335,
    NATURAL = 336,
    LEFT = 337,
    RIGHT = 338,
    OUTER = 339,
    INNER = 340,
    CROSS = 341,
    ON = 342,
    USING = 343,
    FULL = 344,
    TYPE = 345,
    TRANSACTION = 346,
    WITH = 347,
    XOR = 348
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

#line 160 "sql_parser.tab.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SQL_PARSER_TAB_H_INCLUDED  */

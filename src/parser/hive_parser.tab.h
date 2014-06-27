/* A Bison parser, made by GNU Bison 3.0.2.  */

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

#ifndef YY_HIVE_HIVE_PARSER_TAB_H_INCLUDED
# define YY_HIVE_HIVE_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int hivedebug;
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
    TUPLE = 280,
    VERSIONS = 281,
    FROM = 282,
    AS = 283,
    WHERE = 284,
    DISTINCT = 285,
    STARALL = 286,
    AND = 287,
    OR = 288,
    LIKE = 289,
    NOT = 290,
    IN = 291,
    ISNULL = 292,
    BETWEEN = 293,
    EXCEPT = 294,
    EXISTS = 295,
    AMMSC = 296,
    NULLVAL = 297,
    ALL = 298,
    ANY = 299,
    IS = 300,
    SOME = 301,
    UNION = 302,
    INTERSECT = 303,
    MINUS = 304,
    INTO = 305,
    VALUES = 306,
    HAVING = 307,
    GROUP = 308,
    ORDER = 309,
    BY = 310,
    LIMIT = 311,
    SET = 312,
    INT = 313,
    BEGIN_TRANS = 314,
    COMMIT_TRANS = 315,
    ROLLBACK_TRANS = 316,
    CASE = 317,
    WHEN = 318,
    THEN = 319,
    ELSE = 320,
    END = 321,
    OVER_TOK = 322,
    PARTITION = 323,
    ROWS = 324,
    RANGE = 325,
    UNBOUNDED = 326,
    PRECEDING = 327,
    CURRENT = 328,
    ROW = 329,
    FOLLOWING = 330,
    NULLS = 331,
    FIRST = 332,
    LAST = 333,
    ASC = 334,
    DESC = 335,
    DUMMYEXPR = 336,
    JOIN = 337,
    NATURAL = 338,
    LEFT = 339,
    RIGHT = 340,
    OUTER = 341,
    INNER = 342,
    CROSS = 343,
    ON = 344,
    USING = 345,
    FULL = 346,
    TYPE = 347,
    TRANSACTION = 348,
    WITH = 349,
    XOR = 350
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 24 "hive_parser.y" /* yacc.c:1909  */

    /* 
     * Declare some C structure those will be used as data type
     * for various tokens used in grammar rules.
     */
     Node *node;
     List *list;
     char *stringVal;
     int intVal;
     double floatVal;

#line 162 "hive_parser.tab.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE hivelval;

int hiveparse (void);

#endif /* !YY_HIVE_HIVE_PARSER_TAB_H_INCLUDED  */

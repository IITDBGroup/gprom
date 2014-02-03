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
    FROM = 274,
    AS = 275,
    WHERE = 276,
    DISTINCT = 277,
    STARALL = 278,
    AND = 279,
    OR = 280,
    LIKE = 281,
    NOT = 282,
    IN = 283,
    ISNULL = 284,
    BETWEEN = 285,
    EXCEPT = 286,
    EXISTS = 287,
    AMMSC = 288,
    NULLVAL = 289,
    ALL = 290,
    ANY = 291,
    IS = 292,
    SOME = 293,
    UNION = 294,
    INTERSECT = 295,
    MINUS = 296,
    INTO = 297,
    VALUES = 298,
    HAVING = 299,
    GROUP = 300,
    ORDER = 301,
    BY = 302,
    LIMIT = 303,
    SET = 304,
    INT = 305,
    BEGIN_TRANS = 306,
    COMMIT_TRANS = 307,
    ROLLBACK_TRANS = 308,
    DUMMYEXPR = 309,
    JOIN = 310,
    NATURAL = 311,
    LEFT = 312,
    RIGHT = 313,
    OUTER = 314,
    INNER = 315,
    CROSS = 316,
    ON = 317,
    USING = 318,
    FULL = 319,
    TYPE = 320,
    TRANSACTION = 321,
    WITH = 322,
    XOR = 323
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

#line 135 "sql_parser.tab.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SQL_PARSER_TAB_H_INCLUDED  */

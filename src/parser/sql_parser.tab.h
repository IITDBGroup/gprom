/* A Bison parser, made by GNU Bison 3.0.  */

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
    comparisonOps = 262,
    SELECT = 263,
    INSERT = 264,
    UPDATE = 265,
    DELETE = 266,
    PROVENANCE = 267,
    OF = 268,
    BASERELATION = 269,
    FROM = 270,
    AS = 271,
    WHERE = 272,
    DISTINCT = 273,
    STARALL = 274,
    AND = 275,
    OR = 276,
    LIKE = 277,
    NOT = 278,
    IN = 279,
    ISNULL = 280,
    BETWEEN = 281,
    EXCEPT = 282,
    EXISTS = 283,
    AMMSC = 284,
    NULLVAL = 285,
    ALL = 286,
    ANY = 287,
    IS = 288,
    SOME = 289,
    UNION = 290,
    INTERSECT = 291,
    MINUS = 292,
    INTO = 293,
    VALUES = 294,
    HAVING = 295,
    GROUP = 296,
    ORDER = 297,
    BY = 298,
    LIMIT = 299,
    SET = 300,
    INT = 301,
    BEGIN_TRANS = 302,
    COMMIT_TRANS = 303,
    ROLLBACK_TRANS = 304,
    DUMMYEXPR = 305,
    JOIN = 306,
    NATURAL = 307,
    LEFT = 308,
    RIGHT = 309,
    OUTER = 310,
    INNER = 311,
    CROSS = 312,
    ON = 313,
    USING = 314,
    FULL = 315,
    XOR = 316
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

#line 128 "sql_parser.tab.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SQL_PARSER_TAB_H_INCLUDED  */

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
    comparisonOps = 262,
    SELECT = 263,
    INSERT = 264,
    UPDATE = 265,
    DELETE = 266,
    PROVENANCE = 267,
    OF = 268,
    BASERELATION = 269,
    SCN = 270,
    TIMESTAMP = 271,
    FROM = 272,
    AS = 273,
    WHERE = 274,
    DISTINCT = 275,
    STARALL = 276,
    AND = 277,
    OR = 278,
    LIKE = 279,
    NOT = 280,
    IN = 281,
    ISNULL = 282,
    BETWEEN = 283,
    EXCEPT = 284,
    EXISTS = 285,
    AMMSC = 286,
    NULLVAL = 287,
    ALL = 288,
    ANY = 289,
    IS = 290,
    SOME = 291,
    UNION = 292,
    INTERSECT = 293,
    MINUS = 294,
    INTO = 295,
    VALUES = 296,
    HAVING = 297,
    GROUP = 298,
    ORDER = 299,
    BY = 300,
    LIMIT = 301,
    SET = 302,
    INT = 303,
    BEGIN_TRANS = 304,
    COMMIT_TRANS = 305,
    ROLLBACK_TRANS = 306,
    DUMMYEXPR = 307,
    JOIN = 308,
    NATURAL = 309,
    LEFT = 310,
    RIGHT = 311,
    OUTER = 312,
    INNER = 313,
    CROSS = 314,
    ON = 315,
    USING = 316,
    FULL = 317,
    XOR = 318
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

#line 130 "sql_parser.tab.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SQL_PARSER_TAB_H_INCLUDED  */

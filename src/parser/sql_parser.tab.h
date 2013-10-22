/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
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


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
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
     FROM = 269,
     AS = 270,
     WHERE = 271,
     DISTINCT = 272,
     STARALL = 273,
     AND = 274,
     OR = 275,
     LIKE = 276,
     NOT = 277,
     IN = 278,
     ISNULL = 279,
     BETWEEN = 280,
     EXCEPT = 281,
     EXISTS = 282,
     AMMSC = 283,
     NULLVAL = 284,
     ALL = 285,
     ANY = 286,
     BY = 287,
     IS = 288,
     SOME = 289,
     UNION = 290,
     INTERSECT = 291,
     MINUS = 292,
     INTO = 293,
     VALUES = 294,
     DUMMYEXPR = 295,
     JOIN = 296,
     NATURAL = 297,
     LEFT = 298,
     RIGHT = 299,
     OUTER = 300,
     INNER = 301,
     CROSS = 302,
     ON = 303,
     USING = 304,
     FULL = 305,
     XOR = 306
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 2068 of yacc.c  */
#line 24 "sql_parser.y"

    /* 
     * Declare some C structure those will be used as data type
     * for various tokens used in grammar rules.
     */
     Node *node;
     List *list;
     char *stringVal;
     int intVal;
     double floatVal;



/* Line 2068 of yacc.c  */
#line 115 "sql_parser.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;



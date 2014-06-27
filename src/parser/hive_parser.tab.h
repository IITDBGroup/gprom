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
    StringLiteral = 258,
    Identifier = 259,
    kwUser = 260,
    kwRole = 261,
    kwInner = 262,
    KW_TRUE = 263,
    KW_FALSE = 264,
    KW_ALL = 265,
    KW_AND = 266,
    KW_OR = 267,
    KW_NOT = 268,
    KW_LIKE = 269,
    KW_IF = 270,
    KW_EXISTS = 271,
    KW_ASC = 272,
    KW_DESC = 273,
    KW_ORDER = 274,
    KW_GROUP = 275,
    KW_BY = 276,
    KW_HAVING = 277,
    KW_WHERE = 278,
    KW_FROM = 279,
    KW_AS = 280,
    KW_SELECT = 281,
    KW_DISTINCT = 282,
    KW_INSERT = 283,
    KW_OVERWRITE = 284,
    KW_OUTER = 285,
    KW_UNIQUEJOIN = 286,
    KW_PRESERVE = 287,
    KW_JOIN = 288,
    KW_LEFT = 289,
    KW_RIGHT = 290,
    KW_FULL = 291,
    KW_ON = 292,
    KW_PARTITION = 293,
    KW_PARTITIONS = 294,
    KW_TABLE = 295,
    KW_TABLES = 296,
    KW_INDEX = 297,
    KW_INDEXES = 298,
    KW_REBUILD = 299,
    KW_FUNCTIONS = 300,
    KW_SHOW = 301,
    KW_MSCK = 302,
    KW_REPAIR = 303,
    KW_DIRECTORY = 304,
    KW_LOCAL = 305,
    KW_TRANSFORM = 306,
    KW_USING = 307,
    KW_CLUSTER = 308,
    KW_DISTRIBUTE = 309,
    KW_SORT = 310,
    KW_UNION = 311,
    KW_LOAD = 312,
    KW_EXPORT = 313,
    KW_IMPORT = 314,
    KW_DATA = 315,
    KW_INPATH = 316,
    KW_IS = 317,
    KW_NULL = 318,
    KW_CREATE = 319,
    KW_EXTERNAL = 320,
    KW_ALTER = 321,
    KW_CHANGE = 322,
    KW_COLUMN = 323,
    KW_FIRST = 324,
    KW_AFTER = 325,
    KW_DESCRIBE = 326,
    KW_DROP = 327,
    KW_RENAME = 328,
    KW_TO = 329,
    KW_COMMENT = 330,
    KW_BOOLEAN = 331,
    KW_TINYINT = 332,
    KW_SMALLINT = 333,
    KW_INT = 334,
    KW_BIGINT = 335,
    KW_FLOAT = 336,
    KW_DOUBLE = 337,
    KW_DATE = 338,
    KW_DATETIME = 339,
    KW_TIMESTAMP = 340,
    KW_STRING = 341,
    KW_ARRAY = 342,
    KW_STRUCT = 343,
    KW_MAP = 344,
    KW_UNIONTYPE = 345,
    KW_REDUCE = 346,
    KW_PARTITIONED = 347,
    KW_CLUSTERED = 348,
    KW_SORTED = 349,
    KW_INTO = 350,
    KW_BUCKETS = 351,
    KW_ROW = 352,
    KW_FORMAT = 353,
    KW_DELIMITED = 354,
    KW_FIELDS = 355,
    KW_TERMINATED = 356,
    KW_ESCAPED = 357,
    KW_COLLECTION = 358,
    KW_ITEMS = 359,
    KW_KEYS = 360,
    KW_KEY_TYPE = 361,
    KW_LINES = 362,
    KW_STORED = 363,
    KW_FILEFORMAT = 364,
    KW_SEQUENCEFILE = 365,
    KW_TEXTFILE = 366,
    KW_RCFILE = 367,
    KW_INPUTFORMAT = 368,
    KW_OUTPUTFORMAT = 369,
    KW_INPUTDRIVER = 370,
    KW_OUTPUTDRIVER = 371,
    KW_OFFLINE = 372,
    KW_ENABLE = 373,
    KW_DISABLE = 374,
    KW_READONLY = 375,
    KW_NO_DROP = 376,
    KW_LOCATION = 377,
    KW_TABLESAMPLE = 378,
    KW_BUCKET = 379,
    KW_OUT = 380,
    KW_OF = 381,
    KW_PERCENT = 382,
    KW_CAST = 383,
    KW_ADD = 384,
    KW_REPLACE = 385,
    KW_COLUMNS = 386,
    KW_RLIKE = 387,
    KW_REGEXP = 388,
    KW_TEMPORARY = 389,
    KW_FUNCTION = 390,
    KW_EXPLAIN = 391,
    KW_EXTENDED = 392,
    KW_FORMATTED = 393,
    KW_SERDE = 394,
    KW_WITH = 395,
    KW_DEFERRED = 396,
    KW_SERDEPROPERTIES = 397,
    KW_DBPROPERTIES = 398,
    KW_LIMIT = 399,
    KW_SET = 400,
    KW_TBLPROPERTIES = 401,
    KW_IDXPROPERTIES = 402,
    KW_VALUE_TYPE = 403,
    KW_ELEM_TYPE = 404,
    KW_CASE = 405,
    KW_WHEN = 406,
    KW_THEN = 407,
    KW_ELSE = 408,
    KW_END = 409,
    KW_MAPJOIN = 410,
    KW_STREAMTABLE = 411,
    KW_HOLD_DDLTIME = 412,
    KW_CLUSTERSTATUS = 413,
    KW_UTC = 414,
    KW_UTCTIMESTAMP = 415,
    KW_LONG = 416,
    KW_DELETE = 417,
    KW_PLUS = 418,
    KW_MINUS = 419,
    KW_FETCH = 420,
    KW_INTERSECT = 421,
    KW_VIEW = 422,
    KW_IN = 423,
    KW_DATABASE = 424,
    KW_DATABASES = 425,
    KW_MATERIALIZED = 426,
    KW_SCHEMA = 427,
    KW_SCHEMAS = 428,
    KW_GRANT = 429,
    KW_REVOKE = 430,
    KW_SSL = 431,
    KW_UNDO = 432,
    KW_LOCK = 433,
    KW_LOCKS = 434,
    KW_UNLOCK = 435,
    KW_SHARED = 436,
    KW_EXCLUSIVE = 437,
    KW_PROCEDURE = 438,
    KW_UNSIGNED = 439,
    KW_WHILE = 440,
    KW_READ = 441,
    KW_READS = 442,
    KW_PURGE = 443,
    KW_RANGE = 444,
    KW_ANALYZE = 445,
    KW_BEFORE = 446,
    KW_BETWEEN = 447,
    KW_BOTH = 448,
    KW_BINARY = 449,
    KW_CROSS = 450,
    KW_CONTINUE = 451,
    KW_CURSOR = 452,
    KW_TRIGGER = 453,
    KW_RECORDREADER = 454,
    KW_RECORDWRITER = 455,
    KW_SEMI = 456,
    KW_LATERAL = 457,
    KW_TOUCH = 458,
    KW_ARCHIVE = 459,
    KW_UNARCHIVE = 460,
    KW_COMPUTE = 461,
    KW_STATISTICS = 462,
    KW_USE = 463,
    KW_OPTION = 464,
    KW_CONCATENATE = 465,
    KW_SHOW_DATABASE = 466,
    KW_UPDATE = 467,
    KW_RESTRICT = 468,
    KW_CASCADE = 469,
    COMMA = 470,
    LPAREN = 471,
    RPAREN = 472,
    EQUAL = 473
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

#line 285 "hive_parser.tab.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE hivelval;

int hiveparse (void);

#endif /* !YY_HIVE_HIVE_PARSER_TAB_H_INCLUDED  */

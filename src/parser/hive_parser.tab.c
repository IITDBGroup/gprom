/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         hiveparse
#define yylex           hivelex
#define yyerror         hiveerror
#define yydebug         hivedebug
#define yynerrs         hivenerrs

#define yylval          hivelval
#define yychar          hivechar

/* Copy the first part of user declarations.  */
#line 1 "hive_parser.y" /* yacc.c:339  */

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "parser/parse_internal_hive.h"
#include "log/logger.h"
#include "model/query_operator/operator_property.h"

#define RULELOG(grule) \
    { \
        TRACE_LOG("Parsing grammer rule <%s>", #grule); \
    }
    
#undef free

Node *hiveParseResult = NULL;

#line 95 "hive_parser.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "hive_parser.tab.h".  */
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
#line 24 "hive_parser.y" /* yacc.c:355  */

    /* 
     * Declare some C structure those will be used as data type
     * for various tokens used in grammar rules.
     */
     Node *node;
     List *list;
     char *stringVal;
     int intVal;
     double floatVal;

#line 366 "hive_parser.tab.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE hivelval;

int hiveparse (void);

#endif /* !YY_HIVE_HIVE_PARSER_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 381 "hive_parser.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  32
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   208

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  220
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  30
/* YYNRULES -- Number of rules.  */
#define YYNRULES  53
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  88

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   473

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   219,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   149,   149,   155,   164,   165,   170,   177,   178,   179,
     184,   185,   186,   187,   188,   193,   200,   201,   205,   206,
     211,   219,   226,   227,   231,   232,   237,   243,   244,   245,
     273,   274,   278,   279,   280,   284,   285,   295,   302,   303,
     307,   308,   312,   313,   317,   321,   322,   327,   332,   336,
     337,   343,   347,   348
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "StringLiteral", "Identifier", "kwUser",
  "kwRole", "kwInner", "KW_TRUE", "KW_FALSE", "KW_ALL", "KW_AND", "KW_OR",
  "KW_NOT", "KW_LIKE", "KW_IF", "KW_EXISTS", "KW_ASC", "KW_DESC",
  "KW_ORDER", "KW_GROUP", "KW_BY", "KW_HAVING", "KW_WHERE", "KW_FROM",
  "KW_AS", "KW_SELECT", "KW_DISTINCT", "KW_INSERT", "KW_OVERWRITE",
  "KW_OUTER", "KW_UNIQUEJOIN", "KW_PRESERVE", "KW_JOIN", "KW_LEFT",
  "KW_RIGHT", "KW_FULL", "KW_ON", "KW_PARTITION", "KW_PARTITIONS",
  "KW_TABLE", "KW_TABLES", "KW_INDEX", "KW_INDEXES", "KW_REBUILD",
  "KW_FUNCTIONS", "KW_SHOW", "KW_MSCK", "KW_REPAIR", "KW_DIRECTORY",
  "KW_LOCAL", "KW_TRANSFORM", "KW_USING", "KW_CLUSTER", "KW_DISTRIBUTE",
  "KW_SORT", "KW_UNION", "KW_LOAD", "KW_EXPORT", "KW_IMPORT", "KW_DATA",
  "KW_INPATH", "KW_IS", "KW_NULL", "KW_CREATE", "KW_EXTERNAL", "KW_ALTER",
  "KW_CHANGE", "KW_COLUMN", "KW_FIRST", "KW_AFTER", "KW_DESCRIBE",
  "KW_DROP", "KW_RENAME", "KW_TO", "KW_COMMENT", "KW_BOOLEAN",
  "KW_TINYINT", "KW_SMALLINT", "KW_INT", "KW_BIGINT", "KW_FLOAT",
  "KW_DOUBLE", "KW_DATE", "KW_DATETIME", "KW_TIMESTAMP", "KW_STRING",
  "KW_ARRAY", "KW_STRUCT", "KW_MAP", "KW_UNIONTYPE", "KW_REDUCE",
  "KW_PARTITIONED", "KW_CLUSTERED", "KW_SORTED", "KW_INTO", "KW_BUCKETS",
  "KW_ROW", "KW_FORMAT", "KW_DELIMITED", "KW_FIELDS", "KW_TERMINATED",
  "KW_ESCAPED", "KW_COLLECTION", "KW_ITEMS", "KW_KEYS", "KW_KEY_TYPE",
  "KW_LINES", "KW_STORED", "KW_FILEFORMAT", "KW_SEQUENCEFILE",
  "KW_TEXTFILE", "KW_RCFILE", "KW_INPUTFORMAT", "KW_OUTPUTFORMAT",
  "KW_INPUTDRIVER", "KW_OUTPUTDRIVER", "KW_OFFLINE", "KW_ENABLE",
  "KW_DISABLE", "KW_READONLY", "KW_NO_DROP", "KW_LOCATION",
  "KW_TABLESAMPLE", "KW_BUCKET", "KW_OUT", "KW_OF", "KW_PERCENT",
  "KW_CAST", "KW_ADD", "KW_REPLACE", "KW_COLUMNS", "KW_RLIKE", "KW_REGEXP",
  "KW_TEMPORARY", "KW_FUNCTION", "KW_EXPLAIN", "KW_EXTENDED",
  "KW_FORMATTED", "KW_SERDE", "KW_WITH", "KW_DEFERRED",
  "KW_SERDEPROPERTIES", "KW_DBPROPERTIES", "KW_LIMIT", "KW_SET",
  "KW_TBLPROPERTIES", "KW_IDXPROPERTIES", "KW_VALUE_TYPE", "KW_ELEM_TYPE",
  "KW_CASE", "KW_WHEN", "KW_THEN", "KW_ELSE", "KW_END", "KW_MAPJOIN",
  "KW_STREAMTABLE", "KW_HOLD_DDLTIME", "KW_CLUSTERSTATUS", "KW_UTC",
  "KW_UTCTIMESTAMP", "KW_LONG", "KW_DELETE", "KW_PLUS", "KW_MINUS",
  "KW_FETCH", "KW_INTERSECT", "KW_VIEW", "KW_IN", "KW_DATABASE",
  "KW_DATABASES", "KW_MATERIALIZED", "KW_SCHEMA", "KW_SCHEMAS", "KW_GRANT",
  "KW_REVOKE", "KW_SSL", "KW_UNDO", "KW_LOCK", "KW_LOCKS", "KW_UNLOCK",
  "KW_SHARED", "KW_EXCLUSIVE", "KW_PROCEDURE", "KW_UNSIGNED", "KW_WHILE",
  "KW_READ", "KW_READS", "KW_PURGE", "KW_RANGE", "KW_ANALYZE", "KW_BEFORE",
  "KW_BETWEEN", "KW_BOTH", "KW_BINARY", "KW_CROSS", "KW_CONTINUE",
  "KW_CURSOR", "KW_TRIGGER", "KW_RECORDREADER", "KW_RECORDWRITER",
  "KW_SEMI", "KW_LATERAL", "KW_TOUCH", "KW_ARCHIVE", "KW_UNARCHIVE",
  "KW_COMPUTE", "KW_STATISTICS", "KW_USE", "KW_OPTION", "KW_CONCATENATE",
  "KW_SHOW_DATABASE", "KW_UPDATE", "KW_RESTRICT", "KW_CASCADE", "COMMA",
  "LPAREN", "RPAREN", "EQUAL", "';'", "$accept", "stmtList", "statement",
  "explainStatement", "explainOptions", "execStatement", "loadStatement",
  "loadIsLocal", "loadIsOverwrite", "exportStatement", "importStatement",
  "importIsExternal", "importTableOrPartition", "importTableLocation",
  "ddlStatement", "ifExists", "restrictOrCascade", "ifNotExists",
  "createDatabaseStatement", "createDatabaseOrSchema", "dbLocation",
  "optDbProperties", "dbProperties", "dbPropertiesList",
  "switchDatabaseStatement", "dropDatabaseStatement", "databaseComment",
  "keyValueProperty", "queryStatementExpression", "tableOrPartition", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,    59
};
# endif

#define YYPACT_NINF -203

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-203)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -56,   -50,   -26,   -43,  -160,  -160,  -120,    19,     0,  -195,
    -194,  -192,  -203,  -203,  -203,  -203,  -203,  -203,  -203,  -203,
     -24,  -203,  -203,   -12,  -203,  -203,    14,    15,  -203,  -203,
     -53,  -203,  -203,  -188,  -203,  -203,  -203,  -203,   -29,   -41,
    -203,    21,    31,    20,    33,  -203,  -203,    35,    36,    16,
    -203,    25,   -32,  -203,  -193,    17,  -203,    41,  -203,    42,
     -75,  -203,  -203,  -203,  -203,   -47,  -203,  -203,    46,   -90,
      11,  -203,  -203,   -91,  -203,  -203,  -163,  -203,    51,  -203,
    -162,  -202,  -203,    52,    51,  -203,  -203,  -203
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      52,     0,     0,    22,     0,     0,     7,     0,    52,     0,
       0,     0,    11,    12,    13,    14,    27,    28,    29,    10,
      16,    53,    23,     0,    38,    39,    35,    30,     8,     9,
      52,    47,     1,     0,     2,     4,     5,    17,     0,     0,
      24,     0,     0,     0,     0,     6,     3,     0,     0,     0,
      25,     0,    49,    31,    32,    18,    20,     0,    36,     0,
      40,    33,    34,    48,    19,     0,    26,    50,     0,    42,
       0,    21,    41,     0,    37,    53,     0,    15,     0,    43,
       0,     0,    45,     0,     0,    44,    51,    46
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -203,  -203,    53,  -203,  -203,    30,  -203,  -203,  -203,  -203,
    -203,  -203,  -203,  -203,  -203,  -203,  -203,  -203,  -203,    57,
    -203,  -203,  -203,  -203,  -203,  -203,  -203,   -21,  -203,   -33
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     8,     9,    10,    30,    11,    12,    38,    65,    13,
      14,    23,    49,    71,    15,    44,    63,    42,    16,    26,
      69,    74,    79,    81,    17,    18,    60,    82,    19,    39
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      32,     1,     2,     3,     1,     2,     3,    50,     4,    24,
      20,     4,    25,    84,    21,    85,     5,    28,    29,     5,
      61,    62,    22,    31,    34,    35,    37,    36,    40,    41,
      43,    46,    47,    48,    51,    52,    53,    54,    55,    56,
      57,    58,    77,    59,    66,    67,    64,    68,    70,    72,
      73,    75,    76,    78,    80,    86,    83,     1,     2,     3,
      45,    33,    27,    87,     4,     0,     0,     0,     0,     0,
       0,     0,     5,     0,     0,     0,     0,     0,     0,     0,
       6,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     6,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     7,     0,     0,     7,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     7
};

static const yytype_int16 yycheck[] =
{
       0,    57,    58,    59,    57,    58,    59,    40,    64,   169,
      60,    64,   172,   215,    40,   217,    72,   137,   138,    72,
     213,   214,    65,     4,   219,   219,    50,   219,    40,    15,
      15,   219,    61,    74,    13,     4,    16,     4,     3,     3,
      24,    16,    75,    75,     3,     3,    29,   122,    95,     3,
     140,    40,   143,   216,     3,     3,   218,    57,    58,    59,
      30,     8,     5,    84,    64,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    72,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   208,    -1,    -1,   208,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   208
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    57,    58,    59,    64,    72,   136,   208,   221,   222,
     223,   225,   226,   229,   230,   234,   238,   244,   245,   248,
      60,    40,    65,   231,   169,   172,   239,   239,   137,   138,
     224,     4,     0,   222,   219,   219,   219,    50,   227,   249,
      40,    15,   237,    15,   235,   225,   219,    61,    74,   232,
     249,    13,     4,    16,     4,     3,     3,    24,    16,    75,
     246,   213,   214,   236,    29,   228,     3,     3,   122,   240,
      95,   233,     3,   140,   241,    40,   143,   249,   216,   242,
       3,   243,   247,   218,   215,   217,     3,   247
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   220,   221,   221,   222,   222,   223,   224,   224,   224,
     225,   225,   225,   225,   225,   226,   227,   227,   228,   228,
     229,   230,   231,   231,   232,   232,   233,   234,   234,   234,
     235,   235,   236,   236,   236,   237,   237,   238,   239,   239,
     240,   240,   241,   241,   242,   243,   243,   244,   245,   246,
     246,   247,   248,   249
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     3,     2,     2,     3,     0,     1,     1,
       1,     1,     1,     1,     1,     9,     0,     1,     0,     1,
       5,     7,     0,     1,     0,     1,     0,     1,     1,     1,
       0,     2,     0,     1,     1,     0,     3,     7,     1,     1,
       0,     2,     0,     3,     3,     1,     3,     2,     5,     0,
       2,     3,     0,     0
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 150 "hive_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("stmtList::statement"); 
				(yyval.list) = singleton((yyvsp[-1].node));
				hiveParseResult = (Node *) (yyval.list);	 
			}
#line 1626 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 156 "hive_parser.y" /* yacc.c:1646  */
    {
				RULELOG("stmtlist::stmtList::statement");
				(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[-1].node));	
				hiveParseResult = (Node *) (yyval.list); 
			}
#line 1636 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 171 "hive_parser.y" /* yacc.c:1646  */
    {
			(yyval.node) = NULL;
		}
#line 1644 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 177 "hive_parser.y" /* yacc.c:1646  */
    { (yyval.stringVal) = NULL; }
#line 1650 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 194 "hive_parser.y" /* yacc.c:1646  */
    {
			(yyval.node) = NULL;
		}
#line 1658 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 200 "hive_parser.y" /* yacc.c:1646  */
    { (yyval.stringVal) = NULL; }
#line 1664 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 205 "hive_parser.y" /* yacc.c:1646  */
    { (yyval.stringVal) = NULL; }
#line 1670 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 212 "hive_parser.y" /* yacc.c:1646  */
    {
			(yyval.node) = NULL;
		}
#line 1678 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 220 "hive_parser.y" /* yacc.c:1646  */
    {
			(yyval.node) = NULL;
		}
#line 1686 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 226 "hive_parser.y" /* yacc.c:1646  */
    { (yyval.stringVal) = NULL; }
#line 1692 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 231 "hive_parser.y" /* yacc.c:1646  */
    { (yyval.node) = NULL; }
#line 1698 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 237 "hive_parser.y" /* yacc.c:1646  */
    { (yyval.stringVal) = NULL; }
#line 1704 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 273 "hive_parser.y" /* yacc.c:1646  */
    { (yyval.stringVal) = NULL; }
#line 1710 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 278 "hive_parser.y" /* yacc.c:1646  */
    { (yyval.stringVal) = NULL; }
#line 1716 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 284 "hive_parser.y" /* yacc.c:1646  */
    { (yyval.stringVal) = NULL; }
#line 1722 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 296 "hive_parser.y" /* yacc.c:1646  */
    { 
			(yyval.node) = NULL; 
		}
#line 1730 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 307 "hive_parser.y" /* yacc.c:1646  */
    { (yyval.stringVal)=NULL; }
#line 1736 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 312 "hive_parser.y" /* yacc.c:1646  */
    { (yyval.node) = NULL; }
#line 1742 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 313 "hive_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 1748 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 317 "hive_parser.y" /* yacc.c:1646  */
    { (yyval.node) = NIL; }
#line 1754 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 321 "hive_parser.y" /* yacc.c:1646  */
    { (yyval.node) = singleton((yyvsp[0].node)); }
#line 1760 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 322 "hive_parser.y" /* yacc.c:1646  */
    { (yyval.node) = CONCAT_LISTS((yyvsp[-2].node),(yyvsp[0].node)); }
#line 1766 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 327 "hive_parser.y" /* yacc.c:1646  */
    { (yyval.node) = NULL; }
#line 1772 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 332 "hive_parser.y" /* yacc.c:1646  */
    { (yyval.node) = NULL; }
#line 1778 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 336 "hive_parser.y" /* yacc.c:1646  */
    { (yyval.node) = NULL; }
#line 1784 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 337 "hive_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].stringVal); }
#line 1790 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 343 "hive_parser.y" /* yacc.c:1646  */
    { (yyval.node) = createStringKeyValue((yyvsp[-2].stringVal),(yyvsp[0].stringVal)); }
#line 1796 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 347 "hive_parser.y" /* yacc.c:1646  */
    { (yyval.node) = NULL; }
#line 1802 "hive_parser.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 348 "hive_parser.y" /* yacc.c:1646  */
    { (yyval.node) = NULL; }
#line 1808 "hive_parser.tab.c" /* yacc.c:1646  */
    break;


#line 1812 "hive_parser.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 2179 "hive_parser.y" /* yacc.c:1906  */




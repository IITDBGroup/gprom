/* A Bison parser, made by GNU Bison 3.0.  */

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
#define YYBISON_VERSION "3.0"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 8 "sql_parser.y" /* yacc.c:339  */

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "parser/parse_internal.h"
#include "log/logger.h"

#define RULELOG(grule) \
    { \
        TRACE_LOG("Parsing grammer rule <%s>", #grule); \
    }
    
#undef free

Node *bisonParseResult = NULL;

#line 86 "sql_parser.tab.c" /* yacc.c:339  */

# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
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
   by #include "sql_parser.tab.h".  */
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
    IS = 287,
    SOME = 288,
    UNION = 289,
    INTERSECT = 290,
    MINUS = 291,
    INTO = 292,
    VALUES = 293,
    HAVING = 294,
    GROUP = 295,
    ORDER = 296,
    BY = 297,
    LIMIT = 298,
    SET = 299,
    INT = 300,
    BEGIN_TRANS = 301,
    COMMIT_TRANS = 302,
    ROLLBACK_TRANS = 303,
    DUMMYEXPR = 304,
    JOIN = 305,
    NATURAL = 306,
    LEFT = 307,
    RIGHT = 308,
    OUTER = 309,
    INNER = 310,
    CROSS = 311,
    ON = 312,
    USING = 313,
    FULL = 314,
    XOR = 315
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 28 "sql_parser.y" /* yacc.c:355  */

    /* 
     * Declare some C structure those will be used as data type
     * for various tokens used in grammar rules.
     */
     Node *node;
     List *list;
     char *stringVal;
     int intVal;
     double floatVal;

#line 199 "sql_parser.tab.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SQL_PARSER_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 214 "sql_parser.tab.c" /* yacc.c:358  */

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

#ifndef __attribute__
/* This feature is available in gcc versions 2.5 and later.  */
# if (! defined __GNUC__ || __GNUC__ < 2 \
      || (__GNUC__ == 2 && __GNUC_MINOR__ < 5))
#  define __attribute__(Spec) /* empty */
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
#define YYFINAL  29
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   398

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  76
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  46
/* YYNRULES -- Number of rules.  */
#define YYNRULES  131
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  250

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   315

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    15,     2,     2,     2,    11,    13,     2,
      18,    17,     9,     7,    74,     8,    75,    10,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    73,
       2,    19,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    12,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    14,     2,     2,     2,     2,     2,
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
       5,     6,    16,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   123,   123,   129,   138,   143,   148,   159,   160,   161,
     168,   169,   170,   171,   175,   176,   177,   184,   195,   204,
     205,   219,   227,   232,   240,   249,   264,   269,   277,   282,
     287,   292,   306,   311,   316,   324,   325,   334,   357,   358,
     363,   375,   379,   387,   392,   397,   402,   413,   414,   426,
     427,   428,   429,   430,   431,   440,   441,   442,   449,   468,
     475,   482,   489,   496,   503,   512,   519,   528,   538,   550,
     563,   564,   568,   573,   582,   587,   594,   599,   608,   616,
     628,   636,   637,   641,   642,   649,   655,   661,   669,   681,
     682,   683,   684,   685,   686,   687,   691,   692,   696,   701,
     709,   710,   720,   721,   725,   726,   727,   733,   740,   747,
     754,   762,   767,   774,   787,   803,   804,   805,   809,   810,
     814,   815,   819,   820,   830,   831,   835,   836,   840,   845,
     850,   855
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "intConst", "floatConst", "stringConst",
  "identifier", "'+'", "'-'", "'*'", "'/'", "'%'", "'^'", "'&'", "'|'",
  "'!'", "comparisonOps", "')'", "'('", "'='", "SELECT", "INSERT",
  "UPDATE", "DELETE", "PROVENANCE", "OF", "FROM", "AS", "WHERE",
  "DISTINCT", "STARALL", "AND", "OR", "LIKE", "NOT", "IN", "ISNULL",
  "BETWEEN", "EXCEPT", "EXISTS", "AMMSC", "NULLVAL", "ALL", "ANY", "IS",
  "SOME", "UNION", "INTERSECT", "MINUS", "INTO", "VALUES", "HAVING",
  "GROUP", "ORDER", "BY", "LIMIT", "SET", "INT", "BEGIN_TRANS",
  "COMMIT_TRANS", "ROLLBACK_TRANS", "DUMMYEXPR", "JOIN", "NATURAL", "LEFT",
  "RIGHT", "OUTER", "INNER", "CROSS", "ON", "USING", "FULL", "XOR", "';'",
  "','", "'.'", "$accept", "stmtList", "stmt", "dmlStmt", "queryStmt",
  "transactionIdentifier", "provStmt", "deleteQuery", "fromString",
  "updateQuery", "setClause", "setExpression", "insertQuery", "insertList",
  "setOperatorQuery", "optionalAll", "selectQuery", "optionalDistinct",
  "selectClause", "selectItem", "exprList", "expression", "constant",
  "attributeRef", "binaryOperatorExpression", "unaryOperatorExpression",
  "sqlFunctionCall", "optionalFrom", "fromClause", "fromClauseItem",
  "subQuery", "identifierList", "fromJoinItem", "joinType", "joinCond",
  "optionalAlias", "optionalAttrAlias", "optionalWhere", "whereExpression",
  "nestedSubQueryOperator", "optionalNot", "optionalGroupBy",
  "optionalHaving", "optionalOrderBy", "optionalLimit", "clauseList", YY_NULL
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,    43,    45,    42,
      47,    37,    94,    38,   124,    33,   262,    41,    40,    61,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,    59,    44,    46
};
# endif

#define YYPACT_NINF -108

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-108)))

#define YYTABLE_NINF -119

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-119)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      76,    10,   -18,   -24,    20,    16,     6,  -108,  -108,  -108,
      27,  -108,   -30,   -27,   -21,  -108,  -108,  -108,  -108,  -108,
    -108,   100,    19,   277,    74,    34,  -108,    85,    96,  -108,
    -108,  -108,    51,    10,    10,  -108,  -108,  -108,   102,  -108,
    -108,  -108,   -16,  -108,   244,   244,    15,  -108,   291,  -108,
    -108,  -108,  -108,  -108,    -6,   116,   109,    76,  -108,    10,
    -108,  -108,   244,   244,   149,   110,   358,   317,    11,   277,
     134,   244,   244,   244,   244,   244,   244,   244,   244,   244,
     158,   147,   325,  -108,   -20,  -108,   155,   127,   160,  -108,
     -14,   328,   -13,  -108,  -108,     9,   154,   101,   313,     9,
    -108,  -108,   127,   135,   186,   186,   196,   196,   196,  -108,
     358,   348,   378,  -108,   252,   116,  -108,   305,   127,   127,
     197,   230,   181,  -108,  -108,   244,  -108,   198,   213,  -108,
     154,   106,   313,   203,    11,    11,   119,   157,   168,  -108,
     162,   179,   189,  -108,   181,   206,   201,  -108,   -11,  -108,
    -108,   273,   328,  -108,   219,    75,   181,    10,    98,  -108,
     226,   127,   127,   127,   244,   328,   257,  -108,   198,   121,
    -108,     9,   313,   283,    11,   212,  -108,  -108,    11,  -108,
      11,   309,   278,   232,  -108,   388,   273,  -108,   146,   273,
    -108,  -108,  -108,   269,   271,    72,    72,   122,   259,  -108,
       5,  -108,   153,  -108,   127,   276,  -108,  -108,    11,  -108,
     283,  -108,  -108,   222,   110,   290,   262,   264,  -108,  -108,
    -108,   156,    10,    10,   244,  -108,   311,   181,   257,  -108,
    -108,   309,   244,   309,   392,  -108,  -108,   159,   163,   328,
    -108,     7,  -108,  -108,   328,   222,  -108,  -108,  -108,  -108
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    38,     0,     0,    19,     0,    14,    15,    16,
       0,     2,     0,     0,     0,    12,     8,     9,     7,    13,
      11,     0,    39,     0,     0,     0,    20,     0,     0,     1,
       3,     4,    35,     0,     0,     5,     6,    10,     0,    55,
      56,    57,    58,    45,     0,     0,    70,    41,    43,    50,
      51,    52,    53,    54,     0,     0,     0,     0,    36,     0,
      32,    33,     0,     0,     0,    58,    68,     0,     0,     0,
     102,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    27,    58,   102,    22,     0,     0,     0,    34,
       0,    47,     0,    46,    49,    74,     0,    71,    72,    76,
      78,    42,     0,   120,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    44,     0,     0,    21,     0,     0,     0,
       0,   105,    18,    17,    40,     0,    69,   100,     0,    75,
       0,     0,     0,    78,     0,     0,     0,    89,    91,    95,
       0,    94,     0,    77,   103,     0,   122,    29,     0,    28,
      23,     0,    24,    25,   105,     0,   106,     0,     0,   119,
       0,     0,     0,     0,     0,    48,     0,    98,   100,     0,
      80,    83,    73,     0,     0,     0,    90,    92,     0,    93,
       0,     0,     0,   124,    26,     0,     0,   104,     0,     0,
     116,   115,   117,     0,     0,   107,   108,   109,     0,    81,
       0,    99,    80,    79,     0,     0,    88,    84,     0,    86,
       0,   130,   128,   121,     0,     0,     0,   126,    30,    31,
     114,     0,     0,     0,     0,   101,     0,    97,     0,    85,
      87,     0,     0,     0,     0,    37,   112,     0,     0,   110,
      82,     0,   131,   129,   123,   125,   127,   111,   113,    96
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -108,  -108,     0,  -108,    -1,  -108,  -108,  -108,  -108,  -108,
    -108,   207,  -108,  -108,  -108,  -108,  -108,  -108,  -108,   263,
     258,    -7,  -107,   -42,  -108,  -108,   161,  -108,  -108,   -59,
     245,   151,   -95,   227,   139,   -94,   214,   299,   -79,  -108,
    -108,  -108,  -108,  -108,  -108,   165
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    10,    11,    12,    13,    14,    15,    16,    27,    17,
      84,    85,    18,   148,    19,    59,    20,    23,    46,    47,
      90,   121,    49,    50,    51,    52,    53,    70,    97,   132,
      99,   200,   100,   142,   206,   129,   167,   103,   122,   193,
     160,   146,   183,   217,   235,   213
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      21,   133,    63,   124,   126,   143,   184,   149,   102,    98,
      30,    22,     1,    86,     2,   127,    48,    95,     6,    32,
      33,    34,   225,   144,   249,    24,    25,    29,     1,    96,
       2,    28,    60,    61,     6,   133,   128,    66,    67,   155,
     156,    68,    26,    31,    81,     1,    35,     2,     3,     4,
       5,     6,    36,    82,   115,    91,    91,    88,    89,    64,
     125,   125,    48,   185,   104,   105,   106,   107,   108,   109,
     110,   111,   112,    86,   211,   172,   173,   203,   219,   226,
      54,   226,   195,   196,   197,     7,     8,     9,    38,    69,
      55,    56,   187,    58,     1,   131,     2,     3,     4,     5,
       6,    39,    40,    41,    65,   163,   161,   162,   163,   164,
     152,   154,   164,    44,    57,   207,   189,    37,   165,   209,
      62,   210,    83,   170,   242,   227,   211,   246,    63,   169,
      39,    40,    41,    65,     7,     8,     9,    87,   202,   212,
     190,   191,    44,   192,    67,   118,    32,    33,    34,   229,
     131,   112,    32,    33,    34,  -119,   188,   198,    93,  -119,
      95,   119,   102,   220,   113,   114,   120,    32,    33,    34,
     -10,   117,   130,   236,     2,   134,   247,   123,     6,    67,
     248,   174,    67,   137,   138,    21,   139,   145,   221,   243,
     141,   212,    32,    33,    34,    73,    74,    75,    76,   -10,
     -10,   -10,    32,    33,    34,    32,    33,    34,    76,    32,
      33,    34,   161,   162,   163,   157,   166,   239,   164,   168,
     171,   237,   238,   176,   178,   244,    71,    72,    73,    74,
      75,    76,    77,    78,   177,   158,    94,    71,    72,    73,
      74,    75,    76,    77,    78,   179,   158,    39,    40,    41,
      65,   180,   182,   159,  -118,    39,    40,    41,   147,    44,
     181,   194,    45,   199,   159,  -118,    71,    72,    73,    74,
      75,    76,    77,    78,   208,    79,    39,    40,    41,    65,
      39,    40,    41,    42,   214,   216,    43,   222,    44,   223,
     224,   186,    44,     2,   228,    45,   231,     6,    71,    72,
      73,    74,    75,    76,    77,    78,   232,    79,    39,    40,
      41,    65,    39,    40,    41,    83,   233,   240,    80,   234,
      44,    92,   150,   151,    71,    72,    73,    74,    75,    76,
      77,    78,   101,    79,    94,    71,    72,    73,    74,    75,
      76,    77,    78,   215,    79,   135,   136,   137,   138,   230,
     139,   140,   204,   205,   141,    71,    72,    73,    74,    75,
      76,    77,   153,   175,    79,    71,    72,    73,    74,    75,
      76,    32,    33,    34,    79,   135,   136,   137,   138,   241,
     139,   140,   201,   116,   141,    71,    72,    73,    74,    75,
      76,    39,    40,    41,   218,    39,    40,    41,   245
};

static const yytype_uint8 yycheck[] =
{
       1,    96,    18,    17,    17,    99,    17,   114,    28,    68,
      10,    29,    18,    55,    20,     6,    23,     6,    24,    46,
      47,    48,    17,   102,    17,    49,     6,     0,    18,    18,
      20,    25,    33,    34,    24,   130,    27,    44,    45,   118,
     119,    26,    26,    73,    50,    18,    73,    20,    21,    22,
      23,    24,    73,    54,    74,    62,    63,    57,    59,    75,
      74,    74,    69,    74,    71,    72,    73,    74,    75,    76,
      77,    78,    79,   115,   181,   134,   135,   171,   185,    74,
       6,    74,   161,   162,   163,    58,    59,    60,    69,    74,
      56,     6,    17,    42,    18,    96,    20,    21,    22,    23,
      24,     3,     4,     5,     6,    33,    31,    32,    33,    37,
     117,   118,    37,    15,    18,   174,    18,    17,   125,   178,
      18,   180,     6,    17,   231,   204,   233,   234,    18,   130,
       3,     4,     5,     6,    58,    59,    60,    28,    17,   181,
      42,    43,    15,    45,   151,    18,    46,    47,    48,   208,
     151,   158,    46,    47,    48,    33,   157,   164,     9,    37,
       6,    34,    28,    17,     6,    18,    39,    46,    47,    48,
      17,    16,    18,    17,    20,    74,    17,    17,    24,   186,
      17,    62,   189,    64,    65,   186,    67,    52,   189,   231,
      71,   233,    46,    47,    48,     9,    10,    11,    12,    46,
      47,    48,    46,    47,    48,    46,    47,    48,    12,    46,
      47,    48,    31,    32,    33,    18,    18,   224,    37,     6,
      17,   222,   223,    66,    62,   232,     7,     8,     9,    10,
      11,    12,    13,    14,    66,    16,    17,     7,     8,     9,
      10,    11,    12,    13,    14,    66,    16,     3,     4,     5,
       6,    62,    51,    34,    35,     3,     4,     5,     6,    15,
      54,    35,    18,     6,    34,    35,     7,     8,     9,    10,
      11,    12,    13,    14,    62,    16,     3,     4,     5,     6,
       3,     4,     5,     6,     6,    53,     9,    18,    15,    18,
      31,    18,    15,    20,    18,    18,    74,    24,     7,     8,
       9,    10,    11,    12,    13,    14,    16,    16,     3,     4,
       5,     6,     3,     4,     5,     6,    54,     6,    27,    55,
      15,    63,   115,    18,     7,     8,     9,    10,    11,    12,
      13,    14,    69,    16,    17,     7,     8,     9,    10,    11,
      12,    13,    14,   182,    16,    62,    63,    64,    65,   210,
      67,    68,    69,    70,    71,     7,     8,     9,    10,    11,
      12,    13,   117,   136,    16,     7,     8,     9,    10,    11,
      12,    46,    47,    48,    16,    62,    63,    64,    65,   228,
      67,    68,   168,    84,    71,     7,     8,     9,    10,    11,
      12,     3,     4,     5,     6,     3,     4,     5,   233
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    18,    20,    21,    22,    23,    24,    58,    59,    60,
      77,    78,    79,    80,    81,    82,    83,    85,    88,    90,
      92,    80,    29,    93,    49,     6,    26,    84,    25,     0,
      78,    73,    46,    47,    48,    73,    73,    17,    69,     3,
       4,     5,     6,     9,    15,    18,    94,    95,    97,    98,
      99,   100,   101,   102,     6,    56,     6,    18,    42,    91,
      80,    80,    18,    18,    75,     6,    97,    97,    26,    74,
     103,     7,     8,     9,    10,    11,    12,    13,    14,    16,
      27,    50,    80,     6,    86,    87,    99,    28,    78,    80,
      96,    97,    96,     9,    17,     6,    18,   104,   105,   106,
     108,    95,    28,   113,    97,    97,    97,    97,    97,    97,
      97,    97,    97,     6,    18,    74,   113,    16,    18,    34,
      39,    97,   114,    17,    17,    74,    17,     6,    27,   111,
      18,    80,   105,   108,    74,    62,    63,    64,    65,    67,
      68,    71,   109,   111,   114,    52,   117,     6,    89,    98,
      87,    18,    97,   106,    97,   114,   114,    18,    16,    34,
     116,    31,    32,    33,    37,    97,    18,   112,     6,    80,
      17,    17,   105,   105,    62,   109,    66,    66,    62,    66,
      62,    54,    51,   118,    17,    74,    18,    17,    80,    18,
      42,    43,    45,   115,    35,   114,   114,   114,    97,     6,
     107,   112,    17,   111,    69,    70,   110,   105,    62,   105,
     105,    98,    99,   121,     6,   102,    53,   119,     6,    98,
      17,    80,    18,    18,    31,    17,    74,   114,    18,   105,
     110,    74,    16,    54,    55,   120,    17,    80,    80,    97,
       6,   107,    98,    99,    97,   121,    98,    17,    17,    17
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    76,    77,    77,    78,    78,    78,    79,    79,    79,
      80,    80,    80,    80,    81,    81,    81,    82,    83,    84,
      84,    85,    86,    86,    87,    87,    88,    88,    89,    89,
      89,    89,    90,    90,    90,    91,    91,    92,    93,    93,
      93,    94,    94,    95,    95,    95,    95,    96,    96,    97,
      97,    97,    97,    97,    97,    98,    98,    98,    99,   100,
     100,   100,   100,   100,   100,   100,   100,   100,   101,   102,
     103,   103,   104,   104,   105,   105,   105,   105,   105,   105,
     106,   107,   107,   108,   108,   108,   108,   108,   108,   109,
     109,   109,   109,   109,   109,   109,   110,   110,   111,   111,
     112,   112,   113,   113,   114,   114,   114,   114,   114,   114,
     114,   114,   114,   114,   114,   115,   115,   115,   116,   116,
     117,   117,   118,   118,   119,   119,   120,   120,   121,   121,
     121,   121
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     2,     2,     2,     1,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     5,     5,     0,
       1,     5,     1,     3,     3,     3,     7,     4,     1,     1,
       3,     3,     3,     3,     4,     0,     1,     9,     0,     1,
       5,     1,     3,     1,     3,     1,     3,     1,     3,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     4,
       0,     2,     1,     3,     1,     2,     1,     2,     1,     4,
       3,     1,     3,     3,     4,     5,     4,     5,     4,     1,
       2,     1,     2,     2,     1,     1,     4,     2,     2,     3,
       0,     3,     0,     2,     3,     1,     2,     3,     3,     3,
       5,     6,     5,     6,     4,     1,     1,     1,     0,     1,
       0,     3,     0,     4,     0,     3,     0,     2,     1,     3,
       1,     3
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
  YYSIZE_T yysize0 = yytnamerr (YY_NULL, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULL;
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
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULL, yytname[yyx]);
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
#line 124 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("stmtList::stmt"); 
				(yyval.list) = singleton((yyvsp[0].node));
				bisonParseResult = (Node *) (yyval.list);	 
			}
#line 1493 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 130 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("stmtlist::stmtList::stmt");
				(yyval.list) = appendToTailOfList((yyvsp[-1].list), (yyvsp[0].node));	
				bisonParseResult = (Node *) (yyval.list); 
			}
#line 1503 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 139 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::dmlStmt");
            (yyval.node) = (yyvsp[-1].node);
        }
#line 1512 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 144 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::queryStmt");
            (yyval.node) = (yyvsp[-1].node);
        }
#line 1521 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 149 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::transactionIdentifier");
            (yyval.node) = (Node *) createTransactionStmt((yyvsp[-1].stringVal));
        }
#line 1530 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 159 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::insertQuery"); }
#line 1536 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 160 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::deleteQuery"); }
#line 1542 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 161 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::updateQuery"); }
#line 1548 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 168 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::bracketedQuery"); (yyval.node) = (yyvsp[-1].node); }
#line 1554 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 169 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::selectQuery"); }
#line 1560 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 170 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::provStmt"); }
#line 1566 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 171 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::setOperatorQuery"); }
#line 1572 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 175 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::BEGIN"); (yyval.stringVal) = strdup("TRANSACTION_BEGIN"); }
#line 1578 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 176 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::COMMIT"); (yyval.stringVal) = strdup("TRANSACTION_COMMIT"); }
#line 1584 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 177 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::ROLLBACK"); (yyval.stringVal) = strdup("TRANSACTION_ABORT"); }
#line 1590 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 185 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG(provStmt);
            (yyval.node) = (Node *) createProvenanceStmt((yyvsp[-1].node));
        }
#line 1599 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 196 "sql_parser.y" /* yacc.c:1646  */
    { 
             RULELOG("deleteQuery");
             (yyval.node) = (Node *) createDelete((yyvsp[-2].stringVal), (yyvsp[0].node));
         }
#line 1608 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 204 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::NULL"); (yyval.stringVal) = NULL; }
#line 1614 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 205 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::FROM"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 1620 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 220 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG(updateQuery); 
                (yyval.node) = (Node *) createUpdate((yyvsp[-3].stringVal), (yyvsp[-1].list), (yyvsp[0].node)); 
            }
#line 1629 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 228 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setExpression");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 1638 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 233 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setClause::setExpression");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 1647 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 241 "sql_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal),"=")) {
                    RULELOG("setExpression::attributeRef::expression");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 1660 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 250 "sql_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal), "=")) {
                    RULELOG("setExpression::attributeRef::queryStmt");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 1673 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 265 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertQuery::insertList"); 
            	(yyval.node) = (Node *) createInsert((yyvsp[-4].stringVal),(Node *) (yyvsp[-1].list), NULL); 
        	}
#line 1682 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 270 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("insertQuery::queryStmt");
                (yyval.node) = (Node *) createInsert((yyvsp[-1].stringVal), (yyvsp[0].node), NULL);
            }
#line 1691 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 278 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::constant");
            	(yyval.list) = singleton((yyvsp[0].node)); 
            }
#line 1700 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 283 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("insertList::IDENTIFIER");
                (yyval.list) = singleton((yyvsp[0].stringVal));
            }
#line 1709 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 288 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("insertList::insertList::::IDENTIFIER");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].stringVal));
            }
#line 1718 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 293 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::insertList::constant");
            	(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 1727 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 307 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::INTERSECT");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 1736 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 312 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::MINUS");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 1745 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 317 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::UNION");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-2].stringVal), ((yyvsp[-1].stringVal) != NULL), (yyvsp[-3].node), (yyvsp[0].node));
            }
#line 1754 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 324 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::NULL"); (yyval.stringVal) = NULL; }
#line 1760 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 325 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::ALLTRUE"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 1766 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 335 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG(selectQuery);
                QueryBlock *q =  createQueryBlock();
                
                q->distinct = (yyvsp[-7].node);
                q->selectClause = (yyvsp[-6].list);
                q->fromClause = (yyvsp[-5].list);
                q->whereClause = (yyvsp[-4].node);
                q->groupByClause = (yyvsp[-3].list);
                q->havingClause = (yyvsp[-2].node);
                q->orderByClause = (yyvsp[-1].list);
                q->limitClause = (yyvsp[0].node);
                
                (yyval.node) = (Node *) q; 
            }
#line 1786 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 357 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalDistinct::NULL"); (yyval.node) = NULL; }
#line 1792 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 359 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT");
                (yyval.node) = (Node *) createDistinctClause(NULL);
            }
#line 1801 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 364 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT::exprList");
                (yyval.node) = (Node *) createDistinctClause((yyvsp[-1].list));
            }
#line 1810 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 376 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectItem"); (yyval.list) = singleton((yyvsp[0].node));
            }
#line 1818 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 380 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectClause::selectItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node)); 
            }
#line 1827 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 388 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression"); 
                 (yyval.node) = (Node *) createSelectItem(NULL, (yyvsp[0].node)); 
             }
#line 1836 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 393 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression::identifier"); 
                 (yyval.node) = (Node *) createSelectItem((yyvsp[0].stringVal), (yyvsp[-2].node));
             }
#line 1845 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 398 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
         		(yyval.node) = (Node *) createSelectItem(strdup("*"), NULL); 
     		}
#line 1854 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 403 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
     			(yyval.node) = (Node *) createSelectItem(CONCAT_STRINGS((yyvsp[-2].stringVal),".*"), NULL); 
 			}
#line 1863 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 413 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("exprList::SINGLETON"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 1869 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 415 "sql_parser.y" /* yacc.c:1646  */
    {
                  RULELOG("exprList::exprList::expression");
                  (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
             }
#line 1878 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 426 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::bracked"); (yyval.node) = (yyvsp[-1].node); }
#line 1884 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 427 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::constant"); }
#line 1890 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 428 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::attributeRef"); }
#line 1896 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 429 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::binaryOperatorExpression"); }
#line 1902 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 430 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::unaryOperatorExpression"); }
#line 1908 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 431 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlFunctionCall"); }
#line 1914 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 440 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::INT"); (yyval.node) = (Node *) createConstInt((yyvsp[0].intVal)); }
#line 1920 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 441 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::FLOAT"); (yyval.node) = (Node *) createConstFloat((yyvsp[0].floatVal)); }
#line 1926 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 442 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::STRING"); (yyval.node) = (Node *) createConstString((yyvsp[0].stringVal)); }
#line 1932 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 449 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("attributeRef::IDENTIFIER"); (yyval.node) = (Node *) createAttributeReference((yyvsp[0].stringVal)); }
#line 1938 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 469 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '+' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1949 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 476 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '-' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1960 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 483 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '*' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1971 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 490 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '/' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1982 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 497 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '%' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1993 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 504 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '^' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2004 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 513 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '&' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2015 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 520 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '|' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2026 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 529 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression::comparisonOps");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2037 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 539 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("unaryOperatorExpression:: '!' ");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2047 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 551 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("sqlFunctionCall::IDENTIFIER::exprList"); 
                (yyval.node) = (Node *) createFunctionCall((yyvsp[-3].stringVal), (yyvsp[-1].list)); 
            }
#line 2056 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 563 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::NULL"); (yyval.list) = NULL; }
#line 2062 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 564 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::fromClause"); (yyval.list) = (yyvsp[0].list); }
#line 2068 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 569 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClauseItem");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2077 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 574 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClause::fromClauseItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2086 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 583 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
                (yyval.node) = (Node *) createFromTableRef(NULL, NIL, (yyvsp[0].stringVal));
            }
#line 2095 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 588 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
                (yyval.node) = (Node *) createFromTableRef(((FromItem *) (yyvsp[0].node))->name, 
						((FromItem *) (yyvsp[0].node))->attrNames, (yyvsp[-1].stringVal));
            }
#line 2105 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 595 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                (yyval.node) = (yyvsp[0].node);
            }
#line 2114 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 600 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromSubquery *s = (FromSubquery *) (yyvsp[-1].node);
                s->from.name = ((FromItem *) (yyvsp[0].node))->name;
                s->from.attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
                (yyval.node) = (Node *) s;
            }
#line 2126 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 609 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[0].node);
        		f->name = NULL;
        		(yyval.node) = (Node *) f;
        	}
#line 2138 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 617 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[-2].node);
        		f->name = ((FromItem *) (yyvsp[0].node))->name;
                f->attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
        		(yyval.node) = (Node *) f;
        	}
#line 2151 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 629 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("subQuery::queryStmt");
                (yyval.node) = (Node *) createFromSubquery(NULL, NULL, (yyvsp[-1].node));
            }
#line 2160 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 636 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = singleton((yyvsp[0].stringVal)); }
#line 2166 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 637 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].stringVal)); }
#line 2172 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 641 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2178 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 643 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURAL");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
						(FromItem *) (yyvsp[0].node), "JOIN_INNER", "JOIN_COND_NATURAL", 
						NULL);
          	}
#line 2189 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 650 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURALjoinType");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[0].node), (yyvsp[-2].stringVal), "JOIN_COND_NATURAL", NULL);
          	}
#line 2199 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 656 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::CROSS JOIN");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[0].node), "JOIN_CROSS", "JOIN_COND_ON", NULL);
          	}
#line 2209 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 662 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinType::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON";
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[-1].node), (yyvsp[-3].stringVal), condType, (yyvsp[0].node));
          	}
#line 2221 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 670 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON"; 
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[-1].node), "JOIN_INNER", 
                		condType, (yyvsp[0].node));
          	}
#line 2234 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 681 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2240 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 682 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT OUTER"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2246 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 683 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT "); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2252 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 684 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT OUTER"); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2258 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 685 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL OUTER"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2264 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 686 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2270 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 687 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::INNER"); (yyval.stringVal) = "JOIN_INNER"; }
#line 2276 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 691 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (Node *) (yyvsp[-1].list); }
#line 2282 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 692 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2288 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 697 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAlias::identifier"); 
				(yyval.node) = (Node *) createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list)); 
			}
#line 2297 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 702 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAlias::identifier"); 
				(yyval.node) = (Node *) createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list)); 
			}
#line 2306 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 709 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAttrAlias::empty"); (yyval.list) = NULL; }
#line 2312 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 711 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAttrAlias::identifierList"); (yyval.list) = (yyvsp[-1].list); 
			}
#line 2320 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 720 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::NULL"); (yyval.node) = NULL; }
#line 2326 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 721 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::whereExpression"); (yyval.node) = (yyvsp[0].node); }
#line 2332 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 725 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("where::brackedWhereExpression"); (yyval.node) = (yyvsp[-1].node); }
#line 2338 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 726 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("whereExpression::expression"); (yyval.node) = (yyvsp[0].node); }
#line 2344 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 728 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::NOT");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2354 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 734 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2365 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 741 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2376 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 748 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2387 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 755 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::BETWEEN-AND");
                List *expr = singleton((yyvsp[-4].node));
                expr = appendToTailOfList(expr, (yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr);
            }
#line 2399 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 763 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::comparisonOps::nestedSubQueryOperator::Subquery");
                (yyval.node) = (Node *) createNestedSubquery((yyvsp[-3].stringVal), (yyvsp[-5].node), (yyvsp[-4].stringVal), (yyvsp[-1].node));
            }
#line 2408 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 768 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::comparisonOps::Subquery");
                List *expr = singleton((yyvsp[-4].node));
                expr = appendToTailOfList(expr, (yyvsp[-1].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr);
            }
#line 2419 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 775 "sql_parser.y" /* yacc.c:1646  */
    {
                if ((yyvsp[-4].stringVal) == NULL)
                {
                    RULELOG("whereExpression::IN");
                    (yyval.node) = (Node *) createNestedSubquery("ANY", (yyvsp[-5].node), "=", (yyvsp[-1].node));
                }
                else
                {
                    RULELOG("whereExpression::NOT::IN");
                    (yyval.node) = (Node *) createNestedSubquery("ALL",(yyvsp[-5].node), "<>", (yyvsp[-1].node));
                }
            }
#line 2436 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 788 "sql_parser.y" /* yacc.c:1646  */
    {
                /* if ($1 == NULL)
                { */
                    RULELOG("whereExpression::EXISTS");
                    (yyval.node) = (Node *) createNestedSubquery((yyvsp[-3].stringVal), NULL, NULL, (yyvsp[-1].node));
               /*  }
                else
                {
                    RULELOG("whereExpression::EXISTS::NOT");
                    $$ = (Node *) createNestedSubquery($2, NULL, "<>", $4);
                } */
            }
#line 2453 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 803 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ANY"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2459 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 804 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ALL"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2465 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 805 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::SOME"); (yyval.stringVal) = "ANY"; }
#line 2471 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 809 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NULL"); (yyval.stringVal) = NULL; }
#line 2477 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 810 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NOT"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2483 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 814 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::NULL"); (yyval.list) = NULL; }
#line 2489 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 815 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::GROUPBY"); (yyval.list) = (yyvsp[0].list); }
#line 2495 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 819 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.node) = NULL; }
#line 2501 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 821 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("optionalHaving::HAVING"); 
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2512 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 830 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.list) = NULL; }
#line 2518 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 831 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy::ORDERBY"); (yyval.list) = (yyvsp[0].list); }
#line 2524 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 835 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::NULL"); (yyval.node) = NULL; }
#line 2530 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 836 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::CONSTANT"); (yyval.node) = (yyvsp[0].node);}
#line 2536 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 841 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::attributeRef");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2545 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 846 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2554 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 851 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::constant");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2563 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 856 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2572 "sql_parser.tab.c" /* yacc.c:1646  */
    break;


#line 2576 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 863 "sql_parser.y" /* yacc.c:1906  */




/* FUTURE WORK 

PRIORITIES
7)
4)
1)

EXHAUSTIVE LIST
1. Implement support for Case when statemets for all type of queries.
2. Implement support for RETURNING statement in DELETE queries.
3. Implement support for column list like (col1, col2, col3). 
   Needed in insert queries, select queries where conditions etc.
4. Implement support for Transactions.
5. Implement support for Create queries.
6. Implement support for windowing functions.
7. Implement support for AS OF (timestamp) modifier of a table reference
8. Implement support for casting expressions
9. Implement support for IN array expressions like a IN (1,2,3,4,5)
*/

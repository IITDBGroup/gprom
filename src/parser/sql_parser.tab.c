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

Node *bisonParseResult = NULL;

#line 83 "sql_parser.tab.c" /* yacc.c:339  */

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
    DUMMYEXPR = 301,
    JOIN = 302,
    NATURAL = 303,
    LEFT = 304,
    RIGHT = 305,
    OUTER = 306,
    INNER = 307,
    CROSS = 308,
    ON = 309,
    USING = 310,
    FULL = 311,
    XOR = 312
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 25 "sql_parser.y" /* yacc.c:355  */

    /* 
     * Declare some C structure those will be used as data type
     * for various tokens used in grammar rules.
     */
     Node *node;
     List *list;
     char *stringVal;
     int intVal;
     double floatVal;

#line 193 "sql_parser.tab.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SQL_PARSER_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 208 "sql_parser.tab.c" /* yacc.c:358  */

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
#define YYFINAL  25
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   401

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  73
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  44
/* YYNRULES -- Number of rules.  */
#define YYNRULES  122
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  240

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   312

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
      18,    17,     9,     7,    71,     8,    72,    10,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    70,
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
      67,    68,    69
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   120,   120,   126,   134,   139,   150,   151,   152,   159,
     160,   161,   162,   169,   180,   189,   190,   204,   212,   217,
     225,   234,   249,   254,   262,   267,   272,   277,   291,   296,
     301,   309,   310,   319,   342,   343,   348,   360,   364,   372,
     377,   382,   387,   399,   400,   412,   413,   414,   415,   416,
     417,   426,   427,   428,   435,   454,   461,   468,   475,   482,
     489,   498,   505,   514,   524,   536,   549,   550,   554,   559,
     568,   573,   579,   584,   592,   600,   611,   619,   620,   623,
     624,   629,   634,   639,   645,   655,   656,   657,   658,   662,
     663,   666,   667,   674,   675,   679,   680,   681,   687,   694,
     701,   708,   716,   721,   728,   741,   757,   758,   759,   763,
     764,   768,   769,   773,   774,   784,   785,   789,   790,   794,
     799,   804,   809
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
  "GROUP", "ORDER", "BY", "LIMIT", "SET", "INT", "DUMMYEXPR", "JOIN",
  "NATURAL", "LEFT", "RIGHT", "OUTER", "INNER", "CROSS", "ON", "USING",
  "FULL", "XOR", "';'", "','", "'.'", "$accept", "stmtList", "stmt",
  "dmlStmt", "queryStmt", "provStmt", "deleteQuery", "fromString",
  "updateQuery", "setClause", "setExpression", "insertQuery", "insertList",
  "setOperatorQuery", "optionalAll", "selectQuery", "optionalDistinct",
  "selectClause", "selectItem", "exprList", "expression", "constant",
  "attributeRef", "binaryOperatorExpression", "unaryOperatorExpression",
  "sqlFunctionCall", "optionalFrom", "fromClause", "fromClauseItem",
  "subQuery", "identifierList", "fromJoinItem", "joinType", "joinCond",
  "optionalAlias", "optionalWhere", "whereExpression",
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
      59,    44,    46
};
# endif

#define YYPACT_NINF -105

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-105)))

#define YYTABLE_NINF -110

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-110)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     355,     0,   -12,   -33,    13,     1,     5,   167,  -105,   -45,
      61,  -105,  -105,  -105,  -105,  -105,  -105,    -6,   -35,   265,
      39,    20,  -105,    49,    67,  -105,  -105,  -105,    42,     0,
       0,  -105,  -105,    74,  -105,  -105,  -105,   -16,  -105,   293,
     293,   -19,  -105,   279,  -105,  -105,  -105,  -105,  -105,    73,
      96,    76,   355,  -105,     0,  -105,  -105,   293,   293,   102,
      94,   228,   309,     8,   265,   101,   293,   293,   293,   293,
     293,   293,   293,   293,   293,   128,   122,   -11,  -105,   -25,
    -105,   126,   132,   127,  -105,   -13,   320,    -2,  -105,  -105,
       7,   227,    72,   298,     7,  -105,  -105,   132,    97,   377,
     377,   142,   142,   142,  -105,   228,   340,   373,  -105,   387,
      96,  -105,   297,   132,   132,   130,   218,    63,  -105,  -105,
     293,  -105,  -105,   149,  -105,   227,    32,   298,   152,     8,
       8,   306,   107,   111,  -105,   114,   129,   121,  -105,    63,
     139,   125,  -105,     4,  -105,  -105,   261,   320,  -105,   207,
     169,    63,     0,    83,  -105,   168,   132,   132,   132,   293,
     320,  -105,   110,  -105,     7,   298,   278,     8,   145,  -105,
    -105,     8,  -105,     8,   391,   199,   155,  -105,   395,   261,
    -105,   113,   261,  -105,  -105,  -105,   191,   192,    10,    10,
      66,   247,   116,  -105,   132,   204,  -105,  -105,     8,  -105,
     278,  -105,  -105,   172,    94,   230,   194,   195,  -105,  -105,
    -105,   136,     0,     0,   293,    63,   243,  -105,  -105,   391,
     293,   391,   117,  -105,  -105,   148,   151,   320,  -105,     6,
    -105,  -105,   320,   172,  -105,  -105,  -105,  -105,   256,  -105
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    34,     0,     0,    15,     0,     0,     2,     0,
       0,    11,     7,     8,     6,    12,    10,     0,    35,     0,
       0,     0,    16,     0,     0,     1,     3,     4,    31,     0,
       0,     5,     9,     0,    51,    52,    53,    54,    41,     0,
       0,    66,    37,    39,    46,    47,    48,    49,    50,     0,
       0,     0,     0,    32,     0,    28,    29,     0,     0,     0,
      54,    64,     0,     0,     0,    93,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    23,    54,    93,
      18,     0,     0,     0,    30,     0,    43,     0,    42,    45,
      70,     0,    67,    68,    72,    74,    38,     0,   111,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    40,     0,
       0,    17,     0,     0,     0,     0,    96,    14,    13,    36,
       0,    65,    91,     0,    71,     0,     0,     0,    74,     0,
       0,     0,     0,     0,    88,     0,     0,     0,    73,    94,
       0,   113,    25,     0,    24,    19,     0,    20,    21,    96,
       0,    97,     0,     0,   110,     0,     0,     0,     0,     0,
      44,    92,     0,    76,    79,    69,     0,     0,     0,    85,
      86,     0,    87,     0,     0,     0,   115,    22,     0,     0,
      95,     0,     0,   107,   106,   108,     0,     0,    98,    99,
     100,     0,    76,    75,     0,     0,    84,    81,     0,    82,
       0,   121,   119,   112,     0,     0,     0,   117,    26,    27,
     105,     0,     0,     0,     0,    90,     0,    80,    83,     0,
       0,     0,     0,    33,   103,     0,     0,   101,    77,     0,
     122,   120,   114,   116,   118,   102,   104,    89,     0,    78
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -105,  -105,     2,  -105,    -1,  -105,  -105,  -105,  -105,  -105,
     162,  -105,  -105,  -105,  -105,  -105,  -105,  -105,   209,   217,
      -7,  -104,   -42,  -105,  -105,   109,  -105,  -105,   -57,   165,
    -105,   -81,   163,    82,   -93,   225,   -75,  -105,  -105,  -105,
    -105,  -105,  -105,    84
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     7,     8,     9,    10,    11,    12,    23,    13,    79,
      80,    14,   143,    15,    54,    16,    19,    41,    42,    85,
     116,    44,    45,    46,    47,    48,    65,    92,   127,    94,
     229,    95,   137,   196,   124,    98,   117,   186,   155,   141,
     176,   207,   223,   203
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      17,   138,    58,    97,   119,   144,    93,    63,    81,    26,
     128,    32,    43,   122,    90,   121,    20,    18,     1,    21,
       2,   177,   139,   237,     6,    27,    91,    22,    55,    56,
      24,    33,    61,    62,   123,    28,    29,    30,   150,   151,
      28,    29,    30,   158,   128,    49,   110,   159,    77,   163,
      86,    86,    64,    84,    83,    51,    59,    43,   120,    99,
     100,   101,   102,   103,   104,   105,   106,   107,    81,   120,
     201,   193,   165,   166,   209,   178,    50,   238,    28,    29,
      30,   188,   189,   190,    53,    52,    34,    35,    36,    60,
     126,     1,    57,     2,   156,   157,   158,     6,    39,  -110,
     159,   182,    78,  -110,    82,   147,   149,    28,    29,    30,
     197,    88,    58,   160,   199,   230,   200,   201,   234,   215,
      34,    35,    36,    76,   162,   183,   184,   192,   185,    97,
     210,    31,   202,    -9,   108,    34,    35,    36,    60,    62,
     109,   217,   112,   129,   118,   126,   107,    39,   152,   140,
     113,   181,   191,   224,    71,   161,    28,    29,    30,    28,
      29,    30,    -9,    -9,    -9,   235,   114,    25,   236,   164,
     169,   115,    62,   171,   170,    62,   175,   231,    17,   202,
     173,   211,    28,    29,    30,     1,   180,     2,     3,     4,
       5,     6,   172,   174,    28,    29,    30,    28,    29,    30,
     156,   157,   158,   187,   198,   204,   159,   227,   206,   212,
     213,   225,   226,   232,    66,    67,    68,    69,    70,    71,
      72,    73,   216,   153,    89,    66,    67,    68,    69,    70,
      71,    72,    73,    90,   153,    66,    67,    68,    69,    70,
      71,   154,  -109,   219,    74,   125,   220,     2,   221,   228,
     222,     6,   154,  -109,    66,    67,    68,    69,    70,    71,
      72,    73,   239,    74,    34,    35,    36,    60,    34,    35,
      36,    37,   145,    96,    38,    87,    39,   148,   214,   179,
      39,     2,   218,    40,   205,     6,    66,    67,    68,    69,
      70,    71,    72,    73,   168,    74,    34,    35,    36,    60,
      34,    35,    36,    60,   111,   233,    75,     0,    39,     0,
       0,    40,    39,     0,     0,   146,    66,    67,    68,    69,
      70,    71,    72,    73,     0,    74,    89,    66,    67,    68,
      69,    70,    71,    72,    73,     0,    74,   130,   131,   132,
     133,     0,   134,   135,   194,   195,   136,    66,    67,    68,
      69,    70,    71,    72,     0,     0,    74,   130,   131,   132,
     133,     0,   134,   135,     0,   167,   136,   132,   133,     0,
     134,     0,     0,     1,   136,     2,     3,     4,     5,     6,
      66,    67,    68,    69,    70,    71,    68,    69,    70,    71,
      34,    35,    36,   142,    34,    35,    36,    78,    34,    35,
      36,   208
};

static const yytype_int16 yycheck[] =
{
       1,    94,    18,    28,    17,   109,    63,    26,    50,     7,
      91,    17,    19,     6,     6,    17,    49,    29,    18,     6,
      20,    17,    97,    17,    24,    70,    18,    26,    29,    30,
      25,    66,    39,    40,    27,    46,    47,    48,   113,   114,
      46,    47,    48,    33,   125,     6,    71,    37,    49,    17,
      57,    58,    71,    54,    52,     6,    72,    64,    71,    66,
      67,    68,    69,    70,    71,    72,    73,    74,   110,    71,
     174,   164,   129,   130,   178,    71,    56,    71,    46,    47,
      48,   156,   157,   158,    42,    18,     3,     4,     5,     6,
      91,    18,    18,    20,    31,    32,    33,    24,    15,    33,
      37,    18,     6,    37,    28,   112,   113,    46,    47,    48,
     167,     9,    18,   120,   171,   219,   173,   221,   222,   194,
       3,     4,     5,    50,   125,    42,    43,    17,    45,    28,
      17,    70,   174,    17,     6,     3,     4,     5,     6,   146,
      18,   198,    16,    71,    17,   146,   153,    15,    18,    52,
      18,   152,   159,    17,    12,     6,    46,    47,    48,    46,
      47,    48,    46,    47,    48,    17,    34,     0,    17,    17,
      63,    39,   179,    59,    63,   182,    51,   219,   179,   221,
      59,   182,    46,    47,    48,    18,    17,    20,    21,    22,
      23,    24,    63,    54,    46,    47,    48,    46,    47,    48,
      31,    32,    33,    35,    59,     6,    37,   214,    53,    18,
      18,   212,   213,   220,     7,     8,     9,    10,    11,    12,
      13,    14,    18,    16,    17,     7,     8,     9,    10,    11,
      12,    13,    14,     6,    16,     7,     8,     9,    10,    11,
      12,    34,    35,    71,    16,    18,    16,    20,    54,     6,
      55,    24,    34,    35,     7,     8,     9,    10,    11,    12,
      13,    14,     6,    16,     3,     4,     5,     6,     3,     4,
       5,     6,   110,    64,     9,    58,    15,   112,    31,    18,
      15,    20,   200,    18,   175,    24,     7,     8,     9,    10,
      11,    12,    13,    14,   131,    16,     3,     4,     5,     6,
       3,     4,     5,     6,    79,   221,    27,    -1,    15,    -1,
      -1,    18,    15,    -1,    -1,    18,     7,     8,     9,    10,
      11,    12,    13,    14,    -1,    16,    17,     7,     8,     9,
      10,    11,    12,    13,    14,    -1,    16,    59,    60,    61,
      62,    -1,    64,    65,    66,    67,    68,     7,     8,     9,
      10,    11,    12,    13,    -1,    -1,    16,    59,    60,    61,
      62,    -1,    64,    65,    -1,    59,    68,    61,    62,    -1,
      64,    -1,    -1,    18,    68,    20,    21,    22,    23,    24,
       7,     8,     9,    10,    11,    12,     9,    10,    11,    12,
       3,     4,     5,     6,     3,     4,     5,     6,     3,     4,
       5,     6
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    18,    20,    21,    22,    23,    24,    74,    75,    76,
      77,    78,    79,    81,    84,    86,    88,    77,    29,    89,
      49,     6,    26,    80,    25,     0,    75,    70,    46,    47,
      48,    70,    17,    66,     3,     4,     5,     6,     9,    15,
      18,    90,    91,    93,    94,    95,    96,    97,    98,     6,
      56,     6,    18,    42,    87,    77,    77,    18,    18,    72,
       6,    93,    93,    26,    71,    99,     7,     8,     9,    10,
      11,    12,    13,    14,    16,    27,    50,    77,     6,    82,
      83,    95,    28,    75,    77,    92,    93,    92,     9,    17,
       6,    18,   100,   101,   102,   104,    91,    28,   108,    93,
      93,    93,    93,    93,    93,    93,    93,    93,     6,    18,
      71,   108,    16,    18,    34,    39,    93,   109,    17,    17,
      71,    17,     6,    27,   107,    18,    77,   101,   104,    71,
      59,    60,    61,    62,    64,    65,    68,   105,   107,   109,
      52,   112,     6,    85,    94,    83,    18,    93,   102,    93,
     109,   109,    18,    16,    34,   111,    31,    32,    33,    37,
      93,     6,    77,    17,    17,   101,   101,    59,   105,    63,
      63,    59,    63,    59,    54,    51,   113,    17,    71,    18,
      17,    77,    18,    42,    43,    45,   110,    35,   109,   109,
     109,    93,    17,   107,    66,    67,   106,   101,    59,   101,
     101,    94,    95,   116,     6,    98,    53,   114,     6,    94,
      17,    77,    18,    18,    31,   109,    18,   101,   106,    71,
      16,    54,    55,   115,    17,    77,    77,    93,     6,   103,
      94,    95,    93,   116,    94,    17,    17,    17,    71,     6
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    73,    74,    74,    75,    75,    76,    76,    76,    77,
      77,    77,    77,    78,    79,    80,    80,    81,    82,    82,
      83,    83,    84,    84,    85,    85,    85,    85,    86,    86,
      86,    87,    87,    88,    89,    89,    89,    90,    90,    91,
      91,    91,    91,    92,    92,    93,    93,    93,    93,    93,
      93,    94,    94,    94,    95,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    97,    98,    99,    99,   100,   100,
     101,   101,   101,   101,   101,   101,   102,   103,   103,   104,
     104,   104,   104,   104,   104,   105,   105,   105,   105,   106,
     106,   107,   107,   108,   108,   109,   109,   109,   109,   109,
     109,   109,   109,   109,   109,   109,   110,   110,   110,   111,
     111,   112,   112,   113,   113,   114,   114,   115,   115,   116,
     116,   116,   116
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     2,     2,     1,     1,     1,     3,
       1,     1,     1,     5,     5,     0,     1,     5,     1,     3,
       3,     3,     7,     4,     1,     1,     3,     3,     3,     3,
       4,     0,     1,     9,     0,     1,     5,     1,     3,     1,
       3,     1,     3,     1,     3,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     4,     0,     2,     1,     3,
       1,     2,     1,     2,     1,     4,     3,     1,     3,     3,
       5,     4,     4,     5,     4,     2,     2,     2,     1,     4,
       2,     1,     2,     0,     2,     3,     1,     2,     3,     3,
       3,     5,     6,     5,     6,     4,     1,     1,     1,     0,
       1,     0,     3,     0,     4,     0,     3,     0,     2,     1,
       3,     1,     3
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
#line 121 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("stmtList::stmt"); 
				(yyval.list) = singleton((yyvsp[0].node));
				bisonParseResult = (Node *) (yyval.list);	 
			}
#line 1482 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 127 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("stmtlist::stmtList::stmt");
				(yyval.list) = appendToTailOfList((yyvsp[-1].list), (yyvsp[0].node));	
				bisonParseResult = (Node *) (yyval.list); 
			}
#line 1492 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 135 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::dmlStmt");
            (yyval.node) = (yyvsp[-1].node);
        }
#line 1501 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 140 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::queryStmt");
            (yyval.node) = (yyvsp[-1].node);
        }
#line 1510 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 150 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::insertQuery"); }
#line 1516 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 151 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::deleteQuery"); }
#line 1522 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 152 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::updateQuery"); }
#line 1528 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 159 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::bracketedQuery"); (yyval.node) = (yyvsp[-1].node); }
#line 1534 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 160 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::selectQuery"); }
#line 1540 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 161 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::provStmt"); }
#line 1546 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 162 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::setOperatorQuery"); }
#line 1552 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 170 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG(provStmt);
            (yyval.node) = (Node *) createProvenanceStmt((yyvsp[-1].node));
        }
#line 1561 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 181 "sql_parser.y" /* yacc.c:1646  */
    { 
             RULELOG("deleteQuery");
             (yyval.node) = (Node *) createDelete((yyvsp[-2].stringVal), (yyvsp[0].node));
         }
#line 1570 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 189 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::NULL"); (yyval.stringVal) = NULL; }
#line 1576 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 190 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::FROM"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 1582 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 205 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG(updateQuery); 
                (yyval.node) = (Node *) createUpdate((yyvsp[-3].stringVal), (yyvsp[-1].list), (yyvsp[0].node)); 
            }
#line 1591 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 213 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setExpression");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 1600 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 218 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setClause::setExpression");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 1609 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 226 "sql_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal),"=")) {
                    RULELOG("setExpression::attributeRef::expression");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 1622 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 235 "sql_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal), "=")) {
                    RULELOG("setExpression::attributeRef::queryStmt");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 1635 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 250 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertQuery::insertList"); 
            	(yyval.node) = (Node *) createInsert((yyvsp[-4].stringVal),(Node *) (yyvsp[-1].list), NULL); 
        	}
#line 1644 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 255 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("insertQuery::queryStmt");
                (yyval.node) = (Node *) createInsert((yyvsp[-1].stringVal), (yyvsp[0].node), NULL);
            }
#line 1653 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 263 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::constant");
            	(yyval.list) = singleton((yyvsp[0].node)); 
            }
#line 1662 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 268 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("insertList::IDENTIFIER");
                (yyval.list) = singleton((yyvsp[0].stringVal));
            }
#line 1671 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 273 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("insertList::insertList::::IDENTIFIER");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].stringVal));
            }
#line 1680 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 278 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::insertList::constant");
            	(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 1689 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 292 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::INTERSECT");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 1698 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 297 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::MINUS");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 1707 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 302 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::UNION");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-2].stringVal), ((yyvsp[-1].stringVal) != NULL), (yyvsp[-3].node), (yyvsp[0].node));
            }
#line 1716 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 309 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::NULL"); (yyval.stringVal) = NULL; }
#line 1722 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 310 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::ALLTRUE"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 1728 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 320 "sql_parser.y" /* yacc.c:1646  */
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
#line 1748 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 342 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalDistinct::NULL"); (yyval.node) = NULL; }
#line 1754 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 344 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT");
                (yyval.node) = (Node *) createDistinctClause(NULL);
            }
#line 1763 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 349 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT::exprList");
                (yyval.node) = (Node *) createDistinctClause((yyvsp[-1].list));
            }
#line 1772 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 361 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectItem"); (yyval.list) = singleton((yyvsp[0].node));
            }
#line 1780 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 365 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectClause::selectItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node)); 
            }
#line 1789 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 373 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression"); 
                 (yyval.node) = (Node *) createSelectItem(NULL, (yyvsp[0].node)); 
             }
#line 1798 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 378 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression::identifier"); 
                 (yyval.node) = (Node *) createSelectItem((yyvsp[0].stringVal), (yyvsp[-2].node));
             }
#line 1807 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 383 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
         		(yyval.node) = (Node *) createAttributeReference("*"); 
     		}
#line 1816 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 388 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
     			(yyval.node) = (Node *) createAttributeReference(
 						CONCAT_STRINGS((yyvsp[-2].stringVal),".*")); 
 			}
#line 1826 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 399 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("exprList::SINGLETON"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 1832 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 401 "sql_parser.y" /* yacc.c:1646  */
    {
                  RULELOG("exprList::exprList::expression");
                  (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
             }
#line 1841 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 412 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::bracked"); (yyval.node) = (yyvsp[-1].node); }
#line 1847 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 413 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::constant"); }
#line 1853 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 414 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::attributeRef"); }
#line 1859 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 415 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::binaryOperatorExpression"); }
#line 1865 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 416 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::unaryOperatorExpression"); }
#line 1871 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 417 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlFunctionCall"); }
#line 1877 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 426 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::INT"); (yyval.node) = (Node *) createConstInt((yyvsp[0].intVal)); }
#line 1883 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 427 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::FLOAT"); (yyval.node) = (Node *) createConstFloat((yyvsp[0].floatVal)); }
#line 1889 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 428 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::STRING"); (yyval.node) = (Node *) createConstString((yyvsp[0].stringVal)); }
#line 1895 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 435 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("attributeRef::IDENTIFIER"); (yyval.node) = (Node *) createAttributeReference((yyvsp[0].stringVal)); }
#line 1901 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 455 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '+' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1912 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 462 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '-' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1923 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 469 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '*' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1934 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 476 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '/' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1945 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 483 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '%' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1956 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 490 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '^' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1967 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 499 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '&' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1978 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 506 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '|' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1989 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 515 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression::comparisonOps");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2000 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 525 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("unaryOperatorExpression:: '!' ");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2010 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 537 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("sqlFunctionCall::IDENTIFIER::exprList"); 
                (yyval.node) = (Node *) createFunctionCall((yyvsp[-3].stringVal), (yyvsp[-1].list)); 
            }
#line 2019 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 549 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::NULL"); (yyval.list) = NULL; }
#line 2025 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 550 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::fromClause"); (yyval.list) = (yyvsp[0].list); }
#line 2031 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 555 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClauseItem");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2040 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 560 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClause::fromClauseItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2049 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 569 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
                (yyval.node) = (Node *) createFromTableRef(NULL, NIL, (yyvsp[0].stringVal));
            }
#line 2058 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 574 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
                (yyval.node) = (Node *) createFromTableRef((yyvsp[0].stringVal), NIL, (yyvsp[-1].stringVal));
            }
#line 2067 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 580 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                (yyval.node) = (yyvsp[0].node);
            }
#line 2076 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 585 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromSubquery *s = (FromSubquery *) (yyvsp[-1].node);
                s->from.name = (yyvsp[0].stringVal);
                (yyval.node) = (Node *) s;
            }
#line 2087 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 593 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[0].node);
        		f->name = NULL;
        		(yyval.node) = (Node *) f;
        	}
#line 2099 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 601 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[-2].node);
        		f->name = (yyvsp[-1].stringVal);
        		(yyval.node) = (Node *) f;
        	}
#line 2111 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 612 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("subQuery::queryStmt");
                (yyval.node) = (Node *) createFromSubquery(NULL, NULL, (yyvsp[-1].node));
            }
#line 2120 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 619 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = singleton((yyvsp[0].stringVal)); }
#line 2126 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 620 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].stringVal)); }
#line 2132 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 623 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2138 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 625 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURALjoinType");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), (FromItem *) (yyvsp[0].node), (yyvsp[-2].stringVal), "JOIN_COND_NATURAL", NULL);
          	}
#line 2147 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 630 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURAL");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), (FromItem *) (yyvsp[-1].stringVal), (yyvsp[-1].stringVal), "JOIN_COND_NATURAL", NULL);
          	}
#line 2156 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 635 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::CROSS JOIN");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), (FromItem *) (yyvsp[-1].stringVal), "JOIN_CROSS", "JOIN_COND_ON", NULL);
          	}
#line 2165 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 640 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinType::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : "JOIN_COND_ON";
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), (FromItem *) (yyvsp[-1].node), (yyvsp[-3].stringVal), condType, (yyvsp[0].node));
          	}
#line 2175 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 646 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : "JOIN_COND_ON"; 
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), (FromItem *) (yyvsp[-1].node), "JOIN_INNER", 
                	condType, (yyvsp[0].node));
          	}
#line 2186 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 655 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.stringVal) = "JOIN_LEFT OUTER"; }
#line 2192 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 656 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2198 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 657 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2204 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 658 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.stringVal) = "JOIN_INNER"; }
#line 2210 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 662 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (Node *) (yyvsp[-1].list); }
#line 2216 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 663 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2222 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 666 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAlias::identifier"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2228 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 667 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAlias::identifier"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2234 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 674 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::NULL"); (yyval.node) = NULL; }
#line 2240 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 675 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::whereExpression"); (yyval.node) = (yyvsp[0].node); }
#line 2246 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 679 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("where::brackedWhereExpression"); (yyval.node) = (yyvsp[-1].node); }
#line 2252 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 680 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("whereExpression::expression"); (yyval.node) = (yyvsp[0].node); }
#line 2258 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 682 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::NOT");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2268 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 688 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2279 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 695 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2290 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 702 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2301 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 709 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::BETWEEN-AND");
                List *expr = singleton((yyvsp[-4].node));
                expr = appendToTailOfList(expr, (yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr);
            }
#line 2313 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 717 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::comparisonOps::nestedSubQueryOperator::Subquery");
                (yyval.node) = (Node *) createNestedSubquery((yyvsp[-3].stringVal), (yyvsp[-5].node), (yyvsp[-4].stringVal), (yyvsp[-1].node));
            }
#line 2322 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 722 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::comparisonOps::Subquery");
                List *expr = singleton((yyvsp[-4].node));
                expr = appendToTailOfList(expr, (yyvsp[-1].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr);
            }
#line 2333 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 729 "sql_parser.y" /* yacc.c:1646  */
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
#line 2350 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 742 "sql_parser.y" /* yacc.c:1646  */
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
#line 2367 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 757 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ANY"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2373 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 758 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ALL"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2379 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 759 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::SOME"); (yyval.stringVal) = "ANY"; }
#line 2385 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 763 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NULL"); (yyval.stringVal) = NULL; }
#line 2391 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 764 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NOT"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2397 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 768 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::NULL"); (yyval.list) = NULL; }
#line 2403 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 769 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::GROUPBY"); (yyval.list) = (yyvsp[0].list); }
#line 2409 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 773 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.node) = NULL; }
#line 2415 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 775 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("optionalHaving::HAVING"); 
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2426 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 784 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.list) = NULL; }
#line 2432 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 785 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy::ORDERBY"); (yyval.list) = (yyvsp[0].list); }
#line 2438 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 789 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::NULL"); (yyval.node) = NULL; }
#line 2444 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 790 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::CONSTANT"); (yyval.node) = (yyvsp[0].node);}
#line 2450 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 795 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::attributeRef");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2459 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 800 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2468 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 805 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::constant");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2477 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 810 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2486 "sql_parser.tab.c" /* yacc.c:1646  */
    break;


#line 2490 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 817 "sql_parser.y" /* yacc.c:1906  */




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

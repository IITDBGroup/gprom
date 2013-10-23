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
#line 6 "sql_parser.y" /* yacc.c:339  */

#include <stdio.h>
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

#line 84 "sql_parser.tab.c" /* yacc.c:339  */

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
#line 24 "sql_parser.y" /* yacc.c:355  */

    /* 
     * Declare some C structure those will be used as data type
     * for various tokens used in grammar rules.
     */
     Node *node;
     List *list;
     char *stringVal;
     int intVal;
     double floatVal;

#line 194 "sql_parser.tab.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SQL_PARSER_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 209 "sql_parser.tab.c" /* yacc.c:358  */

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
#define YYFINAL  24
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   302

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  72
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  39
/* YYNRULES -- Number of rules.  */
#define YYNRULES  99
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  193

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
      18,    17,     9,     7,    71,     8,     2,    10,     2,     2,
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
       0,   114,   114,   120,   132,   133,   134,   141,   142,   143,
     144,   151,   162,   171,   172,   186,   194,   199,   207,   214,
     227,   232,   240,   245,   263,   268,   273,   281,   282,   291,
     314,   315,   320,   332,   336,   344,   349,   360,   361,   373,
     374,   375,   376,   377,   378,   387,   388,   389,   396,   414,
     421,   428,   435,   442,   449,   458,   465,   474,   484,   496,
     509,   510,   514,   519,   527,   535,   540,   548,   549,   550,
     557,   558,   562,   563,   564,   570,   577,   584,   591,   599,
     604,   611,   624,   640,   641,   642,   646,   647,   651,   652,
     656,   657,   667,   668,   672,   673,   677,   682,   687,   692
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
  "FULL", "XOR", "';'", "','", "$accept", "stmt", "dmlStmt", "queryStmt",
  "provStmt", "deleteQuery", "fromString", "updateQuery", "setClause",
  "setExpression", "insertQuery", "insertList", "setOperatorQuery",
  "optionalAll", "selectQuery", "optionalDistinct", "selectClause",
  "selectItem", "exprList", "expression", "constant", "attributeRef",
  "binaryOperatorExpression", "unaryOperatorExpression", "sqlFunctionCall",
  "optionalFrom", "fromClause", "subQuery", "fromClauseItem",
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
      59,    44
};
# endif

#define YYPACT_NINF -100

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-100)))

#define YYTABLE_NINF -87

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-87)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     256,   117,   -27,   -33,    13,    14,     0,    39,     4,    29,
    -100,  -100,  -100,  -100,  -100,  -100,   -16,    17,    18,    89,
      23,  -100,    90,    82,  -100,  -100,    59,   117,   117,  -100,
    -100,    93,  -100,  -100,  -100,    95,    18,    18,   -22,  -100,
     207,  -100,  -100,  -100,  -100,  -100,   -13,   101,    80,   256,
    -100,   117,  -100,  -100,    18,    18,   259,   228,     2,    18,
      86,    18,    18,    18,    18,    18,    18,    18,    18,    18,
     122,   121,    41,  -100,   -19,  -100,   126,    88,   144,  -100,
      -4,   239,    -3,  -100,    11,   117,    74,  -100,  -100,  -100,
      88,   131,   187,   187,   176,   176,   176,  -100,   259,   249,
     281,  -100,   145,   101,  -100,   128,    88,    88,   182,   168,
     154,  -100,  -100,    18,  -100,  -100,   195,  -100,    -5,     2,
     154,   151,   157,    -2,  -100,  -100,   189,   239,   155,   193,
     196,   117,    67,  -100,   171,    88,    88,    88,    88,   239,
    -100,    11,  -100,   291,   205,   159,  -100,   145,   189,    69,
    -100,    73,   189,  -100,  -100,  -100,   204,   213,   196,   196,
     196,   250,  -100,  -100,  -100,   161,    95,   227,   200,   208,
    -100,  -100,  -100,   106,   117,   117,    88,   291,    18,   291,
     145,  -100,  -100,   109,   112,   196,  -100,  -100,   239,   161,
    -100,  -100,  -100
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    30,     0,     0,    13,     0,     0,     0,     0,
       9,     5,     6,     4,    10,     8,     0,    31,     0,     0,
       0,    14,     0,     0,     1,     2,    27,     0,     0,     3,
       7,     0,    45,    46,    47,    48,     0,     0,    60,    33,
      35,    40,    41,    42,    43,    44,     0,     0,     0,     0,
      28,     0,    24,    25,     0,     0,    58,     0,     0,     0,
      70,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    21,    48,    70,    16,     0,     0,     0,    26,
       0,    37,     0,    39,    67,     0,    61,    66,    62,    34,
       0,    88,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    36,     0,     0,    15,     0,     0,     0,     0,    73,
      12,    11,    32,     0,    59,    68,     0,    65,     0,     0,
      71,     0,    90,     0,    22,    17,     0,    18,    73,     0,
      74,     0,     0,    87,     0,     0,     0,     0,     0,    38,
      69,    67,    63,     0,     0,    92,    20,     0,     0,     0,
      72,     0,     0,    84,    83,    85,     0,     0,    75,    76,
      77,     0,    64,    98,    96,    89,     0,     0,     0,    94,
      23,    19,    82,     0,     0,     0,     0,     0,     0,     0,
       0,    29,    80,     0,     0,    78,    99,    97,    91,    93,
      95,    79,    81
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -100,   215,  -100,    -1,  -100,  -100,  -100,  -100,  -100,   169,
    -100,  -100,  -100,  -100,  -100,  -100,  -100,   214,   229,    -8,
     -99,   -41,  -100,  -100,   141,  -100,  -100,  -100,   167,   158,
     224,   -72,  -100,  -100,  -100,  -100,  -100,  -100,   123
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     7,     8,     9,    10,    11,    22,    12,    74,    75,
      13,   123,    14,    51,    15,    18,    38,    39,    80,   109,
      41,    42,    43,    44,    45,    60,    86,    87,    88,   117,
      91,   110,   156,   134,   122,   145,   169,   181,   165
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      16,    30,    17,   124,    58,     1,    76,     2,    84,    90,
      40,     6,   141,   112,   114,   146,    19,   115,   120,    20,
      85,    32,    33,    34,    35,    23,    52,    53,    56,    57,
      26,    27,    28,    36,   129,   130,    37,    71,   116,    24,
      21,    26,    27,    28,   163,    72,    81,    81,   170,    59,
      79,    40,   103,    92,    93,    94,    95,    96,    97,    98,
      99,   100,    76,   158,   159,   160,   161,   113,   113,   147,
      32,    33,    34,    35,    25,    26,    27,    28,   186,    47,
     163,   190,    36,    31,   118,   152,   171,    26,    27,    28,
     172,    32,    33,    34,    35,    46,    48,   127,   128,    29,
      49,    50,   164,    36,   185,   139,   106,    73,    77,   153,
     154,    54,   155,    55,    90,    26,    27,    28,    57,    26,
      27,    28,   107,   182,   100,   149,   191,   108,   101,   192,
     151,    32,    33,    34,    35,     1,   187,     2,   164,   102,
      57,     6,   105,    36,    57,   119,   126,    16,    32,    33,
      34,   173,    26,    27,    28,    26,    27,    28,    26,    27,
      28,   111,    61,    62,    63,    64,    65,    66,    67,    68,
     188,   132,    83,   183,   184,    61,    62,    63,    64,    65,
      66,    67,    68,   121,   132,   135,   136,   137,    66,   133,
     -86,   138,    32,    33,    34,    35,    63,    64,    65,    66,
     131,   140,   133,   -86,    36,   143,   157,   148,   144,     2,
     150,   166,   168,     6,    61,    62,    63,    64,    65,    66,
      67,    68,   174,    69,   135,   136,   137,   -87,   -87,   -87,
     138,   175,   177,   -87,    70,    61,    62,    63,    64,    65,
      66,    67,    68,   178,    69,    83,    61,    62,    63,    64,
      65,    66,    67,    68,   179,    69,    61,    62,    63,    64,
      65,    66,    67,   180,    78,    69,    61,    62,    63,    64,
      65,    66,   125,    89,     1,    69,     2,     3,     4,     5,
       6,   176,   136,   137,    82,   167,   142,   138,    61,    62,
      63,    64,    65,    66,    32,    33,    34,    73,   104,   162,
       0,     0,   189
};

static const yytype_int16 yycheck[] =
{
       1,    17,    29,   102,    26,    18,    47,    20,     6,    28,
      18,    24,    17,    17,    17,    17,    49,     6,    90,     6,
      18,     3,     4,     5,     6,    25,    27,    28,    36,    37,
      46,    47,    48,    15,   106,   107,    18,    50,    27,     0,
      26,    46,    47,    48,   143,    46,    54,    55,   147,    71,
      51,    59,    71,    61,    62,    63,    64,    65,    66,    67,
      68,    69,   103,   135,   136,   137,   138,    71,    71,    71,
       3,     4,     5,     6,    70,    46,    47,    48,   177,    56,
     179,   180,    15,    66,    85,    18,    17,    46,    47,    48,
      17,     3,     4,     5,     6,     6,     6,   105,   106,    70,
      18,    42,   143,    15,   176,   113,    18,     6,    28,    42,
      43,    18,    45,    18,    28,    46,    47,    48,   126,    46,
      47,    48,    34,    17,   132,   126,    17,    39,     6,    17,
     131,     3,     4,     5,     6,    18,   177,    20,   179,    18,
     148,    24,    16,    15,   152,    71,    18,   148,     3,     4,
       5,   152,    46,    47,    48,    46,    47,    48,    46,    47,
      48,    17,     7,     8,     9,    10,    11,    12,    13,    14,
     178,    16,    17,   174,   175,     7,     8,     9,    10,    11,
      12,    13,    14,    52,    16,    31,    32,    33,    12,    34,
      35,    37,     3,     4,     5,     6,     9,    10,    11,    12,
      18,     6,    34,    35,    15,    54,    35,    18,    51,    20,
      17,     6,    53,    24,     7,     8,     9,    10,    11,    12,
      13,    14,    18,    16,    31,    32,    33,    31,    32,    33,
      37,    18,    71,    37,    27,     7,     8,     9,    10,    11,
      12,    13,    14,    16,    16,    17,     7,     8,     9,    10,
      11,    12,    13,    14,    54,    16,     7,     8,     9,    10,
      11,    12,    13,    55,    49,    16,     7,     8,     9,    10,
      11,    12,   103,    59,    18,    16,    20,    21,    22,    23,
      24,    31,    32,    33,    55,   144,   119,    37,     7,     8,
       9,    10,    11,    12,     3,     4,     5,     6,    74,   141,
      -1,    -1,   179
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    18,    20,    21,    22,    23,    24,    73,    74,    75,
      76,    77,    79,    82,    84,    86,    75,    29,    87,    49,
       6,    26,    78,    25,     0,    70,    46,    47,    48,    70,
      17,    66,     3,     4,     5,     6,    15,    18,    88,    89,
      91,    92,    93,    94,    95,    96,     6,    56,     6,    18,
      42,    85,    75,    75,    18,    18,    91,    91,    26,    71,
      97,     7,     8,     9,    10,    11,    12,    13,    14,    16,
      27,    50,    75,     6,    80,    81,    93,    28,    73,    75,
      90,    91,    90,    17,     6,    18,    98,    99,   100,    89,
      28,   102,    91,    91,    91,    91,    91,    91,    91,    91,
      91,     6,    18,    71,   102,    16,    18,    34,    39,    91,
     103,    17,    17,    71,    17,     6,    27,   101,    75,    71,
     103,    52,   106,    83,    92,    81,    18,    91,    91,   103,
     103,    18,    16,    34,   105,    31,    32,    33,    37,    91,
       6,    17,   100,    54,    51,   107,    17,    71,    18,    75,
      17,    75,    18,    42,    43,    45,   104,    35,   103,   103,
     103,   103,   101,    92,    93,   110,     6,    96,    53,   108,
      92,    17,    17,    75,    18,    18,    31,    71,    16,    54,
      55,   109,    17,    75,    75,   103,    92,    93,    91,   110,
      92,    17,    17
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    72,    73,    73,    74,    74,    74,    75,    75,    75,
      75,    76,    77,    78,    78,    79,    80,    80,    81,    81,
      82,    82,    83,    83,    84,    84,    84,    85,    85,    86,
      87,    87,    87,    88,    88,    89,    89,    90,    90,    91,
      91,    91,    91,    91,    91,    92,    92,    92,    93,    94,
      94,    94,    94,    94,    94,    94,    94,    94,    95,    96,
      97,    97,    98,    98,    99,   100,   100,   101,   101,   101,
     102,   102,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   104,   104,   104,   105,   105,   106,   106,
     107,   107,   108,   108,   109,   109,   110,   110,   110,   110
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     2,     1,     1,     1,     3,     1,     1,
       1,     5,     5,     0,     1,     5,     1,     3,     3,     5,
       7,     4,     1,     3,     3,     3,     4,     0,     1,     9,
       0,     1,     5,     1,     3,     1,     3,     1,     3,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     4,
       0,     2,     1,     3,     4,     2,     1,     0,     1,     2,
       0,     2,     3,     1,     2,     3,     3,     3,     5,     6,
       5,     6,     4,     1,     1,     1,     0,     1,     0,     3,
       0,     4,     0,     3,     0,     2,     1,     3,     1,     3
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
#line 115 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::dmlStmt");
            (yyval.node) = (yyvsp[-1].node);
            bisonParseResult = (Node *) (yyval.node);
        }
#line 1439 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 121 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::queryStmt");
            (yyval.node) = (yyvsp[-1].node);
            bisonParseResult = (Node *) (yyval.node);
        }
#line 1449 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 132 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::insertQuery"); }
#line 1455 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 133 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::deleteQuery"); }
#line 1461 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 134 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::updateQuery"); }
#line 1467 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 141 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::bracketedQuery"); (yyval.node) = (yyvsp[-1].node); }
#line 1473 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 142 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::selectQuery"); }
#line 1479 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 143 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::provStmt"); }
#line 1485 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 144 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::setOperatorQuery"); }
#line 1491 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 152 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG(provStmt);
            (yyval.node) = (Node *) createProvenanceStmt((yyvsp[-1].node));
        }
#line 1500 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 163 "sql_parser.y" /* yacc.c:1646  */
    { 
             RULELOG("deleteQuery");
             (yyval.node) = (Node *) createDelete((yyvsp[-2].stringVal), (yyvsp[0].node));
         }
#line 1509 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 171 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::NULL"); (yyval.stringVal) = NULL; }
#line 1515 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 172 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::FROM"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 1521 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 187 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG(updateQuery); 
                (yyval.node) = (Node *) createUpdate((yyvsp[-3].stringVal), (yyvsp[-1].list), (yyvsp[0].node)); 
            }
#line 1530 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 195 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setExpression");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 1539 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 200 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setClause::setExpression");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 1548 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 208 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setExpression::attributeRef::expression");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1559 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 215 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setExpression::attributeRef::queryStmt");
                List *expr = singleton((yyvsp[-4].node));
                expr = appendToTailOfList(expr, (yyvsp[-2].stringVal));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr);
            }
#line 1570 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 228 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertQuery::insertList"); 
            	(yyval.node) = (Node *) createInsert((yyvsp[-4].stringVal),(Node *) (yyvsp[-1].list), NULL); 
        	}
#line 1579 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 233 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("insertQuery::queryStmt");
                (yyval.node) = (Node *) createInsert((yyvsp[-1].stringVal), (yyvsp[0].node), NULL);
            }
#line 1588 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 241 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::constant");
            	(yyval.list) = singleton((yyvsp[0].node)); 
            }
#line 1597 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 246 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::insertList::constant");
            	(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 1606 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 264 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::INTERSECT");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 1615 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 269 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::MINUS");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 1624 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 274 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::UNION");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-2].stringVal), ((yyvsp[-1].stringVal) != NULL), (yyvsp[-3].node), (yyvsp[0].node));
            }
#line 1633 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 281 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::NULL"); (yyval.stringVal) = NULL; }
#line 1639 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 282 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::ALLTRUE"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 1645 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 292 "sql_parser.y" /* yacc.c:1646  */
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
#line 1665 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 314 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalDistinct::NULL"); (yyval.node) = NULL; }
#line 1671 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 316 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT");
                (yyval.node) = (Node *) createDistinctClause(NULL);
            }
#line 1680 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 321 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT::exprList");
                (yyval.node) = (Node *) createDistinctClause((yyvsp[-1].list));
            }
#line 1689 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 333 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectItem"); (yyval.list) = singleton((yyvsp[0].node));
            }
#line 1697 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 337 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectClause::selectItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node)); 
            }
#line 1706 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 345 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression"); 
                 (yyval.node) = (Node *) createSelectItem(NULL, (yyvsp[0].node)); 
             }
#line 1715 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 350 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression::identifier"); 
                 (yyval.node) = (Node *) createSelectItem((yyvsp[0].stringVal), (yyvsp[-2].node));
             }
#line 1724 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 360 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("exprList::SINGLETON"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 1730 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 362 "sql_parser.y" /* yacc.c:1646  */
    {
                  RULELOG("exprList::exprList::expression");
                  (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
             }
#line 1739 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 373 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::bracked"); (yyval.node) = (yyvsp[-1].node); }
#line 1745 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 374 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::constant"); }
#line 1751 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 375 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::attributeRef"); }
#line 1757 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 376 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::binaryOperatorExpression"); }
#line 1763 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 377 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::unaryOperatorExpression"); }
#line 1769 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 378 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlFunctionCall"); }
#line 1775 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 387 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::INT"); (yyval.node) = (Node *) createConstInt((yyvsp[0].intVal)); }
#line 1781 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 388 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::FLOAT"); (yyval.node) = (Node *) createConstFloat((yyvsp[0].floatVal)); }
#line 1787 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 389 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::STRING"); (yyval.node) = (Node *) createConstString((yyvsp[0].stringVal)); }
#line 1793 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 396 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("attributeRef::IDENTIFIER"); (yyval.node) = (Node *) createAttributeReference((yyvsp[0].stringVal)); }
#line 1799 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 415 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '+' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1810 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 422 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '-' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1821 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 429 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '*' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1832 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 436 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '/' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1843 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 443 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '%' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1854 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 450 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '^' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1865 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 459 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '&' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1876 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 466 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '|' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1887 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 475 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression::comparisonOps");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1898 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 485 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("unaryOperatorExpression:: '!' ");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 1908 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 497 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("sqlFunctionCall::IDENTIFIER::exprList"); 
                (yyval.node) = (Node *) createFunctionCall((yyvsp[-3].stringVal), (yyvsp[-1].list)); 
            }
#line 1917 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 509 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::NULL"); (yyval.list) = NULL; }
#line 1923 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 510 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::fromClause"); (yyval.list) = (yyvsp[0].list); }
#line 1929 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 515 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClauseItem");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 1938 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 520 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClause::fromClauseItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 1947 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 528 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("subQuery::queryStmt");
                (yyval.node) = (Node *) createFromSubquery((yyvsp[0].stringVal), NULL, (yyvsp[-2].node));
            }
#line 1956 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 536 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
                (yyval.node) = (Node *) createFromTableRef((yyvsp[0].stringVal), NIL, (yyvsp[-1].stringVal));
            }
#line 1965 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 541 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                (yyval.node) = (yyvsp[0].node);
            }
#line 1974 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 548 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAlias::NULL"); (yyval.stringVal) = NULL; }
#line 1980 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 549 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAlias::identifier"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 1986 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 550 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAlias::identifier"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 1992 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 557 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::NULL"); (yyval.node) = NULL; }
#line 1998 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 558 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::whereExpression"); (yyval.node) = (yyvsp[0].node); }
#line 2004 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 562 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("where::brackedWhereExpression"); (yyval.node) = (yyvsp[-1].node); }
#line 2010 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 563 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("whereExpression::expression"); (yyval.node) = (yyvsp[0].node); }
#line 2016 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 565 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::NOT");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2026 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 571 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2037 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 578 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2048 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 585 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2059 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 592 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::BETWEEN-AND");
                List *expr = singleton((yyvsp[-4].node));
                expr = appendToTailOfList(expr, (yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr);
            }
#line 2071 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 600 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::comparisonOps::nestedSubQueryOperator::Subquery");
                (yyval.node) = (Node *) createNestedSubquery((yyvsp[-3].stringVal), (yyvsp[-5].node), (yyvsp[-4].stringVal), (yyvsp[-1].node));
            }
#line 2080 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 605 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::comparisonOps::Subquery");
                List *expr = singleton((yyvsp[-4].node));
                expr = appendToTailOfList(expr, (yyvsp[-1].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr);
            }
#line 2091 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 612 "sql_parser.y" /* yacc.c:1646  */
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
#line 2108 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 625 "sql_parser.y" /* yacc.c:1646  */
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
#line 2125 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 640 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ANY"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2131 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 641 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ALL"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2137 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 642 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::SOME"); (yyval.stringVal) = "ANY"; }
#line 2143 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 646 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NULL"); (yyval.stringVal) = NULL; }
#line 2149 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 647 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NOT"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2155 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 651 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::NULL"); (yyval.list) = NULL; }
#line 2161 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 652 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::GROUPBY"); (yyval.list) = (yyvsp[0].list); }
#line 2167 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 656 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.node) = NULL; }
#line 2173 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 658 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("optionalHaving::HAVING"); 
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2184 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 667 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.list) = NULL; }
#line 2190 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 668 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy::ORDERBY"); (yyval.list) = (yyvsp[0].list); }
#line 2196 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 672 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::NULL"); (yyval.node) = NULL; }
#line 2202 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 673 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::CONSTANT"); (yyval.node) = (yyvsp[0].node);}
#line 2208 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 678 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::attributeRef");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2217 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 683 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2226 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 688 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::constant");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2235 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 693 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2244 "sql_parser.tab.c" /* yacc.c:1646  */
    break;


#line 2248 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 699 "sql_parser.y" /* yacc.c:1906  */


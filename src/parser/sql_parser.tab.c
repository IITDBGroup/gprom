/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
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
#define YYBISON_VERSION "2.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 6 "sql_parser.y"

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


/* Line 268 of yacc.c  */
#line 90 "sql_parser.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


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



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 293 of yacc.c  */
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



/* Line 293 of yacc.c  */
#line 197 "sql_parser.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 209 "sql_parser.tab.c"

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
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
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
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
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
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  25
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   322

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  73
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  40
/* YYNRULES -- Number of rules.  */
#define YYNRULES  105
/* YYNRULES -- Number of states.  */
#define YYNSTATES  200

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   312

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
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
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,    11,    14,    16,    18,    20,
      24,    26,    28,    30,    36,    42,    43,    45,    51,    53,
      57,    61,    65,    73,    78,    80,    82,    86,    90,    94,
      98,   103,   104,   106,   116,   117,   119,   125,   127,   131,
     133,   137,   139,   143,   145,   149,   153,   155,   157,   159,
     161,   163,   165,   167,   169,   171,   175,   179,   183,   187,
     191,   195,   199,   203,   207,   210,   215,   216,   219,   221,
     225,   230,   233,   235,   236,   238,   241,   242,   245,   249,
     251,   254,   258,   262,   266,   272,   279,   285,   292,   297,
     299,   301,   303,   304,   306,   307,   311,   312,   317,   318,
     322,   323,   326,   328,   332,   334
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      74,     0,    -1,    75,    -1,    74,    75,    -1,    76,    70,
      -1,    77,    70,    -1,    84,    -1,    79,    -1,    81,    -1,
      18,    77,    17,    -1,    88,    -1,    78,    -1,    86,    -1,
      24,    25,    18,    75,    17,    -1,    23,    80,     6,    28,
     105,    -1,    -1,    26,    -1,    22,     6,    56,    82,   104,
      -1,    83,    -1,    82,    71,    83,    -1,    95,    16,    93,
      -1,    95,    16,   101,    -1,    21,    49,     6,    50,    18,
      85,    17,    -1,    21,    49,     6,    77,    -1,    94,    -1,
       6,    -1,    85,    71,     6,    -1,    85,    71,    94,    -1,
      77,    47,    77,    -1,    77,    48,    77,    -1,    77,    46,
      87,    77,    -1,    -1,    42,    -1,    20,    89,    90,    99,
     104,   108,   109,   110,   111,    -1,    -1,    29,    -1,    29,
      66,    18,    92,    17,    -1,    91,    -1,    90,    71,    91,
      -1,    93,    -1,    93,    27,     6,    -1,     9,    -1,     6,
      72,     9,    -1,    93,    -1,    92,    71,    93,    -1,    18,
      93,    17,    -1,    94,    -1,    95,    -1,    96,    -1,    97,
      -1,    98,    -1,     3,    -1,     4,    -1,     5,    -1,     6,
      -1,    93,     7,    93,    -1,    93,     8,    93,    -1,    93,
       9,    93,    -1,    93,    10,    93,    -1,    93,    11,    93,
      -1,    93,    12,    93,    -1,    93,    13,    93,    -1,    93,
      14,    93,    -1,    93,    16,    93,    -1,    15,    93,    -1,
       6,    18,    92,    17,    -1,    -1,    26,   100,    -1,   102,
      -1,   100,    71,   102,    -1,    18,    77,    17,   103,    -1,
       6,   103,    -1,   101,    -1,    -1,     6,    -1,    27,     6,
      -1,    -1,    28,   105,    -1,    18,   105,    17,    -1,    93,
      -1,    34,   105,    -1,   105,    31,   105,    -1,   105,    32,
     105,    -1,   105,    33,   105,    -1,   105,    37,   105,    31,
     105,    -1,    93,    16,   106,    18,    77,    17,    -1,    93,
      16,    18,    77,    17,    -1,    93,   107,    35,    18,    77,
      17,    -1,    39,    18,    77,    17,    -1,    43,    -1,    42,
      -1,    45,    -1,    -1,    34,    -1,    -1,    52,    54,   112,
      -1,    -1,    51,    98,    16,    93,    -1,    -1,    53,    54,
     112,    -1,    -1,    55,    94,    -1,    95,    -1,   112,    71,
      95,    -1,    94,    -1,   112,    71,    94,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   114,   114,   120,   128,   133,   144,   145,   146,   153,
     154,   155,   156,   163,   174,   183,   184,   198,   206,   211,
     219,   228,   243,   248,   256,   261,   266,   271,   285,   290,
     295,   303,   304,   313,   336,   337,   342,   354,   358,   366,
     371,   376,   381,   393,   394,   406,   407,   408,   409,   410,
     411,   420,   421,   422,   429,   448,   455,   462,   469,   476,
     483,   492,   499,   508,   518,   530,   543,   544,   548,   553,
     561,   569,   574,   582,   583,   584,   591,   592,   596,   597,
     598,   604,   611,   618,   625,   633,   638,   645,   658,   674,
     675,   676,   680,   681,   685,   686,   690,   691,   701,   702,
     706,   707,   711,   716,   721,   726
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
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
  "sqlFunctionCall", "optionalFrom", "fromClause", "subQuery",
  "fromClauseItem", "optionalAlias", "optionalWhere", "whereExpression",
  "nestedSubQueryOperator", "optionalNot", "optionalGroupBy",
  "optionalHaving", "optionalOrderBy", "optionalLimit", "clauseList", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
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
     101,   102,   102,   103,   103,   103,   104,   104,   105,   105,
     105,   105,   105,   105,   105,   105,   105,   105,   105,   106,
     106,   106,   107,   107,   108,   108,   109,   109,   110,   110,
     111,   111,   112,   112,   112,   112
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     2,     2,     1,     1,     1,     3,
       1,     1,     1,     5,     5,     0,     1,     5,     1,     3,
       3,     3,     7,     4,     1,     1,     3,     3,     3,     3,
       4,     0,     1,     9,     0,     1,     5,     1,     3,     1,
       3,     1,     3,     1,     3,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     4,     0,     2,     1,     3,
       4,     2,     1,     0,     1,     2,     0,     2,     3,     1,
       2,     3,     3,     3,     5,     6,     5,     6,     4,     1,
       1,     1,     0,     1,     0,     3,     0,     4,     0,     3,
       0,     2,     1,     3,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    34,     0,     0,    15,     0,     0,     2,     0,
       0,    11,     7,     8,     6,    12,    10,     0,    35,     0,
       0,     0,    16,     0,     0,     1,     3,     4,    31,     0,
       0,     5,     9,     0,    51,    52,    53,    54,    41,     0,
       0,    66,    37,    39,    46,    47,    48,    49,    50,     0,
       0,     0,     0,    32,     0,    28,    29,     0,     0,     0,
      54,    64,     0,     0,     0,    76,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    23,    54,    76,
      18,     0,     0,     0,    30,     0,    43,     0,    42,    45,
      73,     0,    67,    72,    68,    38,     0,    94,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    40,     0,     0,
      17,     0,     0,     0,     0,    79,    14,    13,    36,     0,
      65,    74,     0,    71,     0,     0,    77,     0,    96,    25,
       0,    24,    19,     0,    20,    21,    79,     0,    80,     0,
       0,    93,     0,     0,     0,     0,     0,    44,    75,    73,
      69,     0,     0,    98,    22,     0,     0,    78,     0,     0,
      90,    89,    91,     0,     0,    81,    82,    83,     0,    70,
     104,   102,    95,     0,     0,     0,   100,    26,    27,    88,
       0,     0,     0,     0,     0,     0,     0,     0,    33,    86,
       0,     0,    84,   105,   103,    97,    99,   101,    85,    87
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     7,     8,     9,    10,    11,    12,    23,    13,    79,
      80,    14,   130,    15,    54,    16,    19,    41,    42,    85,
     115,    44,    45,    46,    47,    48,    65,    92,    93,    94,
     123,    97,   116,   163,   142,   128,   153,   176,   188,   172
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -107
static const yytype_int16 yypact[] =
{
     266,   126,    -3,   -18,    31,    15,    29,   182,  -107,     5,
     -23,  -107,  -107,  -107,  -107,  -107,  -107,   -12,    10,   236,
      73,    32,  -107,    89,    78,  -107,  -107,  -107,    61,   126,
     126,  -107,  -107,    95,  -107,  -107,  -107,   -17,  -107,   136,
     136,   -19,  -107,   222,  -107,  -107,  -107,  -107,  -107,   -10,
     117,    59,   266,  -107,   126,  -107,  -107,   136,   136,   116,
     113,   269,   248,    12,   236,   106,   136,   136,   136,   136,
     136,   136,   136,   136,   136,   130,   119,    45,  -107,   -25,
    -107,   131,    96,   139,  -107,   -13,   259,     2,  -107,  -107,
       0,   126,    77,  -107,  -107,  -107,    96,   101,   295,   295,
     145,   145,   145,  -107,   269,   183,   291,  -107,   305,   117,
    -107,   208,    96,    96,   149,   163,   -16,  -107,  -107,   136,
    -107,  -107,   212,  -107,    -4,    12,   -16,   134,   138,  -107,
       3,  -107,  -107,   204,   259,  -107,   152,   184,   215,   126,
      79,  -107,   185,    96,    96,    96,    96,   259,  -107,     0,
    -107,   309,   219,   174,  -107,   313,   204,  -107,    60,   204,
    -107,  -107,  -107,   225,   226,   215,   215,   215,   260,  -107,
    -107,  -107,   166,   113,   234,   199,   227,  -107,  -107,  -107,
      69,   126,   126,    96,   309,   136,   309,   180,  -107,  -107,
      72,    81,   215,  -107,  -107,   259,   166,  -107,  -107,  -107
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -107,  -107,     4,  -107,    -1,  -107,  -107,  -107,  -107,  -107,
     154,  -107,  -107,  -107,  -107,  -107,  -107,  -107,   210,   237,
      -7,  -106,   -41,  -107,  -107,   142,  -107,  -107,   172,   171,
     173,   241,   -74,  -107,  -107,  -107,  -107,  -107,  -107,   135
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -93
static const yytype_int16 yytable[] =
{
      17,    58,   131,    96,   118,    32,   121,    63,     1,    81,
       2,    26,    43,   149,     6,   143,   144,   145,    90,   120,
     154,   146,   126,    28,    29,    30,    18,   122,    55,    56,
      91,    20,    61,    62,    28,    29,    30,    21,   137,   138,
      76,    22,    28,    29,    30,   170,   109,    31,    77,   178,
      86,    86,    64,    84,    24,    59,    83,    43,   119,    98,
      99,   100,   101,   102,   103,   104,   105,   106,    81,   165,
     166,   167,   168,   119,   155,    27,    33,   179,   193,    49,
     170,   197,    34,    35,    36,    60,   189,    82,    50,   198,
     124,    28,    29,    30,    39,    51,    52,   159,   199,    34,
      35,    36,    60,    53,   134,   136,    28,    29,    30,   192,
     171,    39,   147,    57,   112,    28,    29,    30,    28,    29,
      30,   160,   161,    78,   162,    88,    62,    28,    29,    30,
     113,    58,   124,   106,    96,   114,   107,   108,   158,    34,
      35,    36,    60,   194,     1,   171,     2,   111,   125,    62,
       6,    39,    62,   127,    40,    17,   117,    71,   180,    66,
      67,    68,    69,    70,    71,    72,    73,   139,   140,    89,
      66,    67,    68,    69,    70,    71,    72,    73,   195,   140,
     190,   191,    25,    34,    35,    36,   141,   -92,   151,   152,
      66,    67,    68,    69,    70,    71,    72,   141,   -92,    74,
       1,   157,     2,     3,     4,     5,     6,    34,    35,    36,
      60,    34,    35,    36,    60,   143,   144,   145,   148,    39,
     164,   146,   156,    39,     2,   173,   133,   175,     6,    66,
      67,    68,    69,    70,    71,    72,    73,   184,    74,    34,
      35,    36,    37,   181,   182,    38,   -93,   -93,   -93,    75,
     185,    39,   -93,   186,    40,    66,    67,    68,    69,    70,
      71,    72,    73,   132,    74,    89,    66,    67,    68,    69,
      70,    71,    72,    73,    95,    74,    66,    67,    68,    69,
      70,    71,   187,   135,     1,    74,     2,     3,     4,     5,
       6,   183,   144,   145,   174,    87,   150,   146,    66,    67,
      68,    69,    70,    71,    68,    69,    70,    71,    34,    35,
      36,   129,    34,    35,    36,    78,    34,    35,    36,   177,
     110,   196,   169
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-107))

#define yytable_value_is_error(yytable_value) \
  ((yytable_value) == (-93))

static const yytype_uint8 yycheck[] =
{
       1,    18,   108,    28,    17,    17,     6,    26,    18,    50,
      20,     7,    19,    17,    24,    31,    32,    33,     6,    17,
      17,    37,    96,    46,    47,    48,    29,    27,    29,    30,
      18,    49,    39,    40,    46,    47,    48,     6,   112,   113,
      50,    26,    46,    47,    48,   151,    71,    70,    49,   155,
      57,    58,    71,    54,    25,    72,    52,    64,    71,    66,
      67,    68,    69,    70,    71,    72,    73,    74,   109,   143,
     144,   145,   146,    71,    71,    70,    66,    17,   184,     6,
     186,   187,     3,     4,     5,     6,    17,    28,    56,    17,
      91,    46,    47,    48,    15,     6,    18,    18,    17,     3,
       4,     5,     6,    42,   111,   112,    46,    47,    48,   183,
     151,    15,   119,    18,    18,    46,    47,    48,    46,    47,
      48,    42,    43,     6,    45,     9,   133,    46,    47,    48,
      34,    18,   133,   140,    28,    39,     6,    18,   139,     3,
       4,     5,     6,   184,    18,   186,    20,    16,    71,   156,
      24,    15,   159,    52,    18,   156,    17,    12,   159,     7,
       8,     9,    10,    11,    12,    13,    14,    18,    16,    17,
       7,     8,     9,    10,    11,    12,    13,    14,   185,    16,
     181,   182,     0,     3,     4,     5,    34,    35,    54,    51,
       7,     8,     9,    10,    11,    12,    13,    34,    35,    16,
      18,    17,    20,    21,    22,    23,    24,     3,     4,     5,
       6,     3,     4,     5,     6,    31,    32,    33,     6,    15,
      35,    37,    18,    15,    20,     6,    18,    53,    24,     7,
       8,     9,    10,    11,    12,    13,    14,    71,    16,     3,
       4,     5,     6,    18,    18,     9,    31,    32,    33,    27,
      16,    15,    37,    54,    18,     7,     8,     9,    10,    11,
      12,    13,    14,   109,    16,    17,     7,     8,     9,    10,
      11,    12,    13,    14,    64,    16,     7,     8,     9,    10,
      11,    12,    55,   111,    18,    16,    20,    21,    22,    23,
      24,    31,    32,    33,   152,    58,   125,    37,     7,     8,
       9,    10,    11,    12,     9,    10,    11,    12,     3,     4,
       5,     6,     3,     4,     5,     6,     3,     4,     5,     6,
      79,   186,   149
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
       6,    18,   100,   101,   102,    91,    28,   104,    93,    93,
      93,    93,    93,    93,    93,    93,    93,     6,    18,    71,
     104,    16,    18,    34,    39,    93,   105,    17,    17,    71,
      17,     6,    27,   103,    77,    71,   105,    52,   108,     6,
      85,    94,    83,    18,    93,   101,    93,   105,   105,    18,
      16,    34,   107,    31,    32,    33,    37,    93,     6,    17,
     102,    54,    51,   109,    17,    71,    18,    17,    77,    18,
      42,    43,    45,   106,    35,   105,   105,   105,   105,   103,
      94,    95,   112,     6,    98,    53,   110,     6,    94,    17,
      77,    18,    18,    31,    71,    16,    54,    55,   111,    17,
      77,    77,   105,    94,    95,    93,   112,    94,    17,    17
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

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
#ifndef	YYINITDEPTH
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
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
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
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
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
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

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

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

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
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
  int yytoken;
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

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

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
      yychar = YYLEX;
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
  *++yyvsp = yylval;

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
     `$$ = $1'.

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

/* Line 1806 of yacc.c  */
#line 115 "sql_parser.y"
    { 
				RULELOG("stmtList::stmt"); 
				(yyval.list) = singleton((yyvsp[(1) - (1)].node));
				bisonParseResult = (Node *) (yyval.list);	 
			}
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 121 "sql_parser.y"
    {
				RULELOG("stmtlist::stmtList::stmt");
				(yyval.list) = appendToTailOfList((yyvsp[(1) - (2)].list), (yyvsp[(2) - (2)].node));	
				bisonParseResult = (Node *) (yyval.list); 
			}
    break;

  case 4:

/* Line 1806 of yacc.c  */
#line 129 "sql_parser.y"
    {
            RULELOG("stmt::dmlStmt");
            (yyval.node) = (yyvsp[(1) - (2)].node);
        }
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 134 "sql_parser.y"
    {
            RULELOG("stmt::queryStmt");
            (yyval.node) = (yyvsp[(1) - (2)].node);
        }
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 144 "sql_parser.y"
    { RULELOG("dmlStmt::insertQuery"); }
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 145 "sql_parser.y"
    { RULELOG("dmlStmt::deleteQuery"); }
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 146 "sql_parser.y"
    { RULELOG("dmlStmt::updateQuery"); }
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 153 "sql_parser.y"
    { RULELOG("queryStmt::bracketedQuery"); (yyval.node) = (yyvsp[(2) - (3)].node); }
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 154 "sql_parser.y"
    { RULELOG("queryStmt::selectQuery"); }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 155 "sql_parser.y"
    { RULELOG("queryStmt::provStmt"); }
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 156 "sql_parser.y"
    { RULELOG("queryStmt::setOperatorQuery"); }
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 164 "sql_parser.y"
    {
            RULELOG(provStmt);
            (yyval.node) = (Node *) createProvenanceStmt((yyvsp[(4) - (5)].node));
        }
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 175 "sql_parser.y"
    { 
             RULELOG("deleteQuery");
             (yyval.node) = (Node *) createDelete((yyvsp[(3) - (5)].stringVal), (yyvsp[(5) - (5)].node));
         }
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 183 "sql_parser.y"
    { RULELOG("fromString::NULL"); (yyval.stringVal) = NULL; }
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 184 "sql_parser.y"
    { RULELOG("fromString::FROM"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 199 "sql_parser.y"
    { 
                RULELOG(updateQuery); 
                (yyval.node) = (Node *) createUpdate((yyvsp[(2) - (5)].stringVal), (yyvsp[(4) - (5)].list), (yyvsp[(5) - (5)].node)); 
            }
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 207 "sql_parser.y"
    {
                RULELOG("setClause::setExpression");
                (yyval.list) = singleton((yyvsp[(1) - (1)].node));
            }
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 212 "sql_parser.y"
    {
                RULELOG("setClause::setClause::setExpression");
                (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
            }
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 220 "sql_parser.y"
    {
                if (!strcmp((yyvsp[(2) - (3)].stringVal),"=")) {
                    RULELOG("setExpression::attributeRef::expression");
                    List *expr = singleton((yyvsp[(1) - (3)].node));
                    expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
                }
            }
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 229 "sql_parser.y"
    {
                if (!strcmp((yyvsp[(2) - (3)].stringVal), "=")) {
                    RULELOG("setExpression::attributeRef::queryStmt");
                    List *expr = singleton((yyvsp[(1) - (3)].node));
                    expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
                }
            }
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 244 "sql_parser.y"
    { 
            	RULELOG("insertQuery::insertList"); 
            	(yyval.node) = (Node *) createInsert((yyvsp[(3) - (7)].stringVal),(Node *) (yyvsp[(6) - (7)].list), NULL); 
        	}
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 249 "sql_parser.y"
    { 
                RULELOG("insertQuery::queryStmt");
                (yyval.node) = (Node *) createInsert((yyvsp[(3) - (4)].stringVal), (yyvsp[(4) - (4)].node), NULL);
            }
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 257 "sql_parser.y"
    { 
            	RULELOG("insertList::constant");
            	(yyval.list) = singleton((yyvsp[(1) - (1)].node)); 
            }
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 262 "sql_parser.y"
    {
                RULELOG("insertList::IDENTIFIER");
                (yyval.list) = singleton((yyvsp[(1) - (1)].stringVal));
            }
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 267 "sql_parser.y"
    { 
                RULELOG("insertList::insertList::::IDENTIFIER");
                (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].stringVal));
            }
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 272 "sql_parser.y"
    { 
            	RULELOG("insertList::insertList::constant");
            	(yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
            }
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 286 "sql_parser.y"
    {
                RULELOG("setOperatorQuery::INTERSECT");
                (yyval.node) = (Node *) createSetQuery((yyvsp[(2) - (3)].stringVal), FALSE, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
            }
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 291 "sql_parser.y"
    {
                RULELOG("setOperatorQuery::MINUS");
                (yyval.node) = (Node *) createSetQuery((yyvsp[(2) - (3)].stringVal), FALSE, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
            }
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 296 "sql_parser.y"
    {
                RULELOG("setOperatorQuery::UNION");
                (yyval.node) = (Node *) createSetQuery((yyvsp[(2) - (4)].stringVal), ((yyvsp[(3) - (4)].stringVal) != NULL), (yyvsp[(1) - (4)].node), (yyvsp[(4) - (4)].node));
            }
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 303 "sql_parser.y"
    { RULELOG("optionalAll::NULL"); (yyval.stringVal) = NULL; }
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 304 "sql_parser.y"
    { RULELOG("optionalAll::ALLTRUE"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 314 "sql_parser.y"
    {
                RULELOG(selectQuery);
                QueryBlock *q =  createQueryBlock();
                
                q->distinct = (yyvsp[(2) - (9)].node);
                q->selectClause = (yyvsp[(3) - (9)].list);
                q->fromClause = (yyvsp[(4) - (9)].list);
                q->whereClause = (yyvsp[(5) - (9)].node);
                q->groupByClause = (yyvsp[(6) - (9)].list);
                q->havingClause = (yyvsp[(7) - (9)].node);
                q->orderByClause = (yyvsp[(8) - (9)].list);
                q->limitClause = (yyvsp[(9) - (9)].node);
                
                (yyval.node) = (Node *) q; 
            }
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 336 "sql_parser.y"
    { RULELOG("optionalDistinct::NULL"); (yyval.node) = NULL; }
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 338 "sql_parser.y"
    {
                RULELOG("optionalDistinct::DISTINCT");
                (yyval.node) = (Node *) createDistinctClause(NULL);
            }
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 343 "sql_parser.y"
    {
                RULELOG("optionalDistinct::DISTINCT::exprList");
                (yyval.node) = (Node *) createDistinctClause((yyvsp[(4) - (5)].list));
            }
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 355 "sql_parser.y"
    {
                RULELOG("selectClause::selectItem"); (yyval.list) = singleton((yyvsp[(1) - (1)].node));
            }
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 359 "sql_parser.y"
    {
                RULELOG("selectClause::selectClause::selectItem");
                (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node)); 
            }
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 367 "sql_parser.y"
    {
                 RULELOG("selectItem::expression"); 
                 (yyval.node) = (Node *) createSelectItem(NULL, (yyvsp[(1) - (1)].node)); 
             }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 372 "sql_parser.y"
    {
                 RULELOG("selectItem::expression::identifier"); 
                 (yyval.node) = (Node *) createSelectItem((yyvsp[(3) - (3)].stringVal), (yyvsp[(1) - (3)].node));
             }
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 377 "sql_parser.y"
    { 
         		RULELOG("selectItem::*"); 
         		(yyval.node) = (Node *) createAttributeReference("*"); 
     		}
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 382 "sql_parser.y"
    { 
         		RULELOG("selectItem::*"); 
     			(yyval.node) = (Node *) createAttributeReference(
 						CONCAT_STRINGS((yyvsp[(1) - (3)].stringVal),".*")); 
 			}
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 393 "sql_parser.y"
    { RULELOG("exprList::SINGLETON"); (yyval.list) = singleton((yyvsp[(1) - (1)].node)); }
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 395 "sql_parser.y"
    {
                  RULELOG("exprList::exprList::expression");
                  (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
             }
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 406 "sql_parser.y"
    { RULELOG("expression::bracked"); (yyval.node) = (yyvsp[(2) - (3)].node); }
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 407 "sql_parser.y"
    { RULELOG("expression::constant"); }
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 408 "sql_parser.y"
    { RULELOG("expression::attributeRef"); }
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 409 "sql_parser.y"
    { RULELOG("expression::binaryOperatorExpression"); }
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 410 "sql_parser.y"
    { RULELOG("expression::unaryOperatorExpression"); }
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 411 "sql_parser.y"
    { RULELOG("expression::sqlFunctionCall"); }
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 420 "sql_parser.y"
    { RULELOG("constant::INT"); (yyval.node) = (Node *) createConstInt((yyvsp[(1) - (1)].intVal)); }
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 421 "sql_parser.y"
    { RULELOG("constant::FLOAT"); (yyval.node) = (Node *) createConstFloat((yyvsp[(1) - (1)].floatVal)); }
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 422 "sql_parser.y"
    { RULELOG("constant::STRING"); (yyval.node) = (Node *) createConstString((yyvsp[(1) - (1)].stringVal)); }
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 429 "sql_parser.y"
    { RULELOG("attributeRef::IDENTIFIER"); (yyval.node) = (Node *) createAttributeReference((yyvsp[(1) - (1)].stringVal)); }
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 449 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '+' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 456 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '-' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 463 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '*' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 470 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '/' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 477 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '%' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 484 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '^' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 493 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '&' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 500 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '|' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 509 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression::comparisonOps");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 519 "sql_parser.y"
    {
                RULELOG("unaryOperatorExpression:: '!' ");
                List *expr = singleton((yyvsp[(2) - (2)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(1) - (2)].stringVal), expr);
            }
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 531 "sql_parser.y"
    {
                RULELOG("sqlFunctionCall::IDENTIFIER::exprList"); 
                (yyval.node) = (Node *) createFunctionCall((yyvsp[(1) - (4)].stringVal), (yyvsp[(3) - (4)].list)); 
            }
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 543 "sql_parser.y"
    { RULELOG("optionalFrom::NULL"); (yyval.list) = NULL; }
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 544 "sql_parser.y"
    { RULELOG("optionalFrom::fromClause"); (yyval.list) = (yyvsp[(2) - (2)].list); }
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 549 "sql_parser.y"
    {
                RULELOG("fromClause::fromClauseItem");
                (yyval.list) = singleton((yyvsp[(1) - (1)].node));
            }
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 554 "sql_parser.y"
    {
                RULELOG("fromClause::fromClause::fromClauseItem");
                (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
            }
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 562 "sql_parser.y"
    {
                RULELOG("subQuery::queryStmt");
                (yyval.node) = (Node *) createFromSubquery((yyvsp[(4) - (4)].stringVal), NULL, (yyvsp[(2) - (4)].node));
            }
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 570 "sql_parser.y"
    {
                RULELOG("fromClauseItem");
                (yyval.node) = (Node *) createFromTableRef((yyvsp[(2) - (2)].stringVal), NIL, (yyvsp[(1) - (2)].stringVal));
            }
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 575 "sql_parser.y"
    {
                RULELOG("fromClauseItem::subQuery");
                (yyval.node) = (yyvsp[(1) - (1)].node);
            }
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 582 "sql_parser.y"
    { RULELOG("optionalAlias::NULL"); (yyval.stringVal) = NULL; }
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 583 "sql_parser.y"
    { RULELOG("optionalAlias::identifier"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 584 "sql_parser.y"
    { RULELOG("optionalAlias::identifier"); (yyval.stringVal) = (yyvsp[(2) - (2)].stringVal); }
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 591 "sql_parser.y"
    { RULELOG("optionalWhere::NULL"); (yyval.node) = NULL; }
    break;

  case 77:

/* Line 1806 of yacc.c  */
#line 592 "sql_parser.y"
    { RULELOG("optionalWhere::whereExpression"); (yyval.node) = (yyvsp[(2) - (2)].node); }
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 596 "sql_parser.y"
    { RULELOG("where::brackedWhereExpression"); (yyval.node) = (yyvsp[(2) - (3)].node); }
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 597 "sql_parser.y"
    { RULELOG("whereExpression::expression"); (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 599 "sql_parser.y"
    {
                RULELOG("whereExpression::NOT");
                List *expr = singleton((yyvsp[(2) - (2)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(1) - (2)].stringVal), expr);
            }
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 605 "sql_parser.y"
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 612 "sql_parser.y"
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 619 "sql_parser.y"
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 626 "sql_parser.y"
    {
                RULELOG("whereExpression::BETWEEN-AND");
                List *expr = singleton((yyvsp[(1) - (5)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (5)].node));
                expr = appendToTailOfList(expr, (yyvsp[(5) - (5)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (5)].stringVal), expr);
            }
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 634 "sql_parser.y"
    {
                RULELOG("whereExpression::comparisonOps::nestedSubQueryOperator::Subquery");
                (yyval.node) = (Node *) createNestedSubquery((yyvsp[(3) - (6)].stringVal), (yyvsp[(1) - (6)].node), (yyvsp[(2) - (6)].stringVal), (yyvsp[(5) - (6)].node));
            }
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 639 "sql_parser.y"
    {
                RULELOG("whereExpression::comparisonOps::Subquery");
                List *expr = singleton((yyvsp[(1) - (5)].node));
                expr = appendToTailOfList(expr, (yyvsp[(4) - (5)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (5)].stringVal), expr);
            }
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 646 "sql_parser.y"
    {
                if ((yyvsp[(2) - (6)].stringVal) == NULL)
                {
                    RULELOG("whereExpression::IN");
                    (yyval.node) = (Node *) createNestedSubquery("ANY", (yyvsp[(1) - (6)].node), "=", (yyvsp[(5) - (6)].node));
                }
                else
                {
                    RULELOG("whereExpression::NOT::IN");
                    (yyval.node) = (Node *) createNestedSubquery("ALL",(yyvsp[(1) - (6)].node), "<>", (yyvsp[(5) - (6)].node));
                }
            }
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 659 "sql_parser.y"
    {
                /* if ($1 == NULL)
                { */
                    RULELOG("whereExpression::EXISTS");
                    (yyval.node) = (Node *) createNestedSubquery((yyvsp[(1) - (4)].stringVal), NULL, NULL, (yyvsp[(3) - (4)].node));
               /*  }
                else
                {
                    RULELOG("whereExpression::EXISTS::NOT");
                    $$ = (Node *) createNestedSubquery($2, NULL, "<>", $4);
                } */
            }
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 674 "sql_parser.y"
    { RULELOG("nestedSubQueryOperator::ANY"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 675 "sql_parser.y"
    { RULELOG("nestedSubQueryOperator::ALL"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 676 "sql_parser.y"
    { RULELOG("nestedSubQueryOperator::SOME"); (yyval.stringVal) = "ANY"; }
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 680 "sql_parser.y"
    { RULELOG("optionalNot::NULL"); (yyval.stringVal) = NULL; }
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 681 "sql_parser.y"
    { RULELOG("optionalNot::NOT"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 685 "sql_parser.y"
    { RULELOG("optionalGroupBy::NULL"); (yyval.list) = NULL; }
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 686 "sql_parser.y"
    { RULELOG("optionalGroupBy::GROUPBY"); (yyval.list) = (yyvsp[(3) - (3)].list); }
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 690 "sql_parser.y"
    { RULELOG("optionalOrderBy:::NULL"); (yyval.node) = NULL; }
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 692 "sql_parser.y"
    { 
                RULELOG("optionalHaving::HAVING"); 
                List *expr = singleton((yyvsp[(2) - (4)].node));
                expr = appendToTailOfList(expr, (yyvsp[(4) - (4)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(3) - (4)].stringVal), expr);
            }
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 701 "sql_parser.y"
    { RULELOG("optionalOrderBy:::NULL"); (yyval.list) = NULL; }
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 702 "sql_parser.y"
    { RULELOG("optionalOrderBy::ORDERBY"); (yyval.list) = (yyvsp[(3) - (3)].list); }
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 706 "sql_parser.y"
    { RULELOG("optionalLimit::NULL"); (yyval.node) = NULL; }
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 707 "sql_parser.y"
    { RULELOG("optionalLimit::CONSTANT"); (yyval.node) = (yyvsp[(2) - (2)].node);}
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 712 "sql_parser.y"
    {
                RULELOG("clauseList::attributeRef");
                (yyval.list) = singleton((yyvsp[(1) - (1)].node));
            }
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 717 "sql_parser.y"
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
            }
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 722 "sql_parser.y"
    {
                RULELOG("clauseList::constant");
                (yyval.list) = singleton((yyvsp[(1) - (1)].node));
            }
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 727 "sql_parser.y"
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
            }
    break;



/* Line 1806 of yacc.c  */
#line 2626 "sql_parser.tab.c"
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

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
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

  /* Do not reclaim the symbols of the rule which action triggered
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
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

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

  *++yyvsp = yylval;


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

#if !defined(yyoverflow) || YYERROR_VERBOSE
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
  /* Do not reclaim the symbols of the rule which action triggered
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
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 2067 of yacc.c  */
#line 733 "sql_parser.y"




/* Future Work 

1. Implement support for Case when statemets for all type of queries.
2. Implement support for RETURNING statement in DELETE queries.
3. Implement support for column list like (col1, col2, col3). 
   Needed in insert queries, select queries where conditions etc.
4. Implement support for Transactions.
5. Implement support for Create queries.
6. Implement support for windowing functions.
*/


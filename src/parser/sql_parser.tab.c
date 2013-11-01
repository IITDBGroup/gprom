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
#line 8 "sql_parser.y"

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
#line 89 "sql_parser.tab.c"

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



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 293 of yacc.c  */
#line 25 "sql_parser.y"

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
#line 199 "sql_parser.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 211 "sql_parser.tab.c"

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
#define YYFINAL  29
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   395

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  76
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  45
/* YYNRULES -- Number of rules.  */
#define YYNRULES  126
/* YYNRULES -- Number of states.  */
#define YYNSTATES  245

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   315

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
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
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,    11,    14,    17,    19,    21,
      23,    27,    29,    31,    33,    35,    37,    39,    45,    51,
      52,    54,    60,    62,    66,    70,    74,    82,    87,    89,
      91,    95,    99,   103,   107,   112,   113,   115,   125,   126,
     128,   134,   136,   140,   142,   146,   148,   152,   154,   158,
     162,   164,   166,   168,   170,   172,   174,   176,   178,   180,
     184,   188,   192,   196,   200,   204,   208,   212,   216,   219,
     224,   225,   228,   230,   234,   236,   239,   241,   244,   246,
     251,   255,   257,   261,   265,   271,   276,   281,   287,   292,
     295,   298,   301,   303,   308,   311,   313,   316,   317,   320,
     324,   326,   329,   333,   337,   341,   347,   354,   360,   367,
     372,   374,   376,   378,   379,   381,   382,   386,   387,   392,
     393,   397,   398,   401,   403,   407,   409
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      77,     0,    -1,    78,    -1,    77,    78,    -1,    79,    73,
      -1,    80,    73,    -1,    81,    73,    -1,    88,    -1,    83,
      -1,    85,    -1,    18,    80,    17,    -1,    92,    -1,    82,
      -1,    90,    -1,    58,    -1,    59,    -1,    60,    -1,    24,
      25,    18,    78,    17,    -1,    23,    84,     6,    28,   113,
      -1,    -1,    26,    -1,    22,     6,    56,    86,   112,    -1,
      87,    -1,    86,    74,    87,    -1,    99,    16,    97,    -1,
      99,    16,   106,    -1,    21,    49,     6,    50,    18,    89,
      17,    -1,    21,    49,     6,    80,    -1,    98,    -1,     6,
      -1,    89,    74,     6,    -1,    89,    74,    98,    -1,    80,
      47,    80,    -1,    80,    48,    80,    -1,    80,    46,    91,
      80,    -1,    -1,    42,    -1,    20,    93,    94,   103,   112,
     116,   117,   118,   119,    -1,    -1,    29,    -1,    29,    69,
      18,    96,    17,    -1,    95,    -1,    94,    74,    95,    -1,
      97,    -1,    97,    27,     6,    -1,     9,    -1,     6,    75,
       9,    -1,    97,    -1,    96,    74,    97,    -1,    18,    97,
      17,    -1,    98,    -1,    99,    -1,   100,    -1,   101,    -1,
     102,    -1,     3,    -1,     4,    -1,     5,    -1,     6,    -1,
      97,     7,    97,    -1,    97,     8,    97,    -1,    97,     9,
      97,    -1,    97,    10,    97,    -1,    97,    11,    97,    -1,
      97,    12,    97,    -1,    97,    13,    97,    -1,    97,    14,
      97,    -1,    97,    16,    97,    -1,    15,    97,    -1,     6,
      18,    96,    17,    -1,    -1,    26,   104,    -1,   105,    -1,
     104,    74,   105,    -1,     6,    -1,     6,   111,    -1,   106,
      -1,   106,   111,    -1,   108,    -1,    18,   108,    17,   111,
      -1,    18,    80,    17,    -1,     6,    -1,   107,    74,     6,
      -1,    18,   108,    17,    -1,   105,    63,   109,    62,   105,
      -1,   105,    63,    62,   105,    -1,   105,    68,    62,   105,
      -1,   105,   109,    62,   105,   110,    -1,   105,    62,   105,
     110,    -1,    64,    66,    -1,    65,    66,    -1,    71,    66,
      -1,    67,    -1,    70,    18,   107,    17,    -1,    69,   113,
      -1,     6,    -1,    27,     6,    -1,    -1,    28,   113,    -1,
      18,   113,    17,    -1,    97,    -1,    34,   113,    -1,   113,
      31,   113,    -1,   113,    32,   113,    -1,   113,    33,   113,
      -1,   113,    37,    97,    31,    97,    -1,    97,    16,   114,
      18,    80,    17,    -1,    97,    16,    18,    80,    17,    -1,
      97,   115,    35,    18,    80,    17,    -1,    39,    18,    80,
      17,    -1,    43,    -1,    42,    -1,    45,    -1,    -1,    34,
      -1,    -1,    52,    54,   120,    -1,    -1,    51,   102,    16,
      97,    -1,    -1,    53,    54,   120,    -1,    -1,    55,    98,
      -1,    99,    -1,   120,    74,    99,    -1,    98,    -1,   120,
      74,    98,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   120,   120,   126,   134,   139,   144,   155,   156,   157,
     164,   165,   166,   167,   171,   172,   173,   180,   191,   200,
     201,   215,   223,   228,   236,   245,   260,   265,   273,   278,
     283,   288,   302,   307,   312,   320,   321,   330,   353,   354,
     359,   371,   375,   383,   388,   393,   398,   410,   411,   423,
     424,   425,   426,   427,   428,   437,   438,   439,   446,   465,
     472,   479,   486,   493,   500,   509,   516,   525,   535,   547,
     560,   561,   565,   570,   579,   584,   590,   595,   603,   611,
     622,   630,   631,   634,   635,   640,   645,   650,   656,   666,
     667,   668,   669,   673,   674,   677,   678,   685,   686,   690,
     691,   692,   698,   705,   712,   719,   727,   732,   739,   752,
     768,   769,   770,   774,   775,   779,   780,   784,   785,   795,
     796,   800,   801,   805,   810,   815,   820
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
  "optionalAlias", "optionalWhere", "whereExpression",
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
     313,   314,   315,    59,    44,    46
};
# endif

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
     109,   109,   109,   110,   110,   111,   111,   112,   112,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     114,   114,   114,   115,   115,   116,   116,   117,   117,   118,
     118,   119,   119,   120,   120,   120,   120
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
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
       3,     1,     3,     3,     5,     4,     4,     5,     4,     2,
       2,     2,     1,     4,     2,     1,     2,     0,     2,     3,
       1,     2,     3,     3,     3,     5,     6,     5,     6,     4,
       1,     1,     1,     0,     1,     0,     3,     0,     4,     0,
       3,     0,     2,     1,     3,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
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
      97,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    27,    58,    97,    22,     0,     0,     0,    34,
       0,    47,     0,    46,    49,    74,     0,    71,    72,    76,
      78,    42,     0,   115,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    44,     0,     0,    21,     0,     0,     0,
       0,   100,    18,    17,    40,     0,    69,    95,     0,    75,
       0,     0,     0,    78,     0,     0,     0,     0,     0,    92,
       0,     0,     0,    77,    98,     0,   117,    29,     0,    28,
      23,     0,    24,    25,   100,     0,   101,     0,     0,   114,
       0,     0,     0,     0,     0,    48,    96,     0,    80,    83,
      73,     0,     0,     0,    89,    90,     0,    91,     0,     0,
       0,   119,    26,     0,     0,    99,     0,     0,   111,   110,
     112,     0,     0,   102,   103,   104,     0,    80,    79,     0,
       0,    88,    85,     0,    86,     0,   125,   123,   116,     0,
       0,     0,   121,    30,    31,   109,     0,     0,     0,     0,
      94,     0,    84,    87,     0,     0,     0,     0,    37,   107,
       0,     0,   105,    81,     0,   126,   124,   118,   120,   122,
     106,   108,    93,     0,    82
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    10,    11,    12,    13,    14,    15,    16,    27,    17,
      84,    85,    18,   148,    19,    59,    20,    23,    46,    47,
      90,   121,    49,    50,    51,    52,    53,    70,    97,   132,
      99,   234,   100,   142,   201,   129,   103,   122,   191,   160,
     146,   181,   212,   228,   208
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -105
static const yytype_int16 yypact[] =
{
      83,    11,   -17,   -29,    22,    15,    14,  -105,  -105,  -105,
      76,  -105,     9,   154,    12,  -105,  -105,  -105,  -105,  -105,
    -105,     1,    23,   270,    51,    28,  -105,    87,    98,  -105,
    -105,  -105,    67,    11,    11,  -105,  -105,  -105,    99,  -105,
    -105,  -105,   -16,  -105,   237,   237,   -22,  -105,   284,  -105,
    -105,  -105,  -105,  -105,    -5,   124,   112,    83,  -105,    11,
    -105,  -105,   237,   237,   136,   145,   351,   310,     8,   270,
     137,   237,   237,   237,   237,   237,   237,   237,   237,   237,
     160,   155,    80,  -105,   -23,  -105,   153,   156,   165,  -105,
     -11,   321,     0,  -105,  -105,     3,    16,   110,   306,     3,
    -105,  -105,   156,   159,   296,   296,   184,   184,   184,  -105,
     351,   341,   371,  -105,   381,   124,  -105,   298,   156,   156,
     185,   223,   217,  -105,  -105,   237,  -105,  -105,   204,  -105,
      16,    43,   306,   197,     8,     8,   114,   172,   178,  -105,
     183,   187,   189,  -105,   217,   161,   205,  -105,     6,  -105,
    -105,   266,   321,  -105,   212,   176,   217,    11,   149,  -105,
     232,   156,   156,   156,   237,   321,  -105,    66,  -105,     3,
     306,   276,     8,   215,  -105,  -105,     8,  -105,     8,   385,
     272,   227,  -105,   389,   266,  -105,    85,   266,  -105,  -105,
    -105,   264,   269,   -12,   -12,    13,   252,    91,  -105,   156,
     271,  -105,  -105,     8,  -105,   276,  -105,  -105,   225,   145,
     293,   256,   257,  -105,  -105,  -105,   141,    11,    11,   237,
     217,   308,  -105,  -105,   385,   237,   385,   144,  -105,  -105,
     151,   158,   321,  -105,     7,  -105,  -105,   321,   225,  -105,
    -105,  -105,  -105,   309,  -105
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -105,  -105,    -3,  -105,    -1,  -105,  -105,  -105,  -105,  -105,
    -105,   210,  -105,  -105,  -105,  -105,  -105,  -105,  -105,   267,
     279,    -7,  -104,   -54,  -105,  -105,   175,  -105,  -105,   -57,
     239,  -105,   -88,   228,   167,   -96,   281,   -75,  -105,  -105,
    -105,  -105,  -105,  -105,   140
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -114
static const yytype_int16 yytable[] =
{
      21,    86,    63,   143,    68,   102,   124,    30,   133,   127,
     149,    98,    22,     1,    95,     2,    48,   126,    37,     6,
      24,   163,    95,   182,   242,   164,    96,   144,    25,     1,
     128,     2,    60,    61,   130,     6,     2,    66,    67,    28,
       6,    26,   133,   155,   156,    81,  -114,    32,    33,    34,
    -114,   115,    69,    82,    88,    91,    91,    54,    89,    64,
     168,    86,    48,   125,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   198,   125,   206,    29,   170,   171,   214,
     183,   243,    31,   197,    55,    36,   193,   194,   195,    32,
      33,    34,    38,    56,     1,   131,     2,     3,     4,     5,
       6,     1,   215,     2,     3,     4,     5,     6,   -10,    58,
     152,   154,    32,    33,    34,   202,    57,    62,   165,   204,
     235,   205,   206,   239,   220,   207,    32,    33,    34,   167,
      83,    32,    33,    34,     7,     8,     9,   -10,   -10,   -10,
      87,     7,     8,     9,    67,    93,   222,    39,    40,    41,
     131,   112,    39,    40,    41,    65,   186,   196,   229,    39,
      40,    41,    65,    63,    44,   102,   113,   187,   240,   117,
     236,    44,   207,   114,   118,   241,   172,    67,   137,   138,
      67,   139,   123,    21,   134,   141,   216,    32,    33,    34,
     119,   188,   189,   185,   190,   120,    76,    32,    33,    34,
      32,    33,    34,   157,    32,    33,    34,   161,   162,   163,
     166,   145,   232,   164,   169,   179,   230,   231,   237,    71,
      72,    73,    74,    75,    76,    77,    78,    35,   158,    94,
      71,    72,    73,    74,    75,    76,    77,    78,   174,   158,
      39,    40,    41,    65,   175,   176,   159,  -113,   161,   162,
     163,   178,    44,   177,   164,    45,   180,   159,  -113,    71,
      72,    73,    74,    75,    76,    77,    78,   192,    79,    39,
      40,    41,    65,    39,    40,    41,    42,   203,   209,    43,
     211,    44,   217,   219,   184,    44,     2,   218,    45,   221,
       6,    71,    72,    73,    74,    75,    76,    77,    78,   224,
      79,    39,    40,    41,    65,    73,    74,    75,    76,   225,
     226,    80,   227,    44,   233,   244,   151,    71,    72,    73,
      74,    75,    76,    77,    78,   150,    79,    94,    71,    72,
      73,    74,    75,    76,    77,    78,   101,    79,   135,   136,
     137,   138,    92,   139,   140,   199,   200,   141,    71,    72,
      73,    74,    75,    76,    77,   210,   153,    79,    71,    72,
      73,    74,    75,    76,   173,   116,   238,    79,   135,   136,
     137,   138,   223,   139,   140,     0,     0,   141,    71,    72,
      73,    74,    75,    76,    39,    40,    41,   147,    39,    40,
      41,    83,    39,    40,    41,   213
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-105))

#define yytable_value_is_error(yytable_value) \
  ((yytable_value) == (-114))

static const yytype_int16 yycheck[] =
{
       1,    55,    18,    99,    26,    28,    17,    10,    96,     6,
     114,    68,    29,    18,     6,    20,    23,    17,    17,    24,
      49,    33,     6,    17,    17,    37,    18,   102,     6,    18,
      27,    20,    33,    34,    18,    24,    20,    44,    45,    25,
      24,    26,   130,   118,   119,    50,    33,    46,    47,    48,
      37,    74,    74,    54,    57,    62,    63,     6,    59,    75,
      17,   115,    69,    74,    71,    72,    73,    74,    75,    76,
      77,    78,    79,   169,    74,   179,     0,   134,   135,   183,
      74,    74,    73,    17,    56,    73,   161,   162,   163,    46,
      47,    48,    69,     6,    18,    96,    20,    21,    22,    23,
      24,    18,    17,    20,    21,    22,    23,    24,    17,    42,
     117,   118,    46,    47,    48,   172,    18,    18,   125,   176,
     224,   178,   226,   227,   199,   179,    46,    47,    48,   130,
       6,    46,    47,    48,    58,    59,    60,    46,    47,    48,
      28,    58,    59,    60,   151,     9,   203,     3,     4,     5,
     151,   158,     3,     4,     5,     6,   157,   164,    17,     3,
       4,     5,     6,    18,    15,    28,     6,    18,    17,    16,
     224,    15,   226,    18,    18,    17,    62,   184,    64,    65,
     187,    67,    17,   184,    74,    71,   187,    46,    47,    48,
      34,    42,    43,    17,    45,    39,    12,    46,    47,    48,
      46,    47,    48,    18,    46,    47,    48,    31,    32,    33,
       6,    52,   219,    37,    17,    54,   217,   218,   225,     7,
       8,     9,    10,    11,    12,    13,    14,    73,    16,    17,
       7,     8,     9,    10,    11,    12,    13,    14,    66,    16,
       3,     4,     5,     6,    66,    62,    34,    35,    31,    32,
      33,    62,    15,    66,    37,    18,    51,    34,    35,     7,
       8,     9,    10,    11,    12,    13,    14,    35,    16,     3,
       4,     5,     6,     3,     4,     5,     6,    62,     6,     9,
      53,    15,    18,    31,    18,    15,    20,    18,    18,    18,
      24,     7,     8,     9,    10,    11,    12,    13,    14,    74,
      16,     3,     4,     5,     6,     9,    10,    11,    12,    16,
      54,    27,    55,    15,     6,     6,    18,     7,     8,     9,
      10,    11,    12,    13,    14,   115,    16,    17,     7,     8,
       9,    10,    11,    12,    13,    14,    69,    16,    62,    63,
      64,    65,    63,    67,    68,    69,    70,    71,     7,     8,
       9,    10,    11,    12,    13,   180,   117,    16,     7,     8,
       9,    10,    11,    12,   136,    84,   226,    16,    62,    63,
      64,    65,   205,    67,    68,    -1,    -1,    71,     7,     8,
       9,    10,    11,    12,     3,     4,     5,     6,     3,     4,
       5,     6,     3,     4,     5,     6
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
     108,    95,    28,   112,    97,    97,    97,    97,    97,    97,
      97,    97,    97,     6,    18,    74,   112,    16,    18,    34,
      39,    97,   113,    17,    17,    74,    17,     6,    27,   111,
      18,    80,   105,   108,    74,    62,    63,    64,    65,    67,
      68,    71,   109,   111,   113,    52,   116,     6,    89,    98,
      87,    18,    97,   106,    97,   113,   113,    18,    16,    34,
     115,    31,    32,    33,    37,    97,     6,    80,    17,    17,
     105,   105,    62,   109,    66,    66,    62,    66,    62,    54,
      51,   117,    17,    74,    18,    17,    80,    18,    42,    43,
      45,   114,    35,   113,   113,   113,    97,    17,   111,    69,
      70,   110,   105,    62,   105,   105,    98,    99,   120,     6,
     102,    53,   118,     6,    98,    17,    80,    18,    18,    31,
     113,    18,   105,   110,    74,    16,    54,    55,   119,    17,
      80,    80,    97,     6,   107,    98,    99,    97,   120,    98,
      17,    17,    17,    74,     6
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
#line 121 "sql_parser.y"
    { 
				RULELOG("stmtList::stmt"); 
				(yyval.list) = singleton((yyvsp[(1) - (1)].node));
				bisonParseResult = (Node *) (yyval.list);	 
			}
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 127 "sql_parser.y"
    {
				RULELOG("stmtlist::stmtList::stmt");
				(yyval.list) = appendToTailOfList((yyvsp[(1) - (2)].list), (yyvsp[(2) - (2)].node));	
				bisonParseResult = (Node *) (yyval.list); 
			}
    break;

  case 4:

/* Line 1806 of yacc.c  */
#line 135 "sql_parser.y"
    {
            RULELOG("stmt::dmlStmt");
            (yyval.node) = (yyvsp[(1) - (2)].node);
        }
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 140 "sql_parser.y"
    {
            RULELOG("stmt::queryStmt");
            (yyval.node) = (yyvsp[(1) - (2)].node);
        }
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 145 "sql_parser.y"
    {
            RULELOG("stmt::transactionIdentifier");
            (yyval.node) = (yyvsp[(1) - (2)].stringVal);
        }
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 155 "sql_parser.y"
    { RULELOG("dmlStmt::insertQuery"); }
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 156 "sql_parser.y"
    { RULELOG("dmlStmt::deleteQuery"); }
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 157 "sql_parser.y"
    { RULELOG("dmlStmt::updateQuery"); }
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 164 "sql_parser.y"
    { RULELOG("queryStmt::bracketedQuery"); (yyval.node) = (yyvsp[(2) - (3)].node); }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 165 "sql_parser.y"
    { RULELOG("queryStmt::selectQuery"); }
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 166 "sql_parser.y"
    { RULELOG("queryStmt::provStmt"); }
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 167 "sql_parser.y"
    { RULELOG("queryStmt::setOperatorQuery"); }
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 171 "sql_parser.y"
    { RULELOG("transactionIdentifier::BEGIN"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 172 "sql_parser.y"
    { RULELOG("transactionIdentifier::COMMIT"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 173 "sql_parser.y"
    { RULELOG("transactionIdentifier::ROLLBACK"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 181 "sql_parser.y"
    {
            RULELOG(provStmt);
            (yyval.node) = (Node *) createProvenanceStmt((yyvsp[(4) - (5)].node));
        }
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 192 "sql_parser.y"
    { 
             RULELOG("deleteQuery");
             (yyval.node) = (Node *) createDelete((yyvsp[(3) - (5)].stringVal), (yyvsp[(5) - (5)].node));
         }
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 200 "sql_parser.y"
    { RULELOG("fromString::NULL"); (yyval.stringVal) = NULL; }
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 201 "sql_parser.y"
    { RULELOG("fromString::FROM"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 216 "sql_parser.y"
    { 
                RULELOG(updateQuery); 
                (yyval.node) = (Node *) createUpdate((yyvsp[(2) - (5)].stringVal), (yyvsp[(4) - (5)].list), (yyvsp[(5) - (5)].node)); 
            }
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 224 "sql_parser.y"
    {
                RULELOG("setClause::setExpression");
                (yyval.list) = singleton((yyvsp[(1) - (1)].node));
            }
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 229 "sql_parser.y"
    {
                RULELOG("setClause::setClause::setExpression");
                (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
            }
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 237 "sql_parser.y"
    {
                if (!strcmp((yyvsp[(2) - (3)].stringVal),"=")) {
                    RULELOG("setExpression::attributeRef::expression");
                    List *expr = singleton((yyvsp[(1) - (3)].node));
                    expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
                }
            }
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 246 "sql_parser.y"
    {
                if (!strcmp((yyvsp[(2) - (3)].stringVal), "=")) {
                    RULELOG("setExpression::attributeRef::queryStmt");
                    List *expr = singleton((yyvsp[(1) - (3)].node));
                    expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
                }
            }
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 261 "sql_parser.y"
    { 
            	RULELOG("insertQuery::insertList"); 
            	(yyval.node) = (Node *) createInsert((yyvsp[(3) - (7)].stringVal),(Node *) (yyvsp[(6) - (7)].list), NULL); 
        	}
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 266 "sql_parser.y"
    { 
                RULELOG("insertQuery::queryStmt");
                (yyval.node) = (Node *) createInsert((yyvsp[(3) - (4)].stringVal), (yyvsp[(4) - (4)].node), NULL);
            }
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 274 "sql_parser.y"
    { 
            	RULELOG("insertList::constant");
            	(yyval.list) = singleton((yyvsp[(1) - (1)].node)); 
            }
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 279 "sql_parser.y"
    {
                RULELOG("insertList::IDENTIFIER");
                (yyval.list) = singleton((yyvsp[(1) - (1)].stringVal));
            }
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 284 "sql_parser.y"
    { 
                RULELOG("insertList::insertList::::IDENTIFIER");
                (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].stringVal));
            }
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 289 "sql_parser.y"
    { 
            	RULELOG("insertList::insertList::constant");
            	(yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
            }
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 303 "sql_parser.y"
    {
                RULELOG("setOperatorQuery::INTERSECT");
                (yyval.node) = (Node *) createSetQuery((yyvsp[(2) - (3)].stringVal), FALSE, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
            }
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 308 "sql_parser.y"
    {
                RULELOG("setOperatorQuery::MINUS");
                (yyval.node) = (Node *) createSetQuery((yyvsp[(2) - (3)].stringVal), FALSE, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
            }
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 313 "sql_parser.y"
    {
                RULELOG("setOperatorQuery::UNION");
                (yyval.node) = (Node *) createSetQuery((yyvsp[(2) - (4)].stringVal), ((yyvsp[(3) - (4)].stringVal) != NULL), (yyvsp[(1) - (4)].node), (yyvsp[(4) - (4)].node));
            }
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 320 "sql_parser.y"
    { RULELOG("optionalAll::NULL"); (yyval.stringVal) = NULL; }
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 321 "sql_parser.y"
    { RULELOG("optionalAll::ALLTRUE"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 331 "sql_parser.y"
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

  case 38:

/* Line 1806 of yacc.c  */
#line 353 "sql_parser.y"
    { RULELOG("optionalDistinct::NULL"); (yyval.node) = NULL; }
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 355 "sql_parser.y"
    {
                RULELOG("optionalDistinct::DISTINCT");
                (yyval.node) = (Node *) createDistinctClause(NULL);
            }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 360 "sql_parser.y"
    {
                RULELOG("optionalDistinct::DISTINCT::exprList");
                (yyval.node) = (Node *) createDistinctClause((yyvsp[(4) - (5)].list));
            }
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 372 "sql_parser.y"
    {
                RULELOG("selectClause::selectItem"); (yyval.list) = singleton((yyvsp[(1) - (1)].node));
            }
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 376 "sql_parser.y"
    {
                RULELOG("selectClause::selectClause::selectItem");
                (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node)); 
            }
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 384 "sql_parser.y"
    {
                 RULELOG("selectItem::expression"); 
                 (yyval.node) = (Node *) createSelectItem(NULL, (yyvsp[(1) - (1)].node)); 
             }
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 389 "sql_parser.y"
    {
                 RULELOG("selectItem::expression::identifier"); 
                 (yyval.node) = (Node *) createSelectItem((yyvsp[(3) - (3)].stringVal), (yyvsp[(1) - (3)].node));
             }
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 394 "sql_parser.y"
    { 
         		RULELOG("selectItem::*"); 
         		(yyval.node) = (Node *) createAttributeReference("*"); 
     		}
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 399 "sql_parser.y"
    { 
         		RULELOG("selectItem::*"); 
     			(yyval.node) = (Node *) createAttributeReference(
 						CONCAT_STRINGS((yyvsp[(1) - (3)].stringVal),".*")); 
 			}
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 410 "sql_parser.y"
    { RULELOG("exprList::SINGLETON"); (yyval.list) = singleton((yyvsp[(1) - (1)].node)); }
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 412 "sql_parser.y"
    {
                  RULELOG("exprList::exprList::expression");
                  (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
             }
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 423 "sql_parser.y"
    { RULELOG("expression::bracked"); (yyval.node) = (yyvsp[(2) - (3)].node); }
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 424 "sql_parser.y"
    { RULELOG("expression::constant"); }
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 425 "sql_parser.y"
    { RULELOG("expression::attributeRef"); }
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 426 "sql_parser.y"
    { RULELOG("expression::binaryOperatorExpression"); }
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 427 "sql_parser.y"
    { RULELOG("expression::unaryOperatorExpression"); }
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 428 "sql_parser.y"
    { RULELOG("expression::sqlFunctionCall"); }
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 437 "sql_parser.y"
    { RULELOG("constant::INT"); (yyval.node) = (Node *) createConstInt((yyvsp[(1) - (1)].intVal)); }
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 438 "sql_parser.y"
    { RULELOG("constant::FLOAT"); (yyval.node) = (Node *) createConstFloat((yyvsp[(1) - (1)].floatVal)); }
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 439 "sql_parser.y"
    { RULELOG("constant::STRING"); (yyval.node) = (Node *) createConstString((yyvsp[(1) - (1)].stringVal)); }
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 446 "sql_parser.y"
    { RULELOG("attributeRef::IDENTIFIER"); (yyval.node) = (Node *) createAttributeReference((yyvsp[(1) - (1)].stringVal)); }
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 466 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '+' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 473 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '-' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 480 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '*' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 487 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '/' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 494 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '%' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 501 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '^' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 510 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '&' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 517 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '|' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 526 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression::comparisonOps");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 536 "sql_parser.y"
    {
                RULELOG("unaryOperatorExpression:: '!' ");
                List *expr = singleton((yyvsp[(2) - (2)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(1) - (2)].stringVal), expr);
            }
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 548 "sql_parser.y"
    {
                RULELOG("sqlFunctionCall::IDENTIFIER::exprList"); 
                (yyval.node) = (Node *) createFunctionCall((yyvsp[(1) - (4)].stringVal), (yyvsp[(3) - (4)].list)); 
            }
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 560 "sql_parser.y"
    { RULELOG("optionalFrom::NULL"); (yyval.list) = NULL; }
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 561 "sql_parser.y"
    { RULELOG("optionalFrom::fromClause"); (yyval.list) = (yyvsp[(2) - (2)].list); }
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 566 "sql_parser.y"
    {
                RULELOG("fromClause::fromClauseItem");
                (yyval.list) = singleton((yyvsp[(1) - (1)].node));
            }
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 571 "sql_parser.y"
    {
                RULELOG("fromClause::fromClause::fromClauseItem");
                (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
            }
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 580 "sql_parser.y"
    {
                RULELOG("fromClauseItem");
                (yyval.node) = (Node *) createFromTableRef(NULL, NIL, (yyvsp[(1) - (1)].stringVal));
            }
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 585 "sql_parser.y"
    {
                RULELOG("fromClauseItem");
                (yyval.node) = (Node *) createFromTableRef((yyvsp[(2) - (2)].stringVal), NIL, (yyvsp[(1) - (2)].stringVal));
            }
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 591 "sql_parser.y"
    {
                RULELOG("fromClauseItem::subQuery");
                (yyval.node) = (yyvsp[(1) - (1)].node);
            }
    break;

  case 77:

/* Line 1806 of yacc.c  */
#line 596 "sql_parser.y"
    {
                RULELOG("fromClauseItem::subQuery");
                FromSubquery *s = (FromSubquery *) (yyvsp[(1) - (2)].node);
                s->from.name = (yyvsp[(2) - (2)].stringVal);
                (yyval.node) = (Node *) s;
            }
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 604 "sql_parser.y"
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[(1) - (1)].node);
        		f->name = NULL;
        		(yyval.node) = (Node *) f;
        	}
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 612 "sql_parser.y"
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[(2) - (4)].node);
        		f->name = (yyvsp[(3) - (4)].stringVal);
        		(yyval.node) = (Node *) f;
        	}
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 623 "sql_parser.y"
    {
                RULELOG("subQuery::queryStmt");
                (yyval.node) = (Node *) createFromSubquery(NULL, NULL, (yyvsp[(2) - (3)].node));
            }
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 630 "sql_parser.y"
    { (yyval.list) = singleton((yyvsp[(1) - (1)].stringVal)); }
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 631 "sql_parser.y"
    { (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].stringVal)); }
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 634 "sql_parser.y"
    { (yyval.node) = (yyvsp[(2) - (3)].node); }
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 636 "sql_parser.y"
    {
                RULELOG("fromJoinItem::NATURALjoinType");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[(1) - (5)].node), (FromItem *) (yyvsp[(5) - (5)].node), (yyvsp[(3) - (5)].stringVal), "JOIN_COND_NATURAL", NULL);
          	}
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 641 "sql_parser.y"
    {
                RULELOG("fromJoinItem::NATURAL");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[(1) - (4)].node), (FromItem *) (yyvsp[(3) - (4)].stringVal), (yyvsp[(3) - (4)].stringVal), "JOIN_COND_NATURAL", NULL);
          	}
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 646 "sql_parser.y"
    {
				RULELOG("fromJoinItem::CROSS JOIN");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[(1) - (4)].node), (FromItem *) (yyvsp[(3) - (4)].stringVal), "JOIN_CROSS", "JOIN_COND_ON", NULL);
          	}
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 651 "sql_parser.y"
    {
				RULELOG("fromJoinItem::JOIN::joinType::joinCond");
				char *condType = (isA((yyvsp[(5) - (5)].node),List)) ? "JOIN_COND_USING" : "JOIN_COND_ON";
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[(1) - (5)].node), (FromItem *) (yyvsp[(4) - (5)].node), (yyvsp[(2) - (5)].stringVal), condType, (yyvsp[(5) - (5)].node));
          	}
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 657 "sql_parser.y"
    {
				RULELOG("fromJoinItem::JOIN::joinCond");
				char *condType = (isA((yyvsp[(4) - (4)].node),List)) ? "JOIN_COND_USING" : "JOIN_COND_ON"; 
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[(1) - (4)].node), (FromItem *) (yyvsp[(3) - (4)].node), "JOIN_INNER", 
                	condType, (yyvsp[(4) - (4)].node));
          	}
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 666 "sql_parser.y"
    { (yyval.stringVal) = "JOIN_LEFT OUTER"; }
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 667 "sql_parser.y"
    { (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 668 "sql_parser.y"
    { (yyval.stringVal) = "JOIN_FULL_OUTER"; }
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 669 "sql_parser.y"
    { (yyval.stringVal) = "JOIN_INNER"; }
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 673 "sql_parser.y"
    { (yyval.node) = (Node *) (yyvsp[(3) - (4)].list); }
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 674 "sql_parser.y"
    { (yyval.node) = (yyvsp[(2) - (2)].node); }
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 677 "sql_parser.y"
    { RULELOG("optionalAlias::identifier"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 678 "sql_parser.y"
    { RULELOG("optionalAlias::identifier"); (yyval.stringVal) = (yyvsp[(2) - (2)].stringVal); }
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 685 "sql_parser.y"
    { RULELOG("optionalWhere::NULL"); (yyval.node) = NULL; }
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 686 "sql_parser.y"
    { RULELOG("optionalWhere::whereExpression"); (yyval.node) = (yyvsp[(2) - (2)].node); }
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 690 "sql_parser.y"
    { RULELOG("where::brackedWhereExpression"); (yyval.node) = (yyvsp[(2) - (3)].node); }
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 691 "sql_parser.y"
    { RULELOG("whereExpression::expression"); (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 693 "sql_parser.y"
    {
                RULELOG("whereExpression::NOT");
                List *expr = singleton((yyvsp[(2) - (2)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(1) - (2)].stringVal), expr);
            }
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 699 "sql_parser.y"
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 706 "sql_parser.y"
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 713 "sql_parser.y"
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 720 "sql_parser.y"
    {
                RULELOG("whereExpression::BETWEEN-AND");
                List *expr = singleton((yyvsp[(1) - (5)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (5)].node));
                expr = appendToTailOfList(expr, (yyvsp[(5) - (5)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (5)].stringVal), expr);
            }
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 728 "sql_parser.y"
    {
                RULELOG("whereExpression::comparisonOps::nestedSubQueryOperator::Subquery");
                (yyval.node) = (Node *) createNestedSubquery((yyvsp[(3) - (6)].stringVal), (yyvsp[(1) - (6)].node), (yyvsp[(2) - (6)].stringVal), (yyvsp[(5) - (6)].node));
            }
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 733 "sql_parser.y"
    {
                RULELOG("whereExpression::comparisonOps::Subquery");
                List *expr = singleton((yyvsp[(1) - (5)].node));
                expr = appendToTailOfList(expr, (yyvsp[(4) - (5)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (5)].stringVal), expr);
            }
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 740 "sql_parser.y"
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

  case 109:

/* Line 1806 of yacc.c  */
#line 753 "sql_parser.y"
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

  case 110:

/* Line 1806 of yacc.c  */
#line 768 "sql_parser.y"
    { RULELOG("nestedSubQueryOperator::ANY"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 769 "sql_parser.y"
    { RULELOG("nestedSubQueryOperator::ALL"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 770 "sql_parser.y"
    { RULELOG("nestedSubQueryOperator::SOME"); (yyval.stringVal) = "ANY"; }
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 774 "sql_parser.y"
    { RULELOG("optionalNot::NULL"); (yyval.stringVal) = NULL; }
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 775 "sql_parser.y"
    { RULELOG("optionalNot::NOT"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 779 "sql_parser.y"
    { RULELOG("optionalGroupBy::NULL"); (yyval.list) = NULL; }
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 780 "sql_parser.y"
    { RULELOG("optionalGroupBy::GROUPBY"); (yyval.list) = (yyvsp[(3) - (3)].list); }
    break;

  case 117:

/* Line 1806 of yacc.c  */
#line 784 "sql_parser.y"
    { RULELOG("optionalOrderBy:::NULL"); (yyval.node) = NULL; }
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 786 "sql_parser.y"
    { 
                RULELOG("optionalHaving::HAVING"); 
                List *expr = singleton((yyvsp[(2) - (4)].node));
                expr = appendToTailOfList(expr, (yyvsp[(4) - (4)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(3) - (4)].stringVal), expr);
            }
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 795 "sql_parser.y"
    { RULELOG("optionalOrderBy:::NULL"); (yyval.list) = NULL; }
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 796 "sql_parser.y"
    { RULELOG("optionalOrderBy::ORDERBY"); (yyval.list) = (yyvsp[(3) - (3)].list); }
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 800 "sql_parser.y"
    { RULELOG("optionalLimit::NULL"); (yyval.node) = NULL; }
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 801 "sql_parser.y"
    { RULELOG("optionalLimit::CONSTANT"); (yyval.node) = (yyvsp[(2) - (2)].node);}
    break;

  case 123:

/* Line 1806 of yacc.c  */
#line 806 "sql_parser.y"
    {
                RULELOG("clauseList::attributeRef");
                (yyval.list) = singleton((yyvsp[(1) - (1)].node));
            }
    break;

  case 124:

/* Line 1806 of yacc.c  */
#line 811 "sql_parser.y"
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
            }
    break;

  case 125:

/* Line 1806 of yacc.c  */
#line 816 "sql_parser.y"
    {
                RULELOG("clauseList::constant");
                (yyval.list) = singleton((yyvsp[(1) - (1)].node));
            }
    break;

  case 126:

/* Line 1806 of yacc.c  */
#line 821 "sql_parser.y"
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
            }
    break;



/* Line 1806 of yacc.c  */
#line 2865 "sql_parser.tab.c"
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
#line 828 "sql_parser.y"




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


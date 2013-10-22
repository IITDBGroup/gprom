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
#line 191 "sql_parser.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 203 "sql_parser.tab.c"

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
#define YYFINAL  20
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   255

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  65
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  30
/* YYNRULES -- Number of rules.  */
#define YYNRULES  77
/* YYNRULES -- Number of states.  */
#define YYNSTATES  141

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   306

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    15,     2,     2,     2,    11,    13,     2,
      18,    17,     9,     7,    64,     8,     2,    10,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    63,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
       5,     6,    16,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     6,     9,    11,    13,    15,    19,    21,
      23,    25,    31,    33,    35,    37,    41,    45,    50,    51,
      53,    59,    60,    62,    68,    70,    74,    76,    80,    82,
      86,    90,    92,    94,    96,    98,   100,   102,   104,   106,
     108,   112,   116,   120,   124,   128,   132,   136,   140,   144,
     147,   152,   153,   156,   158,   162,   167,   170,   172,   173,
     175,   178,   179,   182,   186,   188,   192,   196,   200,   206,
     213,   219,   226,   232,   234,   236,   238,   239
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      66,     0,    -1,    67,    63,    -1,    68,    63,    -1,    72,
      -1,    70,    -1,    71,    -1,    18,    68,    17,    -1,    75,
      -1,    69,    -1,    73,    -1,    23,    24,    18,    66,    17,
      -1,    22,    -1,    21,    -1,    20,    -1,    68,    47,    68,
      -1,    68,    48,    68,    -1,    68,    46,    74,    68,    -1,
      -1,    41,    -1,    19,    76,    77,    86,    91,    -1,    -1,
      28,    -1,    28,    59,    18,    79,    17,    -1,    78,    -1,
      77,    64,    78,    -1,    80,    -1,    80,    26,     6,    -1,
      80,    -1,    79,    64,    80,    -1,    18,    80,    17,    -1,
      81,    -1,    82,    -1,    83,    -1,    84,    -1,    85,    -1,
       3,    -1,     4,    -1,     5,    -1,     6,    -1,    80,     7,
      80,    -1,    80,     8,    80,    -1,    80,     9,    80,    -1,
      80,    10,    80,    -1,    80,    11,    80,    -1,    80,    12,
      80,    -1,    80,    13,    80,    -1,    80,    14,    80,    -1,
      80,    16,    80,    -1,    15,    80,    -1,     6,    18,    79,
      17,    -1,    -1,    25,    87,    -1,    89,    -1,    87,    64,
      89,    -1,    18,    68,    17,    90,    -1,     6,    90,    -1,
      88,    -1,    -1,     6,    -1,    26,     6,    -1,    -1,    27,
      92,    -1,    18,    92,    17,    -1,    80,    -1,    92,    30,
      92,    -1,    92,    31,    92,    -1,    92,    32,    92,    -1,
      92,    36,    92,    30,    92,    -1,    80,    16,    93,    18,
      68,    17,    -1,    80,    16,    18,    68,    17,    -1,    80,
      94,    34,    18,    68,    17,    -1,    94,    38,    18,    68,
      17,    -1,    42,    -1,    41,    -1,    45,    -1,    -1,    33,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   111,   111,   117,   129,   130,   131,   138,   139,   140,
     141,   148,   159,   166,   173,   181,   186,   191,   199,   200,
     209,   228,   229,   234,   246,   250,   258,   263,   274,   275,
     287,   288,   289,   290,   291,   292,   301,   302,   303,   310,
     328,   335,   342,   349,   356,   363,   372,   379,   388,   398,
     410,   423,   424,   428,   433,   441,   449,   454,   462,   463,
     464,   471,   472,   476,   477,   478,   485,   492,   499,   507,
     512,   519,   532,   548,   549,   550,   554,   555
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "intConst", "floatConst", "stringConst",
  "identifier", "'+'", "'-'", "'*'", "'/'", "'%'", "'^'", "'&'", "'|'",
  "'!'", "comparisonOps", "')'", "'('", "SELECT", "INSERT", "UPDATE",
  "DELETE", "PROVENANCE", "OF", "FROM", "AS", "WHERE", "DISTINCT",
  "STARALL", "AND", "OR", "LIKE", "NOT", "IN", "ISNULL", "BETWEEN",
  "EXCEPT", "EXISTS", "AMMSC", "NULLVAL", "ALL", "ANY", "BY", "IS", "SOME",
  "UNION", "INTERSECT", "MINUS", "INTO", "VALUES", "DUMMYEXPR", "JOIN",
  "NATURAL", "LEFT", "RIGHT", "OUTER", "INNER", "CROSS", "ON", "USING",
  "FULL", "XOR", "';'", "','", "$accept", "stmt", "dmlStmt", "queryStmt",
  "provStmt", "deleteQuery", "updateQuery", "insertQuery",
  "setOperatorQuery", "optionalAll", "selectQuery", "optionalDistinct",
  "selectClause", "selectItem", "exprList", "expression", "constant",
  "attributeRef", "binaryOperatorExpression", "unaryOperatorExpression",
  "sqlFunctionCall", "optionalFrom", "fromClause", "subQuery",
  "fromClauseItem", "optionalAlias", "optionalWhere", "whereExpression",
  "nestedSubQueryOperator", "optionalNot", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,    43,    45,    42,
      47,    37,    94,    38,   124,    33,   262,    41,    40,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,    59,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    65,    66,    66,    67,    67,    67,    68,    68,    68,
      68,    69,    70,    71,    72,    73,    73,    73,    74,    74,
      75,    76,    76,    76,    77,    77,    78,    78,    79,    79,
      80,    80,    80,    80,    80,    80,    81,    81,    81,    82,
      83,    83,    83,    83,    83,    83,    83,    83,    83,    84,
      85,    86,    86,    87,    87,    88,    89,    89,    90,    90,
      90,    91,    91,    92,    92,    92,    92,    92,    92,    92,
      92,    92,    92,    93,    93,    93,    94,    94
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     2,     1,     1,     1,     3,     1,     1,
       1,     5,     1,     1,     1,     3,     3,     4,     0,     1,
       5,     0,     1,     5,     1,     3,     1,     3,     1,     3,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       4,     0,     2,     1,     3,     4,     2,     1,     0,     1,
       2,     0,     2,     3,     1,     3,     3,     3,     5,     6,
       5,     6,     5,     1,     1,     1,     0,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    21,    14,    13,    12,     0,     0,     0,     0,
       9,     5,     6,     4,    10,     8,     0,    22,     0,     0,
       1,     2,    18,     0,     0,     3,     7,     0,    36,    37,
      38,    39,     0,     0,    51,    24,    26,    31,    32,    33,
      34,    35,     0,    19,     0,    15,    16,     0,     0,    49,
       0,     0,     0,    61,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    17,     0,    28,     0,    30,
      58,     0,    52,    57,    53,    25,    76,    20,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    27,    11,    23,
       0,    50,    59,     0,    56,     0,     0,    76,    77,    64,
      62,     0,    29,    60,    58,    54,    64,     0,     0,     0,
      76,    76,    76,    76,     0,    55,    63,     0,    74,    73,
      75,     0,     0,    65,    66,    67,     0,     0,     0,     0,
       0,     0,    76,     0,    70,     0,     0,    68,    72,    69,
      71
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     7,     8,     9,    10,    11,    12,    13,    14,    44,
      15,    18,    34,    35,    66,    99,    37,    38,    39,    40,
      41,    53,    72,    73,    74,    94,    77,   100,   121,   101
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -98
static const yytype_int16 yypact[] =
{
     216,   228,   -11,   -98,   -98,   -98,     2,    19,    -3,   -34,
     -98,   -98,   -98,   -98,   -98,   -98,   -16,     7,   174,    61,
     -98,   -98,    42,   228,   228,   -98,   -98,    66,   -98,   -98,
     -98,    67,   174,   174,    -5,   -98,   160,   -98,   -98,   -98,
     -98,   -98,   216,   -98,   228,   -98,   -98,   174,   174,   217,
     186,    10,   174,    72,   174,   174,   174,   174,   174,   174,
     174,   174,   174,    95,    93,   -98,    -9,   197,    -6,   -98,
       1,   228,    47,   -98,   -98,   -98,    99,   -98,   243,   243,
     101,   101,   101,   -98,   217,   207,   233,   -98,   -98,   -98,
     174,   -98,   -98,   109,   -98,    16,    10,    99,   -98,   126,
      92,    80,   197,   -98,     1,   -98,    64,    76,     0,    85,
      99,    99,    99,    99,   103,   -98,   -98,   143,   -98,   -98,
     -98,   107,   113,   120,   120,   120,   151,   228,   143,    21,
     228,   228,    99,    40,   -98,    44,    48,   120,   -98,   -98,
     -98
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -98,   102,   -98,    -1,   -98,   -98,   -98,   -98,   -98,   -98,
     -98,   -98,   -98,    89,    97,    -8,   -98,   -98,   -98,   -98,
     -98,   -98,   -98,   -98,    57,    39,   -98,   -76,   -98,   -97
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -77
static const yytype_int16 yytable[] =
{
      16,    26,   109,    28,    29,    30,    31,    92,    89,   109,
      36,    91,    22,    23,    24,    32,    70,    17,   117,    20,
      51,   107,    45,    46,    49,    50,    19,    93,    71,    25,
      22,    23,    24,   104,   123,   124,   125,   126,   134,    67,
      67,   118,   119,    65,    36,   120,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    90,   137,   138,    90,    52,
      21,   139,    22,    23,    24,   140,    27,    22,    23,    24,
      95,    54,    55,    56,    57,    58,    59,    60,    61,    42,
     108,    69,   102,    43,    47,    48,    22,    23,    24,   106,
      22,    23,    24,   116,    22,    23,    24,    98,   -76,    76,
      86,    87,    28,    29,    30,    31,   110,   111,   112,    50,
      88,    96,   113,    59,    32,   103,   129,    97,   114,   122,
      50,   127,   110,   111,   112,   130,   133,    16,   113,   135,
     136,   131,    98,    54,    55,    56,    57,    58,    59,    60,
      61,    75,   108,   115,    64,    68,    28,    29,    30,    31,
     -77,   -77,   -77,   105,     0,     0,   -77,     0,    32,    98,
     -76,   128,     2,     0,     0,     0,     6,    54,    55,    56,
      57,    58,    59,    60,    61,     0,    62,    28,    29,    30,
      31,   132,   111,   112,     0,     0,    63,   113,     0,    32,
       0,     0,    33,    54,    55,    56,    57,    58,    59,    60,
      61,     0,    62,    69,    54,    55,    56,    57,    58,    59,
      60,    61,     0,    62,    54,    55,    56,    57,    58,    59,
      60,     0,     0,    62,    54,    55,    56,    57,    58,    59,
       0,     0,     0,    62,     1,     2,     3,     4,     5,     6,
      54,    55,    56,    57,    58,    59,     1,     2,     0,     0,
       0,     6,    56,    57,    58,    59
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-98))

#define yytable_value_is_error(yytable_value) \
  ((yytable_value) == (-77))

static const yytype_int16 yycheck[] =
{
       1,    17,    99,     3,     4,     5,     6,     6,    17,   106,
      18,    17,    46,    47,    48,    15,     6,    28,    18,     0,
      25,    97,    23,    24,    32,    33,    24,    26,    18,    63,
      46,    47,    48,    17,   110,   111,   112,   113,    17,    47,
      48,    41,    42,    44,    52,    45,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    64,   132,    17,    64,    64,
      63,    17,    46,    47,    48,    17,    59,    46,    47,    48,
      71,     7,     8,     9,    10,    11,    12,    13,    14,    18,
      16,    17,    90,    41,    18,    18,    46,    47,    48,    97,
      46,    47,    48,    17,    46,    47,    48,    33,    34,    27,
     108,     6,     3,     4,     5,     6,    30,    31,    32,   117,
      17,    64,    36,    12,    15,     6,   117,    18,    38,    34,
     128,    18,    30,    31,    32,    18,   127,   128,    36,   130,
     131,    18,    33,     7,     8,     9,    10,    11,    12,    13,
      14,    52,    16,   104,    42,    48,     3,     4,     5,     6,
      30,    31,    32,    96,    -1,    -1,    36,    -1,    15,    33,
      34,    18,    19,    -1,    -1,    -1,    23,     7,     8,     9,
      10,    11,    12,    13,    14,    -1,    16,     3,     4,     5,
       6,    30,    31,    32,    -1,    -1,    26,    36,    -1,    15,
      -1,    -1,    18,     7,     8,     9,    10,    11,    12,    13,
      14,    -1,    16,    17,     7,     8,     9,    10,    11,    12,
      13,    14,    -1,    16,     7,     8,     9,    10,    11,    12,
      13,    -1,    -1,    16,     7,     8,     9,    10,    11,    12,
      -1,    -1,    -1,    16,    18,    19,    20,    21,    22,    23,
       7,     8,     9,    10,    11,    12,    18,    19,    -1,    -1,
      -1,    23,     9,    10,    11,    12
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    18,    19,    20,    21,    22,    23,    66,    67,    68,
      69,    70,    71,    72,    73,    75,    68,    28,    76,    24,
       0,    63,    46,    47,    48,    63,    17,    59,     3,     4,
       5,     6,    15,    18,    77,    78,    80,    81,    82,    83,
      84,    85,    18,    41,    74,    68,    68,    18,    18,    80,
      80,    25,    64,    86,     7,     8,     9,    10,    11,    12,
      13,    14,    16,    26,    66,    68,    79,    80,    79,    17,
       6,    18,    87,    88,    89,    78,    27,    91,    80,    80,
      80,    80,    80,    80,    80,    80,    80,     6,    17,    17,
      64,    17,     6,    26,    90,    68,    64,    18,    33,    80,
      92,    94,    80,     6,    17,    89,    80,    92,    16,    94,
      30,    31,    32,    36,    38,    90,    17,    18,    41,    42,
      45,    93,    34,    92,    92,    92,    92,    18,    18,    68,
      18,    18,    30,    68,    17,    68,    68,    92,    17,    17,
      17
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
#line 112 "sql_parser.y"
    {
            RULELOG("stmt::dmlStmt");
            (yyval.node) = (yyvsp[(1) - (2)].node);
            bisonParseResult = (Node *) (yyval.node);
        }
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 118 "sql_parser.y"
    {
            RULELOG("stmt::queryStmt");
            (yyval.node) = (yyvsp[(1) - (2)].node);
            bisonParseResult = (Node *) (yyval.node);
        }
    break;

  case 4:

/* Line 1806 of yacc.c  */
#line 129 "sql_parser.y"
    { RULELOG("dmlStmt::insertQuery"); }
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 130 "sql_parser.y"
    { RULELOG("dmlStmt::deleteQuery"); }
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 131 "sql_parser.y"
    { RULELOG("dmlStmt::updateQuery"); }
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 138 "sql_parser.y"
    { RULELOG("queryStmt::bracketedQuery"); (yyval.node) = (yyvsp[(2) - (3)].node); }
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 139 "sql_parser.y"
    { RULELOG("queryStmt::selectQuery"); }
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 140 "sql_parser.y"
    { RULELOG("queryStmt::provStmt"); }
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 141 "sql_parser.y"
    { RULELOG("queryStmt::setOperatorQuery"); }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 149 "sql_parser.y"
    {
            RULELOG(provStmt);
            (yyval.node) = (Node *) createProvenanceStmt((yyvsp[(4) - (5)].node));
        }
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 159 "sql_parser.y"
    { RULELOG(deleteQuery); (yyval.node) = NULL; }
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 166 "sql_parser.y"
    { RULELOG(updateQuery); (yyval.node) = NULL; }
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 173 "sql_parser.y"
    { RULELOG(insertQuery); (yyval.node) = NULL; }
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 182 "sql_parser.y"
    {
                RULELOG("setOperatorQuery::INTERSECT");
                (yyval.node) = (Node *) createSetQuery((yyvsp[(2) - (3)].stringVal), FALSE, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
            }
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 187 "sql_parser.y"
    {
                RULELOG("setOperatorQuery::MINUS");
                (yyval.node) = (Node *) createSetQuery((yyvsp[(2) - (3)].stringVal), FALSE, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
            }
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 192 "sql_parser.y"
    {
                RULELOG("setOperatorQuery::UNION");
                (yyval.node) = (Node *) createSetQuery((yyvsp[(2) - (4)].stringVal), ((yyvsp[(3) - (4)].stringVal) != NULL), (yyvsp[(1) - (4)].node), (yyvsp[(4) - (4)].node));
            }
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 199 "sql_parser.y"
    { RULELOG("optionalAll::NULL"); (yyval.stringVal) = NULL; }
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 200 "sql_parser.y"
    { RULELOG("optionalAll::ALLTRUE"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 210 "sql_parser.y"
    {
                RULELOG(selectQuery);
                QueryBlock *q =  createQueryBlock();
                
                q->distinct = (yyvsp[(2) - (5)].node);
                q->selectClause = (yyvsp[(3) - (5)].list);
                q->fromClause = (yyvsp[(4) - (5)].list);
                q->whereClause = (yyvsp[(5) - (5)].node);
                
                (yyval.node) = (Node *) q; 
            }
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 228 "sql_parser.y"
    { RULELOG("optionalDistinct::NULL"); (yyval.node) = NULL; }
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 230 "sql_parser.y"
    {
                RULELOG("optionalDistinct::DISTINCT");
                (yyval.node) = (Node *) createDistinctClause(NULL);
            }
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 235 "sql_parser.y"
    {
                RULELOG("optionalDistinct::DISTINCT::exprList");
                (yyval.node) = (Node *) createDistinctClause((yyvsp[(4) - (5)].list));
            }
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 247 "sql_parser.y"
    {
                RULELOG("selectClause::selectItem"); (yyval.list) = singleton((yyvsp[(1) - (1)].node));
            }
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 251 "sql_parser.y"
    {
                RULELOG("selectClause::selectClause::selectItem");
                (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node)); 
            }
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 259 "sql_parser.y"
    {
                 RULELOG("selectItem::expression"); 
                 (yyval.node) = (Node *) createSelectItem(NULL, (yyvsp[(1) - (1)].node)); 
             }
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 264 "sql_parser.y"
    {
                 RULELOG("selectItem::expression::identifier"); 
                 (yyval.node) = (Node *) createSelectItem((yyvsp[(3) - (3)].stringVal), (yyvsp[(1) - (3)].node));
             }
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 274 "sql_parser.y"
    { RULELOG("exprList::SINGLETON"); (yyval.list) = singleton((yyvsp[(1) - (1)].node)); }
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 276 "sql_parser.y"
    {
                  RULELOG("exprList::exprList::expression");
                  (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
             }
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 287 "sql_parser.y"
    { RULELOG("expression::bracked"); (yyval.node) = (yyvsp[(2) - (3)].node); }
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 288 "sql_parser.y"
    { RULELOG("expression::constant"); }
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 289 "sql_parser.y"
    { RULELOG("expression::attributeRef"); }
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 290 "sql_parser.y"
    { RULELOG("expression::binaryOperatorExpression"); }
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 291 "sql_parser.y"
    { RULELOG("expression::unaryOperatorExpression"); }
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 292 "sql_parser.y"
    { RULELOG("expression::sqlFunctionCall"); }
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 301 "sql_parser.y"
    { RULELOG("constant::INT"); (yyval.node) = (Node *) createConstInt((yyvsp[(1) - (1)].intVal)); }
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 302 "sql_parser.y"
    { RULELOG("constant::FLOAT"); (yyval.node) = (Node *) createConstFloat((yyvsp[(1) - (1)].floatVal)); }
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 303 "sql_parser.y"
    { RULELOG("constant::STRING"); (yyval.node) = (Node *) createConstString((yyvsp[(1) - (1)].stringVal)); }
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 310 "sql_parser.y"
    { RULELOG("attributeRef::IDENTIFIER"); (yyval.node) = (Node *) createAttributeReference((yyvsp[(1) - (1)].stringVal)); }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 329 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '+' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 336 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '-' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 343 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '*' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 350 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '/' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 357 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '%' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 364 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '^' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 373 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '&' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 380 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '|' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 389 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression::comparisonOps");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 399 "sql_parser.y"
    {
                RULELOG("unaryOperatorExpression:: '!' ");
                List *expr = singleton((yyvsp[(2) - (2)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(1) - (2)].stringVal), expr);
            }
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 411 "sql_parser.y"
    {
                RULELOG("sqlFunctionCall::IDENTIFIER::exprList"); 
                (yyval.node) = (Node *) createFunctionCall((yyvsp[(1) - (4)].stringVal), (yyvsp[(3) - (4)].list)); 
            }
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 423 "sql_parser.y"
    { RULELOG("optionalFrom::NULL"); (yyval.list) = NULL; }
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 424 "sql_parser.y"
    { RULELOG("optionalFrom::fromClause"); (yyval.list) = (yyvsp[(2) - (2)].list); }
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 429 "sql_parser.y"
    {
                RULELOG("fromClause::fromClauseItem");
                (yyval.list) = singleton((yyvsp[(1) - (1)].node));
            }
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 434 "sql_parser.y"
    {
                RULELOG("fromClause::fromClause::fromClauseItem");
                (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
            }
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 442 "sql_parser.y"
    {
                RULELOG("subQuery::queryStmt");
                (yyval.node) = (Node *) createFromSubquery((yyvsp[(4) - (4)].stringVal), NULL, (yyvsp[(2) - (4)].node));
            }
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 450 "sql_parser.y"
    {
                RULELOG("fromClauseItem");
                (yyval.node) = (Node *) createFromTableRef((yyvsp[(2) - (2)].stringVal), NIL, (yyvsp[(1) - (2)].stringVal));
            }
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 455 "sql_parser.y"
    {
                RULELOG("fromClauseItem::subQuery");
                (yyval.node) = (yyvsp[(1) - (1)].node);
            }
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 462 "sql_parser.y"
    { RULELOG("optionalAlias::NULL"); (yyval.stringVal) = NULL; }
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 463 "sql_parser.y"
    { RULELOG("optionalAlias::identifier"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 464 "sql_parser.y"
    { RULELOG("optionalAlias::identifier"); (yyval.stringVal) = (yyvsp[(2) - (2)].stringVal); }
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 471 "sql_parser.y"
    { RULELOG("optionalWhere::NULL"); (yyval.node) = NULL; }
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 472 "sql_parser.y"
    { RULELOG("optionalWhere::whereExpression"); (yyval.node) = (yyvsp[(2) - (2)].node); }
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 476 "sql_parser.y"
    { RULELOG("where::brackedWhereExpression"); (yyval.node) = (yyvsp[(2) - (3)].node); }
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 477 "sql_parser.y"
    { RULELOG("whereExpression::expression"); (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 479 "sql_parser.y"
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 486 "sql_parser.y"
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 493 "sql_parser.y"
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 500 "sql_parser.y"
    {
                RULELOG("whereExpression::BETWEEN-AND");
                List *expr = singleton((yyvsp[(1) - (5)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (5)].node));
                expr = appendToTailOfList(expr, (yyvsp[(5) - (5)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (5)].stringVal), expr);
            }
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 508 "sql_parser.y"
    {
                RULELOG("whereExpression::comparisonOps::nestedSubQueryOperator::Subquery");
                (yyval.node) = (Node *) createNestedSubquery((yyvsp[(3) - (6)].stringVal), (yyvsp[(1) - (6)].node), (yyvsp[(2) - (6)].stringVal), (yyvsp[(5) - (6)].node));
            }
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 513 "sql_parser.y"
    {
                RULELOG("whereExpression::comparisonOps::Subquery");
                List *expr = singleton((yyvsp[(1) - (5)].node));
                expr = appendToTailOfList(expr, (yyvsp[(4) - (5)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (5)].stringVal), expr);
            }
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 520 "sql_parser.y"
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

  case 72:

/* Line 1806 of yacc.c  */
#line 533 "sql_parser.y"
    {
                if ((yyvsp[(1) - (5)].stringVal) == NULL)
                {
                    RULELOG("whereExpression::EXISTS");
                    (yyval.node) = (Node *) createNestedSubquery((yyvsp[(2) - (5)].stringVal), NULL, (yyvsp[(1) - (5)].stringVal), (yyvsp[(4) - (5)].node));
                }
                else
                {
                    RULELOG("whereExpression::EXISTS::NOT");
                    (yyval.node) = (Node *) createNestedSubquery((yyvsp[(2) - (5)].stringVal), NULL, "<>", (yyvsp[(4) - (5)].node));
                }
            }
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 548 "sql_parser.y"
    { RULELOG("nestedSubQueryOperator::ANY"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 549 "sql_parser.y"
    { RULELOG("nestedSubQueryOperator::ALL"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 550 "sql_parser.y"
    { RULELOG("nestedSubQueryOperator::SOME"); (yyval.stringVal) = "ANY"; }
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 554 "sql_parser.y"
    { RULELOG("optionalNot::NULL"); (yyval.stringVal) = NULL; }
    break;

  case 77:

/* Line 1806 of yacc.c  */
#line 555 "sql_parser.y"
    { RULELOG("optionalNot::NOT"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;



/* Line 1806 of yacc.c  */
#line 2285 "sql_parser.tab.c"
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
#line 558 "sql_parser.y"



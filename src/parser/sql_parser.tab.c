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


/* Line 268 of yacc.c  */
#line 92 "sql_parser.tab.c"

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
     BASERELATION = 269,
     SCN = 270,
     TIMESTAMP = 271,
     HAS = 272,
     FROM = 273,
     AS = 274,
     WHERE = 275,
     DISTINCT = 276,
     STARALL = 277,
     AND = 278,
     OR = 279,
     LIKE = 280,
     NOT = 281,
     IN = 282,
     ISNULL = 283,
     BETWEEN = 284,
     EXCEPT = 285,
     EXISTS = 286,
     AMMSC = 287,
     NULLVAL = 288,
     ALL = 289,
     ANY = 290,
     IS = 291,
     SOME = 292,
     UNION = 293,
     INTERSECT = 294,
     MINUS = 295,
     INTO = 296,
     VALUES = 297,
     HAVING = 298,
     GROUP = 299,
     ORDER = 300,
     BY = 301,
     LIMIT = 302,
     SET = 303,
     INT = 304,
     BEGIN_TRANS = 305,
     COMMIT_TRANS = 306,
     ROLLBACK_TRANS = 307,
     DUMMYEXPR = 308,
     JOIN = 309,
     NATURAL = 310,
     LEFT = 311,
     RIGHT = 312,
     OUTER = 313,
     INNER = 314,
     CROSS = 315,
     ON = 316,
     USING = 317,
     FULL = 318,
     TYPE = 319,
     TRANSACTION = 320,
     WITH = 321,
     XOR = 322
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 293 of yacc.c  */
#line 28 "sql_parser.y"

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
#line 209 "sql_parser.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 221 "sql_parser.tab.c"

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
#define YYFINAL  30
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   431

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  83
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  51
/* YYNRULES -- Number of rules.  */
#define YYNRULES  144
/* YYNRULES -- Number of states.  */
#define YYNSTATES  276

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   322

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    15,     2,     2,     2,    11,    13,     2,
      18,    17,     9,     7,    81,     8,    82,    10,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    80,
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
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,    10,    12,    14,    16,    18,    20,
      22,    26,    28,    30,    32,    34,    36,    38,    46,    54,
      61,    62,    67,    72,    73,    76,    78,    81,    84,    90,
      91,    93,    99,   101,   105,   109,   113,   121,   126,   128,
     130,   134,   138,   142,   146,   151,   152,   154,   164,   165,
     167,   173,   175,   179,   181,   185,   187,   191,   193,   197,
     201,   203,   205,   207,   209,   211,   213,   215,   217,   219,
     223,   227,   231,   235,   239,   243,   247,   251,   255,   258,
     263,   264,   267,   269,   273,   276,   279,   282,   285,   287,
     292,   296,   298,   302,   306,   311,   317,   322,   328,   333,
     335,   338,   340,   343,   346,   348,   350,   355,   358,   362,
     367,   368,   370,   376,   377,   381,   382,   385,   389,   391,
     394,   398,   402,   406,   412,   419,   425,   432,   437,   439,
     441,   443,   444,   446,   447,   451,   452,   457,   458,   462,
     463,   466,   468,   472,   474
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      84,     0,    -1,    85,    80,    -1,    84,    85,    80,    -1,
      86,    -1,    87,    -1,    88,    -1,    99,    -1,    94,    -1,
      96,    -1,    18,    87,    17,    -1,   103,    -1,    89,    -1,
     101,    -1,    62,    -1,    63,    -1,    64,    -1,    24,    90,
      91,    25,    18,    85,    17,    -1,    24,    90,    91,    25,
      18,    84,    17,    -1,    24,    90,    91,    25,    77,     3,
      -1,    -1,    31,    25,    27,     3,    -1,    31,    25,    28,
       5,    -1,    -1,    78,    92,    -1,    93,    -1,    92,    93,
      -1,    76,     5,    -1,    23,    95,     6,    32,   126,    -1,
      -1,    30,    -1,    22,     6,    60,    97,   125,    -1,    98,
      -1,    97,    81,    98,    -1,   110,    16,   108,    -1,   110,
      16,   117,    -1,    21,    53,     6,    54,    18,   100,    17,
      -1,    21,    53,     6,    87,    -1,   109,    -1,     6,    -1,
     100,    81,     6,    -1,   100,    81,   109,    -1,    87,    51,
      87,    -1,    87,    52,    87,    -1,    87,    50,   102,    87,
      -1,    -1,    46,    -1,    20,   104,   105,   114,   125,   129,
     130,   131,   132,    -1,    -1,    33,    -1,    33,    73,    18,
     107,    17,    -1,   106,    -1,   105,    81,   106,    -1,   108,
      -1,   108,    31,     6,    -1,     9,    -1,     6,    82,     9,
      -1,   108,    -1,   107,    81,   108,    -1,    18,   108,    17,
      -1,   109,    -1,   110,    -1,   111,    -1,   112,    -1,   113,
      -1,     3,    -1,     4,    -1,     5,    -1,     6,    -1,   108,
       7,   108,    -1,   108,     8,   108,    -1,   108,     9,   108,
      -1,   108,    10,   108,    -1,   108,    11,   108,    -1,   108,
      12,   108,    -1,   108,    13,   108,    -1,   108,    14,   108,
      -1,   108,    16,   108,    -1,    15,   108,    -1,     6,    18,
     107,    17,    -1,    -1,    30,   115,    -1,   116,    -1,   115,
      81,   116,    -1,     6,   123,    -1,     6,   122,    -1,   117,
     123,    -1,   117,   122,    -1,   119,    -1,    18,   119,    17,
     122,    -1,    18,    87,    17,    -1,     6,    -1,   118,    81,
       6,    -1,    18,   119,    17,    -1,   116,    67,    66,   116,
      -1,   116,    67,   120,    66,   116,    -1,   116,    72,    66,
     116,    -1,   116,   120,    66,   116,   121,    -1,   116,    66,
     116,   121,    -1,    68,    -1,    68,    70,    -1,    69,    -1,
      69,    70,    -1,    75,    70,    -1,    75,    -1,    71,    -1,
      74,    18,   118,    17,    -1,    73,   126,    -1,   123,     6,
     124,    -1,   123,    31,     6,   124,    -1,    -1,    26,    -1,
      29,    24,    18,   118,    17,    -1,    -1,    18,   118,    17,
      -1,    -1,    32,   126,    -1,    18,   126,    17,    -1,   108,
      -1,    38,   126,    -1,   126,    35,   126,    -1,   126,    36,
     126,    -1,   126,    37,   126,    -1,   126,    41,   108,    35,
     108,    -1,   108,    16,   127,    18,    87,    17,    -1,   108,
      16,    18,    87,    17,    -1,   108,   128,    39,    18,    87,
      17,    -1,    43,    18,    87,    17,    -1,    47,    -1,    46,
      -1,    49,    -1,    -1,    38,    -1,    -1,    56,    58,   133,
      -1,    -1,    55,   113,    16,   108,    -1,    -1,    57,    58,
     133,    -1,    -1,    59,   109,    -1,   110,    -1,   133,    81,
     110,    -1,   109,    -1,   133,    81,   109,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   124,   124,   130,   139,   144,   149,   160,   161,   162,
     169,   170,   171,   172,   176,   177,   178,   185,   196,   206,
     218,   219,   224,   232,   233,   241,   242,   250,   261,   270,
     271,   285,   293,   298,   306,   315,   330,   335,   343,   348,
     353,   358,   372,   377,   382,   390,   391,   400,   423,   424,
     429,   441,   445,   453,   458,   463,   468,   479,   480,   492,
     493,   494,   495,   496,   497,   506,   507,   508,   515,   534,
     541,   548,   555,   562,   569,   578,   585,   594,   604,   616,
     629,   630,   634,   639,   648,   655,   664,   671,   680,   688,
     701,   709,   710,   714,   715,   722,   728,   734,   742,   754,
     755,   756,   757,   758,   759,   760,   764,   765,   769,   776,
     786,   787,   795,   806,   807,   817,   818,   822,   823,   824,
     830,   837,   844,   851,   859,   864,   871,   884,   900,   901,
     902,   906,   907,   911,   912,   916,   917,   927,   928,   932,
     933,   937,   942,   947,   952
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
  "UPDATE", "DELETE", "PROVENANCE", "OF", "BASERELATION", "SCN",
  "TIMESTAMP", "HAS", "FROM", "AS", "WHERE", "DISTINCT", "STARALL", "AND",
  "OR", "LIKE", "NOT", "IN", "ISNULL", "BETWEEN", "EXCEPT", "EXISTS",
  "AMMSC", "NULLVAL", "ALL", "ANY", "IS", "SOME", "UNION", "INTERSECT",
  "MINUS", "INTO", "VALUES", "HAVING", "GROUP", "ORDER", "BY", "LIMIT",
  "SET", "INT", "BEGIN_TRANS", "COMMIT_TRANS", "ROLLBACK_TRANS",
  "DUMMYEXPR", "JOIN", "NATURAL", "LEFT", "RIGHT", "OUTER", "INNER",
  "CROSS", "ON", "USING", "FULL", "TYPE", "TRANSACTION", "WITH", "XOR",
  "';'", "','", "'.'", "$accept", "stmtList", "stmt", "dmlStmt",
  "queryStmt", "transactionIdentifier", "provStmt", "optionalProvAsOf",
  "optionalProvWith", "provOptionList", "provOption", "deleteQuery",
  "fromString", "updateQuery", "setClause", "setExpression", "insertQuery",
  "insertList", "setOperatorQuery", "optionalAll", "selectQuery",
  "optionalDistinct", "selectClause", "selectItem", "exprList",
  "expression", "constant", "attributeRef", "binaryOperatorExpression",
  "unaryOperatorExpression", "sqlFunctionCall", "optionalFrom",
  "fromClause", "fromClauseItem", "subQuery", "identifierList",
  "fromJoinItem", "joinType", "joinCond", "optionalAlias",
  "optionalFromProv", "optionalAttrAlias", "optionalWhere",
  "whereExpression", "nestedSubQueryOperator", "optionalNot",
  "optionalGroupBy", "optionalHaving", "optionalOrderBy", "optionalLimit",
  "clauseList", 0
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
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
      59,    44,    46
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    83,    84,    84,    85,    85,    85,    86,    86,    86,
      87,    87,    87,    87,    88,    88,    88,    89,    89,    89,
      90,    90,    90,    91,    91,    92,    92,    93,    94,    95,
      95,    96,    97,    97,    98,    98,    99,    99,   100,   100,
     100,   100,   101,   101,   101,   102,   102,   103,   104,   104,
     104,   105,   105,   106,   106,   106,   106,   107,   107,   108,
     108,   108,   108,   108,   108,   109,   109,   109,   110,   111,
     111,   111,   111,   111,   111,   111,   111,   111,   112,   113,
     114,   114,   115,   115,   116,   116,   116,   116,   116,   116,
     117,   118,   118,   119,   119,   119,   119,   119,   119,   120,
     120,   120,   120,   120,   120,   120,   121,   121,   122,   122,
     123,   123,   123,   124,   124,   125,   125,   126,   126,   126,
     126,   126,   126,   126,   126,   126,   126,   126,   127,   127,
     127,   128,   128,   129,   129,   130,   130,   131,   131,   132,
     132,   133,   133,   133,   133
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     3,     1,     1,     1,     1,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     7,     7,     6,
       0,     4,     4,     0,     2,     1,     2,     2,     5,     0,
       1,     5,     1,     3,     3,     3,     7,     4,     1,     1,
       3,     3,     3,     3,     4,     0,     1,     9,     0,     1,
       5,     1,     3,     1,     3,     1,     3,     1,     3,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     4,
       0,     2,     1,     3,     2,     2,     2,     2,     1,     4,
       3,     1,     3,     3,     4,     5,     4,     5,     4,     1,
       2,     1,     2,     2,     1,     1,     4,     2,     3,     4,
       0,     1,     5,     0,     3,     0,     2,     3,     1,     2,
       3,     3,     3,     5,     6,     5,     6,     4,     1,     1,
       1,     0,     1,     0,     3,     0,     4,     0,     3,     0,
       2,     1,     3,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    48,     0,     0,    29,    20,    14,    15,    16,
       0,     0,     4,     5,     6,    12,     8,     9,     7,    13,
      11,     0,    49,     0,     0,     0,    30,     0,     0,    23,
       1,     0,     2,    45,     0,     0,    10,     0,    65,    66,
      67,    68,    55,     0,     0,    80,    51,    53,    60,    61,
      62,    63,    64,     0,     0,     0,     0,     0,     0,     3,
      46,     0,    42,    43,     0,     0,     0,    68,    78,     0,
       0,     0,   115,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    37,    68,   115,    32,     0,     0,
       0,     0,     0,    24,    25,     0,    44,     0,    57,     0,
      56,    59,   110,     0,    81,    82,   110,    88,    52,     0,
     133,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      54,     0,     0,    31,     0,     0,     0,     0,   118,    28,
      21,    22,    27,    26,     0,     0,    50,     0,    79,   111,
       0,    85,    84,     0,     0,     0,    88,     0,     0,     0,
      99,   101,   105,     0,   104,     0,    87,    86,   116,     0,
     135,    39,     0,    38,    33,     0,    34,    35,   118,     0,
     119,     0,     0,   132,     0,     0,     0,     0,     0,     0,
       0,    19,    58,     0,   113,     0,     0,    90,    93,    83,
       0,     0,     0,   100,   102,     0,   103,     0,     0,     0,
     137,    36,     0,     0,   117,     0,     0,   129,   128,   130,
       0,     0,   120,   121,   122,     0,    18,    17,     0,     0,
     108,   113,    90,    89,     0,     0,     0,    98,    94,     0,
      96,     0,   143,   141,   134,     0,     0,     0,   139,    40,
      41,   127,     0,     0,     0,     0,    91,     0,     0,   109,
     107,     0,    95,    97,     0,     0,     0,     0,    47,   125,
       0,     0,   123,   112,     0,   114,     0,   144,   142,   136,
     138,   140,   124,   126,    92,   106
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    10,    31,    12,    13,    14,    15,    29,    58,    93,
      94,    16,    27,    17,    86,    87,    18,   162,    19,    61,
      20,    23,    45,    46,    97,   128,    48,    49,    50,    51,
      52,    72,   104,   145,   106,   247,   107,   155,   227,   141,
     142,   220,   110,   129,   210,   174,   160,   200,   238,   258,
     234
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -211
static const yytype_int16 yypact[] =
{
      90,   117,   -23,   -36,    19,   -11,    -9,  -211,  -211,  -211,
      26,   -40,  -211,   111,  -211,  -211,  -211,  -211,  -211,  -211,
    -211,    82,   -20,   279,    55,    36,  -211,    56,    73,    38,
    -211,    39,  -211,    78,   117,   117,  -211,   125,  -211,  -211,
    -211,    -6,  -211,   310,   310,   -22,  -211,   292,  -211,  -211,
    -211,  -211,  -211,     0,   150,   112,    28,    81,   151,  -211,
    -211,   117,  -211,  -211,   310,   310,   170,   162,   367,   326,
      17,   279,   153,   310,   310,   310,   310,   310,   310,   310,
     310,   310,   176,   165,   111,  -211,   -18,  -211,   184,   163,
     183,   198,   199,    81,  -211,    20,  -211,    -4,   337,     4,
    -211,  -211,    16,   103,   127,   318,    16,  -211,  -211,   163,
     157,   225,   225,   202,   202,   202,  -211,   367,   357,   400,
    -211,   410,   150,  -211,   314,   163,   163,   201,   253,   365,
    -211,  -211,  -211,  -211,    90,   220,  -211,   310,  -211,  -211,
     203,  -211,    89,   103,    98,   318,   227,    17,    17,   328,
     177,   187,  -211,   180,   208,   221,  -211,    89,   365,   210,
     231,  -211,    10,  -211,  -211,   306,   337,  -211,   242,   204,
     365,   117,   169,  -211,   250,   163,   163,   163,   310,    83,
     -15,  -211,   337,   272,   275,   289,   138,  -211,   191,   318,
     288,    17,   230,  -211,  -211,    17,  -211,    17,   414,   301,
     265,  -211,   418,   306,  -211,   142,   306,  -211,  -211,  -211,
     309,   313,     2,     2,    88,   263,  -211,  -211,   335,   335,
    -211,   275,   160,  -211,    89,   163,   334,  -211,  -211,    17,
    -211,   288,  -211,  -211,   277,   162,   355,   330,   332,  -211,
    -211,  -211,   174,   117,   117,   310,  -211,    11,    12,  -211,
     365,   335,  -211,  -211,   414,   310,   414,   377,  -211,  -211,
     178,   181,   337,  -211,   366,  -211,    13,  -211,  -211,   337,
     277,  -211,  -211,  -211,  -211,  -211
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -211,   258,     6,  -211,    -1,  -211,  -211,  -211,  -211,  -211,
     302,  -211,  -211,  -211,  -211,   276,  -211,  -211,  -211,  -211,
    -211,  -211,  -211,   333,   340,    -7,  -118,   -47,  -211,  -211,
     226,  -211,  -211,   -69,   303,  -210,   -92,   280,   195,  -102,
    -101,   207,   344,   -94,  -211,  -211,  -211,  -211,  -211,  -211,
     175
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -132
static const yytype_int16 yytable[] =
{
      21,   105,   217,   163,   156,   157,    11,    88,    70,   248,
      22,   146,    65,   136,   109,   158,    47,    24,     1,    26,
       2,   138,    28,   102,     6,    25,    30,   201,   263,   265,
     275,   169,   170,    62,    63,   103,    68,    69,   134,   177,
      32,   266,   139,   178,     1,   140,     2,     3,     4,     5,
       6,   146,    84,    37,    83,    90,    91,    98,    98,    71,
      96,    53,    55,   122,    47,    32,   111,   112,   113,   114,
     115,   116,   117,   118,   119,    88,    66,   137,   189,   190,
     232,   212,   213,   214,   240,   137,   223,   224,     7,     8,
       9,   202,   264,   264,   264,   184,    54,   135,    56,    36,
     216,     1,   144,     2,     3,     4,     5,     6,     1,   102,
       2,     3,     4,     5,     6,   187,    57,   166,   168,    59,
     185,   143,   228,     2,    60,  -132,   230,     6,   231,  -132,
     182,   250,    33,    34,    35,     1,   267,     2,   232,   271,
     180,     6,   186,    64,    89,     7,     8,     9,    33,    34,
      35,   233,     7,     8,     9,   222,    85,    92,    69,   241,
     252,    33,    34,    35,   144,   119,    38,    39,    40,    67,
     205,   215,    38,    39,    40,    67,    95,   -10,    43,   100,
      65,   125,   120,   121,    43,   109,   130,   206,    33,    34,
      35,   259,    33,    34,    35,   272,    69,  -110,   273,    69,
     124,   126,    21,   131,   132,   242,   127,   268,   147,   233,
     -10,   -10,   -10,   159,    78,   207,   208,   139,   209,   171,
     140,   204,  -110,   181,    33,    34,    35,   183,    33,    34,
      35,    33,    34,    35,    75,    76,    77,    78,   262,   175,
     176,   177,   260,   261,   188,   178,   195,   193,   269,    73,
      74,    75,    76,    77,    78,    79,    80,   194,   172,   101,
      73,    74,    75,    76,    77,    78,    79,    80,   198,   172,
      73,    74,    75,    76,    77,    78,    79,    80,   196,    81,
     173,  -131,    38,    39,    40,    41,   199,   197,    42,   211,
     218,   173,  -131,   219,    43,   221,   229,    44,   245,    73,
      74,    75,    76,    77,    78,    79,    80,   235,    81,    38,
      39,    40,    67,    38,    39,    40,    67,    38,    39,    40,
      67,    43,   237,    82,   203,    43,     2,   243,    44,    43,
       6,   244,   165,    73,    74,    75,    76,    77,    78,    79,
      80,   246,    81,   101,    73,    74,    75,    76,    77,    78,
      79,    80,   251,    81,   148,   149,   150,   151,   254,   152,
     153,   225,   226,   154,    73,    74,    75,    76,    77,    78,
      79,   255,   274,    81,    73,    74,    75,    76,    77,    78,
      38,    39,    40,    81,   148,   149,   150,   151,   256,   152,
     153,   257,   179,   154,   191,   133,   150,   151,   164,   152,
     175,   176,   177,   154,   108,    99,   178,    73,    74,    75,
      76,    77,    78,    38,    39,    40,   161,    38,    39,    40,
      85,    38,    39,    40,   239,   236,   253,   167,   249,   192,
     123,   270
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-211))

#define yytable_value_is_error(yytable_value) \
  ((yytable_value) == (-132))

static const yytype_uint16 yycheck[] =
{
       1,    70,    17,   121,   106,   106,     0,    54,    30,   219,
      33,   103,    18,    17,    32,   109,    23,    53,    18,    30,
      20,    17,    31,     6,    24,     6,     0,    17,    17,    17,
      17,   125,   126,    34,    35,    18,    43,    44,    18,    37,
      80,   251,    26,    41,    18,    29,    20,    21,    22,    23,
      24,   143,    53,    73,    54,    27,    28,    64,    65,    81,
      61,     6,     6,    81,    71,    80,    73,    74,    75,    76,
      77,    78,    79,    80,    81,   122,    82,    81,   147,   148,
     198,   175,   176,   177,   202,    81,   188,   188,    62,    63,
      64,    81,    81,    81,    81,     6,    60,    77,    25,    17,
      17,    18,   103,    20,    21,    22,    23,    24,    18,     6,
      20,    21,    22,    23,    24,    17,    78,   124,   125,    80,
      31,    18,   191,    20,    46,    37,   195,    24,   197,    41,
     137,   225,    50,    51,    52,    18,   254,    20,   256,   257,
     134,    24,   143,    18,    32,    62,    63,    64,    50,    51,
      52,   198,    62,    63,    64,    17,     6,    76,   165,    17,
     229,    50,    51,    52,   165,   172,     3,     4,     5,     6,
     171,   178,     3,     4,     5,     6,    25,    17,    15,     9,
      18,    18,     6,    18,    15,    32,     3,    18,    50,    51,
      52,    17,    50,    51,    52,    17,   203,     6,    17,   206,
      16,    38,   203,     5,     5,   206,    43,   254,    81,   256,
      50,    51,    52,    56,    12,    46,    47,    26,    49,    18,
      29,    17,    31,     3,    50,    51,    52,    24,    50,    51,
      52,    50,    51,    52,     9,    10,    11,    12,   245,    35,
      36,    37,   243,   244,    17,    41,    66,    70,   255,     7,
       8,     9,    10,    11,    12,    13,    14,    70,    16,    17,
       7,     8,     9,    10,    11,    12,    13,    14,    58,    16,
       7,     8,     9,    10,    11,    12,    13,    14,    70,    16,
      38,    39,     3,     4,     5,     6,    55,    66,     9,    39,
      18,    38,    39,    18,    15,     6,    66,    18,    35,     7,
       8,     9,    10,    11,    12,    13,    14,     6,    16,     3,
       4,     5,     6,     3,     4,     5,     6,     3,     4,     5,
       6,    15,    57,    31,    18,    15,    20,    18,    18,    15,
      24,    18,    18,     7,     8,     9,    10,    11,    12,    13,
      14,     6,    16,    17,     7,     8,     9,    10,    11,    12,
      13,    14,    18,    16,    66,    67,    68,    69,    81,    71,
      72,    73,    74,    75,     7,     8,     9,    10,    11,    12,
      13,    16,     6,    16,     7,     8,     9,    10,    11,    12,
       3,     4,     5,    16,    66,    67,    68,    69,    58,    71,
      72,    59,   134,    75,    66,    93,    68,    69,   122,    71,
      35,    36,    37,    75,    71,    65,    41,     7,     8,     9,
      10,    11,    12,     3,     4,     5,     6,     3,     4,     5,
       6,     3,     4,     5,     6,   199,   231,   124,   221,   149,
      86,   256
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    18,    20,    21,    22,    23,    24,    62,    63,    64,
      84,    85,    86,    87,    88,    89,    94,    96,    99,   101,
     103,    87,    33,   104,    53,     6,    30,    95,    31,    90,
       0,    85,    80,    50,    51,    52,    17,    73,     3,     4,
       5,     6,     9,    15,    18,   105,   106,   108,   109,   110,
     111,   112,   113,     6,    60,     6,    25,    78,    91,    80,
      46,   102,    87,    87,    18,    18,    82,     6,   108,   108,
      30,    81,   114,     7,     8,     9,    10,    11,    12,    13,
      14,    16,    31,    54,    87,     6,    97,    98,   110,    32,
      27,    28,    76,    92,    93,    25,    87,   107,   108,   107,
       9,    17,     6,    18,   115,   116,   117,   119,   106,    32,
     125,   108,   108,   108,   108,   108,   108,   108,   108,   108,
       6,    18,    81,   125,    16,    18,    38,    43,   108,   126,
       3,     5,     5,    93,    18,    77,    17,    81,    17,    26,
      29,   122,   123,    18,    87,   116,   119,    81,    66,    67,
      68,    69,    71,    72,    75,   120,   122,   123,   126,    56,
     129,     6,   100,   109,    98,    18,   108,   117,   108,   126,
     126,    18,    16,    38,   128,    35,    36,    37,    41,    84,
      85,     3,   108,    24,     6,    31,    87,    17,    17,   116,
     116,    66,   120,    70,    70,    66,    70,    66,    58,    55,
     130,    17,    81,    18,    17,    87,    18,    46,    47,    49,
     127,    39,   126,   126,   126,   108,    17,    17,    18,    18,
     124,     6,    17,   122,   123,    73,    74,   121,   116,    66,
     116,   116,   109,   110,   133,     6,   113,    57,   131,     6,
     109,    17,    87,    18,    18,    35,     6,   118,   118,   124,
     126,    18,   116,   121,    81,    16,    58,    59,   132,    17,
      87,    87,   108,    17,    81,    17,   118,   109,   110,   108,
     133,   109,    17,    17,     6,    17
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
#line 125 "sql_parser.y"
    { 
				RULELOG("stmtList::stmt"); 
				(yyval.list) = singleton((yyvsp[(1) - (2)].node));
				bisonParseResult = (Node *) (yyval.list);	 
			}
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 131 "sql_parser.y"
    {
				RULELOG("stmtlist::stmtList::stmt");
				(yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(2) - (3)].node));	
				bisonParseResult = (Node *) (yyval.list); 
			}
    break;

  case 4:

/* Line 1806 of yacc.c  */
#line 140 "sql_parser.y"
    {
            RULELOG("stmt::dmlStmt");
            (yyval.node) = (yyvsp[(1) - (1)].node);
        }
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 145 "sql_parser.y"
    {
            RULELOG("stmt::queryStmt");
            (yyval.node) = (yyvsp[(1) - (1)].node);
        }
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 150 "sql_parser.y"
    {
            RULELOG("stmt::transactionIdentifier");
            (yyval.node) = (Node *) createTransactionStmt((yyvsp[(1) - (1)].stringVal));
        }
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 160 "sql_parser.y"
    { RULELOG("dmlStmt::insertQuery"); }
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 161 "sql_parser.y"
    { RULELOG("dmlStmt::deleteQuery"); }
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 162 "sql_parser.y"
    { RULELOG("dmlStmt::updateQuery"); }
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 169 "sql_parser.y"
    { RULELOG("queryStmt::bracketedQuery"); (yyval.node) = (yyvsp[(2) - (3)].node); }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 170 "sql_parser.y"
    { RULELOG("queryStmt::selectQuery"); }
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 171 "sql_parser.y"
    { RULELOG("queryStmt::provStmt"); }
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 172 "sql_parser.y"
    { RULELOG("queryStmt::setOperatorQuery"); }
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 176 "sql_parser.y"
    { RULELOG("transactionIdentifier::BEGIN"); (yyval.stringVal) = strdup("TRANSACTION_BEGIN"); }
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 177 "sql_parser.y"
    { RULELOG("transactionIdentifier::COMMIT"); (yyval.stringVal) = strdup("TRANSACTION_COMMIT"); }
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 178 "sql_parser.y"
    { RULELOG("transactionIdentifier::ROLLBACK"); (yyval.stringVal) = strdup("TRANSACTION_ABORT"); }
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 186 "sql_parser.y"
    {
            RULELOG("provStmt::stmt");
            Node *stmt = (yyvsp[(6) - (7)].node);
	    	ProvenanceStmt *p = createProvenanceStmt(stmt);
		    p->inputType = isQBUpdate(stmt) ? PROV_INPUT_UPDATE : PROV_INPUT_QUERY;
		    p->provType = PROV_PI_CS;
		    p->asOf = (Node *) (yyvsp[(2) - (7)].node);
		    p->options = (yyvsp[(3) - (7)].list);
            (yyval.node) = (Node *) p;
        }
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 197 "sql_parser.y"
    {
			RULELOG("provStmt::stmtlist");
			ProvenanceStmt *p = createProvenanceStmt((Node *) (yyvsp[(6) - (7)].list));
			p->inputType = PROV_INPUT_UPDATE_SEQUENCE;
			p->provType = PROV_PI_CS;
			p->asOf = (Node *) (yyvsp[(2) - (7)].node);
			p->options = (yyvsp[(3) - (7)].list);
			(yyval.node) = (Node *) p;
		}
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 207 "sql_parser.y"
    {
			RULELOG("provStmt::transaction");
			ProvenanceStmt *p = createProvenanceStmt((Node *) createConstInt((yyvsp[(6) - (6)].intVal)));
			p->inputType = PROV_INPUT_TRANSACTION;
			p->provType = PROV_PI_CS;
			p->options = (yyvsp[(3) - (6)].list);
			(yyval.node) = (Node *) p;
		}
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 218 "sql_parser.y"
    { RULELOG("optionalProvAsOf::EMPTY"); (yyval.node) = NULL; }
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 220 "sql_parser.y"
    {
			RULELOG("optionalProvAsOf::SCN");
			(yyval.node) = (Node *) createConstInt((yyvsp[(4) - (4)].intVal));
		}
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 225 "sql_parser.y"
    {
			RULELOG("optionalProvAsOf::TIMESTAMP");
			(yyval.node) = (Node *) createConstString((yyvsp[(4) - (4)].stringVal));
		}
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 232 "sql_parser.y"
    { RULELOG("optionalProvWith::EMPTY"); (yyval.list) = NIL; }
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 234 "sql_parser.y"
    {
			RULELOG("optionalProvWith::WITH");
			(yyval.list) = (yyvsp[(2) - (2)].list);
		}
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 241 "sql_parser.y"
    { RULELOG("provOptionList::option"); (yyval.list) = singleton((yyvsp[(1) - (1)].node)); }
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 243 "sql_parser.y"
    { 
			RULELOG("provOptionList::list"); 
			(yyval.list) = appendToTailOfList((yyvsp[(1) - (2)].list),(yyvsp[(2) - (2)].node)); 
		}
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 251 "sql_parser.y"
    { 
			RULELOG("provOption::TYPE"); 
			(yyval.node) = (Node *) createStringKeyValue("TYPE", (yyvsp[(2) - (2)].stringVal)); 
		}
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 262 "sql_parser.y"
    { 
             RULELOG("deleteQuery");
             (yyval.node) = (Node *) createDelete((yyvsp[(3) - (5)].stringVal), (yyvsp[(5) - (5)].node));
         }
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 270 "sql_parser.y"
    { RULELOG("fromString::NULL"); (yyval.stringVal) = NULL; }
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 271 "sql_parser.y"
    { RULELOG("fromString::FROM"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 286 "sql_parser.y"
    { 
                RULELOG(updateQuery); 
                (yyval.node) = (Node *) createUpdate((yyvsp[(2) - (5)].stringVal), (yyvsp[(4) - (5)].list), (yyvsp[(5) - (5)].node)); 
            }
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 294 "sql_parser.y"
    {
                RULELOG("setClause::setExpression");
                (yyval.list) = singleton((yyvsp[(1) - (1)].node));
            }
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 299 "sql_parser.y"
    {
                RULELOG("setClause::setClause::setExpression");
                (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
            }
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 307 "sql_parser.y"
    {
                if (!strcmp((yyvsp[(2) - (3)].stringVal),"=")) {
                    RULELOG("setExpression::attributeRef::expression");
                    List *expr = singleton((yyvsp[(1) - (3)].node));
                    expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
                }
            }
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 316 "sql_parser.y"
    {
                if (!strcmp((yyvsp[(2) - (3)].stringVal), "=")) {
                    RULELOG("setExpression::attributeRef::queryStmt");
                    List *expr = singleton((yyvsp[(1) - (3)].node));
                    expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
                }
            }
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 331 "sql_parser.y"
    { 
            	RULELOG("insertQuery::insertList"); 
            	(yyval.node) = (Node *) createInsert((yyvsp[(3) - (7)].stringVal),(Node *) (yyvsp[(6) - (7)].list), NULL); 
        	}
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 336 "sql_parser.y"
    { 
                RULELOG("insertQuery::queryStmt");
                (yyval.node) = (Node *) createInsert((yyvsp[(3) - (4)].stringVal), (yyvsp[(4) - (4)].node), NULL);
            }
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 344 "sql_parser.y"
    { 
            	RULELOG("insertList::constant");
            	(yyval.list) = singleton((yyvsp[(1) - (1)].node)); 
            }
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 349 "sql_parser.y"
    {
                RULELOG("insertList::IDENTIFIER");
                (yyval.list) = singleton(createAttributeReference((yyvsp[(1) - (1)].stringVal)));
            }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 354 "sql_parser.y"
    { 
                RULELOG("insertList::insertList::::IDENTIFIER");
                (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), createAttributeReference((yyvsp[(3) - (3)].stringVal)));
            }
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 359 "sql_parser.y"
    { 
            	RULELOG("insertList::insertList::constant");
            	(yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
            }
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 373 "sql_parser.y"
    {
                RULELOG("setOperatorQuery::INTERSECT");
                (yyval.node) = (Node *) createSetQuery((yyvsp[(2) - (3)].stringVal), FALSE, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
            }
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 378 "sql_parser.y"
    {
                RULELOG("setOperatorQuery::MINUS");
                (yyval.node) = (Node *) createSetQuery((yyvsp[(2) - (3)].stringVal), FALSE, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
            }
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 383 "sql_parser.y"
    {
                RULELOG("setOperatorQuery::UNION");
                (yyval.node) = (Node *) createSetQuery((yyvsp[(2) - (4)].stringVal), ((yyvsp[(3) - (4)].stringVal) != NULL), (yyvsp[(1) - (4)].node), (yyvsp[(4) - (4)].node));
            }
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 390 "sql_parser.y"
    { RULELOG("optionalAll::NULL"); (yyval.stringVal) = NULL; }
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 391 "sql_parser.y"
    { RULELOG("optionalAll::ALLTRUE"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 401 "sql_parser.y"
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

  case 48:

/* Line 1806 of yacc.c  */
#line 423 "sql_parser.y"
    { RULELOG("optionalDistinct::NULL"); (yyval.node) = NULL; }
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 425 "sql_parser.y"
    {
                RULELOG("optionalDistinct::DISTINCT");
                (yyval.node) = (Node *) createDistinctClause(NULL);
            }
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 430 "sql_parser.y"
    {
                RULELOG("optionalDistinct::DISTINCT::exprList");
                (yyval.node) = (Node *) createDistinctClause((yyvsp[(4) - (5)].list));
            }
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 442 "sql_parser.y"
    {
                RULELOG("selectClause::selectItem"); (yyval.list) = singleton((yyvsp[(1) - (1)].node));
            }
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 446 "sql_parser.y"
    {
                RULELOG("selectClause::selectClause::selectItem");
                (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node)); 
            }
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 454 "sql_parser.y"
    {
                 RULELOG("selectItem::expression"); 
                 (yyval.node) = (Node *) createSelectItem(NULL, (yyvsp[(1) - (1)].node)); 
             }
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 459 "sql_parser.y"
    {
                 RULELOG("selectItem::expression::identifier"); 
                 (yyval.node) = (Node *) createSelectItem((yyvsp[(3) - (3)].stringVal), (yyvsp[(1) - (3)].node));
             }
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 464 "sql_parser.y"
    { 
         		RULELOG("selectItem::*"); 
         		(yyval.node) = (Node *) createSelectItem(strdup("*"), NULL); 
     		}
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 469 "sql_parser.y"
    { 
         		RULELOG("selectItem::*"); 
     			(yyval.node) = (Node *) createSelectItem(CONCAT_STRINGS((yyvsp[(1) - (3)].stringVal),".*"), NULL); 
 			}
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 479 "sql_parser.y"
    { RULELOG("exprList::SINGLETON"); (yyval.list) = singleton((yyvsp[(1) - (1)].node)); }
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 481 "sql_parser.y"
    {
                  RULELOG("exprList::exprList::expression");
                  (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
             }
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 492 "sql_parser.y"
    { RULELOG("expression::bracked"); (yyval.node) = (yyvsp[(2) - (3)].node); }
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 493 "sql_parser.y"
    { RULELOG("expression::constant"); }
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 494 "sql_parser.y"
    { RULELOG("expression::attributeRef"); }
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 495 "sql_parser.y"
    { RULELOG("expression::binaryOperatorExpression"); }
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 496 "sql_parser.y"
    { RULELOG("expression::unaryOperatorExpression"); }
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 497 "sql_parser.y"
    { RULELOG("expression::sqlFunctionCall"); }
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 506 "sql_parser.y"
    { RULELOG("constant::INT"); (yyval.node) = (Node *) createConstInt((yyvsp[(1) - (1)].intVal)); }
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 507 "sql_parser.y"
    { RULELOG("constant::FLOAT"); (yyval.node) = (Node *) createConstFloat((yyvsp[(1) - (1)].floatVal)); }
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 508 "sql_parser.y"
    { RULELOG("constant::STRING"); (yyval.node) = (Node *) createConstString((yyvsp[(1) - (1)].stringVal)); }
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 515 "sql_parser.y"
    { RULELOG("attributeRef::IDENTIFIER"); (yyval.node) = (Node *) createAttributeReference((yyvsp[(1) - (1)].stringVal)); }
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 535 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '+' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 542 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '-' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 549 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '*' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 556 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '/' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 563 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '%' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 570 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '^' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 579 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '&' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 586 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression:: '|' ");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 77:

/* Line 1806 of yacc.c  */
#line 595 "sql_parser.y"
    {
                RULELOG("binaryOperatorExpression::comparisonOps");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 605 "sql_parser.y"
    {
                RULELOG("unaryOperatorExpression:: '!' ");
                List *expr = singleton((yyvsp[(2) - (2)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(1) - (2)].stringVal), expr);
            }
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 617 "sql_parser.y"
    {
                RULELOG("sqlFunctionCall::IDENTIFIER::exprList"); 
                (yyval.node) = (Node *) createFunctionCall((yyvsp[(1) - (4)].stringVal), (yyvsp[(3) - (4)].list)); 
            }
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 629 "sql_parser.y"
    { RULELOG("optionalFrom::NULL"); (yyval.list) = NULL; }
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 630 "sql_parser.y"
    { RULELOG("optionalFrom::fromClause"); (yyval.list) = (yyvsp[(2) - (2)].list); }
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 635 "sql_parser.y"
    {
                RULELOG("fromClause::fromClauseItem");
                (yyval.list) = singleton((yyvsp[(1) - (1)].node));
            }
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 640 "sql_parser.y"
    {
                RULELOG("fromClause::fromClause::fromClauseItem");
                (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
            }
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 649 "sql_parser.y"
    {
                RULELOG("fromClauseItem");
				FromItem *f = createFromTableRef(NULL, NIL, (yyvsp[(1) - (2)].stringVal));
				f->provInfo = (FromProvInfo *) (yyvsp[(2) - (2)].node);
                (yyval.node) = (Node *) f;
            }
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 656 "sql_parser.y"
    {
                RULELOG("fromClauseItem");
                FromItem *f = createFromTableRef(((FromItem *) (yyvsp[(2) - (2)].node))->name, 
						((FromItem *) (yyvsp[(2) - (2)].node))->attrNames, (yyvsp[(1) - (2)].stringVal));
				f->provInfo = ((FromItem *) (yyvsp[(2) - (2)].node))->provInfo;
                (yyval.node) = (Node *) f;
            }
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 665 "sql_parser.y"
    {
                RULELOG("fromClauseItem::subQuery");
                FromItem *f = (FromItem *) (yyvsp[(1) - (2)].node);
                f->provInfo = (FromProvInfo *) (yyvsp[(2) - (2)].node);
                (yyval.node) = (yyvsp[(1) - (2)].node);
            }
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 672 "sql_parser.y"
    {
                RULELOG("fromClauseItem::subQuery");
                FromSubquery *s = (FromSubquery *) (yyvsp[(1) - (2)].node);
                s->from.name = ((FromItem *) (yyvsp[(2) - (2)].node))->name;
                s->from.attrNames = ((FromItem *) (yyvsp[(2) - (2)].node))->attrNames;
                s->from.provInfo = ((FromItem *) (yyvsp[(2) - (2)].node))->provInfo;
                (yyval.node) = (Node *) s;
            }
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 681 "sql_parser.y"
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[(1) - (1)].node);
        		f->name = NULL;
        		(yyval.node) = (Node *) f;
        	}
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 689 "sql_parser.y"
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[(2) - (4)].node);
        		f->name = ((FromItem *) (yyvsp[(4) - (4)].node))->name;
                f->attrNames = ((FromItem *) (yyvsp[(4) - (4)].node))->attrNames;
                f->provInfo = ((FromItem *) (yyvsp[(4) - (4)].node))->provInfo;
        		(yyval.node) = (Node *) f;
        	}
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 702 "sql_parser.y"
    {
                RULELOG("subQuery::queryStmt");
                (yyval.node) = (Node *) createFromSubquery(NULL, NULL, (yyvsp[(2) - (3)].node));
            }
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 709 "sql_parser.y"
    { (yyval.list) = singleton((yyvsp[(1) - (1)].stringVal)); }
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 710 "sql_parser.y"
    { (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].stringVal)); }
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 714 "sql_parser.y"
    { (yyval.node) = (yyvsp[(2) - (3)].node); }
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 716 "sql_parser.y"
    {
                RULELOG("fromJoinItem::NATURAL");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[(1) - (4)].node), 
						(FromItem *) (yyvsp[(4) - (4)].node), "JOIN_INNER", "JOIN_COND_NATURAL", 
						NULL);
          	}
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 723 "sql_parser.y"
    {
                RULELOG("fromJoinItem::NATURALjoinType");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[(1) - (5)].node), 
                		(FromItem *) (yyvsp[(5) - (5)].node), (yyvsp[(3) - (5)].stringVal), "JOIN_COND_NATURAL", NULL);
          	}
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 729 "sql_parser.y"
    {
				RULELOG("fromJoinItem::CROSS JOIN");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[(1) - (4)].node), 
                		(FromItem *) (yyvsp[(4) - (4)].node), "JOIN_CROSS", "JOIN_COND_ON", NULL);
          	}
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 735 "sql_parser.y"
    {
				RULELOG("fromJoinItem::JOIN::joinType::joinCond");
				char *condType = (isA((yyvsp[(5) - (5)].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON";
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[(1) - (5)].node), 
                		(FromItem *) (yyvsp[(4) - (5)].node), (yyvsp[(2) - (5)].stringVal), condType, (yyvsp[(5) - (5)].node));
          	}
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 743 "sql_parser.y"
    {
				RULELOG("fromJoinItem::JOIN::joinCond");
				char *condType = (isA((yyvsp[(4) - (4)].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON"; 
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[(1) - (4)].node), 
                		(FromItem *) (yyvsp[(3) - (4)].node), "JOIN_INNER", 
                		condType, (yyvsp[(4) - (4)].node));
          	}
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 754 "sql_parser.y"
    { RULELOG("joinType::LEFT"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 755 "sql_parser.y"
    { RULELOG("joinType::LEFT OUTER"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 756 "sql_parser.y"
    { RULELOG("joinType::RIGHT "); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 757 "sql_parser.y"
    { RULELOG("joinType::RIGHT OUTER"); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 758 "sql_parser.y"
    { RULELOG("joinType::FULL OUTER"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 759 "sql_parser.y"
    { RULELOG("joinType::FULL"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 760 "sql_parser.y"
    { RULELOG("joinType::INNER"); (yyval.stringVal) = "JOIN_INNER"; }
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 764 "sql_parser.y"
    { (yyval.node) = (Node *) (yyvsp[(3) - (4)].list); }
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 765 "sql_parser.y"
    { (yyval.node) = (yyvsp[(2) - (2)].node); }
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 770 "sql_parser.y"
    {
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[(2) - (3)].stringVal),(yyvsp[(3) - (3)].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[(1) - (3)].node);
				(yyval.node) = (Node *) f;
			}
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 777 "sql_parser.y"
    { 
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[(3) - (4)].stringVal),(yyvsp[(4) - (4)].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[(1) - (4)].node); 
				(yyval.node) = (Node *) f;
			}
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 786 "sql_parser.y"
    { RULELOG("optionalFromProv::empty"); (yyval.node) = NULL; }
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 788 "sql_parser.y"
    {
				RULELOG("optionalFromProv");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = TRUE;
				p->userProvAttrs = NIL;				 
				(yyval.node) = (Node *) p; 
			}
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 796 "sql_parser.y"
    {
				RULELOG("optionalFromProv::userProvAttr");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = FALSE;
				p->userProvAttrs = (yyvsp[(4) - (5)].list);				 
				(yyval.node) = (Node *) p; 
			}
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 806 "sql_parser.y"
    { RULELOG("optionalAttrAlias::empty"); (yyval.list) = NULL; }
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 808 "sql_parser.y"
    { 
				RULELOG("optionalAttrAlias::identifierList"); (yyval.list) = (yyvsp[(2) - (3)].list); 
			}
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 817 "sql_parser.y"
    { RULELOG("optionalWhere::NULL"); (yyval.node) = NULL; }
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 818 "sql_parser.y"
    { RULELOG("optionalWhere::whereExpression"); (yyval.node) = (yyvsp[(2) - (2)].node); }
    break;

  case 117:

/* Line 1806 of yacc.c  */
#line 822 "sql_parser.y"
    { RULELOG("where::brackedWhereExpression"); (yyval.node) = (yyvsp[(2) - (3)].node); }
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 823 "sql_parser.y"
    { RULELOG("whereExpression::expression"); (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 825 "sql_parser.y"
    {
                RULELOG("whereExpression::NOT");
                List *expr = singleton((yyvsp[(2) - (2)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(1) - (2)].stringVal), expr);
            }
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 831 "sql_parser.y"
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 838 "sql_parser.y"
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 845 "sql_parser.y"
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[(1) - (3)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (3)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (3)].stringVal), expr);
            }
    break;

  case 123:

/* Line 1806 of yacc.c  */
#line 852 "sql_parser.y"
    {
                RULELOG("whereExpression::BETWEEN-AND");
                List *expr = singleton((yyvsp[(1) - (5)].node));
                expr = appendToTailOfList(expr, (yyvsp[(3) - (5)].node));
                expr = appendToTailOfList(expr, (yyvsp[(5) - (5)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (5)].stringVal), expr);
            }
    break;

  case 124:

/* Line 1806 of yacc.c  */
#line 860 "sql_parser.y"
    {
                RULELOG("whereExpression::comparisonOps::nestedSubQueryOperator::Subquery");
                (yyval.node) = (Node *) createNestedSubquery((yyvsp[(3) - (6)].stringVal), (yyvsp[(1) - (6)].node), (yyvsp[(2) - (6)].stringVal), (yyvsp[(5) - (6)].node));
            }
    break;

  case 125:

/* Line 1806 of yacc.c  */
#line 865 "sql_parser.y"
    {
                RULELOG("whereExpression::comparisonOps::Subquery");
                List *expr = singleton((yyvsp[(1) - (5)].node));
                expr = appendToTailOfList(expr, (yyvsp[(4) - (5)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(2) - (5)].stringVal), expr);
            }
    break;

  case 126:

/* Line 1806 of yacc.c  */
#line 872 "sql_parser.y"
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

  case 127:

/* Line 1806 of yacc.c  */
#line 885 "sql_parser.y"
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

  case 128:

/* Line 1806 of yacc.c  */
#line 900 "sql_parser.y"
    { RULELOG("nestedSubQueryOperator::ANY"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 129:

/* Line 1806 of yacc.c  */
#line 901 "sql_parser.y"
    { RULELOG("nestedSubQueryOperator::ALL"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 130:

/* Line 1806 of yacc.c  */
#line 902 "sql_parser.y"
    { RULELOG("nestedSubQueryOperator::SOME"); (yyval.stringVal) = "ANY"; }
    break;

  case 131:

/* Line 1806 of yacc.c  */
#line 906 "sql_parser.y"
    { RULELOG("optionalNot::NULL"); (yyval.stringVal) = NULL; }
    break;

  case 132:

/* Line 1806 of yacc.c  */
#line 907 "sql_parser.y"
    { RULELOG("optionalNot::NOT"); (yyval.stringVal) = (yyvsp[(1) - (1)].stringVal); }
    break;

  case 133:

/* Line 1806 of yacc.c  */
#line 911 "sql_parser.y"
    { RULELOG("optionalGroupBy::NULL"); (yyval.list) = NULL; }
    break;

  case 134:

/* Line 1806 of yacc.c  */
#line 912 "sql_parser.y"
    { RULELOG("optionalGroupBy::GROUPBY"); (yyval.list) = (yyvsp[(3) - (3)].list); }
    break;

  case 135:

/* Line 1806 of yacc.c  */
#line 916 "sql_parser.y"
    { RULELOG("optionalOrderBy:::NULL"); (yyval.node) = NULL; }
    break;

  case 136:

/* Line 1806 of yacc.c  */
#line 918 "sql_parser.y"
    { 
                RULELOG("optionalHaving::HAVING"); 
                List *expr = singleton((yyvsp[(2) - (4)].node));
                expr = appendToTailOfList(expr, (yyvsp[(4) - (4)].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[(3) - (4)].stringVal), expr);
            }
    break;

  case 137:

/* Line 1806 of yacc.c  */
#line 927 "sql_parser.y"
    { RULELOG("optionalOrderBy:::NULL"); (yyval.list) = NULL; }
    break;

  case 138:

/* Line 1806 of yacc.c  */
#line 928 "sql_parser.y"
    { RULELOG("optionalOrderBy::ORDERBY"); (yyval.list) = (yyvsp[(3) - (3)].list); }
    break;

  case 139:

/* Line 1806 of yacc.c  */
#line 932 "sql_parser.y"
    { RULELOG("optionalLimit::NULL"); (yyval.node) = NULL; }
    break;

  case 140:

/* Line 1806 of yacc.c  */
#line 933 "sql_parser.y"
    { RULELOG("optionalLimit::CONSTANT"); (yyval.node) = (yyvsp[(2) - (2)].node);}
    break;

  case 141:

/* Line 1806 of yacc.c  */
#line 938 "sql_parser.y"
    {
                RULELOG("clauseList::attributeRef");
                (yyval.list) = singleton((yyvsp[(1) - (1)].node));
            }
    break;

  case 142:

/* Line 1806 of yacc.c  */
#line 943 "sql_parser.y"
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
            }
    break;

  case 143:

/* Line 1806 of yacc.c  */
#line 948 "sql_parser.y"
    {
                RULELOG("clauseList::constant");
                (yyval.list) = singleton((yyvsp[(1) - (1)].node));
            }
    break;

  case 144:

/* Line 1806 of yacc.c  */
#line 953 "sql_parser.y"
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
            }
    break;



/* Line 1806 of yacc.c  */
#line 3118 "sql_parser.tab.c"
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
#line 960 "sql_parser.y"




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


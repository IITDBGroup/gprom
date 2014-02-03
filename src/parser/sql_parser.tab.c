/* A Bison parser, made by GNU Bison 3.0.1.  */

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
#define YYBISON_VERSION "3.0.1"

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
    parameter = 262,
    comparisonOps = 263,
    SELECT = 264,
    INSERT = 265,
    UPDATE = 266,
    DELETE = 267,
    PROVENANCE = 268,
    OF = 269,
    BASERELATION = 270,
    SCN = 271,
    TIMESTAMP = 272,
    HAS = 273,
    FROM = 274,
    AS = 275,
    WHERE = 276,
    DISTINCT = 277,
    STARALL = 278,
    AND = 279,
    OR = 280,
    LIKE = 281,
    NOT = 282,
    IN = 283,
    ISNULL = 284,
    BETWEEN = 285,
    EXCEPT = 286,
    EXISTS = 287,
    AMMSC = 288,
    NULLVAL = 289,
    ALL = 290,
    ANY = 291,
    IS = 292,
    SOME = 293,
    UNION = 294,
    INTERSECT = 295,
    MINUS = 296,
    INTO = 297,
    VALUES = 298,
    HAVING = 299,
    GROUP = 300,
    ORDER = 301,
    BY = 302,
    LIMIT = 303,
    SET = 304,
    INT = 305,
    BEGIN_TRANS = 306,
    COMMIT_TRANS = 307,
    ROLLBACK_TRANS = 308,
    DUMMYEXPR = 309,
    JOIN = 310,
    NATURAL = 311,
    LEFT = 312,
    RIGHT = 313,
    OUTER = 314,
    INNER = 315,
    CROSS = 316,
    ON = 317,
    USING = 318,
    FULL = 319,
    TYPE = 320,
    TRANSACTION = 321,
    WITH = 322,
    XOR = 323
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

#line 207 "sql_parser.tab.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SQL_PARSER_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 222 "sql_parser.tab.c" /* yacc.c:358  */

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
#define YYFINAL  30
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   437

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  84
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  52
/* YYNRULES -- Number of rules.  */
#define YYNRULES  146
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  278

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   323

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    16,     2,     2,     2,    12,    14,     2,
      19,    18,    10,     8,    82,     9,    83,    11,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    81,
       2,    20,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    13,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    15,     2,     2,     2,     2,     2,
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
       5,     6,     7,    17,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   125,   125,   131,   140,   145,   150,   161,   162,   163,
     170,   171,   172,   173,   177,   178,   179,   186,   197,   207,
     219,   220,   225,   233,   234,   242,   243,   251,   262,   271,
     272,   286,   294,   299,   307,   316,   331,   336,   344,   349,
     354,   359,   373,   378,   383,   391,   392,   401,   424,   425,
     430,   442,   446,   454,   459,   464,   469,   480,   481,   493,
     494,   495,   496,   497,   498,   499,   508,   509,   510,   517,
     523,   542,   549,   556,   563,   570,   577,   586,   593,   602,
     612,   624,   637,   638,   642,   647,   656,   663,   672,   679,
     688,   696,   709,   717,   718,   722,   723,   730,   736,   742,
     750,   762,   763,   764,   765,   766,   767,   768,   772,   773,
     777,   784,   794,   795,   803,   814,   815,   825,   826,   830,
     831,   832,   838,   845,   852,   859,   867,   872,   879,   892,
     908,   909,   910,   914,   915,   919,   920,   924,   925,   935,
     936,   940,   941,   945,   950,   955,   960
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "intConst", "floatConst", "stringConst",
  "identifier", "parameter", "'+'", "'-'", "'*'", "'/'", "'%'", "'^'",
  "'&'", "'|'", "'!'", "comparisonOps", "')'", "'('", "'='", "SELECT",
  "INSERT", "UPDATE", "DELETE", "PROVENANCE", "OF", "BASERELATION", "SCN",
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
  "expression", "constant", "attributeRef", "sqlParameter",
  "binaryOperatorExpression", "unaryOperatorExpression", "sqlFunctionCall",
  "optionalFrom", "fromClause", "fromClauseItem", "subQuery",
  "identifierList", "fromJoinItem", "joinType", "joinCond",
  "optionalAlias", "optionalFromProv", "optionalAttrAlias",
  "optionalWhere", "whereExpression", "nestedSubQueryOperator",
  "optionalNot", "optionalGroupBy", "optionalHaving", "optionalOrderBy",
  "optionalLimit", "clauseList", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,    43,    45,
      42,    47,    37,    94,    38,   124,    33,   263,    41,    40,
      61,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,    59,    44,    46
};
# endif

#define YYPACT_NINF -212

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-212)))

#define YYTABLE_NINF -134

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-134)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     154,    -3,     6,     7,    32,    69,    70,  -212,  -212,  -212,
      26,    25,  -212,   130,  -212,  -212,  -212,  -212,  -212,  -212,
    -212,    45,    46,   332,   113,    66,  -212,   131,   119,    72,
    -212,    75,  -212,   110,    -3,    -3,  -212,   142,  -212,  -212,
    -212,    -8,  -212,  -212,   313,   313,   -22,  -212,   291,  -212,
    -212,  -212,  -212,  -212,  -212,    -2,   157,   134,    -4,   115,
     148,  -212,  -212,    -3,  -212,  -212,   313,   313,   185,   145,
     383,   352,    97,   332,   169,   313,   313,   313,   313,   313,
     313,   313,   313,   313,   197,   186,   130,  -212,   -26,  -212,
     189,   181,   206,   207,   211,   115,  -212,     8,  -212,    -5,
     363,     3,  -212,  -212,    80,   309,   129,   334,    80,  -212,
    -212,   181,   164,   284,   284,   213,   213,   213,  -212,   383,
     276,   403,  -212,   414,   157,  -212,   340,   181,   181,   222,
     252,   316,  -212,  -212,  -212,  -212,   154,   227,  -212,   313,
    -212,  -212,   218,  -212,    89,   309,    81,   334,   229,    97,
      97,   170,   212,   236,  -212,   190,   243,   201,  -212,    89,
     316,   219,   226,  -212,    10,  -212,  -212,   306,   363,  -212,
     241,    87,   316,    -3,   143,  -212,   281,   181,   181,   181,
     313,    90,   -16,  -212,   363,   305,   307,   327,   117,  -212,
      14,   334,   314,    97,   273,  -212,  -212,    97,  -212,    97,
     418,   335,   292,  -212,   422,   306,  -212,   162,   306,  -212,
    -212,  -212,   330,   336,     1,     1,    17,   262,  -212,  -212,
     351,   351,  -212,   307,   171,  -212,    89,   181,   349,  -212,
    -212,    97,  -212,   314,  -212,  -212,   297,   145,   368,   338,
     339,  -212,  -212,  -212,   176,    -3,    -3,   313,  -212,    11,
      12,  -212,   316,   351,  -212,  -212,   418,   313,   418,   138,
    -212,  -212,   180,   183,   363,  -212,   392,  -212,    19,  -212,
    -212,   363,   297,  -212,  -212,  -212,  -212,  -212
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    48,     0,     0,    29,    20,    14,    15,    16,
       0,     0,     4,     5,     6,    12,     8,     9,     7,    13,
      11,     0,    49,     0,     0,     0,    30,     0,     0,    23,
       1,     0,     2,    45,     0,     0,    10,     0,    66,    67,
      68,    69,    70,    55,     0,     0,    82,    51,    53,    60,
      61,    62,    63,    64,    65,     0,     0,     0,     0,     0,
       0,     3,    46,     0,    42,    43,     0,     0,     0,    69,
      80,     0,     0,     0,   117,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    37,    69,   117,    32,
       0,     0,     0,     0,     0,    24,    25,     0,    44,     0,
      57,     0,    56,    59,   112,     0,    83,    84,   112,    90,
      52,     0,   135,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    54,     0,     0,    31,     0,     0,     0,     0,
     120,    28,    21,    22,    27,    26,     0,     0,    50,     0,
      81,   113,     0,    87,    86,     0,     0,     0,    90,     0,
       0,     0,   101,   103,   107,     0,   106,     0,    89,    88,
     118,     0,   137,    39,     0,    38,    33,     0,    34,    35,
     120,     0,   121,     0,     0,   134,     0,     0,     0,     0,
       0,     0,     0,    19,    58,     0,   115,     0,     0,    92,
      95,    85,     0,     0,     0,   102,   104,     0,   105,     0,
       0,     0,   139,    36,     0,     0,   119,     0,     0,   131,
     130,   132,     0,     0,   122,   123,   124,     0,    18,    17,
       0,     0,   110,   115,    92,    91,     0,     0,     0,   100,
      96,     0,    98,     0,   145,   143,   136,     0,     0,     0,
     141,    40,    41,   129,     0,     0,     0,     0,    93,     0,
       0,   111,   109,     0,    97,    99,     0,     0,     0,     0,
      47,   127,     0,     0,   125,   114,     0,   116,     0,   146,
     144,   138,   140,   142,   126,   128,    94,   108
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -212,   269,     4,  -212,    -1,  -212,  -212,  -212,  -212,  -212,
     337,  -212,  -212,  -212,  -212,   285,  -212,  -212,  -212,  -212,
    -212,  -212,  -212,   356,   341,    -9,  -120,   -48,  -212,  -212,
    -212,   230,  -212,  -212,   -71,   304,  -211,   -93,   282,   202,
    -103,  -102,   214,   346,   -96,  -212,  -212,  -212,  -212,  -212,
    -212,   178
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    10,    31,    12,    13,    14,    15,    29,    60,    95,
      96,    16,    27,    17,    88,    89,    18,   164,    19,    63,
      20,    23,    46,    47,    99,   130,    49,    50,    51,    52,
      53,    54,    74,   106,   147,   108,   249,   109,   157,   229,
     143,   144,   222,   112,   131,   212,   176,   162,   202,   240,
     260,   236
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      21,   107,   219,   165,    11,   158,   159,   111,    90,    72,
     250,    67,   148,   138,    48,   160,     1,     1,     2,     2,
    -112,   140,     6,     6,    92,    93,    30,   136,   203,   265,
     267,   171,   172,    64,    65,    70,    71,   277,    25,   179,
      22,   141,   268,   180,   142,     1,  -112,     2,     3,     4,
       5,     6,   148,    85,    86,  -134,   124,   100,   100,  -134,
      73,    24,    98,    36,    48,    32,   113,   114,   115,   116,
     117,   118,   119,   120,   121,    68,    90,   139,   191,   192,
     234,   214,   215,   216,   242,   139,   137,   225,   226,     7,
       8,     9,   204,   266,   266,   186,    33,    34,    35,   189,
      26,   266,    28,   104,   146,   206,    32,   141,   218,     1,
     142,     2,     3,     4,     5,     6,   105,   168,   170,    55,
      37,   187,   230,   177,   178,   179,   232,    56,   233,   180,
     184,   252,    33,    34,    35,   224,   269,    57,   234,   273,
     182,    38,    39,    40,   188,    58,    38,    39,    40,    69,
      42,    59,   235,     7,     8,     9,    61,    62,    71,    44,
     254,    66,   208,    87,    67,   121,   146,    91,    33,    34,
      35,   217,   207,     1,    97,     2,     3,     4,     5,     6,
     243,    33,    34,    35,    38,    39,    40,    69,    42,   -10,
     209,   210,    94,   211,   261,   102,    71,    44,   274,    71,
     127,   275,   111,   122,    21,   123,   126,   244,   270,   132,
     235,   149,   133,    33,    34,    35,   134,     7,     8,     9,
     128,   161,   -10,   -10,   -10,   129,    80,    33,    34,    35,
     183,    33,    34,    35,    33,    34,    35,   193,   264,   152,
     153,   173,   154,   185,   262,   263,   156,   190,   271,    75,
      76,    77,    78,    79,    80,    81,    82,   197,   174,   103,
      75,    76,    77,    78,    79,    80,    81,    82,   199,   174,
      75,    76,    77,    78,    79,    80,    81,    82,   200,    83,
     175,  -133,   201,   195,    75,    76,    77,    78,    79,    80,
      81,   175,  -133,    83,    77,    78,    79,    80,   247,    75,
      76,    77,    78,    79,    80,    81,    82,   196,    83,    38,
      39,    40,    69,    42,   198,   104,    38,    39,    40,    69,
      42,   213,    44,    84,   220,   205,   221,     2,   145,    44,
       2,     6,    45,   223,     6,    38,    39,    40,    41,    42,
     231,   237,    43,    38,    39,    40,    69,    42,    44,   245,
     239,    45,   177,   178,   179,   246,    44,   248,   180,   167,
      75,    76,    77,    78,    79,    80,    81,    82,   253,    83,
     103,    75,    76,    77,    78,    79,    80,    81,    82,   256,
      83,   150,   151,   152,   153,   257,   154,   155,   227,   228,
     156,    75,    76,    77,    78,    79,    80,   258,   276,   259,
      83,   150,   151,   152,   153,   181,   154,   155,   101,   166,
     156,    75,    76,    77,    78,    79,    80,    38,    39,    40,
     163,    38,    39,    40,    87,    38,    39,    40,   241,   110,
     169,   238,   135,   194,   125,   255,   272,   251
};

static const yytype_uint16 yycheck[] =
{
       1,    72,    18,   123,     0,   108,   108,    33,    56,    31,
     221,    19,   105,    18,    23,   111,    19,    19,    21,    21,
       6,    18,    25,    25,    28,    29,     0,    19,    18,    18,
      18,   127,   128,    34,    35,    44,    45,    18,     6,    38,
      34,    27,   253,    42,    30,    19,    32,    21,    22,    23,
      24,    25,   145,    55,    55,    38,    82,    66,    67,    42,
      82,    54,    63,    18,    73,    81,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    83,   124,    82,   149,   150,
     200,   177,   178,   179,   204,    82,    78,   190,   190,    63,
      64,    65,    82,    82,    82,     6,    51,    52,    53,    18,
      31,    82,    32,     6,   105,    18,    81,    27,    18,    19,
      30,    21,    22,    23,    24,    25,    19,   126,   127,     6,
      74,    32,   193,    36,    37,    38,   197,    61,   199,    42,
     139,   227,    51,    52,    53,    18,   256,     6,   258,   259,
     136,     3,     4,     5,   145,    26,     3,     4,     5,     6,
       7,    79,   200,    63,    64,    65,    81,    47,   167,    16,
     231,    19,    19,     6,    19,   174,   167,    33,    51,    52,
      53,   180,   173,    19,    26,    21,    22,    23,    24,    25,
      18,    51,    52,    53,     3,     4,     5,     6,     7,    18,
      47,    48,    77,    50,    18,    10,   205,    16,    18,   208,
      19,    18,    33,     6,   205,    19,    17,   208,   256,     3,
     258,    82,     5,    51,    52,    53,     5,    63,    64,    65,
      39,    57,    51,    52,    53,    44,    13,    51,    52,    53,
       3,    51,    52,    53,    51,    52,    53,    67,   247,    69,
      70,    19,    72,    25,   245,   246,    76,    18,   257,     8,
       9,    10,    11,    12,    13,    14,    15,    67,    17,    18,
       8,     9,    10,    11,    12,    13,    14,    15,    67,    17,
       8,     9,    10,    11,    12,    13,    14,    15,    59,    17,
      39,    40,    56,    71,     8,     9,    10,    11,    12,    13,
      14,    39,    40,    17,    10,    11,    12,    13,    36,     8,
       9,    10,    11,    12,    13,    14,    15,    71,    17,     3,
       4,     5,     6,     7,    71,     6,     3,     4,     5,     6,
       7,    40,    16,    32,    19,    19,    19,    21,    19,    16,
      21,    25,    19,     6,    25,     3,     4,     5,     6,     7,
      67,     6,    10,     3,     4,     5,     6,     7,    16,    19,
      58,    19,    36,    37,    38,    19,    16,     6,    42,    19,
       8,     9,    10,    11,    12,    13,    14,    15,    19,    17,
      18,     8,     9,    10,    11,    12,    13,    14,    15,    82,
      17,    67,    68,    69,    70,    17,    72,    73,    74,    75,
      76,     8,     9,    10,    11,    12,    13,    59,     6,    60,
      17,    67,    68,    69,    70,   136,    72,    73,    67,   124,
      76,     8,     9,    10,    11,    12,    13,     3,     4,     5,
       6,     3,     4,     5,     6,     3,     4,     5,     6,    73,
     126,   201,    95,   151,    88,   233,   258,   223
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    19,    21,    22,    23,    24,    25,    63,    64,    65,
      85,    86,    87,    88,    89,    90,    95,    97,   100,   102,
     104,    88,    34,   105,    54,     6,    31,    96,    32,    91,
       0,    86,    81,    51,    52,    53,    18,    74,     3,     4,
       5,     6,     7,    10,    16,    19,   106,   107,   109,   110,
     111,   112,   113,   114,   115,     6,    61,     6,    26,    79,
      92,    81,    47,   103,    88,    88,    19,    19,    83,     6,
     109,   109,    31,    82,   116,     8,     9,    10,    11,    12,
      13,    14,    15,    17,    32,    55,    88,     6,    98,    99,
     111,    33,    28,    29,    77,    93,    94,    26,    88,   108,
     109,   108,    10,    18,     6,    19,   117,   118,   119,   121,
     107,    33,   127,   109,   109,   109,   109,   109,   109,   109,
     109,   109,     6,    19,    82,   127,    17,    19,    39,    44,
     109,   128,     3,     5,     5,    94,    19,    78,    18,    82,
      18,    27,    30,   124,   125,    19,    88,   118,   121,    82,
      67,    68,    69,    70,    72,    73,    76,   122,   124,   125,
     128,    57,   131,     6,   101,   110,    99,    19,   109,   119,
     109,   128,   128,    19,    17,    39,   130,    36,    37,    38,
      42,    85,    86,     3,   109,    25,     6,    32,    88,    18,
      18,   118,   118,    67,   122,    71,    71,    67,    71,    67,
      59,    56,   132,    18,    82,    19,    18,    88,    19,    47,
      48,    50,   129,    40,   128,   128,   128,   109,    18,    18,
      19,    19,   126,     6,    18,   124,   125,    74,    75,   123,
     118,    67,   118,   118,   110,   111,   135,     6,   115,    58,
     133,     6,   110,    18,    88,    19,    19,    36,     6,   120,
     120,   126,   128,    19,   118,   123,    82,    17,    59,    60,
     134,    18,    88,    88,   109,    18,    82,    18,   120,   110,
     111,   109,   135,   110,    18,    18,     6,    18
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    84,    85,    85,    86,    86,    86,    87,    87,    87,
      88,    88,    88,    88,    89,    89,    89,    90,    90,    90,
      91,    91,    91,    92,    92,    93,    93,    94,    95,    96,
      96,    97,    98,    98,    99,    99,   100,   100,   101,   101,
     101,   101,   102,   102,   102,   103,   103,   104,   105,   105,
     105,   106,   106,   107,   107,   107,   107,   108,   108,   109,
     109,   109,   109,   109,   109,   109,   110,   110,   110,   111,
     112,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     114,   115,   116,   116,   117,   117,   118,   118,   118,   118,
     118,   118,   119,   120,   120,   121,   121,   121,   121,   121,
     121,   122,   122,   122,   122,   122,   122,   122,   123,   123,
     124,   124,   125,   125,   125,   126,   126,   127,   127,   128,
     128,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     129,   129,   129,   130,   130,   131,   131,   132,   132,   133,
     133,   134,   134,   135,   135,   135,   135
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     3,     1,     1,     1,     1,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     7,     7,     6,
       0,     4,     4,     0,     2,     1,     2,     2,     5,     0,
       1,     5,     1,     3,     3,     3,     7,     4,     1,     1,
       3,     3,     3,     3,     4,     0,     1,     9,     0,     1,
       5,     1,     3,     1,     3,     1,     3,     1,     3,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     4,     0,     2,     1,     3,     2,     2,     2,     2,
       1,     4,     3,     1,     3,     3,     4,     5,     4,     5,
       4,     1,     2,     1,     2,     2,     1,     1,     4,     2,
       3,     4,     0,     1,     5,     0,     3,     0,     2,     3,
       1,     2,     3,     3,     3,     5,     6,     5,     6,     4,
       1,     1,     1,     0,     1,     0,     3,     0,     4,     0,
       3,     0,     2,     1,     3,     1,     3
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
#line 126 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("stmtList::stmt"); 
				(yyval.list) = singleton((yyvsp[-1].node));
				bisonParseResult = (Node *) (yyval.list);	 
			}
#line 1548 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 132 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("stmtlist::stmtList::stmt");
				(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[-1].node));	
				bisonParseResult = (Node *) (yyval.list); 
			}
#line 1558 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 141 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::dmlStmt");
            (yyval.node) = (yyvsp[0].node);
        }
#line 1567 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 146 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::queryStmt");
            (yyval.node) = (yyvsp[0].node);
        }
#line 1576 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 151 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::transactionIdentifier");
            (yyval.node) = (Node *) createTransactionStmt((yyvsp[0].stringVal));
        }
#line 1585 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 161 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::insertQuery"); }
#line 1591 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 162 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::deleteQuery"); }
#line 1597 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 163 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::updateQuery"); }
#line 1603 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 170 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::bracketedQuery"); (yyval.node) = (yyvsp[-1].node); }
#line 1609 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 171 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::selectQuery"); }
#line 1615 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 172 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::provStmt"); }
#line 1621 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 173 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::setOperatorQuery"); }
#line 1627 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 177 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::BEGIN"); (yyval.stringVal) = strdup("TRANSACTION_BEGIN"); }
#line 1633 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 178 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::COMMIT"); (yyval.stringVal) = strdup("TRANSACTION_COMMIT"); }
#line 1639 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 179 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::ROLLBACK"); (yyval.stringVal) = strdup("TRANSACTION_ABORT"); }
#line 1645 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 187 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("provStmt::stmt");
            Node *stmt = (yyvsp[-1].node);
	    	ProvenanceStmt *p = createProvenanceStmt(stmt);
		    p->inputType = isQBUpdate(stmt) ? PROV_INPUT_UPDATE : PROV_INPUT_QUERY;
		    p->provType = PROV_PI_CS;
		    p->asOf = (Node *) (yyvsp[-5].node);
		    p->options = (yyvsp[-4].list);
            (yyval.node) = (Node *) p;
        }
#line 1660 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 198 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provStmt::stmtlist");
			ProvenanceStmt *p = createProvenanceStmt((Node *) (yyvsp[-1].list));
			p->inputType = PROV_INPUT_UPDATE_SEQUENCE;
			p->provType = PROV_PI_CS;
			p->asOf = (Node *) (yyvsp[-5].node);
			p->options = (yyvsp[-4].list);
			(yyval.node) = (Node *) p;
		}
#line 1674 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 208 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provStmt::transaction");
			ProvenanceStmt *p = createProvenanceStmt((Node *) createConstInt((yyvsp[0].intVal)));
			p->inputType = PROV_INPUT_TRANSACTION;
			p->provType = PROV_PI_CS;
			p->options = (yyvsp[-3].list);
			(yyval.node) = (Node *) p;
		}
#line 1687 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 219 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalProvAsOf::EMPTY"); (yyval.node) = NULL; }
#line 1693 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 221 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvAsOf::SCN");
			(yyval.node) = (Node *) createConstInt((yyvsp[0].intVal));
		}
#line 1702 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 226 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvAsOf::TIMESTAMP");
			(yyval.node) = (Node *) createConstString((yyvsp[0].stringVal));
		}
#line 1711 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 233 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalProvWith::EMPTY"); (yyval.list) = NIL; }
#line 1717 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 235 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvWith::WITH");
			(yyval.list) = (yyvsp[0].list);
		}
#line 1726 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 242 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("provOptionList::option"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 1732 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 244 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("provOptionList::list"); 
			(yyval.list) = appendToTailOfList((yyvsp[-1].list),(yyvsp[0].node)); 
		}
#line 1741 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 252 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("provOption::TYPE"); 
			(yyval.node) = (Node *) createStringKeyValue("TYPE", (yyvsp[0].stringVal)); 
		}
#line 1750 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 263 "sql_parser.y" /* yacc.c:1646  */
    { 
             RULELOG("deleteQuery");
             (yyval.node) = (Node *) createDelete((yyvsp[-2].stringVal), (yyvsp[0].node));
         }
#line 1759 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 271 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::NULL"); (yyval.stringVal) = NULL; }
#line 1765 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 272 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::FROM"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 1771 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 287 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG(updateQuery); 
                (yyval.node) = (Node *) createUpdate((yyvsp[-3].stringVal), (yyvsp[-1].list), (yyvsp[0].node)); 
            }
#line 1780 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 295 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setExpression");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 1789 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 300 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setClause::setExpression");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 1798 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 308 "sql_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal),"=")) {
                    RULELOG("setExpression::attributeRef::expression");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 1811 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 317 "sql_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal), "=")) {
                    RULELOG("setExpression::attributeRef::queryStmt");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 1824 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 332 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertQuery::insertList"); 
            	(yyval.node) = (Node *) createInsert((yyvsp[-4].stringVal),(Node *) (yyvsp[-1].list), NULL); 
        	}
#line 1833 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 337 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("insertQuery::queryStmt");
                (yyval.node) = (Node *) createInsert((yyvsp[-1].stringVal), (yyvsp[0].node), NULL);
            }
#line 1842 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 345 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::constant");
            	(yyval.list) = singleton((yyvsp[0].node)); 
            }
#line 1851 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 350 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("insertList::IDENTIFIER");
                (yyval.list) = singleton(createAttributeReference((yyvsp[0].stringVal)));
            }
#line 1860 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 355 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("insertList::insertList::::IDENTIFIER");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), createAttributeReference((yyvsp[0].stringVal)));
            }
#line 1869 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 360 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::insertList::constant");
            	(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 1878 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 374 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::INTERSECT");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 1887 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 379 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::MINUS");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 1896 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 384 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::UNION");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-2].stringVal), ((yyvsp[-1].stringVal) != NULL), (yyvsp[-3].node), (yyvsp[0].node));
            }
#line 1905 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 391 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::NULL"); (yyval.stringVal) = NULL; }
#line 1911 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 392 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::ALLTRUE"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 1917 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 402 "sql_parser.y" /* yacc.c:1646  */
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
#line 1937 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 424 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalDistinct::NULL"); (yyval.node) = NULL; }
#line 1943 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 426 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT");
                (yyval.node) = (Node *) createDistinctClause(NULL);
            }
#line 1952 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 431 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT::exprList");
                (yyval.node) = (Node *) createDistinctClause((yyvsp[-1].list));
            }
#line 1961 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 443 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectItem"); (yyval.list) = singleton((yyvsp[0].node));
            }
#line 1969 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 447 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectClause::selectItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node)); 
            }
#line 1978 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 455 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression"); 
                 (yyval.node) = (Node *) createSelectItem(NULL, (yyvsp[0].node)); 
             }
#line 1987 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 460 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression::identifier"); 
                 (yyval.node) = (Node *) createSelectItem((yyvsp[0].stringVal), (yyvsp[-2].node));
             }
#line 1996 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 465 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
         		(yyval.node) = (Node *) createSelectItem(strdup("*"), NULL); 
     		}
#line 2005 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 470 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
     			(yyval.node) = (Node *) createSelectItem(CONCAT_STRINGS((yyvsp[-2].stringVal),".*"), NULL); 
 			}
#line 2014 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 480 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("exprList::SINGLETON"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 2020 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 482 "sql_parser.y" /* yacc.c:1646  */
    {
                  RULELOG("exprList::exprList::expression");
                  (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
             }
#line 2029 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 493 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::bracked"); (yyval.node) = (yyvsp[-1].node); }
#line 2035 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 494 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::constant"); }
#line 2041 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 495 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::attributeRef"); }
#line 2047 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 496 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlParameter"); }
#line 2053 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 497 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::binaryOperatorExpression"); }
#line 2059 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 498 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::unaryOperatorExpression"); }
#line 2065 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 499 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlFunctionCall"); }
#line 2071 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 508 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::INT"); (yyval.node) = (Node *) createConstInt((yyvsp[0].intVal)); }
#line 2077 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 509 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::FLOAT"); (yyval.node) = (Node *) createConstFloat((yyvsp[0].floatVal)); }
#line 2083 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 510 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::STRING"); (yyval.node) = (Node *) createConstString((yyvsp[0].stringVal)); }
#line 2089 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 517 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("attributeRef::IDENTIFIER"); (yyval.node) = (Node *) createAttributeReference((yyvsp[0].stringVal)); }
#line 2095 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 523 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("sqlParameter::PARAMETER"); (yyval.node) = (Node *) createSQLParameter((yyvsp[0].stringVal)); }
#line 2101 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 543 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '+' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2112 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 550 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '-' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2123 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 557 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '*' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2134 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 564 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '/' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2145 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 571 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '%' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2156 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 578 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '^' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2167 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 587 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '&' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2178 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 594 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '|' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2189 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 603 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression::comparisonOps");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2200 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 613 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("unaryOperatorExpression:: '!' ");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2210 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 625 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("sqlFunctionCall::IDENTIFIER::exprList"); 
                (yyval.node) = (Node *) createFunctionCall((yyvsp[-3].stringVal), (yyvsp[-1].list)); 
            }
#line 2219 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 637 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::NULL"); (yyval.list) = NULL; }
#line 2225 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 638 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::fromClause"); (yyval.list) = (yyvsp[0].list); }
#line 2231 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 643 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClauseItem");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2240 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 648 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClause::fromClauseItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2249 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 657 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
				FromItem *f = createFromTableRef(NULL, NIL, (yyvsp[-1].stringVal));
				f->provInfo = (FromProvInfo *) (yyvsp[0].node);
                (yyval.node) = (Node *) f;
            }
#line 2260 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 664 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
                FromItem *f = createFromTableRef(((FromItem *) (yyvsp[0].node))->name, 
						((FromItem *) (yyvsp[0].node))->attrNames, (yyvsp[-1].stringVal));
				f->provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
                (yyval.node) = (Node *) f;
            }
#line 2272 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 673 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromItem *f = (FromItem *) (yyvsp[-1].node);
                f->provInfo = (FromProvInfo *) (yyvsp[0].node);
                (yyval.node) = (yyvsp[-1].node);
            }
#line 2283 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 680 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromSubquery *s = (FromSubquery *) (yyvsp[-1].node);
                s->from.name = ((FromItem *) (yyvsp[0].node))->name;
                s->from.attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
                s->from.provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
                (yyval.node) = (Node *) s;
            }
#line 2296 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 689 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[0].node);
        		f->name = NULL;
        		(yyval.node) = (Node *) f;
        	}
#line 2308 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 697 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[-2].node);
        		f->name = ((FromItem *) (yyvsp[0].node))->name;
                f->attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
                f->provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
        		(yyval.node) = (Node *) f;
        	}
#line 2322 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 710 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("subQuery::queryStmt");
                (yyval.node) = (Node *) createFromSubquery(NULL, NULL, (yyvsp[-1].node));
            }
#line 2331 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 717 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = singleton((yyvsp[0].stringVal)); }
#line 2337 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 718 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].stringVal)); }
#line 2343 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 722 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2349 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 724 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURAL");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
						(FromItem *) (yyvsp[0].node), "JOIN_INNER", "JOIN_COND_NATURAL", 
						NULL);
          	}
#line 2360 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 731 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURALjoinType");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[0].node), (yyvsp[-2].stringVal), "JOIN_COND_NATURAL", NULL);
          	}
#line 2370 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 737 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::CROSS JOIN");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[0].node), "JOIN_CROSS", "JOIN_COND_ON", NULL);
          	}
#line 2380 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 743 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinType::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON";
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[-1].node), (yyvsp[-3].stringVal), condType, (yyvsp[0].node));
          	}
#line 2392 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 751 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON"; 
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[-1].node), "JOIN_INNER", 
                		condType, (yyvsp[0].node));
          	}
#line 2405 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 762 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2411 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 763 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT OUTER"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2417 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 764 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT "); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2423 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 765 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT OUTER"); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2429 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 766 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL OUTER"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2435 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 767 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2441 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 768 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::INNER"); (yyval.stringVal) = "JOIN_INNER"; }
#line 2447 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 772 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (Node *) (yyvsp[-1].list); }
#line 2453 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 773 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2459 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 778 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[-2].node);
				(yyval.node) = (Node *) f;
			}
#line 2470 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 785 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[-3].node); 
				(yyval.node) = (Node *) f;
			}
#line 2481 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 794 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFromProv::empty"); (yyval.node) = NULL; }
#line 2487 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 796 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = TRUE;
				p->userProvAttrs = NIL;				 
				(yyval.node) = (Node *) p; 
			}
#line 2499 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 804 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::userProvAttr");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = FALSE;
				p->userProvAttrs = (yyvsp[-1].list);				 
				(yyval.node) = (Node *) p; 
			}
#line 2511 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 814 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAttrAlias::empty"); (yyval.list) = NULL; }
#line 2517 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 816 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAttrAlias::identifierList"); (yyval.list) = (yyvsp[-1].list); 
			}
#line 2525 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 825 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::NULL"); (yyval.node) = NULL; }
#line 2531 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 826 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::whereExpression"); (yyval.node) = (yyvsp[0].node); }
#line 2537 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 830 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("where::brackedWhereExpression"); (yyval.node) = (yyvsp[-1].node); }
#line 2543 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 831 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("whereExpression::expression"); (yyval.node) = (yyvsp[0].node); }
#line 2549 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 833 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::NOT");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2559 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 839 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2570 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 846 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2581 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 853 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2592 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 860 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::BETWEEN-AND");
                List *expr = singleton((yyvsp[-4].node));
                expr = appendToTailOfList(expr, (yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr);
            }
#line 2604 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 868 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::comparisonOps::nestedSubQueryOperator::Subquery");
                (yyval.node) = (Node *) createNestedSubquery((yyvsp[-3].stringVal), (yyvsp[-5].node), (yyvsp[-4].stringVal), (yyvsp[-1].node));
            }
#line 2613 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 873 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::comparisonOps::Subquery");
                List *expr = singleton((yyvsp[-4].node));
                expr = appendToTailOfList(expr, (yyvsp[-1].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr);
            }
#line 2624 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 880 "sql_parser.y" /* yacc.c:1646  */
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
#line 2641 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 893 "sql_parser.y" /* yacc.c:1646  */
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
#line 2658 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 908 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ANY"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2664 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 909 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ALL"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2670 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 910 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::SOME"); (yyval.stringVal) = "ANY"; }
#line 2676 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 914 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NULL"); (yyval.stringVal) = NULL; }
#line 2682 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 915 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NOT"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2688 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 919 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::NULL"); (yyval.list) = NULL; }
#line 2694 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 920 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::GROUPBY"); (yyval.list) = (yyvsp[0].list); }
#line 2700 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 924 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.node) = NULL; }
#line 2706 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 926 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("optionalHaving::HAVING"); 
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2717 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 935 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.list) = NULL; }
#line 2723 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 936 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy::ORDERBY"); (yyval.list) = (yyvsp[0].list); }
#line 2729 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 940 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::NULL"); (yyval.node) = NULL; }
#line 2735 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 941 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::CONSTANT"); (yyval.node) = (yyvsp[0].node);}
#line 2741 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 946 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::attributeRef");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2750 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 951 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2759 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 956 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::constant");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2768 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 961 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2777 "sql_parser.tab.c" /* yacc.c:1646  */
    break;


#line 2781 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 968 "sql_parser.y" /* yacc.c:1906  */




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

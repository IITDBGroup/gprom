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
    TABLE = 274,
    FROM = 275,
    AS = 276,
    WHERE = 277,
    DISTINCT = 278,
    STARALL = 279,
    AND = 280,
    OR = 281,
    LIKE = 282,
    NOT = 283,
    IN = 284,
    ISNULL = 285,
    BETWEEN = 286,
    EXCEPT = 287,
    EXISTS = 288,
    AMMSC = 289,
    NULLVAL = 290,
    ALL = 291,
    ANY = 292,
    IS = 293,
    SOME = 294,
    UNION = 295,
    INTERSECT = 296,
    MINUS = 297,
    INTO = 298,
    VALUES = 299,
    HAVING = 300,
    GROUP = 301,
    ORDER = 302,
    BY = 303,
    LIMIT = 304,
    SET = 305,
    INT = 306,
    BEGIN_TRANS = 307,
    COMMIT_TRANS = 308,
    ROLLBACK_TRANS = 309,
    CASE = 310,
    WHEN = 311,
    THEN = 312,
    ELSE = 313,
    END = 314,
    OVER_TOK = 315,
    PARTITION = 316,
    ROWS = 317,
    RANGE = 318,
    UNBOUNDED = 319,
    PRECEDING = 320,
    CURRENT = 321,
    ROW = 322,
    FOLLOWING = 323,
    DUMMYEXPR = 324,
    JOIN = 325,
    NATURAL = 326,
    LEFT = 327,
    RIGHT = 328,
    OUTER = 329,
    INNER = 330,
    CROSS = 331,
    ON = 332,
    USING = 333,
    FULL = 334,
    TYPE = 335,
    TRANSACTION = 336,
    WITH = 337,
    XOR = 338
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

#line 222 "sql_parser.tab.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SQL_PARSER_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 237 "sql_parser.tab.c" /* yacc.c:358  */

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
#define YYLAST   539

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  99
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  62
/* YYNRULES -- Number of rules.  */
#define YYNRULES  169
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  321

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   338

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
      19,    18,    10,     8,    97,     9,    98,    11,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    96,
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
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   128,   128,   134,   143,   148,   153,   164,   165,   166,
     173,   174,   175,   176,   180,   181,   182,   189,   200,   210,
     222,   223,   228,   236,   237,   245,   246,   254,   259,   270,
     279,   280,   294,   302,   307,   315,   324,   339,   344,   352,
     357,   362,   367,   381,   386,   391,   399,   400,   409,   432,
     433,   438,   450,   454,   462,   467,   472,   477,   488,   489,
     501,   502,   503,   504,   505,   506,   507,   508,   517,   518,
     519,   526,   532,   551,   558,   565,   572,   579,   586,   595,
     602,   611,   621,   633,   648,   653,   661,   666,   674,   682,
     683,   694,   695,   703,   711,   712,   720,   721,   730,   742,
     747,   755,   760,   765,   770,   783,   784,   788,   793,   802,
     809,   818,   825,   834,   842,   855,   863,   864,   868,   869,
     876,   882,   888,   896,   908,   909,   910,   911,   912,   913,
     914,   918,   919,   923,   930,   940,   941,   949,   960,   961,
     971,   972,   976,   977,   978,   984,   991,   998,  1005,  1013,
    1018,  1025,  1038,  1054,  1055,  1056,  1060,  1061,  1065,  1066,
    1070,  1071,  1079,  1080,  1084,  1085,  1089,  1094,  1099,  1104
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
  "TIMESTAMP", "HAS", "TABLE", "FROM", "AS", "WHERE", "DISTINCT",
  "STARALL", "AND", "OR", "LIKE", "NOT", "IN", "ISNULL", "BETWEEN",
  "EXCEPT", "EXISTS", "AMMSC", "NULLVAL", "ALL", "ANY", "IS", "SOME",
  "UNION", "INTERSECT", "MINUS", "INTO", "VALUES", "HAVING", "GROUP",
  "ORDER", "BY", "LIMIT", "SET", "INT", "BEGIN_TRANS", "COMMIT_TRANS",
  "ROLLBACK_TRANS", "CASE", "WHEN", "THEN", "ELSE", "END", "OVER_TOK",
  "PARTITION", "ROWS", "RANGE", "UNBOUNDED", "PRECEDING", "CURRENT", "ROW",
  "FOLLOWING", "DUMMYEXPR", "JOIN", "NATURAL", "LEFT", "RIGHT", "OUTER",
  "INNER", "CROSS", "ON", "USING", "FULL", "TYPE", "TRANSACTION", "WITH",
  "XOR", "';'", "','", "'.'", "$accept", "stmtList", "stmt", "dmlStmt",
  "queryStmt", "transactionIdentifier", "provStmt", "optionalProvAsOf",
  "optionalProvWith", "provOptionList", "provOption", "deleteQuery",
  "fromString", "updateQuery", "setClause", "setExpression", "insertQuery",
  "insertList", "setOperatorQuery", "optionalAll", "selectQuery",
  "optionalDistinct", "selectClause", "selectItem", "exprList",
  "expression", "constant", "attributeRef", "sqlParameter",
  "binaryOperatorExpression", "unaryOperatorExpression", "sqlFunctionCall",
  "caseExpression", "caseWhenList", "caseWhen", "optionalCaseElse",
  "overClause", "windowSpec", "optWindowPart", "optWindowFrame",
  "windowBoundaries", "windowBound", "optionalFrom", "fromClause",
  "fromClauseItem", "subQuery", "identifierList", "fromJoinItem",
  "joinType", "joinCond", "optionalAlias", "optionalFromProv",
  "optionalAttrAlias", "optionalWhere", "whereExpression",
  "nestedSubQueryOperator", "optionalNot", "optionalGroupBy",
  "optionalHaving", "optionalOrderBy", "optionalLimit", "clauseList", YY_NULLPTR
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
     323,   324,   325,   326,   327,   328,   329,   330,   331,   332,
     333,   334,   335,   336,   337,   338,    59,    44,    46
};
# endif

#define YYPACT_NINF -277

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-277)))

#define YYTABLE_NINF -157

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-157)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     343,   180,    18,   -14,    49,    25,    56,  -277,  -277,  -277,
     169,    31,  -277,    60,  -277,  -277,  -277,  -277,  -277,  -277,
    -277,   129,    46,   310,   101,    80,  -277,   144,   132,    65,
    -277,    71,  -277,   136,   180,   180,  -277,   158,  -277,  -277,
    -277,   -13,  -277,  -277,   279,   279,   159,   -30,  -277,   456,
    -277,  -277,  -277,  -277,  -277,  -277,  -277,    85,   179,   153,
       3,   -15,   171,  -277,  -277,   180,  -277,  -277,   279,   279,
     190,   183,   504,   473,   279,   330,    62,  -277,    23,   310,
     170,   279,   279,   279,   279,   279,   279,   279,   279,   279,
     206,   197,    60,  -277,   -22,  -277,   200,   261,   219,   220,
     222,   227,   -15,  -277,    -5,  -277,   -10,   484,    -3,  -277,
    -277,   228,    62,   279,  -277,   198,    98,   453,   154,   440,
      98,  -277,  -277,   261,   196,   237,   237,   248,   248,   248,
    -277,   504,   441,   370,  -277,   253,   179,  -277,   318,   261,
     261,   251,   416,   133,  -277,  -277,  -277,  -277,  -277,   343,
     267,  -277,   279,   201,   279,   207,   484,  -277,  -277,   254,
    -277,    14,   453,   161,   440,   269,    23,    23,   448,   202,
     208,  -277,   221,   218,   230,  -277,    14,   133,   258,   262,
    -277,    -1,  -277,  -277,   286,   484,  -277,   405,   356,   133,
     180,   204,  -277,   295,   261,   261,   261,   279,   336,     9,
    -277,   484,   308,  -277,   484,  -277,   344,   353,   367,   177,
    -277,   127,   440,   420,    23,   293,  -277,  -277,    23,  -277,
      23,   279,   261,   325,  -277,   327,   286,  -277,   256,   286,
    -277,  -277,  -277,   377,   378,    -9,    -9,    11,   426,  -277,
    -277,   316,  -277,   397,   397,  -277,   353,   317,  -277,    14,
     261,   385,  -277,  -277,    23,  -277,   420,   309,   133,   361,
     371,  -277,  -277,  -277,   334,   180,   180,   279,   382,   325,
    -277,     0,     1,  -277,   133,   397,  -277,  -277,   345,   134,
    -277,  -277,   338,   358,   484,   279,    78,  -277,   399,  -277,
       6,  -277,  -277,   347,  -277,  -277,  -277,   309,    33,    33,
     429,  -277,  -277,   345,    67,   383,   369,   109,  -277,  -277,
    -277,  -277,  -277,  -277,   424,  -277,  -277,  -277,  -277,    67,
    -277
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    49,     0,     0,    30,    20,    14,    15,    16,
       0,     0,     4,     5,     6,    12,     8,     9,     7,    13,
      11,     0,    50,     0,     0,     0,    31,     0,     0,    23,
       1,     0,     2,    46,     0,     0,    10,     0,    68,    69,
      70,    71,    72,    56,     0,     0,     0,   105,    52,    54,
      61,    62,    63,    64,    65,    66,    67,     0,     0,     0,
       0,     0,     0,     3,    47,     0,    43,    44,     0,     0,
       0,    71,    82,     0,     0,     0,    89,    87,     0,     0,
     140,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    38,    71,   140,    33,     0,     0,     0,     0,
       0,     0,    24,    25,     0,    45,     0,    58,     0,    57,
      60,     0,    89,     0,    86,     0,   135,     0,   106,   107,
     135,   113,    53,     0,   158,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    55,     0,     0,    32,     0,     0,
       0,     0,   143,    29,    21,    22,    28,    27,    26,     0,
       0,    51,     0,    91,     0,     0,    90,    85,   136,     0,
     110,   109,     0,     0,     0,   113,     0,     0,     0,   124,
     126,   130,     0,   129,     0,   112,   111,   141,     0,   160,
      40,     0,    39,    34,     0,    35,    36,   143,     0,   144,
       0,     0,   157,     0,     0,     0,     0,     0,     0,     0,
      19,    59,     0,    83,    88,    84,     0,   138,     0,     0,
     115,   118,   108,     0,     0,     0,   125,   127,     0,   128,
       0,     0,     0,   162,    37,     0,     0,   142,     0,     0,
     154,   153,   155,     0,     0,   145,   146,   147,     0,    18,
      17,    94,    92,     0,     0,   133,   138,   115,   114,     0,
       0,     0,   123,   119,     0,   121,     0,   159,   161,     0,
     164,    41,    42,   152,     0,     0,     0,     0,     0,   162,
     116,     0,     0,   134,   132,     0,   120,   122,     0,     0,
      48,   150,     0,     0,   148,     0,    96,   137,     0,   139,
       0,   168,   166,   163,   165,   149,   151,    95,     0,     0,
       0,   117,   131,     0,     0,     0,     0,     0,    97,   100,
      98,    93,   169,   167,     0,   101,   102,   103,   104,     0,
      99
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -277,   313,     7,  -277,    34,  -277,  -277,  -277,  -277,  -277,
     373,  -277,  -277,  -277,  -277,   340,  -277,  -277,  -277,  -277,
    -277,  -277,  -277,   398,   -66,   -23,  -130,   -57,  -277,  -277,
    -277,  -277,  -277,   404,   -28,   368,  -277,  -277,  -277,  -277,
     235,  -276,  -277,  -277,   -74,   362,  -231,   -84,   350,   250,
    -110,  -109,   273,   432,  -114,  -277,  -277,  -277,  -277,   260,
    -277,  -277
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    10,    31,    12,    13,    14,    15,    29,    62,   102,
     103,    16,    27,    17,    94,    95,    18,   181,    19,    65,
      20,    23,    47,    48,   106,   142,    50,    51,    52,    53,
      54,    55,    56,    76,    77,   115,   203,   242,   269,   300,
     308,   309,    80,   118,   164,   120,   271,   121,   174,   252,
     160,   161,   245,   124,   143,   233,   193,   179,   223,   260,
     280,   293
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      49,    96,    78,   108,   119,   182,    69,    11,   151,   177,
     175,   176,   123,   272,   149,   153,   100,   224,   287,   289,
     207,    72,    73,    75,   302,   188,   189,   240,   314,   116,
     196,    98,    99,   165,   197,    21,    38,    39,    40,    71,
      42,    24,   117,   320,   290,   107,   107,   208,   114,    44,
    -157,   111,    45,    22,  -157,    25,    49,    26,   125,   126,
     127,   128,   129,   130,   131,   132,   133,    79,    66,    67,
      38,    39,    40,    71,    42,   136,   304,   101,   165,    96,
     235,   236,   237,    44,   114,    70,    45,   152,   150,    28,
     156,    92,   212,   213,   152,   262,   225,   288,   288,   105,
      46,   248,   249,   288,     1,    32,     2,    57,   258,   305,
       6,   306,    33,    34,    35,   185,   187,    81,    82,    83,
      84,    85,    86,    87,    88,   158,    89,    32,   159,   201,
      74,   204,   113,  -135,    46,    37,   274,    38,    39,    40,
     253,    91,    58,   305,   255,   306,   256,    36,   291,   294,
      59,   163,   298,   299,   158,   257,   199,   159,    60,    61,
    -135,    73,    38,    39,    40,    71,    42,    63,   133,    30,
     194,   195,   196,   312,   238,    44,   197,    68,    45,   210,
     276,    33,    34,    35,    64,    93,   317,    97,     1,   318,
       2,     3,     4,     5,     6,   247,   209,   104,   107,     1,
     109,     2,    69,    73,   123,     6,    73,    38,    39,    40,
      71,    42,   134,    33,    34,    35,   135,   138,   163,   297,
      44,   292,   144,   229,   228,   145,    46,    74,   146,    33,
      34,    35,   147,     7,     8,     9,    81,    82,    83,    84,
      85,    86,    87,    88,   284,    89,   313,    83,    84,    85,
      86,   166,   230,   231,   178,   232,    38,    39,    40,   180,
      21,    86,   107,   264,    38,    39,    40,    71,    42,   157,
     190,    46,   200,   202,   263,   307,   307,    44,   205,   206,
     139,   307,    38,    39,    40,    71,    42,   211,   216,    38,
      39,    40,    71,    42,   217,    44,   307,   154,    45,   282,
     283,   140,    44,   218,   219,   226,   141,     2,    33,    34,
      35,     6,   220,    38,    39,    40,    41,    42,   221,   222,
      43,    38,    39,    40,    71,    42,    44,   241,    46,    45,
      38,    39,    40,   261,    44,   -10,   234,   184,    81,    82,
      83,    84,    85,    86,    87,    88,    46,    89,    38,    39,
      40,    93,   281,    46,   239,     1,   295,     2,     3,     4,
       5,     6,     1,   243,     2,     3,     4,     5,     6,   -10,
     -10,   -10,   244,   246,   227,   254,   296,    46,    81,    82,
      83,    84,    85,    86,   259,    46,    33,    34,    35,   268,
      33,    34,    35,   194,   195,   196,   265,   266,    74,   197,
       7,     8,     9,   270,   275,   301,   152,     7,     8,     9,
      33,    34,    35,    81,    82,    83,    84,    85,    86,    87,
      88,   278,   191,   110,    81,    82,    83,    84,    85,    86,
      87,    88,   279,   191,    81,    82,    83,    84,    85,    86,
      87,    88,   285,    89,   303,   192,  -156,   311,   316,    81,
      82,    83,    84,    85,    86,    87,   192,  -156,    89,   116,
     315,   319,   198,   267,    81,    82,    83,    84,    85,    86,
      87,    88,   162,    89,     2,   148,   183,   122,     6,   112,
     155,    81,    82,    83,    84,    85,    86,    87,    88,    90,
      89,   110,    81,    82,    83,    84,    85,    86,    87,    88,
     186,    89,   167,   168,   169,   170,   277,   171,   172,   250,
     251,   173,    81,    82,    83,    84,    85,    86,   215,   273,
       0,    89,   167,   168,   169,   170,   137,   171,   172,   286,
     214,   173,   169,   170,   310,   171,     0,     0,     0,   173
};

static const yytype_int16 yycheck[] =
{
      23,    58,    32,    69,    78,   135,    19,     0,    18,   123,
     120,   120,    34,   244,    19,    18,    31,    18,    18,    18,
       6,    44,    45,    46,    18,   139,   140,    18,   304,     6,
      39,    28,    29,   117,    43,     1,     3,     4,     5,     6,
       7,    55,    19,   319,   275,    68,    69,    33,    76,    16,
      39,    74,    19,    35,    43,     6,    79,    32,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    97,    34,    35,
       3,     4,     5,     6,     7,    97,    43,    92,   162,   136,
     194,   195,   196,    16,   112,    98,    19,    97,    93,    33,
     113,    57,   166,   167,    97,   225,    97,    97,    97,    65,
      67,   211,   211,    97,    19,    96,    21,     6,   222,    76,
      25,    78,    52,    53,    54,   138,   139,     8,     9,    10,
      11,    12,    13,    14,    15,    27,    17,    96,    30,   152,
      68,   154,    70,     6,    67,    89,   250,     3,     4,     5,
     214,    56,    62,    76,   218,    78,   220,    18,   278,   279,
       6,   117,    74,    75,    27,   221,   149,    30,    26,    94,
      33,   184,     3,     4,     5,     6,     7,    96,   191,     0,
      37,    38,    39,   303,   197,    16,    43,    19,    19,    18,
     254,    52,    53,    54,    48,     6,    77,    34,    19,    80,
      21,    22,    23,    24,    25,    18,   162,    26,   221,    19,
      10,    21,    19,   226,    34,    25,   229,     3,     4,     5,
       6,     7,     6,    52,    53,    54,    19,    17,   184,   285,
      16,   278,     3,    19,   190,     5,    67,    68,     6,    52,
      53,    54,     5,    64,    65,    66,     8,     9,    10,    11,
      12,    13,    14,    15,   267,    17,   303,    10,    11,    12,
      13,    97,    48,    49,    58,    51,     3,     4,     5,     6,
     226,    13,   285,   229,     3,     4,     5,     6,     7,    71,
      19,    67,     5,    72,    18,   298,   299,    16,    71,    25,
      19,   304,     3,     4,     5,     6,     7,    18,    86,     3,
       4,     5,     6,     7,    86,    16,   319,    69,    19,   265,
     266,    40,    16,    82,    86,    19,    45,    21,    52,    53,
      54,    25,    82,     3,     4,     5,     6,     7,    60,    57,
      10,     3,     4,     5,     6,     7,    16,    19,    67,    19,
       3,     4,     5,     6,    16,    18,    41,    19,     8,     9,
      10,    11,    12,    13,    14,    15,    67,    17,     3,     4,
       5,     6,    18,    67,    18,    19,    18,    21,    22,    23,
      24,    25,    19,    19,    21,    22,    23,    24,    25,    52,
      53,    54,    19,     6,    18,    82,    18,    67,     8,     9,
      10,    11,    12,    13,    59,    67,    52,    53,    54,    73,
      52,    53,    54,    37,    38,    39,    19,    19,    68,    43,
      64,    65,    66,     6,    19,     6,    97,    64,    65,    66,
      52,    53,    54,     8,     9,    10,    11,    12,    13,    14,
      15,    60,    17,    18,     8,     9,    10,    11,    12,    13,
      14,    15,    61,    17,     8,     9,    10,    11,    12,    13,
      14,    15,    60,    17,    97,    40,    41,    18,    79,     8,
       9,    10,    11,    12,    13,    14,    40,    41,    17,     6,
      77,    37,   149,    37,     8,     9,    10,    11,    12,    13,
      14,    15,    19,    17,    21,   102,   136,    79,    25,    75,
     112,     8,     9,    10,    11,    12,    13,    14,    15,    33,
      17,    18,     8,     9,    10,    11,    12,    13,    14,    15,
     138,    17,    82,    83,    84,    85,   256,    87,    88,    89,
      90,    91,     8,     9,    10,    11,    12,    13,   168,   246,
      -1,    17,    82,    83,    84,    85,    94,    87,    88,   269,
      82,    91,    84,    85,   299,    87,    -1,    -1,    -1,    91
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    19,    21,    22,    23,    24,    25,    64,    65,    66,
     100,   101,   102,   103,   104,   105,   110,   112,   115,   117,
     119,   103,    35,   120,    55,     6,    32,   111,    33,   106,
       0,   101,    96,    52,    53,    54,    18,    89,     3,     4,
       5,     6,     7,    10,    16,    19,    67,   121,   122,   124,
     125,   126,   127,   128,   129,   130,   131,     6,    62,     6,
      26,    94,   107,    96,    48,   118,   103,   103,    19,    19,
      98,     6,   124,   124,    68,   124,   132,   133,    32,    97,
     141,     8,     9,    10,    11,    12,    13,    14,    15,    17,
      33,    56,   103,     6,   113,   114,   126,    34,    28,    29,
      31,    92,   108,   109,    26,   103,   123,   124,   123,    10,
      18,   124,   132,    70,   133,   134,     6,    19,   142,   143,
     144,   146,   122,    34,   152,   124,   124,   124,   124,   124,
     124,   124,   124,   124,     6,    19,    97,   152,    17,    19,
      40,    45,   124,   153,     3,     5,     6,     5,   109,    19,
      93,    18,    97,    18,    69,   134,   124,    71,    27,    30,
     149,   150,    19,   103,   143,   146,    97,    82,    83,    84,
      85,    87,    88,    91,   147,   149,   150,   153,    58,   156,
       6,   116,   125,   114,    19,   124,   144,   124,   153,   153,
      19,    17,    40,   155,    37,    38,    39,    43,   100,   101,
       5,   124,    72,   135,   124,    71,    25,     6,    33,   103,
      18,    18,   143,   143,    82,   147,    86,    86,    82,    86,
      82,    60,    57,   157,    18,    97,    19,    18,   103,    19,
      48,    49,    51,   154,    41,   153,   153,   153,   124,    18,
      18,    19,   136,    19,    19,   151,     6,    18,   149,   150,
      89,    90,   148,   143,    82,   143,   143,   123,   153,    59,
     158,     6,   125,    18,   103,    19,    19,    37,    73,   137,
       6,   145,   145,   151,   153,    19,   143,   148,    60,    61,
     159,    18,   103,   103,   124,    60,   158,    18,    97,    18,
     145,   125,   126,   160,   125,    18,    18,   123,    74,    75,
     138,     6,    18,    97,    43,    76,    78,   124,   139,   140,
     139,    18,   125,   126,   140,    77,    79,    77,    80,    37,
     140
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    99,   100,   100,   101,   101,   101,   102,   102,   102,
     103,   103,   103,   103,   104,   104,   104,   105,   105,   105,
     106,   106,   106,   107,   107,   108,   108,   109,   109,   110,
     111,   111,   112,   113,   113,   114,   114,   115,   115,   116,
     116,   116,   116,   117,   117,   117,   118,   118,   119,   120,
     120,   120,   121,   121,   122,   122,   122,   122,   123,   123,
     124,   124,   124,   124,   124,   124,   124,   124,   125,   125,
     125,   126,   127,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   129,   130,   131,   131,   132,   132,   133,   134,
     134,   135,   135,   136,   137,   137,   138,   138,   138,   139,
     139,   140,   140,   140,   140,   141,   141,   142,   142,   143,
     143,   143,   143,   143,   143,   144,   145,   145,   146,   146,
     146,   146,   146,   146,   147,   147,   147,   147,   147,   147,
     147,   148,   148,   149,   149,   150,   150,   150,   151,   151,
     152,   152,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   154,   154,   154,   155,   155,   156,   156,
     157,   157,   158,   158,   159,   159,   160,   160,   160,   160
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     3,     1,     1,     1,     1,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     7,     7,     6,
       0,     4,     4,     0,     2,     1,     2,     2,     2,     5,
       0,     1,     5,     1,     3,     3,     3,     7,     4,     1,
       1,     3,     3,     3,     3,     4,     0,     1,     9,     0,
       1,     5,     1,     3,     1,     3,     1,     3,     1,     3,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     5,     5,     4,     2,     1,     4,     0,
       2,     0,     2,     5,     0,     3,     0,     2,     2,     4,
       1,     2,     2,     2,     2,     0,     2,     1,     3,     2,
       2,     2,     2,     1,     4,     3,     1,     3,     3,     4,
       5,     4,     5,     4,     1,     2,     1,     2,     2,     1,
       1,     4,     2,     3,     4,     0,     1,     5,     0,     3,
       0,     2,     3,     1,     2,     3,     3,     3,     5,     6,
       5,     6,     4,     1,     1,     1,     0,     1,     0,     3,
       0,     2,     0,     3,     0,     2,     1,     3,     1,     3
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
#line 129 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("stmtList::stmt"); 
				(yyval.list) = singleton((yyvsp[-1].node));
				bisonParseResult = (Node *) (yyval.list);	 
			}
#line 1612 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 135 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("stmtlist::stmtList::stmt");
				(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[-1].node));	
				bisonParseResult = (Node *) (yyval.list); 
			}
#line 1622 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 144 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::dmlStmt");
            (yyval.node) = (yyvsp[0].node);
        }
#line 1631 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 149 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::queryStmt");
            (yyval.node) = (yyvsp[0].node);
        }
#line 1640 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 154 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::transactionIdentifier");
            (yyval.node) = (Node *) createTransactionStmt((yyvsp[0].stringVal));
        }
#line 1649 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 164 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::insertQuery"); }
#line 1655 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 165 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::deleteQuery"); }
#line 1661 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 166 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::updateQuery"); }
#line 1667 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 173 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::bracketedQuery"); (yyval.node) = (yyvsp[-1].node); }
#line 1673 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 174 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::selectQuery"); }
#line 1679 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 175 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::provStmt"); }
#line 1685 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 176 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::setOperatorQuery"); }
#line 1691 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 180 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::BEGIN"); (yyval.stringVal) = strdup("TRANSACTION_BEGIN"); }
#line 1697 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 181 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::COMMIT"); (yyval.stringVal) = strdup("TRANSACTION_COMMIT"); }
#line 1703 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 182 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::ROLLBACK"); (yyval.stringVal) = strdup("TRANSACTION_ABORT"); }
#line 1709 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 190 "sql_parser.y" /* yacc.c:1646  */
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
#line 1724 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 201 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provStmt::stmtlist");
			ProvenanceStmt *p = createProvenanceStmt((Node *) (yyvsp[-1].list));
			p->inputType = PROV_INPUT_UPDATE_SEQUENCE;
			p->provType = PROV_PI_CS;
			p->asOf = (Node *) (yyvsp[-5].node);
			p->options = (yyvsp[-4].list);
			(yyval.node) = (Node *) p;
		}
#line 1738 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 211 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provStmt::transaction");
			ProvenanceStmt *p = createProvenanceStmt((Node *) createConstString((yyvsp[0].stringVal)));
			p->inputType = PROV_INPUT_TRANSACTION;
			p->provType = PROV_PI_CS;
			p->options = (yyvsp[-3].list);
			(yyval.node) = (Node *) p;
		}
#line 1751 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 222 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalProvAsOf::EMPTY"); (yyval.node) = NULL; }
#line 1757 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 224 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvAsOf::SCN");
			(yyval.node) = (Node *) createConstLong((yyvsp[0].intVal));
		}
#line 1766 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 229 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvAsOf::TIMESTAMP");
			(yyval.node) = (Node *) createConstString((yyvsp[0].stringVal));
		}
#line 1775 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 236 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalProvWith::EMPTY"); (yyval.list) = NIL; }
#line 1781 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 238 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvWith::WITH");
			(yyval.list) = (yyvsp[0].list);
		}
#line 1790 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 245 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("provOptionList::option"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 1796 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 247 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("provOptionList::list"); 
			(yyval.list) = appendToTailOfList((yyvsp[-1].list),(yyvsp[0].node)); 
		}
#line 1805 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 255 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("provOption::TYPE"); 
			(yyval.node) = (Node *) createStringKeyValue("TYPE", (yyvsp[0].stringVal)); 
		}
#line 1814 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 260 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::TABLE");
			(yyval.node) = (Node *) createStringKeyValue("TABLE", (yyvsp[0].stringVal));
		}
#line 1823 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 271 "sql_parser.y" /* yacc.c:1646  */
    { 
             RULELOG("deleteQuery");
             (yyval.node) = (Node *) createDelete((yyvsp[-2].stringVal), (yyvsp[0].node));
         }
#line 1832 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 279 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::NULL"); (yyval.stringVal) = NULL; }
#line 1838 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 280 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::FROM"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 1844 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 295 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG(updateQuery); 
                (yyval.node) = (Node *) createUpdate((yyvsp[-3].stringVal), (yyvsp[-1].list), (yyvsp[0].node)); 
            }
#line 1853 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 303 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setExpression");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 1862 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 308 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setClause::setExpression");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 1871 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 316 "sql_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal),"=")) {
                    RULELOG("setExpression::attributeRef::expression");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 1884 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 325 "sql_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal), "=")) {
                    RULELOG("setExpression::attributeRef::queryStmt");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 1897 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 340 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertQuery::insertList"); 
            	(yyval.node) = (Node *) createInsert((yyvsp[-4].stringVal),(Node *) (yyvsp[-1].list), NULL); 
        	}
#line 1906 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 345 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("insertQuery::queryStmt");
                (yyval.node) = (Node *) createInsert((yyvsp[-1].stringVal), (yyvsp[0].node), NULL);
            }
#line 1915 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 353 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::constant");
            	(yyval.list) = singleton((yyvsp[0].node)); 
            }
#line 1924 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 358 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("insertList::IDENTIFIER");
                (yyval.list) = singleton(createAttributeReference((yyvsp[0].stringVal)));
            }
#line 1933 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 363 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("insertList::insertList::::IDENTIFIER");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), createAttributeReference((yyvsp[0].stringVal)));
            }
#line 1942 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 368 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::insertList::constant");
            	(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 1951 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 382 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::INTERSECT");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 1960 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 387 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::MINUS");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 1969 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 392 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::UNION");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-2].stringVal), ((yyvsp[-1].stringVal) != NULL), (yyvsp[-3].node), (yyvsp[0].node));
            }
#line 1978 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 399 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::NULL"); (yyval.stringVal) = NULL; }
#line 1984 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 400 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::ALLTRUE"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 1990 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 410 "sql_parser.y" /* yacc.c:1646  */
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
#line 2010 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 432 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalDistinct::NULL"); (yyval.node) = NULL; }
#line 2016 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 434 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT");
                (yyval.node) = (Node *) createDistinctClause(NULL);
            }
#line 2025 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 439 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT::exprList");
                (yyval.node) = (Node *) createDistinctClause((yyvsp[-1].list));
            }
#line 2034 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 451 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectItem"); (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2042 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 455 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectClause::selectItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node)); 
            }
#line 2051 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 463 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression"); 
                 (yyval.node) = (Node *) createSelectItem(NULL, (yyvsp[0].node)); 
             }
#line 2060 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 468 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression::identifier"); 
                 (yyval.node) = (Node *) createSelectItem((yyvsp[0].stringVal), (yyvsp[-2].node));
             }
#line 2069 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 473 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
         		(yyval.node) = (Node *) createSelectItem(strdup("*"), NULL); 
     		}
#line 2078 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 478 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
     			(yyval.node) = (Node *) createSelectItem(CONCAT_STRINGS((yyvsp[-2].stringVal),".*"), NULL); 
 			}
#line 2087 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 488 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("exprList::SINGLETON"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 2093 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 490 "sql_parser.y" /* yacc.c:1646  */
    {
                  RULELOG("exprList::exprList::expression");
                  (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
             }
#line 2102 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 501 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::bracked"); (yyval.node) = (yyvsp[-1].node); }
#line 2108 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 502 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::constant"); }
#line 2114 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 503 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::attributeRef"); }
#line 2120 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 504 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlParameter"); }
#line 2126 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 505 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::binaryOperatorExpression"); }
#line 2132 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 506 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::unaryOperatorExpression"); }
#line 2138 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 507 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlFunctionCall"); }
#line 2144 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 508 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::case"); }
#line 2150 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 517 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::INT"); (yyval.node) = (Node *) createConstInt((yyvsp[0].intVal)); }
#line 2156 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 518 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::FLOAT"); (yyval.node) = (Node *) createConstFloat((yyvsp[0].floatVal)); }
#line 2162 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 519 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::STRING"); (yyval.node) = (Node *) createConstString((yyvsp[0].stringVal)); }
#line 2168 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 526 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("attributeRef::IDENTIFIER"); (yyval.node) = (Node *) createAttributeReference((yyvsp[0].stringVal)); }
#line 2174 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 532 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("sqlParameter::PARAMETER"); (yyval.node) = (Node *) createSQLParameter((yyvsp[0].stringVal)); }
#line 2180 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 552 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '+' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2191 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 559 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '-' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2202 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 566 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '*' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2213 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 573 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '/' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2224 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 580 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '%' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2235 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 587 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '^' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2246 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 596 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '&' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2257 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 603 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '|' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2268 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 612 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression::comparisonOps");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2279 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 622 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("unaryOperatorExpression:: '!' ");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2289 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 634 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("sqlFunctionCall::IDENTIFIER::exprList");
				FunctionCall *f = createFunctionCall((yyvsp[-4].stringVal), (yyvsp[-2].list));
				if ((yyvsp[0].node) != NULL)
					(yyval.node) = (Node *) createWindowFunction(f, (WindowDef *) (yyvsp[0].node));
				else  
                	(yyval.node) = (Node *) f; 
            }
#line 2302 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 649 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseExpression::CASE::expression::whens:else:END");
				(yyval.node) = (Node *) createCaseExpr((yyvsp[-3].node), (yyvsp[-2].list), (yyvsp[-1].node));
			}
#line 2311 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 654 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseExpression::CASE::whens::else::END");
				(yyval.node) = (Node *) createCaseExpr(NULL, (yyvsp[-2].list), (yyvsp[-1].node));
			}
#line 2320 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 662 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhenList::list::caseWhen");
				(yyval.list) = appendToTailOfList((yyvsp[-1].list), (yyvsp[0].node));
			}
#line 2329 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 667 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhenList::caseWhen");
				(yyval.list) = singleton((yyvsp[0].node));
			}
#line 2338 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 675 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhen::WHEN::expression::THEN::expression");
				(yyval.node) = (Node *) createCaseWhen((yyvsp[-2].node),(yyvsp[0].node));
			}
#line 2347 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 682 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalCaseElse::NULL"); (yyval.node) = NULL; }
#line 2353 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 684 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalCaseElse::ELSE::expression");
				(yyval.node) = (yyvsp[0].node);
			}
#line 2362 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 694 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("overclause::NULL"); (yyval.node) = NULL; }
#line 2368 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 696 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("overclause::window");
				(yyval.node) = (yyvsp[0].node);
			}
#line 2377 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 704 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("window");
				(yyval.node) = (Node *) createWindowDef((yyvsp[-3].list),(yyvsp[-2].list), (WindowFrame *) (yyvsp[-1].node));
			}
#line 2386 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 711 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optWindowPart::NULL"); (yyval.list) = NIL; }
#line 2392 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 713 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optWindowPart::PARTITION:BY::expressionList");
				(yyval.list) = (yyvsp[0].list);
			}
#line 2401 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 720 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optWindowFrame::NULL"); (yyval.node) = NULL; }
#line 2407 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 722 "sql_parser.y" /* yacc.c:1646  */
    { 
				WindowBound *l, *u = NULL;
				RULELOG("optWindowFrame::ROWS::windoBoundaries");
				l = getNthOfListP((yyvsp[0].list), 0);
				if(LIST_LENGTH((yyvsp[0].list)) > 1)
					u = getNthOfListP((yyvsp[0].list), 1);
				(yyval.node) = (Node *) createWindowFrame(WINFRAME_ROWS, l, u); 
			}
#line 2420 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 731 "sql_parser.y" /* yacc.c:1646  */
    {
				WindowBound *l, *u = NULL; 
				RULELOG("optWindowFrame::RANGE::windoBoundaries"); 
				l = getNthOfListP((yyvsp[0].list), 0);
				if(LIST_LENGTH((yyvsp[0].list)) > 1)
					u = getNthOfListP((yyvsp[0].list), 1);
				(yyval.node) = (Node *) createWindowFrame(WINFRAME_RANGE, l, u); 
			}
#line 2433 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 743 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBoundaries::BETWEEN"); 
				(yyval.list) = LIST_MAKE((yyvsp[-2].node), (yyvsp[0].node)); 
			}
#line 2442 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 748 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBoundaries::windowBound"); 
				(yyval.list) = singleton((yyvsp[0].node)); 
			}
#line 2451 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 756 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::UNBOUNDED::PRECEDING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_UNBOUND_PREC, NULL); 
			}
#line 2460 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 761 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::CURRENT::ROW"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_CURRENT_ROW, NULL); 
			}
#line 2469 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 766 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::expression::PRECEDING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_EXPR_PREC, (yyvsp[-1].node)); 
			}
#line 2478 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 771 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::expression::FOLLOWING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_EXPR_FOLLOW, (yyvsp[-1].node)); 
			}
#line 2487 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 783 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::NULL"); (yyval.list) = NULL; }
#line 2493 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 784 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::fromClause"); (yyval.list) = (yyvsp[0].list); }
#line 2499 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 789 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClauseItem");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2508 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 794 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClause::fromClauseItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2517 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 803 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
				FromItem *f = createFromTableRef(NULL, NIL, (yyvsp[-1].stringVal));
				f->provInfo = (FromProvInfo *) (yyvsp[0].node);
                (yyval.node) = (Node *) f;
            }
#line 2528 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 810 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
                FromItem *f = createFromTableRef(((FromItem *) (yyvsp[0].node))->name, 
						((FromItem *) (yyvsp[0].node))->attrNames, (yyvsp[-1].stringVal));
				f->provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
                (yyval.node) = (Node *) f;
            }
#line 2540 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 819 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromItem *f = (FromItem *) (yyvsp[-1].node);
                f->provInfo = (FromProvInfo *) (yyvsp[0].node);
                (yyval.node) = (yyvsp[-1].node);
            }
#line 2551 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 826 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromSubquery *s = (FromSubquery *) (yyvsp[-1].node);
                s->from.name = ((FromItem *) (yyvsp[0].node))->name;
                s->from.attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
                s->from.provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
                (yyval.node) = (Node *) s;
            }
#line 2564 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 835 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[0].node);
        		f->name = NULL;
        		(yyval.node) = (Node *) f;
        	}
#line 2576 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 843 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[-2].node);
        		f->name = ((FromItem *) (yyvsp[0].node))->name;
                f->attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
                f->provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
        		(yyval.node) = (Node *) f;
        	}
#line 2590 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 856 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("subQuery::queryStmt");
                (yyval.node) = (Node *) createFromSubquery(NULL, NULL, (yyvsp[-1].node));
            }
#line 2599 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 863 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = singleton((yyvsp[0].stringVal)); }
#line 2605 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 864 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].stringVal)); }
#line 2611 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 868 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2617 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 870 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURAL");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
						(FromItem *) (yyvsp[0].node), "JOIN_INNER", "JOIN_COND_NATURAL", 
						NULL);
          	}
#line 2628 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 877 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURALjoinType");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[0].node), (yyvsp[-2].stringVal), "JOIN_COND_NATURAL", NULL);
          	}
#line 2638 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 883 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::CROSS JOIN");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[0].node), "JOIN_CROSS", "JOIN_COND_ON", NULL);
          	}
#line 2648 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 889 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinType::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON";
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[-1].node), (yyvsp[-3].stringVal), condType, (yyvsp[0].node));
          	}
#line 2660 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 897 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON"; 
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[-1].node), "JOIN_INNER", 
                		condType, (yyvsp[0].node));
          	}
#line 2673 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 908 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2679 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 909 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT OUTER"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2685 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 910 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT "); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2691 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 911 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT OUTER"); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2697 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 912 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL OUTER"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2703 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 913 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2709 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 914 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::INNER"); (yyval.stringVal) = "JOIN_INNER"; }
#line 2715 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 918 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (Node *) (yyvsp[-1].list); }
#line 2721 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 919 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2727 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 924 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[-2].node);
				(yyval.node) = (Node *) f;
			}
#line 2738 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 931 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[-3].node); 
				(yyval.node) = (Node *) f;
			}
#line 2749 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 940 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFromProv::empty"); (yyval.node) = NULL; }
#line 2755 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 942 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = TRUE;
				p->userProvAttrs = NIL;				 
				(yyval.node) = (Node *) p; 
			}
#line 2767 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 950 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::userProvAttr");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = FALSE;
				p->userProvAttrs = (yyvsp[-1].list);				 
				(yyval.node) = (Node *) p; 
			}
#line 2779 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 960 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAttrAlias::empty"); (yyval.list) = NULL; }
#line 2785 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 962 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAttrAlias::identifierList"); (yyval.list) = (yyvsp[-1].list); 
			}
#line 2793 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 971 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::NULL"); (yyval.node) = NULL; }
#line 2799 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 972 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::whereExpression"); (yyval.node) = (yyvsp[0].node); }
#line 2805 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 976 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("where::brackedWhereExpression"); (yyval.node) = (yyvsp[-1].node); }
#line 2811 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 977 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("whereExpression::expression"); (yyval.node) = (yyvsp[0].node); }
#line 2817 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 979 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::NOT");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2827 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 985 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2838 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 992 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2849 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 999 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2860 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1006 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::BETWEEN-AND");
                List *expr = singleton((yyvsp[-4].node));
                expr = appendToTailOfList(expr, (yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr);
            }
#line 2872 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1014 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::comparisonOps::nestedSubQueryOperator::Subquery");
                (yyval.node) = (Node *) createNestedSubquery((yyvsp[-3].stringVal), (yyvsp[-5].node), (yyvsp[-4].stringVal), (yyvsp[-1].node));
            }
#line 2881 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1019 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::Subquery");
                Node *q = (Node *) createNestedSubquery("SCALAR", NULL, NULL, (yyvsp[-1].node)); 
                List *expr = LIST_MAKE((yyvsp[-4].node), q);
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr); 
            }
#line 2892 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1026 "sql_parser.y" /* yacc.c:1646  */
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
#line 2909 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1039 "sql_parser.y" /* yacc.c:1646  */
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
#line 2926 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1054 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ANY"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2932 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1055 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ALL"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2938 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1056 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::SOME"); (yyval.stringVal) = "ANY"; }
#line 2944 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 1060 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NULL"); (yyval.stringVal) = NULL; }
#line 2950 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 1061 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NOT"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2956 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1065 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::NULL"); (yyval.list) = NULL; }
#line 2962 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1066 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::GROUPBY"); (yyval.list) = (yyvsp[0].list); }
#line 2968 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 1070 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.node) = NULL; }
#line 2974 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1072 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("optionalHaving::HAVING"); 
                (yyval.node) = (Node *) (yyvsp[0].node);
            }
#line 2983 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1079 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.list) = NULL; }
#line 2989 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1080 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy::ORDERBY"); (yyval.list) = (yyvsp[0].list); }
#line 2995 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 1084 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::NULL"); (yyval.node) = NULL; }
#line 3001 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 1085 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::CONSTANT"); (yyval.node) = (yyvsp[0].node);}
#line 3007 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 1090 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::attributeRef");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 3016 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1095 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 3025 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 1100 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::constant");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 3034 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 1105 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 3043 "sql_parser.tab.c" /* yacc.c:1646  */
    break;


#line 3047 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 1112 "sql_parser.y" /* yacc.c:1906  */




/* FUTURE WORK 

PRIORITIES
7)
4)
1)

EXHAUSTIVE LIST
2. Implement support for RETURNING statement in DELETE queries.
3. Implement support for column list like (col1, col2, col3). 
   Needed in insert queries, select queries where conditions etc.
4. Implement support for Transactions.
5. Implement support for Create queries.
6. Implement support for windowing functions.
7. Implement support for AS OF (timestamp) modifier of a table reference
8. Implement support for casting expressions
9. Implement support for IN array expressions like a IN (1,2,3,4,5)
10. Implement support for ASC, DESC, NULLS FIRST/LAST in ORDER BY
*/

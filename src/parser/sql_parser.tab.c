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
#include "model/query_operator/operator_property.h"

#define RULELOG(grule) \
    { \
        TRACE_LOG("Parsing grammer rule <%s>", #grule); \
    }
    
#undef free

Node *bisonParseResult = NULL;

#line 87 "sql_parser.tab.c" /* yacc.c:339  */

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
    ONLY = 275,
    UPDATED = 276,
    SHOW = 277,
    INTERMEDIATE = 278,
    USE = 279,
    TUPLE = 280,
    VERSIONS = 281,
    FROM = 282,
    AS = 283,
    WHERE = 284,
    DISTINCT = 285,
    STARALL = 286,
    AND = 287,
    OR = 288,
    LIKE = 289,
    NOT = 290,
    IN = 291,
    ISNULL = 292,
    BETWEEN = 293,
    EXCEPT = 294,
    EXISTS = 295,
    AMMSC = 296,
    NULLVAL = 297,
    ALL = 298,
    ANY = 299,
    IS = 300,
    SOME = 301,
    UNION = 302,
    INTERSECT = 303,
    MINUS = 304,
    INTO = 305,
    VALUES = 306,
    HAVING = 307,
    GROUP = 308,
    ORDER = 309,
    BY = 310,
    LIMIT = 311,
    SET = 312,
    INT = 313,
    BEGIN_TRANS = 314,
    COMMIT_TRANS = 315,
    ROLLBACK_TRANS = 316,
    CASE = 317,
    WHEN = 318,
    THEN = 319,
    ELSE = 320,
    END = 321,
    OVER_TOK = 322,
    PARTITION = 323,
    ROWS = 324,
    RANGE = 325,
    UNBOUNDED = 326,
    PRECEDING = 327,
    CURRENT = 328,
    ROW = 329,
    FOLLOWING = 330,
    NULLS = 331,
    FIRST = 332,
    LAST = 333,
    ASC = 334,
    DESC = 335,
    DUMMYEXPR = 336,
    JOIN = 337,
    NATURAL = 338,
    LEFT = 339,
    RIGHT = 340,
    OUTER = 341,
    INNER = 342,
    CROSS = 343,
    ON = 344,
    USING = 345,
    FULL = 346,
    TYPE = 347,
    TRANSACTION = 348,
    WITH = 349,
    XOR = 350
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 29 "sql_parser.y" /* yacc.c:355  */

    /* 
     * Declare some C structure those will be used as data type
     * for various tokens used in grammar rules.
     */
     Node *node;
     List *list;
     char *stringVal;
     int intVal;
     double floatVal;

#line 235 "sql_parser.tab.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SQL_PARSER_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 250 "sql_parser.tab.c" /* yacc.c:358  */

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
#define YYFINAL  35
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   583

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  111
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  68
/* YYNRULES -- Number of rules.  */
#define YYNRULES  185
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  356

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   350

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
      19,    18,    10,     8,   109,     9,   110,    11,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   108,
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
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   133,   133,   139,   148,   153,   158,   163,   174,   175,
     176,   183,   184,   185,   186,   190,   198,   203,   211,   219,
     220,   221,   228,   239,   249,   261,   262,   267,   275,   276,
     284,   285,   293,   298,   303,   309,   315,   327,   336,   337,
     351,   359,   364,   372,   381,   396,   401,   409,   414,   419,
     424,   438,   443,   448,   456,   457,   466,   489,   490,   495,
     507,   511,   519,   524,   529,   534,   545,   546,   558,   559,
     560,   561,   562,   563,   564,   565,   574,   575,   576,   583,
     589,   608,   615,   622,   629,   636,   643,   652,   659,   668,
     678,   690,   705,   710,   718,   723,   731,   739,   740,   751,
     752,   760,   768,   769,   777,   778,   787,   799,   804,   812,
     817,   822,   827,   840,   841,   845,   850,   859,   866,   875,
     882,   891,   899,   912,   920,   921,   925,   926,   933,   939,
     945,   953,   965,   966,   967,   968,   969,   970,   971,   975,
     976,   980,   987,   997,   998,  1006,  1014,  1022,  1029,  1040,
    1041,  1051,  1052,  1056,  1057,  1058,  1064,  1071,  1078,  1096,
    1104,  1109,  1116,  1129,  1137,  1138,  1139,  1143,  1144,  1148,
    1149,  1153,  1154,  1162,  1163,  1167,  1168,  1172,  1177,  1185,
    1197,  1198,  1203,  1211,  1212,  1217
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
  "TIMESTAMP", "HAS", "TABLE", "ONLY", "UPDATED", "SHOW", "INTERMEDIATE",
  "USE", "TUPLE", "VERSIONS", "FROM", "AS", "WHERE", "DISTINCT", "STARALL",
  "AND", "OR", "LIKE", "NOT", "IN", "ISNULL", "BETWEEN", "EXCEPT",
  "EXISTS", "AMMSC", "NULLVAL", "ALL", "ANY", "IS", "SOME", "UNION",
  "INTERSECT", "MINUS", "INTO", "VALUES", "HAVING", "GROUP", "ORDER", "BY",
  "LIMIT", "SET", "INT", "BEGIN_TRANS", "COMMIT_TRANS", "ROLLBACK_TRANS",
  "CASE", "WHEN", "THEN", "ELSE", "END", "OVER_TOK", "PARTITION", "ROWS",
  "RANGE", "UNBOUNDED", "PRECEDING", "CURRENT", "ROW", "FOLLOWING",
  "NULLS", "FIRST", "LAST", "ASC", "DESC", "DUMMYEXPR", "JOIN", "NATURAL",
  "LEFT", "RIGHT", "OUTER", "INNER", "CROSS", "ON", "USING", "FULL",
  "TYPE", "TRANSACTION", "WITH", "XOR", "';'", "','", "'.'", "$accept",
  "stmtList", "stmt", "dmlStmt", "queryStmt", "withQuery", "withViewList",
  "withView", "transactionIdentifier", "provStmt", "optionalProvAsOf",
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
  "optionalHaving", "optionalOrderBy", "optionalLimit", "orderList",
  "orderExpr", "optionalSortOrder", "optionalNullOrder", YY_NULLPTR
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
     333,   334,   335,   336,   337,   338,   339,   340,   341,   342,
     343,   344,   345,   346,   347,   348,   349,   350,    59,    44,
      46
};
# endif

#define YYPACT_NINF -294

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-294)))

#define YYTABLE_NINF -168

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-168)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     115,   264,   -13,    -6,    53,    38,    44,  -294,  -294,  -294,
     104,    14,    25,  -294,    64,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,  -294,    95,    15,   346,   112,    66,  -294,   144,
     132,    55,   128,    23,  -294,  -294,    58,  -294,   135,   264,
     264,  -294,   175,  -294,  -294,  -294,   -11,  -294,  -294,   363,
     363,   258,   -26,  -294,   473,  -294,  -294,  -294,  -294,  -294,
    -294,  -294,   126,   199,   174,   152,    60,   191,   200,   104,
      64,  -294,  -294,   264,  -294,  -294,   363,   363,   212,   205,
     548,   507,   363,   390,    18,  -294,    26,   346,   184,   363,
     363,   363,   363,   363,   363,   363,   363,   363,   244,   232,
      64,  -294,     6,  -294,   242,   289,   252,   253,   267,   247,
     269,   249,   277,    60,  -294,    -7,   264,  -294,  -294,    -9,
     518,    -1,  -294,  -294,   380,    18,   363,  -294,   210,    19,
     306,   198,   472,    19,  -294,  -294,   289,   235,   441,   441,
     296,   296,   296,  -294,   548,   538,   229,  -294,   488,   199,
    -294,   368,   289,   289,   294,   449,   366,  -294,  -294,  -294,
    -294,  -294,  -294,  -294,  -294,   115,   309,   142,  -294,   363,
     240,   363,   250,   518,  -294,  -294,   297,   295,   312,  -294,
      10,   306,   147,   472,   316,    26,    26,   117,   256,   259,
    -294,   246,   261,   254,  -294,    10,   366,   271,   283,  -294,
       0,  -294,  -294,   339,   518,  -294,   421,   400,   366,   264,
     265,  -294,   313,   289,   289,   289,   363,    57,    -3,  -294,
    -294,   518,   377,  -294,   518,  -294,   389,   356,   396,   398,
     403,   238,  -294,   474,   472,   442,    26,   333,  -294,  -294,
      26,  -294,    26,   363,   289,   374,  -294,   573,   339,  -294,
     257,   339,  -294,  -294,  -294,   409,   422,     8,     8,   102,
     462,  -294,  -294,   375,  -294,   437,   448,   437,   437,  -294,
     398,   317,  -294,    10,   289,   459,  -294,  -294,    26,  -294,
     442,   386,   366,   431,   432,  -294,  -294,  -294,   362,   264,
     264,   363,   435,   374,  -294,     2,   437,     3,     5,  -294,
     366,   437,  -294,  -294,   363,   138,  -294,  -294,   365,   388,
     518,   363,   111,  -294,   483,    12,  -294,  -294,    13,   162,
     394,  -294,  -294,  -294,  -294,   386,   193,   193,   481,  -294,
    -294,  -294,  -294,  -294,   417,   363,   241,   423,   425,   219,
    -294,  -294,  -294,  -294,    94,  -294,  -294,   465,  -294,  -294,
    -294,  -294,  -294,  -294,   241,  -294
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    57,     0,     0,    38,    25,    19,    20,    21,
       0,     0,     0,     4,     5,     7,     6,    13,     9,    10,
       8,    14,    12,     0,    58,     0,     0,     0,    39,     0,
       0,    28,     0,     0,    17,     1,     0,     2,    54,     0,
       0,    11,     0,    76,    77,    78,    79,    80,    64,     0,
       0,     0,   113,    60,    62,    69,    70,    71,    72,    73,
      74,    75,     0,     0,     0,     0,     0,     0,     0,     0,
      15,     3,    55,     0,    51,    52,     0,     0,     0,    79,
      90,     0,     0,     0,    97,    95,     0,     0,   151,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      46,    79,   151,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    29,    30,     0,     0,    16,    53,     0,
      66,     0,    65,    68,     0,    97,     0,    94,     0,   143,
       0,   114,   115,   143,   121,    61,     0,   169,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    63,     0,     0,
      40,     0,     0,     0,     0,   154,    37,    26,    27,    33,
      34,    35,    36,    32,    31,     0,     0,     0,    59,     0,
      99,     0,     0,    98,    93,   144,     0,     0,     0,   118,
     117,     0,     0,     0,   121,     0,     0,     0,   132,   134,
     138,     0,   137,     0,   120,   119,   152,     0,   171,    48,
       0,    47,    42,     0,    43,    44,   154,     0,   155,     0,
       0,   168,     0,     0,     0,     0,     0,     0,     0,    24,
      18,    67,     0,    91,    96,    92,     0,     0,     0,   149,
       0,     0,   123,   126,   116,     0,     0,     0,   133,   135,
       0,   136,     0,     0,     0,   173,    45,     0,     0,   153,
       0,     0,   165,   164,   166,     0,     0,   156,   157,   158,
       0,    23,    22,   102,   100,     0,   147,     0,     0,   141,
     149,   123,   122,     0,     0,     0,   131,   127,     0,   129,
       0,   170,   172,     0,   175,    49,    50,   163,     0,     0,
       0,     0,     0,   173,   124,     0,     0,     0,     0,   142,
     140,     0,   128,   130,     0,     0,    56,   161,     0,     0,
     159,     0,   104,   145,     0,     0,   146,   150,     0,   180,
     174,   177,   176,   160,   162,   103,     0,     0,     0,   125,
     148,   139,   181,   182,   183,     0,     0,     0,     0,     0,
     105,   108,   106,   101,     0,   179,   178,     0,   109,   110,
     111,   112,   184,   185,     0,   107
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -294,   347,     4,  -294,     1,  -294,  -294,   454,  -294,  -294,
    -294,  -294,  -294,   427,  -294,  -294,  -294,  -294,   385,  -294,
    -294,  -294,  -294,  -294,  -294,  -294,   466,   -76,   -25,  -143,
     -53,  -294,  -294,  -294,  -294,  -294,   471,   -65,   438,  -294,
    -294,  -294,  -294,   237,  -293,  -294,  -294,   -83,   411,   118,
    -108,   383,   293,  -127,  -126,   304,   478,  -125,  -294,  -294,
    -294,  -294,   288,  -294,  -294,   248,  -294,  -294
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    11,    36,    13,    14,    15,    33,    34,    16,    17,
      31,    67,   113,   114,    18,    29,    19,   102,   103,    20,
     200,    21,    73,    22,    25,    52,    53,   119,   155,    55,
      56,    57,    58,    59,    60,    61,    84,    85,   128,   223,
     264,   293,   328,   340,   341,    88,   131,   183,   133,   295,
     134,   193,   276,   179,   180,   269,   137,   156,   255,   212,
     198,   245,   284,   306,   320,   321,   334,   345
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      54,   121,    23,   132,    12,   201,   194,   195,    77,   168,
     104,   196,   165,    86,    35,   262,   229,   170,   246,   127,
     313,   316,   184,   317,    80,    81,    83,   207,   208,    24,
     330,   331,   129,     1,    70,     2,     3,     4,     5,     6,
      74,    75,     1,   347,     2,   130,   175,   136,     6,   176,
     230,   120,   120,   177,   215,   178,    26,   124,   216,    27,
     127,   355,    54,   100,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   184,   118,   261,     1,    28,     2,     3,
       4,     5,     6,    87,    30,     7,     8,     9,   257,   258,
     259,   108,   109,    82,   110,   126,   104,   111,   166,    78,
     169,   173,   234,   235,   286,    37,   272,   273,   169,   247,
      32,   314,   314,    41,   314,   149,    42,   167,    62,   282,
      10,   314,   314,    38,    39,    40,   204,   206,     7,     8,
       9,   182,    69,    37,     1,    63,     2,     3,     4,     5,
       6,    43,    44,    45,   221,     1,   224,     2,  -168,   300,
      64,     6,  -168,   277,    38,    39,    40,   279,    65,   280,
     220,    66,   322,    10,   112,   232,    71,   281,    68,   218,
      89,    90,    91,    92,    93,    94,    95,    96,    81,    97,
     106,   107,   231,   352,   353,   146,     7,     8,     9,    99,
      72,   260,   326,   327,    76,   302,    43,    44,    45,    79,
      47,    38,    39,    40,   182,   101,    38,    39,    40,    49,
     250,   236,    50,   188,   189,   105,   190,   115,   120,   116,
     192,    10,   122,    81,    77,   136,    81,    89,    90,    91,
      92,    93,    94,    95,    96,   325,    97,    89,    90,    91,
      92,    93,    94,   336,    43,    44,    45,    79,    47,    23,
     147,   148,   288,   332,   333,   157,   271,    49,   158,   151,
      50,    43,    44,    45,    79,    47,   310,    51,    43,    44,
      45,    79,    47,   159,    49,   287,   337,    50,   338,   319,
     160,    49,   163,     1,   251,     2,   120,   162,   174,     6,
     308,   309,    43,    44,    45,    79,    47,    38,    39,    40,
     197,   339,   339,   350,   161,    49,   351,   185,   152,    94,
     319,   339,   129,   209,   219,    51,    38,    39,    40,   222,
     252,   253,   226,   254,   337,   181,   338,     2,   225,   339,
     227,     6,    51,    82,   233,   -11,   153,   228,   243,    51,
     240,   154,    43,    44,    45,    79,    47,   244,   242,    43,
      44,    45,    46,    47,   238,    49,    48,   239,   248,   241,
       2,   256,    49,    51,     6,    50,    43,    44,    45,    79,
      47,    43,    44,    45,    79,    47,   -11,   -11,   -11,    49,
     307,   266,    50,   323,    49,   297,   298,   203,    89,    90,
      91,    92,    93,    94,    95,    96,   263,    97,    89,    90,
      91,    92,    93,    94,    95,    96,   324,    97,   265,   270,
     213,   214,   215,    51,   315,   267,   216,   268,   249,   318,
      51,    38,    39,    40,    38,    39,    40,   278,   289,    89,
      90,    91,    92,    93,    94,    95,    96,    51,   210,   123,
     283,   290,    51,   294,   213,   214,   215,    38,    39,    40,
     216,    91,    92,    93,    94,   292,   171,    89,    90,    91,
      92,    93,    94,    95,    96,    82,   210,   296,   211,  -167,
      89,    90,    91,    92,    93,    94,    95,    96,   301,    97,
    -143,    89,    90,    91,    92,    93,    94,    95,    96,   329,
      97,    43,    44,    45,   199,   169,   211,  -167,   304,   343,
     305,   175,   311,   335,   176,   344,   291,   348,   177,   354,
     178,   349,   217,    98,  -143,    89,    90,    91,    92,    93,
      94,    95,    96,   117,    97,   123,    89,    90,    91,    92,
      93,    94,    95,    96,   202,    97,   186,   187,   188,   189,
     164,   190,   191,   274,   275,   192,    89,    90,    91,    92,
      93,    94,    95,   135,   125,    97,    89,    90,    91,    92,
      93,    94,   205,   172,   342,    97,   186,   187,   188,   189,
     237,   190,   191,   303,   299,   192,    43,    44,    45,   285,
     150,   312,     0,   346
};

static const yytype_int16 yycheck[] =
{
      25,    77,     1,    86,     0,   148,   133,   133,    19,    18,
      63,   136,    19,    39,     0,    18,     6,    18,    18,    84,
      18,    18,   130,    18,    49,    50,    51,   152,   153,    42,
      18,    18,     6,    19,    33,    21,    22,    23,    24,    25,
      39,    40,    19,   336,    21,    19,    27,    41,    25,    30,
      40,    76,    77,    34,    46,    36,    62,    82,    50,     6,
     125,   354,    87,    62,    89,    90,    91,    92,    93,    94,
      95,    96,    97,   181,    73,    18,    19,    39,    21,    22,
      23,    24,    25,   109,    40,    71,    72,    73,   213,   214,
     215,    31,    32,    75,    34,    77,   149,    37,   105,   110,
     109,   126,   185,   186,   247,   108,   233,   233,   109,   109,
       6,   109,   109,    18,   109,   109,   101,   116,     6,   244,
     106,   109,   109,    59,    60,    61,   151,   152,    71,    72,
      73,   130,   109,   108,    19,    69,    21,    22,    23,    24,
      25,     3,     4,     5,   169,    19,   171,    21,    46,   274,
       6,    25,    50,   236,    59,    60,    61,   240,    26,   242,
      18,   106,   305,   106,   104,    18,   108,   243,    40,   165,
       8,     9,    10,    11,    12,    13,    14,    15,   203,    17,
      28,    29,   181,    89,    90,   210,    71,    72,    73,    63,
      55,   216,    81,    82,    19,   278,     3,     4,     5,     6,
       7,    59,    60,    61,   203,     6,    59,    60,    61,    16,
     209,    94,    19,    96,    97,    41,    99,    26,   243,    19,
     103,   106,    10,   248,    19,    41,   251,     8,     9,    10,
      11,    12,    13,    14,    15,   311,    17,     8,     9,    10,
      11,    12,    13,    50,     3,     4,     5,     6,     7,   248,
       6,    19,   251,    91,    92,     3,    18,    16,     5,    17,
      19,     3,     4,     5,     6,     7,   291,    74,     3,     4,
       5,     6,     7,     6,    16,    18,    83,    19,    85,   304,
      33,    16,     5,    19,    19,    21,   311,    38,    78,    25,
     289,   290,     3,     4,     5,     6,     7,    59,    60,    61,
      65,   326,   327,    84,    35,    16,    87,   109,    19,    13,
     335,   336,     6,    19,     5,    74,    59,    60,    61,    79,
      55,    56,    25,    58,    83,    19,    85,    21,    78,   354,
      35,    25,    74,    75,    18,    18,    47,    25,    67,    74,
      94,    52,     3,     4,     5,     6,     7,    64,    94,     3,
       4,     5,     6,     7,    98,    16,    10,    98,    19,    98,
      21,    48,    16,    74,    25,    19,     3,     4,     5,     6,
       7,     3,     4,     5,     6,     7,    59,    60,    61,    16,
      18,    25,    19,    18,    16,   267,   268,    19,     8,     9,
      10,    11,    12,    13,    14,    15,    19,    17,     8,     9,
      10,    11,    12,    13,    14,    15,    18,    17,    19,     6,
      44,    45,    46,    74,   296,    19,    50,    19,    18,   301,
      74,    59,    60,    61,    59,    60,    61,    94,    19,     8,
       9,    10,    11,    12,    13,    14,    15,    74,    17,    18,
      66,    19,    74,     6,    44,    45,    46,    59,    60,    61,
      50,    10,    11,    12,    13,    80,    76,     8,     9,    10,
      11,    12,    13,    14,    15,    75,    17,    19,    47,    48,
       8,     9,    10,    11,    12,    13,    14,    15,    19,    17,
       6,     8,     9,    10,    11,    12,    13,    14,    15,     6,
      17,     3,     4,     5,     6,   109,    47,    48,    67,    18,
      68,    27,    67,   109,    30,    88,    44,    84,    34,    44,
      36,    86,   165,    40,    40,     8,     9,    10,    11,    12,
      13,    14,    15,    69,    17,    18,     8,     9,    10,    11,
      12,    13,    14,    15,   149,    17,    94,    95,    96,    97,
     113,    99,   100,   101,   102,   103,     8,     9,    10,    11,
      12,    13,    14,    87,    83,    17,     8,     9,    10,    11,
      12,    13,   151,   125,   327,    17,    94,    95,    96,    97,
     187,    99,   100,   280,   270,   103,     3,     4,     5,     6,
     102,   293,    -1,   335
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    19,    21,    22,    23,    24,    25,    71,    72,    73,
     106,   112,   113,   114,   115,   116,   119,   120,   125,   127,
     130,   132,   134,   115,    42,   135,    62,     6,    39,   126,
      40,   121,     6,   117,   118,     0,   113,   108,    59,    60,
      61,    18,   101,     3,     4,     5,     6,     7,    10,    16,
      19,    74,   136,   137,   139,   140,   141,   142,   143,   144,
     145,   146,     6,    69,     6,    26,   106,   122,    40,   109,
     115,   108,    55,   133,   115,   115,    19,    19,   110,     6,
     139,   139,    75,   139,   147,   148,    39,   109,   156,     8,
       9,    10,    11,    12,    13,    14,    15,    17,    40,    63,
     115,     6,   128,   129,   141,    41,    28,    29,    31,    32,
      34,    37,   104,   123,   124,    26,    19,   118,   115,   138,
     139,   138,    10,    18,   139,   147,    77,   148,   149,     6,
      19,   157,   158,   159,   161,   137,    41,   167,   139,   139,
     139,   139,   139,   139,   139,   139,   139,     6,    19,   109,
     167,    17,    19,    47,    52,   139,   168,     3,     5,     6,
      33,    35,    38,     5,   124,    19,   105,   115,    18,   109,
      18,    76,   149,   139,    78,    27,    30,    34,    36,   164,
     165,    19,   115,   158,   161,   109,    94,    95,    96,    97,
      99,   100,   103,   162,   164,   165,   168,    65,   171,     6,
     131,   140,   129,    19,   139,   159,   139,   168,   168,    19,
      17,    47,   170,    44,    45,    46,    50,   112,   113,     5,
      18,   139,    79,   150,   139,    78,    25,    35,    25,     6,
      40,   115,    18,    18,   158,   158,    94,   162,    98,    98,
      94,    98,    94,    67,    64,   172,    18,   109,    19,    18,
     115,    19,    55,    56,    58,   169,    48,   168,   168,   168,
     139,    18,    18,    19,   151,    19,    25,    19,    19,   166,
       6,    18,   164,   165,   101,   102,   163,   158,    94,   158,
     158,   138,   168,    66,   173,     6,   140,    18,   115,    19,
      19,    44,    80,   152,     6,   160,    19,   160,   160,   166,
     168,    19,   158,   163,    67,    68,   174,    18,   115,   115,
     139,    67,   173,    18,   109,   160,    18,    18,   160,   139,
     175,   176,   140,    18,    18,   138,    81,    82,   153,     6,
      18,    18,    91,    92,   177,   109,    50,    83,    85,   139,
     154,   155,   154,    18,    88,   178,   176,   155,    84,    86,
      84,    87,    89,    90,    44,   155
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   111,   112,   112,   113,   113,   113,   113,   114,   114,
     114,   115,   115,   115,   115,   116,   117,   117,   118,   119,
     119,   119,   120,   120,   120,   121,   121,   121,   122,   122,
     123,   123,   124,   124,   124,   124,   124,   125,   126,   126,
     127,   128,   128,   129,   129,   130,   130,   131,   131,   131,
     131,   132,   132,   132,   133,   133,   134,   135,   135,   135,
     136,   136,   137,   137,   137,   137,   138,   138,   139,   139,
     139,   139,   139,   139,   139,   139,   140,   140,   140,   141,
     142,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     144,   145,   146,   146,   147,   147,   148,   149,   149,   150,
     150,   151,   152,   152,   153,   153,   153,   154,   154,   155,
     155,   155,   155,   156,   156,   157,   157,   158,   158,   158,
     158,   158,   158,   159,   160,   160,   161,   161,   161,   161,
     161,   161,   162,   162,   162,   162,   162,   162,   162,   163,
     163,   164,   164,   165,   165,   165,   165,   165,   165,   166,
     166,   167,   167,   168,   168,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   169,   169,   169,   170,   170,   171,
     171,   172,   172,   173,   173,   174,   174,   175,   175,   176,
     177,   177,   177,   178,   178,   178
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     3,     1,     1,     1,     1,     1,     1,
       1,     3,     1,     1,     1,     3,     3,     1,     5,     1,
       1,     1,     7,     7,     6,     0,     4,     4,     0,     2,
       1,     2,     2,     2,     2,     2,     2,     5,     0,     1,
       5,     1,     3,     3,     3,     7,     4,     1,     1,     3,
       3,     3,     3,     4,     0,     1,     9,     0,     1,     5,
       1,     3,     1,     3,     1,     3,     1,     3,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     5,     5,     4,     2,     1,     4,     0,     2,     0,
       2,     5,     0,     3,     0,     2,     2,     4,     1,     2,
       2,     2,     2,     0,     2,     1,     3,     2,     2,     2,
       2,     1,     4,     3,     1,     3,     3,     4,     5,     4,
       5,     4,     1,     2,     1,     2,     2,     1,     1,     4,
       2,     3,     4,     0,     1,     5,     5,     3,     6,     0,
       3,     0,     2,     3,     1,     2,     3,     3,     3,     5,
       6,     5,     6,     4,     1,     1,     1,     0,     1,     0,
       3,     0,     2,     0,     3,     0,     2,     1,     3,     3,
       0,     1,     1,     0,     2,     2
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
#line 134 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("stmtList::stmt"); 
				(yyval.list) = singleton((yyvsp[-1].node));
				bisonParseResult = (Node *) (yyval.list);	 
			}
#line 1657 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 140 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("stmtlist::stmtList::stmt");
				(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[-1].node));	
				bisonParseResult = (Node *) (yyval.list); 
			}
#line 1667 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 149 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::dmlStmt");
            (yyval.node) = (yyvsp[0].node);
        }
#line 1676 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 154 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::queryStmt");
            (yyval.node) = (yyvsp[0].node);
        }
#line 1685 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 159 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::transactionIdentifier");
            (yyval.node) = (Node *) createTransactionStmt((yyvsp[0].stringVal));
        }
#line 1694 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 164 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("stmt::withQuery"); 
			(yyval.node) = (yyvsp[0].node); 
		}
#line 1703 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 174 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::insertQuery"); }
#line 1709 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 175 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::deleteQuery"); }
#line 1715 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 176 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::updateQuery"); }
#line 1721 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 183 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::bracketedQuery"); (yyval.node) = (yyvsp[-1].node); }
#line 1727 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 184 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::selectQuery"); }
#line 1733 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 185 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::provStmt"); }
#line 1739 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 186 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::setOperatorQuery"); }
#line 1745 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 191 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withQuery::withViewList::queryStmt");
			(yyval.node) = (Node *) createWithStmt((yyvsp[-1].list), (yyvsp[0].node));
		}
#line 1754 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 199 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withViewList::list::view");
			(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
		}
#line 1763 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 204 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withViewList::view");
			(yyval.list) = singleton((yyvsp[0].node));
		}
#line 1772 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 212 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withView::ident::AS:queryStmt");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString((yyvsp[-4].stringVal)), (yyvsp[-1].node));
		}
#line 1781 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 219 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::BEGIN"); (yyval.stringVal) = strdup("TRANSACTION_BEGIN"); }
#line 1787 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 220 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::COMMIT"); (yyval.stringVal) = strdup("TRANSACTION_COMMIT"); }
#line 1793 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 221 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::ROLLBACK"); (yyval.stringVal) = strdup("TRANSACTION_ABORT"); }
#line 1799 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 229 "sql_parser.y" /* yacc.c:1646  */
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
#line 1814 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 240 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provStmt::stmtlist");
			ProvenanceStmt *p = createProvenanceStmt((Node *) (yyvsp[-1].list));
			p->inputType = PROV_INPUT_UPDATE_SEQUENCE;
			p->provType = PROV_PI_CS;
			p->asOf = (Node *) (yyvsp[-5].node);
			p->options = (yyvsp[-4].list);
			(yyval.node) = (Node *) p;
		}
#line 1828 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 250 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provStmt::transaction");
			ProvenanceStmt *p = createProvenanceStmt((Node *) createConstString((yyvsp[0].stringVal)));
			p->inputType = PROV_INPUT_TRANSACTION;
			p->provType = PROV_PI_CS;
			p->options = (yyvsp[-3].list);
			(yyval.node) = (Node *) p;
		}
#line 1841 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 261 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalProvAsOf::EMPTY"); (yyval.node) = NULL; }
#line 1847 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 263 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvAsOf::SCN");
			(yyval.node) = (Node *) createConstLong((yyvsp[0].intVal));
		}
#line 1856 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 268 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvAsOf::TIMESTAMP");
			(yyval.node) = (Node *) createConstString((yyvsp[0].stringVal));
		}
#line 1865 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 275 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalProvWith::EMPTY"); (yyval.list) = NIL; }
#line 1871 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 277 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvWith::WITH");
			(yyval.list) = (yyvsp[0].list);
		}
#line 1880 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 284 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("provOptionList::option"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 1886 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 286 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("provOptionList::list"); 
			(yyval.list) = appendToTailOfList((yyvsp[-1].list),(yyvsp[0].node)); 
		}
#line 1895 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 294 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("provOption::TYPE"); 
			(yyval.node) = (Node *) createStringKeyValue("TYPE", (yyvsp[0].stringVal)); 
		}
#line 1904 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 299 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::TABLE");
			(yyval.node) = (Node *) createStringKeyValue("TABLE", (yyvsp[0].stringVal));
		}
#line 1913 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 304 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::ONLY::UPDATED");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_ONLY_UPDATED), 
					(Node *) createConstBool(TRUE));
		}
#line 1923 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 310 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::SHOW::INTERMEDIATE");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_SHOW_INTERMEDIATE), 
					(Node *) createConstBool(TRUE));
		}
#line 1933 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 316 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::TUPLE::VERSIONS");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_TUPLE_VERSIONS),
					(Node *) createConstBool(TRUE));
		}
#line 1943 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 328 "sql_parser.y" /* yacc.c:1646  */
    { 
             RULELOG("deleteQuery");
             (yyval.node) = (Node *) createDelete((yyvsp[-2].stringVal), (yyvsp[0].node));
         }
#line 1952 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 336 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::NULL"); (yyval.stringVal) = NULL; }
#line 1958 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 337 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::FROM"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 1964 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 352 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG(updateQuery); 
                (yyval.node) = (Node *) createUpdate((yyvsp[-3].stringVal), (yyvsp[-1].list), (yyvsp[0].node)); 
            }
#line 1973 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 360 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setExpression");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 1982 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 365 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setClause::setExpression");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 1991 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 373 "sql_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal),"=")) {
                    RULELOG("setExpression::attributeRef::expression");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 2004 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 382 "sql_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal), "=")) {
                    RULELOG("setExpression::attributeRef::queryStmt");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 2017 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 397 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertQuery::insertList"); 
            	(yyval.node) = (Node *) createInsert((yyvsp[-4].stringVal),(Node *) (yyvsp[-1].list), NULL); 
        	}
#line 2026 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 402 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("insertQuery::queryStmt");
                (yyval.node) = (Node *) createInsert((yyvsp[-1].stringVal), (yyvsp[0].node), NULL);
            }
#line 2035 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 410 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::constant");
            	(yyval.list) = singleton((yyvsp[0].node)); 
            }
#line 2044 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 415 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("insertList::IDENTIFIER");
                (yyval.list) = singleton(createAttributeReference((yyvsp[0].stringVal)));
            }
#line 2053 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 420 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("insertList::insertList::::IDENTIFIER");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), createAttributeReference((yyvsp[0].stringVal)));
            }
#line 2062 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 425 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::insertList::constant");
            	(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2071 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 439 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::INTERSECT");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 2080 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 444 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::MINUS");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 2089 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 449 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::UNION");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-2].stringVal), ((yyvsp[-1].stringVal) != NULL), (yyvsp[-3].node), (yyvsp[0].node));
            }
#line 2098 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 456 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::NULL"); (yyval.stringVal) = NULL; }
#line 2104 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 457 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::ALLTRUE"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2110 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 467 "sql_parser.y" /* yacc.c:1646  */
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
#line 2130 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 489 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalDistinct::NULL"); (yyval.node) = NULL; }
#line 2136 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 491 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT");
                (yyval.node) = (Node *) createDistinctClause(NULL);
            }
#line 2145 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 496 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT::exprList");
                (yyval.node) = (Node *) createDistinctClause((yyvsp[-1].list));
            }
#line 2154 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 508 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectItem"); (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2162 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 512 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectClause::selectItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node)); 
            }
#line 2171 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 520 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression"); 
                 (yyval.node) = (Node *) createSelectItem(NULL, (yyvsp[0].node)); 
             }
#line 2180 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 525 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression::identifier"); 
                 (yyval.node) = (Node *) createSelectItem((yyvsp[0].stringVal), (yyvsp[-2].node));
             }
#line 2189 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 530 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
         		(yyval.node) = (Node *) createSelectItem(strdup("*"), NULL); 
     		}
#line 2198 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 535 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
     			(yyval.node) = (Node *) createSelectItem(CONCAT_STRINGS((yyvsp[-2].stringVal),".*"), NULL); 
 			}
#line 2207 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 545 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("exprList::SINGLETON"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 2213 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 547 "sql_parser.y" /* yacc.c:1646  */
    {
                  RULELOG("exprList::exprList::expression");
                  (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
             }
#line 2222 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 558 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::bracked"); (yyval.node) = (yyvsp[-1].node); }
#line 2228 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 559 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::constant"); }
#line 2234 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 560 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::attributeRef"); }
#line 2240 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 561 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlParameter"); }
#line 2246 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 562 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::binaryOperatorExpression"); }
#line 2252 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 563 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::unaryOperatorExpression"); }
#line 2258 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 564 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlFunctionCall"); }
#line 2264 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 565 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::case"); }
#line 2270 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 574 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::INT"); (yyval.node) = (Node *) createConstInt((yyvsp[0].intVal)); }
#line 2276 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 575 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::FLOAT"); (yyval.node) = (Node *) createConstFloat((yyvsp[0].floatVal)); }
#line 2282 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 576 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::STRING"); (yyval.node) = (Node *) createConstString((yyvsp[0].stringVal)); }
#line 2288 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 583 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("attributeRef::IDENTIFIER"); (yyval.node) = (Node *) createAttributeReference((yyvsp[0].stringVal)); }
#line 2294 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 589 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("sqlParameter::PARAMETER"); (yyval.node) = (Node *) createSQLParameter((yyvsp[0].stringVal)); }
#line 2300 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 609 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '+' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2311 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 616 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '-' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2322 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 623 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '*' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2333 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 630 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '/' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2344 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 637 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '%' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2355 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 644 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '^' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2366 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 653 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '&' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2377 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 660 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '|' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2388 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 669 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression::comparisonOps");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2399 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 679 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("unaryOperatorExpression:: '!' ");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2409 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 691 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("sqlFunctionCall::IDENTIFIER::exprList");
				FunctionCall *f = createFunctionCall((yyvsp[-4].stringVal), (yyvsp[-2].list));
				if ((yyvsp[0].node) != NULL)
					(yyval.node) = (Node *) createWindowFunction(f, (WindowDef *) (yyvsp[0].node));
				else  
                	(yyval.node) = (Node *) f; 
            }
#line 2422 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 706 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseExpression::CASE::expression::whens:else:END");
				(yyval.node) = (Node *) createCaseExpr((yyvsp[-3].node), (yyvsp[-2].list), (yyvsp[-1].node));
			}
#line 2431 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 711 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseExpression::CASE::whens::else::END");
				(yyval.node) = (Node *) createCaseExpr(NULL, (yyvsp[-2].list), (yyvsp[-1].node));
			}
#line 2440 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 719 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhenList::list::caseWhen");
				(yyval.list) = appendToTailOfList((yyvsp[-1].list), (yyvsp[0].node));
			}
#line 2449 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 724 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhenList::caseWhen");
				(yyval.list) = singleton((yyvsp[0].node));
			}
#line 2458 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 732 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhen::WHEN::expression::THEN::expression");
				(yyval.node) = (Node *) createCaseWhen((yyvsp[-2].node),(yyvsp[0].node));
			}
#line 2467 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 739 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalCaseElse::NULL"); (yyval.node) = NULL; }
#line 2473 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 741 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalCaseElse::ELSE::expression");
				(yyval.node) = (yyvsp[0].node);
			}
#line 2482 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 751 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("overclause::NULL"); (yyval.node) = NULL; }
#line 2488 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 753 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("overclause::window");
				(yyval.node) = (yyvsp[0].node);
			}
#line 2497 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 761 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("window");
				(yyval.node) = (Node *) createWindowDef((yyvsp[-3].list),(yyvsp[-2].list), (WindowFrame *) (yyvsp[-1].node));
			}
#line 2506 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 768 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optWindowPart::NULL"); (yyval.list) = NIL; }
#line 2512 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 770 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optWindowPart::PARTITION:BY::expressionList");
				(yyval.list) = (yyvsp[0].list);
			}
#line 2521 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 777 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optWindowFrame::NULL"); (yyval.node) = NULL; }
#line 2527 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 779 "sql_parser.y" /* yacc.c:1646  */
    { 
				WindowBound *l, *u = NULL;
				RULELOG("optWindowFrame::ROWS::windoBoundaries");
				l = getNthOfListP((yyvsp[0].list), 0);
				if(LIST_LENGTH((yyvsp[0].list)) > 1)
					u = getNthOfListP((yyvsp[0].list), 1);
				(yyval.node) = (Node *) createWindowFrame(WINFRAME_ROWS, l, u); 
			}
#line 2540 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 788 "sql_parser.y" /* yacc.c:1646  */
    {
				WindowBound *l, *u = NULL; 
				RULELOG("optWindowFrame::RANGE::windoBoundaries"); 
				l = getNthOfListP((yyvsp[0].list), 0);
				if(LIST_LENGTH((yyvsp[0].list)) > 1)
					u = getNthOfListP((yyvsp[0].list), 1);
				(yyval.node) = (Node *) createWindowFrame(WINFRAME_RANGE, l, u); 
			}
#line 2553 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 800 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBoundaries::BETWEEN"); 
				(yyval.list) = LIST_MAKE((yyvsp[-2].node), (yyvsp[0].node)); 
			}
#line 2562 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 805 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBoundaries::windowBound"); 
				(yyval.list) = singleton((yyvsp[0].node)); 
			}
#line 2571 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 813 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::UNBOUNDED::PRECEDING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_UNBOUND_PREC, NULL); 
			}
#line 2580 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 818 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::CURRENT::ROW"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_CURRENT_ROW, NULL); 
			}
#line 2589 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 823 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::expression::PRECEDING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_EXPR_PREC, (yyvsp[-1].node)); 
			}
#line 2598 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 828 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::expression::FOLLOWING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_EXPR_FOLLOW, (yyvsp[-1].node)); 
			}
#line 2607 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 840 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::NULL"); (yyval.list) = NULL; }
#line 2613 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 841 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::fromClause"); (yyval.list) = (yyvsp[0].list); }
#line 2619 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 846 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClauseItem");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2628 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 851 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClause::fromClauseItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2637 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 860 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
				FromItem *f = createFromTableRef(NULL, NIL, (yyvsp[-1].stringVal));
				f->provInfo = (FromProvInfo *) (yyvsp[0].node);
                (yyval.node) = (Node *) f;
            }
#line 2648 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 867 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
                FromItem *f = createFromTableRef(((FromItem *) (yyvsp[0].node))->name, 
						((FromItem *) (yyvsp[0].node))->attrNames, (yyvsp[-1].stringVal));
				f->provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
                (yyval.node) = (Node *) f;
            }
#line 2660 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 876 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromItem *f = (FromItem *) (yyvsp[-1].node);
                f->provInfo = (FromProvInfo *) (yyvsp[0].node);
                (yyval.node) = (yyvsp[-1].node);
            }
#line 2671 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 883 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromSubquery *s = (FromSubquery *) (yyvsp[-1].node);
                s->from.name = ((FromItem *) (yyvsp[0].node))->name;
                s->from.attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
                s->from.provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
                (yyval.node) = (Node *) s;
            }
#line 2684 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 892 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[0].node);
        		f->name = NULL;
        		(yyval.node) = (Node *) f;
        	}
#line 2696 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 900 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[-2].node);
        		f->name = ((FromItem *) (yyvsp[0].node))->name;
                f->attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
                f->provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
        		(yyval.node) = (Node *) f;
        	}
#line 2710 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 913 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("subQuery::queryStmt");
                (yyval.node) = (Node *) createFromSubquery(NULL, NULL, (yyvsp[-1].node));
            }
#line 2719 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 920 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = singleton((yyvsp[0].stringVal)); }
#line 2725 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 921 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].stringVal)); }
#line 2731 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 925 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2737 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 927 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURAL");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
						(FromItem *) (yyvsp[0].node), "JOIN_INNER", "JOIN_COND_NATURAL", 
						NULL);
          	}
#line 2748 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 934 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURALjoinType");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[0].node), (yyvsp[-2].stringVal), "JOIN_COND_NATURAL", NULL);
          	}
#line 2758 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 940 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::CROSS JOIN");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[0].node), "JOIN_CROSS", "JOIN_COND_ON", NULL);
          	}
#line 2768 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 946 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinType::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON";
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[-1].node), (yyvsp[-3].stringVal), condType, (yyvsp[0].node));
          	}
#line 2780 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 954 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON"; 
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[-1].node), "JOIN_INNER", 
                		condType, (yyvsp[0].node));
          	}
#line 2793 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 965 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2799 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 966 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT OUTER"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2805 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 967 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT "); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2811 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 968 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT OUTER"); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2817 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 969 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL OUTER"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2823 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 970 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2829 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 971 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::INNER"); (yyval.stringVal) = "JOIN_INNER"; }
#line 2835 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 975 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (Node *) (yyvsp[-1].list); }
#line 2841 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 976 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2847 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 981 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[-2].node);
				(yyval.node) = (Node *) f;
			}
#line 2858 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 988 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[-3].node); 
				(yyval.node) = (Node *) f;
			}
#line 2869 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 997 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFromProv::empty"); (yyval.node) = NULL; }
#line 2875 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 999 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::BASERELATION");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = TRUE;
				p->userProvAttrs = NIL;				 
				(yyval.node) = (Node *) p; 
			}
#line 2887 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1007 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::userProvAttr");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = FALSE;
				p->userProvAttrs = (yyvsp[-1].list);				 
				(yyval.node) = (Node *) p; 
			}
#line 2899 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1015 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::userProvDupAttr");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = TRUE;
				p->userProvAttrs = (yyvsp[-1].list);
				(yyval.node) = (Node *) p;
			}
#line 2911 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1023 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::intermediateProv");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->intermediateProv = TRUE;
				(yyval.node) = (Node *) p;
			}
#line 2922 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1030 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::intermediateProv::attrList");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->intermediateProv = TRUE;
				p->userProvAttrs = (yyvsp[-1].list);
				(yyval.node) = (Node *) p;
			}
#line 2934 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1040 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAttrAlias::empty"); (yyval.list) = NULL; }
#line 2940 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1042 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAttrAlias::identifierList"); (yyval.list) = (yyvsp[-1].list); 
			}
#line 2948 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1051 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::NULL"); (yyval.node) = NULL; }
#line 2954 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1052 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::whereExpression"); (yyval.node) = (yyvsp[0].node); }
#line 2960 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1056 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("where::brackedWhereExpression"); (yyval.node) = (yyvsp[-1].node); }
#line 2966 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1057 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("whereExpression::expression"); (yyval.node) = (yyvsp[0].node); }
#line 2972 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1059 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::NOT");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2982 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 1065 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2993 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 1072 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::OR");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 3004 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1079 "sql_parser.y" /* yacc.c:1646  */
    {
				//if ($2 == NULL)
                //{
                	RULELOG("whereExpression::LIKE");
	                List *expr = singleton((yyvsp[-2].node));
	                expr = appendToTailOfList(expr, (yyvsp[0].node));
	                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                /* }
				else
				{   
                	RULELOG("whereExpression::NOT::LIKE");
                	List *expr = singleton($1);
                	expr = appendToTailOfList(expr, $4);
                	Node *like = (Node *) createOpExpr($3, expr);
                	$$ = (Node *) createOpExpr("NOT", singleton(like));
				}*/
            }
#line 3026 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1097 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::BETWEEN-AND");
                List *expr = singleton((yyvsp[-4].node));
                expr = appendToTailOfList(expr, (yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr);
            }
#line 3038 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 1105 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::comparisonOps::nestedSubQueryOperator::Subquery");
                (yyval.node) = (Node *) createNestedSubquery((yyvsp[-3].stringVal), (yyvsp[-5].node), (yyvsp[-4].stringVal), (yyvsp[-1].node));
            }
#line 3047 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1110 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::Subquery");
                Node *q = (Node *) createNestedSubquery("SCALAR", NULL, NULL, (yyvsp[-1].node)); 
                List *expr = LIST_MAKE((yyvsp[-4].node), q);
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr); 
            }
#line 3058 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1117 "sql_parser.y" /* yacc.c:1646  */
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
#line 3075 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1130 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::EXISTS");
                (yyval.node) = (Node *) createNestedSubquery((yyvsp[-3].stringVal), NULL, NULL, (yyvsp[-1].node));
            }
#line 3084 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 1137 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ANY"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 3090 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 1138 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ALL"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 3096 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 1139 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::SOME"); (yyval.stringVal) = "ANY"; }
#line 3102 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1143 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NULL"); (yyval.stringVal) = NULL; }
#line 3108 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 1144 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NOT"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 3114 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 1148 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::NULL"); (yyval.list) = NULL; }
#line 3120 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 1149 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::GROUPBY"); (yyval.list) = (yyvsp[0].list); }
#line 3126 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 1153 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.node) = NULL; }
#line 3132 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 1155 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("optionalHaving::HAVING"); 
                (yyval.node) = (Node *) (yyvsp[0].node);
            }
#line 3141 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 1162 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.list) = NULL; }
#line 3147 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 1163 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy::ORDERBY"); (yyval.list) = (yyvsp[0].list); }
#line 3153 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 1167 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::NULL"); (yyval.node) = NULL; }
#line 3159 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 1168 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::CONSTANT"); (yyval.node) = (yyvsp[0].node);}
#line 3165 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 1173 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::orderExpr");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 3174 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 1178 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("orderList::orderList::orderExpr");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 3183 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 1186 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("orderExpr::expr::sortOrder::nullOrder");
				SortOrder o = (strcmp((yyvsp[-1].stringVal),"ASC") == 0) ?  SORT_ASC : SORT_DESC;
				SortNullOrder n = (strcmp((yyvsp[0].stringVal),"NULLS_FIRST") == 0) ? 
						SORT_NULLS_FIRST : 
						SORT_NULLS_LAST;
				(yyval.node) = (Node *) createOrderExpr((yyvsp[-2].node), o, n);
			}
#line 3196 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 1197 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalSortOrder::empty"); (yyval.stringVal) = "ASC"; }
#line 3202 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 1199 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalSortOrder::ASC");
				(yyval.stringVal) = "ASC";
			}
#line 3211 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 1204 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalSortOrder::DESC");
				(yyval.stringVal) = "DESC";
			}
#line 3220 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 1211 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNullOrder::empty"); (yyval.stringVal) = "NULLS_LAST"; }
#line 3226 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 1213 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalNullOrder::NULLS FIRST");
				(yyval.stringVal) = "NULLS_FIRST";
			}
#line 3235 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 1218 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalNullOrder::NULLS LAST");
				(yyval.stringVal) = "NULLS_LAST";
			}
#line 3244 "sql_parser.tab.c" /* yacc.c:1646  */
    break;


#line 3248 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 1225 "sql_parser.y" /* yacc.c:1906  */




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
7. Implement support for AS OF (timestamp) modifier of a table reference
8. Implement support for casting expressions
9. Implement support for IN array expressions like a IN (1,2,3,4,5)
10. Implement support for ASC, DESC, NULLS FIRST/LAST in ORDER BY
*/

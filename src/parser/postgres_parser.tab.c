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


/* Substitute the variable and function names.  */
#define yyparse         postgresparse
#define yylex           postgreslex
#define yyerror         postgreserror
#define yydebug         postgresdebug
#define yynerrs         postgresnerrs

#define yylval          postgreslval
#define yychar          postgreschar

/* Copy the first part of user declarations.  */
#line 1 "postgres_parser.y" /* yacc.c:339  */

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "parser/parse_internal_postgres.h"
#include "log/logger.h"
#include "model/query_operator/operator_property.h"

#define RULELOG(grule) \
    { \
        TRACE_LOG("Parsing grammer rule <%s>", #grule); \
    }
    
#undef free

Node *postgresParseResult = NULL;

#line 95 "postgres_parser.tab.c" /* yacc.c:339  */

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
   by #include "postgres_parser.tab.h".  */
#ifndef YY_POSTGRES_POSTGRES_PARSER_TAB_H_INCLUDED
# define YY_POSTGRES_POSTGRES_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int postgresdebug;
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
#line 24 "postgres_parser.y" /* yacc.c:355  */

    /* 
     * Declare some C structure those will be used as data type
     * for various tokens used in grammar rules.
     */
     Node *node;
     List *list;
     char *stringVal;
     int intVal;
     double floatVal;

#line 243 "postgres_parser.tab.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE postgreslval;

int postgresparse (void);

#endif /* !YY_POSTGRES_POSTGRES_PARSER_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 258 "postgres_parser.tab.c" /* yacc.c:358  */

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
#define YYLAST   597

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  111
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  68
/* YYNRULES -- Number of rules.  */
#define YYNRULES  186
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  361

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
       0,   128,   128,   134,   143,   148,   153,   158,   169,   170,
     171,   178,   179,   180,   181,   185,   193,   198,   206,   214,
     215,   216,   223,   234,   244,   256,   257,   262,   270,   271,
     279,   280,   288,   293,   298,   304,   310,   322,   331,   332,
     346,   354,   359,   367,   376,   391,   396,   404,   409,   414,
     419,   433,   438,   443,   451,   452,   461,   484,   485,   490,
     502,   506,   514,   519,   524,   529,   540,   541,   553,   554,
     555,   556,   557,   558,   559,   560,   569,   570,   571,   578,
     584,   603,   610,   617,   624,   631,   638,   647,   654,   663,
     673,   685,   694,   709,   714,   722,   727,   735,   743,   744,
     755,   756,   764,   772,   773,   781,   782,   791,   803,   808,
     816,   821,   826,   831,   844,   845,   849,   854,   863,   870,
     879,   886,   895,   903,   916,   924,   925,   929,   930,   937,
     943,   949,   957,   969,   970,   971,   972,   973,   974,   975,
     979,   980,   984,   991,  1001,  1002,  1010,  1018,  1026,  1033,
    1044,  1045,  1055,  1056,  1060,  1061,  1062,  1068,  1075,  1082,
    1100,  1108,  1113,  1120,  1133,  1141,  1142,  1143,  1147,  1148,
    1152,  1153,  1157,  1158,  1166,  1167,  1171,  1172,  1176,  1181,
    1189,  1201,  1202,  1207,  1215,  1216,  1221
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

#define YYPACT_NINF -282

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-282)))

#define YYTABLE_NINF -169

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-169)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     130,   352,   -12,   -29,    43,    22,    37,  -282,  -282,  -282,
      73,    15,   -16,  -282,   253,  -282,  -282,  -282,  -282,  -282,
    -282,  -282,  -282,    80,   -17,   412,    93,    55,  -282,   126,
     111,    61,   132,    29,  -282,  -282,    77,  -282,   124,   352,
     352,  -282,   178,  -282,  -282,  -282,    -9,  -282,  -282,   432,
     432,   197,   201,   -26,  -282,   545,  -282,  -282,  -282,  -282,
    -282,  -282,  -282,    26,   186,   170,    27,    94,   193,   202,
      73,   253,  -282,  -282,   352,  -282,  -282,   432,   432,   212,
     210,   277,   555,   432,   432,   459,    71,  -282,    38,   412,
     189,   432,   432,   432,   432,   432,   432,   432,   432,   432,
     229,   218,   253,  -282,   -24,  -282,   221,   359,   245,   251,
     244,   226,   231,   234,   262,    94,  -282,    -5,   352,  -282,
    -282,    -7,   566,    -6,  -282,  -282,    -2,   449,    71,   432,
    -282,   195,   493,   175,   169,   290,   493,  -282,  -282,   359,
     228,   315,   315,   291,   291,   291,  -282,   277,   470,   336,
    -282,   420,   186,  -282,   437,   359,   359,   286,   504,   168,
    -282,  -282,  -282,  -282,  -282,  -282,  -282,  -282,   130,   304,
     109,  -282,   432,   243,   243,   432,   257,   566,  -282,  -282,
     349,   296,   351,  -282,    23,   175,   115,   290,   321,    38,
      38,   306,   270,   293,  -282,   246,   298,   300,  -282,    23,
     168,   331,   335,  -282,     4,  -282,  -282,   376,   566,  -282,
     483,   292,   168,   352,   314,  -282,   356,   359,   359,   359,
     432,    72,    28,  -282,  -282,   566,   388,  -282,  -282,   566,
    -282,   389,   385,   394,   395,   414,   165,  -282,   150,   290,
     492,    38,   327,  -282,  -282,    38,  -282,    38,   432,   359,
     366,  -282,   499,   376,  -282,   173,   376,  -282,  -282,  -282,
     415,   430,    68,    68,   112,   527,  -282,  -282,   372,  -282,
     448,   436,   448,   448,  -282,   395,   192,  -282,    23,   359,
     456,  -282,  -282,    38,  -282,   492,   368,   168,   421,   431,
    -282,  -282,  -282,   209,   352,   352,   432,   422,   366,  -282,
       8,   448,    10,    13,  -282,   168,   448,  -282,  -282,   432,
     442,  -282,  -282,   239,   242,   566,   432,    34,  -282,   501,
      14,  -282,  -282,    25,   232,   399,  -282,  -282,  -282,  -282,
     368,   258,   258,   491,  -282,  -282,  -282,  -282,  -282,   434,
     432,   276,   426,   438,   343,  -282,  -282,  -282,  -282,    75,
    -282,  -282,   482,  -282,  -282,  -282,  -282,  -282,  -282,   276,
    -282
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
       0,     0,     0,   114,    60,    62,    69,    70,    71,    72,
      73,    74,    75,     0,     0,     0,     0,     0,     0,     0,
       0,    15,     3,    55,     0,    51,    52,     0,     0,     0,
      79,    90,     0,     0,     0,     0,    98,    96,     0,     0,
     152,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    46,    79,   152,    41,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    29,    30,     0,     0,    16,
      53,     0,    66,     0,    65,    68,     0,     0,    98,     0,
      95,     0,   144,     0,   115,   116,   144,   122,    61,     0,
     170,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      63,     0,     0,    40,     0,     0,     0,     0,   155,    37,
      26,    27,    33,    34,    35,    36,    32,    31,     0,     0,
       0,    59,     0,   100,   100,     0,     0,    99,    94,   145,
       0,     0,     0,   119,   118,     0,     0,     0,   122,     0,
       0,     0,   133,   135,   139,     0,   138,     0,   121,   120,
     153,     0,   172,    48,     0,    47,    42,     0,    43,    44,
     155,     0,   156,     0,     0,   169,     0,     0,     0,     0,
       0,     0,     0,    24,    18,    67,     0,    91,    92,    97,
      93,     0,     0,     0,   150,     0,     0,   124,   127,   117,
       0,     0,     0,   134,   136,     0,   137,     0,     0,     0,
     174,    45,     0,     0,   154,     0,     0,   166,   165,   167,
       0,     0,   157,   158,   159,     0,    23,    22,   103,   101,
       0,   148,     0,     0,   142,   150,   124,   123,     0,     0,
       0,   132,   128,     0,   130,     0,   171,   173,     0,   176,
      49,    50,   164,     0,     0,     0,     0,     0,   174,   125,
       0,     0,     0,     0,   143,   141,     0,   129,   131,     0,
       0,    56,   162,     0,     0,   160,     0,   105,   146,     0,
       0,   147,   151,     0,   181,   175,   178,   177,   161,   163,
     104,     0,     0,     0,   126,   149,   140,   182,   183,   184,
       0,     0,     0,     0,     0,   106,   109,   107,   102,     0,
     180,   179,     0,   110,   111,   112,   113,   185,   186,     0,
     108
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -282,   360,     5,  -282,     2,  -282,  -282,   462,  -282,  -282,
    -282,  -282,  -282,   418,  -282,  -282,  -282,  -282,   391,  -282,
    -282,  -282,  -282,  -282,  -282,  -282,   457,   -77,   -25,  -144,
     -43,  -282,  -282,  -282,  -282,  -282,   460,   -66,   419,   374,
    -282,  -282,  -282,   217,  -281,  -282,  -282,   -84,   396,  -113,
    -110,   370,   297,  -128,  -127,   309,   486,  -137,  -282,  -282,
    -282,  -282,   299,  -282,  -282,   256,  -282,  -282
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    11,    36,    13,    14,    15,    33,    34,    16,    17,
      31,    68,   115,   116,    18,    29,    19,   104,   105,    20,
     204,    21,    74,    22,    25,    53,    54,   121,   158,    56,
      57,    58,    59,    60,    61,    62,    86,    87,   131,   227,
     269,   298,   333,   345,   346,    90,   134,   187,   136,   300,
     137,   197,   281,   183,   184,   274,   140,   159,   260,   216,
     202,   250,   289,   311,   325,   326,   339,   350
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      55,   123,   200,    23,   135,    12,   126,   205,   198,   199,
      78,   171,   173,    88,   168,    35,   174,   139,   211,   212,
     130,   106,   251,   188,    81,    82,   318,    85,   321,   234,
      24,   322,   335,    26,     1,    71,     2,     3,     4,     5,
       6,    75,    76,   336,   132,     1,   267,     2,     1,    27,
       2,     6,   122,   122,     6,   108,   109,   133,   122,   127,
     352,    28,   130,   235,    55,   102,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   188,   120,    30,   360,    32,
     262,   263,   264,    89,    42,   152,     7,     8,     9,   101,
     266,     1,    37,     2,     3,     4,     5,     6,    41,    63,
     169,    79,   172,   172,   177,   239,   240,   172,   291,   106,
     277,   278,   287,   252,   219,   331,   332,   319,   220,   319,
     170,    10,   319,   319,    64,   110,   111,   224,   112,   208,
     210,   113,    65,   237,   319,   186,    37,    66,    70,    38,
      39,    40,   305,     7,     8,     9,    84,   225,   129,     1,
     229,     2,     3,     4,     5,     6,  -144,   282,  -169,   302,
     303,   284,  -169,   285,   357,   358,   327,    67,    38,    39,
      40,   286,    69,   222,    38,    39,    40,   179,    10,    73,
     180,   132,    82,   276,   181,    72,   182,   236,   320,   149,
    -144,   292,   103,   323,   185,   265,     2,    77,   114,   307,
       6,     7,     8,     9,    43,    44,    45,    80,    47,   186,
     -11,   107,   217,   218,   219,   255,    83,    49,   220,   117,
      50,   118,   124,   122,    38,    39,    40,   312,    82,    78,
     139,    82,    38,    39,    40,   150,    10,   151,   154,   330,
      91,    92,    93,    94,    95,    96,    97,    98,   160,    99,
     162,   -11,   -11,   -11,    51,    23,   161,   328,   293,   163,
     329,    43,    44,    45,    80,    47,   164,   166,    38,    39,
      40,   315,   165,   178,    49,    52,    84,    50,   189,    43,
      44,    45,    80,    47,   324,    91,    92,    93,    94,    95,
      96,   122,    49,   201,    99,    50,   313,   314,    38,    39,
      40,    38,    39,    40,    96,   213,   344,   344,   341,   223,
     254,    51,    38,    39,    40,   324,   344,    43,    44,    45,
      80,    47,   226,   337,   338,    93,    94,    95,    96,    51,
      49,   232,    52,   256,   344,   230,   217,   218,   219,   238,
     245,   342,   220,   343,    91,    92,    93,    94,    95,    96,
      52,    91,    92,    93,    94,    95,    96,    97,    98,   342,
      99,   343,    43,    44,    45,    80,    47,    51,   243,   257,
     258,     1,   259,     2,   231,    49,   233,     6,   155,    43,
      44,    45,    80,    47,   190,   191,   192,   193,    52,   194,
     195,   244,    49,   196,   247,   253,   246,     2,   248,   249,
     241,     6,   192,   193,   261,   194,   156,   268,   270,   196,
     271,   157,    51,   272,   273,    43,    44,    45,    46,    47,
     275,   283,    48,    43,    44,    45,   203,   355,    49,    51,
     356,    50,   288,    52,   294,    43,    44,    45,    80,    47,
      43,    44,    45,    80,    47,    43,    44,    45,    49,   295,
      52,    50,   297,    49,   299,   301,   207,    91,    92,    93,
      94,    95,    96,    97,    98,    51,    99,    91,    92,    93,
      94,    95,    96,    97,    98,   306,    99,   172,    91,    92,
      93,    94,    95,    96,    97,    51,    52,    99,   309,   316,
      51,    91,    92,    93,    94,    95,    96,    97,    98,   310,
     214,   125,    43,    44,    45,   290,    52,   334,   340,   348,
     353,    52,    91,    92,    93,    94,    95,    96,    97,    98,
     179,   214,   349,   180,   354,   175,   359,   181,   221,   182,
     215,  -168,   119,   167,    84,    91,    92,    93,    94,    95,
      96,    97,    98,   206,    99,   128,   138,   176,   228,   347,
     209,   215,  -168,    91,    92,    93,    94,    95,    96,    97,
      98,   242,    99,    91,    92,    93,    94,    95,    96,    97,
      98,   296,    99,   125,    91,    92,    93,    94,    95,    96,
      97,    98,   308,    99,   304,   100,   190,   191,   192,   193,
     153,   194,   195,   279,   280,   196,   351,   317
};

static const yytype_uint16 yycheck[] =
{
      25,    78,   139,     1,    88,     0,    83,   151,   136,   136,
      19,    18,    18,    39,    19,     0,    18,    41,   155,   156,
      86,    64,    18,   133,    49,    50,    18,    52,    18,     6,
      42,    18,    18,    62,    19,    33,    21,    22,    23,    24,
      25,    39,    40,    18,     6,    19,    18,    21,    19,     6,
      21,    25,    77,    78,    25,    28,    29,    19,    83,    84,
     341,    39,   128,    40,    89,    63,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   185,    74,    40,   359,     6,
     217,   218,   219,   109,   101,   109,    71,    72,    73,    63,
      18,    19,   108,    21,    22,    23,    24,    25,    18,     6,
     105,   110,   109,   109,   129,   189,   190,   109,   252,   152,
     238,   238,   249,   109,    46,    81,    82,   109,    50,   109,
     118,   106,   109,   109,    69,    31,    32,    18,    34,   154,
     155,    37,     6,    18,   109,   133,   108,    26,   109,    59,
      60,    61,   279,    71,    72,    73,    75,   172,    77,    19,
     175,    21,    22,    23,    24,    25,     6,   241,    46,   272,
     273,   245,    50,   247,    89,    90,   310,   106,    59,    60,
      61,   248,    40,   168,    59,    60,    61,    27,   106,    55,
      30,     6,   207,    18,    34,   108,    36,   185,   301,   214,
      40,    18,     6,   306,    19,   220,    21,    19,   104,   283,
      25,    71,    72,    73,     3,     4,     5,     6,     7,   207,
      18,    41,    44,    45,    46,   213,    19,    16,    50,    26,
      19,    19,    10,   248,    59,    60,    61,    18,   253,    19,
      41,   256,    59,    60,    61,     6,   106,    19,    17,   316,
       8,     9,    10,    11,    12,    13,    14,    15,     3,    17,
       6,    59,    60,    61,    53,   253,     5,    18,   256,    33,
      18,     3,     4,     5,     6,     7,    35,     5,    59,    60,
      61,   296,    38,    78,    16,    74,    75,    19,   109,     3,
       4,     5,     6,     7,   309,     8,     9,    10,    11,    12,
      13,   316,    16,    65,    17,    19,   294,   295,    59,    60,
      61,    59,    60,    61,    13,    19,   331,   332,    50,     5,
      18,    53,    59,    60,    61,   340,   341,     3,     4,     5,
       6,     7,    79,    91,    92,    10,    11,    12,    13,    53,
      16,    35,    74,    19,   359,    78,    44,    45,    46,    18,
      94,    83,    50,    85,     8,     9,    10,    11,    12,    13,
      74,     8,     9,    10,    11,    12,    13,    14,    15,    83,
      17,    85,     3,     4,     5,     6,     7,    53,    98,    55,
      56,    19,    58,    21,    25,    16,    25,    25,    19,     3,
       4,     5,     6,     7,    94,    95,    96,    97,    74,    99,
     100,    98,    16,   103,    94,    19,    98,    21,    67,    64,
      94,    25,    96,    97,    48,    99,    47,    19,    19,   103,
      25,    52,    53,    19,    19,     3,     4,     5,     6,     7,
       6,    94,    10,     3,     4,     5,     6,    84,    16,    53,
      87,    19,    66,    74,    19,     3,     4,     5,     6,     7,
       3,     4,     5,     6,     7,     3,     4,     5,    16,    19,
      74,    19,    80,    16,     6,    19,    19,     8,     9,    10,
      11,    12,    13,    14,    15,    53,    17,     8,     9,    10,
      11,    12,    13,    14,    15,    19,    17,   109,     8,     9,
      10,    11,    12,    13,    14,    53,    74,    17,    67,    67,
      53,     8,     9,    10,    11,    12,    13,    14,    15,    68,
      17,    18,     3,     4,     5,     6,    74,     6,   109,    18,
      84,    74,     8,     9,    10,    11,    12,    13,    14,    15,
      27,    17,    88,    30,    86,    76,    44,    34,   168,    36,
      47,    48,    70,   115,    75,     8,     9,    10,    11,    12,
      13,    14,    15,   152,    17,    85,    89,   128,   174,   332,
     154,    47,    48,     8,     9,    10,    11,    12,    13,    14,
      15,   191,    17,     8,     9,    10,    11,    12,    13,    14,
      15,    44,    17,    18,     8,     9,    10,    11,    12,    13,
      14,    15,   285,    17,   275,    40,    94,    95,    96,    97,
     104,    99,   100,   101,   102,   103,   340,   298
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
      19,    53,    74,   136,   137,   139,   140,   141,   142,   143,
     144,   145,   146,     6,    69,     6,    26,   106,   122,    40,
     109,   115,   108,    55,   133,   115,   115,    19,    19,   110,
       6,   139,   139,    19,    75,   139,   147,   148,    39,   109,
     156,     8,     9,    10,    11,    12,    13,    14,    15,    17,
      40,    63,   115,     6,   128,   129,   141,    41,    28,    29,
      31,    32,    34,    37,   104,   123,   124,    26,    19,   118,
     115,   138,   139,   138,    10,    18,   138,   139,   147,    77,
     148,   149,     6,    19,   157,   158,   159,   161,   137,    41,
     167,   139,   139,   139,   139,   139,   139,   139,   139,   139,
       6,    19,   109,   167,    17,    19,    47,    52,   139,   168,
       3,     5,     6,    33,    35,    38,     5,   124,    19,   105,
     115,    18,   109,    18,    18,    76,   149,   139,    78,    27,
      30,    34,    36,   164,   165,    19,   115,   158,   161,   109,
      94,    95,    96,    97,    99,   100,   103,   162,   164,   165,
     168,    65,   171,     6,   131,   140,   129,    19,   139,   159,
     139,   168,   168,    19,    17,    47,   170,    44,    45,    46,
      50,   112,   113,     5,    18,   139,    79,   150,   150,   139,
      78,    25,    35,    25,     6,    40,   115,    18,    18,   158,
     158,    94,   162,    98,    98,    94,    98,    94,    67,    64,
     172,    18,   109,    19,    18,   115,    19,    55,    56,    58,
     169,    48,   168,   168,   168,   139,    18,    18,    19,   151,
      19,    25,    19,    19,   166,     6,    18,   164,   165,   101,
     102,   163,   158,    94,   158,   158,   138,   168,    66,   173,
       6,   140,    18,   115,    19,    19,    44,    80,   152,     6,
     160,    19,   160,   160,   166,   168,    19,   158,   163,    67,
      68,   174,    18,   115,   115,   139,    67,   173,    18,   109,
     160,    18,    18,   160,   139,   175,   176,   140,    18,    18,
     138,    81,    82,   153,     6,    18,    18,    91,    92,   177,
     109,    50,    83,    85,   139,   154,   155,   154,    18,    88,
     178,   176,   155,    84,    86,    84,    87,    89,    90,    44,
     155
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
     144,   145,   145,   146,   146,   147,   147,   148,   149,   149,
     150,   150,   151,   152,   152,   153,   153,   153,   154,   154,
     155,   155,   155,   155,   156,   156,   157,   157,   158,   158,
     158,   158,   158,   158,   159,   160,   160,   161,   161,   161,
     161,   161,   161,   162,   162,   162,   162,   162,   162,   162,
     163,   163,   164,   164,   165,   165,   165,   165,   165,   165,
     166,   166,   167,   167,   168,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   168,   169,   169,   169,   170,   170,
     171,   171,   172,   172,   173,   173,   174,   174,   175,   175,
     176,   177,   177,   177,   178,   178,   178
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
       2,     5,     5,     5,     4,     2,     1,     4,     0,     2,
       0,     2,     5,     0,     3,     0,     2,     2,     4,     1,
       2,     2,     2,     2,     0,     2,     1,     3,     2,     2,
       2,     2,     1,     4,     3,     1,     3,     3,     4,     5,
       4,     5,     4,     1,     2,     1,     2,     2,     1,     1,
       4,     2,     3,     4,     0,     1,     5,     5,     3,     6,
       0,     3,     0,     2,     3,     1,     2,     3,     3,     3,
       5,     6,     5,     6,     4,     1,     1,     1,     0,     1,
       0,     3,     0,     2,     0,     3,     0,     2,     1,     3,
       3,     0,     1,     1,     0,     2,     2
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
#line 129 "postgres_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("stmtList::stmt"); 
				(yyval.list) = singleton((yyvsp[-1].node));
				postgresParseResult = (Node *) (yyval.list);	 
			}
#line 1670 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 135 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("stmtlist::stmtList::stmt");
				(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[-1].node));	
				postgresParseResult = (Node *) (yyval.list); 
			}
#line 1680 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 144 "postgres_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::dmlStmt");
            (yyval.node) = (yyvsp[0].node);
        }
#line 1689 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 149 "postgres_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::queryStmt");
            (yyval.node) = (yyvsp[0].node);
        }
#line 1698 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 154 "postgres_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::transactionIdentifier");
            (yyval.node) = (Node *) createTransactionStmt((yyvsp[0].stringVal));
        }
#line 1707 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 159 "postgres_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("stmt::withQuery"); 
			(yyval.node) = (yyvsp[0].node); 
		}
#line 1716 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 169 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::insertQuery"); }
#line 1722 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 170 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::deleteQuery"); }
#line 1728 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 171 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::updateQuery"); }
#line 1734 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 178 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::bracketedQuery"); (yyval.node) = (yyvsp[-1].node); }
#line 1740 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 179 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::selectQuery"); }
#line 1746 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 180 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::provStmt"); }
#line 1752 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 181 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::setOperatorQuery"); }
#line 1758 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 186 "postgres_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withQuery::withViewList::queryStmt");
			(yyval.node) = (Node *) createWithStmt((yyvsp[-1].list), (yyvsp[0].node));
		}
#line 1767 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 194 "postgres_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withViewList::list::view");
			(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
		}
#line 1776 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 199 "postgres_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withViewList::view");
			(yyval.list) = singleton((yyvsp[0].node));
		}
#line 1785 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 207 "postgres_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withView::ident::AS:queryStmt");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString((yyvsp[-4].stringVal)), (yyvsp[-1].node));
		}
#line 1794 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 214 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::BEGIN"); (yyval.stringVal) = strdup("TRANSACTION_BEGIN"); }
#line 1800 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 215 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::COMMIT"); (yyval.stringVal) = strdup("TRANSACTION_COMMIT"); }
#line 1806 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 216 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::ROLLBACK"); (yyval.stringVal) = strdup("TRANSACTION_ABORT"); }
#line 1812 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 224 "postgres_parser.y" /* yacc.c:1646  */
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
#line 1827 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 235 "postgres_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provStmt::stmtlist");
			ProvenanceStmt *p = createProvenanceStmt((Node *) (yyvsp[-1].list));
			p->inputType = PROV_INPUT_UPDATE_SEQUENCE;
			p->provType = PROV_PI_CS;
			p->asOf = (Node *) (yyvsp[-5].node);
			p->options = (yyvsp[-4].list);
			(yyval.node) = (Node *) p;
		}
#line 1841 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 245 "postgres_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provStmt::transaction");
			ProvenanceStmt *p = createProvenanceStmt((Node *) createConstString((yyvsp[0].stringVal)));
			p->inputType = PROV_INPUT_TRANSACTION;
			p->provType = PROV_PI_CS;
			p->options = (yyvsp[-3].list);
			(yyval.node) = (Node *) p;
		}
#line 1854 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 256 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalProvAsOf::EMPTY"); (yyval.node) = NULL; }
#line 1860 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 258 "postgres_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvAsOf::SCN");
			(yyval.node) = (Node *) createConstLong((yyvsp[0].intVal));
		}
#line 1869 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 263 "postgres_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvAsOf::TIMESTAMP");
			(yyval.node) = (Node *) createConstString((yyvsp[0].stringVal));
		}
#line 1878 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 270 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalProvWith::EMPTY"); (yyval.list) = NIL; }
#line 1884 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 272 "postgres_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvWith::WITH");
			(yyval.list) = (yyvsp[0].list);
		}
#line 1893 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 279 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("provOptionList::option"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 1899 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 281 "postgres_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("provOptionList::list"); 
			(yyval.list) = appendToTailOfList((yyvsp[-1].list),(yyvsp[0].node)); 
		}
#line 1908 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 289 "postgres_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("provOption::TYPE"); 
			(yyval.node) = (Node *) createStringKeyValue("TYPE", (yyvsp[0].stringVal)); 
		}
#line 1917 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 294 "postgres_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::TABLE");
			(yyval.node) = (Node *) createStringKeyValue("TABLE", (yyvsp[0].stringVal));
		}
#line 1926 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 299 "postgres_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::ONLY::UPDATED");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_ONLY_UPDATED), 
					(Node *) createConstBool(TRUE));
		}
#line 1936 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 305 "postgres_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::SHOW::INTERMEDIATE");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_SHOW_INTERMEDIATE), 
					(Node *) createConstBool(TRUE));
		}
#line 1946 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 311 "postgres_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::TUPLE::VERSIONS");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_TUPLE_VERSIONS),
					(Node *) createConstBool(TRUE));
		}
#line 1956 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 323 "postgres_parser.y" /* yacc.c:1646  */
    { 
             RULELOG("deleteQuery");
             (yyval.node) = (Node *) createDelete((yyvsp[-2].stringVal), (yyvsp[0].node));
         }
#line 1965 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 331 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::NULL"); (yyval.stringVal) = NULL; }
#line 1971 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 332 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::FROM"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 1977 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 347 "postgres_parser.y" /* yacc.c:1646  */
    { 
                RULELOG(updateQuery); 
                (yyval.node) = (Node *) createUpdate((yyvsp[-3].stringVal), (yyvsp[-1].list), (yyvsp[0].node)); 
            }
#line 1986 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 355 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setExpression");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 1995 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 360 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setClause::setExpression");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2004 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 368 "postgres_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal),"=")) {
                    RULELOG("setExpression::attributeRef::expression");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 2017 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 377 "postgres_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal), "=")) {
                    RULELOG("setExpression::attributeRef::queryStmt");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 2030 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 392 "postgres_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertQuery::insertList"); 
            	(yyval.node) = (Node *) createInsert((yyvsp[-4].stringVal),(Node *) (yyvsp[-1].list), NULL); 
        	}
#line 2039 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 397 "postgres_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("insertQuery::queryStmt");
                (yyval.node) = (Node *) createInsert((yyvsp[-1].stringVal), (yyvsp[0].node), NULL);
            }
#line 2048 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 405 "postgres_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::constant");
            	(yyval.list) = singleton((yyvsp[0].node)); 
            }
#line 2057 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 410 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("insertList::IDENTIFIER");
                (yyval.list) = singleton(createAttributeReference((yyvsp[0].stringVal)));
            }
#line 2066 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 415 "postgres_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("insertList::insertList::::IDENTIFIER");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), createAttributeReference((yyvsp[0].stringVal)));
            }
#line 2075 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 420 "postgres_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::insertList::constant");
            	(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2084 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 434 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::INTERSECT");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 2093 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 439 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::MINUS");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 2102 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 444 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::UNION");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-2].stringVal), ((yyvsp[-1].stringVal) != NULL), (yyvsp[-3].node), (yyvsp[0].node));
            }
#line 2111 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 451 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::NULL"); (yyval.stringVal) = NULL; }
#line 2117 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 452 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::ALLTRUE"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2123 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 462 "postgres_parser.y" /* yacc.c:1646  */
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
#line 2143 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 484 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalDistinct::NULL"); (yyval.node) = NULL; }
#line 2149 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 486 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT");
                (yyval.node) = (Node *) createDistinctClause(NULL);
            }
#line 2158 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 491 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT::exprList");
                (yyval.node) = (Node *) createDistinctClause((yyvsp[-1].list));
            }
#line 2167 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 503 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectItem"); (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2175 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 507 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectClause::selectItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node)); 
            }
#line 2184 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 515 "postgres_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression"); 
                 (yyval.node) = (Node *) createSelectItem(NULL, (yyvsp[0].node)); 
             }
#line 2193 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 520 "postgres_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression::identifier"); 
                 (yyval.node) = (Node *) createSelectItem((yyvsp[0].stringVal), (yyvsp[-2].node));
             }
#line 2202 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 525 "postgres_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
         		(yyval.node) = (Node *) createSelectItem(strdup("*"), NULL); 
     		}
#line 2211 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 530 "postgres_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
     			(yyval.node) = (Node *) createSelectItem(CONCAT_STRINGS((yyvsp[-2].stringVal),".*"), NULL); 
 			}
#line 2220 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 540 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("exprList::SINGLETON"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 2226 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 542 "postgres_parser.y" /* yacc.c:1646  */
    {
                  RULELOG("exprList::exprList::expression");
                  (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
             }
#line 2235 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 553 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::bracked"); (yyval.node) = (yyvsp[-1].node); }
#line 2241 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 554 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::constant"); }
#line 2247 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 555 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::attributeRef"); }
#line 2253 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 556 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlParameter"); }
#line 2259 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 557 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::binaryOperatorExpression"); }
#line 2265 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 558 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::unaryOperatorExpression"); }
#line 2271 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 559 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlFunctionCall"); }
#line 2277 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 560 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::case"); }
#line 2283 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 569 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::INT"); (yyval.node) = (Node *) createConstInt((yyvsp[0].intVal)); }
#line 2289 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 570 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::FLOAT"); (yyval.node) = (Node *) createConstFloat((yyvsp[0].floatVal)); }
#line 2295 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 571 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::STRING"); (yyval.node) = (Node *) createConstString((yyvsp[0].stringVal)); }
#line 2301 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 578 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("attributeRef::IDENTIFIER"); (yyval.node) = (Node *) createAttributeReference((yyvsp[0].stringVal)); }
#line 2307 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 584 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("sqlParameter::PARAMETER"); (yyval.node) = (Node *) createSQLParameter((yyvsp[0].stringVal)); }
#line 2313 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 604 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '+' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2324 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 611 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '-' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2335 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 618 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '*' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2346 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 625 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '/' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2357 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 632 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '%' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2368 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 639 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '^' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2379 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 648 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '&' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2390 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 655 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '|' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2401 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 664 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression::comparisonOps");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2412 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 674 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("unaryOperatorExpression:: '!' ");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2422 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 686 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("sqlFunctionCall::IDENTIFIER::exprList");
				FunctionCall *f = createFunctionCall((yyvsp[-4].stringVal), (yyvsp[-2].list));
				if ((yyvsp[0].node) != NULL)
					(yyval.node) = (Node *) createWindowFunction(f, (WindowDef *) (yyvsp[0].node));
				else  
                	(yyval.node) = (Node *) f; 
            }
#line 2435 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 695 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("sqlFunctionCall::AMMSC::exprList");
				FunctionCall *f = createFunctionCall((yyvsp[-4].stringVal), (yyvsp[-2].list));
				if ((yyvsp[0].node) != NULL)
					(yyval.node) = (Node *) createWindowFunction(f, (WindowDef *) (yyvsp[0].node));
				else  
                	(yyval.node) = (Node *) f; 
            }
#line 2448 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 710 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseExpression::CASE::expression::whens:else:END");
				(yyval.node) = (Node *) createCaseExpr((yyvsp[-3].node), (yyvsp[-2].list), (yyvsp[-1].node));
			}
#line 2457 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 715 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseExpression::CASE::whens::else::END");
				(yyval.node) = (Node *) createCaseExpr(NULL, (yyvsp[-2].list), (yyvsp[-1].node));
			}
#line 2466 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 723 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhenList::list::caseWhen");
				(yyval.list) = appendToTailOfList((yyvsp[-1].list), (yyvsp[0].node));
			}
#line 2475 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 728 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhenList::caseWhen");
				(yyval.list) = singleton((yyvsp[0].node));
			}
#line 2484 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 736 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhen::WHEN::expression::THEN::expression");
				(yyval.node) = (Node *) createCaseWhen((yyvsp[-2].node),(yyvsp[0].node));
			}
#line 2493 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 743 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalCaseElse::NULL"); (yyval.node) = NULL; }
#line 2499 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 745 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalCaseElse::ELSE::expression");
				(yyval.node) = (yyvsp[0].node);
			}
#line 2508 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 755 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("overclause::NULL"); (yyval.node) = NULL; }
#line 2514 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 757 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("overclause::window");
				(yyval.node) = (yyvsp[0].node);
			}
#line 2523 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 765 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("window");
				(yyval.node) = (Node *) createWindowDef((yyvsp[-3].list),(yyvsp[-2].list), (WindowFrame *) (yyvsp[-1].node));
			}
#line 2532 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 772 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optWindowPart::NULL"); (yyval.list) = NIL; }
#line 2538 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 774 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optWindowPart::PARTITION:BY::expressionList");
				(yyval.list) = (yyvsp[0].list);
			}
#line 2547 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 781 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optWindowFrame::NULL"); (yyval.node) = NULL; }
#line 2553 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 783 "postgres_parser.y" /* yacc.c:1646  */
    { 
				WindowBound *l, *u = NULL;
				RULELOG("optWindowFrame::ROWS::windoBoundaries");
				l = getNthOfListP((yyvsp[0].list), 0);
				if(LIST_LENGTH((yyvsp[0].list)) > 1)
					u = getNthOfListP((yyvsp[0].list), 1);
				(yyval.node) = (Node *) createWindowFrame(WINFRAME_ROWS, l, u); 
			}
#line 2566 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 792 "postgres_parser.y" /* yacc.c:1646  */
    {
				WindowBound *l, *u = NULL; 
				RULELOG("optWindowFrame::RANGE::windoBoundaries"); 
				l = getNthOfListP((yyvsp[0].list), 0);
				if(LIST_LENGTH((yyvsp[0].list)) > 1)
					u = getNthOfListP((yyvsp[0].list), 1);
				(yyval.node) = (Node *) createWindowFrame(WINFRAME_RANGE, l, u); 
			}
#line 2579 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 804 "postgres_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBoundaries::BETWEEN"); 
				(yyval.list) = LIST_MAKE((yyvsp[-2].node), (yyvsp[0].node)); 
			}
#line 2588 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 809 "postgres_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBoundaries::windowBound"); 
				(yyval.list) = singleton((yyvsp[0].node)); 
			}
#line 2597 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 817 "postgres_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::UNBOUNDED::PRECEDING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_UNBOUND_PREC, NULL); 
			}
#line 2606 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 822 "postgres_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::CURRENT::ROW"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_CURRENT_ROW, NULL); 
			}
#line 2615 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 827 "postgres_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::expression::PRECEDING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_EXPR_PREC, (yyvsp[-1].node)); 
			}
#line 2624 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 832 "postgres_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::expression::FOLLOWING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_EXPR_FOLLOW, (yyvsp[-1].node)); 
			}
#line 2633 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 844 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::NULL"); (yyval.list) = NULL; }
#line 2639 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 845 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::fromClause"); (yyval.list) = (yyvsp[0].list); }
#line 2645 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 850 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClauseItem");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2654 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 855 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClause::fromClauseItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2663 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 864 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
				FromItem *f = createFromTableRef(NULL, NIL, (yyvsp[-1].stringVal));
				f->provInfo = (FromProvInfo *) (yyvsp[0].node);
                (yyval.node) = (Node *) f;
            }
#line 2674 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 871 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
                FromItem *f = createFromTableRef(((FromItem *) (yyvsp[0].node))->name, 
						((FromItem *) (yyvsp[0].node))->attrNames, (yyvsp[-1].stringVal));
				f->provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
                (yyval.node) = (Node *) f;
            }
#line 2686 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 880 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromItem *f = (FromItem *) (yyvsp[-1].node);
                f->provInfo = (FromProvInfo *) (yyvsp[0].node);
                (yyval.node) = (yyvsp[-1].node);
            }
#line 2697 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 887 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromSubquery *s = (FromSubquery *) (yyvsp[-1].node);
                s->from.name = ((FromItem *) (yyvsp[0].node))->name;
                s->from.attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
                s->from.provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
                (yyval.node) = (Node *) s;
            }
#line 2710 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 896 "postgres_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[0].node);
        		f->name = NULL;
        		(yyval.node) = (Node *) f;
        	}
#line 2722 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 904 "postgres_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[-2].node);
        		f->name = ((FromItem *) (yyvsp[0].node))->name;
                f->attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
                f->provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
        		(yyval.node) = (Node *) f;
        	}
#line 2736 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 917 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("subQuery::queryStmt");
                (yyval.node) = (Node *) createFromSubquery(NULL, NULL, (yyvsp[-1].node));
            }
#line 2745 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 924 "postgres_parser.y" /* yacc.c:1646  */
    { (yyval.list) = singleton((yyvsp[0].stringVal)); }
#line 2751 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 925 "postgres_parser.y" /* yacc.c:1646  */
    { (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].stringVal)); }
#line 2757 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 929 "postgres_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2763 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 931 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURAL");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
						(FromItem *) (yyvsp[0].node), "JOIN_INNER", "JOIN_COND_NATURAL", 
						NULL);
          	}
#line 2774 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 938 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURALjoinType");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[0].node), (yyvsp[-2].stringVal), "JOIN_COND_NATURAL", NULL);
          	}
#line 2784 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 944 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::CROSS JOIN");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[0].node), "JOIN_CROSS", "JOIN_COND_ON", NULL);
          	}
#line 2794 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 950 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinType::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON";
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[-1].node), (yyvsp[-3].stringVal), condType, (yyvsp[0].node));
          	}
#line 2806 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 958 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON"; 
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[-1].node), "JOIN_INNER", 
                		condType, (yyvsp[0].node));
          	}
#line 2819 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 969 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2825 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 970 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT OUTER"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2831 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 971 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT "); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2837 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 972 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT OUTER"); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2843 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 973 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL OUTER"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2849 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 974 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2855 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 975 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::INNER"); (yyval.stringVal) = "JOIN_INNER"; }
#line 2861 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 979 "postgres_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (Node *) (yyvsp[-1].list); }
#line 2867 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 980 "postgres_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2873 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 985 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[-2].node);
				(yyval.node) = (Node *) f;
			}
#line 2884 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 992 "postgres_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[-3].node); 
				(yyval.node) = (Node *) f;
			}
#line 2895 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 1001 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFromProv::empty"); (yyval.node) = NULL; }
#line 2901 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1003 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::BASERELATION");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = TRUE;
				p->userProvAttrs = NIL;				 
				(yyval.node) = (Node *) p; 
			}
#line 2913 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1011 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::userProvAttr");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = FALSE;
				p->userProvAttrs = (yyvsp[-1].list);				 
				(yyval.node) = (Node *) p; 
			}
#line 2925 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1019 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::userProvDupAttr");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = TRUE;
				p->userProvAttrs = (yyvsp[-1].list);
				(yyval.node) = (Node *) p;
			}
#line 2937 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1027 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::intermediateProv");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->intermediateProv = TRUE;
				(yyval.node) = (Node *) p;
			}
#line 2948 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1034 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::intermediateProv::attrList");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->intermediateProv = TRUE;
				p->userProvAttrs = (yyvsp[-1].list);
				(yyval.node) = (Node *) p;
			}
#line 2960 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1044 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAttrAlias::empty"); (yyval.list) = NULL; }
#line 2966 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1046 "postgres_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAttrAlias::identifierList"); (yyval.list) = (yyvsp[-1].list); 
			}
#line 2974 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1055 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::NULL"); (yyval.node) = NULL; }
#line 2980 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1056 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::whereExpression"); (yyval.node) = (yyvsp[0].node); }
#line 2986 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1060 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("where::brackedWhereExpression"); (yyval.node) = (yyvsp[-1].node); }
#line 2992 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1061 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("whereExpression::expression"); (yyval.node) = (yyvsp[0].node); }
#line 2998 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 1063 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::NOT");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 3008 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 1069 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 3019 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1076 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::OR");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 3030 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1083 "postgres_parser.y" /* yacc.c:1646  */
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
#line 3052 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 1101 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::BETWEEN-AND");
                List *expr = singleton((yyvsp[-4].node));
                expr = appendToTailOfList(expr, (yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr);
            }
#line 3064 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1109 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::comparisonOps::nestedSubQueryOperator::Subquery");
                (yyval.node) = (Node *) createNestedSubquery((yyvsp[-3].stringVal), (yyvsp[-5].node), (yyvsp[-4].stringVal), (yyvsp[-1].node));
            }
#line 3073 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1114 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::Subquery");
                Node *q = (Node *) createNestedSubquery("SCALAR", NULL, NULL, (yyvsp[-1].node)); 
                List *expr = LIST_MAKE((yyvsp[-4].node), q);
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr); 
            }
#line 3084 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1121 "postgres_parser.y" /* yacc.c:1646  */
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
#line 3101 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 1134 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::EXISTS");
                (yyval.node) = (Node *) createNestedSubquery((yyvsp[-3].stringVal), NULL, NULL, (yyvsp[-1].node));
            }
#line 3110 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 1141 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ANY"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 3116 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 1142 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ALL"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 3122 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1143 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::SOME"); (yyval.stringVal) = "ANY"; }
#line 3128 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 1147 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NULL"); (yyval.stringVal) = NULL; }
#line 3134 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 1148 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NOT"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 3140 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 1152 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::NULL"); (yyval.list) = NULL; }
#line 3146 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 1153 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::GROUPBY"); (yyval.list) = (yyvsp[0].list); }
#line 3152 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 1157 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.node) = NULL; }
#line 3158 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 1159 "postgres_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("optionalHaving::HAVING"); 
                (yyval.node) = (Node *) (yyvsp[0].node);
            }
#line 3167 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 1166 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.list) = NULL; }
#line 3173 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 1167 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy::ORDERBY"); (yyval.list) = (yyvsp[0].list); }
#line 3179 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 1171 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::NULL"); (yyval.node) = NULL; }
#line 3185 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 1172 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::CONSTANT"); (yyval.node) = (yyvsp[0].node);}
#line 3191 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 1177 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::orderExpr");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 3200 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 1182 "postgres_parser.y" /* yacc.c:1646  */
    {
                RULELOG("orderList::orderList::orderExpr");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 3209 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 1190 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("orderExpr::expr::sortOrder::nullOrder");
				SortOrder o = (strcmp((yyvsp[-1].stringVal),"ASC") == 0) ?  SORT_ASC : SORT_DESC;
				SortNullOrder n = (strcmp((yyvsp[0].stringVal),"NULLS_FIRST") == 0) ? 
						SORT_NULLS_FIRST : 
						SORT_NULLS_LAST;
				(yyval.node) = (Node *) createOrderExpr((yyvsp[-2].node), o, n);
			}
#line 3222 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 1201 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalSortOrder::empty"); (yyval.stringVal) = "ASC"; }
#line 3228 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 1203 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalSortOrder::ASC");
				(yyval.stringVal) = "ASC";
			}
#line 3237 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 1208 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalSortOrder::DESC");
				(yyval.stringVal) = "DESC";
			}
#line 3246 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 1215 "postgres_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNullOrder::empty"); (yyval.stringVal) = "NULLS_LAST"; }
#line 3252 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 1217 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalNullOrder::NULLS FIRST");
				(yyval.stringVal) = "NULLS_FIRST";
			}
#line 3261 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;

  case 186:
#line 1222 "postgres_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalNullOrder::NULLS LAST");
				(yyval.stringVal) = "NULLS_LAST";
			}
#line 3270 "postgres_parser.tab.c" /* yacc.c:1646  */
    break;


#line 3274 "postgres_parser.tab.c" /* yacc.c:1646  */
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
#line 1229 "postgres_parser.y" /* yacc.c:1906  */




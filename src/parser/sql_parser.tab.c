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
    ROWNUM = 298,
    ALL = 299,
    ANY = 300,
    IS = 301,
    SOME = 302,
    UNION = 303,
    INTERSECT = 304,
    MINUS = 305,
    INTO = 306,
    VALUES = 307,
    HAVING = 308,
    GROUP = 309,
    ORDER = 310,
    BY = 311,
    LIMIT = 312,
    SET = 313,
    INT = 314,
    BEGIN_TRANS = 315,
    COMMIT_TRANS = 316,
    ROLLBACK_TRANS = 317,
    CASE = 318,
    WHEN = 319,
    THEN = 320,
    ELSE = 321,
    END = 322,
    OVER_TOK = 323,
    PARTITION = 324,
    ROWS = 325,
    RANGE = 326,
    UNBOUNDED = 327,
    PRECEDING = 328,
    CURRENT = 329,
    ROW = 330,
    FOLLOWING = 331,
    NULLS = 332,
    FIRST = 333,
    LAST = 334,
    ASC = 335,
    DESC = 336,
    DUMMYEXPR = 337,
    JOIN = 338,
    NATURAL = 339,
    LEFT = 340,
    RIGHT = 341,
    OUTER = 342,
    INNER = 343,
    CROSS = 344,
    ON = 345,
    USING = 346,
    FULL = 347,
    TYPE = 348,
    TRANSACTION = 349,
    WITH = 350,
    XOR = 351
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

#line 236 "sql_parser.tab.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SQL_PARSER_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 251 "sql_parser.tab.c" /* yacc.c:358  */

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
#define YYLAST   652

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  112
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  69
/* YYNRULES -- Number of rules.  */
#define YYNRULES  187
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  365

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   351

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
      19,    18,    10,     8,   110,     9,   111,    11,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   109,
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
     107,   108
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   134,   134,   140,   149,   154,   159,   164,   175,   176,
     177,   184,   185,   186,   187,   191,   199,   204,   212,   220,
     221,   222,   229,   240,   250,   262,   263,   268,   276,   277,
     285,   286,   294,   299,   304,   310,   316,   328,   337,   338,
     352,   360,   365,   373,   382,   397,   402,   416,   417,   475,
     480,   485,   493,   494,   503,   526,   527,   532,   544,   548,
     556,   561,   566,   571,   582,   583,   595,   596,   597,   598,
     599,   600,   601,   602,   603,   612,   613,   614,   621,   627,
     646,   653,   660,   667,   674,   681,   690,   697,   706,   716,
     728,   737,   752,   757,   765,   770,   778,   786,   787,   798,
     799,   807,   815,   816,   824,   825,   834,   846,   851,   859,
     864,   869,   874,   887,   888,   892,   897,   906,   913,   922,
     929,   938,   946,   959,   967,   968,   972,   973,   980,   986,
     992,  1000,  1012,  1013,  1014,  1015,  1016,  1017,  1018,  1022,
    1023,  1027,  1034,  1044,  1045,  1053,  1061,  1069,  1076,  1087,
    1088,  1098,  1099,  1103,  1104,  1105,  1111,  1118,  1125,  1143,
    1151,  1156,  1163,  1176,  1184,  1185,  1186,  1190,  1191,  1195,
    1196,  1200,  1201,  1209,  1210,  1214,  1215,  1219,  1224,  1232,
    1244,  1245,  1250,  1258,  1259,  1264,  1272,  1273
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
  "EXISTS", "AMMSC", "NULLVAL", "ROWNUM", "ALL", "ANY", "IS", "SOME",
  "UNION", "INTERSECT", "MINUS", "INTO", "VALUES", "HAVING", "GROUP",
  "ORDER", "BY", "LIMIT", "SET", "INT", "BEGIN_TRANS", "COMMIT_TRANS",
  "ROLLBACK_TRANS", "CASE", "WHEN", "THEN", "ELSE", "END", "OVER_TOK",
  "PARTITION", "ROWS", "RANGE", "UNBOUNDED", "PRECEDING", "CURRENT", "ROW",
  "FOLLOWING", "NULLS", "FIRST", "LAST", "ASC", "DESC", "DUMMYEXPR",
  "JOIN", "NATURAL", "LEFT", "RIGHT", "OUTER", "INNER", "CROSS", "ON",
  "USING", "FULL", "TYPE", "TRANSACTION", "WITH", "XOR", "';'", "','",
  "'.'", "$accept", "stmtList", "stmt", "dmlStmt", "queryStmt",
  "withQuery", "withViewList", "withView", "transactionIdentifier",
  "provStmt", "optionalProvAsOf", "optionalProvWith", "provOptionList",
  "provOption", "deleteQuery", "fromString", "updateQuery", "setClause",
  "setExpression", "insertQuery", "insertContent", "setOperatorQuery",
  "optionalAll", "selectQuery", "optionalDistinct", "selectClause",
  "selectItem", "exprList", "expression", "constant", "attributeRef",
  "sqlParameter", "binaryOperatorExpression", "unaryOperatorExpression",
  "sqlFunctionCall", "caseExpression", "caseWhenList", "caseWhen",
  "optionalCaseElse", "overClause", "windowSpec", "optWindowPart",
  "optWindowFrame", "windowBoundaries", "windowBound", "optionalFrom",
  "fromClause", "fromClauseItem", "subQuery", "identifierList",
  "fromJoinItem", "joinType", "joinCond", "optionalAlias",
  "optionalFromProv", "optionalAttrAlias", "optionalWhere",
  "whereExpression", "nestedSubQueryOperator", "optionalNot",
  "optionalGroupBy", "optionalHaving", "optionalOrderBy", "optionalLimit",
  "orderList", "orderExpr", "optionalSortOrder", "optionalNullOrder",
  "delimIdentifier", YY_NULLPTR
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
     343,   344,   345,   346,   347,   348,   349,   350,   351,    59,
      44,    46
};
# endif

#define YYPACT_NINF -303

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-303)))

#define YYTABLE_NINF -168

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-168)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     147,   193,    22,   -30,    46,    18,    41,  -303,  -303,  -303,
      80,    15,   -65,  -303,   163,  -303,  -303,  -303,  -303,  -303,
    -303,  -303,  -303,    38,    43,   423,    86,    42,  -303,    91,
     126,    48,   122,    30,  -303,  -303,    54,  -303,   123,   193,
     193,  -303,   172,  -303,  -303,  -303,    -9,  -303,  -303,   440,
     440,   175,  -303,   341,   -19,  -303,   566,  -303,  -303,  -303,
    -303,  -303,  -303,  -303,    95,   189,   159,    98,   -15,   177,
     185,    80,   163,  -303,  -303,   193,  -303,  -303,   440,   440,
     200,   196,   619,   577,   440,   440,   471,    55,  -303,    44,
     423,   176,   440,   440,   440,   440,   440,   440,   440,   440,
     440,   225,   289,   226,   163,  -303,  -303,     7,  -303,   222,
     359,   257,   256,   270,   244,   248,   241,   280,   -15,  -303,
     -10,   193,  -303,  -303,    -5,   588,    -4,  -303,  -303,     0,
     458,    55,   440,  -303,   207,   124,   334,   178,   542,   124,
    -303,  -303,   359,   228,   479,   479,   286,   286,   286,  -303,
     619,   609,   639,  -303,  -303,     5,   198,   440,   189,  -303,
     446,   359,   359,   294,   514,   130,  -303,  -303,  -303,  -303,
    -303,  -303,  -303,  -303,   147,   310,   146,  -303,   440,   236,
     236,   440,   246,   588,  -303,  -303,   299,   291,   303,  -303,
      40,   334,   174,   542,   323,    44,    44,   279,   250,   251,
    -303,   247,   252,   261,  -303,    40,   130,   290,   296,   109,
     361,   367,     8,  -303,   383,   588,  -303,   494,   183,   130,
     193,   365,  -303,   332,   359,   359,   359,   440,   125,    -6,
    -303,  -303,   588,   363,  -303,  -303,   588,  -303,   366,   368,
     372,   373,   389,   191,  -303,   533,   542,   512,    44,   302,
    -303,  -303,    44,  -303,    44,   440,   359,   331,  -303,   198,
    -303,  -303,   383,  -303,   195,   383,  -303,  -303,  -303,   381,
     382,    12,    12,    32,   540,  -303,  -303,   324,  -303,   361,
     384,   361,   361,  -303,   373,   208,  -303,    40,   359,   390,
    -303,  -303,    44,  -303,   512,   305,   130,   345,   350,  -303,
     220,   193,   193,   440,   355,   331,     9,   361,    11,    14,
    -303,   130,   361,  -303,  -303,   440,   238,  -303,  -303,   240,
     245,   588,   440,     1,  -303,    27,  -303,  -303,    29,   173,
     315,  -303,  -303,  -303,  -303,   305,   243,   243,   413,  -303,
    -303,  -303,  -303,   343,   440,   268,   352,   348,   322,  -303,
    -303,  -303,  -303,    51,  -303,  -303,   397,  -303,  -303,  -303,
    -303,  -303,  -303,   268,  -303
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    55,     0,     0,    38,    25,    19,    20,    21,
       0,     0,     0,     4,     5,     7,     6,    13,     9,    10,
       8,    14,    12,     0,    56,     0,     0,     0,    39,     0,
       0,    28,     0,     0,    17,     1,     0,     2,    52,     0,
       0,    11,     0,    75,    76,    77,    78,    79,    62,     0,
       0,     0,    74,     0,   113,    58,    60,    67,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
       0,     0,    15,     3,    53,     0,    49,    50,     0,     0,
       0,    78,    89,     0,     0,     0,     0,    97,    95,     0,
       0,   151,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,    45,    78,   151,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    29,    30,
       0,     0,    16,    51,     0,    64,     0,    63,    66,     0,
       0,    97,     0,    94,     0,   143,     0,   114,   115,   143,
     121,    59,     0,   169,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    61,   186,     0,   124,     0,     0,    40,
       0,     0,     0,     0,   154,    37,    26,    27,    33,    34,
      35,    36,    32,    31,     0,     0,     0,    57,     0,    99,
      99,     0,     0,    98,    93,   144,     0,     0,     0,   118,
     117,     0,     0,     0,   121,     0,     0,     0,   132,   134,
     138,     0,   137,     0,   120,   119,   152,     0,   171,     0,
       0,     0,     0,    42,     0,    43,    44,   154,     0,   155,
       0,     0,   168,     0,     0,     0,     0,     0,     0,     0,
      24,    18,    65,     0,    90,    91,    96,    92,     0,     0,
       0,   149,     0,     0,   123,   126,   116,     0,     0,     0,
     133,   135,     0,   136,     0,     0,     0,   173,    46,   125,
     187,    47,     0,   153,     0,     0,   165,   164,   166,     0,
       0,   156,   157,   158,     0,    23,    22,   102,   100,     0,
     147,     0,     0,   141,   149,   123,   122,     0,     0,     0,
     131,   127,     0,   129,     0,   170,   172,     0,   175,   163,
       0,     0,     0,     0,     0,   173,     0,     0,     0,     0,
     142,   140,     0,   128,   130,     0,     0,    54,   161,     0,
       0,   159,     0,   104,   145,     0,   146,   150,     0,   180,
     174,   177,   176,   160,   162,   103,     0,     0,     0,   148,
     139,   181,   182,   183,     0,     0,     0,     0,     0,   105,
     108,   106,   101,     0,   179,   178,     0,   109,   110,   111,
     112,   184,   185,     0,   107
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -303,   274,     4,  -303,     2,  -303,  -303,   386,  -303,  -303,
    -303,  -303,  -303,   336,  -303,  -303,  -303,  -303,   297,  -303,
     254,  -303,  -303,  -303,  -303,  -303,   370,   -78,   -25,   145,
     -57,  -303,  -303,  -303,  -303,  -303,   378,    -2,   346,   307,
    -303,  -303,  -303,   137,  -302,  -303,  -303,   -87,   337,    10,
    -115,   313,   202,  -134,  -132,   210,   393,  -131,  -303,  -303,
    -303,  -303,   209,  -303,  -303,   169,  -303,  -303,   306
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    11,    36,    13,    14,    15,    33,    34,    16,    17,
      31,    69,   118,   119,    18,    29,    19,   107,   108,    20,
     105,    21,    75,    22,    25,    54,    55,   124,   164,    57,
      58,    59,    60,    61,    62,    63,    87,    88,   134,   234,
     278,   305,   338,   349,   350,    91,   137,   193,   139,   155,
     140,   203,   290,   189,   190,   283,   143,   165,   269,   223,
     208,   257,   298,   317,   330,   331,   343,   354,   156
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      56,   126,   138,    23,    12,   204,   129,   205,   109,   174,
      79,   206,   276,   177,   179,    35,   113,   114,   180,   115,
      89,   194,   116,   209,    82,    83,   261,   324,    86,   326,
     218,   219,   327,    26,     1,    72,     2,     3,     4,     5,
       6,    76,    77,   356,    37,   339,   241,   340,   142,     1,
     135,     2,    27,   125,   125,     6,    41,    28,   226,   125,
     130,   364,   227,   136,    24,    56,   104,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   194,   123,  -168,   212,
     242,    30,  -168,   336,   337,   133,    32,     7,     8,     9,
     117,    90,    64,   271,   272,   273,   175,    66,    38,    39,
      40,   109,    80,    37,    23,   178,   178,   183,   246,   247,
     178,   286,    65,   287,   102,   210,     2,   158,   178,   210,
       6,   210,    10,   176,   210,   296,   111,   112,     1,   133,
       2,    85,   125,   132,     6,   215,   217,   210,   192,   210,
      71,   361,   362,   275,     1,    42,     2,     3,     4,     5,
       6,   185,    67,   232,   186,    68,   236,   311,   187,   103,
     188,   291,    70,    73,   231,   293,     1,   294,     2,     3,
       4,     5,     6,   103,   224,   225,   226,   295,   229,    74,
     227,    92,    93,    94,    95,    96,    97,    98,    99,    83,
     100,    78,   244,   243,    84,   106,   152,     7,     8,     9,
     110,   263,   274,   120,   121,   313,    38,    39,    40,   285,
     127,   104,     1,   299,     2,    79,   192,   142,     6,     7,
       8,     9,   264,    38,    39,    40,   -11,   224,   225,   226,
     125,   153,    10,   227,    38,    39,    40,    83,   318,   160,
      83,    43,    44,    45,   335,   157,    43,    44,    45,    81,
      47,    38,    39,    40,    10,    38,    39,    40,   333,    49,
     166,   167,    50,   334,    23,   341,   342,   300,   -11,   -11,
     -11,    43,    44,    45,    81,    47,   168,   169,   321,   171,
      38,    39,    40,   170,    49,   172,   184,    50,   195,   306,
     329,   308,   309,   345,   207,   154,    51,   125,    52,    97,
      38,    39,    40,   319,   320,    38,    39,    40,     1,   211,
       2,   348,   348,   220,     6,   230,   233,   325,    53,   329,
     348,    51,   328,    52,   238,   237,   239,   346,   240,   347,
      92,    93,    94,    95,    96,    97,    98,    99,   348,   100,
     135,   245,   252,    53,    43,    44,    45,    81,    47,   250,
     251,   253,   346,   191,   347,     2,   254,    49,   255,     6,
      50,   256,    43,    44,    45,    81,    47,   154,    43,    44,
      45,    81,    47,   260,   248,    49,   198,   199,   161,   200,
     270,    49,   277,   202,   265,   279,    43,    44,    45,    81,
      47,   281,   282,   280,    51,   284,    52,   292,   297,    49,
     301,   302,   262,   307,     2,   304,   162,   359,     6,   312,
     360,   163,    51,   315,    52,   178,    53,    85,    51,   316,
      52,   266,   267,   322,   268,   344,    43,    44,    45,    46,
      47,   352,   353,    48,    53,   358,    51,   357,    52,    49,
      53,   363,    50,    43,    44,    45,    81,    47,   228,    43,
      44,    45,    81,    47,   173,   213,    49,   122,    53,    50,
     141,   332,    49,   258,   131,   214,    92,    93,    94,    95,
      96,    97,    98,    99,   351,   100,    51,   182,    52,    92,
      93,    94,    95,    96,    97,    98,    99,   235,   100,    94,
      95,    96,    97,    51,   310,    52,   314,   216,    53,    51,
     159,    52,    92,    93,    94,    95,    96,    97,    98,    99,
     249,   221,   128,   355,   323,    53,   259,     0,     0,     0,
       0,    53,    92,    93,    94,    95,    96,    97,    98,    99,
       0,   221,     0,     0,     0,   181,     0,     0,     0,  -143,
       0,   222,  -167,     0,     0,     0,     0,    85,    92,    93,
      94,    95,    96,    97,    98,    99,     0,   100,     0,     0,
     185,   222,  -167,   186,     0,     0,     0,   187,     0,   188,
       0,     0,     0,  -143,    92,    93,    94,    95,    96,    97,
      98,    99,     0,   100,   303,    92,    93,    94,    95,    96,
      97,    98,    99,     0,   100,   128,    92,    93,    94,    95,
      96,    97,    98,    99,     0,   100,   101,   196,   197,   198,
     199,     0,   200,   201,   288,   289,   202,    92,    93,    94,
      95,    96,    97,    98,     0,     0,   100,    92,    93,    94,
      95,    96,    97,     0,     0,     0,   100,   196,   197,   198,
     199,     0,   200,   201,     0,     0,   202,    92,    93,    94,
      95,    96,    97
};

static const yytype_int16 yycheck[] =
{
      25,    79,    89,     1,     0,   139,    84,   139,    65,    19,
      19,   142,    18,    18,    18,     0,    31,    32,    18,    34,
      39,   136,    37,    18,    49,    50,    18,    18,    53,    18,
     161,   162,    18,    63,    19,    33,    21,    22,    23,    24,
      25,    39,    40,   345,   109,    18,     6,    18,    41,    19,
       6,    21,     6,    78,    79,    25,    18,    39,    46,    84,
      85,   363,    50,    19,    42,    90,    64,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   191,    75,    46,   157,
      40,    40,    50,    82,    83,    87,     6,    72,    73,    74,
     105,   110,     6,   224,   225,   226,   106,     6,    60,    61,
      62,   158,   111,   109,   102,   110,   110,   132,   195,   196,
     110,   245,    70,   245,    19,   110,    21,   110,   110,   110,
      25,   110,   107,   121,   110,   256,    28,    29,    19,   131,
      21,    76,   157,    78,    25,   160,   161,   110,   136,   110,
     110,    90,    91,    18,    19,   102,    21,    22,    23,    24,
      25,    27,    26,   178,    30,   107,   181,   288,    34,    64,
      36,   248,    40,   109,    18,   252,    19,   254,    21,    22,
      23,    24,    25,    64,    44,    45,    46,   255,   174,    56,
      50,     8,     9,    10,    11,    12,    13,    14,    15,   214,
      17,    19,    18,   191,    19,     6,   221,    72,    73,    74,
      41,    18,   227,    26,    19,   292,    60,    61,    62,    18,
      10,   209,    19,    18,    21,    19,   214,    41,    25,    72,
      73,    74,   220,    60,    61,    62,    18,    44,    45,    46,
     255,     6,   107,    50,    60,    61,    62,   262,    18,    17,
     265,     3,     4,     5,   322,    19,     3,     4,     5,     6,
       7,    60,    61,    62,   107,    60,    61,    62,    18,    16,
       3,     5,    19,    18,   262,    92,    93,   265,    60,    61,
      62,     3,     4,     5,     6,     7,     6,    33,   303,    38,
      60,    61,    62,    35,    16,     5,    79,    19,   110,   279,
     315,   281,   282,    50,    66,     6,    53,   322,    55,    13,
      60,    61,    62,   301,   302,    60,    61,    62,    19,   111,
      21,   336,   337,    19,    25,     5,    80,   307,    75,   344,
     345,    53,   312,    55,    25,    79,    35,    84,    25,    86,
       8,     9,    10,    11,    12,    13,    14,    15,   363,    17,
       6,    18,    95,    75,     3,     4,     5,     6,     7,    99,
      99,    99,    84,    19,    86,    21,    95,    16,    68,    25,
      19,    65,     3,     4,     5,     6,     7,     6,     3,     4,
       5,     6,     7,     6,    95,    16,    97,    98,    19,   100,
      48,    16,    19,   104,    19,    19,     3,     4,     5,     6,
       7,    19,    19,    25,    53,     6,    55,    95,    67,    16,
      19,    19,    19,    19,    21,    81,    47,    85,    25,    19,
      88,    52,    53,    68,    55,   110,    75,    76,    53,    69,
      55,    56,    57,    68,    59,   110,     3,     4,     5,     6,
       7,    18,    89,    10,    75,    87,    53,    85,    55,    16,
      75,    44,    19,     3,     4,     5,     6,     7,   174,     3,
       4,     5,     6,     7,   118,   158,    16,    71,    75,    19,
      90,   316,    16,   209,    86,    19,     8,     9,    10,    11,
      12,    13,    14,    15,   337,    17,    53,   131,    55,     8,
       9,    10,    11,    12,    13,    14,    15,   180,    17,    10,
      11,    12,    13,    53,   284,    55,   294,   160,    75,    53,
     107,    55,     8,     9,    10,    11,    12,    13,    14,    15,
     197,    17,    18,   344,   305,    75,   210,    -1,    -1,    -1,
      -1,    75,     8,     9,    10,    11,    12,    13,    14,    15,
      -1,    17,    -1,    -1,    -1,    77,    -1,    -1,    -1,     6,
      -1,    47,    48,    -1,    -1,    -1,    -1,    76,     8,     9,
      10,    11,    12,    13,    14,    15,    -1,    17,    -1,    -1,
      27,    47,    48,    30,    -1,    -1,    -1,    34,    -1,    36,
      -1,    -1,    -1,    40,     8,     9,    10,    11,    12,    13,
      14,    15,    -1,    17,    44,     8,     9,    10,    11,    12,
      13,    14,    15,    -1,    17,    18,     8,     9,    10,    11,
      12,    13,    14,    15,    -1,    17,    40,    95,    96,    97,
      98,    -1,   100,   101,   102,   103,   104,     8,     9,    10,
      11,    12,    13,    14,    -1,    -1,    17,     8,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    17,    95,    96,    97,
      98,    -1,   100,   101,    -1,    -1,   104,     8,     9,    10,
      11,    12,    13
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    19,    21,    22,    23,    24,    25,    72,    73,    74,
     107,   113,   114,   115,   116,   117,   120,   121,   126,   128,
     131,   133,   135,   116,    42,   136,    63,     6,    39,   127,
      40,   122,     6,   118,   119,     0,   114,   109,    60,    61,
      62,    18,   102,     3,     4,     5,     6,     7,    10,    16,
      19,    53,    55,    75,   137,   138,   140,   141,   142,   143,
     144,   145,   146,   147,     6,    70,     6,    26,   107,   123,
      40,   110,   116,   109,    56,   134,   116,   116,    19,    19,
     111,     6,   140,   140,    19,    76,   140,   148,   149,    39,
     110,   157,     8,     9,    10,    11,    12,    13,    14,    15,
      17,    40,    19,    64,   116,   132,     6,   129,   130,   142,
      41,    28,    29,    31,    32,    34,    37,   105,   124,   125,
      26,    19,   119,   116,   139,   140,   139,    10,    18,   139,
     140,   148,    78,   149,   150,     6,    19,   158,   159,   160,
     162,   138,    41,   168,   140,   140,   140,   140,   140,   140,
     140,   140,   140,     6,     6,   161,   180,    19,   110,   168,
      17,    19,    47,    52,   140,   169,     3,     5,     6,    33,
      35,    38,     5,   125,    19,   106,   116,    18,   110,    18,
      18,    77,   150,   140,    79,    27,    30,    34,    36,   165,
     166,    19,   116,   159,   162,   110,    95,    96,    97,    98,
     100,   101,   104,   163,   165,   166,   169,    66,   172,    18,
     110,   111,   139,   130,    19,   140,   160,   140,   169,   169,
      19,    17,    47,   171,    44,    45,    46,    50,   113,   114,
       5,    18,   140,    80,   151,   151,   140,    79,    25,    35,
      25,     6,    40,   116,    18,    18,   159,   159,    95,   163,
      99,    99,    95,    99,    95,    68,    65,   173,   132,   180,
       6,    18,    19,    18,   116,    19,    56,    57,    59,   170,
      48,   169,   169,   169,   140,    18,    18,    19,   152,    19,
      25,    19,    19,   167,     6,    18,   165,   166,   102,   103,
     164,   159,    95,   159,   159,   139,   169,    67,   174,    18,
     116,    19,    19,    44,    81,   153,   161,    19,   161,   161,
     167,   169,    19,   159,   164,    68,    69,   175,    18,   116,
     116,   140,    68,   174,    18,   161,    18,    18,   161,   140,
     176,   177,   141,    18,    18,   139,    82,    83,   154,    18,
      18,    92,    93,   178,   110,    50,    84,    86,   140,   155,
     156,   155,    18,    89,   179,   177,   156,    85,    87,    85,
      88,    90,    91,    44,   156
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   112,   113,   113,   114,   114,   114,   114,   115,   115,
     115,   116,   116,   116,   116,   117,   118,   118,   119,   120,
     120,   120,   121,   121,   121,   122,   122,   122,   123,   123,
     124,   124,   125,   125,   125,   125,   125,   126,   127,   127,
     128,   129,   129,   130,   130,   131,   131,   132,   132,   133,
     133,   133,   134,   134,   135,   136,   136,   136,   137,   137,
     138,   138,   138,   138,   139,   139,   140,   140,   140,   140,
     140,   140,   140,   140,   140,   141,   141,   141,   142,   143,
     144,   144,   144,   144,   144,   144,   144,   144,   144,   145,
     146,   146,   147,   147,   148,   148,   149,   150,   150,   151,
     151,   152,   153,   153,   154,   154,   154,   155,   155,   156,
     156,   156,   156,   157,   157,   158,   158,   159,   159,   159,
     159,   159,   159,   160,   161,   161,   162,   162,   162,   162,
     162,   162,   163,   163,   163,   163,   163,   163,   163,   164,
     164,   165,   165,   166,   166,   166,   166,   166,   166,   167,
     167,   168,   168,   169,   169,   169,   169,   169,   169,   169,
     169,   169,   169,   169,   170,   170,   170,   171,   171,   172,
     172,   173,   173,   174,   174,   175,   175,   176,   176,   177,
     178,   178,   178,   179,   179,   179,   180,   180
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     3,     1,     1,     1,     1,     1,     1,
       1,     3,     1,     1,     1,     3,     3,     1,     5,     1,
       1,     1,     7,     7,     6,     0,     4,     4,     0,     2,
       1,     2,     2,     2,     2,     2,     2,     5,     0,     1,
       5,     1,     3,     3,     3,     4,     7,     4,     1,     3,
       3,     4,     0,     1,     9,     0,     1,     5,     1,     3,
       1,     3,     1,     3,     1,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       5,     5,     5,     4,     2,     1,     4,     0,     2,     0,
       2,     5,     0,     3,     0,     2,     2,     4,     1,     2,
       2,     2,     2,     0,     2,     1,     3,     2,     2,     2,
       2,     1,     4,     3,     1,     3,     3,     4,     5,     4,
       5,     4,     1,     2,     1,     2,     2,     1,     1,     4,
       2,     3,     4,     0,     1,     5,     5,     3,     6,     0,
       3,     0,     2,     3,     1,     2,     3,     3,     3,     5,
       6,     5,     6,     4,     1,     1,     1,     0,     1,     0,
       3,     0,     2,     0,     3,     0,     2,     1,     3,     3,
       0,     1,     1,     0,     2,     2,     1,     3
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
#line 135 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("stmtList::stmt"); 
				(yyval.list) = singleton((yyvsp[-1].node));
				bisonParseResult = (Node *) (yyval.list);	 
			}
#line 1676 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 141 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("stmtlist::stmtList::stmt");
				(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[-1].node));	
				bisonParseResult = (Node *) (yyval.list); 
			}
#line 1686 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 150 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::dmlStmt");
            (yyval.node) = (yyvsp[0].node);
        }
#line 1695 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 155 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::queryStmt");
            (yyval.node) = (yyvsp[0].node);
        }
#line 1704 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 160 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::transactionIdentifier");
            (yyval.node) = (Node *) createTransactionStmt((yyvsp[0].stringVal));
        }
#line 1713 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 165 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("stmt::withQuery"); 
			(yyval.node) = (yyvsp[0].node); 
		}
#line 1722 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 175 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::insertQuery"); }
#line 1728 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 176 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::deleteQuery"); }
#line 1734 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 177 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::updateQuery"); }
#line 1740 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 184 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::bracketedQuery"); (yyval.node) = (yyvsp[-1].node); }
#line 1746 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 185 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::selectQuery"); }
#line 1752 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 186 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::provStmt"); }
#line 1758 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 187 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::setOperatorQuery"); }
#line 1764 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 192 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withQuery::withViewList::queryStmt");
			(yyval.node) = (Node *) createWithStmt((yyvsp[-1].list), (yyvsp[0].node));
		}
#line 1773 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 200 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withViewList::list::view");
			(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
		}
#line 1782 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 205 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withViewList::view");
			(yyval.list) = singleton((yyvsp[0].node));
		}
#line 1791 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 213 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withView::ident::AS:queryStmt");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString((yyvsp[-4].stringVal)), (yyvsp[-1].node));
		}
#line 1800 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 220 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::BEGIN"); (yyval.stringVal) = strdup("TRANSACTION_BEGIN"); }
#line 1806 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 221 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::COMMIT"); (yyval.stringVal) = strdup("TRANSACTION_COMMIT"); }
#line 1812 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 222 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::ROLLBACK"); (yyval.stringVal) = strdup("TRANSACTION_ABORT"); }
#line 1818 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 230 "sql_parser.y" /* yacc.c:1646  */
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
#line 1833 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 241 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provStmt::stmtlist");
			ProvenanceStmt *p = createProvenanceStmt((Node *) (yyvsp[-1].list));
			p->inputType = PROV_INPUT_UPDATE_SEQUENCE;
			p->provType = PROV_PI_CS;
			p->asOf = (Node *) (yyvsp[-5].node);
			p->options = (yyvsp[-4].list);
			(yyval.node) = (Node *) p;
		}
#line 1847 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 251 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provStmt::transaction");
			ProvenanceStmt *p = createProvenanceStmt((Node *) createConstString((yyvsp[0].stringVal)));
			p->inputType = PROV_INPUT_TRANSACTION;
			p->provType = PROV_PI_CS;
			p->options = (yyvsp[-3].list);
			(yyval.node) = (Node *) p;
		}
#line 1860 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 262 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalProvAsOf::EMPTY"); (yyval.node) = NULL; }
#line 1866 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 264 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvAsOf::SCN");
			(yyval.node) = (Node *) createConstLong((yyvsp[0].intVal));
		}
#line 1875 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 269 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvAsOf::TIMESTAMP");
			(yyval.node) = (Node *) createConstString((yyvsp[0].stringVal));
		}
#line 1884 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 276 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalProvWith::EMPTY"); (yyval.list) = NIL; }
#line 1890 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 278 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvWith::WITH");
			(yyval.list) = (yyvsp[0].list);
		}
#line 1899 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 285 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("provOptionList::option"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 1905 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 287 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("provOptionList::list"); 
			(yyval.list) = appendToTailOfList((yyvsp[-1].list),(yyvsp[0].node)); 
		}
#line 1914 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 295 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("provOption::TYPE"); 
			(yyval.node) = (Node *) createStringKeyValue("TYPE", (yyvsp[0].stringVal)); 
		}
#line 1923 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 300 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::TABLE");
			(yyval.node) = (Node *) createStringKeyValue("TABLE", (yyvsp[0].stringVal));
		}
#line 1932 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 305 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::ONLY::UPDATED");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_ONLY_UPDATED), 
					(Node *) createConstBool(TRUE));
		}
#line 1942 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 311 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::SHOW::INTERMEDIATE");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_SHOW_INTERMEDIATE), 
					(Node *) createConstBool(TRUE));
		}
#line 1952 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 317 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::TUPLE::VERSIONS");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_TUPLE_VERSIONS),
					(Node *) createConstBool(TRUE));
		}
#line 1962 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 329 "sql_parser.y" /* yacc.c:1646  */
    { 
             RULELOG("deleteQuery");
             (yyval.node) = (Node *) createDelete((yyvsp[-2].stringVal), (yyvsp[0].node));
         }
#line 1971 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 337 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::NULL"); (yyval.stringVal) = NULL; }
#line 1977 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 338 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::FROM"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 1983 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 353 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG(updateQuery); 
                (yyval.node) = (Node *) createUpdate((yyvsp[-3].stringVal), (yyvsp[-1].list), (yyvsp[0].node)); 
            }
#line 1992 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 361 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setExpression");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2001 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 366 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setClause::setExpression");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2010 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 374 "sql_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal),"=")) {
                    RULELOG("setExpression::attributeRef::expression");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 2023 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 383 "sql_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal), "=")) {
                    RULELOG("setExpression::attributeRef::queryStmt");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 2036 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 398 "sql_parser.y" /* yacc.c:1646  */
    { 
           	RULELOG("insertQuery::insertList"); 
           	(yyval.node) = (Node *) createInsert((yyvsp[-1].stringVal),(Node *) (yyvsp[0].node), NIL); 
        }
#line 2045 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 403 "sql_parser.y" /* yacc.c:1646  */
    { 
           	RULELOG("insertQuery::insertList"); 
           	(yyval.node) = (Node *) createInsert((yyvsp[-4].stringVal),(Node *) (yyvsp[0].node), (yyvsp[-2].list)); 
     	}
#line 2054 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 416 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (Node *) (yyvsp[-1].list); }
#line 2060 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 476 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::INTERSECT");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 2069 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 481 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::MINUS");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 2078 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 486 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::UNION");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-2].stringVal), ((yyvsp[-1].stringVal) != NULL), (yyvsp[-3].node), (yyvsp[0].node));
            }
#line 2087 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 493 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::NULL"); (yyval.stringVal) = NULL; }
#line 2093 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 494 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::ALLTRUE"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2099 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 504 "sql_parser.y" /* yacc.c:1646  */
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
#line 2119 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 526 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalDistinct::NULL"); (yyval.node) = NULL; }
#line 2125 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 528 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT");
                (yyval.node) = (Node *) createDistinctClause(NULL);
            }
#line 2134 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 533 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT::exprList");
                (yyval.node) = (Node *) createDistinctClause((yyvsp[-1].list));
            }
#line 2143 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 545 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectItem"); (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2151 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 549 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectClause::selectItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node)); 
            }
#line 2160 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 557 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression"); 
                 (yyval.node) = (Node *) createSelectItem(NULL, (yyvsp[0].node)); 
             }
#line 2169 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 562 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression::identifier"); 
                 (yyval.node) = (Node *) createSelectItem((yyvsp[0].stringVal), (yyvsp[-2].node));
             }
#line 2178 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 567 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
         		(yyval.node) = (Node *) createSelectItem(strdup("*"), NULL); 
     		}
#line 2187 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 572 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
     			(yyval.node) = (Node *) createSelectItem(CONCAT_STRINGS((yyvsp[-2].stringVal),".*"), NULL); 
 			}
#line 2196 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 582 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("exprList::SINGLETON"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 2202 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 584 "sql_parser.y" /* yacc.c:1646  */
    {
                  RULELOG("exprList::exprList::expression");
                  (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
             }
#line 2211 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 595 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::bracked"); (yyval.node) = (yyvsp[-1].node); }
#line 2217 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 596 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::constant"); }
#line 2223 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 597 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::attributeRef"); }
#line 2229 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 598 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlParameter"); }
#line 2235 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 599 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::binaryOperatorExpression"); }
#line 2241 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 600 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::unaryOperatorExpression"); }
#line 2247 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 601 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlFunctionCall"); }
#line 2253 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 602 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::case"); }
#line 2259 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 603 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::ROWNUM"); (yyval.node) = (Node *) makeNode(RowNumExpr); }
#line 2265 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 612 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::INT"); (yyval.node) = (Node *) createConstInt((yyvsp[0].intVal)); }
#line 2271 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 613 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::FLOAT"); (yyval.node) = (Node *) createConstFloat((yyvsp[0].floatVal)); }
#line 2277 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 614 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::STRING"); (yyval.node) = (Node *) createConstString((yyvsp[0].stringVal)); }
#line 2283 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 621 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("attributeRef::IDENTIFIER"); (yyval.node) = (Node *) createAttributeReference((yyvsp[0].stringVal)); }
#line 2289 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 627 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("sqlParameter::PARAMETER"); (yyval.node) = (Node *) createSQLParameter((yyvsp[0].stringVal)); }
#line 2295 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 647 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '+' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2306 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 654 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '-' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2317 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 661 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '*' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2328 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 668 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '/' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2339 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 675 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '%' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2350 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 682 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '^' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2361 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 691 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '&' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2372 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 698 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '|' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2383 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 707 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression::comparisonOps");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2394 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 717 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("unaryOperatorExpression:: '!' ");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2404 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 729 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("sqlFunctionCall::IDENTIFIER::exprList");
				FunctionCall *f = createFunctionCall((yyvsp[-4].stringVal), (yyvsp[-2].list));
				if ((yyvsp[0].node) != NULL)
					(yyval.node) = (Node *) createWindowFunction(f, (WindowDef *) (yyvsp[0].node));
				else  
                	(yyval.node) = (Node *) f; 
            }
#line 2417 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 738 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("sqlFunctionCall::AMMSC::exprList");
				FunctionCall *f = createFunctionCall((yyvsp[-4].stringVal), (yyvsp[-2].list));
				if ((yyvsp[0].node) != NULL)
					(yyval.node) = (Node *) createWindowFunction(f, (WindowDef *) (yyvsp[0].node));
				else  
                	(yyval.node) = (Node *) f; 
            }
#line 2430 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 753 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseExpression::CASE::expression::whens:else:END");
				(yyval.node) = (Node *) createCaseExpr((yyvsp[-3].node), (yyvsp[-2].list), (yyvsp[-1].node));
			}
#line 2439 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 758 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseExpression::CASE::whens::else::END");
				(yyval.node) = (Node *) createCaseExpr(NULL, (yyvsp[-2].list), (yyvsp[-1].node));
			}
#line 2448 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 766 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhenList::list::caseWhen");
				(yyval.list) = appendToTailOfList((yyvsp[-1].list), (yyvsp[0].node));
			}
#line 2457 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 771 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhenList::caseWhen");
				(yyval.list) = singleton((yyvsp[0].node));
			}
#line 2466 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 779 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhen::WHEN::expression::THEN::expression");
				(yyval.node) = (Node *) createCaseWhen((yyvsp[-2].node),(yyvsp[0].node));
			}
#line 2475 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 786 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalCaseElse::NULL"); (yyval.node) = NULL; }
#line 2481 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 788 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalCaseElse::ELSE::expression");
				(yyval.node) = (yyvsp[0].node);
			}
#line 2490 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 798 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("overclause::NULL"); (yyval.node) = NULL; }
#line 2496 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 800 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("overclause::window");
				(yyval.node) = (yyvsp[0].node);
			}
#line 2505 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 808 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("window");
				(yyval.node) = (Node *) createWindowDef((yyvsp[-3].list),(yyvsp[-2].list), (WindowFrame *) (yyvsp[-1].node));
			}
#line 2514 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 815 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optWindowPart::NULL"); (yyval.list) = NIL; }
#line 2520 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 817 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optWindowPart::PARTITION:BY::expressionList");
				(yyval.list) = (yyvsp[0].list);
			}
#line 2529 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 824 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optWindowFrame::NULL"); (yyval.node) = NULL; }
#line 2535 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 826 "sql_parser.y" /* yacc.c:1646  */
    { 
				WindowBound *l, *u = NULL;
				RULELOG("optWindowFrame::ROWS::windoBoundaries");
				l = getNthOfListP((yyvsp[0].list), 0);
				if(LIST_LENGTH((yyvsp[0].list)) > 1)
					u = getNthOfListP((yyvsp[0].list), 1);
				(yyval.node) = (Node *) createWindowFrame(WINFRAME_ROWS, l, u); 
			}
#line 2548 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 835 "sql_parser.y" /* yacc.c:1646  */
    {
				WindowBound *l, *u = NULL; 
				RULELOG("optWindowFrame::RANGE::windoBoundaries"); 
				l = getNthOfListP((yyvsp[0].list), 0);
				if(LIST_LENGTH((yyvsp[0].list)) > 1)
					u = getNthOfListP((yyvsp[0].list), 1);
				(yyval.node) = (Node *) createWindowFrame(WINFRAME_RANGE, l, u); 
			}
#line 2561 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 847 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBoundaries::BETWEEN"); 
				(yyval.list) = LIST_MAKE((yyvsp[-2].node), (yyvsp[0].node)); 
			}
#line 2570 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 852 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBoundaries::windowBound"); 
				(yyval.list) = singleton((yyvsp[0].node)); 
			}
#line 2579 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 860 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::UNBOUNDED::PRECEDING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_UNBOUND_PREC, NULL); 
			}
#line 2588 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 865 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::CURRENT::ROW"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_CURRENT_ROW, NULL); 
			}
#line 2597 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 870 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::expression::PRECEDING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_EXPR_PREC, (yyvsp[-1].node)); 
			}
#line 2606 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 875 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::expression::FOLLOWING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_EXPR_FOLLOW, (yyvsp[-1].node)); 
			}
#line 2615 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 887 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::NULL"); (yyval.list) = NULL; }
#line 2621 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 888 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::fromClause"); (yyval.list) = (yyvsp[0].list); }
#line 2627 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 893 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClauseItem");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2636 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 898 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClause::fromClauseItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2645 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 907 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
				FromItem *f = createFromTableRef(NULL, NIL, (yyvsp[-1].stringVal));
				f->provInfo = (FromProvInfo *) (yyvsp[0].node);
                (yyval.node) = (Node *) f;
            }
#line 2656 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 914 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
                FromItem *f = createFromTableRef(((FromItem *) (yyvsp[0].node))->name, 
						((FromItem *) (yyvsp[0].node))->attrNames, (yyvsp[-1].stringVal));
				f->provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
                (yyval.node) = (Node *) f;
            }
#line 2668 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 923 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromItem *f = (FromItem *) (yyvsp[-1].node);
                f->provInfo = (FromProvInfo *) (yyvsp[0].node);
                (yyval.node) = (yyvsp[-1].node);
            }
#line 2679 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 930 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromSubquery *s = (FromSubquery *) (yyvsp[-1].node);
                s->from.name = ((FromItem *) (yyvsp[0].node))->name;
                s->from.attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
                s->from.provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
                (yyval.node) = (Node *) s;
            }
#line 2692 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 939 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[0].node);
        		f->name = NULL;
        		(yyval.node) = (Node *) f;
        	}
#line 2704 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 947 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[-2].node);
        		f->name = ((FromItem *) (yyvsp[0].node))->name;
                f->attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
                f->provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
        		(yyval.node) = (Node *) f;
        	}
#line 2718 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 960 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("subQuery::queryStmt");
                (yyval.node) = (Node *) createFromSubquery(NULL, NULL, (yyvsp[-1].node));
            }
#line 2727 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 967 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = singleton((yyvsp[0].stringVal)); }
#line 2733 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 968 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].stringVal)); }
#line 2739 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 972 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2745 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 974 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURAL");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
						(FromItem *) (yyvsp[0].node), "JOIN_INNER", "JOIN_COND_NATURAL", 
						NULL);
          	}
#line 2756 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 981 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURALjoinType");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[0].node), (yyvsp[-2].stringVal), "JOIN_COND_NATURAL", NULL);
          	}
#line 2766 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 987 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::CROSS JOIN");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[0].node), "JOIN_CROSS", "JOIN_COND_ON", NULL);
          	}
#line 2776 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 993 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinType::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON";
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[-1].node), (yyvsp[-3].stringVal), condType, (yyvsp[0].node));
          	}
#line 2788 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 1001 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON"; 
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[-1].node), "JOIN_INNER", 
                		condType, (yyvsp[0].node));
          	}
#line 2801 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 1012 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2807 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 1013 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT OUTER"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2813 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 1014 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT "); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2819 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 1015 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT OUTER"); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2825 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 1016 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL OUTER"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2831 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 1017 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2837 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 1018 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::INNER"); (yyval.stringVal) = "JOIN_INNER"; }
#line 2843 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 1022 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (Node *) (yyvsp[-1].list); }
#line 2849 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 1023 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2855 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 1028 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[-2].node);
				(yyval.node) = (Node *) f;
			}
#line 2866 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 1035 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[-3].node); 
				(yyval.node) = (Node *) f;
			}
#line 2877 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 1044 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFromProv::empty"); (yyval.node) = NULL; }
#line 2883 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 1046 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::BASERELATION");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = TRUE;
				p->userProvAttrs = NIL;				 
				(yyval.node) = (Node *) p; 
			}
#line 2895 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1054 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::userProvAttr");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = FALSE;
				p->userProvAttrs = (yyvsp[-1].list);				 
				(yyval.node) = (Node *) p; 
			}
#line 2907 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1062 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::userProvDupAttr");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = TRUE;
				p->userProvAttrs = (yyvsp[-1].list);
				(yyval.node) = (Node *) p;
			}
#line 2919 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1070 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::intermediateProv");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->intermediateProv = TRUE;
				(yyval.node) = (Node *) p;
			}
#line 2930 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1077 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::intermediateProv::attrList");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->intermediateProv = TRUE;
				p->userProvAttrs = (yyvsp[-1].list);
				(yyval.node) = (Node *) p;
			}
#line 2942 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1087 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAttrAlias::empty"); (yyval.list) = NULL; }
#line 2948 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1089 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAttrAlias::identifierList"); (yyval.list) = (yyvsp[-1].list); 
			}
#line 2956 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1098 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::NULL"); (yyval.node) = NULL; }
#line 2962 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1099 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::whereExpression"); (yyval.node) = (yyvsp[0].node); }
#line 2968 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1103 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("where::brackedWhereExpression"); (yyval.node) = (yyvsp[-1].node); }
#line 2974 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1104 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("whereExpression::expression"); (yyval.node) = (yyvsp[0].node); }
#line 2980 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1106 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::NOT");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2990 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 1112 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 3001 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 1119 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::OR");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 3012 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1126 "sql_parser.y" /* yacc.c:1646  */
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
#line 3034 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1144 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::BETWEEN-AND");
                List *expr = singleton((yyvsp[-4].node));
                expr = appendToTailOfList(expr, (yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr);
            }
#line 3046 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 1152 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::comparisonOps::nestedSubQueryOperator::Subquery");
                (yyval.node) = (Node *) createNestedSubquery((yyvsp[-3].stringVal), (yyvsp[-5].node), (yyvsp[-4].stringVal), (yyvsp[-1].node));
            }
#line 3055 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1157 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::Subquery");
                Node *q = (Node *) createNestedSubquery("SCALAR", NULL, NULL, (yyvsp[-1].node)); 
                List *expr = LIST_MAKE((yyvsp[-4].node), q);
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr); 
            }
#line 3066 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1164 "sql_parser.y" /* yacc.c:1646  */
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
#line 3083 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1177 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::EXISTS");
                (yyval.node) = (Node *) createNestedSubquery((yyvsp[-3].stringVal), NULL, NULL, (yyvsp[-1].node));
            }
#line 3092 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 1184 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ANY"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 3098 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 1185 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ALL"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 3104 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 1186 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::SOME"); (yyval.stringVal) = "ANY"; }
#line 3110 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1190 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NULL"); (yyval.stringVal) = NULL; }
#line 3116 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 1191 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NOT"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 3122 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 1195 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::NULL"); (yyval.list) = NULL; }
#line 3128 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 1196 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::GROUPBY"); (yyval.list) = (yyvsp[0].list); }
#line 3134 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 1200 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.node) = NULL; }
#line 3140 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 1202 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("optionalHaving::HAVING"); 
                (yyval.node) = (Node *) (yyvsp[0].node);
            }
#line 3149 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 1209 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.list) = NULL; }
#line 3155 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 1210 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy::ORDERBY"); (yyval.list) = (yyvsp[0].list); }
#line 3161 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 1214 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::NULL"); (yyval.node) = NULL; }
#line 3167 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 1215 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::CONSTANT"); (yyval.node) = (yyvsp[0].node);}
#line 3173 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 1220 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::orderExpr");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 3182 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 1225 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("orderList::orderList::orderExpr");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 3191 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 1233 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("orderExpr::expr::sortOrder::nullOrder");
				SortOrder o = (strcmp((yyvsp[-1].stringVal),"ASC") == 0) ?  SORT_ASC : SORT_DESC;
				SortNullOrder n = (strcmp((yyvsp[0].stringVal),"NULLS_FIRST") == 0) ? 
						SORT_NULLS_FIRST : 
						SORT_NULLS_LAST;
				(yyval.node) = (Node *) createOrderExpr((yyvsp[-2].node), o, n);
			}
#line 3204 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 1244 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalSortOrder::empty"); (yyval.stringVal) = "ASC"; }
#line 3210 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 1246 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalSortOrder::ASC");
				(yyval.stringVal) = "ASC";
			}
#line 3219 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 1251 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalSortOrder::DESC");
				(yyval.stringVal) = "DESC";
			}
#line 3228 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 1258 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNullOrder::empty"); (yyval.stringVal) = "NULLS_LAST"; }
#line 3234 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 1260 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalNullOrder::NULLS FIRST");
				(yyval.stringVal) = "NULLS_FIRST";
			}
#line 3243 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 1265 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalNullOrder::NULLS LAST");
				(yyval.stringVal) = "NULLS_LAST";
			}
#line 3252 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 186:
#line 1272 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("identifierList::ident"); }
#line 3258 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 187:
#line 1274 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("identifierList::list::ident"); 
			(yyval.stringVal) = CONCAT_STRINGS((yyvsp[-2].stringVal), ".", (yyvsp[0].stringVal)); //TODO 
		}
#line 3267 "sql_parser.tab.c" /* yacc.c:1646  */
    break;


#line 3271 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 1280 "sql_parser.y" /* yacc.c:1906  */




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
11. Add DDL statements
12. Add provenance storage policy language
13. Add provenance export options (also requires several other changes in the application)
*/

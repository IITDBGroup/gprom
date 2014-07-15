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
#define YYFINAL  61
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   673

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  112
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  69
/* YYNRULES -- Number of rules.  */
#define YYNRULES  188
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  366

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
       0,   134,   134,   140,   149,   154,   159,   164,   169,   180,
     181,   182,   189,   190,   191,   192,   196,   204,   209,   217,
     225,   226,   227,   234,   245,   255,   267,   268,   273,   281,
     282,   290,   291,   299,   304,   309,   315,   321,   333,   342,
     343,   357,   365,   370,   378,   387,   402,   407,   421,   422,
     480,   485,   490,   498,   499,   508,   531,   532,   537,   549,
     553,   561,   566,   571,   576,   587,   588,   600,   601,   602,
     603,   604,   605,   606,   607,   608,   617,   618,   619,   626,
     632,   651,   658,   665,   672,   679,   686,   695,   702,   711,
     721,   733,   742,   757,   762,   770,   775,   783,   791,   792,
     803,   804,   812,   820,   821,   829,   830,   839,   851,   856,
     864,   869,   874,   879,   892,   893,   897,   902,   911,   918,
     927,   934,   943,   951,   964,   972,   973,   977,   978,   985,
     991,   997,  1005,  1017,  1018,  1019,  1020,  1021,  1022,  1023,
    1027,  1028,  1032,  1039,  1049,  1050,  1058,  1066,  1074,  1081,
    1092,  1093,  1103,  1104,  1108,  1109,  1110,  1116,  1123,  1130,
    1148,  1156,  1161,  1168,  1181,  1189,  1190,  1191,  1195,  1196,
    1200,  1201,  1205,  1206,  1214,  1215,  1219,  1220,  1224,  1229,
    1237,  1249,  1250,  1255,  1263,  1264,  1269,  1277,  1278
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

#define YYPACT_NINF -305

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-305)))

#define YYTABLE_NINF -169

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-169)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     268,  -305,  -305,  -305,    11,  -305,   461,   384,     3,   -25,
      50,    64,    22,    40,  -305,  -305,  -305,  -305,   331,    83,
     109,     2,  -305,   181,  -305,  -305,  -305,  -305,  -305,  -305,
    -305,  -305,   608,  -305,  -305,  -305,  -305,  -305,  -305,  -305,
     461,   461,   638,   196,   593,    43,   437,   145,    96,  -305,
     179,   163,    91,   461,   461,   496,    72,  -305,   160,    14,
    -305,  -305,   100,  -305,   159,   138,   138,   461,   461,   461,
     461,   461,   461,   461,   461,   461,   -14,   608,  -305,  -305,
     199,   -11,  -305,   -29,  -305,   575,   128,   214,   180,    37,
      18,   197,    -9,   485,    72,   461,  -305,   147,   210,   138,
      83,   181,  -305,  -305,   138,  -305,  -305,   527,   527,   222,
     222,   222,  -305,   638,   628,   566,   156,   461,   461,   229,
      79,   437,   207,   243,    23,   236,   181,  -305,  -305,   -19,
    -305,   242,   408,   259,   262,   258,   248,   234,   245,   275,
      18,  -305,   -16,   156,   461,   209,   608,  -305,   138,  -305,
    -305,   279,  -305,   608,    -6,  -305,   368,   135,   189,   561,
     368,  -305,  -305,   408,   235,  -305,  -305,    -5,   192,   461,
     214,  -305,   466,   408,   408,   287,   546,   274,  -305,  -305,
    -305,  -305,  -305,  -305,  -305,  -305,   268,   302,  -305,   608,
    -305,   216,   228,  -305,  -305,  -305,   300,   292,   303,  -305,
      20,   135,   312,   561,   321,    79,    79,   569,   246,   249,
    -305,   251,   250,   265,  -305,    20,   274,   276,   299,   149,
     365,   370,    -4,  -305,   384,   608,  -305,   534,    33,   274,
     138,   363,  -305,   329,   408,   408,   408,   461,   172,    -1,
    -305,  -305,   313,   311,   361,   358,   378,   380,   387,   374,
    -305,   131,   561,   531,    79,   306,  -305,  -305,    79,  -305,
      79,   461,   408,   311,  -305,   192,  -305,  -305,  -305,   390,
     384,  -305,  -305,  -305,   391,   398,    36,    36,    93,   556,
    -305,  -305,   461,   353,   -58,   365,   404,   365,   365,  -305,
     380,   427,  -305,    20,   408,   406,  -305,  -305,    79,  -305,
     531,   316,   274,   359,  -305,   463,   138,   138,   461,   316,
     461,   247,   247,   412,    -3,   365,     9,    16,  -305,   274,
     365,  -305,  -305,   291,  -305,  -305,   468,   473,   608,   193,
     323,  -305,   310,   346,   362,   344,  -305,  -305,  -305,  -305,
    -305,    19,  -305,  -305,    25,  -305,  -305,  -305,  -305,  -305,
     357,   461,   410,  -305,  -305,  -305,  -305,  -305,  -305,   121,
    -305,  -305,   310,  -305,  -305,  -305
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    76,    77,    78,    79,    80,     0,     0,    56,     0,
       0,    39,    26,     0,    75,    20,    21,    22,     0,     0,
       0,     0,     4,     5,     7,     6,    14,    10,    11,     9,
      15,    13,     8,    68,    69,    70,    71,    72,    73,    74,
       0,     0,    90,     0,     0,    57,     0,     0,     0,    40,
       0,     0,    29,     0,     0,     0,    98,    96,     0,     0,
      18,     1,     0,     2,    53,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    65,    12,    67,
       0,    79,    63,   114,    59,    61,     0,     0,     0,     0,
       0,     0,     0,     0,    98,     0,    95,     0,     0,     0,
       0,    16,     3,    54,     0,    50,    51,    81,    82,    83,
      84,    85,    86,    87,    88,    89,   100,     0,     0,     0,
       0,     0,   152,     0,     0,     0,    49,    46,    79,   152,
      42,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      30,    31,     0,   100,     0,     0,    99,    94,     0,    17,
      52,     0,    91,    66,     0,    64,   144,     0,   115,   116,
     144,   122,    60,     0,   170,    62,   187,     0,   125,     0,
       0,    41,     0,     0,     0,     0,   155,    38,    27,    28,
      34,    35,    36,    37,    33,    32,     0,     0,    92,    97,
      93,     0,   103,   101,    58,   145,     0,     0,     0,   119,
     118,     0,     0,     0,   122,     0,     0,     0,   133,   135,
     139,     0,   138,     0,   121,   120,   153,     0,   172,     0,
       0,     0,     0,    43,     0,    44,    45,   155,     0,   156,
       0,     0,   169,     0,     0,     0,     0,     0,     0,     0,
      25,    19,     0,   174,     0,     0,     0,   150,     0,     0,
     124,   127,   117,     0,     0,     0,   134,   136,     0,   137,
       0,     0,     0,   174,    47,   126,   188,    48,   154,     0,
       0,   166,   165,   167,     0,     0,   157,   158,   159,     0,
      24,    23,     0,     0,   105,     0,   148,     0,     0,   142,
     150,   124,   123,     0,     0,     0,   132,   128,     0,   130,
       0,   171,   173,   176,   164,     0,     0,     0,     0,   104,
       0,     0,     0,     0,     0,     0,     0,     0,   143,   141,
       0,   129,   131,     0,    55,   162,     0,     0,   160,   181,
     175,   178,     0,     0,     0,     0,   106,   109,   107,   102,
     146,     0,   147,   151,     0,   177,   161,   163,   182,   183,
     184,     0,     0,   110,   111,   112,   113,   149,   140,     0,
     180,   179,     0,   185,   186,   108
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -305,   271,     1,  -305,    -2,  -305,  -305,   348,  -305,  -305,
    -305,  -305,  -305,   318,  -305,  -305,  -305,  -305,   304,  -305,
     256,  -305,  -305,  -305,  -305,  -305,   341,   -42,     0,   153,
     -71,  -305,  -305,  -305,  -305,  -305,   423,   -33,   385,   360,
    -305,  -305,  -305,   203,  -304,  -305,  -305,  -118,   345,   -55,
    -121,   277,   201,  -141,  -113,   230,   389,  -142,  -305,  -305,
    -305,  -305,   263,  -305,  -305,   171,  -305,  -305,   307
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    20,    62,    22,    23,    24,    59,    60,    25,    26,
      52,    91,   140,   141,    27,    50,    28,   129,   130,    29,
     127,    30,   104,    31,    46,    83,    84,    76,   176,    33,
      34,    35,    36,    37,    38,    39,    56,    57,    97,   152,
     193,   243,   313,   336,   337,   122,   158,   203,   160,   167,
     161,   213,   296,   199,   200,   289,   164,   177,   274,   233,
     218,   263,   284,   324,   330,   331,   350,   360,   168
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      32,    21,   159,   186,   116,    43,    42,    44,    40,   143,
     120,    92,   194,   219,   267,   340,   131,   281,    55,   214,
      32,   216,   163,    96,   311,   312,   247,   342,   352,   166,
      40,   228,   229,    99,   343,     8,   204,   357,    47,    12,
      77,    44,    99,   358,     8,    45,    85,   215,    12,   135,
     136,   268,   137,    77,    93,   138,    48,   101,   365,    53,
     248,    96,    51,   105,   106,   133,   134,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   154,   234,   235,   236,
     204,   121,   236,   237,   126,   156,   237,   252,   253,    58,
     187,   170,   276,   277,   278,   146,   117,    43,   157,   131,
     119,   117,   150,    49,   117,   220,   117,   220,    63,    61,
     292,    63,     1,     2,     3,     4,     5,   153,    77,   220,
     302,    85,    43,   139,   100,     6,   220,   222,     7,   220,
       8,     9,    10,    11,    12,   220,   297,  -144,   293,  -169,
     299,   156,   300,  -169,   189,    80,   191,   124,    54,     8,
      95,    86,   319,    12,   201,   202,     8,    99,   195,     8,
      12,   196,    13,    12,    14,   197,    87,   198,    99,    77,
       8,  -144,   225,   227,    12,     1,     2,     3,     4,     5,
     321,    15,    16,    17,    18,    88,    32,   239,     6,    89,
     280,     7,   125,     8,     9,    10,    11,    12,    90,   249,
      98,    67,    68,    69,    70,    71,    72,    73,    74,   102,
      75,   363,   364,   125,    78,   103,    19,   126,   118,   301,
     128,   132,   202,   142,    44,    13,   147,    14,   269,   148,
     314,   115,   316,   317,   241,    72,   151,   279,    32,   155,
     309,    64,    65,    66,    15,    16,    17,    18,   163,   165,
       1,     2,     3,     4,     5,   169,    64,    65,    66,   172,
     341,    77,   178,     6,   180,   344,    41,   179,   305,   182,
      44,     1,     2,     3,     4,     5,    64,    65,    66,    19,
     184,   181,    77,   183,     6,   348,   349,     7,   190,     8,
       9,    10,    11,    12,     1,     2,     3,   332,   192,   205,
      13,   217,    14,   221,   326,   327,   230,   240,   328,   242,
     329,   335,   335,     1,     2,     3,     4,     5,   234,   235,
     236,    13,    18,    14,   237,   244,     6,   245,   246,    41,
     250,   333,   335,   334,     1,     2,     3,     4,     5,   251,
      15,    16,    17,    18,   261,   256,   258,     6,   257,   259,
      41,   329,    67,    68,    69,    70,    71,    72,    73,    74,
     260,    75,   335,    13,   262,    14,     1,     2,     3,     4,
       5,   166,    64,    65,    66,    19,   266,   275,   283,     6,
     285,   282,   270,   286,    13,    18,    14,     1,     2,     3,
       4,     5,   291,   290,   333,   195,   334,   287,   196,   288,
       6,   298,   197,     7,   198,     8,    18,    54,   304,    12,
     306,     1,     2,     3,     4,     5,    13,   307,    14,   271,
     272,   310,   273,   315,     6,   320,   117,   173,   323,   355,
     339,   353,   356,   351,    64,    65,    66,    13,    18,    14,
       1,     2,     3,    81,     5,   -12,   359,    82,   149,   354,
      64,    65,    66,     6,   362,   174,    41,   238,   185,    18,
     175,    13,   162,    14,     1,     2,     3,     4,     5,     1,
       2,     3,     4,     5,   223,   264,   345,     6,    94,   145,
      41,   325,     6,    18,   255,   224,   346,   -12,   -12,   -12,
      13,   347,    14,    67,    68,    69,    70,    71,    72,    73,
      74,   322,    75,   188,    67,    68,    69,    70,    71,    72,
      73,    74,    18,    75,    13,   338,    14,   226,   171,    13,
     318,    14,   361,    64,    65,    66,   303,   265,    64,    65,
      66,     0,     0,    64,    65,    66,    18,    69,    70,    71,
      72,    18,    67,    68,    69,    70,    71,    72,    73,    74,
       0,   231,    79,     0,    67,    68,    69,    70,    71,    72,
      73,    74,   144,   231,    67,    68,    69,    70,    71,    72,
      73,    74,    54,    75,    67,    68,    69,    70,    71,    72,
       0,   232,  -168,    67,    68,    69,    70,    71,    72,    73,
      74,     0,    75,   232,  -168,     0,     0,     0,     0,     0,
     308,    67,    68,    69,    70,    71,    72,    73,    74,     0,
      75,    79,     0,     0,     0,   123,    67,    68,    69,    70,
      71,    72,    73,    74,     0,    75,   206,   207,   208,   209,
       0,   210,   211,   294,   295,   212,    67,    68,    69,    70,
      71,    72,    73,     0,     0,    75,    67,    68,    69,    70,
      71,    72,     0,     0,     0,    75,   206,   207,   208,   209,
       0,   210,   211,     0,   254,   212,   208,   209,     0,   210,
       0,     0,     0,   212
};

static const yytype_int16 yycheck[] =
{
       0,     0,   120,    19,    18,     7,     6,     7,    19,    18,
      39,    53,    18,    18,    18,    18,    87,    18,    18,   160,
      20,   163,    41,    56,    82,    83,     6,    18,   332,     6,
      19,   173,   174,    19,    18,    21,   157,    18,    63,    25,
      40,    41,    19,    18,    21,    42,    46,   160,    25,    31,
      32,    18,    34,    53,    54,    37,     6,    59,   362,    19,
      40,    94,    40,    65,    66,    28,    29,    67,    68,    69,
      70,    71,    72,    73,    74,    75,   118,    44,    45,    46,
     201,   110,    46,    50,    86,     6,    50,   205,   206,     6,
     106,   110,   234,   235,   236,    95,   110,    99,    19,   170,
     111,   110,   104,    39,   110,   110,   110,   110,   109,     0,
     251,   109,     3,     4,     5,     6,     7,   117,   118,   110,
     262,   121,   124,   105,   110,    16,   110,   169,    19,   110,
      21,    22,    23,    24,    25,   110,   254,     6,   251,    46,
     258,     6,   260,    50,   144,   102,   148,    19,    76,    21,
      78,     6,   294,    25,    19,   157,    21,    19,    27,    21,
      25,    30,    53,    25,    55,    34,    70,    36,    19,   169,
      21,    40,   172,   173,    25,     3,     4,     5,     6,     7,
     298,    72,    73,    74,    75,     6,   186,   186,    16,    26,
      18,    19,    64,    21,    22,    23,    24,    25,   107,   201,
      40,     8,     9,    10,    11,    12,    13,    14,    15,   109,
      17,    90,    91,    64,    18,    56,   107,   219,    19,   261,
       6,    41,   224,    26,   224,    53,    79,    55,   230,    19,
     285,   231,   287,   288,    18,    13,    80,   237,   238,    10,
     282,    60,    61,    62,    72,    73,    74,    75,    41,     6,
       3,     4,     5,     6,     7,    19,    60,    61,    62,    17,
     315,   261,     3,    16,     6,   320,    19,     5,   270,    35,
     270,     3,     4,     5,     6,     7,    60,    61,    62,   107,
       5,    33,   282,    38,    16,    92,    93,    19,    79,    21,
      22,    23,    24,    25,     3,     4,     5,    50,    19,   110,
      53,    66,    55,   111,   306,   307,    19,     5,   308,    81,
     310,   311,   312,     3,     4,     5,     6,     7,    44,    45,
      46,    53,    75,    55,    50,    25,    16,    35,    25,    19,
      18,    84,   332,    86,     3,     4,     5,     6,     7,    18,
      72,    73,    74,    75,    68,    99,    95,    16,    99,    99,
      19,   351,     8,     9,    10,    11,    12,    13,    14,    15,
      95,    17,   362,    53,    65,    55,     3,     4,     5,     6,
       7,     6,    60,    61,    62,   107,     6,    48,    67,    16,
      19,    68,    19,    25,    53,    75,    55,     3,     4,     5,
       6,     7,    18,     6,    84,    27,    86,    19,    30,    19,
      16,    95,    34,    19,    36,    21,    75,    76,    18,    25,
      19,     3,     4,     5,     6,     7,    53,    19,    55,    56,
      57,    68,    59,    19,    16,    19,   110,    19,    69,    85,
      18,    85,    88,   110,    60,    61,    62,    53,    75,    55,
       3,     4,     5,     6,     7,    18,    89,    10,   100,    87,
      60,    61,    62,    16,    44,    47,    19,   186,   140,    75,
      52,    53,   121,    55,     3,     4,     5,     6,     7,     3,
       4,     5,     6,     7,   170,   219,   323,    16,    55,    94,
      19,    18,    16,    75,   207,    19,    18,    60,    61,    62,
      53,    18,    55,     8,     9,    10,    11,    12,    13,    14,
      15,   300,    17,   143,     8,     9,    10,    11,    12,    13,
      14,    15,    75,    17,    53,   312,    55,   172,   129,    53,
     290,    55,   351,    60,    61,    62,   263,   220,    60,    61,
      62,    -1,    -1,    60,    61,    62,    75,    10,    11,    12,
      13,    75,     8,     9,    10,    11,    12,    13,    14,    15,
      -1,    17,    18,    -1,     8,     9,    10,    11,    12,    13,
      14,    15,    77,    17,     8,     9,    10,    11,    12,    13,
      14,    15,    76,    17,     8,     9,    10,    11,    12,    13,
      -1,    47,    48,     8,     9,    10,    11,    12,    13,    14,
      15,    -1,    17,    47,    48,    -1,    -1,    -1,    -1,    -1,
      44,     8,     9,    10,    11,    12,    13,    14,    15,    -1,
      17,    18,    -1,    -1,    -1,    40,     8,     9,    10,    11,
      12,    13,    14,    15,    -1,    17,    95,    96,    97,    98,
      -1,   100,   101,   102,   103,   104,     8,     9,    10,    11,
      12,    13,    14,    -1,    -1,    17,     8,     9,    10,    11,
      12,    13,    -1,    -1,    -1,    17,    95,    96,    97,    98,
      -1,   100,   101,    -1,    95,   104,    97,    98,    -1,   100,
      -1,    -1,    -1,   104
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     7,    16,    19,    21,    22,
      23,    24,    25,    53,    55,    72,    73,    74,    75,   107,
     113,   114,   115,   116,   117,   120,   121,   126,   128,   131,
     133,   135,   140,   141,   142,   143,   144,   145,   146,   147,
      19,    19,   140,   116,   140,    42,   136,    63,     6,    39,
     127,    40,   122,    19,    76,   140,   148,   149,     6,   118,
     119,     0,   114,   109,    60,    61,    62,     8,     9,    10,
      11,    12,    13,    14,    15,    17,   139,   140,    18,    18,
     102,     6,    10,   137,   138,   140,     6,    70,     6,    26,
     107,   123,   139,   140,   148,    78,   149,   150,    40,    19,
     110,   116,   109,    56,   134,   116,   116,   140,   140,   140,
     140,   140,   140,   140,   140,   140,    18,   110,    19,   111,
      39,   110,   157,    40,    19,    64,   116,   132,     6,   129,
     130,   142,    41,    28,    29,    31,    32,    34,    37,   105,
     124,   125,    26,    18,    77,   150,   140,    79,    19,   119,
     116,    80,   151,   140,   139,    10,     6,    19,   158,   159,
     160,   162,   138,    41,   168,     6,     6,   161,   180,    19,
     110,   168,    17,    19,    47,    52,   140,   169,     3,     5,
       6,    33,    35,    38,     5,   125,    19,   106,   151,   140,
      79,   116,    19,   152,    18,    27,    30,    34,    36,   165,
     166,    19,   116,   159,   162,   110,    95,    96,    97,    98,
     100,   101,   104,   163,   165,   166,   169,    66,   172,    18,
     110,   111,   139,   130,    19,   140,   160,   140,   169,   169,
      19,    17,    47,   171,    44,    45,    46,    50,   113,   114,
       5,    18,    81,   153,    25,    35,    25,     6,    40,   116,
      18,    18,   159,   159,    95,   163,    99,    99,    95,    99,
      95,    68,    65,   173,   132,   180,     6,    18,    18,   116,
      19,    56,    57,    59,   170,    48,   169,   169,   169,   140,
      18,    18,    68,    67,   174,    19,    25,    19,    19,   167,
       6,    18,   165,   166,   102,   103,   164,   159,    95,   159,
     159,   139,   169,   174,    18,   116,    19,    19,    44,   139,
      68,    82,    83,   154,   161,    19,   161,   161,   167,   169,
      19,   159,   164,    69,   175,    18,   116,   116,   140,   140,
     176,   177,    50,    84,    86,   140,   155,   156,   155,    18,
      18,   161,    18,    18,   161,   141,    18,    18,    92,    93,
     178,   110,   156,    85,    87,    85,    88,    18,    18,    89,
     179,   177,    44,    90,    91,   156
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   112,   113,   113,   114,   114,   114,   114,   114,   115,
     115,   115,   116,   116,   116,   116,   117,   118,   118,   119,
     120,   120,   120,   121,   121,   121,   122,   122,   122,   123,
     123,   124,   124,   125,   125,   125,   125,   125,   126,   127,
     127,   128,   129,   129,   130,   130,   131,   131,   132,   132,
     133,   133,   133,   134,   134,   135,   136,   136,   136,   137,
     137,   138,   138,   138,   138,   139,   139,   140,   140,   140,
     140,   140,   140,   140,   140,   140,   141,   141,   141,   142,
     143,   144,   144,   144,   144,   144,   144,   144,   144,   144,
     145,   146,   146,   147,   147,   148,   148,   149,   150,   150,
     151,   151,   152,   153,   153,   154,   154,   154,   155,   155,
     156,   156,   156,   156,   157,   157,   158,   158,   159,   159,
     159,   159,   159,   159,   160,   161,   161,   162,   162,   162,
     162,   162,   162,   163,   163,   163,   163,   163,   163,   163,
     164,   164,   165,   165,   166,   166,   166,   166,   166,   166,
     167,   167,   168,   168,   169,   169,   169,   169,   169,   169,
     169,   169,   169,   169,   169,   170,   170,   170,   171,   171,
     172,   172,   173,   173,   174,   174,   175,   175,   176,   176,
     177,   178,   178,   178,   179,   179,   179,   180,   180
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     1,     1,     1,     3,     3,     1,     5,
       1,     1,     1,     7,     7,     6,     0,     4,     4,     0,
       2,     1,     2,     2,     2,     2,     2,     2,     5,     0,
       1,     5,     1,     3,     3,     3,     4,     7,     4,     1,
       3,     3,     4,     0,     1,     9,     0,     1,     5,     1,
       3,     1,     3,     1,     3,     1,     3,     3,     1,     1,
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
       3,     0,     1,     1,     0,     2,     2,     1,     3
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
#line 1680 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 141 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("stmtlist::stmtList::stmt");
				(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[-1].node));	
				bisonParseResult = (Node *) (yyval.list); 
			}
#line 1690 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 150 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::dmlStmt");
            (yyval.node) = (yyvsp[0].node);
        }
#line 1699 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 155 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::queryStmt");
            (yyval.node) = (yyvsp[0].node);
        }
#line 1708 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 160 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::transactionIdentifier");
            (yyval.node) = (Node *) createTransactionStmt((yyvsp[0].stringVal));
        }
#line 1717 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 165 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("stmt::withQuery"); 
			(yyval.node) = (yyvsp[0].node); 
		}
#line 1726 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 170 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("stmt::expression");
			(yyval.node) = (yyvsp[0].node);
		}
#line 1735 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 180 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::insertQuery"); }
#line 1741 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 181 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::deleteQuery"); }
#line 1747 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 182 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::updateQuery"); }
#line 1753 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 189 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::bracketedQuery"); (yyval.node) = (yyvsp[-1].node); }
#line 1759 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 190 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::selectQuery"); }
#line 1765 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 191 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::provStmt"); }
#line 1771 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 192 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::setOperatorQuery"); }
#line 1777 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 197 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withQuery::withViewList::queryStmt");
			(yyval.node) = (Node *) createWithStmt((yyvsp[-1].list), (yyvsp[0].node));
		}
#line 1786 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 205 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withViewList::list::view");
			(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
		}
#line 1795 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 210 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withViewList::view");
			(yyval.list) = singleton((yyvsp[0].node));
		}
#line 1804 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 218 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withView::ident::AS:queryStmt");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString((yyvsp[-4].stringVal)), (yyvsp[-1].node));
		}
#line 1813 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 225 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::BEGIN"); (yyval.stringVal) = strdup("TRANSACTION_BEGIN"); }
#line 1819 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 226 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::COMMIT"); (yyval.stringVal) = strdup("TRANSACTION_COMMIT"); }
#line 1825 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 227 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::ROLLBACK"); (yyval.stringVal) = strdup("TRANSACTION_ABORT"); }
#line 1831 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 235 "sql_parser.y" /* yacc.c:1646  */
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
#line 1846 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 246 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provStmt::stmtlist");
			ProvenanceStmt *p = createProvenanceStmt((Node *) (yyvsp[-1].list));
			p->inputType = PROV_INPUT_UPDATE_SEQUENCE;
			p->provType = PROV_PI_CS;
			p->asOf = (Node *) (yyvsp[-5].node);
			p->options = (yyvsp[-4].list);
			(yyval.node) = (Node *) p;
		}
#line 1860 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 256 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provStmt::transaction");
			ProvenanceStmt *p = createProvenanceStmt((Node *) createConstString((yyvsp[0].stringVal)));
			p->inputType = PROV_INPUT_TRANSACTION;
			p->provType = PROV_PI_CS;
			p->options = (yyvsp[-3].list);
			(yyval.node) = (Node *) p;
		}
#line 1873 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 267 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalProvAsOf::EMPTY"); (yyval.node) = NULL; }
#line 1879 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 269 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvAsOf::SCN");
			(yyval.node) = (Node *) createConstLong((yyvsp[0].intVal));
		}
#line 1888 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 274 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvAsOf::TIMESTAMP");
			(yyval.node) = (Node *) createConstString((yyvsp[0].stringVal));
		}
#line 1897 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 281 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalProvWith::EMPTY"); (yyval.list) = NIL; }
#line 1903 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 283 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvWith::WITH");
			(yyval.list) = (yyvsp[0].list);
		}
#line 1912 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 290 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("provOptionList::option"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 1918 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 292 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("provOptionList::list"); 
			(yyval.list) = appendToTailOfList((yyvsp[-1].list),(yyvsp[0].node)); 
		}
#line 1927 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 300 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("provOption::TYPE"); 
			(yyval.node) = (Node *) createStringKeyValue("TYPE", (yyvsp[0].stringVal)); 
		}
#line 1936 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 305 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::TABLE");
			(yyval.node) = (Node *) createStringKeyValue("TABLE", (yyvsp[0].stringVal));
		}
#line 1945 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 310 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::ONLY::UPDATED");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_ONLY_UPDATED), 
					(Node *) createConstBool(TRUE));
		}
#line 1955 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 316 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::SHOW::INTERMEDIATE");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_SHOW_INTERMEDIATE), 
					(Node *) createConstBool(TRUE));
		}
#line 1965 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 322 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::TUPLE::VERSIONS");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_TUPLE_VERSIONS),
					(Node *) createConstBool(TRUE));
		}
#line 1975 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 334 "sql_parser.y" /* yacc.c:1646  */
    { 
             RULELOG("deleteQuery");
             (yyval.node) = (Node *) createDelete((yyvsp[-2].stringVal), (yyvsp[0].node));
         }
#line 1984 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 342 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::NULL"); (yyval.stringVal) = NULL; }
#line 1990 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 343 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::FROM"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 1996 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 358 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG(updateQuery); 
                (yyval.node) = (Node *) createUpdate((yyvsp[-3].stringVal), (yyvsp[-1].list), (yyvsp[0].node)); 
            }
#line 2005 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 366 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setExpression");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2014 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 371 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setClause::setExpression");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2023 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 379 "sql_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal),"=")) {
                    RULELOG("setExpression::attributeRef::expression");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 2036 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 388 "sql_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal), "=")) {
                    RULELOG("setExpression::attributeRef::queryStmt");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 2049 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 403 "sql_parser.y" /* yacc.c:1646  */
    { 
           	RULELOG("insertQuery::insertList"); 
           	(yyval.node) = (Node *) createInsert((yyvsp[-1].stringVal),(Node *) (yyvsp[0].node), NIL); 
        }
#line 2058 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 408 "sql_parser.y" /* yacc.c:1646  */
    { 
           	RULELOG("insertQuery::insertList"); 
           	(yyval.node) = (Node *) createInsert((yyvsp[-4].stringVal),(Node *) (yyvsp[0].node), (yyvsp[-2].list)); 
     	}
#line 2067 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 421 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (Node *) (yyvsp[-1].list); }
#line 2073 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 481 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::INTERSECT");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 2082 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 486 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::MINUS");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 2091 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 491 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::UNION");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-2].stringVal), ((yyvsp[-1].stringVal) != NULL), (yyvsp[-3].node), (yyvsp[0].node));
            }
#line 2100 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 498 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::NULL"); (yyval.stringVal) = NULL; }
#line 2106 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 499 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::ALLTRUE"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2112 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 509 "sql_parser.y" /* yacc.c:1646  */
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
#line 2132 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 531 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalDistinct::NULL"); (yyval.node) = NULL; }
#line 2138 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 533 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT");
                (yyval.node) = (Node *) createDistinctClause(NULL);
            }
#line 2147 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 538 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT::exprList");
                (yyval.node) = (Node *) createDistinctClause((yyvsp[-1].list));
            }
#line 2156 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 550 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectItem"); (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2164 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 554 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectClause::selectItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node)); 
            }
#line 2173 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 562 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression"); 
                 (yyval.node) = (Node *) createSelectItem(NULL, (yyvsp[0].node)); 
             }
#line 2182 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 567 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression::identifier"); 
                 (yyval.node) = (Node *) createSelectItem((yyvsp[0].stringVal), (yyvsp[-2].node));
             }
#line 2191 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 572 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
         		(yyval.node) = (Node *) createSelectItem(strdup("*"), NULL); 
     		}
#line 2200 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 577 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
     			(yyval.node) = (Node *) createSelectItem(CONCAT_STRINGS((yyvsp[-2].stringVal),".*"), NULL); 
 			}
#line 2209 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 587 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("exprList::SINGLETON"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 2215 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 589 "sql_parser.y" /* yacc.c:1646  */
    {
                  RULELOG("exprList::exprList::expression");
                  (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
             }
#line 2224 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 600 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::bracked"); (yyval.node) = (yyvsp[-1].node); }
#line 2230 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 601 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::constant"); }
#line 2236 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 602 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::attributeRef"); }
#line 2242 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 603 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlParameter"); }
#line 2248 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 604 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::binaryOperatorExpression"); }
#line 2254 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 605 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::unaryOperatorExpression"); }
#line 2260 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 606 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlFunctionCall"); }
#line 2266 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 607 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::case"); }
#line 2272 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 608 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::ROWNUM"); (yyval.node) = (Node *) makeNode(RowNumExpr); }
#line 2278 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 617 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::INT"); (yyval.node) = (Node *) createConstInt((yyvsp[0].intVal)); }
#line 2284 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 618 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::FLOAT"); (yyval.node) = (Node *) createConstFloat((yyvsp[0].floatVal)); }
#line 2290 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 619 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::STRING"); (yyval.node) = (Node *) createConstString((yyvsp[0].stringVal)); }
#line 2296 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 626 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("attributeRef::IDENTIFIER"); (yyval.node) = (Node *) createAttributeReference((yyvsp[0].stringVal)); }
#line 2302 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 632 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("sqlParameter::PARAMETER"); (yyval.node) = (Node *) createSQLParameter((yyvsp[0].stringVal)); }
#line 2308 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 652 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '+' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2319 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 659 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '-' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2330 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 666 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '*' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2341 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 673 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '/' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2352 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 680 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '%' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2363 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 687 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '^' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2374 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 696 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '&' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2385 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 703 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '|' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2396 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 712 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression::comparisonOps");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2407 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 722 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("unaryOperatorExpression:: '!' ");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2417 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 734 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("sqlFunctionCall::IDENTIFIER::exprList");
				FunctionCall *f = createFunctionCall((yyvsp[-4].stringVal), (yyvsp[-2].list));
				if ((yyvsp[0].node) != NULL)
					(yyval.node) = (Node *) createWindowFunction(f, (WindowDef *) (yyvsp[0].node));
				else  
                	(yyval.node) = (Node *) f; 
            }
#line 2430 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 743 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("sqlFunctionCall::AMMSC::exprList");
				FunctionCall *f = createFunctionCall((yyvsp[-4].stringVal), (yyvsp[-2].list));
				if ((yyvsp[0].node) != NULL)
					(yyval.node) = (Node *) createWindowFunction(f, (WindowDef *) (yyvsp[0].node));
				else  
                	(yyval.node) = (Node *) f; 
            }
#line 2443 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 758 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseExpression::CASE::expression::whens:else:END");
				(yyval.node) = (Node *) createCaseExpr((yyvsp[-3].node), (yyvsp[-2].list), (yyvsp[-1].node));
			}
#line 2452 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 763 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseExpression::CASE::whens::else::END");
				(yyval.node) = (Node *) createCaseExpr(NULL, (yyvsp[-2].list), (yyvsp[-1].node));
			}
#line 2461 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 771 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhenList::list::caseWhen");
				(yyval.list) = appendToTailOfList((yyvsp[-1].list), (yyvsp[0].node));
			}
#line 2470 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 776 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhenList::caseWhen");
				(yyval.list) = singleton((yyvsp[0].node));
			}
#line 2479 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 784 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhen::WHEN::expression::THEN::expression");
				(yyval.node) = (Node *) createCaseWhen((yyvsp[-2].node),(yyvsp[0].node));
			}
#line 2488 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 791 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalCaseElse::NULL"); (yyval.node) = NULL; }
#line 2494 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 793 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalCaseElse::ELSE::expression");
				(yyval.node) = (yyvsp[0].node);
			}
#line 2503 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 803 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("overclause::NULL"); (yyval.node) = NULL; }
#line 2509 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 805 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("overclause::window");
				(yyval.node) = (yyvsp[0].node);
			}
#line 2518 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 813 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("window");
				(yyval.node) = (Node *) createWindowDef((yyvsp[-3].list),(yyvsp[-2].list), (WindowFrame *) (yyvsp[-1].node));
			}
#line 2527 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 820 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optWindowPart::NULL"); (yyval.list) = NIL; }
#line 2533 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 822 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optWindowPart::PARTITION:BY::expressionList");
				(yyval.list) = (yyvsp[0].list);
			}
#line 2542 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 829 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optWindowFrame::NULL"); (yyval.node) = NULL; }
#line 2548 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 831 "sql_parser.y" /* yacc.c:1646  */
    { 
				WindowBound *l, *u = NULL;
				RULELOG("optWindowFrame::ROWS::windoBoundaries");
				l = getNthOfListP((yyvsp[0].list), 0);
				if(LIST_LENGTH((yyvsp[0].list)) > 1)
					u = getNthOfListP((yyvsp[0].list), 1);
				(yyval.node) = (Node *) createWindowFrame(WINFRAME_ROWS, l, u); 
			}
#line 2561 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 840 "sql_parser.y" /* yacc.c:1646  */
    {
				WindowBound *l, *u = NULL; 
				RULELOG("optWindowFrame::RANGE::windoBoundaries"); 
				l = getNthOfListP((yyvsp[0].list), 0);
				if(LIST_LENGTH((yyvsp[0].list)) > 1)
					u = getNthOfListP((yyvsp[0].list), 1);
				(yyval.node) = (Node *) createWindowFrame(WINFRAME_RANGE, l, u); 
			}
#line 2574 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 852 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBoundaries::BETWEEN"); 
				(yyval.list) = LIST_MAKE((yyvsp[-2].node), (yyvsp[0].node)); 
			}
#line 2583 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 857 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBoundaries::windowBound"); 
				(yyval.list) = singleton((yyvsp[0].node)); 
			}
#line 2592 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 865 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::UNBOUNDED::PRECEDING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_UNBOUND_PREC, NULL); 
			}
#line 2601 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 870 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::CURRENT::ROW"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_CURRENT_ROW, NULL); 
			}
#line 2610 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 875 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::expression::PRECEDING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_EXPR_PREC, (yyvsp[-1].node)); 
			}
#line 2619 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 880 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::expression::FOLLOWING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_EXPR_FOLLOW, (yyvsp[-1].node)); 
			}
#line 2628 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 892 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::NULL"); (yyval.list) = NULL; }
#line 2634 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 893 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::fromClause"); (yyval.list) = (yyvsp[0].list); }
#line 2640 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 898 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClauseItem");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2649 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 903 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClause::fromClauseItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2658 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 912 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
				FromItem *f = createFromTableRef(NULL, NIL, (yyvsp[-1].stringVal));
				f->provInfo = (FromProvInfo *) (yyvsp[0].node);
                (yyval.node) = (Node *) f;
            }
#line 2669 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 919 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
                FromItem *f = createFromTableRef(((FromItem *) (yyvsp[0].node))->name, 
						((FromItem *) (yyvsp[0].node))->attrNames, (yyvsp[-1].stringVal));
				f->provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
                (yyval.node) = (Node *) f;
            }
#line 2681 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 928 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromItem *f = (FromItem *) (yyvsp[-1].node);
                f->provInfo = (FromProvInfo *) (yyvsp[0].node);
                (yyval.node) = (yyvsp[-1].node);
            }
#line 2692 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 935 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromSubquery *s = (FromSubquery *) (yyvsp[-1].node);
                s->from.name = ((FromItem *) (yyvsp[0].node))->name;
                s->from.attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
                s->from.provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
                (yyval.node) = (Node *) s;
            }
#line 2705 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 944 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[0].node);
        		f->name = NULL;
        		(yyval.node) = (Node *) f;
        	}
#line 2717 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 952 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[-2].node);
        		f->name = ((FromItem *) (yyvsp[0].node))->name;
                f->attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
                f->provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
        		(yyval.node) = (Node *) f;
        	}
#line 2731 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 965 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("subQuery::queryStmt");
                (yyval.node) = (Node *) createFromSubquery(NULL, NULL, (yyvsp[-1].node));
            }
#line 2740 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 972 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = singleton((yyvsp[0].stringVal)); }
#line 2746 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 973 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].stringVal)); }
#line 2752 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 977 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2758 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 979 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURAL");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
						(FromItem *) (yyvsp[0].node), "JOIN_INNER", "JOIN_COND_NATURAL", 
						NULL);
          	}
#line 2769 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 986 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURALjoinType");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[0].node), (yyvsp[-2].stringVal), "JOIN_COND_NATURAL", NULL);
          	}
#line 2779 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 992 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::CROSS JOIN");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[0].node), "JOIN_CROSS", "JOIN_COND_ON", NULL);
          	}
#line 2789 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 998 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinType::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON";
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[-1].node), (yyvsp[-3].stringVal), condType, (yyvsp[0].node));
          	}
#line 2801 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 1006 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON"; 
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[-1].node), "JOIN_INNER", 
                		condType, (yyvsp[0].node));
          	}
#line 2814 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 1017 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2820 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 1018 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT OUTER"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2826 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 1019 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT "); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2832 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 1020 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT OUTER"); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2838 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 1021 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL OUTER"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2844 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 1022 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2850 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 1023 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::INNER"); (yyval.stringVal) = "JOIN_INNER"; }
#line 2856 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 1027 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (Node *) (yyvsp[-1].list); }
#line 2862 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 1028 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2868 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 1033 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[-2].node);
				(yyval.node) = (Node *) f;
			}
#line 2879 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 1040 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[-3].node); 
				(yyval.node) = (Node *) f;
			}
#line 2890 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 1049 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFromProv::empty"); (yyval.node) = NULL; }
#line 2896 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1051 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::BASERELATION");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = TRUE;
				p->userProvAttrs = NIL;				 
				(yyval.node) = (Node *) p; 
			}
#line 2908 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1059 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::userProvAttr");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = FALSE;
				p->userProvAttrs = (yyvsp[-1].list);				 
				(yyval.node) = (Node *) p; 
			}
#line 2920 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1067 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::userProvDupAttr");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = TRUE;
				p->userProvAttrs = (yyvsp[-1].list);
				(yyval.node) = (Node *) p;
			}
#line 2932 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1075 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::intermediateProv");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->intermediateProv = TRUE;
				(yyval.node) = (Node *) p;
			}
#line 2943 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1082 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::intermediateProv::attrList");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->intermediateProv = TRUE;
				p->userProvAttrs = (yyvsp[-1].list);
				(yyval.node) = (Node *) p;
			}
#line 2955 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1092 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAttrAlias::empty"); (yyval.list) = NULL; }
#line 2961 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1094 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAttrAlias::identifierList"); (yyval.list) = (yyvsp[-1].list); 
			}
#line 2969 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1103 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::NULL"); (yyval.node) = NULL; }
#line 2975 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1104 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::whereExpression"); (yyval.node) = (yyvsp[0].node); }
#line 2981 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1108 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("where::brackedWhereExpression"); (yyval.node) = (yyvsp[-1].node); }
#line 2987 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1109 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("whereExpression::expression"); (yyval.node) = (yyvsp[0].node); }
#line 2993 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 1111 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::NOT");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 3003 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 1117 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 3014 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1124 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::OR");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 3025 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1131 "sql_parser.y" /* yacc.c:1646  */
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
#line 3047 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 1149 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::BETWEEN-AND");
                List *expr = singleton((yyvsp[-4].node));
                expr = appendToTailOfList(expr, (yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr);
            }
#line 3059 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1157 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::comparisonOps::nestedSubQueryOperator::Subquery");
                (yyval.node) = (Node *) createNestedSubquery((yyvsp[-3].stringVal), (yyvsp[-5].node), (yyvsp[-4].stringVal), (yyvsp[-1].node));
            }
#line 3068 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1162 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::Subquery");
                Node *q = (Node *) createNestedSubquery("SCALAR", NULL, NULL, (yyvsp[-1].node)); 
                List *expr = LIST_MAKE((yyvsp[-4].node), q);
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr); 
            }
#line 3079 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1169 "sql_parser.y" /* yacc.c:1646  */
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
#line 3096 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 1182 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::EXISTS");
                (yyval.node) = (Node *) createNestedSubquery((yyvsp[-3].stringVal), NULL, NULL, (yyvsp[-1].node));
            }
#line 3105 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 1189 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ANY"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 3111 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 1190 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ALL"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 3117 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1191 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::SOME"); (yyval.stringVal) = "ANY"; }
#line 3123 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 1195 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NULL"); (yyval.stringVal) = NULL; }
#line 3129 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 1196 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NOT"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 3135 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 1200 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::NULL"); (yyval.list) = NULL; }
#line 3141 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 1201 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::GROUPBY"); (yyval.list) = (yyvsp[0].list); }
#line 3147 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 1205 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.node) = NULL; }
#line 3153 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 1207 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("optionalHaving::HAVING"); 
                (yyval.node) = (Node *) (yyvsp[0].node);
            }
#line 3162 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 1214 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.list) = NULL; }
#line 3168 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 1215 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy::ORDERBY"); (yyval.list) = (yyvsp[0].list); }
#line 3174 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 1219 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::NULL"); (yyval.node) = NULL; }
#line 3180 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 1220 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::CONSTANT"); (yyval.node) = (yyvsp[0].node);}
#line 3186 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 1225 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::orderExpr");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 3195 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 1230 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("orderList::orderList::orderExpr");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 3204 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 1238 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("orderExpr::expr::sortOrder::nullOrder");
				SortOrder o = (strcmp((yyvsp[-1].stringVal),"ASC") == 0) ?  SORT_ASC : SORT_DESC;
				SortNullOrder n = (strcmp((yyvsp[0].stringVal),"NULLS_FIRST") == 0) ? 
						SORT_NULLS_FIRST : 
						SORT_NULLS_LAST;
				(yyval.node) = (Node *) createOrderExpr((yyvsp[-2].node), o, n);
			}
#line 3217 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 1249 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalSortOrder::empty"); (yyval.stringVal) = "ASC"; }
#line 3223 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 1251 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalSortOrder::ASC");
				(yyval.stringVal) = "ASC";
			}
#line 3232 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 1256 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalSortOrder::DESC");
				(yyval.stringVal) = "DESC";
			}
#line 3241 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 1263 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNullOrder::empty"); (yyval.stringVal) = "NULLS_LAST"; }
#line 3247 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 1265 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalNullOrder::NULLS FIRST");
				(yyval.stringVal) = "NULLS_FIRST";
			}
#line 3256 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 186:
#line 1270 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalNullOrder::NULLS LAST");
				(yyval.stringVal) = "NULLS_LAST";
			}
#line 3265 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 187:
#line 1277 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("identifierList::ident"); }
#line 3271 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 188:
#line 1279 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("identifierList::list::ident"); 
			(yyval.stringVal) = CONCAT_STRINGS((yyvsp[-2].stringVal), ".", (yyvsp[0].stringVal)); //TODO 
		}
#line 3280 "sql_parser.tab.c" /* yacc.c:1646  */
    break;


#line 3284 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 1285 "sql_parser.y" /* yacc.c:1906  */




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

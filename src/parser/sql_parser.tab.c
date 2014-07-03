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
#define YYLAST   618

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  112
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  68
/* YYNRULES -- Number of rules.  */
#define YYNRULES  187
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  362

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
       0,   133,   133,   139,   148,   153,   158,   163,   174,   175,
     176,   183,   184,   185,   186,   190,   198,   203,   211,   219,
     220,   221,   228,   239,   249,   261,   262,   267,   275,   276,
     284,   285,   293,   298,   303,   309,   315,   327,   336,   337,
     351,   359,   364,   372,   381,   396,   401,   409,   414,   419,
     424,   438,   443,   448,   456,   457,   466,   489,   490,   495,
     507,   511,   519,   524,   529,   534,   545,   546,   558,   559,
     560,   561,   562,   563,   564,   565,   566,   575,   576,   577,
     584,   590,   609,   616,   623,   630,   637,   644,   653,   660,
     669,   679,   691,   700,   715,   720,   728,   733,   741,   749,
     750,   761,   762,   770,   778,   779,   787,   788,   797,   809,
     814,   822,   827,   832,   837,   850,   851,   855,   860,   869,
     876,   885,   892,   901,   909,   922,   930,   931,   935,   936,
     943,   949,   955,   963,   975,   976,   977,   978,   979,   980,
     981,   985,   986,   990,   997,  1007,  1008,  1016,  1024,  1032,
    1039,  1050,  1051,  1061,  1062,  1066,  1067,  1068,  1074,  1081,
    1088,  1106,  1114,  1119,  1126,  1139,  1147,  1148,  1149,  1153,
    1154,  1158,  1159,  1163,  1164,  1172,  1173,  1177,  1178,  1182,
    1187,  1195,  1207,  1208,  1213,  1221,  1222,  1227
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
  "setExpression", "insertQuery", "insertList", "setOperatorQuery",
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
  "orderList", "orderExpr", "optionalSortOrder", "optionalNullOrder", YY_NULLPTR
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

#define YYPACT_NINF -323

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-323)))

#define YYTABLE_NINF -170

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-170)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     119,    31,    -8,   -15,    34,    37,    39,  -323,  -323,  -323,
     123,    61,     7,  -323,   117,  -323,  -323,  -323,  -323,  -323,
    -323,  -323,  -323,   114,    35,   413,   133,    79,  -323,   159,
     156,    77,   145,    18,  -323,  -323,    78,  -323,   139,    31,
      31,  -323,   170,  -323,  -323,  -323,   -12,  -323,  -323,   353,
     353,   204,  -323,   299,   -17,  -323,   542,  -323,  -323,  -323,
    -323,  -323,  -323,  -323,    93,   219,   186,    29,   135,   202,
     211,   123,   117,  -323,  -323,    31,  -323,  -323,   353,   353,
     223,   227,   585,   552,   353,   353,   461,    41,  -323,    30,
     413,   214,   353,   353,   353,   353,   353,   353,   353,   353,
     353,   252,   242,   117,  -323,   -23,  -323,   246,   360,   261,
     264,   265,   240,   259,   243,   279,   135,  -323,    -5,    31,
    -323,  -323,    -3,   563,    -2,  -323,  -323,    -1,   448,    41,
     353,  -323,   216,   181,   241,   209,   508,   181,  -323,  -323,
     360,   248,   276,   276,   283,   283,   283,  -323,   585,   473,
     605,  -323,   421,   219,  -323,   436,   360,   360,   293,   495,
     230,  -323,  -323,  -323,  -323,  -323,  -323,  -323,  -323,   119,
     315,   152,  -323,   353,   253,   253,   353,   244,   563,  -323,
    -323,   296,   301,   320,  -323,    38,   241,   176,   508,   329,
      30,    30,   251,   262,   263,  -323,   273,   271,   278,  -323,
      38,   230,   303,   312,  -323,     3,  -323,  -323,   378,   563,
    -323,   484,     1,   230,    31,   334,  -323,   330,   360,   360,
     360,   353,    73,    -7,  -323,  -323,   563,   361,  -323,  -323,
     563,  -323,   367,   363,   373,   376,   390,   188,  -323,   494,
     508,   488,    30,   305,  -323,  -323,    30,  -323,    30,   353,
     360,   331,  -323,   535,   378,  -323,   191,   378,  -323,  -323,
    -323,   382,   391,   108,   108,   110,   505,  -323,  -323,   333,
    -323,   405,   402,   405,   405,  -323,   376,   217,  -323,    38,
     360,   403,  -323,  -323,    30,  -323,   488,   324,   230,   362,
     368,  -323,  -323,  -323,   249,    31,    31,   353,   370,   331,
    -323,     5,   405,    12,    13,  -323,   230,   405,  -323,  -323,
     353,    59,  -323,  -323,   282,   386,   563,   353,    44,  -323,
     430,    14,  -323,  -323,    15,   190,   335,  -323,  -323,  -323,
    -323,   324,   215,   215,   426,  -323,  -323,  -323,  -323,  -323,
     365,   353,   238,   364,   377,   317,  -323,  -323,  -323,  -323,
      72,  -323,  -323,   406,  -323,  -323,  -323,  -323,  -323,  -323,
     238,  -323
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
       0,    11,     0,    77,    78,    79,    80,    81,    64,     0,
       0,     0,    76,     0,   115,    60,    62,    69,    70,    71,
      72,    73,    74,    75,     0,     0,     0,     0,     0,     0,
       0,     0,    15,     3,    55,     0,    51,    52,     0,     0,
       0,    80,    91,     0,     0,     0,     0,    99,    97,     0,
       0,   153,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    46,    80,   153,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    29,    30,     0,     0,
      16,    53,     0,    66,     0,    65,    68,     0,     0,    99,
       0,    96,     0,   145,     0,   116,   117,   145,   123,    61,
       0,   171,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    63,     0,     0,    40,     0,     0,     0,     0,   156,
      37,    26,    27,    33,    34,    35,    36,    32,    31,     0,
       0,     0,    59,     0,   101,   101,     0,     0,   100,    95,
     146,     0,     0,     0,   120,   119,     0,     0,     0,   123,
       0,     0,     0,   134,   136,   140,     0,   139,     0,   122,
     121,   154,     0,   173,    48,     0,    47,    42,     0,    43,
      44,   156,     0,   157,     0,     0,   170,     0,     0,     0,
       0,     0,     0,     0,    24,    18,    67,     0,    92,    93,
      98,    94,     0,     0,     0,   151,     0,     0,   125,   128,
     118,     0,     0,     0,   135,   137,     0,   138,     0,     0,
       0,   175,    45,     0,     0,   155,     0,     0,   167,   166,
     168,     0,     0,   158,   159,   160,     0,    23,    22,   104,
     102,     0,   149,     0,     0,   143,   151,   125,   124,     0,
       0,     0,   133,   129,     0,   131,     0,   172,   174,     0,
     177,    49,    50,   165,     0,     0,     0,     0,     0,   175,
     126,     0,     0,     0,     0,   144,   142,     0,   130,   132,
       0,     0,    56,   163,     0,     0,   161,     0,   106,   147,
       0,     0,   148,   152,     0,   182,   176,   179,   178,   162,
     164,   105,     0,     0,     0,   127,   150,   141,   183,   184,
     185,     0,     0,     0,     0,     0,   107,   110,   108,   103,
       0,   181,   180,     0,   111,   112,   113,   114,   186,   187,
       0,   109
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -323,   298,     4,  -323,     2,  -323,  -323,   380,  -323,  -323,
    -323,  -323,  -323,   407,  -323,  -323,  -323,  -323,   326,  -323,
    -323,  -323,  -323,  -323,  -323,  -323,   387,   -78,   -25,  -147,
     -53,  -323,  -323,  -323,  -323,  -323,   394,   -74,   397,   352,
    -323,  -323,  -323,   196,  -322,  -323,  -323,   -87,   381,  -121,
    -105,   341,   258,  -129,  -128,   269,   441,  -130,  -323,  -323,
    -323,  -323,   236,  -323,  -323,   206,  -323,  -323
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    11,    36,    13,    14,    15,    33,    34,    16,    17,
      31,    69,   116,   117,    18,    29,    19,   105,   106,    20,
     205,    21,    75,    22,    25,    54,    55,   122,   159,    57,
      58,    59,    60,    61,    62,    63,    87,    88,   132,   228,
     270,   299,   334,   346,   347,    91,   135,   188,   137,   301,
     138,   198,   282,   184,   185,   275,   141,   160,   261,   217,
     203,   251,   290,   312,   326,   327,   340,   351
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      56,   124,   136,    23,    12,   206,   127,    79,   199,   200,
     201,   268,   107,   131,   169,   172,   174,   175,   140,   255,
     353,   252,    89,   319,    82,    83,   212,   213,    86,   189,
     322,   323,   336,   337,    24,    72,   133,     1,   361,     2,
      27,    76,    77,     6,   235,   218,   219,   220,    26,   134,
       1,   221,     2,   123,   123,   131,     6,   109,   110,   123,
     128,    35,    43,    44,    45,    56,   103,   142,   143,   144,
     145,   146,   147,   148,   149,   150,    28,   121,   236,    30,
       1,   189,     2,     3,     4,     5,     6,   153,   263,   264,
     265,   267,     1,    90,     2,     3,     4,     5,     6,    80,
     107,   170,    37,   240,   241,   178,   292,   173,   173,   173,
     278,   279,     1,   253,     2,   320,    37,    85,     6,   130,
     288,   171,   320,   320,   320,   320,   332,   333,    71,    32,
     209,   211,    41,     7,     8,     9,   187,    42,     1,    64,
       2,     3,     4,     5,     6,     7,     8,     9,   226,    65,
     306,   230,   303,   304,   220,   283,  -170,   102,   221,   285,
    -170,   286,   358,   359,   328,    66,   111,   112,    10,   113,
     225,   287,   114,   223,    38,    39,    40,    38,    39,    40,
      10,   321,    67,    83,    68,    70,   324,    73,   237,    78,
     150,     7,     8,     9,   238,    74,   266,   308,    92,    93,
      94,    95,    96,    97,    98,    99,   277,   100,   180,   293,
     187,   181,    38,    39,    40,   182,   256,   183,    43,    44,
      45,    81,    47,    84,   123,   104,    10,   108,   118,    83,
     119,    49,    83,   125,    50,   -11,    38,    39,    40,   331,
     115,    43,    44,    45,    81,    47,    79,   133,    38,    39,
      40,    38,    39,    40,    49,   140,    23,    50,   151,   294,
     186,   152,     2,   155,   161,   342,     6,   313,    51,   162,
      52,   163,   316,   164,   218,   219,   220,   -11,   -11,   -11,
     221,   166,   338,   339,   167,   325,    94,    95,    96,    97,
      53,    51,   123,    52,   165,   179,    97,   314,   315,   343,
     329,   344,    43,    44,    45,    81,    47,   345,   345,    38,
      39,    40,   214,    53,   202,    49,   325,   345,    50,   190,
     224,   232,   343,   231,   344,    92,    93,    94,    95,    96,
      97,    98,    99,   227,   100,   345,   233,    43,    44,    45,
      81,    47,    38,    39,    40,   234,   242,   239,   193,   194,
      49,   195,    51,   257,    52,   197,    43,    44,    45,    81,
      47,   244,   245,    43,    44,    45,    81,    47,   246,    49,
     247,   249,    50,   248,    53,    85,    49,   250,   262,   156,
     269,    43,    44,    45,    81,    47,   271,    51,   272,    52,
     258,   259,   273,   260,    49,   274,   276,   254,   289,     2,
     284,   295,   356,     6,   330,   357,    51,   157,    52,    53,
     296,   300,   158,    51,   298,    52,    43,    44,    45,    46,
      47,   302,   307,    48,    43,    44,    45,   204,    53,    49,
     310,    51,    50,    52,   173,    53,   335,   311,   317,    43,
      44,    45,    81,    47,   349,   341,    38,    39,    40,   354,
     360,   120,    49,    53,   350,   208,    92,    93,    94,    95,
      96,    97,    98,    99,   355,   100,    51,   222,    52,    92,
      93,    94,    95,    96,    97,    98,    99,   139,   100,   207,
     129,    92,    93,    94,    95,    96,    97,    98,    53,    51,
     100,    52,    92,    93,    94,    95,    96,    97,    98,    99,
    -145,   215,   126,    92,    93,    94,    95,    96,    97,    98,
      99,    53,   215,    92,    93,    94,    95,    96,    97,    98,
      99,   180,   100,   168,   181,   176,   177,   229,   182,   348,
     183,   216,  -169,   243,  -145,   318,   210,    85,    43,    44,
      45,   291,   216,  -169,   309,   305,   154,   352,     0,   297,
      92,    93,    94,    95,    96,    97,    98,    99,     0,   100,
      92,    93,    94,    95,    96,    97,    98,    99,     0,   100,
     126,    92,    93,    94,    95,    96,    97,    98,    99,     0,
     100,     0,   101,   191,   192,   193,   194,     0,   195,   196,
     280,   281,   197,    92,    93,    94,    95,    96,    97,     0,
       0,     0,   100,   191,   192,   193,   194,     0,   195,   196,
       0,     0,   197,    92,    93,    94,    95,    96,    97
};

static const yytype_int16 yycheck[] =
{
      25,    79,    89,     1,     0,   152,    84,    19,   137,   137,
     140,    18,    65,    87,    19,    18,    18,    18,    41,    18,
     342,    18,    39,    18,    49,    50,   156,   157,    53,   134,
      18,    18,    18,    18,    42,    33,     6,    19,   360,    21,
       6,    39,    40,    25,     6,    44,    45,    46,    63,    19,
      19,    50,    21,    78,    79,   129,    25,    28,    29,    84,
      85,     0,     3,     4,     5,    90,    64,    92,    93,    94,
      95,    96,    97,    98,    99,   100,    39,    75,    40,    40,
      19,   186,    21,    22,    23,    24,    25,   110,   218,   219,
     220,    18,    19,   110,    21,    22,    23,    24,    25,   111,
     153,   106,   109,   190,   191,   130,   253,   110,   110,   110,
     239,   239,    19,   110,    21,   110,   109,    76,    25,    78,
     250,   119,   110,   110,   110,   110,    82,    83,   110,     6,
     155,   156,    18,    72,    73,    74,   134,   102,    19,     6,
      21,    22,    23,    24,    25,    72,    73,    74,   173,    70,
     280,   176,   273,   274,    46,   242,    46,    64,    50,   246,
      50,   248,    90,    91,   311,     6,    31,    32,   107,    34,
      18,   249,    37,   169,    60,    61,    62,    60,    61,    62,
     107,   302,    26,   208,   107,    40,   307,   109,   186,    19,
     215,    72,    73,    74,    18,    56,   221,   284,     8,     9,
      10,    11,    12,    13,    14,    15,    18,    17,    27,    18,
     208,    30,    60,    61,    62,    34,   214,    36,     3,     4,
       5,     6,     7,    19,   249,     6,   107,    41,    26,   254,
      19,    16,   257,    10,    19,    18,    60,    61,    62,   317,
     105,     3,     4,     5,     6,     7,    19,     6,    60,    61,
      62,    60,    61,    62,    16,    41,   254,    19,     6,   257,
      19,    19,    21,    17,     3,    50,    25,    18,    53,     5,
      55,     6,   297,    33,    44,    45,    46,    60,    61,    62,
      50,    38,    92,    93,     5,   310,    10,    11,    12,    13,
      75,    53,   317,    55,    35,    79,    13,   295,   296,    84,
      18,    86,     3,     4,     5,     6,     7,   332,   333,    60,
      61,    62,    19,    75,    66,    16,   341,   342,    19,   110,
       5,    25,    84,    79,    86,     8,     9,    10,    11,    12,
      13,    14,    15,    80,    17,   360,    35,     3,     4,     5,
       6,     7,    60,    61,    62,    25,    95,    18,    97,    98,
      16,   100,    53,    19,    55,   104,     3,     4,     5,     6,
       7,    99,    99,     3,     4,     5,     6,     7,    95,    16,
      99,    68,    19,    95,    75,    76,    16,    65,    48,    19,
      19,     3,     4,     5,     6,     7,    19,    53,    25,    55,
      56,    57,    19,    59,    16,    19,     6,    19,    67,    21,
      95,    19,    85,    25,    18,    88,    53,    47,    55,    75,
      19,     6,    52,    53,    81,    55,     3,     4,     5,     6,
       7,    19,    19,    10,     3,     4,     5,     6,    75,    16,
      68,    53,    19,    55,   110,    75,     6,    69,    68,     3,
       4,     5,     6,     7,    18,   110,    60,    61,    62,    85,
      44,    71,    16,    75,    89,    19,     8,     9,    10,    11,
      12,    13,    14,    15,    87,    17,    53,   169,    55,     8,
       9,    10,    11,    12,    13,    14,    15,    90,    17,   153,
      86,     8,     9,    10,    11,    12,    13,    14,    75,    53,
      17,    55,     8,     9,    10,    11,    12,    13,    14,    15,
       6,    17,    18,     8,     9,    10,    11,    12,    13,    14,
      15,    75,    17,     8,     9,    10,    11,    12,    13,    14,
      15,    27,    17,   116,    30,    77,   129,   175,    34,   333,
      36,    47,    48,   192,    40,   299,   155,    76,     3,     4,
       5,     6,    47,    48,   286,   276,   105,   341,    -1,    44,
       8,     9,    10,    11,    12,    13,    14,    15,    -1,    17,
       8,     9,    10,    11,    12,    13,    14,    15,    -1,    17,
      18,     8,     9,    10,    11,    12,    13,    14,    15,    -1,
      17,    -1,    40,    95,    96,    97,    98,    -1,   100,   101,
     102,   103,   104,     8,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    17,    95,    96,    97,    98,    -1,   100,   101,
      -1,    -1,   104,     8,     9,    10,    11,    12,    13
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
      17,    40,    64,   116,     6,   129,   130,   142,    41,    28,
      29,    31,    32,    34,    37,   105,   124,   125,    26,    19,
     119,   116,   139,   140,   139,    10,    18,   139,   140,   148,
      78,   149,   150,     6,    19,   158,   159,   160,   162,   138,
      41,   168,   140,   140,   140,   140,   140,   140,   140,   140,
     140,     6,    19,   110,   168,    17,    19,    47,    52,   140,
     169,     3,     5,     6,    33,    35,    38,     5,   125,    19,
     106,   116,    18,   110,    18,    18,    77,   150,   140,    79,
      27,    30,    34,    36,   165,   166,    19,   116,   159,   162,
     110,    95,    96,    97,    98,   100,   101,   104,   163,   165,
     166,   169,    66,   172,     6,   132,   141,   130,    19,   140,
     160,   140,   169,   169,    19,    17,    47,   171,    44,    45,
      46,    50,   113,   114,     5,    18,   140,    80,   151,   151,
     140,    79,    25,    35,    25,     6,    40,   116,    18,    18,
     159,   159,    95,   163,    99,    99,    95,    99,    95,    68,
      65,   173,    18,   110,    19,    18,   116,    19,    56,    57,
      59,   170,    48,   169,   169,   169,   140,    18,    18,    19,
     152,    19,    25,    19,    19,   167,     6,    18,   165,   166,
     102,   103,   164,   159,    95,   159,   159,   139,   169,    67,
     174,     6,   141,    18,   116,    19,    19,    44,    81,   153,
       6,   161,    19,   161,   161,   167,   169,    19,   159,   164,
      68,    69,   175,    18,   116,   116,   140,    68,   174,    18,
     110,   161,    18,    18,   161,   140,   176,   177,   141,    18,
      18,   139,    82,    83,   154,     6,    18,    18,    92,    93,
     178,   110,    50,    84,    86,   140,   155,   156,   155,    18,
      89,   179,   177,   156,    85,    87,    85,    88,    90,    91,
      44,   156
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   112,   113,   113,   114,   114,   114,   114,   115,   115,
     115,   116,   116,   116,   116,   117,   118,   118,   119,   120,
     120,   120,   121,   121,   121,   122,   122,   122,   123,   123,
     124,   124,   125,   125,   125,   125,   125,   126,   127,   127,
     128,   129,   129,   130,   130,   131,   131,   132,   132,   132,
     132,   133,   133,   133,   134,   134,   135,   136,   136,   136,
     137,   137,   138,   138,   138,   138,   139,   139,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   141,   141,   141,
     142,   143,   144,   144,   144,   144,   144,   144,   144,   144,
     144,   145,   146,   146,   147,   147,   148,   148,   149,   150,
     150,   151,   151,   152,   153,   153,   154,   154,   154,   155,
     155,   156,   156,   156,   156,   157,   157,   158,   158,   159,
     159,   159,   159,   159,   159,   160,   161,   161,   162,   162,
     162,   162,   162,   162,   163,   163,   163,   163,   163,   163,
     163,   164,   164,   165,   165,   166,   166,   166,   166,   166,
     166,   167,   167,   168,   168,   169,   169,   169,   169,   169,
     169,   169,   169,   169,   169,   169,   170,   170,   170,   171,
     171,   172,   172,   173,   173,   174,   174,   175,   175,   176,
     176,   177,   178,   178,   178,   179,   179,   179
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
       1,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     5,     5,     5,     4,     2,     1,     4,     0,
       2,     0,     2,     5,     0,     3,     0,     2,     2,     4,
       1,     2,     2,     2,     2,     0,     2,     1,     3,     2,
       2,     2,     2,     1,     4,     3,     1,     3,     3,     4,
       5,     4,     5,     4,     1,     2,     1,     2,     2,     1,
       1,     4,     2,     3,     4,     0,     1,     5,     5,     3,
       6,     0,     3,     0,     2,     3,     1,     2,     3,     3,
       3,     5,     6,     5,     6,     4,     1,     1,     1,     0,
       1,     0,     3,     0,     2,     0,     3,     0,     2,     1,
       3,     3,     0,     1,     1,     0,     2,     2
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
#line 1667 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 140 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("stmtlist::stmtList::stmt");
				(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[-1].node));	
				bisonParseResult = (Node *) (yyval.list); 
			}
#line 1677 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 149 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::dmlStmt");
            (yyval.node) = (yyvsp[0].node);
        }
#line 1686 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 154 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::queryStmt");
            (yyval.node) = (yyvsp[0].node);
        }
#line 1695 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 159 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::transactionIdentifier");
            (yyval.node) = (Node *) createTransactionStmt((yyvsp[0].stringVal));
        }
#line 1704 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 164 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("stmt::withQuery"); 
			(yyval.node) = (yyvsp[0].node); 
		}
#line 1713 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 174 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::insertQuery"); }
#line 1719 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 175 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::deleteQuery"); }
#line 1725 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 176 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::updateQuery"); }
#line 1731 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 183 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::bracketedQuery"); (yyval.node) = (yyvsp[-1].node); }
#line 1737 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 184 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::selectQuery"); }
#line 1743 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 185 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::provStmt"); }
#line 1749 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 186 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::setOperatorQuery"); }
#line 1755 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 191 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withQuery::withViewList::queryStmt");
			(yyval.node) = (Node *) createWithStmt((yyvsp[-1].list), (yyvsp[0].node));
		}
#line 1764 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 199 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withViewList::list::view");
			(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
		}
#line 1773 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 204 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withViewList::view");
			(yyval.list) = singleton((yyvsp[0].node));
		}
#line 1782 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 212 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withView::ident::AS:queryStmt");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString((yyvsp[-4].stringVal)), (yyvsp[-1].node));
		}
#line 1791 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 219 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::BEGIN"); (yyval.stringVal) = strdup("TRANSACTION_BEGIN"); }
#line 1797 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 220 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::COMMIT"); (yyval.stringVal) = strdup("TRANSACTION_COMMIT"); }
#line 1803 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 221 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::ROLLBACK"); (yyval.stringVal) = strdup("TRANSACTION_ABORT"); }
#line 1809 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 1824 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 1838 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 1851 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 261 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalProvAsOf::EMPTY"); (yyval.node) = NULL; }
#line 1857 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 263 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvAsOf::SCN");
			(yyval.node) = (Node *) createConstLong((yyvsp[0].intVal));
		}
#line 1866 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 268 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvAsOf::TIMESTAMP");
			(yyval.node) = (Node *) createConstString((yyvsp[0].stringVal));
		}
#line 1875 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 275 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalProvWith::EMPTY"); (yyval.list) = NIL; }
#line 1881 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 277 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvWith::WITH");
			(yyval.list) = (yyvsp[0].list);
		}
#line 1890 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 284 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("provOptionList::option"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 1896 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 286 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("provOptionList::list"); 
			(yyval.list) = appendToTailOfList((yyvsp[-1].list),(yyvsp[0].node)); 
		}
#line 1905 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 294 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("provOption::TYPE"); 
			(yyval.node) = (Node *) createStringKeyValue("TYPE", (yyvsp[0].stringVal)); 
		}
#line 1914 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 299 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::TABLE");
			(yyval.node) = (Node *) createStringKeyValue("TABLE", (yyvsp[0].stringVal));
		}
#line 1923 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 304 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::ONLY::UPDATED");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_ONLY_UPDATED), 
					(Node *) createConstBool(TRUE));
		}
#line 1933 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 310 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::SHOW::INTERMEDIATE");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_SHOW_INTERMEDIATE), 
					(Node *) createConstBool(TRUE));
		}
#line 1943 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 316 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::TUPLE::VERSIONS");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString(PROP_PC_TUPLE_VERSIONS),
					(Node *) createConstBool(TRUE));
		}
#line 1953 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 328 "sql_parser.y" /* yacc.c:1646  */
    { 
             RULELOG("deleteQuery");
             (yyval.node) = (Node *) createDelete((yyvsp[-2].stringVal), (yyvsp[0].node));
         }
#line 1962 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 336 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::NULL"); (yyval.stringVal) = NULL; }
#line 1968 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 337 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::FROM"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 1974 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 352 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG(updateQuery); 
                (yyval.node) = (Node *) createUpdate((yyvsp[-3].stringVal), (yyvsp[-1].list), (yyvsp[0].node)); 
            }
#line 1983 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 360 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setExpression");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 1992 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 365 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setClause::setExpression");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2001 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 2014 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 2027 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 397 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertQuery::insertList"); 
            	(yyval.node) = (Node *) createInsert((yyvsp[-4].stringVal),(Node *) (yyvsp[-1].list), NULL); 
        	}
#line 2036 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 402 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("insertQuery::queryStmt");
                (yyval.node) = (Node *) createInsert((yyvsp[-1].stringVal), (yyvsp[0].node), NULL);
            }
#line 2045 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 410 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::constant");
            	(yyval.list) = singleton((yyvsp[0].node)); 
            }
#line 2054 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 415 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("insertList::IDENTIFIER");
                (yyval.list) = singleton(createAttributeReference((yyvsp[0].stringVal)));
            }
#line 2063 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 420 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("insertList::insertList::::IDENTIFIER");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), createAttributeReference((yyvsp[0].stringVal)));
            }
#line 2072 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 425 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::insertList::constant");
            	(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2081 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 439 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::INTERSECT");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 2090 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 444 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::MINUS");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 2099 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 449 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::UNION");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-2].stringVal), ((yyvsp[-1].stringVal) != NULL), (yyvsp[-3].node), (yyvsp[0].node));
            }
#line 2108 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 456 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::NULL"); (yyval.stringVal) = NULL; }
#line 2114 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 457 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::ALLTRUE"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2120 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 2140 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 489 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalDistinct::NULL"); (yyval.node) = NULL; }
#line 2146 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 491 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT");
                (yyval.node) = (Node *) createDistinctClause(NULL);
            }
#line 2155 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 496 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT::exprList");
                (yyval.node) = (Node *) createDistinctClause((yyvsp[-1].list));
            }
#line 2164 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 508 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectItem"); (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2172 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 512 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectClause::selectItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node)); 
            }
#line 2181 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 520 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression"); 
                 (yyval.node) = (Node *) createSelectItem(NULL, (yyvsp[0].node)); 
             }
#line 2190 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 525 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression::identifier"); 
                 (yyval.node) = (Node *) createSelectItem((yyvsp[0].stringVal), (yyvsp[-2].node));
             }
#line 2199 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 530 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
         		(yyval.node) = (Node *) createSelectItem(strdup("*"), NULL); 
     		}
#line 2208 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 535 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
     			(yyval.node) = (Node *) createSelectItem(CONCAT_STRINGS((yyvsp[-2].stringVal),".*"), NULL); 
 			}
#line 2217 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 545 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("exprList::SINGLETON"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 2223 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 547 "sql_parser.y" /* yacc.c:1646  */
    {
                  RULELOG("exprList::exprList::expression");
                  (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
             }
#line 2232 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 558 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::bracked"); (yyval.node) = (yyvsp[-1].node); }
#line 2238 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 559 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::constant"); }
#line 2244 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 560 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::attributeRef"); }
#line 2250 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 561 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlParameter"); }
#line 2256 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 562 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::binaryOperatorExpression"); }
#line 2262 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 563 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::unaryOperatorExpression"); }
#line 2268 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 564 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlFunctionCall"); }
#line 2274 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 565 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::case"); }
#line 2280 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 566 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::ROWNUM"); (yyval.node) = (Node *) makeNode(RowNumExpr); }
#line 2286 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 575 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::INT"); (yyval.node) = (Node *) createConstInt((yyvsp[0].intVal)); }
#line 2292 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 576 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::FLOAT"); (yyval.node) = (Node *) createConstFloat((yyvsp[0].floatVal)); }
#line 2298 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 577 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::STRING"); (yyval.node) = (Node *) createConstString((yyvsp[0].stringVal)); }
#line 2304 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 584 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("attributeRef::IDENTIFIER"); (yyval.node) = (Node *) createAttributeReference((yyvsp[0].stringVal)); }
#line 2310 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 590 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("sqlParameter::PARAMETER"); (yyval.node) = (Node *) createSQLParameter((yyvsp[0].stringVal)); }
#line 2316 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 610 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '+' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2327 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 617 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '-' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2338 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 624 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '*' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2349 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 631 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '/' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2360 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 638 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '%' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2371 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 645 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '^' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2382 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 654 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '&' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2393 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 661 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '|' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2404 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 670 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression::comparisonOps");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2415 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 680 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("unaryOperatorExpression:: '!' ");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2425 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 692 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("sqlFunctionCall::IDENTIFIER::exprList");
				FunctionCall *f = createFunctionCall((yyvsp[-4].stringVal), (yyvsp[-2].list));
				if ((yyvsp[0].node) != NULL)
					(yyval.node) = (Node *) createWindowFunction(f, (WindowDef *) (yyvsp[0].node));
				else  
                	(yyval.node) = (Node *) f; 
            }
#line 2438 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 701 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("sqlFunctionCall::AMMSC::exprList");
				FunctionCall *f = createFunctionCall((yyvsp[-4].stringVal), (yyvsp[-2].list));
				if ((yyvsp[0].node) != NULL)
					(yyval.node) = (Node *) createWindowFunction(f, (WindowDef *) (yyvsp[0].node));
				else  
                	(yyval.node) = (Node *) f; 
            }
#line 2451 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 716 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseExpression::CASE::expression::whens:else:END");
				(yyval.node) = (Node *) createCaseExpr((yyvsp[-3].node), (yyvsp[-2].list), (yyvsp[-1].node));
			}
#line 2460 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 721 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseExpression::CASE::whens::else::END");
				(yyval.node) = (Node *) createCaseExpr(NULL, (yyvsp[-2].list), (yyvsp[-1].node));
			}
#line 2469 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 729 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhenList::list::caseWhen");
				(yyval.list) = appendToTailOfList((yyvsp[-1].list), (yyvsp[0].node));
			}
#line 2478 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 734 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhenList::caseWhen");
				(yyval.list) = singleton((yyvsp[0].node));
			}
#line 2487 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 742 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhen::WHEN::expression::THEN::expression");
				(yyval.node) = (Node *) createCaseWhen((yyvsp[-2].node),(yyvsp[0].node));
			}
#line 2496 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 749 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalCaseElse::NULL"); (yyval.node) = NULL; }
#line 2502 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 751 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalCaseElse::ELSE::expression");
				(yyval.node) = (yyvsp[0].node);
			}
#line 2511 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 761 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("overclause::NULL"); (yyval.node) = NULL; }
#line 2517 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 763 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("overclause::window");
				(yyval.node) = (yyvsp[0].node);
			}
#line 2526 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 771 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("window");
				(yyval.node) = (Node *) createWindowDef((yyvsp[-3].list),(yyvsp[-2].list), (WindowFrame *) (yyvsp[-1].node));
			}
#line 2535 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 778 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optWindowPart::NULL"); (yyval.list) = NIL; }
#line 2541 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 780 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optWindowPart::PARTITION:BY::expressionList");
				(yyval.list) = (yyvsp[0].list);
			}
#line 2550 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 787 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optWindowFrame::NULL"); (yyval.node) = NULL; }
#line 2556 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 789 "sql_parser.y" /* yacc.c:1646  */
    { 
				WindowBound *l, *u = NULL;
				RULELOG("optWindowFrame::ROWS::windoBoundaries");
				l = getNthOfListP((yyvsp[0].list), 0);
				if(LIST_LENGTH((yyvsp[0].list)) > 1)
					u = getNthOfListP((yyvsp[0].list), 1);
				(yyval.node) = (Node *) createWindowFrame(WINFRAME_ROWS, l, u); 
			}
#line 2569 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 798 "sql_parser.y" /* yacc.c:1646  */
    {
				WindowBound *l, *u = NULL; 
				RULELOG("optWindowFrame::RANGE::windoBoundaries"); 
				l = getNthOfListP((yyvsp[0].list), 0);
				if(LIST_LENGTH((yyvsp[0].list)) > 1)
					u = getNthOfListP((yyvsp[0].list), 1);
				(yyval.node) = (Node *) createWindowFrame(WINFRAME_RANGE, l, u); 
			}
#line 2582 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 810 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBoundaries::BETWEEN"); 
				(yyval.list) = LIST_MAKE((yyvsp[-2].node), (yyvsp[0].node)); 
			}
#line 2591 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 815 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBoundaries::windowBound"); 
				(yyval.list) = singleton((yyvsp[0].node)); 
			}
#line 2600 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 823 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::UNBOUNDED::PRECEDING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_UNBOUND_PREC, NULL); 
			}
#line 2609 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 828 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::CURRENT::ROW"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_CURRENT_ROW, NULL); 
			}
#line 2618 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 833 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::expression::PRECEDING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_EXPR_PREC, (yyvsp[-1].node)); 
			}
#line 2627 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 838 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::expression::FOLLOWING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_EXPR_FOLLOW, (yyvsp[-1].node)); 
			}
#line 2636 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 850 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::NULL"); (yyval.list) = NULL; }
#line 2642 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 851 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::fromClause"); (yyval.list) = (yyvsp[0].list); }
#line 2648 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 856 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClauseItem");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2657 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 861 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClause::fromClauseItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2666 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 870 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
				FromItem *f = createFromTableRef(NULL, NIL, (yyvsp[-1].stringVal));
				f->provInfo = (FromProvInfo *) (yyvsp[0].node);
                (yyval.node) = (Node *) f;
            }
#line 2677 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 877 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
                FromItem *f = createFromTableRef(((FromItem *) (yyvsp[0].node))->name, 
						((FromItem *) (yyvsp[0].node))->attrNames, (yyvsp[-1].stringVal));
				f->provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
                (yyval.node) = (Node *) f;
            }
#line 2689 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 886 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromItem *f = (FromItem *) (yyvsp[-1].node);
                f->provInfo = (FromProvInfo *) (yyvsp[0].node);
                (yyval.node) = (yyvsp[-1].node);
            }
#line 2700 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 893 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromSubquery *s = (FromSubquery *) (yyvsp[-1].node);
                s->from.name = ((FromItem *) (yyvsp[0].node))->name;
                s->from.attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
                s->from.provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
                (yyval.node) = (Node *) s;
            }
#line 2713 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 902 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[0].node);
        		f->name = NULL;
        		(yyval.node) = (Node *) f;
        	}
#line 2725 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 910 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[-2].node);
        		f->name = ((FromItem *) (yyvsp[0].node))->name;
                f->attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
                f->provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
        		(yyval.node) = (Node *) f;
        	}
#line 2739 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 923 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("subQuery::queryStmt");
                (yyval.node) = (Node *) createFromSubquery(NULL, NULL, (yyvsp[-1].node));
            }
#line 2748 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 930 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = singleton((yyvsp[0].stringVal)); }
#line 2754 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 931 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].stringVal)); }
#line 2760 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 935 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2766 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 937 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURAL");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
						(FromItem *) (yyvsp[0].node), "JOIN_INNER", "JOIN_COND_NATURAL", 
						NULL);
          	}
#line 2777 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 944 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURALjoinType");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[0].node), (yyvsp[-2].stringVal), "JOIN_COND_NATURAL", NULL);
          	}
#line 2787 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 950 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::CROSS JOIN");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[0].node), "JOIN_CROSS", "JOIN_COND_ON", NULL);
          	}
#line 2797 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 956 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinType::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON";
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[-1].node), (yyvsp[-3].stringVal), condType, (yyvsp[0].node));
          	}
#line 2809 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 964 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON"; 
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[-1].node), "JOIN_INNER", 
                		condType, (yyvsp[0].node));
          	}
#line 2822 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 975 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2828 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 976 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT OUTER"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2834 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 977 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT "); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2840 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 978 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT OUTER"); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2846 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 979 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL OUTER"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2852 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 980 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2858 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 981 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::INNER"); (yyval.stringVal) = "JOIN_INNER"; }
#line 2864 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 985 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (Node *) (yyvsp[-1].list); }
#line 2870 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 986 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2876 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 991 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[-2].node);
				(yyval.node) = (Node *) f;
			}
#line 2887 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 998 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[-3].node); 
				(yyval.node) = (Node *) f;
			}
#line 2898 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1007 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFromProv::empty"); (yyval.node) = NULL; }
#line 2904 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1009 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::BASERELATION");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = TRUE;
				p->userProvAttrs = NIL;				 
				(yyval.node) = (Node *) p; 
			}
#line 2916 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1017 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::userProvAttr");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = FALSE;
				p->userProvAttrs = (yyvsp[-1].list);				 
				(yyval.node) = (Node *) p; 
			}
#line 2928 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1025 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::userProvDupAttr");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = TRUE;
				p->userProvAttrs = (yyvsp[-1].list);
				(yyval.node) = (Node *) p;
			}
#line 2940 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1033 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::intermediateProv");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->intermediateProv = TRUE;
				(yyval.node) = (Node *) p;
			}
#line 2951 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1040 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::intermediateProv::attrList");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->intermediateProv = TRUE;
				p->userProvAttrs = (yyvsp[-1].list);
				(yyval.node) = (Node *) p;
			}
#line 2963 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1050 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAttrAlias::empty"); (yyval.list) = NULL; }
#line 2969 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1052 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAttrAlias::identifierList"); (yyval.list) = (yyvsp[-1].list); 
			}
#line 2977 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1061 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::NULL"); (yyval.node) = NULL; }
#line 2983 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1062 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::whereExpression"); (yyval.node) = (yyvsp[0].node); }
#line 2989 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1066 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("where::brackedWhereExpression"); (yyval.node) = (yyvsp[-1].node); }
#line 2995 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 1067 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("whereExpression::expression"); (yyval.node) = (yyvsp[0].node); }
#line 3001 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 1069 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::NOT");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 3011 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1075 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 3022 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1082 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::OR");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 3033 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 1089 "sql_parser.y" /* yacc.c:1646  */
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
#line 3055 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1107 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::BETWEEN-AND");
                List *expr = singleton((yyvsp[-4].node));
                expr = appendToTailOfList(expr, (yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr);
            }
#line 3067 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1115 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::comparisonOps::nestedSubQueryOperator::Subquery");
                (yyval.node) = (Node *) createNestedSubquery((yyvsp[-3].stringVal), (yyvsp[-5].node), (yyvsp[-4].stringVal), (yyvsp[-1].node));
            }
#line 3076 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1120 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::Subquery");
                Node *q = (Node *) createNestedSubquery("SCALAR", NULL, NULL, (yyvsp[-1].node)); 
                List *expr = LIST_MAKE((yyvsp[-4].node), q);
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr); 
            }
#line 3087 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 1127 "sql_parser.y" /* yacc.c:1646  */
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
#line 3104 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 1140 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::EXISTS");
                (yyval.node) = (Node *) createNestedSubquery((yyvsp[-3].stringVal), NULL, NULL, (yyvsp[-1].node));
            }
#line 3113 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 1147 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ANY"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 3119 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1148 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ALL"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 3125 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 1149 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::SOME"); (yyval.stringVal) = "ANY"; }
#line 3131 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 1153 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NULL"); (yyval.stringVal) = NULL; }
#line 3137 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 1154 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NOT"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 3143 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 1158 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::NULL"); (yyval.list) = NULL; }
#line 3149 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 1159 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::GROUPBY"); (yyval.list) = (yyvsp[0].list); }
#line 3155 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 1163 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.node) = NULL; }
#line 3161 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 1165 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("optionalHaving::HAVING"); 
                (yyval.node) = (Node *) (yyvsp[0].node);
            }
#line 3170 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 1172 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.list) = NULL; }
#line 3176 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 1173 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy::ORDERBY"); (yyval.list) = (yyvsp[0].list); }
#line 3182 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 1177 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::NULL"); (yyval.node) = NULL; }
#line 3188 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 1178 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::CONSTANT"); (yyval.node) = (yyvsp[0].node);}
#line 3194 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 1183 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::orderExpr");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 3203 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 1188 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("orderList::orderList::orderExpr");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 3212 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 1196 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("orderExpr::expr::sortOrder::nullOrder");
				SortOrder o = (strcmp((yyvsp[-1].stringVal),"ASC") == 0) ?  SORT_ASC : SORT_DESC;
				SortNullOrder n = (strcmp((yyvsp[0].stringVal),"NULLS_FIRST") == 0) ? 
						SORT_NULLS_FIRST : 
						SORT_NULLS_LAST;
				(yyval.node) = (Node *) createOrderExpr((yyvsp[-2].node), o, n);
			}
#line 3225 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 1207 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalSortOrder::empty"); (yyval.stringVal) = "ASC"; }
#line 3231 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 1209 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalSortOrder::ASC");
				(yyval.stringVal) = "ASC";
			}
#line 3240 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 1214 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalSortOrder::DESC");
				(yyval.stringVal) = "DESC";
			}
#line 3249 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 1221 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNullOrder::empty"); (yyval.stringVal) = "NULLS_LAST"; }
#line 3255 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 186:
#line 1223 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalNullOrder::NULLS FIRST");
				(yyval.stringVal) = "NULLS_FIRST";
			}
#line 3264 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 187:
#line 1228 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalNullOrder::NULLS LAST");
				(yyval.stringVal) = "NULLS_LAST";
			}
#line 3273 "sql_parser.tab.c" /* yacc.c:1646  */
    break;


#line 3277 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 1235 "sql_parser.y" /* yacc.c:1906  */




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

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
    FROM = 272,
    AS = 273,
    WHERE = 274,
    DISTINCT = 275,
    STARALL = 276,
    AND = 277,
    OR = 278,
    LIKE = 279,
    NOT = 280,
    IN = 281,
    ISNULL = 282,
    BETWEEN = 283,
    EXCEPT = 284,
    EXISTS = 285,
    AMMSC = 286,
    NULLVAL = 287,
    ALL = 288,
    ANY = 289,
    IS = 290,
    SOME = 291,
    UNION = 292,
    INTERSECT = 293,
    MINUS = 294,
    INTO = 295,
    VALUES = 296,
    HAVING = 297,
    GROUP = 298,
    ORDER = 299,
    BY = 300,
    LIMIT = 301,
    SET = 302,
    INT = 303,
    BEGIN_TRANS = 304,
    COMMIT_TRANS = 305,
    ROLLBACK_TRANS = 306,
    DUMMYEXPR = 307,
    JOIN = 308,
    NATURAL = 309,
    LEFT = 310,
    RIGHT = 311,
    OUTER = 312,
    INNER = 313,
    CROSS = 314,
    ON = 315,
    USING = 316,
    FULL = 317,
    XOR = 318
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

#line 202 "sql_parser.tab.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SQL_PARSER_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 217 "sql_parser.tab.c" /* yacc.c:358  */

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
#define YYLAST   425

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  79
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  48
/* YYNRULES -- Number of rules.  */
#define YYNRULES  138
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  266

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   318

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    15,     2,     2,     2,    11,    13,     2,
      18,    17,     9,     7,    77,     8,    78,    10,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    76,
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
      67,    68,    69,    70,    71,    72,    73,    74,    75
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   124,   124,   130,   139,   144,   149,   160,   161,   162,
     169,   170,   171,   172,   176,   177,   178,   185,   195,   207,
     208,   213,   223,   232,   233,   247,   255,   260,   268,   277,
     292,   297,   305,   310,   315,   320,   334,   339,   344,   352,
     353,   362,   385,   386,   391,   403,   407,   415,   420,   425,
     430,   441,   442,   454,   455,   456,   457,   458,   459,   468,
     469,   470,   477,   496,   503,   510,   517,   524,   531,   540,
     547,   556,   566,   578,   591,   592,   596,   601,   610,   617,
     626,   633,   642,   650,   663,   671,   672,   676,   677,   684,
     690,   696,   704,   716,   717,   718,   719,   720,   721,   722,
     726,   727,   731,   738,   748,   749,   757,   768,   769,   779,
     780,   784,   785,   786,   792,   799,   806,   813,   821,   826,
     833,   846,   862,   863,   864,   868,   869,   873,   874,   878,
     879,   889,   890,   894,   895,   899,   904,   909,   914
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "intConst", "floatConst", "stringConst",
  "identifier", "'+'", "'-'", "'*'", "'/'", "'%'", "'^'", "'&'", "'|'",
  "'!'", "comparisonOps", "')'", "'('", "'='", "SELECT", "INSERT",
  "UPDATE", "DELETE", "PROVENANCE", "OF", "BASERELATION", "SCN",
  "TIMESTAMP", "FROM", "AS", "WHERE", "DISTINCT", "STARALL", "AND", "OR",
  "LIKE", "NOT", "IN", "ISNULL", "BETWEEN", "EXCEPT", "EXISTS", "AMMSC",
  "NULLVAL", "ALL", "ANY", "IS", "SOME", "UNION", "INTERSECT", "MINUS",
  "INTO", "VALUES", "HAVING", "GROUP", "ORDER", "BY", "LIMIT", "SET",
  "INT", "BEGIN_TRANS", "COMMIT_TRANS", "ROLLBACK_TRANS", "DUMMYEXPR",
  "JOIN", "NATURAL", "LEFT", "RIGHT", "OUTER", "INNER", "CROSS", "ON",
  "USING", "FULL", "XOR", "';'", "','", "'.'", "$accept", "stmtList",
  "stmt", "dmlStmt", "queryStmt", "transactionIdentifier", "provStmt",
  "optionalProvAsOf", "deleteQuery", "fromString", "updateQuery",
  "setClause", "setExpression", "insertQuery", "insertList",
  "setOperatorQuery", "optionalAll", "selectQuery", "optionalDistinct",
  "selectClause", "selectItem", "exprList", "expression", "constant",
  "attributeRef", "binaryOperatorExpression", "unaryOperatorExpression",
  "sqlFunctionCall", "optionalFrom", "fromClause", "fromClauseItem",
  "subQuery", "identifierList", "fromJoinItem", "joinType", "joinCond",
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
       0,   256,   257,   258,   259,   260,   261,    43,    45,    42,
      47,    37,    94,    38,   124,    33,   262,    41,    40,    61,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,    59,    44,    46
};
# endif

#define YYPACT_NINF -195

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-195)))

#define YYTABLE_NINF -126

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-126)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      88,   208,    16,   -20,    49,    29,    34,  -195,  -195,  -195,
      81,    24,  -195,   259,  -195,  -195,  -195,  -195,  -195,  -195,
    -195,    -9,    44,   260,   112,    87,  -195,   147,   129,   143,
    -195,   103,  -195,   148,   208,   208,  -195,   170,  -195,  -195,
    -195,     0,  -195,   309,   309,   -23,  -195,   287,  -195,  -195,
    -195,  -195,  -195,   102,   188,   166,   110,   185,  -195,  -195,
     208,  -195,  -195,   309,   309,   200,   198,   368,   337,   139,
     260,   190,   309,   309,   309,   309,   309,   309,   309,   309,
     309,   216,   209,   259,  -195,   -24,  -195,   213,     9,   228,
     244,    88,  -195,     3,   348,     6,  -195,  -195,    -5,   256,
     156,   320,    -5,  -195,  -195,     9,   205,   151,   151,   255,
     255,   255,  -195,   368,   326,   396,  -195,   180,   188,  -195,
     325,     9,     9,   250,   245,   165,  -195,  -195,   149,    18,
    -195,   309,  -195,   252,  -195,  -195,    20,   256,    -6,   320,
     262,   139,   139,   328,   204,   212,  -195,   227,   249,   237,
    -195,    20,   165,   220,   266,  -195,     8,  -195,  -195,   301,
     348,  -195,   234,   179,   165,   208,   172,  -195,   284,     9,
       9,     9,   309,  -195,  -195,   348,   317,   308,   335,    11,
    -195,    91,   320,   300,   139,   267,  -195,  -195,   139,  -195,
     139,   233,   346,   307,  -195,   406,   301,  -195,    78,   301,
    -195,  -195,  -195,   351,   371,    56,    56,    95,   277,  -195,
      12,   317,  -195,   308,    90,  -195,    20,     9,   374,  -195,
    -195,   139,  -195,   300,  -195,  -195,   322,   198,   381,   343,
     355,  -195,  -195,  -195,   131,   208,   208,   309,  -195,   395,
      13,  -195,   165,   317,  -195,  -195,   233,   309,   233,   378,
    -195,  -195,   157,   174,   348,  -195,  -195,    14,  -195,  -195,
     348,   322,  -195,  -195,  -195,  -195
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    42,     0,     0,    23,    19,    14,    15,    16,
       0,     0,     4,     5,     6,    12,     8,     9,     7,    13,
      11,     0,    43,     0,     0,     0,    24,     0,     0,     0,
       1,     0,     2,    39,     0,     0,    10,     0,    59,    60,
      61,    62,    49,     0,     0,    74,    45,    47,    54,    55,
      56,    57,    58,     0,     0,     0,     0,     0,     3,    40,
       0,    36,    37,     0,     0,     0,    62,    72,     0,     0,
       0,   109,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    31,    62,   109,    26,     0,     0,     0,
       0,     0,    38,     0,    51,     0,    50,    53,   104,     0,
      75,    76,   104,    82,    46,     0,   127,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    48,     0,     0,    25,
       0,     0,     0,     0,   112,    22,    20,    21,     0,     0,
      44,     0,    73,     0,   105,    79,    78,     0,     0,     0,
      82,     0,     0,     0,    93,    95,    99,     0,    98,     0,
      81,    80,   110,     0,   129,    33,     0,    32,    27,     0,
      28,    29,   112,     0,   113,     0,     0,   126,     0,     0,
       0,     0,     0,    18,    17,    52,     0,   107,     0,     0,
      84,    87,    77,     0,     0,     0,    94,    96,     0,    97,
       0,     0,     0,   131,    30,     0,     0,   111,     0,     0,
     123,   122,   124,     0,     0,   114,   115,   116,     0,    85,
       0,     0,   102,   107,    84,    83,     0,     0,     0,    92,
      88,     0,    90,     0,   137,   135,   128,     0,     0,     0,
     133,    34,    35,   121,     0,     0,     0,     0,   106,     0,
       0,   103,   101,     0,    89,    91,     0,     0,     0,     0,
      41,   119,     0,     0,   117,    86,   108,     0,   138,   136,
     130,   132,   134,   118,   120,   100
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -195,   323,     2,  -195,    -1,  -195,  -195,  -195,  -195,  -195,
    -195,  -195,   297,  -195,  -195,  -195,  -195,  -195,  -195,  -195,
     347,   352,    -7,  -116,   -44,  -195,  -195,   226,  -195,  -195,
     -65,   299,  -194,   -90,   278,   197,   -99,   -97,   210,   339,
     -83,  -195,  -195,  -195,  -195,  -195,  -195,   177
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    10,    31,    12,    13,    14,    15,    29,    16,    27,
      17,    85,    86,    18,   156,    19,    60,    20,    23,    45,
      46,    93,   124,    48,    49,    50,    51,    52,    71,   100,
     139,   102,   210,   103,   149,   219,   135,   136,   212,   106,
     125,   203,   168,   154,   193,   230,   250,   226
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      21,   157,    11,   150,   101,   151,    69,   105,    36,   140,
      87,   180,    38,    39,    40,    66,    47,   240,    64,   133,
     130,   134,   152,   132,    43,   194,   177,   121,   214,   238,
     256,   265,    24,    61,    62,   174,    67,    68,   163,   164,
      33,    34,    35,    33,    34,    35,   122,   140,    22,   257,
     178,   123,    83,   118,    70,    25,    94,    94,    26,    92,
      33,    34,    35,    47,    28,   107,   108,   109,   110,   111,
     112,   113,   114,   115,    87,   224,   182,   183,    65,   232,
     131,    30,   215,   131,   216,   195,   205,   206,   207,   239,
     239,   239,   171,   129,    32,   233,   172,  -104,   138,     1,
      32,     2,     3,     4,     5,     6,     1,   -10,     2,     3,
       4,     5,     6,   160,   162,   133,    37,   134,    53,   220,
       1,  -104,     2,   222,   175,   223,     6,    33,    34,    35,
     258,  -126,   224,   262,   242,  -126,   179,    89,    90,   -10,
     -10,   -10,     7,     8,     9,    98,    54,   225,   251,     7,
       8,     9,    68,    55,    56,    82,   244,    99,   138,   115,
      74,    75,    76,    77,   198,   208,   173,     1,    57,     2,
       3,     4,     5,     6,   263,    38,    39,    40,    66,    58,
      33,    34,    35,    38,    39,    40,   155,    43,    63,    68,
     199,   264,    68,    59,    84,    21,   197,    88,   234,   169,
     170,   171,   259,    91,   225,   172,    33,    34,    35,    96,
       7,     8,     9,   169,   170,   171,    64,   200,   201,   172,
     202,   105,   116,    33,    34,    35,     1,   117,     2,   120,
     254,   126,     6,   141,   252,   253,    38,    39,    40,    84,
     260,    72,    73,    74,    75,    76,    77,    78,    79,   127,
     166,    97,    72,    73,    74,    75,    76,    77,    78,    79,
     153,   166,    98,    38,    39,    40,    41,    77,   165,    42,
     176,   167,  -125,   186,   137,    43,     2,   191,    44,   181,
       6,   187,   167,  -125,    72,    73,    74,    75,    76,    77,
      78,    79,   188,    80,    72,    73,    74,    75,    76,    77,
      78,    79,   190,    80,    38,    39,    40,    66,    33,    34,
      35,   237,    38,    39,    40,    66,    43,    81,   189,   196,
     192,     2,   204,   209,    43,     6,   211,    44,    38,    39,
      40,    66,   221,    72,    73,    74,    75,    76,    77,    78,
      43,   213,    80,   159,    72,    73,    74,    75,    76,    77,
      78,    79,   227,    80,    97,    72,    73,    74,    75,    76,
      77,    78,    79,   229,    80,   142,   143,   144,   145,   235,
     146,   147,   217,   218,   148,    72,    73,    74,    75,    76,
      77,    38,    39,    40,    80,   142,   143,   144,   145,   236,
     146,   147,   243,   184,   148,   144,   145,   247,   146,   246,
     248,   255,   148,    72,    73,    74,    75,    76,    77,    38,
      39,    40,   231,   249,   128,   158,    95,   104,   228,   161,
     245,   185,     0,   241,   119,   261
};

static const yytype_int16 yycheck[] =
{
       1,   117,     0,   102,    69,   102,    29,    31,    17,    99,
      54,    17,     3,     4,     5,     6,    23,   211,    18,    24,
      17,    26,   105,    17,    15,    17,     6,    18,    17,    17,
      17,    17,    52,    34,    35,    17,    43,    44,   121,   122,
      49,    50,    51,    49,    50,    51,    37,   137,    32,   243,
      30,    42,    53,    77,    77,     6,    63,    64,    29,    60,
      49,    50,    51,    70,    30,    72,    73,    74,    75,    76,
      77,    78,    79,    80,   118,   191,   141,   142,    78,   195,
      77,     0,   181,    77,   181,    77,   169,   170,   171,    77,
      77,    77,    36,    91,    76,    17,    40,     6,    99,    18,
      76,    20,    21,    22,    23,    24,    18,    17,    20,    21,
      22,    23,    24,   120,   121,    24,    72,    26,     6,   184,
      18,    30,    20,   188,   131,   190,    24,    49,    50,    51,
     246,    36,   248,   249,   217,    40,   137,    27,    28,    49,
      50,    51,    61,    62,    63,     6,    59,   191,    17,    61,
      62,    63,   159,     6,    25,    53,   221,    18,   159,   166,
       9,    10,    11,    12,   165,   172,    17,    18,    25,    20,
      21,    22,    23,    24,    17,     3,     4,     5,     6,    76,
      49,    50,    51,     3,     4,     5,     6,    15,    18,   196,
      18,    17,   199,    45,     6,   196,    17,    31,   199,    34,
      35,    36,   246,    18,   248,    40,    49,    50,    51,     9,
      61,    62,    63,    34,    35,    36,    18,    45,    46,    40,
      48,    31,     6,    49,    50,    51,    18,    18,    20,    16,
     237,     3,    24,    77,   235,   236,     3,     4,     5,     6,
     247,     7,     8,     9,    10,    11,    12,    13,    14,     5,
      16,    17,     7,     8,     9,    10,    11,    12,    13,    14,
      55,    16,     6,     3,     4,     5,     6,    12,    18,     9,
      18,    37,    38,    69,    18,    15,    20,    57,    18,    17,
      24,    69,    37,    38,     7,     8,     9,    10,    11,    12,
      13,    14,    65,    16,     7,     8,     9,    10,    11,    12,
      13,    14,    65,    16,     3,     4,     5,     6,    49,    50,
      51,    34,     3,     4,     5,     6,    15,    30,    69,    18,
      54,    20,    38,     6,    15,    24,    18,    18,     3,     4,
       5,     6,    65,     7,     8,     9,    10,    11,    12,    13,
      15,     6,    16,    18,     7,     8,     9,    10,    11,    12,
      13,    14,     6,    16,    17,     7,     8,     9,    10,    11,
      12,    13,    14,    56,    16,    65,    66,    67,    68,    18,
      70,    71,    72,    73,    74,     7,     8,     9,    10,    11,
      12,     3,     4,     5,    16,    65,    66,    67,    68,    18,
      70,    71,    18,    65,    74,    67,    68,    16,    70,    77,
      57,     6,    74,     7,     8,     9,    10,    11,    12,     3,
       4,     5,     6,    58,    91,   118,    64,    70,   192,   120,
     223,   143,    -1,   213,    85,   248
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    18,    20,    21,    22,    23,    24,    61,    62,    63,
      80,    81,    82,    83,    84,    85,    87,    89,    92,    94,
      96,    83,    32,    97,    52,     6,    29,    88,    30,    86,
       0,    81,    76,    49,    50,    51,    17,    72,     3,     4,
       5,     6,     9,    15,    18,    98,    99,   101,   102,   103,
     104,   105,   106,     6,    59,     6,    25,    25,    76,    45,
      95,    83,    83,    18,    18,    78,     6,   101,   101,    29,
      77,   107,     7,     8,     9,    10,    11,    12,    13,    14,
      16,    30,    53,    83,     6,    90,    91,   103,    31,    27,
      28,    18,    83,   100,   101,   100,     9,    17,     6,    18,
     108,   109,   110,   112,    99,    31,   118,   101,   101,   101,
     101,   101,   101,   101,   101,   101,     6,    18,    77,   118,
      16,    18,    37,    42,   101,   119,     3,     5,    80,    81,
      17,    77,    17,    24,    26,   115,   116,    18,    83,   109,
     112,    77,    65,    66,    67,    68,    70,    71,    74,   113,
     115,   116,   119,    55,   122,     6,    93,   102,    91,    18,
     101,   110,   101,   119,   119,    18,    16,    37,   121,    34,
      35,    36,    40,    17,    17,   101,    18,     6,    30,    83,
      17,    17,   109,   109,    65,   113,    69,    69,    65,    69,
      65,    57,    54,   123,    17,    77,    18,    17,    83,    18,
      45,    46,    48,   120,    38,   119,   119,   119,   101,     6,
     111,    18,   117,     6,    17,   115,   116,    72,    73,   114,
     109,    65,   109,   109,   102,   103,   126,     6,   106,    56,
     124,     6,   102,    17,    83,    18,    18,    34,    17,    77,
     111,   117,   119,    18,   109,   114,    77,    16,    57,    58,
     125,    17,    83,    83,   101,     6,    17,   111,   102,   103,
     101,   126,   102,    17,    17,    17
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    79,    80,    80,    81,    81,    81,    82,    82,    82,
      83,    83,    83,    83,    84,    84,    84,    85,    85,    86,
      86,    86,    87,    88,    88,    89,    90,    90,    91,    91,
      92,    92,    93,    93,    93,    93,    94,    94,    94,    95,
      95,    96,    97,    97,    97,    98,    98,    99,    99,    99,
      99,   100,   100,   101,   101,   101,   101,   101,   101,   102,
     102,   102,   103,   104,   104,   104,   104,   104,   104,   104,
     104,   104,   105,   106,   107,   107,   108,   108,   109,   109,
     109,   109,   109,   109,   110,   111,   111,   112,   112,   112,
     112,   112,   112,   113,   113,   113,   113,   113,   113,   113,
     114,   114,   115,   115,   116,   116,   116,   117,   117,   118,
     118,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   120,   120,   120,   121,   121,   122,   122,   123,
     123,   124,   124,   125,   125,   126,   126,   126,   126
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     3,     1,     1,     1,     1,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     6,     6,     0,
       4,     4,     5,     0,     1,     5,     1,     3,     3,     3,
       7,     4,     1,     1,     3,     3,     3,     3,     4,     0,
       1,     9,     0,     1,     5,     1,     3,     1,     3,     1,
       3,     1,     3,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     4,     0,     2,     1,     3,     2,     2,
       2,     2,     1,     4,     3,     1,     3,     3,     4,     5,
       4,     5,     4,     1,     2,     1,     2,     2,     1,     1,
       4,     2,     3,     4,     0,     1,     4,     0,     3,     0,
       2,     3,     1,     2,     3,     3,     3,     5,     6,     5,
       6,     4,     1,     1,     1,     0,     1,     0,     3,     0,
       4,     0,     3,     0,     2,     1,     3,     1,     3
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
#line 125 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("stmtList::stmt"); 
				(yyval.list) = singleton((yyvsp[-1].node));
				bisonParseResult = (Node *) (yyval.list);	 
			}
#line 1529 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 131 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("stmtlist::stmtList::stmt");
				(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[-1].node));	
				bisonParseResult = (Node *) (yyval.list); 
			}
#line 1539 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 140 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::dmlStmt");
            (yyval.node) = (yyvsp[0].node);
        }
#line 1548 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 145 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::queryStmt");
            (yyval.node) = (yyvsp[0].node);
        }
#line 1557 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 150 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::transactionIdentifier");
            (yyval.node) = (Node *) createTransactionStmt((yyvsp[0].stringVal));
        }
#line 1566 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 160 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::insertQuery"); }
#line 1572 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 161 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::deleteQuery"); }
#line 1578 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 162 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::updateQuery"); }
#line 1584 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 169 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::bracketedQuery"); (yyval.node) = (yyvsp[-1].node); }
#line 1590 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 170 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::selectQuery"); }
#line 1596 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 171 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::provStmt"); }
#line 1602 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 172 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::setOperatorQuery"); }
#line 1608 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 176 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::BEGIN"); (yyval.stringVal) = strdup("TRANSACTION_BEGIN"); }
#line 1614 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 177 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::COMMIT"); (yyval.stringVal) = strdup("TRANSACTION_COMMIT"); }
#line 1620 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 178 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::ROLLBACK"); (yyval.stringVal) = strdup("TRANSACTION_ABORT"); }
#line 1626 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 186 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("provStmt::stmt");
            Node *stmt = (yyvsp[-1].node);
	    	ProvenanceStmt *p = createProvenanceStmt(stmt);
		    p->inputType = isQBUpdate(stmt) ? PROV_INPUT_UPDATE : PROV_INPUT_QUERY;
		    p->provType = PROV_PI_CS;
		    p->asOf = (Node *) (yyvsp[-4].node);
            (yyval.node) = (Node *) p;
        }
#line 1640 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 196 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provStmt::stmtlist");
			ProvenanceStmt *p = createProvenanceStmt((Node *) (yyvsp[-1].list));
			p->inputType = PROV_INPUT_UPDATE_SEQUENCE;
			p->provType = PROV_PI_CS;
			p->asOf = (Node *) (yyvsp[-4].node);
			(yyval.node) = (Node *) p;
		}
#line 1653 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 207 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalProvAsOf::EMPTY"); (yyval.node) = NULL; }
#line 1659 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 209 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvAsOf::SCN");
			(yyval.node) = (Node *) createConstInt((yyvsp[0].intVal));
		}
#line 1668 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 214 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvAsOf::TIMESTAMP");
			(yyval.node) = (Node *) createConstString((yyvsp[0].stringVal));
		}
#line 1677 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 224 "sql_parser.y" /* yacc.c:1646  */
    { 
             RULELOG("deleteQuery");
             (yyval.node) = (Node *) createDelete((yyvsp[-2].stringVal), (yyvsp[0].node));
         }
#line 1686 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 232 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::NULL"); (yyval.stringVal) = NULL; }
#line 1692 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 233 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::FROM"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 1698 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 248 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG(updateQuery); 
                (yyval.node) = (Node *) createUpdate((yyvsp[-3].stringVal), (yyvsp[-1].list), (yyvsp[0].node)); 
            }
#line 1707 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 256 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setExpression");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 1716 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 261 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setClause::setExpression");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 1725 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 269 "sql_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal),"=")) {
                    RULELOG("setExpression::attributeRef::expression");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 1738 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 278 "sql_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal), "=")) {
                    RULELOG("setExpression::attributeRef::queryStmt");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 1751 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 293 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertQuery::insertList"); 
            	(yyval.node) = (Node *) createInsert((yyvsp[-4].stringVal),(Node *) (yyvsp[-1].list), NULL); 
        	}
#line 1760 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 298 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("insertQuery::queryStmt");
                (yyval.node) = (Node *) createInsert((yyvsp[-1].stringVal), (yyvsp[0].node), NULL);
            }
#line 1769 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 306 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::constant");
            	(yyval.list) = singleton((yyvsp[0].node)); 
            }
#line 1778 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 311 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("insertList::IDENTIFIER");
                (yyval.list) = singleton(createAttributeReference((yyvsp[0].stringVal)));
            }
#line 1787 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 316 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("insertList::insertList::::IDENTIFIER");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), createAttributeReference((yyvsp[0].stringVal)));
            }
#line 1796 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 321 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::insertList::constant");
            	(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 1805 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 335 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::INTERSECT");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 1814 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 340 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::MINUS");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 1823 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 345 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::UNION");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-2].stringVal), ((yyvsp[-1].stringVal) != NULL), (yyvsp[-3].node), (yyvsp[0].node));
            }
#line 1832 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 352 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::NULL"); (yyval.stringVal) = NULL; }
#line 1838 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 353 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::ALLTRUE"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 1844 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 363 "sql_parser.y" /* yacc.c:1646  */
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
#line 1864 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 385 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalDistinct::NULL"); (yyval.node) = NULL; }
#line 1870 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 387 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT");
                (yyval.node) = (Node *) createDistinctClause(NULL);
            }
#line 1879 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 392 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT::exprList");
                (yyval.node) = (Node *) createDistinctClause((yyvsp[-1].list));
            }
#line 1888 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 404 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectItem"); (yyval.list) = singleton((yyvsp[0].node));
            }
#line 1896 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 408 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectClause::selectItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node)); 
            }
#line 1905 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 416 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression"); 
                 (yyval.node) = (Node *) createSelectItem(NULL, (yyvsp[0].node)); 
             }
#line 1914 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 421 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression::identifier"); 
                 (yyval.node) = (Node *) createSelectItem((yyvsp[0].stringVal), (yyvsp[-2].node));
             }
#line 1923 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 426 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
         		(yyval.node) = (Node *) createSelectItem(strdup("*"), NULL); 
     		}
#line 1932 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 431 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
     			(yyval.node) = (Node *) createSelectItem(CONCAT_STRINGS((yyvsp[-2].stringVal),".*"), NULL); 
 			}
#line 1941 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 441 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("exprList::SINGLETON"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 1947 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 443 "sql_parser.y" /* yacc.c:1646  */
    {
                  RULELOG("exprList::exprList::expression");
                  (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
             }
#line 1956 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 454 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::bracked"); (yyval.node) = (yyvsp[-1].node); }
#line 1962 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 455 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::constant"); }
#line 1968 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 456 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::attributeRef"); }
#line 1974 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 457 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::binaryOperatorExpression"); }
#line 1980 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 458 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::unaryOperatorExpression"); }
#line 1986 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 459 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlFunctionCall"); }
#line 1992 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 468 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::INT"); (yyval.node) = (Node *) createConstInt((yyvsp[0].intVal)); }
#line 1998 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 469 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::FLOAT"); (yyval.node) = (Node *) createConstFloat((yyvsp[0].floatVal)); }
#line 2004 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 470 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::STRING"); (yyval.node) = (Node *) createConstString((yyvsp[0].stringVal)); }
#line 2010 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 477 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("attributeRef::IDENTIFIER"); (yyval.node) = (Node *) createAttributeReference((yyvsp[0].stringVal)); }
#line 2016 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 497 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '+' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2027 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 504 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '-' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2038 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 511 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '*' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2049 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 518 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '/' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2060 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 525 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '%' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2071 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 532 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '^' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2082 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 541 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '&' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2093 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 548 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '|' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2104 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 557 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression::comparisonOps");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2115 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 567 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("unaryOperatorExpression:: '!' ");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2125 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 579 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("sqlFunctionCall::IDENTIFIER::exprList"); 
                (yyval.node) = (Node *) createFunctionCall((yyvsp[-3].stringVal), (yyvsp[-1].list)); 
            }
#line 2134 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 591 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::NULL"); (yyval.list) = NULL; }
#line 2140 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 592 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::fromClause"); (yyval.list) = (yyvsp[0].list); }
#line 2146 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 597 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClauseItem");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2155 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 602 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClause::fromClauseItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2164 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 611 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
				FromItem *f = createFromTableRef(NULL, NIL, (yyvsp[-1].stringVal));
				f->provInfo = (FromProvInfo *) (yyvsp[0].node);
                (yyval.node) = (Node *) f;
            }
#line 2175 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 618 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
                FromItem *f = createFromTableRef(((FromItem *) (yyvsp[0].node))->name, 
						((FromItem *) (yyvsp[0].node))->attrNames, (yyvsp[-1].stringVal));
				f->provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
                (yyval.node) = (Node *) f;
            }
#line 2187 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 627 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromItem *f = (FromItem *) (yyvsp[-1].node);
                f->provInfo = (FromProvInfo *) (yyvsp[0].node);
                (yyval.node) = (yyvsp[-1].node);
            }
#line 2198 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 634 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromSubquery *s = (FromSubquery *) (yyvsp[-1].node);
                s->from.name = ((FromItem *) (yyvsp[0].node))->name;
                s->from.attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
                s->from.provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
                (yyval.node) = (Node *) s;
            }
#line 2211 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 643 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[0].node);
        		f->name = NULL;
        		(yyval.node) = (Node *) f;
        	}
#line 2223 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 651 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[-2].node);
        		f->name = ((FromItem *) (yyvsp[0].node))->name;
                f->attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
                f->provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
        		(yyval.node) = (Node *) f;
        	}
#line 2237 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 664 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("subQuery::queryStmt");
                (yyval.node) = (Node *) createFromSubquery(NULL, NULL, (yyvsp[-1].node));
            }
#line 2246 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 671 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = singleton((yyvsp[0].stringVal)); }
#line 2252 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 672 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].stringVal)); }
#line 2258 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 676 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2264 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 678 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURAL");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
						(FromItem *) (yyvsp[0].node), "JOIN_INNER", "JOIN_COND_NATURAL", 
						NULL);
          	}
#line 2275 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 685 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURALjoinType");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[0].node), (yyvsp[-2].stringVal), "JOIN_COND_NATURAL", NULL);
          	}
#line 2285 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 691 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::CROSS JOIN");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[0].node), "JOIN_CROSS", "JOIN_COND_ON", NULL);
          	}
#line 2295 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 697 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinType::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON";
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[-1].node), (yyvsp[-3].stringVal), condType, (yyvsp[0].node));
          	}
#line 2307 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 705 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON"; 
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[-1].node), "JOIN_INNER", 
                		condType, (yyvsp[0].node));
          	}
#line 2320 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 716 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2326 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 717 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT OUTER"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2332 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 718 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT "); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2338 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 719 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT OUTER"); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2344 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 720 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL OUTER"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2350 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 721 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2356 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 722 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::INNER"); (yyval.stringVal) = "JOIN_INNER"; }
#line 2362 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 726 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (Node *) (yyvsp[-1].list); }
#line 2368 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 727 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2374 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 732 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[-2].node);
				(yyval.node) = (Node *) f;
			}
#line 2385 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 739 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[-3].node); 
				(yyval.node) = (Node *) f;
			}
#line 2396 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 748 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFromProv::empty"); (yyval.node) = NULL; }
#line 2402 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 750 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = TRUE;
				p->userProvAttrs = NIL;				 
				(yyval.node) = (Node *) p; 
			}
#line 2414 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 758 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::userProvAttr");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = FALSE;
				p->userProvAttrs = (yyvsp[-1].list);				 
				(yyval.node) = (Node *) p; 
			}
#line 2426 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 768 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAttrAlias::empty"); (yyval.list) = NULL; }
#line 2432 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 770 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAttrAlias::identifierList"); (yyval.list) = (yyvsp[-1].list); 
			}
#line 2440 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 779 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::NULL"); (yyval.node) = NULL; }
#line 2446 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 780 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::whereExpression"); (yyval.node) = (yyvsp[0].node); }
#line 2452 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 784 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("where::brackedWhereExpression"); (yyval.node) = (yyvsp[-1].node); }
#line 2458 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 785 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("whereExpression::expression"); (yyval.node) = (yyvsp[0].node); }
#line 2464 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 787 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::NOT");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2474 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 793 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2485 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 800 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2496 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 807 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2507 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 814 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::BETWEEN-AND");
                List *expr = singleton((yyvsp[-4].node));
                expr = appendToTailOfList(expr, (yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr);
            }
#line 2519 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 822 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::comparisonOps::nestedSubQueryOperator::Subquery");
                (yyval.node) = (Node *) createNestedSubquery((yyvsp[-3].stringVal), (yyvsp[-5].node), (yyvsp[-4].stringVal), (yyvsp[-1].node));
            }
#line 2528 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 827 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::comparisonOps::Subquery");
                List *expr = singleton((yyvsp[-4].node));
                expr = appendToTailOfList(expr, (yyvsp[-1].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr);
            }
#line 2539 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 834 "sql_parser.y" /* yacc.c:1646  */
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
#line 2556 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 847 "sql_parser.y" /* yacc.c:1646  */
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
#line 2573 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 862 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ANY"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2579 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 863 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ALL"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2585 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 864 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::SOME"); (yyval.stringVal) = "ANY"; }
#line 2591 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 868 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NULL"); (yyval.stringVal) = NULL; }
#line 2597 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 869 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NOT"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2603 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 873 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::NULL"); (yyval.list) = NULL; }
#line 2609 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 874 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::GROUPBY"); (yyval.list) = (yyvsp[0].list); }
#line 2615 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 878 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.node) = NULL; }
#line 2621 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 880 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("optionalHaving::HAVING"); 
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2632 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 889 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.list) = NULL; }
#line 2638 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 890 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy::ORDERBY"); (yyval.list) = (yyvsp[0].list); }
#line 2644 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 894 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::NULL"); (yyval.node) = NULL; }
#line 2650 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 895 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::CONSTANT"); (yyval.node) = (yyvsp[0].node);}
#line 2656 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 900 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::attributeRef");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2665 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 905 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2674 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 910 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::constant");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2683 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 915 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2692 "sql_parser.tab.c" /* yacc.c:1646  */
    break;


#line 2696 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 922 "sql_parser.y" /* yacc.c:1906  */




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

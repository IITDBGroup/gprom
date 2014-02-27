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
    ONLY = 275,
    UPDATED = 276,
    SHOW = 277,
    INTERMEDIATE = 278,
    FROM = 279,
    AS = 280,
    WHERE = 281,
    DISTINCT = 282,
    STARALL = 283,
    AND = 284,
    OR = 285,
    LIKE = 286,
    NOT = 287,
    IN = 288,
    ISNULL = 289,
    BETWEEN = 290,
    EXCEPT = 291,
    EXISTS = 292,
    AMMSC = 293,
    NULLVAL = 294,
    ALL = 295,
    ANY = 296,
    IS = 297,
    SOME = 298,
    UNION = 299,
    INTERSECT = 300,
    MINUS = 301,
    INTO = 302,
    VALUES = 303,
    HAVING = 304,
    GROUP = 305,
    ORDER = 306,
    BY = 307,
    LIMIT = 308,
    SET = 309,
    INT = 310,
    BEGIN_TRANS = 311,
    COMMIT_TRANS = 312,
    ROLLBACK_TRANS = 313,
    CASE = 314,
    WHEN = 315,
    THEN = 316,
    ELSE = 317,
    END = 318,
    OVER_TOK = 319,
    PARTITION = 320,
    ROWS = 321,
    RANGE = 322,
    UNBOUNDED = 323,
    PRECEDING = 324,
    CURRENT = 325,
    ROW = 326,
    FOLLOWING = 327,
    DUMMYEXPR = 328,
    JOIN = 329,
    NATURAL = 330,
    LEFT = 331,
    RIGHT = 332,
    OUTER = 333,
    INNER = 334,
    CROSS = 335,
    ON = 336,
    USING = 337,
    FULL = 338,
    TYPE = 339,
    TRANSACTION = 340,
    WITH = 341,
    XOR = 342
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

#line 226 "sql_parser.tab.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SQL_PARSER_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 241 "sql_parser.tab.c" /* yacc.c:358  */

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
#define YYLAST   549

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  103
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  62
/* YYNRULES -- Number of rules.  */
#define YYNRULES  171
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  326

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   342

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
      19,    18,    10,     8,   101,     9,   102,    11,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   100,
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
      97,    98,    99
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   128,   128,   134,   143,   148,   153,   164,   165,   166,
     173,   174,   175,   176,   180,   181,   182,   189,   200,   210,
     222,   223,   228,   236,   237,   245,   246,   254,   259,   264,
     275,   284,   285,   299,   307,   312,   320,   329,   344,   349,
     357,   362,   367,   372,   386,   391,   396,   404,   405,   414,
     437,   438,   443,   455,   459,   467,   472,   477,   482,   493,
     494,   506,   507,   508,   509,   510,   511,   512,   513,   522,
     523,   524,   531,   537,   556,   563,   570,   577,   584,   591,
     600,   607,   616,   626,   638,   653,   658,   666,   671,   679,
     687,   688,   699,   700,   708,   716,   717,   725,   726,   735,
     747,   752,   760,   765,   770,   775,   788,   789,   793,   798,
     807,   814,   823,   830,   839,   847,   860,   868,   869,   873,
     874,   881,   887,   893,   901,   913,   914,   915,   916,   917,
     918,   919,   923,   924,   928,   935,   945,   946,   954,   962,
     972,   973,   983,   984,   988,   989,   990,   996,  1003,  1010,
    1017,  1025,  1030,  1037,  1050,  1066,  1067,  1068,  1072,  1073,
    1077,  1078,  1082,  1083,  1091,  1092,  1096,  1097,  1101,  1106,
    1111,  1116
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
  "FROM", "AS", "WHERE", "DISTINCT", "STARALL", "AND", "OR", "LIKE", "NOT",
  "IN", "ISNULL", "BETWEEN", "EXCEPT", "EXISTS", "AMMSC", "NULLVAL", "ALL",
  "ANY", "IS", "SOME", "UNION", "INTERSECT", "MINUS", "INTO", "VALUES",
  "HAVING", "GROUP", "ORDER", "BY", "LIMIT", "SET", "INT", "BEGIN_TRANS",
  "COMMIT_TRANS", "ROLLBACK_TRANS", "CASE", "WHEN", "THEN", "ELSE", "END",
  "OVER_TOK", "PARTITION", "ROWS", "RANGE", "UNBOUNDED", "PRECEDING",
  "CURRENT", "ROW", "FOLLOWING", "DUMMYEXPR", "JOIN", "NATURAL", "LEFT",
  "RIGHT", "OUTER", "INNER", "CROSS", "ON", "USING", "FULL", "TYPE",
  "TRANSACTION", "WITH", "XOR", "';'", "','", "'.'", "$accept", "stmtList",
  "stmt", "dmlStmt", "queryStmt", "transactionIdentifier", "provStmt",
  "optionalProvAsOf", "optionalProvWith", "provOptionList", "provOption",
  "deleteQuery", "fromString", "updateQuery", "setClause", "setExpression",
  "insertQuery", "insertList", "setOperatorQuery", "optionalAll",
  "selectQuery", "optionalDistinct", "selectClause", "selectItem",
  "exprList", "expression", "constant", "attributeRef", "sqlParameter",
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
     333,   334,   335,   336,   337,   338,   339,   340,   341,   342,
      59,    44,    46
};
# endif

#define YYPACT_NINF -196

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-196)))

#define YYTABLE_NINF -159

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-159)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     281,   161,    10,   -18,    38,    40,    50,  -196,  -196,  -196,
     193,     4,  -196,    11,  -196,  -196,  -196,  -196,  -196,  -196,
    -196,    22,    19,   315,   112,    72,  -196,   142,   144,    74,
    -196,    85,  -196,   122,   161,   161,  -196,   172,  -196,  -196,
    -196,   -10,  -196,  -196,   323,   323,   162,   -28,  -196,   452,
    -196,  -196,  -196,  -196,  -196,  -196,  -196,    29,   188,   157,
     130,   -14,   173,  -196,  -196,   161,  -196,  -196,   323,   323,
     194,   187,   512,   462,   323,   366,    67,  -196,    36,   315,
     170,   323,   323,   323,   323,   323,   323,   323,   323,   323,
     203,   200,    11,  -196,   -24,  -196,   196,   266,   218,   219,
     217,   213,   242,   -14,  -196,    -7,  -196,    -8,   482,    -5,
    -196,  -196,   356,    67,   323,  -196,   175,   306,   154,   152,
     444,   306,  -196,  -196,   266,   206,   396,   396,   243,   243,
     243,  -196,   512,   502,   415,  -196,   437,   188,  -196,   340,
     266,   266,   236,   404,   311,  -196,  -196,  -196,  -196,  -196,
    -196,   281,   269,  -196,   323,   208,   323,   204,   482,  -196,
    -196,   253,   248,  -196,    33,   154,    53,   444,   274,    36,
      36,   454,   197,   222,  -196,   221,   224,   230,  -196,    33,
     311,   259,   233,  -196,    -3,  -196,  -196,   292,   482,  -196,
     387,   392,   311,   161,   238,  -196,   279,   266,   266,   266,
     323,   207,     8,  -196,   482,   313,  -196,   482,  -196,   316,
     330,   319,   335,    88,  -196,   355,   444,   414,    36,   262,
    -196,  -196,    36,  -196,    36,   323,   266,   294,  -196,   441,
     292,  -196,    97,   292,  -196,  -196,  -196,   341,   343,     0,
       0,    87,   442,  -196,  -196,   295,  -196,   378,  -196,   378,
    -196,   319,   132,  -196,    33,   266,   368,  -196,  -196,    36,
    -196,   414,   287,   311,   326,   328,  -196,  -196,  -196,   179,
     161,   161,   323,   327,   294,  -196,    -2,     2,  -196,   311,
     378,  -196,  -196,   481,   261,  -196,  -196,   182,   202,   482,
     323,    83,  -196,   397,  -196,     6,  -196,  -196,   321,  -196,
    -196,  -196,   287,   116,   116,   402,  -196,  -196,   481,   121,
     349,   353,    21,  -196,  -196,  -196,  -196,  -196,  -196,   417,
    -196,  -196,  -196,  -196,   121,  -196
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    50,     0,     0,    31,    20,    14,    15,    16,
       0,     0,     4,     5,     6,    12,     8,     9,     7,    13,
      11,     0,    51,     0,     0,     0,    32,     0,     0,    23,
       1,     0,     2,    47,     0,     0,    10,     0,    69,    70,
      71,    72,    73,    57,     0,     0,     0,   106,    53,    55,
      62,    63,    64,    65,    66,    67,    68,     0,     0,     0,
       0,     0,     0,     3,    48,     0,    44,    45,     0,     0,
       0,    72,    83,     0,     0,     0,    90,    88,     0,     0,
     142,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    39,    72,   142,    34,     0,     0,     0,     0,
       0,     0,     0,    24,    25,     0,    46,     0,    59,     0,
      58,    61,     0,    90,     0,    87,     0,   136,     0,   107,
     108,   136,   114,    54,     0,   160,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    56,     0,     0,    33,     0,
       0,     0,     0,   145,    30,    21,    22,    28,    29,    27,
      26,     0,     0,    52,     0,    92,     0,     0,    91,    86,
     137,     0,     0,   111,   110,     0,     0,     0,   114,     0,
       0,     0,   125,   127,   131,     0,   130,     0,   113,   112,
     143,     0,   162,    41,     0,    40,    35,     0,    36,    37,
     145,     0,   146,     0,     0,   159,     0,     0,     0,     0,
       0,     0,     0,    19,    60,     0,    84,    89,    85,     0,
       0,   140,     0,     0,   116,   119,   109,     0,     0,     0,
     126,   128,     0,   129,     0,     0,     0,   164,    38,     0,
       0,   144,     0,     0,   156,   155,   157,     0,     0,   147,
     148,   149,     0,    18,    17,    95,    93,     0,   139,     0,
     134,   140,   116,   115,     0,     0,     0,   124,   120,     0,
     122,     0,   161,   163,     0,   166,    42,    43,   154,     0,
       0,     0,     0,     0,   164,   117,     0,     0,   135,   133,
       0,   121,   123,     0,     0,    49,   152,     0,     0,   150,
       0,    97,   138,     0,   141,     0,   170,   168,   165,   167,
     151,   153,    96,     0,     0,     0,   118,   132,     0,     0,
       0,     0,     0,    98,   101,    99,    94,   171,   169,     0,
     102,   103,   104,   105,     0,   100
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -196,   286,     5,  -196,    18,  -196,  -196,  -196,  -196,  -196,
     365,  -196,  -196,  -196,  -196,   344,  -196,  -196,  -196,  -196,
    -196,  -196,  -196,   399,   -68,   -23,  -132,   -56,  -196,  -196,
    -196,  -196,  -196,   407,   -39,   375,  -196,  -196,  -196,  -196,
     214,  -195,  -196,  -196,   -75,   359,  -192,   -93,   333,   256,
    -115,  -114,   275,   433,  -113,  -196,  -196,  -196,  -196,   254,
    -196,  -196
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    10,    31,    12,    13,    14,    15,    29,    62,   103,
     104,    16,    27,    17,    94,    95,    18,   184,    19,    65,
      20,    23,    47,    48,   107,   143,    50,    51,    52,    53,
      54,    55,    56,    76,    77,   116,   206,   246,   274,   305,
     313,   314,    80,   119,   167,   121,   276,   122,   177,   257,
     163,   164,   250,   125,   144,   237,   196,   182,   227,   265,
     285,   298
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      49,   109,    96,   120,   185,    11,   178,   179,    78,    69,
     153,   180,   151,   155,   124,   228,   292,   100,   101,    21,
     294,    72,    73,    75,   307,   168,   244,   191,   192,    81,
      82,    83,    84,    85,    86,    87,    88,   115,    89,   211,
      36,    24,   117,   199,    25,   108,   108,   200,     1,    22,
       2,   112,    66,    67,     6,   118,    49,   277,   126,   127,
     128,   129,   130,   131,   132,   133,   134,    33,    34,    35,
     212,   214,   168,    79,   115,    92,    26,   137,    33,    34,
      35,    96,   102,   106,   239,   240,   241,    28,   295,    91,
     152,   158,    70,   154,   216,   217,   154,   267,   229,   293,
     253,   254,   322,   293,    32,   323,   252,   293,    32,    33,
      34,    35,    37,   263,   319,   268,   188,   190,    57,    38,
      39,    40,    71,    42,    38,    39,    40,    71,    42,   325,
    -159,   204,    44,   207,  -159,    45,   166,    44,    58,    74,
      45,   114,   279,   258,    33,    34,    35,   260,    59,   261,
     -10,   296,   299,    33,    34,    35,   202,   262,    98,    99,
     117,   303,   304,   309,    73,    38,    39,    40,    71,    42,
      60,   134,    61,   165,    64,     2,   317,   242,    44,     6,
       1,    45,     2,   213,   281,    63,     6,    46,   -10,   -10,
     -10,    68,    46,    30,    93,    97,   310,   286,   311,   105,
     300,   310,   108,   311,   110,   166,    69,    73,   124,   135,
      73,   232,     1,   139,     2,     3,     4,     5,     6,   136,
     301,   145,   302,   147,   146,   243,     1,   297,     2,     3,
       4,     5,     6,    46,    74,    33,    34,    35,    33,    34,
      35,    38,    39,    40,    71,    42,   148,   149,    21,   289,
     159,   269,   318,   169,    44,   193,    86,   233,    33,    34,
      35,     7,     8,     9,    38,    39,    40,   108,   181,    38,
      39,    40,    71,    42,   203,     7,     8,     9,   209,   208,
     312,   312,    44,   210,   205,   140,   312,   220,   287,   288,
     234,   235,   215,   236,   226,    38,    39,    40,    71,    42,
       1,   312,     2,     3,     4,     5,     6,   222,    44,    46,
     141,   230,   221,     2,   223,   142,   224,     6,    38,    39,
      40,    41,    42,   225,   238,    43,    38,    39,    40,    71,
      42,    44,   245,   160,    45,   247,   161,    46,   249,    44,
     162,   251,    45,    38,    39,    40,    71,    42,   259,     7,
       8,     9,   197,   198,   199,   248,    44,   264,   200,   187,
     270,  -136,   271,    46,    81,    82,    83,    84,    85,    86,
      87,    88,   273,    89,    81,    82,    83,    84,    85,    86,
      87,    88,   160,    89,   275,   161,    46,   280,   154,   162,
     283,   290,  -136,   284,    46,    81,    82,    83,    84,    85,
      86,    87,    88,   306,   194,   111,    83,    84,    85,    86,
     231,    46,    81,    82,    83,    84,    85,    86,    87,    88,
     316,   194,   308,    81,    82,    83,    84,    85,    86,   156,
     320,   195,  -158,   197,   198,   199,   321,   201,    74,   200,
      38,    39,    40,   183,    38,    39,    40,   266,   195,  -158,
      81,    82,    83,    84,    85,    86,    87,    88,   324,    89,
      81,    82,    83,    84,    85,    86,    87,    88,   150,    89,
      81,    82,    83,    84,    85,    86,    87,    88,   123,    89,
     111,   186,   113,   272,    38,    39,    40,    93,   157,    90,
      81,    82,    83,    84,    85,    86,    87,    88,   189,    89,
     170,   171,   172,   173,   219,   174,   175,   255,   256,   176,
      81,    82,    83,    84,    85,    86,    87,   282,   315,    89,
      81,    82,    83,    84,    85,    86,   278,   138,   291,    89,
     170,   171,   172,   173,     0,   174,   175,     0,     0,   176,
     218,     0,   172,   173,     0,   174,     0,     0,     0,   176
};

static const yytype_int16 yycheck[] =
{
      23,    69,    58,    78,   136,     0,   121,   121,    36,    19,
      18,   124,    19,    18,    38,    18,    18,    31,    32,     1,
      18,    44,    45,    46,    18,   118,    18,   140,   141,     8,
       9,    10,    11,    12,    13,    14,    15,    76,    17,     6,
      18,    59,     6,    43,     6,    68,    69,    47,    19,    39,
      21,    74,    34,    35,    25,    19,    79,   249,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    56,    57,    58,
      37,    18,   165,   101,   113,    57,    36,   101,    56,    57,
      58,   137,    96,    65,   197,   198,   199,    37,   280,    60,
      97,   114,   102,   101,   169,   170,   101,   229,   101,   101,
     215,   215,    81,   101,   100,    84,    18,   101,   100,    56,
      57,    58,    93,   226,   309,    18,   139,   140,     6,     3,
       4,     5,     6,     7,     3,     4,     5,     6,     7,   324,
      43,   154,    16,   156,    47,    19,   118,    16,    66,    72,
      19,    74,   255,   218,    56,    57,    58,   222,     6,   224,
      18,   283,   284,    56,    57,    58,   151,   225,    28,    29,
       6,    78,    79,    47,   187,     3,     4,     5,     6,     7,
      26,   194,    98,    19,    52,    21,   308,   200,    16,    25,
      19,    19,    21,   165,   259,   100,    25,    71,    56,    57,
      58,    19,    71,     0,     6,    38,    80,    18,    82,    26,
      18,    80,   225,    82,    10,   187,    19,   230,    38,     6,
     233,   193,    19,    17,    21,    22,    23,    24,    25,    19,
      18,     3,   290,     6,     5,    18,    19,   283,    21,    22,
      23,    24,    25,    71,    72,    56,    57,    58,    56,    57,
      58,     3,     4,     5,     6,     7,    33,     5,   230,   272,
      75,   233,   308,   101,    16,    19,    13,    19,    56,    57,
      58,    68,    69,    70,     3,     4,     5,   290,    62,     3,
       4,     5,     6,     7,     5,    68,    69,    70,    25,    75,
     303,   304,    16,    35,    76,    19,   309,    90,   270,   271,
      52,    53,    18,    55,    61,     3,     4,     5,     6,     7,
      19,   324,    21,    22,    23,    24,    25,    86,    16,    71,
      44,    19,    90,    21,    90,    49,    86,    25,     3,     4,
       5,     6,     7,    64,    45,    10,     3,     4,     5,     6,
       7,    16,    19,    27,    19,    19,    30,    71,    19,    16,
      34,     6,    19,     3,     4,     5,     6,     7,    86,    68,
      69,    70,    41,    42,    43,    25,    16,    63,    47,    19,
      19,     6,    19,    71,     8,     9,    10,    11,    12,    13,
      14,    15,    77,    17,     8,     9,    10,    11,    12,    13,
      14,    15,    27,    17,     6,    30,    71,    19,   101,    34,
      64,    64,    37,    65,    71,     8,     9,    10,    11,    12,
      13,    14,    15,     6,    17,    18,    10,    11,    12,    13,
      18,    71,     8,     9,    10,    11,    12,    13,    14,    15,
      18,    17,   101,     8,     9,    10,    11,    12,    13,    73,
      81,    44,    45,    41,    42,    43,    83,   151,    72,    47,
       3,     4,     5,     6,     3,     4,     5,     6,    44,    45,
       8,     9,    10,    11,    12,    13,    14,    15,    41,    17,
       8,     9,    10,    11,    12,    13,    14,    15,   103,    17,
       8,     9,    10,    11,    12,    13,    14,    15,    79,    17,
      18,   137,    75,    41,     3,     4,     5,     6,   113,    37,
       8,     9,    10,    11,    12,    13,    14,    15,   139,    17,
      86,    87,    88,    89,   171,    91,    92,    93,    94,    95,
       8,     9,    10,    11,    12,    13,    14,   261,   304,    17,
       8,     9,    10,    11,    12,    13,   251,    94,   274,    17,
      86,    87,    88,    89,    -1,    91,    92,    -1,    -1,    95,
      86,    -1,    88,    89,    -1,    91,    -1,    -1,    -1,    95
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    19,    21,    22,    23,    24,    25,    68,    69,    70,
     104,   105,   106,   107,   108,   109,   114,   116,   119,   121,
     123,   107,    39,   124,    59,     6,    36,   115,    37,   110,
       0,   105,   100,    56,    57,    58,    18,    93,     3,     4,
       5,     6,     7,    10,    16,    19,    71,   125,   126,   128,
     129,   130,   131,   132,   133,   134,   135,     6,    66,     6,
      26,    98,   111,   100,    52,   122,   107,   107,    19,    19,
     102,     6,   128,   128,    72,   128,   136,   137,    36,   101,
     145,     8,     9,    10,    11,    12,    13,    14,    15,    17,
      37,    60,   107,     6,   117,   118,   130,    38,    28,    29,
      31,    32,    96,   112,   113,    26,   107,   127,   128,   127,
      10,    18,   128,   136,    74,   137,   138,     6,    19,   146,
     147,   148,   150,   126,    38,   156,   128,   128,   128,   128,
     128,   128,   128,   128,   128,     6,    19,   101,   156,    17,
      19,    44,    49,   128,   157,     3,     5,     6,    33,     5,
     113,    19,    97,    18,   101,    18,    73,   138,   128,    75,
      27,    30,    34,   153,   154,    19,   107,   147,   150,   101,
      86,    87,    88,    89,    91,    92,    95,   151,   153,   154,
     157,    62,   160,     6,   120,   129,   118,    19,   128,   148,
     128,   157,   157,    19,    17,    44,   159,    41,    42,    43,
      47,   104,   105,     5,   128,    76,   139,   128,    75,    25,
      35,     6,    37,   107,    18,    18,   147,   147,    86,   151,
      90,    90,    86,    90,    86,    64,    61,   161,    18,   101,
      19,    18,   107,    19,    52,    53,    55,   158,    45,   157,
     157,   157,   128,    18,    18,    19,   140,    19,    25,    19,
     155,     6,    18,   153,   154,    93,    94,   152,   147,    86,
     147,   147,   127,   157,    63,   162,     6,   129,    18,   107,
      19,    19,    41,    77,   141,     6,   149,   149,   155,   157,
      19,   147,   152,    64,    65,   163,    18,   107,   107,   128,
      64,   162,    18,   101,    18,   149,   129,   130,   164,   129,
      18,    18,   127,    78,    79,   142,     6,    18,   101,    47,
      80,    82,   128,   143,   144,   143,    18,   129,   130,   144,
      81,    83,    81,    84,    41,   144
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   103,   104,   104,   105,   105,   105,   106,   106,   106,
     107,   107,   107,   107,   108,   108,   108,   109,   109,   109,
     110,   110,   110,   111,   111,   112,   112,   113,   113,   113,
     114,   115,   115,   116,   117,   117,   118,   118,   119,   119,
     120,   120,   120,   120,   121,   121,   121,   122,   122,   123,
     124,   124,   124,   125,   125,   126,   126,   126,   126,   127,
     127,   128,   128,   128,   128,   128,   128,   128,   128,   129,
     129,   129,   130,   131,   132,   132,   132,   132,   132,   132,
     132,   132,   132,   133,   134,   135,   135,   136,   136,   137,
     138,   138,   139,   139,   140,   141,   141,   142,   142,   142,
     143,   143,   144,   144,   144,   144,   145,   145,   146,   146,
     147,   147,   147,   147,   147,   147,   148,   149,   149,   150,
     150,   150,   150,   150,   150,   151,   151,   151,   151,   151,
     151,   151,   152,   152,   153,   153,   154,   154,   154,   154,
     155,   155,   156,   156,   157,   157,   157,   157,   157,   157,
     157,   157,   157,   157,   157,   158,   158,   158,   159,   159,
     160,   160,   161,   161,   162,   162,   163,   163,   164,   164,
     164,   164
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     3,     1,     1,     1,     1,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     7,     7,     6,
       0,     4,     4,     0,     2,     1,     2,     2,     2,     2,
       5,     0,     1,     5,     1,     3,     3,     3,     7,     4,
       1,     1,     3,     3,     3,     3,     4,     0,     1,     9,
       0,     1,     5,     1,     3,     1,     3,     1,     3,     1,
       3,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     5,     5,     4,     2,     1,     4,
       0,     2,     0,     2,     5,     0,     3,     0,     2,     2,
       4,     1,     2,     2,     2,     2,     0,     2,     1,     3,
       2,     2,     2,     2,     1,     4,     3,     1,     3,     3,
       4,     5,     4,     5,     4,     1,     2,     1,     2,     2,
       1,     1,     4,     2,     3,     4,     0,     1,     5,     3,
       0,     3,     0,     2,     3,     1,     2,     3,     3,     3,
       5,     6,     5,     6,     4,     1,     1,     1,     0,     1,
       0,     3,     0,     2,     0,     3,     0,     2,     1,     3,
       1,     3
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
#line 1623 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 135 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("stmtlist::stmtList::stmt");
				(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[-1].node));	
				bisonParseResult = (Node *) (yyval.list); 
			}
#line 1633 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 144 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::dmlStmt");
            (yyval.node) = (yyvsp[0].node);
        }
#line 1642 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 149 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::queryStmt");
            (yyval.node) = (yyvsp[0].node);
        }
#line 1651 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 154 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::transactionIdentifier");
            (yyval.node) = (Node *) createTransactionStmt((yyvsp[0].stringVal));
        }
#line 1660 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 164 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::insertQuery"); }
#line 1666 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 165 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::deleteQuery"); }
#line 1672 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 166 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::updateQuery"); }
#line 1678 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 173 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::bracketedQuery"); (yyval.node) = (yyvsp[-1].node); }
#line 1684 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 174 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::selectQuery"); }
#line 1690 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 175 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::provStmt"); }
#line 1696 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 176 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::setOperatorQuery"); }
#line 1702 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 180 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::BEGIN"); (yyval.stringVal) = strdup("TRANSACTION_BEGIN"); }
#line 1708 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 181 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::COMMIT"); (yyval.stringVal) = strdup("TRANSACTION_COMMIT"); }
#line 1714 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 182 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::ROLLBACK"); (yyval.stringVal) = strdup("TRANSACTION_ABORT"); }
#line 1720 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 1735 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 1749 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 1762 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 222 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalProvAsOf::EMPTY"); (yyval.node) = NULL; }
#line 1768 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 224 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvAsOf::SCN");
			(yyval.node) = (Node *) createConstLong((yyvsp[0].intVal));
		}
#line 1777 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 229 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvAsOf::TIMESTAMP");
			(yyval.node) = (Node *) createConstString((yyvsp[0].stringVal));
		}
#line 1786 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 236 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalProvWith::EMPTY"); (yyval.list) = NIL; }
#line 1792 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 238 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvWith::WITH");
			(yyval.list) = (yyvsp[0].list);
		}
#line 1801 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 245 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("provOptionList::option"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 1807 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 247 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("provOptionList::list"); 
			(yyval.list) = appendToTailOfList((yyvsp[-1].list),(yyvsp[0].node)); 
		}
#line 1816 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 255 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("provOption::TYPE"); 
			(yyval.node) = (Node *) createStringKeyValue("TYPE", (yyvsp[0].stringVal)); 
		}
#line 1825 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 260 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::TABLE");
			(yyval.node) = (Node *) createStringKeyValue("TABLE", (yyvsp[0].stringVal));
		}
#line 1834 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 265 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::ONLY::UPDATED");
			(yyval.node) = (Node *) createStringKeyValue("ONLY_UPDATED", NULL);
		}
#line 1843 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 276 "sql_parser.y" /* yacc.c:1646  */
    { 
             RULELOG("deleteQuery");
             (yyval.node) = (Node *) createDelete((yyvsp[-2].stringVal), (yyvsp[0].node));
         }
#line 1852 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 284 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::NULL"); (yyval.stringVal) = NULL; }
#line 1858 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 285 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::FROM"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 1864 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 300 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG(updateQuery); 
                (yyval.node) = (Node *) createUpdate((yyvsp[-3].stringVal), (yyvsp[-1].list), (yyvsp[0].node)); 
            }
#line 1873 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 308 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setExpression");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 1882 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 313 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setClause::setExpression");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 1891 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 321 "sql_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal),"=")) {
                    RULELOG("setExpression::attributeRef::expression");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 1904 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 330 "sql_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal), "=")) {
                    RULELOG("setExpression::attributeRef::queryStmt");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 1917 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 345 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertQuery::insertList"); 
            	(yyval.node) = (Node *) createInsert((yyvsp[-4].stringVal),(Node *) (yyvsp[-1].list), NULL); 
        	}
#line 1926 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 350 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("insertQuery::queryStmt");
                (yyval.node) = (Node *) createInsert((yyvsp[-1].stringVal), (yyvsp[0].node), NULL);
            }
#line 1935 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 358 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::constant");
            	(yyval.list) = singleton((yyvsp[0].node)); 
            }
#line 1944 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 363 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("insertList::IDENTIFIER");
                (yyval.list) = singleton(createAttributeReference((yyvsp[0].stringVal)));
            }
#line 1953 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 368 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("insertList::insertList::::IDENTIFIER");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), createAttributeReference((yyvsp[0].stringVal)));
            }
#line 1962 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 373 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::insertList::constant");
            	(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 1971 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 387 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::INTERSECT");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 1980 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 392 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::MINUS");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 1989 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 397 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::UNION");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-2].stringVal), ((yyvsp[-1].stringVal) != NULL), (yyvsp[-3].node), (yyvsp[0].node));
            }
#line 1998 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 404 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::NULL"); (yyval.stringVal) = NULL; }
#line 2004 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 405 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::ALLTRUE"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2010 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 415 "sql_parser.y" /* yacc.c:1646  */
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
#line 2030 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 437 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalDistinct::NULL"); (yyval.node) = NULL; }
#line 2036 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 439 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT");
                (yyval.node) = (Node *) createDistinctClause(NULL);
            }
#line 2045 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 444 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT::exprList");
                (yyval.node) = (Node *) createDistinctClause((yyvsp[-1].list));
            }
#line 2054 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 456 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectItem"); (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2062 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 460 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectClause::selectItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node)); 
            }
#line 2071 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 468 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression"); 
                 (yyval.node) = (Node *) createSelectItem(NULL, (yyvsp[0].node)); 
             }
#line 2080 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 473 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression::identifier"); 
                 (yyval.node) = (Node *) createSelectItem((yyvsp[0].stringVal), (yyvsp[-2].node));
             }
#line 2089 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 478 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
         		(yyval.node) = (Node *) createSelectItem(strdup("*"), NULL); 
     		}
#line 2098 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 483 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
     			(yyval.node) = (Node *) createSelectItem(CONCAT_STRINGS((yyvsp[-2].stringVal),".*"), NULL); 
 			}
#line 2107 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 493 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("exprList::SINGLETON"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 2113 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 495 "sql_parser.y" /* yacc.c:1646  */
    {
                  RULELOG("exprList::exprList::expression");
                  (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
             }
#line 2122 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 506 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::bracked"); (yyval.node) = (yyvsp[-1].node); }
#line 2128 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 507 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::constant"); }
#line 2134 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 508 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::attributeRef"); }
#line 2140 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 509 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlParameter"); }
#line 2146 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 510 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::binaryOperatorExpression"); }
#line 2152 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 511 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::unaryOperatorExpression"); }
#line 2158 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 512 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlFunctionCall"); }
#line 2164 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 513 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::case"); }
#line 2170 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 522 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::INT"); (yyval.node) = (Node *) createConstInt((yyvsp[0].intVal)); }
#line 2176 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 523 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::FLOAT"); (yyval.node) = (Node *) createConstFloat((yyvsp[0].floatVal)); }
#line 2182 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 524 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::STRING"); (yyval.node) = (Node *) createConstString((yyvsp[0].stringVal)); }
#line 2188 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 531 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("attributeRef::IDENTIFIER"); (yyval.node) = (Node *) createAttributeReference((yyvsp[0].stringVal)); }
#line 2194 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 537 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("sqlParameter::PARAMETER"); (yyval.node) = (Node *) createSQLParameter((yyvsp[0].stringVal)); }
#line 2200 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 557 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '+' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2211 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 564 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '-' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2222 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 571 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '*' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2233 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 578 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '/' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2244 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 585 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '%' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2255 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 592 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '^' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2266 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 601 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '&' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2277 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 608 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '|' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2288 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 617 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression::comparisonOps");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2299 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 627 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("unaryOperatorExpression:: '!' ");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2309 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 639 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("sqlFunctionCall::IDENTIFIER::exprList");
				FunctionCall *f = createFunctionCall((yyvsp[-4].stringVal), (yyvsp[-2].list));
				if ((yyvsp[0].node) != NULL)
					(yyval.node) = (Node *) createWindowFunction(f, (WindowDef *) (yyvsp[0].node));
				else  
                	(yyval.node) = (Node *) f; 
            }
#line 2322 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 654 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseExpression::CASE::expression::whens:else:END");
				(yyval.node) = (Node *) createCaseExpr((yyvsp[-3].node), (yyvsp[-2].list), (yyvsp[-1].node));
			}
#line 2331 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 659 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseExpression::CASE::whens::else::END");
				(yyval.node) = (Node *) createCaseExpr(NULL, (yyvsp[-2].list), (yyvsp[-1].node));
			}
#line 2340 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 667 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhenList::list::caseWhen");
				(yyval.list) = appendToTailOfList((yyvsp[-1].list), (yyvsp[0].node));
			}
#line 2349 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 672 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhenList::caseWhen");
				(yyval.list) = singleton((yyvsp[0].node));
			}
#line 2358 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 680 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhen::WHEN::expression::THEN::expression");
				(yyval.node) = (Node *) createCaseWhen((yyvsp[-2].node),(yyvsp[0].node));
			}
#line 2367 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 687 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalCaseElse::NULL"); (yyval.node) = NULL; }
#line 2373 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 689 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalCaseElse::ELSE::expression");
				(yyval.node) = (yyvsp[0].node);
			}
#line 2382 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 699 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("overclause::NULL"); (yyval.node) = NULL; }
#line 2388 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 701 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("overclause::window");
				(yyval.node) = (yyvsp[0].node);
			}
#line 2397 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 709 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("window");
				(yyval.node) = (Node *) createWindowDef((yyvsp[-3].list),(yyvsp[-2].list), (WindowFrame *) (yyvsp[-1].node));
			}
#line 2406 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 716 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optWindowPart::NULL"); (yyval.list) = NIL; }
#line 2412 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 718 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optWindowPart::PARTITION:BY::expressionList");
				(yyval.list) = (yyvsp[0].list);
			}
#line 2421 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 725 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optWindowFrame::NULL"); (yyval.node) = NULL; }
#line 2427 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 727 "sql_parser.y" /* yacc.c:1646  */
    { 
				WindowBound *l, *u = NULL;
				RULELOG("optWindowFrame::ROWS::windoBoundaries");
				l = getNthOfListP((yyvsp[0].list), 0);
				if(LIST_LENGTH((yyvsp[0].list)) > 1)
					u = getNthOfListP((yyvsp[0].list), 1);
				(yyval.node) = (Node *) createWindowFrame(WINFRAME_ROWS, l, u); 
			}
#line 2440 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 736 "sql_parser.y" /* yacc.c:1646  */
    {
				WindowBound *l, *u = NULL; 
				RULELOG("optWindowFrame::RANGE::windoBoundaries"); 
				l = getNthOfListP((yyvsp[0].list), 0);
				if(LIST_LENGTH((yyvsp[0].list)) > 1)
					u = getNthOfListP((yyvsp[0].list), 1);
				(yyval.node) = (Node *) createWindowFrame(WINFRAME_RANGE, l, u); 
			}
#line 2453 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 748 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBoundaries::BETWEEN"); 
				(yyval.list) = LIST_MAKE((yyvsp[-2].node), (yyvsp[0].node)); 
			}
#line 2462 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 753 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBoundaries::windowBound"); 
				(yyval.list) = singleton((yyvsp[0].node)); 
			}
#line 2471 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 761 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::UNBOUNDED::PRECEDING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_UNBOUND_PREC, NULL); 
			}
#line 2480 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 766 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::CURRENT::ROW"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_CURRENT_ROW, NULL); 
			}
#line 2489 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 771 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::expression::PRECEDING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_EXPR_PREC, (yyvsp[-1].node)); 
			}
#line 2498 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 776 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::expression::FOLLOWING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_EXPR_FOLLOW, (yyvsp[-1].node)); 
			}
#line 2507 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 788 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::NULL"); (yyval.list) = NULL; }
#line 2513 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 789 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::fromClause"); (yyval.list) = (yyvsp[0].list); }
#line 2519 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 794 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClauseItem");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2528 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 799 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClause::fromClauseItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2537 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 808 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
				FromItem *f = createFromTableRef(NULL, NIL, (yyvsp[-1].stringVal));
				f->provInfo = (FromProvInfo *) (yyvsp[0].node);
                (yyval.node) = (Node *) f;
            }
#line 2548 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 815 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
                FromItem *f = createFromTableRef(((FromItem *) (yyvsp[0].node))->name, 
						((FromItem *) (yyvsp[0].node))->attrNames, (yyvsp[-1].stringVal));
				f->provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
                (yyval.node) = (Node *) f;
            }
#line 2560 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 824 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromItem *f = (FromItem *) (yyvsp[-1].node);
                f->provInfo = (FromProvInfo *) (yyvsp[0].node);
                (yyval.node) = (yyvsp[-1].node);
            }
#line 2571 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 831 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromSubquery *s = (FromSubquery *) (yyvsp[-1].node);
                s->from.name = ((FromItem *) (yyvsp[0].node))->name;
                s->from.attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
                s->from.provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
                (yyval.node) = (Node *) s;
            }
#line 2584 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 840 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[0].node);
        		f->name = NULL;
        		(yyval.node) = (Node *) f;
        	}
#line 2596 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 848 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[-2].node);
        		f->name = ((FromItem *) (yyvsp[0].node))->name;
                f->attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
                f->provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
        		(yyval.node) = (Node *) f;
        	}
#line 2610 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 861 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("subQuery::queryStmt");
                (yyval.node) = (Node *) createFromSubquery(NULL, NULL, (yyvsp[-1].node));
            }
#line 2619 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 868 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = singleton((yyvsp[0].stringVal)); }
#line 2625 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 869 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].stringVal)); }
#line 2631 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 873 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2637 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 875 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURAL");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
						(FromItem *) (yyvsp[0].node), "JOIN_INNER", "JOIN_COND_NATURAL", 
						NULL);
          	}
#line 2648 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 882 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURALjoinType");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[0].node), (yyvsp[-2].stringVal), "JOIN_COND_NATURAL", NULL);
          	}
#line 2658 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 888 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::CROSS JOIN");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[0].node), "JOIN_CROSS", "JOIN_COND_ON", NULL);
          	}
#line 2668 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 894 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinType::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON";
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[-1].node), (yyvsp[-3].stringVal), condType, (yyvsp[0].node));
          	}
#line 2680 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 902 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON"; 
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[-1].node), "JOIN_INNER", 
                		condType, (yyvsp[0].node));
          	}
#line 2693 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 913 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2699 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 914 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT OUTER"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2705 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 915 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT "); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2711 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 916 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT OUTER"); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2717 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 917 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL OUTER"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2723 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 918 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2729 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 919 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::INNER"); (yyval.stringVal) = "JOIN_INNER"; }
#line 2735 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 923 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (Node *) (yyvsp[-1].list); }
#line 2741 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 924 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2747 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 929 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[-2].node);
				(yyval.node) = (Node *) f;
			}
#line 2758 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 936 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[-3].node); 
				(yyval.node) = (Node *) f;
			}
#line 2769 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 945 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFromProv::empty"); (yyval.node) = NULL; }
#line 2775 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 947 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::BASERELATION");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = TRUE;
				p->userProvAttrs = NIL;				 
				(yyval.node) = (Node *) p; 
			}
#line 2787 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 955 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::userProvAttr");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = FALSE;
				p->userProvAttrs = (yyvsp[-1].list);				 
				(yyval.node) = (Node *) p; 
			}
#line 2799 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 963 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::intermediateProv");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->intermediateProv = TRUE;
				(yyval.node) = (Node *) p;
			}
#line 2810 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 972 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAttrAlias::empty"); (yyval.list) = NULL; }
#line 2816 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 974 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAttrAlias::identifierList"); (yyval.list) = (yyvsp[-1].list); 
			}
#line 2824 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 983 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::NULL"); (yyval.node) = NULL; }
#line 2830 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 984 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::whereExpression"); (yyval.node) = (yyvsp[0].node); }
#line 2836 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 988 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("where::brackedWhereExpression"); (yyval.node) = (yyvsp[-1].node); }
#line 2842 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 989 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("whereExpression::expression"); (yyval.node) = (yyvsp[0].node); }
#line 2848 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 991 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::NOT");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2858 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 997 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2869 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1004 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2880 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1011 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2891 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1018 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::BETWEEN-AND");
                List *expr = singleton((yyvsp[-4].node));
                expr = appendToTailOfList(expr, (yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr);
            }
#line 2903 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1026 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::comparisonOps::nestedSubQueryOperator::Subquery");
                (yyval.node) = (Node *) createNestedSubquery((yyvsp[-3].stringVal), (yyvsp[-5].node), (yyvsp[-4].stringVal), (yyvsp[-1].node));
            }
#line 2912 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1031 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::Subquery");
                Node *q = (Node *) createNestedSubquery("SCALAR", NULL, NULL, (yyvsp[-1].node)); 
                List *expr = LIST_MAKE((yyvsp[-4].node), q);
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr); 
            }
#line 2923 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1038 "sql_parser.y" /* yacc.c:1646  */
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
#line 2940 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1051 "sql_parser.y" /* yacc.c:1646  */
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
#line 2957 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1066 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ANY"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2963 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 1067 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ALL"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2969 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 1068 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::SOME"); (yyval.stringVal) = "ANY"; }
#line 2975 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1072 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NULL"); (yyval.stringVal) = NULL; }
#line 2981 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1073 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NOT"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2987 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 1077 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::NULL"); (yyval.list) = NULL; }
#line 2993 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1078 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::GROUPBY"); (yyval.list) = (yyvsp[0].list); }
#line 2999 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1082 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.node) = NULL; }
#line 3005 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1084 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("optionalHaving::HAVING"); 
                (yyval.node) = (Node *) (yyvsp[0].node);
            }
#line 3014 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 1091 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.list) = NULL; }
#line 3020 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 1092 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy::ORDERBY"); (yyval.list) = (yyvsp[0].list); }
#line 3026 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 1096 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::NULL"); (yyval.node) = NULL; }
#line 3032 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1097 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::CONSTANT"); (yyval.node) = (yyvsp[0].node);}
#line 3038 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 1102 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::attributeRef");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 3047 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 1107 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 3056 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 1112 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::constant");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 3065 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 1117 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 3074 "sql_parser.tab.c" /* yacc.c:1646  */
    break;


#line 3078 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 1124 "sql_parser.y" /* yacc.c:1906  */




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

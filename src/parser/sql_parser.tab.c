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
    USE = 279,
    FROM = 280,
    AS = 281,
    WHERE = 282,
    DISTINCT = 283,
    STARALL = 284,
    AND = 285,
    OR = 286,
    LIKE = 287,
    NOT = 288,
    IN = 289,
    ISNULL = 290,
    BETWEEN = 291,
    EXCEPT = 292,
    EXISTS = 293,
    AMMSC = 294,
    NULLVAL = 295,
    ALL = 296,
    ANY = 297,
    IS = 298,
    SOME = 299,
    UNION = 300,
    INTERSECT = 301,
    MINUS = 302,
    INTO = 303,
    VALUES = 304,
    HAVING = 305,
    GROUP = 306,
    ORDER = 307,
    BY = 308,
    LIMIT = 309,
    SET = 310,
    INT = 311,
    BEGIN_TRANS = 312,
    COMMIT_TRANS = 313,
    ROLLBACK_TRANS = 314,
    CASE = 315,
    WHEN = 316,
    THEN = 317,
    ELSE = 318,
    END = 319,
    OVER_TOK = 320,
    PARTITION = 321,
    ROWS = 322,
    RANGE = 323,
    UNBOUNDED = 324,
    PRECEDING = 325,
    CURRENT = 326,
    ROW = 327,
    FOLLOWING = 328,
    DUMMYEXPR = 329,
    JOIN = 330,
    NATURAL = 331,
    LEFT = 332,
    RIGHT = 333,
    OUTER = 334,
    INNER = 335,
    CROSS = 336,
    ON = 337,
    USING = 338,
    FULL = 339,
    TYPE = 340,
    TRANSACTION = 341,
    WITH = 342,
    XOR = 343
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

#line 227 "sql_parser.tab.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SQL_PARSER_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 242 "sql_parser.tab.c" /* yacc.c:358  */

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
#define YYLAST   589

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  104
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  65
/* YYNRULES -- Number of rules.  */
#define YYNRULES  179
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  348

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   343

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
      19,    18,    10,     8,   102,     9,   103,    11,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   101,
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
      97,    98,    99,   100
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   131,   131,   137,   146,   151,   156,   161,   172,   173,
     174,   181,   182,   183,   184,   188,   196,   201,   209,   217,
     218,   219,   226,   237,   247,   259,   260,   265,   273,   274,
     282,   283,   291,   296,   301,   307,   319,   328,   329,   343,
     351,   356,   364,   373,   388,   393,   401,   406,   411,   416,
     430,   435,   440,   448,   449,   458,   481,   482,   487,   499,
     503,   511,   516,   521,   526,   537,   538,   550,   551,   552,
     553,   554,   555,   556,   557,   566,   567,   568,   575,   581,
     600,   607,   614,   621,   628,   635,   644,   651,   660,   670,
     682,   697,   702,   710,   715,   723,   731,   732,   743,   744,
     752,   760,   761,   769,   770,   779,   791,   796,   804,   809,
     814,   819,   832,   833,   837,   842,   851,   858,   867,   874,
     883,   891,   904,   912,   913,   917,   918,   925,   931,   937,
     945,   957,   958,   959,   960,   961,   962,   963,   967,   968,
     972,   979,   989,   990,   998,  1006,  1014,  1021,  1032,  1033,
    1043,  1044,  1048,  1049,  1050,  1056,  1063,  1070,  1077,  1085,
    1090,  1097,  1110,  1126,  1127,  1128,  1132,  1133,  1137,  1138,
    1142,  1143,  1151,  1152,  1156,  1157,  1161,  1166,  1171,  1176
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
  "USE", "FROM", "AS", "WHERE", "DISTINCT", "STARALL", "AND", "OR", "LIKE",
  "NOT", "IN", "ISNULL", "BETWEEN", "EXCEPT", "EXISTS", "AMMSC", "NULLVAL",
  "ALL", "ANY", "IS", "SOME", "UNION", "INTERSECT", "MINUS", "INTO",
  "VALUES", "HAVING", "GROUP", "ORDER", "BY", "LIMIT", "SET", "INT",
  "BEGIN_TRANS", "COMMIT_TRANS", "ROLLBACK_TRANS", "CASE", "WHEN", "THEN",
  "ELSE", "END", "OVER_TOK", "PARTITION", "ROWS", "RANGE", "UNBOUNDED",
  "PRECEDING", "CURRENT", "ROW", "FOLLOWING", "DUMMYEXPR", "JOIN",
  "NATURAL", "LEFT", "RIGHT", "OUTER", "INNER", "CROSS", "ON", "USING",
  "FULL", "TYPE", "TRANSACTION", "WITH", "XOR", "';'", "','", "'.'",
  "$accept", "stmtList", "stmt", "dmlStmt", "queryStmt", "withQuery",
  "withViewList", "withView", "transactionIdentifier", "provStmt",
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
     343,    59,    44,    46
};
# endif

#define YYPACT_NINF -287

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-287)))

#define YYTABLE_NINF -167

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-167)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     149,   110,   -18,   -29,    43,    24,    35,  -287,  -287,  -287,
      85,    14,     9,  -287,   120,  -287,  -287,  -287,  -287,  -287,
    -287,  -287,  -287,   105,    23,   337,   118,    61,  -287,   144,
     106,    54,   121,    25,  -287,  -287,    65,  -287,   122,   110,
     110,  -287,   176,  -287,  -287,  -287,   -11,  -287,  -287,   359,
     359,   238,   -20,  -287,   474,  -287,  -287,  -287,  -287,  -287,
    -287,  -287,   126,   195,   164,    47,    22,   181,   191,    85,
     120,  -287,  -287,   110,  -287,  -287,   359,   359,   201,   194,
     536,   505,   359,   386,    68,  -287,    36,   337,   178,   359,
     359,   359,   359,   359,   359,   359,   359,   359,   209,   206,
     120,  -287,    -7,  -287,   229,   289,   223,   218,   231,   226,
     216,   234,    22,  -287,     0,   110,  -287,  -287,    -8,   516,
      -6,  -287,  -287,   376,    68,   359,  -287,   177,   222,   484,
     153,   467,   222,  -287,  -287,   289,   197,   180,   180,   253,
     253,   253,  -287,   536,   526,   564,  -287,   575,   195,  -287,
     364,   289,   289,   248,   429,   409,  -287,  -287,  -287,  -287,
    -287,  -287,  -287,   149,   270,   147,  -287,   359,   203,   359,
     205,   516,  -287,  -287,   258,   251,   262,  -287,    10,   484,
     204,   467,   272,    36,    36,   475,   200,   207,  -287,   210,
     211,   225,  -287,    10,   409,   242,   241,  -287,    -5,  -287,
    -287,   310,   516,  -287,   415,   363,   409,   110,   266,  -287,
     281,   289,   289,   289,   359,   115,    11,  -287,  -287,   516,
     309,  -287,   516,  -287,   311,   307,   317,   318,   339,   220,
    -287,   443,   467,   405,    36,   259,  -287,  -287,    36,  -287,
      36,   359,   289,   287,  -287,   579,   310,  -287,   291,   310,
    -287,  -287,  -287,   333,   335,    -1,    -1,    67,   454,  -287,
    -287,   277,  -287,   354,   373,   354,   354,  -287,   318,   300,
    -287,    10,   289,   383,  -287,  -287,    36,  -287,   405,   275,
     409,   343,   338,  -287,  -287,  -287,   315,   110,   110,   359,
     345,   287,  -287,    -3,   354,     2,     3,  -287,   409,   354,
    -287,  -287,   583,   320,  -287,  -287,   358,   361,   516,   359,
     102,  -287,   406,     5,  -287,  -287,    12,  -287,  -287,   312,
    -287,  -287,  -287,   275,    74,    74,   395,  -287,  -287,  -287,
     583,   193,   340,   350,   219,  -287,  -287,  -287,  -287,  -287,
    -287,   379,  -287,  -287,  -287,  -287,   193,  -287
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    56,     0,     0,    37,    25,    19,    20,    21,
       0,     0,     0,     4,     5,     7,     6,    13,     9,    10,
       8,    14,    12,     0,    57,     0,     0,     0,    38,     0,
       0,    28,     0,     0,    17,     1,     0,     2,    53,     0,
       0,    11,     0,    75,    76,    77,    78,    79,    63,     0,
       0,     0,   112,    59,    61,    68,    69,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,     0,     0,
      15,     3,    54,     0,    50,    51,     0,     0,     0,    78,
      89,     0,     0,     0,    96,    94,     0,     0,   150,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      45,    78,   150,    40,     0,     0,     0,     0,     0,     0,
       0,     0,    29,    30,     0,     0,    16,    52,     0,    65,
       0,    64,    67,     0,    96,     0,    93,     0,   142,     0,
     113,   114,   142,   120,    60,     0,   168,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    62,     0,     0,    39,
       0,     0,     0,     0,   153,    36,    26,    27,    33,    34,
      35,    32,    31,     0,     0,     0,    58,     0,    98,     0,
       0,    97,    92,   143,     0,     0,     0,   117,   116,     0,
       0,     0,   120,     0,     0,     0,   131,   133,   137,     0,
     136,     0,   119,   118,   151,     0,   170,    47,     0,    46,
      41,     0,    42,    43,   153,     0,   154,     0,     0,   167,
       0,     0,     0,     0,     0,     0,     0,    24,    18,    66,
       0,    90,    95,    91,     0,     0,     0,   148,     0,     0,
     122,   125,   115,     0,     0,     0,   132,   134,     0,   135,
       0,     0,     0,   172,    44,     0,     0,   152,     0,     0,
     164,   163,   165,     0,     0,   155,   156,   157,     0,    23,
      22,   101,    99,     0,   146,     0,     0,   140,   148,   122,
     121,     0,     0,     0,   130,   126,     0,   128,     0,   169,
     171,     0,   174,    48,    49,   162,     0,     0,     0,     0,
       0,   172,   123,     0,     0,     0,     0,   141,   139,     0,
     127,   129,     0,     0,    55,   160,     0,     0,   158,     0,
     103,   144,     0,     0,   145,   149,     0,   178,   176,   173,
     175,   159,   161,   102,     0,     0,     0,   124,   147,   138,
       0,     0,     0,     0,     0,   104,   107,   105,   100,   179,
     177,     0,   108,   109,   110,   111,     0,   106
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -287,   282,     6,  -287,     1,  -287,  -287,   366,  -287,  -287,
    -287,  -287,  -287,   336,  -287,  -287,  -287,  -287,   299,  -287,
    -287,  -287,  -287,  -287,  -287,  -287,   367,   -74,   -25,  -142,
     -62,  -287,  -287,  -287,  -287,  -287,   372,   -66,   332,  -287,
    -287,  -287,  -287,   133,  -286,  -287,  -287,   -82,   322,  -145,
     -70,   293,   198,  -125,  -123,   212,   400,  -124,  -287,  -287,
    -287,  -287,   213,  -287,  -287
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    11,    36,    13,    14,    15,    33,    34,    16,    17,
      31,    67,   112,   113,    18,    29,    19,   102,   103,    20,
     198,    21,    73,    22,    25,    52,    53,   118,   154,    55,
      56,    57,    58,    59,    60,    61,    84,    85,   127,   221,
     262,   291,   326,   335,   336,    88,   130,   181,   132,   293,
     133,   191,   274,   177,   178,   267,   136,   155,   253,   210,
     196,   243,   282,   304,   319
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      54,   104,    23,   120,   131,   199,    12,   192,    77,   193,
     166,   194,   168,   244,    35,   311,   227,    86,   126,   163,
     314,   315,    24,   328,    80,    81,    83,   205,   206,   260,
     329,    26,   135,     1,    70,     2,     3,     4,     5,     6,
      74,    75,   128,   213,     1,   341,     2,   214,   228,    27,
       6,   119,   119,   108,   109,   129,   110,   123,   126,   182,
     347,    28,    54,   100,   137,   138,   139,   140,   141,   142,
     143,   144,   145,    30,   117,   106,   107,    43,    44,    45,
      79,    47,    87,     7,     8,     9,   104,   255,   256,   257,
      49,    32,    78,    50,   167,   148,   167,   245,   164,   312,
     171,   232,   233,   284,   312,   312,   270,   312,   271,   182,
      37,  -167,    37,    10,   312,  -167,   165,    42,   280,   111,
     295,   296,   331,    41,    62,   202,   204,    69,    63,     1,
     180,     2,    65,   259,     1,     6,     2,     3,     4,     5,
       6,    82,   219,   125,   222,     1,    51,     2,   298,   313,
      64,     6,   275,    66,   316,   332,   277,   333,   278,    68,
     317,   320,    38,    39,    40,   218,    71,   279,     1,   216,
       2,     3,     4,     5,     6,    72,    81,    38,    39,    40,
     229,   324,   325,   145,     7,     8,     9,    99,   339,   258,
      91,    92,    93,    94,   300,    76,    43,    44,    45,    79,
      47,   101,   180,   105,    38,    39,    40,   114,   248,    49,
     115,   121,    50,    77,    10,   146,   119,   135,     7,     8,
       9,    81,   230,   157,    81,   147,   156,    89,    90,    91,
      92,    93,    94,    95,    96,   323,    97,   158,   269,   161,
     318,    43,    44,    45,    79,    47,   150,    23,    10,   173,
     286,   160,   174,   172,    49,   183,   175,    50,   176,   159,
     195,    38,    39,    40,   308,    51,    94,   207,   340,    43,
      44,    45,    79,    47,   332,   217,   333,    38,    39,    40,
     220,   223,    49,   224,   119,   249,   225,   226,   306,   307,
     231,   236,    43,    44,    45,    79,    47,   238,   237,   334,
     334,   344,   239,   242,   345,    49,   334,   241,   151,   285,
      51,    82,   240,    43,    44,    45,    79,    47,   -11,   250,
     251,   334,   252,    43,    44,    45,    49,   254,   261,   246,
     263,     2,   264,   305,   152,     6,   265,   266,    51,   153,
      43,    44,    45,    46,    47,   268,   276,    48,    38,    39,
      40,   281,   287,    49,   288,   290,    50,   -11,   -11,   -11,
     292,    51,    43,    44,    45,    79,    47,    43,    44,    45,
      79,    47,    38,    39,    40,    49,   321,   167,    50,   322,
      49,   247,    51,   201,    89,    90,    91,    92,    93,    94,
      95,    96,   294,    97,    89,    90,    91,    92,    93,    94,
      95,    96,   299,    97,   303,   211,   212,   213,   302,    51,
     309,   214,   327,   338,   330,    38,    39,    40,    38,    39,
      40,   346,   342,    89,    90,    91,    92,    93,    94,    95,
      96,    51,   208,   122,   343,   116,    51,    89,    90,    91,
      92,    93,    94,    95,    96,   215,   208,   200,   162,  -142,
     169,   211,   212,   213,   134,   124,   170,   214,   337,    82,
     209,  -166,    89,    90,    91,    92,    93,    94,    95,    96,
     173,    97,   203,   174,   209,  -166,   301,   175,   235,   176,
     297,  -142,    89,    90,    91,    92,    93,    94,    95,    96,
     128,    97,   184,   185,   186,   187,   289,   188,   189,   272,
     273,   190,   149,   179,   310,     2,     0,     0,     0,     6,
       0,     0,    98,    89,    90,    91,    92,    93,    94,    95,
      96,     0,    97,   122,    89,    90,    91,    92,    93,    94,
      95,    96,     0,    97,    89,    90,    91,    92,    93,    94,
      95,     0,     0,    97,    89,    90,    91,    92,    93,    94,
       0,     0,     0,    97,   184,   185,   186,   187,     0,   188,
     189,     0,   234,   190,   186,   187,     0,   188,     0,     0,
       0,   190,    89,    90,    91,    92,    93,    94,    43,    44,
      45,   197,    43,    44,    45,   283,    43,    44,    45,   101
};

static const yytype_int16 yycheck[] =
{
      25,    63,     1,    77,    86,   147,     0,   132,    19,   132,
      18,   135,    18,    18,     0,    18,     6,    37,    84,    19,
      18,    18,    40,    18,    49,    50,    51,   151,   152,    18,
      18,    60,    39,    19,    33,    21,    22,    23,    24,    25,
      39,    40,     6,    44,    19,   331,    21,    48,    38,     6,
      25,    76,    77,    31,    32,    19,    34,    82,   124,   129,
     346,    37,    87,    62,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    38,    73,    28,    29,     3,     4,     5,
       6,     7,   102,    69,    70,    71,   148,   211,   212,   213,
      16,     6,   103,    19,   102,   102,   102,   102,    98,   102,
     125,   183,   184,   245,   102,   102,   231,   102,   231,   179,
     101,    44,   101,    99,   102,    48,   115,    94,   242,    97,
     265,   266,    48,    18,     6,   150,   151,   102,    67,    19,
     129,    21,    26,    18,    19,    25,    21,    22,    23,    24,
      25,    73,   167,    75,   169,    19,    72,    21,   272,   294,
       6,    25,   234,    99,   299,    81,   238,    83,   240,    38,
     302,   303,    57,    58,    59,    18,   101,   241,    19,   163,
      21,    22,    23,    24,    25,    53,   201,    57,    58,    59,
     179,    79,    80,   208,    69,    70,    71,    61,   330,   214,
      10,    11,    12,    13,   276,    19,     3,     4,     5,     6,
       7,     6,   201,    39,    57,    58,    59,    26,   207,    16,
      19,    10,    19,    19,    99,     6,   241,    39,    69,    70,
      71,   246,    18,     5,   249,    19,     3,     8,     9,    10,
      11,    12,    13,    14,    15,   309,    17,     6,    18,     5,
     302,     3,     4,     5,     6,     7,    17,   246,    99,    27,
     249,    35,    30,    76,    16,   102,    34,    19,    36,    33,
      63,    57,    58,    59,   289,    72,    13,    19,   330,     3,
       4,     5,     6,     7,    81,     5,    83,    57,    58,    59,
      77,    76,    16,    25,   309,    19,    35,    25,   287,   288,
      18,    91,     3,     4,     5,     6,     7,    87,    91,   324,
     325,    82,    91,    62,    85,    16,   331,    65,    19,    18,
      72,    73,    87,     3,     4,     5,     6,     7,    18,    53,
      54,   346,    56,     3,     4,     5,    16,    46,    19,    19,
      19,    21,    25,    18,    45,    25,    19,    19,    72,    50,
       3,     4,     5,     6,     7,     6,    87,    10,    57,    58,
      59,    64,    19,    16,    19,    78,    19,    57,    58,    59,
       6,    72,     3,     4,     5,     6,     7,     3,     4,     5,
       6,     7,    57,    58,    59,    16,    18,   102,    19,    18,
      16,    18,    72,    19,     8,     9,    10,    11,    12,    13,
      14,    15,    19,    17,     8,     9,    10,    11,    12,    13,
      14,    15,    19,    17,    66,    42,    43,    44,    65,    72,
      65,    48,     6,    18,   102,    57,    58,    59,    57,    58,
      59,    42,    82,     8,     9,    10,    11,    12,    13,    14,
      15,    72,    17,    18,    84,    69,    72,     8,     9,    10,
      11,    12,    13,    14,    15,   163,    17,   148,   112,     6,
      74,    42,    43,    44,    87,    83,   124,    48,   325,    73,
      45,    46,     8,     9,    10,    11,    12,    13,    14,    15,
      27,    17,   150,    30,    45,    46,   278,    34,   185,    36,
     268,    38,     8,     9,    10,    11,    12,    13,    14,    15,
       6,    17,    87,    88,    89,    90,    42,    92,    93,    94,
      95,    96,   102,    19,   291,    21,    -1,    -1,    -1,    25,
      -1,    -1,    38,     8,     9,    10,    11,    12,    13,    14,
      15,    -1,    17,    18,     8,     9,    10,    11,    12,    13,
      14,    15,    -1,    17,     8,     9,    10,    11,    12,    13,
      14,    -1,    -1,    17,     8,     9,    10,    11,    12,    13,
      -1,    -1,    -1,    17,    87,    88,    89,    90,    -1,    92,
      93,    -1,    87,    96,    89,    90,    -1,    92,    -1,    -1,
      -1,    96,     8,     9,    10,    11,    12,    13,     3,     4,
       5,     6,     3,     4,     5,     6,     3,     4,     5,     6
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    19,    21,    22,    23,    24,    25,    69,    70,    71,
      99,   105,   106,   107,   108,   109,   112,   113,   118,   120,
     123,   125,   127,   108,    40,   128,    60,     6,    37,   119,
      38,   114,     6,   110,   111,     0,   106,   101,    57,    58,
      59,    18,    94,     3,     4,     5,     6,     7,    10,    16,
      19,    72,   129,   130,   132,   133,   134,   135,   136,   137,
     138,   139,     6,    67,     6,    26,    99,   115,    38,   102,
     108,   101,    53,   126,   108,   108,    19,    19,   103,     6,
     132,   132,    73,   132,   140,   141,    37,   102,   149,     8,
       9,    10,    11,    12,    13,    14,    15,    17,    38,    61,
     108,     6,   121,   122,   134,    39,    28,    29,    31,    32,
      34,    97,   116,   117,    26,    19,   111,   108,   131,   132,
     131,    10,    18,   132,   140,    75,   141,   142,     6,    19,
     150,   151,   152,   154,   130,    39,   160,   132,   132,   132,
     132,   132,   132,   132,   132,   132,     6,    19,   102,   160,
      17,    19,    45,    50,   132,   161,     3,     5,     6,    33,
      35,     5,   117,    19,    98,   108,    18,   102,    18,    74,
     142,   132,    76,    27,    30,    34,    36,   157,   158,    19,
     108,   151,   154,   102,    87,    88,    89,    90,    92,    93,
      96,   155,   157,   158,   161,    63,   164,     6,   124,   133,
     122,    19,   132,   152,   132,   161,   161,    19,    17,    45,
     163,    42,    43,    44,    48,   105,   106,     5,    18,   132,
      77,   143,   132,    76,    25,    35,    25,     6,    38,   108,
      18,    18,   151,   151,    87,   155,    91,    91,    87,    91,
      87,    65,    62,   165,    18,   102,    19,    18,   108,    19,
      53,    54,    56,   162,    46,   161,   161,   161,   132,    18,
      18,    19,   144,    19,    25,    19,    19,   159,     6,    18,
     157,   158,    94,    95,   156,   151,    87,   151,   151,   131,
     161,    64,   166,     6,   133,    18,   108,    19,    19,    42,
      78,   145,     6,   153,    19,   153,   153,   159,   161,    19,
     151,   156,    65,    66,   167,    18,   108,   108,   132,    65,
     166,    18,   102,   153,    18,    18,   153,   133,   134,   168,
     133,    18,    18,   131,    79,    80,   146,     6,    18,    18,
     102,    48,    81,    83,   132,   147,   148,   147,    18,   133,
     134,   148,    82,    84,    82,    85,    42,   148
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   104,   105,   105,   106,   106,   106,   106,   107,   107,
     107,   108,   108,   108,   108,   109,   110,   110,   111,   112,
     112,   112,   113,   113,   113,   114,   114,   114,   115,   115,
     116,   116,   117,   117,   117,   117,   118,   119,   119,   120,
     121,   121,   122,   122,   123,   123,   124,   124,   124,   124,
     125,   125,   125,   126,   126,   127,   128,   128,   128,   129,
     129,   130,   130,   130,   130,   131,   131,   132,   132,   132,
     132,   132,   132,   132,   132,   133,   133,   133,   134,   135,
     136,   136,   136,   136,   136,   136,   136,   136,   136,   137,
     138,   139,   139,   140,   140,   141,   142,   142,   143,   143,
     144,   145,   145,   146,   146,   146,   147,   147,   148,   148,
     148,   148,   149,   149,   150,   150,   151,   151,   151,   151,
     151,   151,   152,   153,   153,   154,   154,   154,   154,   154,
     154,   155,   155,   155,   155,   155,   155,   155,   156,   156,
     157,   157,   158,   158,   158,   158,   158,   158,   159,   159,
     160,   160,   161,   161,   161,   161,   161,   161,   161,   161,
     161,   161,   161,   162,   162,   162,   163,   163,   164,   164,
     165,   165,   166,   166,   167,   167,   168,   168,   168,   168
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     3,     1,     1,     1,     1,     1,     1,
       1,     3,     1,     1,     1,     3,     3,     1,     5,     1,
       1,     1,     7,     7,     6,     0,     4,     4,     0,     2,
       1,     2,     2,     2,     2,     2,     5,     0,     1,     5,
       1,     3,     3,     3,     7,     4,     1,     1,     3,     3,
       3,     3,     4,     0,     1,     9,     0,     1,     5,     1,
       3,     1,     3,     1,     3,     1,     3,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       5,     5,     4,     2,     1,     4,     0,     2,     0,     2,
       5,     0,     3,     0,     2,     2,     4,     1,     2,     2,
       2,     2,     0,     2,     1,     3,     2,     2,     2,     2,
       1,     4,     3,     1,     3,     3,     4,     5,     4,     5,
       4,     1,     2,     1,     2,     2,     1,     1,     4,     2,
       3,     4,     0,     1,     5,     5,     3,     6,     0,     3,
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
#line 132 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("stmtList::stmt"); 
				(yyval.list) = singleton((yyvsp[-1].node));
				bisonParseResult = (Node *) (yyval.list);	 
			}
#line 1639 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 138 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("stmtlist::stmtList::stmt");
				(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[-1].node));	
				bisonParseResult = (Node *) (yyval.list); 
			}
#line 1649 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 147 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::dmlStmt");
            (yyval.node) = (yyvsp[0].node);
        }
#line 1658 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 152 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::queryStmt");
            (yyval.node) = (yyvsp[0].node);
        }
#line 1667 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 157 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::transactionIdentifier");
            (yyval.node) = (Node *) createTransactionStmt((yyvsp[0].stringVal));
        }
#line 1676 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 162 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("stmt::withQuery"); 
			(yyval.node) = (yyvsp[0].node); 
		}
#line 1685 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 172 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::insertQuery"); }
#line 1691 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 173 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::deleteQuery"); }
#line 1697 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 174 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::updateQuery"); }
#line 1703 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 181 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::bracketedQuery"); (yyval.node) = (yyvsp[-1].node); }
#line 1709 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 182 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::selectQuery"); }
#line 1715 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 183 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::provStmt"); }
#line 1721 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 184 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::setOperatorQuery"); }
#line 1727 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 189 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withQuery::withViewList::queryStmt");
			(yyval.node) = (Node *) createWithStmt((yyvsp[-1].list), (yyvsp[0].node));
		}
#line 1736 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 197 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withViewList::list::view");
			(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
		}
#line 1745 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 202 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withViewList::view");
			(yyval.list) = singleton((yyvsp[0].node));
		}
#line 1754 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 210 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withView::ident::AS:queryStmt");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString((yyvsp[-4].stringVal)), (yyvsp[-1].node));
		}
#line 1763 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 217 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::BEGIN"); (yyval.stringVal) = strdup("TRANSACTION_BEGIN"); }
#line 1769 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 218 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::COMMIT"); (yyval.stringVal) = strdup("TRANSACTION_COMMIT"); }
#line 1775 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 219 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::ROLLBACK"); (yyval.stringVal) = strdup("TRANSACTION_ABORT"); }
#line 1781 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 227 "sql_parser.y" /* yacc.c:1646  */
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
#line 1796 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 238 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provStmt::stmtlist");
			ProvenanceStmt *p = createProvenanceStmt((Node *) (yyvsp[-1].list));
			p->inputType = PROV_INPUT_UPDATE_SEQUENCE;
			p->provType = PROV_PI_CS;
			p->asOf = (Node *) (yyvsp[-5].node);
			p->options = (yyvsp[-4].list);
			(yyval.node) = (Node *) p;
		}
#line 1810 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 248 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provStmt::transaction");
			ProvenanceStmt *p = createProvenanceStmt((Node *) createConstString((yyvsp[0].stringVal)));
			p->inputType = PROV_INPUT_TRANSACTION;
			p->provType = PROV_PI_CS;
			p->options = (yyvsp[-3].list);
			(yyval.node) = (Node *) p;
		}
#line 1823 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 259 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalProvAsOf::EMPTY"); (yyval.node) = NULL; }
#line 1829 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 261 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvAsOf::SCN");
			(yyval.node) = (Node *) createConstLong((yyvsp[0].intVal));
		}
#line 1838 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 266 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvAsOf::TIMESTAMP");
			(yyval.node) = (Node *) createConstString((yyvsp[0].stringVal));
		}
#line 1847 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 273 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalProvWith::EMPTY"); (yyval.list) = NIL; }
#line 1853 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 275 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvWith::WITH");
			(yyval.list) = (yyvsp[0].list);
		}
#line 1862 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 282 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("provOptionList::option"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 1868 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 284 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("provOptionList::list"); 
			(yyval.list) = appendToTailOfList((yyvsp[-1].list),(yyvsp[0].node)); 
		}
#line 1877 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 292 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("provOption::TYPE"); 
			(yyval.node) = (Node *) createStringKeyValue("TYPE", (yyvsp[0].stringVal)); 
		}
#line 1886 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 297 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::TABLE");
			(yyval.node) = (Node *) createStringKeyValue("TABLE", (yyvsp[0].stringVal));
		}
#line 1895 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 302 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::ONLY::UPDATED");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString("ONLY UPDATED"), 
					(Node *) createConstBool(TRUE));
		}
#line 1905 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 308 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::SHOW::INTERMEDIATE");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString("SHOW ALL INTERMEDIATE"), 
					(Node *) createConstBool(TRUE));
		}
#line 1915 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 320 "sql_parser.y" /* yacc.c:1646  */
    { 
             RULELOG("deleteQuery");
             (yyval.node) = (Node *) createDelete((yyvsp[-2].stringVal), (yyvsp[0].node));
         }
#line 1924 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 328 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::NULL"); (yyval.stringVal) = NULL; }
#line 1930 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 329 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::FROM"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 1936 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 344 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG(updateQuery); 
                (yyval.node) = (Node *) createUpdate((yyvsp[-3].stringVal), (yyvsp[-1].list), (yyvsp[0].node)); 
            }
#line 1945 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 352 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setExpression");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 1954 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 357 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setClause::setExpression");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 1963 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 365 "sql_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal),"=")) {
                    RULELOG("setExpression::attributeRef::expression");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 1976 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 374 "sql_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal), "=")) {
                    RULELOG("setExpression::attributeRef::queryStmt");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 1989 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 389 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertQuery::insertList"); 
            	(yyval.node) = (Node *) createInsert((yyvsp[-4].stringVal),(Node *) (yyvsp[-1].list), NULL); 
        	}
#line 1998 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 394 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("insertQuery::queryStmt");
                (yyval.node) = (Node *) createInsert((yyvsp[-1].stringVal), (yyvsp[0].node), NULL);
            }
#line 2007 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 402 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::constant");
            	(yyval.list) = singleton((yyvsp[0].node)); 
            }
#line 2016 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 407 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("insertList::IDENTIFIER");
                (yyval.list) = singleton(createAttributeReference((yyvsp[0].stringVal)));
            }
#line 2025 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 412 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("insertList::insertList::::IDENTIFIER");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), createAttributeReference((yyvsp[0].stringVal)));
            }
#line 2034 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 417 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::insertList::constant");
            	(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2043 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 431 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::INTERSECT");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 2052 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 436 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::MINUS");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 2061 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 441 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::UNION");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-2].stringVal), ((yyvsp[-1].stringVal) != NULL), (yyvsp[-3].node), (yyvsp[0].node));
            }
#line 2070 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 448 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::NULL"); (yyval.stringVal) = NULL; }
#line 2076 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 449 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::ALLTRUE"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2082 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 459 "sql_parser.y" /* yacc.c:1646  */
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
#line 2102 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 481 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalDistinct::NULL"); (yyval.node) = NULL; }
#line 2108 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 483 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT");
                (yyval.node) = (Node *) createDistinctClause(NULL);
            }
#line 2117 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 488 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT::exprList");
                (yyval.node) = (Node *) createDistinctClause((yyvsp[-1].list));
            }
#line 2126 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 500 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectItem"); (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2134 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 504 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectClause::selectItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node)); 
            }
#line 2143 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 512 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression"); 
                 (yyval.node) = (Node *) createSelectItem(NULL, (yyvsp[0].node)); 
             }
#line 2152 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 517 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression::identifier"); 
                 (yyval.node) = (Node *) createSelectItem((yyvsp[0].stringVal), (yyvsp[-2].node));
             }
#line 2161 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 522 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
         		(yyval.node) = (Node *) createSelectItem(strdup("*"), NULL); 
     		}
#line 2170 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 527 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
     			(yyval.node) = (Node *) createSelectItem(CONCAT_STRINGS((yyvsp[-2].stringVal),".*"), NULL); 
 			}
#line 2179 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 537 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("exprList::SINGLETON"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 2185 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 539 "sql_parser.y" /* yacc.c:1646  */
    {
                  RULELOG("exprList::exprList::expression");
                  (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
             }
#line 2194 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 550 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::bracked"); (yyval.node) = (yyvsp[-1].node); }
#line 2200 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 551 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::constant"); }
#line 2206 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 552 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::attributeRef"); }
#line 2212 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 553 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlParameter"); }
#line 2218 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 554 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::binaryOperatorExpression"); }
#line 2224 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 555 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::unaryOperatorExpression"); }
#line 2230 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 556 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlFunctionCall"); }
#line 2236 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 557 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::case"); }
#line 2242 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 566 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::INT"); (yyval.node) = (Node *) createConstInt((yyvsp[0].intVal)); }
#line 2248 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 567 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::FLOAT"); (yyval.node) = (Node *) createConstFloat((yyvsp[0].floatVal)); }
#line 2254 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 568 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::STRING"); (yyval.node) = (Node *) createConstString((yyvsp[0].stringVal)); }
#line 2260 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 575 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("attributeRef::IDENTIFIER"); (yyval.node) = (Node *) createAttributeReference((yyvsp[0].stringVal)); }
#line 2266 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 581 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("sqlParameter::PARAMETER"); (yyval.node) = (Node *) createSQLParameter((yyvsp[0].stringVal)); }
#line 2272 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 601 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '+' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2283 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 608 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '-' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2294 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 615 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '*' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2305 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 622 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '/' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2316 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 629 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '%' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2327 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 636 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '^' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2338 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 645 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '&' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2349 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 652 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '|' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2360 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 661 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression::comparisonOps");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2371 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 671 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("unaryOperatorExpression:: '!' ");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2381 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 683 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("sqlFunctionCall::IDENTIFIER::exprList");
				FunctionCall *f = createFunctionCall((yyvsp[-4].stringVal), (yyvsp[-2].list));
				if ((yyvsp[0].node) != NULL)
					(yyval.node) = (Node *) createWindowFunction(f, (WindowDef *) (yyvsp[0].node));
				else  
                	(yyval.node) = (Node *) f; 
            }
#line 2394 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 698 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseExpression::CASE::expression::whens:else:END");
				(yyval.node) = (Node *) createCaseExpr((yyvsp[-3].node), (yyvsp[-2].list), (yyvsp[-1].node));
			}
#line 2403 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 703 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseExpression::CASE::whens::else::END");
				(yyval.node) = (Node *) createCaseExpr(NULL, (yyvsp[-2].list), (yyvsp[-1].node));
			}
#line 2412 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 711 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhenList::list::caseWhen");
				(yyval.list) = appendToTailOfList((yyvsp[-1].list), (yyvsp[0].node));
			}
#line 2421 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 716 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhenList::caseWhen");
				(yyval.list) = singleton((yyvsp[0].node));
			}
#line 2430 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 724 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhen::WHEN::expression::THEN::expression");
				(yyval.node) = (Node *) createCaseWhen((yyvsp[-2].node),(yyvsp[0].node));
			}
#line 2439 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 731 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalCaseElse::NULL"); (yyval.node) = NULL; }
#line 2445 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 733 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalCaseElse::ELSE::expression");
				(yyval.node) = (yyvsp[0].node);
			}
#line 2454 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 743 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("overclause::NULL"); (yyval.node) = NULL; }
#line 2460 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 745 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("overclause::window");
				(yyval.node) = (yyvsp[0].node);
			}
#line 2469 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 753 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("window");
				(yyval.node) = (Node *) createWindowDef((yyvsp[-3].list),(yyvsp[-2].list), (WindowFrame *) (yyvsp[-1].node));
			}
#line 2478 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 760 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optWindowPart::NULL"); (yyval.list) = NIL; }
#line 2484 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 762 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optWindowPart::PARTITION:BY::expressionList");
				(yyval.list) = (yyvsp[0].list);
			}
#line 2493 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 769 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optWindowFrame::NULL"); (yyval.node) = NULL; }
#line 2499 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 771 "sql_parser.y" /* yacc.c:1646  */
    { 
				WindowBound *l, *u = NULL;
				RULELOG("optWindowFrame::ROWS::windoBoundaries");
				l = getNthOfListP((yyvsp[0].list), 0);
				if(LIST_LENGTH((yyvsp[0].list)) > 1)
					u = getNthOfListP((yyvsp[0].list), 1);
				(yyval.node) = (Node *) createWindowFrame(WINFRAME_ROWS, l, u); 
			}
#line 2512 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 780 "sql_parser.y" /* yacc.c:1646  */
    {
				WindowBound *l, *u = NULL; 
				RULELOG("optWindowFrame::RANGE::windoBoundaries"); 
				l = getNthOfListP((yyvsp[0].list), 0);
				if(LIST_LENGTH((yyvsp[0].list)) > 1)
					u = getNthOfListP((yyvsp[0].list), 1);
				(yyval.node) = (Node *) createWindowFrame(WINFRAME_RANGE, l, u); 
			}
#line 2525 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 792 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBoundaries::BETWEEN"); 
				(yyval.list) = LIST_MAKE((yyvsp[-2].node), (yyvsp[0].node)); 
			}
#line 2534 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 797 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBoundaries::windowBound"); 
				(yyval.list) = singleton((yyvsp[0].node)); 
			}
#line 2543 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 805 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::UNBOUNDED::PRECEDING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_UNBOUND_PREC, NULL); 
			}
#line 2552 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 810 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::CURRENT::ROW"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_CURRENT_ROW, NULL); 
			}
#line 2561 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 815 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::expression::PRECEDING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_EXPR_PREC, (yyvsp[-1].node)); 
			}
#line 2570 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 820 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::expression::FOLLOWING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_EXPR_FOLLOW, (yyvsp[-1].node)); 
			}
#line 2579 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 832 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::NULL"); (yyval.list) = NULL; }
#line 2585 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 833 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::fromClause"); (yyval.list) = (yyvsp[0].list); }
#line 2591 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 838 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClauseItem");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2600 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 843 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClause::fromClauseItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2609 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 852 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
				FromItem *f = createFromTableRef(NULL, NIL, (yyvsp[-1].stringVal));
				f->provInfo = (FromProvInfo *) (yyvsp[0].node);
                (yyval.node) = (Node *) f;
            }
#line 2620 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 859 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
                FromItem *f = createFromTableRef(((FromItem *) (yyvsp[0].node))->name, 
						((FromItem *) (yyvsp[0].node))->attrNames, (yyvsp[-1].stringVal));
				f->provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
                (yyval.node) = (Node *) f;
            }
#line 2632 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 868 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromItem *f = (FromItem *) (yyvsp[-1].node);
                f->provInfo = (FromProvInfo *) (yyvsp[0].node);
                (yyval.node) = (yyvsp[-1].node);
            }
#line 2643 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 875 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromSubquery *s = (FromSubquery *) (yyvsp[-1].node);
                s->from.name = ((FromItem *) (yyvsp[0].node))->name;
                s->from.attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
                s->from.provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
                (yyval.node) = (Node *) s;
            }
#line 2656 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 884 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[0].node);
        		f->name = NULL;
        		(yyval.node) = (Node *) f;
        	}
#line 2668 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 892 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[-2].node);
        		f->name = ((FromItem *) (yyvsp[0].node))->name;
                f->attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
                f->provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
        		(yyval.node) = (Node *) f;
        	}
#line 2682 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 905 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("subQuery::queryStmt");
                (yyval.node) = (Node *) createFromSubquery(NULL, NULL, (yyvsp[-1].node));
            }
#line 2691 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 912 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = singleton((yyvsp[0].stringVal)); }
#line 2697 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 913 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].stringVal)); }
#line 2703 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 917 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2709 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 919 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURAL");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
						(FromItem *) (yyvsp[0].node), "JOIN_INNER", "JOIN_COND_NATURAL", 
						NULL);
          	}
#line 2720 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 926 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURALjoinType");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[0].node), (yyvsp[-2].stringVal), "JOIN_COND_NATURAL", NULL);
          	}
#line 2730 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 932 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::CROSS JOIN");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[0].node), "JOIN_CROSS", "JOIN_COND_ON", NULL);
          	}
#line 2740 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 938 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinType::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON";
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[-1].node), (yyvsp[-3].stringVal), condType, (yyvsp[0].node));
          	}
#line 2752 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 946 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON"; 
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[-1].node), "JOIN_INNER", 
                		condType, (yyvsp[0].node));
          	}
#line 2765 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 957 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2771 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 958 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT OUTER"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2777 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 959 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT "); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2783 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 960 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT OUTER"); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2789 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 961 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL OUTER"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2795 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 962 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2801 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 963 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::INNER"); (yyval.stringVal) = "JOIN_INNER"; }
#line 2807 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 967 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (Node *) (yyvsp[-1].list); }
#line 2813 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 968 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2819 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 973 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[-2].node);
				(yyval.node) = (Node *) f;
			}
#line 2830 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 980 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[-3].node); 
				(yyval.node) = (Node *) f;
			}
#line 2841 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 989 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFromProv::empty"); (yyval.node) = NULL; }
#line 2847 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 991 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::BASERELATION");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = TRUE;
				p->userProvAttrs = NIL;				 
				(yyval.node) = (Node *) p; 
			}
#line 2859 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 999 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::userProvAttr");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = FALSE;
				p->userProvAttrs = (yyvsp[-1].list);				 
				(yyval.node) = (Node *) p; 
			}
#line 2871 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1007 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::userProvDupAttr");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = TRUE;
				p->userProvAttrs = (yyvsp[-1].list);
				(yyval.node) = (Node *) p;
			}
#line 2883 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1015 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::intermediateProv");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->intermediateProv = TRUE;
				(yyval.node) = (Node *) p;
			}
#line 2894 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1022 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::intermediateProv::attrList");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->intermediateProv = TRUE;
				p->userProvAttrs = (yyvsp[-1].list);
				(yyval.node) = (Node *) p;
			}
#line 2906 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1032 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAttrAlias::empty"); (yyval.list) = NULL; }
#line 2912 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1034 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAttrAlias::identifierList"); (yyval.list) = (yyvsp[-1].list); 
			}
#line 2920 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1043 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::NULL"); (yyval.node) = NULL; }
#line 2926 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1044 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::whereExpression"); (yyval.node) = (yyvsp[0].node); }
#line 2932 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1048 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("where::brackedWhereExpression"); (yyval.node) = (yyvsp[-1].node); }
#line 2938 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1049 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("whereExpression::expression"); (yyval.node) = (yyvsp[0].node); }
#line 2944 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1051 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::NOT");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2954 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1057 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2965 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 1064 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2976 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 1071 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2987 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1078 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::BETWEEN-AND");
                List *expr = singleton((yyvsp[-4].node));
                expr = appendToTailOfList(expr, (yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr);
            }
#line 2999 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1086 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::comparisonOps::nestedSubQueryOperator::Subquery");
                (yyval.node) = (Node *) createNestedSubquery((yyvsp[-3].stringVal), (yyvsp[-5].node), (yyvsp[-4].stringVal), (yyvsp[-1].node));
            }
#line 3008 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 1091 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::Subquery");
                Node *q = (Node *) createNestedSubquery("SCALAR", NULL, NULL, (yyvsp[-1].node)); 
                List *expr = LIST_MAKE((yyvsp[-4].node), q);
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr); 
            }
#line 3019 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1098 "sql_parser.y" /* yacc.c:1646  */
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
#line 3036 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1111 "sql_parser.y" /* yacc.c:1646  */
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
#line 3053 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1126 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ANY"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 3059 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 1127 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ALL"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 3065 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 1128 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::SOME"); (yyval.stringVal) = "ANY"; }
#line 3071 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 1132 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NULL"); (yyval.stringVal) = NULL; }
#line 3077 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1133 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NOT"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 3083 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 1137 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::NULL"); (yyval.list) = NULL; }
#line 3089 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 1138 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::GROUPBY"); (yyval.list) = (yyvsp[0].list); }
#line 3095 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 1142 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.node) = NULL; }
#line 3101 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 1144 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("optionalHaving::HAVING"); 
                (yyval.node) = (Node *) (yyvsp[0].node);
            }
#line 3110 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 1151 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.list) = NULL; }
#line 3116 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 1152 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy::ORDERBY"); (yyval.list) = (yyvsp[0].list); }
#line 3122 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 1156 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::NULL"); (yyval.node) = NULL; }
#line 3128 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 1157 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::CONSTANT"); (yyval.node) = (yyvsp[0].node);}
#line 3134 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 1162 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::attributeRef");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 3143 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 1167 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 3152 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 1172 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::constant");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 3161 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 1177 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 3170 "sql_parser.tab.c" /* yacc.c:1646  */
    break;


#line 3174 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 1184 "sql_parser.y" /* yacc.c:1906  */




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

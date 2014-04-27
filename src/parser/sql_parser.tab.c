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
#define YYLAST   569

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  104
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  65
/* YYNRULES -- Number of rules.  */
#define YYNRULES  178
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  346

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
     282,   283,   291,   296,   301,   313,   322,   323,   337,   345,
     350,   358,   367,   382,   387,   395,   400,   405,   410,   424,
     429,   434,   442,   443,   452,   475,   476,   481,   493,   497,
     505,   510,   515,   520,   531,   532,   544,   545,   546,   547,
     548,   549,   550,   551,   560,   561,   562,   569,   575,   594,
     601,   608,   615,   622,   629,   638,   645,   654,   664,   676,
     691,   696,   704,   709,   717,   725,   726,   737,   738,   746,
     754,   755,   763,   764,   773,   785,   790,   798,   803,   808,
     813,   826,   827,   831,   836,   845,   852,   861,   868,   877,
     885,   898,   906,   907,   911,   912,   919,   925,   931,   939,
     951,   952,   953,   954,   955,   956,   957,   961,   962,   966,
     973,   983,   984,   992,  1000,  1008,  1015,  1026,  1027,  1037,
    1038,  1042,  1043,  1044,  1050,  1057,  1064,  1071,  1079,  1084,
    1091,  1104,  1120,  1121,  1122,  1126,  1127,  1131,  1132,  1136,
    1137,  1145,  1146,  1150,  1151,  1155,  1160,  1165,  1170
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

#define YYPACT_NINF -314

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-314)))

#define YYTABLE_NINF -166

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-166)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     161,   184,    13,   -24,    44,    24,    83,  -314,  -314,  -314,
      89,    23,    35,  -314,   215,  -314,  -314,  -314,  -314,  -314,
    -314,  -314,  -314,    -3,    51,   335,   132,    92,  -314,   164,
     153,    95,   151,    18,  -314,  -314,    96,  -314,   165,   184,
     184,  -314,   188,  -314,  -314,  -314,    -7,  -314,  -314,   343,
     343,   264,   -18,  -314,   464,  -314,  -314,  -314,  -314,  -314,
    -314,  -314,   116,   222,   190,    90,   -14,   208,   216,    89,
     215,  -314,  -314,   184,  -314,  -314,   343,   343,   227,   224,
     506,   474,   343,   383,   -40,  -314,    54,   335,   207,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   255,   228,
     215,  -314,    10,  -314,   247,   272,   262,   276,   278,   252,
     284,   -14,  -314,    -9,   184,  -314,  -314,    -5,   486,    -4,
    -314,  -314,   373,   -40,   343,  -314,   214,   168,   238,   197,
     437,   168,  -314,  -314,   272,   237,   283,   283,   289,   289,
     289,  -314,   506,   496,   534,  -314,   545,   222,  -314,   360,
     272,   272,   291,   425,   110,  -314,  -314,  -314,  -314,  -314,
    -314,   161,   311,    91,  -314,   343,   243,   343,   249,   486,
    -314,  -314,   303,   296,   307,  -314,    52,   238,   105,   437,
     316,    54,    54,   445,   244,   261,  -314,   256,   266,   273,
    -314,    52,   110,   288,   299,  -314,     2,  -314,  -314,   308,
     486,  -314,   412,   148,   110,   184,   302,  -314,   322,   272,
     272,   272,   343,    57,   -10,  -314,  -314,   486,   370,  -314,
     486,  -314,   382,   348,   390,   395,   369,   154,  -314,   372,
     437,   356,    54,   332,  -314,  -314,    54,  -314,    54,   343,
     272,   364,  -314,   549,   308,  -314,   158,   308,  -314,  -314,
    -314,   422,   434,   123,   123,   125,   451,  -314,  -314,   353,
    -314,   448,   436,   448,   448,  -314,   395,   183,  -314,    52,
     272,   450,  -314,  -314,    54,  -314,   356,   365,   110,   415,
     424,  -314,  -314,  -314,   312,   184,   184,   343,   446,   364,
    -314,     3,   448,     4,    11,  -314,   110,   448,  -314,  -314,
     553,   400,  -314,  -314,   354,   359,   486,   343,   141,  -314,
     514,    12,  -314,  -314,    14,  -314,  -314,   410,  -314,  -314,
    -314,   365,   127,   127,   503,  -314,  -314,  -314,   553,   220,
     440,   444,   241,  -314,  -314,  -314,  -314,  -314,  -314,   489,
    -314,  -314,  -314,  -314,   220,  -314
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    55,     0,     0,    36,    25,    19,    20,    21,
       0,     0,     0,     4,     5,     7,     6,    13,     9,    10,
       8,    14,    12,     0,    56,     0,     0,     0,    37,     0,
       0,    28,     0,     0,    17,     1,     0,     2,    52,     0,
       0,    11,     0,    74,    75,    76,    77,    78,    62,     0,
       0,     0,   111,    58,    60,    67,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,     0,     0,
      15,     3,    53,     0,    49,    50,     0,     0,     0,    77,
      88,     0,     0,     0,    95,    93,     0,     0,   149,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      44,    77,   149,    39,     0,     0,     0,     0,     0,     0,
       0,    29,    30,     0,     0,    16,    51,     0,    64,     0,
      63,    66,     0,    95,     0,    92,     0,   141,     0,   112,
     113,   141,   119,    59,     0,   167,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    61,     0,     0,    38,     0,
       0,     0,     0,   152,    35,    26,    27,    33,    34,    32,
      31,     0,     0,     0,    57,     0,    97,     0,     0,    96,
      91,   142,     0,     0,     0,   116,   115,     0,     0,     0,
     119,     0,     0,     0,   130,   132,   136,     0,   135,     0,
     118,   117,   150,     0,   169,    46,     0,    45,    40,     0,
      41,    42,   152,     0,   153,     0,     0,   166,     0,     0,
       0,     0,     0,     0,     0,    24,    18,    65,     0,    89,
      94,    90,     0,     0,     0,   147,     0,     0,   121,   124,
     114,     0,     0,     0,   131,   133,     0,   134,     0,     0,
       0,   171,    43,     0,     0,   151,     0,     0,   163,   162,
     164,     0,     0,   154,   155,   156,     0,    23,    22,   100,
      98,     0,   145,     0,     0,   139,   147,   121,   120,     0,
       0,     0,   129,   125,     0,   127,     0,   168,   170,     0,
     173,    47,    48,   161,     0,     0,     0,     0,     0,   171,
     122,     0,     0,     0,     0,   140,   138,     0,   126,   128,
       0,     0,    54,   159,     0,     0,   157,     0,   102,   143,
       0,     0,   144,   148,     0,   177,   175,   172,   174,   158,
     160,   101,     0,     0,     0,   123,   146,   137,     0,     0,
       0,     0,     0,   103,   106,   104,    99,   178,   176,     0,
     107,   108,   109,   110,     0,   105
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -314,   375,     7,  -314,     1,  -314,  -314,   469,  -314,  -314,
    -314,  -314,  -314,   428,  -314,  -314,  -314,  -314,   393,  -314,
    -314,  -314,  -314,  -314,  -314,  -314,   473,   -74,   -25,  -140,
     -62,  -314,  -314,  -314,  -314,  -314,   478,   -46,   439,  -314,
    -314,  -314,  -314,   240,  -313,  -314,  -314,   -81,   416,  -153,
     -69,   381,   290,  -127,  -122,   301,   466,  -123,  -314,  -314,
    -314,  -314,   280,  -314,  -314
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    11,    36,    13,    14,    15,    33,    34,    16,    17,
      31,    67,   111,   112,    18,    29,    19,   102,   103,    20,
     196,    21,    73,    22,    25,    52,    53,   117,   153,    55,
      56,    57,    58,    59,    60,    61,    84,    85,   126,   219,
     260,   289,   324,   333,   334,    88,   129,   179,   131,   291,
     132,   189,   272,   175,   176,   265,   135,   154,   251,   208,
     194,   241,   280,   302,   317
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      54,   104,    23,   119,   190,   130,   197,    12,   258,   191,
     161,   192,    77,   164,   166,    41,   339,   108,   109,    86,
     242,   309,   312,    35,    80,    81,    83,   203,   204,   313,
     326,   345,   327,    82,    70,   124,    26,     1,   125,     2,
      74,    75,     1,     6,     2,     3,     4,     5,     6,   134,
      27,   118,   118,    24,    38,    39,    40,   122,   225,   180,
     127,    28,    54,   100,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   128,   116,   257,     1,   125,     2,     3,
       4,     5,     6,   110,    87,   104,   253,   254,   255,   162,
     226,    37,     7,     8,     9,    32,    78,   165,   165,   169,
     230,   231,   268,   282,   243,   310,   310,   269,   180,   216,
     293,   294,   147,   310,   310,   163,   310,   278,   106,   107,
      69,    30,    10,   228,   200,   202,     7,     8,     9,   178,
      43,    44,    45,    79,    47,     1,    37,     2,    62,   311,
     217,     6,   220,    49,   314,    42,    50,   296,    38,    39,
      40,   273,   209,   210,   211,   275,    10,   276,   212,    63,
     315,   318,    38,    39,    40,   277,   245,   211,   214,  -166,
      64,   212,   267,  -166,    81,   329,   283,    99,   227,    65,
       1,   144,     2,     3,     4,     5,     6,   256,   337,    68,
     209,   210,   211,   298,    66,   171,   212,    71,   172,    51,
     178,   -11,   173,     1,   174,     2,   246,    76,   330,     6,
     331,    38,    39,    40,   118,    38,    39,    40,    72,    81,
     322,   323,    81,    43,    44,    45,    79,    47,   101,   105,
       7,     8,     9,   321,   113,   114,    49,   120,   316,    50,
     -11,   -11,   -11,    77,   127,    23,   134,   146,   284,    89,
      90,    91,    92,    93,    94,    95,    96,   177,    97,     2,
      10,   145,   306,     6,   149,   155,   338,    43,    44,    45,
      79,    47,    38,    39,    40,    43,    44,    45,    79,    47,
      49,   156,   118,    50,   157,   158,   304,   305,    49,   159,
     170,   150,    51,    91,    92,    93,    94,   332,   332,   181,
     193,   330,    94,   331,   332,    43,    44,    45,    79,    47,
     205,    43,    44,    45,    79,    47,   215,   151,    49,   332,
     218,   247,   152,   342,    49,   221,   343,   244,   222,     2,
     303,   223,   224,     6,   229,   234,    51,    82,    43,    44,
      45,    46,    47,   236,    51,    48,    43,    44,    45,    79,
      47,    49,   235,   239,    50,   248,   249,   237,   250,    49,
     238,   240,    50,    43,    44,    45,    79,    47,   252,    38,
      39,    40,   319,   262,    51,   266,    49,   320,  -141,   199,
      51,    89,    90,    91,    92,    93,    94,    95,    96,   259,
      97,    89,    90,    91,    92,    93,    94,    95,    96,   171,
      97,   261,   172,    43,    44,    45,   173,    51,   174,   263,
    -141,    38,    39,    40,   264,    51,    38,    39,    40,   274,
      89,    90,    91,    92,    93,    94,    95,    96,   279,   206,
     121,   288,    51,    89,    90,    91,    92,    93,    94,    95,
      96,   285,   206,   182,   183,   184,   185,   167,   186,   187,
     270,   271,   188,   286,   290,   292,    82,   207,  -165,    89,
      90,    91,    92,    93,    94,    95,    96,   165,    97,   297,
     207,  -165,    89,    90,    91,    92,    93,    94,    95,    96,
     300,    97,    89,    90,    91,    92,    93,    94,    95,    96,
     301,    97,   121,   287,    89,    90,    91,    92,    93,    94,
      95,    96,    98,    97,    89,    90,    91,    92,    93,    94,
      95,   307,   328,    97,    89,    90,    91,    92,    93,    94,
     325,   336,   340,    97,   182,   183,   184,   185,   341,   186,
     187,   344,   232,   188,   184,   185,   213,   186,   115,   160,
     198,   188,    89,    90,    91,    92,    93,    94,    43,    44,
      45,   195,    43,    44,    45,   281,    43,    44,    45,   101,
     133,   123,   168,   335,   233,   201,   299,   295,   148,   308
};

static const yytype_uint16 yycheck[] =
{
      25,    63,     1,    77,   131,    86,   146,     0,    18,   131,
      19,   134,    19,    18,    18,    18,   329,    31,    32,    37,
      18,    18,    18,     0,    49,    50,    51,   150,   151,    18,
      18,   344,    18,    73,    33,    75,    60,    19,    84,    21,
      39,    40,    19,    25,    21,    22,    23,    24,    25,    39,
       6,    76,    77,    40,    57,    58,    59,    82,     6,   128,
       6,    37,    87,    62,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    19,    73,    18,    19,   123,    21,    22,
      23,    24,    25,    97,   102,   147,   209,   210,   211,    98,
      38,   101,    69,    70,    71,     6,   103,   102,   102,   124,
     181,   182,   229,   243,   102,   102,   102,   229,   177,    18,
     263,   264,   102,   102,   102,   114,   102,   240,    28,    29,
     102,    38,    99,    18,   149,   150,    69,    70,    71,   128,
       3,     4,     5,     6,     7,    19,   101,    21,     6,   292,
     165,    25,   167,    16,   297,    94,    19,   270,    57,    58,
      59,   232,    42,    43,    44,   236,    99,   238,    48,    67,
     300,   301,    57,    58,    59,   239,    18,    44,   161,    44,
       6,    48,    18,    48,   199,    48,    18,    61,   177,    26,
      19,   206,    21,    22,    23,    24,    25,   212,   328,    38,
      42,    43,    44,   274,    99,    27,    48,   101,    30,    72,
     199,    18,    34,    19,    36,    21,   205,    19,    81,    25,
      83,    57,    58,    59,   239,    57,    58,    59,    53,   244,
      79,    80,   247,     3,     4,     5,     6,     7,     6,    39,
      69,    70,    71,   307,    26,    19,    16,    10,   300,    19,
      57,    58,    59,    19,     6,   244,    39,    19,   247,     8,
       9,    10,    11,    12,    13,    14,    15,    19,    17,    21,
      99,     6,   287,    25,    17,     3,   328,     3,     4,     5,
       6,     7,    57,    58,    59,     3,     4,     5,     6,     7,
      16,     5,   307,    19,     6,    33,   285,   286,    16,     5,
      76,    19,    72,    10,    11,    12,    13,   322,   323,   102,
      63,    81,    13,    83,   329,     3,     4,     5,     6,     7,
      19,     3,     4,     5,     6,     7,     5,    45,    16,   344,
      77,    19,    50,    82,    16,    76,    85,    19,    25,    21,
      18,    35,    25,    25,    18,    91,    72,    73,     3,     4,
       5,     6,     7,    87,    72,    10,     3,     4,     5,     6,
       7,    16,    91,    65,    19,    53,    54,    91,    56,    16,
      87,    62,    19,     3,     4,     5,     6,     7,    46,    57,
      58,    59,    18,    25,    72,     6,    16,    18,     6,    19,
      72,     8,     9,    10,    11,    12,    13,    14,    15,    19,
      17,     8,     9,    10,    11,    12,    13,    14,    15,    27,
      17,    19,    30,     3,     4,     5,    34,    72,    36,    19,
      38,    57,    58,    59,    19,    72,    57,    58,    59,    87,
       8,     9,    10,    11,    12,    13,    14,    15,    64,    17,
      18,    78,    72,     8,     9,    10,    11,    12,    13,    14,
      15,    19,    17,    87,    88,    89,    90,    74,    92,    93,
      94,    95,    96,    19,     6,    19,    73,    45,    46,     8,
       9,    10,    11,    12,    13,    14,    15,   102,    17,    19,
      45,    46,     8,     9,    10,    11,    12,    13,    14,    15,
      65,    17,     8,     9,    10,    11,    12,    13,    14,    15,
      66,    17,    18,    42,     8,     9,    10,    11,    12,    13,
      14,    15,    38,    17,     8,     9,    10,    11,    12,    13,
      14,    65,   102,    17,     8,     9,    10,    11,    12,    13,
       6,    18,    82,    17,    87,    88,    89,    90,    84,    92,
      93,    42,    87,    96,    89,    90,   161,    92,    69,   111,
     147,    96,     8,     9,    10,    11,    12,    13,     3,     4,
       5,     6,     3,     4,     5,     6,     3,     4,     5,     6,
      87,    83,   123,   323,   183,   149,   276,   266,   102,   289
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
      97,   116,   117,    26,    19,   111,   108,   131,   132,   131,
      10,    18,   132,   140,    75,   141,   142,     6,    19,   150,
     151,   152,   154,   130,    39,   160,   132,   132,   132,   132,
     132,   132,   132,   132,   132,     6,    19,   102,   160,    17,
      19,    45,    50,   132,   161,     3,     5,     6,    33,     5,
     117,    19,    98,   108,    18,   102,    18,    74,   142,   132,
      76,    27,    30,    34,    36,   157,   158,    19,   108,   151,
     154,   102,    87,    88,    89,    90,    92,    93,    96,   155,
     157,   158,   161,    63,   164,     6,   124,   133,   122,    19,
     132,   152,   132,   161,   161,    19,    17,    45,   163,    42,
      43,    44,    48,   105,   106,     5,    18,   132,    77,   143,
     132,    76,    25,    35,    25,     6,    38,   108,    18,    18,
     151,   151,    87,   155,    91,    91,    87,    91,    87,    65,
      62,   165,    18,   102,    19,    18,   108,    19,    53,    54,
      56,   162,    46,   161,   161,   161,   132,    18,    18,    19,
     144,    19,    25,    19,    19,   159,     6,    18,   157,   158,
      94,    95,   156,   151,    87,   151,   151,   131,   161,    64,
     166,     6,   133,    18,   108,    19,    19,    42,    78,   145,
       6,   153,    19,   153,   153,   159,   161,    19,   151,   156,
      65,    66,   167,    18,   108,   108,   132,    65,   166,    18,
     102,   153,    18,    18,   153,   133,   134,   168,   133,    18,
      18,   131,    79,    80,   146,     6,    18,    18,   102,    48,
      81,    83,   132,   147,   148,   147,    18,   133,   134,   148,
      82,    84,    82,    85,    42,   148
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   104,   105,   105,   106,   106,   106,   106,   107,   107,
     107,   108,   108,   108,   108,   109,   110,   110,   111,   112,
     112,   112,   113,   113,   113,   114,   114,   114,   115,   115,
     116,   116,   117,   117,   117,   118,   119,   119,   120,   121,
     121,   122,   122,   123,   123,   124,   124,   124,   124,   125,
     125,   125,   126,   126,   127,   128,   128,   128,   129,   129,
     130,   130,   130,   130,   131,   131,   132,   132,   132,   132,
     132,   132,   132,   132,   133,   133,   133,   134,   135,   136,
     136,   136,   136,   136,   136,   136,   136,   136,   137,   138,
     139,   139,   140,   140,   141,   142,   142,   143,   143,   144,
     145,   145,   146,   146,   146,   147,   147,   148,   148,   148,
     148,   149,   149,   150,   150,   151,   151,   151,   151,   151,
     151,   152,   153,   153,   154,   154,   154,   154,   154,   154,
     155,   155,   155,   155,   155,   155,   155,   156,   156,   157,
     157,   158,   158,   158,   158,   158,   158,   159,   159,   160,
     160,   161,   161,   161,   161,   161,   161,   161,   161,   161,
     161,   161,   162,   162,   162,   163,   163,   164,   164,   165,
     165,   166,   166,   167,   167,   168,   168,   168,   168
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     3,     1,     1,     1,     1,     1,     1,
       1,     3,     1,     1,     1,     3,     3,     1,     5,     1,
       1,     1,     7,     7,     6,     0,     4,     4,     0,     2,
       1,     2,     2,     2,     2,     5,     0,     1,     5,     1,
       3,     3,     3,     7,     4,     1,     1,     3,     3,     3,
       3,     4,     0,     1,     9,     0,     1,     5,     1,     3,
       1,     3,     1,     3,     1,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     5,
       5,     4,     2,     1,     4,     0,     2,     0,     2,     5,
       0,     3,     0,     2,     2,     4,     1,     2,     2,     2,
       2,     0,     2,     1,     3,     2,     2,     2,     2,     1,
       4,     3,     1,     3,     3,     4,     5,     4,     5,     4,
       1,     2,     1,     2,     2,     1,     1,     4,     2,     3,
       4,     0,     1,     5,     5,     3,     6,     0,     3,     0,
       2,     3,     1,     2,     3,     3,     3,     5,     6,     5,
       6,     4,     1,     1,     1,     0,     1,     0,     3,     0,
       2,     0,     3,     0,     2,     1,     3,     1,     3
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
#line 1635 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 138 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("stmtlist::stmtList::stmt");
				(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[-1].node));	
				bisonParseResult = (Node *) (yyval.list); 
			}
#line 1645 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 147 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::dmlStmt");
            (yyval.node) = (yyvsp[0].node);
        }
#line 1654 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 152 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::queryStmt");
            (yyval.node) = (yyvsp[0].node);
        }
#line 1663 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 157 "sql_parser.y" /* yacc.c:1646  */
    {
            RULELOG("stmt::transactionIdentifier");
            (yyval.node) = (Node *) createTransactionStmt((yyvsp[0].stringVal));
        }
#line 1672 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 162 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("stmt::withQuery"); 
			(yyval.node) = (yyvsp[0].node); 
		}
#line 1681 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 172 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::insertQuery"); }
#line 1687 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 173 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::deleteQuery"); }
#line 1693 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 174 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("dmlStmt::updateQuery"); }
#line 1699 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 181 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::bracketedQuery"); (yyval.node) = (yyvsp[-1].node); }
#line 1705 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 182 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::selectQuery"); }
#line 1711 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 183 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::provStmt"); }
#line 1717 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 184 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("queryStmt::setOperatorQuery"); }
#line 1723 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 189 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withQuery::withViewList::queryStmt");
			(yyval.node) = (Node *) createWithStmt((yyvsp[-1].list), (yyvsp[0].node));
		}
#line 1732 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 197 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withViewList::list::view");
			(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
		}
#line 1741 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 202 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withViewList::view");
			(yyval.list) = singleton((yyvsp[0].node));
		}
#line 1750 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 210 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("withView::ident::AS:queryStmt");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString((yyvsp[-4].stringVal)), (yyvsp[-1].node));
		}
#line 1759 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 217 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::BEGIN"); (yyval.stringVal) = strdup("TRANSACTION_BEGIN"); }
#line 1765 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 218 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::COMMIT"); (yyval.stringVal) = strdup("TRANSACTION_COMMIT"); }
#line 1771 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 219 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("transactionIdentifier::ROLLBACK"); (yyval.stringVal) = strdup("TRANSACTION_ABORT"); }
#line 1777 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 1792 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 1806 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 1819 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 259 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalProvAsOf::EMPTY"); (yyval.node) = NULL; }
#line 1825 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 261 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvAsOf::SCN");
			(yyval.node) = (Node *) createConstLong((yyvsp[0].intVal));
		}
#line 1834 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 266 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvAsOf::TIMESTAMP");
			(yyval.node) = (Node *) createConstString((yyvsp[0].stringVal));
		}
#line 1843 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 273 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalProvWith::EMPTY"); (yyval.list) = NIL; }
#line 1849 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 275 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("optionalProvWith::WITH");
			(yyval.list) = (yyvsp[0].list);
		}
#line 1858 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 282 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("provOptionList::option"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 1864 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 284 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("provOptionList::list"); 
			(yyval.list) = appendToTailOfList((yyvsp[-1].list),(yyvsp[0].node)); 
		}
#line 1873 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 292 "sql_parser.y" /* yacc.c:1646  */
    { 
			RULELOG("provOption::TYPE"); 
			(yyval.node) = (Node *) createStringKeyValue("TYPE", (yyvsp[0].stringVal)); 
		}
#line 1882 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 297 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::TABLE");
			(yyval.node) = (Node *) createStringKeyValue("TABLE", (yyvsp[0].stringVal));
		}
#line 1891 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 302 "sql_parser.y" /* yacc.c:1646  */
    {
			RULELOG("provOption::ONLY::UPDATED");
			(yyval.node) = (Node *) createNodeKeyValue((Node *) createConstString("ONLY UPDATED"), 
					(Node *) createConstBool(TRUE));
		}
#line 1901 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 314 "sql_parser.y" /* yacc.c:1646  */
    { 
             RULELOG("deleteQuery");
             (yyval.node) = (Node *) createDelete((yyvsp[-2].stringVal), (yyvsp[0].node));
         }
#line 1910 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 322 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::NULL"); (yyval.stringVal) = NULL; }
#line 1916 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 323 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("fromString::FROM"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 1922 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 338 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG(updateQuery); 
                (yyval.node) = (Node *) createUpdate((yyvsp[-3].stringVal), (yyvsp[-1].list), (yyvsp[0].node)); 
            }
#line 1931 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 346 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setExpression");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 1940 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 351 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setClause::setClause::setExpression");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 1949 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 359 "sql_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal),"=")) {
                    RULELOG("setExpression::attributeRef::expression");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 1962 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 368 "sql_parser.y" /* yacc.c:1646  */
    {
                if (!strcmp((yyvsp[-1].stringVal), "=")) {
                    RULELOG("setExpression::attributeRef::queryStmt");
                    List *expr = singleton((yyvsp[-2].node));
                    expr = appendToTailOfList(expr, (yyvsp[0].node));
                    (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
                }
            }
#line 1975 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 383 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertQuery::insertList"); 
            	(yyval.node) = (Node *) createInsert((yyvsp[-4].stringVal),(Node *) (yyvsp[-1].list), NULL); 
        	}
#line 1984 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 388 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("insertQuery::queryStmt");
                (yyval.node) = (Node *) createInsert((yyvsp[-1].stringVal), (yyvsp[0].node), NULL);
            }
#line 1993 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 396 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::constant");
            	(yyval.list) = singleton((yyvsp[0].node)); 
            }
#line 2002 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 401 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("insertList::IDENTIFIER");
                (yyval.list) = singleton(createAttributeReference((yyvsp[0].stringVal)));
            }
#line 2011 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 406 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("insertList::insertList::::IDENTIFIER");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), createAttributeReference((yyvsp[0].stringVal)));
            }
#line 2020 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 411 "sql_parser.y" /* yacc.c:1646  */
    { 
            	RULELOG("insertList::insertList::constant");
            	(yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2029 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 425 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::INTERSECT");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 2038 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 430 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::MINUS");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-1].stringVal), FALSE, (yyvsp[-2].node), (yyvsp[0].node));
            }
#line 2047 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 435 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("setOperatorQuery::UNION");
                (yyval.node) = (Node *) createSetQuery((yyvsp[-2].stringVal), ((yyvsp[-1].stringVal) != NULL), (yyvsp[-3].node), (yyvsp[0].node));
            }
#line 2056 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 442 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::NULL"); (yyval.stringVal) = NULL; }
#line 2062 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 443 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAll::ALLTRUE"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 2068 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 453 "sql_parser.y" /* yacc.c:1646  */
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
#line 2088 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 475 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalDistinct::NULL"); (yyval.node) = NULL; }
#line 2094 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 477 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT");
                (yyval.node) = (Node *) createDistinctClause(NULL);
            }
#line 2103 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 482 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("optionalDistinct::DISTINCT::exprList");
                (yyval.node) = (Node *) createDistinctClause((yyvsp[-1].list));
            }
#line 2112 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 494 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectItem"); (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2120 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 498 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("selectClause::selectClause::selectItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node)); 
            }
#line 2129 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 506 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression"); 
                 (yyval.node) = (Node *) createSelectItem(NULL, (yyvsp[0].node)); 
             }
#line 2138 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 511 "sql_parser.y" /* yacc.c:1646  */
    {
                 RULELOG("selectItem::expression::identifier"); 
                 (yyval.node) = (Node *) createSelectItem((yyvsp[0].stringVal), (yyvsp[-2].node));
             }
#line 2147 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 516 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
         		(yyval.node) = (Node *) createSelectItem(strdup("*"), NULL); 
     		}
#line 2156 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 521 "sql_parser.y" /* yacc.c:1646  */
    { 
         		RULELOG("selectItem::*"); 
     			(yyval.node) = (Node *) createSelectItem(CONCAT_STRINGS((yyvsp[-2].stringVal),".*"), NULL); 
 			}
#line 2165 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 531 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("exprList::SINGLETON"); (yyval.list) = singleton((yyvsp[0].node)); }
#line 2171 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 533 "sql_parser.y" /* yacc.c:1646  */
    {
                  RULELOG("exprList::exprList::expression");
                  (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
             }
#line 2180 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 544 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::bracked"); (yyval.node) = (yyvsp[-1].node); }
#line 2186 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 545 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::constant"); }
#line 2192 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 546 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::attributeRef"); }
#line 2198 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 547 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlParameter"); }
#line 2204 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 548 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::binaryOperatorExpression"); }
#line 2210 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 549 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::unaryOperatorExpression"); }
#line 2216 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 550 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::sqlFunctionCall"); }
#line 2222 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 551 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("expression::case"); }
#line 2228 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 560 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::INT"); (yyval.node) = (Node *) createConstInt((yyvsp[0].intVal)); }
#line 2234 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 561 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::FLOAT"); (yyval.node) = (Node *) createConstFloat((yyvsp[0].floatVal)); }
#line 2240 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 562 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("constant::STRING"); (yyval.node) = (Node *) createConstString((yyvsp[0].stringVal)); }
#line 2246 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 569 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("attributeRef::IDENTIFIER"); (yyval.node) = (Node *) createAttributeReference((yyvsp[0].stringVal)); }
#line 2252 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 575 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("sqlParameter::PARAMETER"); (yyval.node) = (Node *) createSQLParameter((yyvsp[0].stringVal)); }
#line 2258 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 595 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '+' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2269 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 602 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '-' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2280 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 609 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '*' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2291 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 616 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '/' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2302 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 623 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '%' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2313 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 630 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '^' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2324 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 639 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '&' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2335 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 646 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression:: '|' ");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2346 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 655 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("binaryOperatorExpression::comparisonOps");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2357 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 665 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("unaryOperatorExpression:: '!' ");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2367 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 677 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("sqlFunctionCall::IDENTIFIER::exprList");
				FunctionCall *f = createFunctionCall((yyvsp[-4].stringVal), (yyvsp[-2].list));
				if ((yyvsp[0].node) != NULL)
					(yyval.node) = (Node *) createWindowFunction(f, (WindowDef *) (yyvsp[0].node));
				else  
                	(yyval.node) = (Node *) f; 
            }
#line 2380 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 692 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseExpression::CASE::expression::whens:else:END");
				(yyval.node) = (Node *) createCaseExpr((yyvsp[-3].node), (yyvsp[-2].list), (yyvsp[-1].node));
			}
#line 2389 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 697 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseExpression::CASE::whens::else::END");
				(yyval.node) = (Node *) createCaseExpr(NULL, (yyvsp[-2].list), (yyvsp[-1].node));
			}
#line 2398 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 705 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhenList::list::caseWhen");
				(yyval.list) = appendToTailOfList((yyvsp[-1].list), (yyvsp[0].node));
			}
#line 2407 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 710 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhenList::caseWhen");
				(yyval.list) = singleton((yyvsp[0].node));
			}
#line 2416 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 718 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("caseWhen::WHEN::expression::THEN::expression");
				(yyval.node) = (Node *) createCaseWhen((yyvsp[-2].node),(yyvsp[0].node));
			}
#line 2425 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 725 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalCaseElse::NULL"); (yyval.node) = NULL; }
#line 2431 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 727 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalCaseElse::ELSE::expression");
				(yyval.node) = (yyvsp[0].node);
			}
#line 2440 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 737 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("overclause::NULL"); (yyval.node) = NULL; }
#line 2446 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 739 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("overclause::window");
				(yyval.node) = (yyvsp[0].node);
			}
#line 2455 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 747 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("window");
				(yyval.node) = (Node *) createWindowDef((yyvsp[-3].list),(yyvsp[-2].list), (WindowFrame *) (yyvsp[-1].node));
			}
#line 2464 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 754 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optWindowPart::NULL"); (yyval.list) = NIL; }
#line 2470 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 756 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optWindowPart::PARTITION:BY::expressionList");
				(yyval.list) = (yyvsp[0].list);
			}
#line 2479 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 763 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optWindowFrame::NULL"); (yyval.node) = NULL; }
#line 2485 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 765 "sql_parser.y" /* yacc.c:1646  */
    { 
				WindowBound *l, *u = NULL;
				RULELOG("optWindowFrame::ROWS::windoBoundaries");
				l = getNthOfListP((yyvsp[0].list), 0);
				if(LIST_LENGTH((yyvsp[0].list)) > 1)
					u = getNthOfListP((yyvsp[0].list), 1);
				(yyval.node) = (Node *) createWindowFrame(WINFRAME_ROWS, l, u); 
			}
#line 2498 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 774 "sql_parser.y" /* yacc.c:1646  */
    {
				WindowBound *l, *u = NULL; 
				RULELOG("optWindowFrame::RANGE::windoBoundaries"); 
				l = getNthOfListP((yyvsp[0].list), 0);
				if(LIST_LENGTH((yyvsp[0].list)) > 1)
					u = getNthOfListP((yyvsp[0].list), 1);
				(yyval.node) = (Node *) createWindowFrame(WINFRAME_RANGE, l, u); 
			}
#line 2511 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 786 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBoundaries::BETWEEN"); 
				(yyval.list) = LIST_MAKE((yyvsp[-2].node), (yyvsp[0].node)); 
			}
#line 2520 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 791 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBoundaries::windowBound"); 
				(yyval.list) = singleton((yyvsp[0].node)); 
			}
#line 2529 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 799 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::UNBOUNDED::PRECEDING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_UNBOUND_PREC, NULL); 
			}
#line 2538 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 804 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::CURRENT::ROW"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_CURRENT_ROW, NULL); 
			}
#line 2547 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 809 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::expression::PRECEDING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_EXPR_PREC, (yyvsp[-1].node)); 
			}
#line 2556 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 814 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("windowBound::expression::FOLLOWING"); 
				(yyval.node) = (Node *) createWindowBound(WINBOUND_EXPR_FOLLOW, (yyvsp[-1].node)); 
			}
#line 2565 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 826 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::NULL"); (yyval.list) = NULL; }
#line 2571 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 827 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFrom::fromClause"); (yyval.list) = (yyvsp[0].list); }
#line 2577 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 832 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClauseItem");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 2586 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 837 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClause::fromClause::fromClauseItem");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 2595 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 846 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
				FromItem *f = createFromTableRef(NULL, NIL, (yyvsp[-1].stringVal));
				f->provInfo = (FromProvInfo *) (yyvsp[0].node);
                (yyval.node) = (Node *) f;
            }
#line 2606 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 853 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem");
                FromItem *f = createFromTableRef(((FromItem *) (yyvsp[0].node))->name, 
						((FromItem *) (yyvsp[0].node))->attrNames, (yyvsp[-1].stringVal));
				f->provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
                (yyval.node) = (Node *) f;
            }
#line 2618 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 862 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromItem *f = (FromItem *) (yyvsp[-1].node);
                f->provInfo = (FromProvInfo *) (yyvsp[0].node);
                (yyval.node) = (yyvsp[-1].node);
            }
#line 2629 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 869 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromClauseItem::subQuery");
                FromSubquery *s = (FromSubquery *) (yyvsp[-1].node);
                s->from.name = ((FromItem *) (yyvsp[0].node))->name;
                s->from.attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
                s->from.provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
                (yyval.node) = (Node *) s;
            }
#line 2642 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 878 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[0].node);
        		f->name = NULL;
        		(yyval.node) = (Node *) f;
        	}
#line 2654 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 886 "sql_parser.y" /* yacc.c:1646  */
    {
        		FromItem *f;
        		RULELOG("fromClauseItem::fromJoinItem");
        		f = (FromItem *) (yyvsp[-2].node);
        		f->name = ((FromItem *) (yyvsp[0].node))->name;
                f->attrNames = ((FromItem *) (yyvsp[0].node))->attrNames;
                f->provInfo = ((FromItem *) (yyvsp[0].node))->provInfo;
        		(yyval.node) = (Node *) f;
        	}
#line 2668 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 899 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("subQuery::queryStmt");
                (yyval.node) = (Node *) createFromSubquery(NULL, NULL, (yyvsp[-1].node));
            }
#line 2677 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 906 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = singleton((yyvsp[0].stringVal)); }
#line 2683 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 907 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].stringVal)); }
#line 2689 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 911 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2695 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 913 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURAL");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
						(FromItem *) (yyvsp[0].node), "JOIN_INNER", "JOIN_COND_NATURAL", 
						NULL);
          	}
#line 2706 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 920 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("fromJoinItem::NATURALjoinType");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[0].node), (yyvsp[-2].stringVal), "JOIN_COND_NATURAL", NULL);
          	}
#line 2716 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 926 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::CROSS JOIN");
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[0].node), "JOIN_CROSS", "JOIN_COND_ON", NULL);
          	}
#line 2726 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 932 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinType::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON";
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-4].node), 
                		(FromItem *) (yyvsp[-1].node), (yyvsp[-3].stringVal), condType, (yyvsp[0].node));
          	}
#line 2738 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 940 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("fromJoinItem::JOIN::joinCond");
				char *condType = (isA((yyvsp[0].node),List)) ? "JOIN_COND_USING" : 
						"JOIN_COND_ON"; 
                (yyval.node) = (Node *) createFromJoin(NULL, NIL, (FromItem *) (yyvsp[-3].node), 
                		(FromItem *) (yyvsp[-1].node), "JOIN_INNER", 
                		condType, (yyvsp[0].node));
          	}
#line 2751 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 951 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2757 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 952 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::LEFT OUTER"); (yyval.stringVal) = "JOIN_LEFT_OUTER"; }
#line 2763 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 953 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT "); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2769 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 954 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::RIGHT OUTER"); (yyval.stringVal) = "JOIN_RIGHT_OUTER"; }
#line 2775 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 955 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL OUTER"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2781 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 956 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::FULL"); (yyval.stringVal) = "JOIN_FULL_OUTER"; }
#line 2787 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 957 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("joinType::INNER"); (yyval.stringVal) = "JOIN_INNER"; }
#line 2793 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 961 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (Node *) (yyvsp[-1].list); }
#line 2799 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 962 "sql_parser.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2805 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 967 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[-2].node);
				(yyval.node) = (Node *) f;
			}
#line 2816 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 974 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAlias::identifier"); 
				FromItem *f = createFromItem((yyvsp[-1].stringVal),(yyvsp[0].list));
 				f->provInfo = (FromProvInfo *) (yyvsp[-3].node); 
				(yyval.node) = (Node *) f;
			}
#line 2827 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 983 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalFromProv::empty"); (yyval.node) = NULL; }
#line 2833 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 985 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::BASERELATION");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = TRUE;
				p->userProvAttrs = NIL;				 
				(yyval.node) = (Node *) p; 
			}
#line 2845 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 993 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::userProvAttr");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = FALSE;
				p->userProvAttrs = (yyvsp[-1].list);				 
				(yyval.node) = (Node *) p; 
			}
#line 2857 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 1001 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::userProvDupAttr");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->baserel = TRUE;
				p->userProvAttrs = (yyvsp[-1].list);
				(yyval.node) = (Node *) p;
			}
#line 2869 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1009 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::intermediateProv");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->intermediateProv = TRUE;
				(yyval.node) = (Node *) p;
			}
#line 2880 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1016 "sql_parser.y" /* yacc.c:1646  */
    {
				RULELOG("optionalFromProv::intermediateProv::attrList");
				FromProvInfo *p = makeNode(FromProvInfo);
				p->intermediateProv = TRUE;
				p->userProvAttrs = (yyvsp[-1].list);
				(yyval.node) = (Node *) p;
			}
#line 2892 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1026 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalAttrAlias::empty"); (yyval.list) = NULL; }
#line 2898 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1028 "sql_parser.y" /* yacc.c:1646  */
    { 
				RULELOG("optionalAttrAlias::identifierList"); (yyval.list) = (yyvsp[-1].list); 
			}
#line 2906 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1037 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::NULL"); (yyval.node) = NULL; }
#line 2912 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1038 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalWhere::whereExpression"); (yyval.node) = (yyvsp[0].node); }
#line 2918 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1042 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("where::brackedWhereExpression"); (yyval.node) = (yyvsp[-1].node); }
#line 2924 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1043 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("whereExpression::expression"); (yyval.node) = (yyvsp[0].node); }
#line 2930 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1045 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::NOT");
                List *expr = singleton((yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2940 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1051 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2951 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1058 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2962 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 1065 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::AND");
                List *expr = singleton((yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-1].stringVal), expr);
            }
#line 2973 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 1072 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::BETWEEN-AND");
                List *expr = singleton((yyvsp[-4].node));
                expr = appendToTailOfList(expr, (yyvsp[-2].node));
                expr = appendToTailOfList(expr, (yyvsp[0].node));
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr);
            }
#line 2985 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1080 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::comparisonOps::nestedSubQueryOperator::Subquery");
                (yyval.node) = (Node *) createNestedSubquery((yyvsp[-3].stringVal), (yyvsp[-5].node), (yyvsp[-4].stringVal), (yyvsp[-1].node));
            }
#line 2994 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1085 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("whereExpression::Subquery");
                Node *q = (Node *) createNestedSubquery("SCALAR", NULL, NULL, (yyvsp[-1].node)); 
                List *expr = LIST_MAKE((yyvsp[-4].node), q);
                (yyval.node) = (Node *) createOpExpr((yyvsp[-3].stringVal), expr); 
            }
#line 3005 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 1092 "sql_parser.y" /* yacc.c:1646  */
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
#line 3022 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1105 "sql_parser.y" /* yacc.c:1646  */
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
#line 3039 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1120 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ANY"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 3045 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1121 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::ALL"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 3051 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 1122 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("nestedSubQueryOperator::SOME"); (yyval.stringVal) = "ANY"; }
#line 3057 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 1126 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NULL"); (yyval.stringVal) = NULL; }
#line 3063 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 1127 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalNot::NOT"); (yyval.stringVal) = (yyvsp[0].stringVal); }
#line 3069 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1131 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::NULL"); (yyval.list) = NULL; }
#line 3075 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 1132 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalGroupBy::GROUPBY"); (yyval.list) = (yyvsp[0].list); }
#line 3081 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 1136 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.node) = NULL; }
#line 3087 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 1138 "sql_parser.y" /* yacc.c:1646  */
    { 
                RULELOG("optionalHaving::HAVING"); 
                (yyval.node) = (Node *) (yyvsp[0].node);
            }
#line 3096 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 1145 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy:::NULL"); (yyval.list) = NULL; }
#line 3102 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 1146 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalOrderBy::ORDERBY"); (yyval.list) = (yyvsp[0].list); }
#line 3108 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 1150 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::NULL"); (yyval.node) = NULL; }
#line 3114 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 1151 "sql_parser.y" /* yacc.c:1646  */
    { RULELOG("optionalLimit::CONSTANT"); (yyval.node) = (yyvsp[0].node);}
#line 3120 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 1156 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::attributeRef");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 3129 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 1161 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 3138 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 1166 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::constant");
                (yyval.list) = singleton((yyvsp[0].node));
            }
#line 3147 "sql_parser.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 1171 "sql_parser.y" /* yacc.c:1646  */
    {
                RULELOG("clauseList::clauseList::attributeRef");
                (yyval.list) = appendToTailOfList((yyvsp[-2].list), (yyvsp[0].node));
            }
#line 3156 "sql_parser.tab.c" /* yacc.c:1646  */
    break;


#line 3160 "sql_parser.tab.c" /* yacc.c:1646  */
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
#line 1178 "sql_parser.y" /* yacc.c:1906  */




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

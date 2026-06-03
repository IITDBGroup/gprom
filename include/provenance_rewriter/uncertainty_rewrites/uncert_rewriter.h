#include "common.h"
#include "model/query_operator/query_operator.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"

#define UNCERTAIN_ROW_ATTR backendifyIdentifier("R")
#define UNCERTAIN_FULL_ROW_ATTR getUncertString(UNCERTAIN_ROW_ATTR)
#define UNCERTAIN_MAKER_FUNC_NAME "UNCERT"
#define ROW_CERTAIN backendifyIdentifier("CET_R")
#define ROW_BESTGUESS backendifyIdentifier("BST_R")
#define ROW_POSSIBLE backendifyIdentifier("POS_R")
#define ROW_CERTAIN_TWO backendifyIdentifier("CET_R1")
#define ROW_BESTGUESS_TWO backendifyIdentifier("BST_R1")
#define ROW_POSSIBLE_TWO backendifyIdentifier("POS_R1")
#define ALPHABET " !\"#$\%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"

// USET相关常量定义
#define USET_FUNC_NAME "USET"

/* AUDB / i4r：int4range[] 算术（set_*） */
#define SET_ADD_FUNC_NAME "set_add"
#define SET_SUBTRACT_FUNC_NAME "set_subtract"
#define SET_MULTIPLY_FUNC_NAME "set_multiply"
#define SET_DIVIDE_FUNC_NAME "set_divide"

/* AUDB / i4r：int4range[] 三值比较（boolean） */
#define AUDB_SET_EQ_FUNC_NAME "set_eq"
#define AUDB_SET_LT_FUNC_NAME "set_lt"
#define AUDB_SET_LTE_FUNC_NAME "set_lte"
#define AUDB_SET_GT_FUNC_NAME "set_gt"
#define AUDB_SET_GTE_FUNC_NAME "set_gte"

/* AUDB / i4r：int4range 单区间三值比较（boolean） */
#define AUDB_RANGE_EQ_FUNC_NAME "range_eq"
#define AUDB_RANGE_LT_FUNC_NAME "range_lt"
#define AUDB_RANGE_LTE_FUNC_NAME "range_lte"
#define AUDB_RANGE_GT_FUNC_NAME "range_gt"
#define AUDB_RANGE_GTE_FUNC_NAME "range_gte"

#define LIFT_SCALAR_FUNC_NAME "lift_scalar"
#define LIFT_RANGE_FUNC_NAME "lift_range"
#define SET_NORMALIZE_FUNC_NAME "set_normalize"
#define SET_SORT_FUNC_NAME "set_sort"
#define SET_REDUCE_SIZE_FUNC_NAME "set_reduce_size"
#define ARRAY_LENGTH_FUNC_NAME "array_length"
#define RANGE_COVERAGE_FUNC_NAME "range_coverage"
#define SET_COVERAGE_FUNC_NAME "set_coverage"

/* i4r：聚合 combine（每行 value × multiplicity） */
#define COMBINE_SET_MULT_SUM_FUNC_NAME "combine_set_mult_sum"
#define COMBINE_SET_MULT_MIN_FUNC_NAME "combine_set_mult_min"
#define COMBINE_SET_MULT_MAX_FUNC_NAME "combine_set_mult_max"
#define COMBINE_RANGE_MULT_SUM_FUNC_NAME "combine_range_mult_sum"
#define COMBINE_RANGE_MULT_MIN_FUNC_NAME "combine_range_mult_min"
#define COMBINE_RANGE_MULT_MAX_FUNC_NAME "combine_range_mult_max"
#define USET_AVG_RANGE_DATA_FUNC_NAME "range_set_first"

/* i4r：int4range[] 投影剪枝（SELECT 收缩） */
#define PRUNE_SET_EQ_FUNC_NAME "prune_set_eq"
#define PRUNE_SET_LT_FUNC_NAME "prune_set_lt"
#define PRUNE_SET_LTE_FUNC_NAME "prune_set_lte"
#define PRUNE_SET_GT_FUNC_NAME "prune_set_gt"
#define PRUNE_SET_GTE_FUNC_NAME "prune_set_gte"
#define PRUNE_SET_AND_FUNC_NAME "prune_set_and"
#define PRUNE_SET_OR_FUNC_NAME "prune_set_or"

/* i4r：int4range 单区间投影剪枝 */
#define PRUNE_RANGE_EQ_FUNC_NAME "prune_range_eq"
#define PRUNE_RANGE_LT_FUNC_NAME "prune_range_lt"
#define PRUNE_RANGE_LTE_FUNC_NAME "prune_range_lte"
#define PRUNE_RANGE_GT_FUNC_NAME "prune_range_gt"
#define PRUNE_RANGE_GTE_FUNC_NAME "prune_range_gte"
#define PRUNE_RANGE_AND_FUNC_NAME "prune_range_and"
#define PRUNE_RANGE_OR_FUNC_NAME "prune_range_or"

/* 向后兼容旧 PL/pgSQL 名（内部不再生成） */
#define PRUNE_EQ_FUNC_NAME PRUNE_SET_EQ_FUNC_NAME
#define PRUNE_LT_FUNC_NAME PRUNE_SET_LT_FUNC_NAME
#define PRUNE_GT_FUNC_NAME PRUNE_SET_GT_FUNC_NAME
#define PRUNE_AND_FUNC_NAME PRUNE_SET_AND_FUNC_NAME
#define PRUNE_OR_FUNC_NAME PRUNE_SET_OR_FUNC_NAME

extern QueryOperator *rewriteUncert(QueryOperator *op);
extern QueryOperator *rewriteUncertTuple(QueryOperator *op);
extern QueryOperator *rewriteRange(QueryOperator *op);
extern QueryOperator *rewriteUset(QueryOperator *op);
extern Node *getUncertaintyExpr(Node *expr, HashMap *hmp); //hashmap of attrref->attrref(attribute->attribute uncertainty)
extern Node *removeUncertOpFromExpr(Node *expr);
extern char *getUncertString(char *in);
extern char *getUBString(char *in);
extern char *getLBString(char *in);

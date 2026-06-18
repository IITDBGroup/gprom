/*-------------------------------------------------------------------------
 *
 * i4r_audb_extension.c
 *
 * Entry point for the AUDB PostgreSQL extension. Registers all SQL-callable
 * functions and defines shared macros used across arithmetic, logical,
 * pruning, and aggregate operations on uncertain RangeType and ArrayType internal 
 * data types. Extension of Original GPRoM system https://github.com/IITDBGroup/gprom
 *
 * 
 * src/audb_extension.c
 *
 *-------------------------------------------------------------------------
 */

// source files
#include "postgres.h"           // src
#include "fmgr.h"               // must be included
#include "utils/rangetypes.h"   // rangeType
#include "utils/array.h"        // arrayType
#include "utils/typcache.h"     // speed type lookup
#include "utils/lsyscache.h"    // typcache convenience functions 
#include "catalog/pg_type_d.h"  // pg_type OIDs
#include "catalog/namespace.h"  // get typenames
#include "funcapi.h"

// local code
#include "arithmetic.h"         // logic for arithmetic 
#include "logicalOperators.h"   // logic for logical ops
#include "helperFunctions.h"    // logic for helpers
#include "serialization.h"      // serial and deserial helpers
#include "prune.h"              // prune stuff

PG_MODULE_MAGIC;

/*(Arithmetic Functions)*/
PG_FUNCTION_INFO_V1(range_add);
PG_FUNCTION_INFO_V1(range_subtract);
PG_FUNCTION_INFO_V1(range_multiply);
PG_FUNCTION_INFO_V1(range_divide);
PG_FUNCTION_INFO_V1(set_add);
PG_FUNCTION_INFO_V1(set_subtract);
PG_FUNCTION_INFO_V1(set_multiply);
PG_FUNCTION_INFO_V1(set_divide);

/*(Logical / Comparison)*/
PG_FUNCTION_INFO_V1(range_lt);
PG_FUNCTION_INFO_V1(range_lte);
PG_FUNCTION_INFO_V1(range_gt);
PG_FUNCTION_INFO_V1(range_gte);
PG_FUNCTION_INFO_V1(range_eq);
PG_FUNCTION_INFO_V1(set_lt);
PG_FUNCTION_INFO_V1(set_lte);
PG_FUNCTION_INFO_V1(set_gt);
PG_FUNCTION_INFO_V1(set_gte);
PG_FUNCTION_INFO_V1(set_eq);

/*(Helper Functions)*/
PG_FUNCTION_INFO_V1(array_length);
PG_FUNCTION_INFO_V1(range_coverage);
PG_FUNCTION_INFO_V1(set_coverage);
PG_FUNCTION_INFO_V1(lift_scalar);
PG_FUNCTION_INFO_V1(lift_range);
PG_FUNCTION_INFO_V1(set_sort);
PG_FUNCTION_INFO_V1(set_normalize);
PG_FUNCTION_INFO_V1(set_reduce_size);

/*(Prune Logical Functions)*/
PG_FUNCTION_INFO_V1(prune_range_lt);
PG_FUNCTION_INFO_V1(prune_range_gt);
PG_FUNCTION_INFO_V1(prune_range_lte);
PG_FUNCTION_INFO_V1(prune_range_gte);
PG_FUNCTION_INFO_V1(prune_range_eq);
PG_FUNCTION_INFO_V1(prune_range_and);
PG_FUNCTION_INFO_V1(prune_range_or);
PG_FUNCTION_INFO_V1(prune_set_lt);
PG_FUNCTION_INFO_V1(prune_set_lte);
PG_FUNCTION_INFO_V1(prune_set_gt);
PG_FUNCTION_INFO_V1(prune_set_gte);
PG_FUNCTION_INFO_V1(prune_set_eq);
PG_FUNCTION_INFO_V1(prune_set_and);
PG_FUNCTION_INFO_V1(prune_set_or);

/*(Aggregate Functions)*/
//          sum
PG_FUNCTION_INFO_V1(combine_range_mult_sum);
PG_FUNCTION_INFO_V1(combine_set_mult_sum);
PG_FUNCTION_INFO_V1(agg_sum_range_transfunc);           // not used anymore bc combine_range_mult_sum returns i4r[]
PG_FUNCTION_INFO_V1(agg_sum_set_transfunc);
PG_FUNCTION_INFO_V1(agg_sum_set_finalfunc);
PG_FUNCTION_INFO_V1(agg_sum_set_transfunc_metrics);     // stores extra information/metrics for internal testing.
PG_FUNCTION_INFO_V1(agg_sum_set_finalfunc_metrics);     // ^^

//          min/max
PG_FUNCTION_INFO_V1(combine_range_mult_min);
PG_FUNCTION_INFO_V1(combine_range_mult_max);
PG_FUNCTION_INFO_V1(combine_set_mult_min);
PG_FUNCTION_INFO_V1(combine_set_mult_max);
PG_FUNCTION_INFO_V1(agg_min_range_transfunc);
PG_FUNCTION_INFO_V1(agg_max_range_transfunc);
PG_FUNCTION_INFO_V1(agg_min_set_transfunc);
PG_FUNCTION_INFO_V1(agg_max_set_transfunc);
PG_FUNCTION_INFO_V1(agg_min_max_set_finalfunc);         // shared final function

//          count
PG_FUNCTION_INFO_V1(agg_count_transfunc);

//          avg == sum/count
PG_FUNCTION_INFO_V1(agg_avg_range_transfunc);
PG_FUNCTION_INFO_V1(agg_avg_range_finalfunc);
PG_FUNCTION_INFO_V1(agg_avg_set_transfunc);
PG_FUNCTION_INFO_V1(agg_avg_set_finalfunc);


/*
* easy change for future implementation. currently only affects lift funciton
* can maybe remove use of PRIMARY_DATA_TYPE and instead 
* get OID of x, and lift that to range type for that OID (if possible ofc)
*/ 
#define PRIMARY_DATA_TYPE "int4range"


///////////////////////////////////////////////////////////////
 //   MACROS
///////////////////////////////////////////////////////////////

/// check for NULLS parameters. Different from empty range check
//  returns the parameter that is not null
#define HANDLE_BOTH_ARG_ISNULL()                          \
    do {                                                        \
        if (PG_ARGISNULL(0) && PG_ARGISNULL(1))                 \
            PG_RETURN_NULL();                                   \
        else if (PG_ARGISNULL(0))                               \
            PG_RETURN_DATUM(PG_GETARG_DATUM(1));                \
        else if (PG_ARGISNULL(1))                               \
            PG_RETURN_DATUM(PG_GETARG_DATUM(0));                \
    } while (0)

// returns NULL on either PGARG(0) OR PGARG(1)
#define HANDLE_EITHER_ARG_ISNULL()                               \
    do {                                                        \
        if (PG_ARGISNULL(0) || PG_ARGISNULL(1))                 \
            PG_RETURN_NULL();                                   \
    } while (0)

/* takes in 2 RangeType parameters, and returns a single RangeType with provided operator result */
#define DEFINE_RANGE_ARITHMETIC_FUNC(func_name, internal_func)      \
Datum func_name(PG_FUNCTION_ARGS)                                   \
{                                                                   \
    RangeType *r1;                                                  \
    RangeType *r2;                                                  \
    RangeType *output;                                              \
    HANDLE_BOTH_ARG_ISNULL();                                        \
    r1 = PG_GETARG_RANGE_P(0);                                      \
    r2 = PG_GETARG_RANGE_P(1);                                      \
    output = arithmetic_range_helper(r1, r2, internal_func);        \
    PG_RETURN_RANGE_P(output);                                      \
}

/* takes in 2 ArrayType parameters, and returns a single ArrayType with provided operator result */
#define DEFINE_SET_ARITHMETIC_FUNC(func_name, internal_func)        \
Datum func_name(PG_FUNCTION_ARGS)                                   \
{                                                                   \
    ArrayType *a1;                                                  \
    ArrayType *a2;                                                  \
    ArrayType *output;                                              \
    HANDLE_EITHER_ARG_ISNULL();                                      \
    a1 = PG_GETARG_ARRAYTYPE_P(0);                                  \
    a2 = PG_GETARG_ARRAYTYPE_P(1);                                  \
    output = arithmetic_set_helper(a1, a2, internal_func);          \
    PG_RETURN_ARRAYTYPE_P(output);                                  \
}

/* takes in 2 RangeType parameters, and returns a 3VL boolean after comparison*/
#define DEFINE_RANGE_LOGICAL_FUNC(func_name, internal_func)         \
Datum func_name(PG_FUNCTION_ARGS)                                   \
{                                                                   \
    RangeType *r1;                                                  \
    RangeType *r2;                                                  \
    int rv;                                                         \
    HANDLE_EITHER_ARG_ISNULL();                                      \
    r1 = PG_GETARG_RANGE_P(0);                                      \
    r2 = PG_GETARG_RANGE_P(1);                                      \
    rv = logical_range_helper(r1, r2, internal_func);               \
    if (rv == -1){                                                  \
        PG_RETURN_NULL();                                           \
    }                                                               \
    PG_RETURN_BOOL((bool)rv);                                       \
}

/* takes in 2 ArrayType parameters, and returns a 3VL boolean after comparison*/
#define DEFINE_SET_LOGICAL_FUNC(func_name, internal_func)           \
Datum func_name(PG_FUNCTION_ARGS)                                   \
{                                                                   \
    ArrayType *a1;                                                  \
    ArrayType *a2;                                                  \
    int rv;                                                         \
    HANDLE_EITHER_ARG_ISNULL();                                   \
    a1 = PG_GETARG_ARRAYTYPE_P(0);                                  \
    a2 = PG_GETARG_ARRAYTYPE_P(1);                                  \
    rv = logical_set_helper(a1, a2, internal_func);                 \
    if (rv == -1){                                                  \
        PG_RETURN_NULL();                                           \
    }                                                               \
    PG_RETURN_BOOL((bool)rv);                                       \
}

/* range- prune/cuts away impossible values based on logical condition (lt, gt, lte, gte, eq)
 * contains direction */
#define DEFINE_PRUNE_RANGE_FUNC_COMPARISON(func_name, internal_func)           \
Datum func_name(PG_FUNCTION_ARGS)                                   \
{                                                                   \
    RangeType      *a_range, *b_range, *output;                     \
    bool            direction;                                      \
    TypeCacheEntry *typcache;                                       \
    Int4Range       a, b, result;                                   \
    if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2))     \
        PG_RETURN_NULL();                                           \
    a_range   = PG_GETARG_RANGE_P(0);                               \
    b_range   = PG_GETARG_RANGE_P(1);                               \
    direction = PG_GETARG_BOOL(2);                                  \
    typcache  = lookup_type_cache(a_range->rangetypid, TYPECACHE_RANGE_INFO);            \
    a = deserialize_RangeType(a_range, typcache);                   \
    b = deserialize_RangeType(b_range, typcache);                   \
    result = internal_func(a, b, direction);                        \
    if (result.isNull)                                              \
        PG_RETURN_NULL();                                           \
    output = serialize_RangeType(result, typcache);                 \
    PG_RETURN_RANGE_P(output);                                      \
}

/* prune/cuts away impossible values based on logical condition (and, eq)
 * does not contain direction */
#define DEFINE_PRUNE_RANGE_FUNC_LOGICAL(func_name, internal_func)       \
Datum func_name(PG_FUNCTION_ARGS)                                       \
{                                                                       \
    RangeType      *a_range, *b_range, *output;                         \
    TypeCacheEntry *typcache;                                           \
    Int4Range       a, b, result;                                       \
    if (PG_ARGISNULL(0) || PG_ARGISNULL(1)) {PG_RETURN_NULL();}         \
    a_range   = PG_GETARG_RANGE_P(0);                                   \
    b_range   = PG_GETARG_RANGE_P(1);                                   \
    typcache  = lookup_type_cache(a_range->rangetypid, TYPECACHE_RANGE_INFO);   \
    a = deserialize_RangeType(a_range, typcache);                       \
    b = deserialize_RangeType(b_range, typcache);                       \
    result = internal_func (a, b);                                      \
    if (result.isNull) {PG_RETURN_NULL();}                              \
    output = serialize_RangeType(result, typcache);                     \
    PG_RETURN_RANGE_P(output);                                          \
}

/* prune/cuts away impossible values based on logical condition (OR)
 * special case bc it can grow. 2 rangetypes == arraytype worst case */
#define DEFINE_PRUNE_RANGE_FUNC_LOGICAL_OR(func_name, internal_func)    \
Datum func_name(PG_FUNCTION_ARGS)                                       \
{                                                                       \
    ArrayType *output;                                                  \
    RangeType      *a_range, *b_range;                                  \
    Oid rangetypid;                                                    \
    TypeCacheEntry *typcache;                                           \
    Int4Range       a, b;                                               \
    Int4RangeSet result;                                                \
    if (PG_ARGISNULL(0) || PG_ARGISNULL(1)) {PG_RETURN_NULL();}         \
    a_range   = PG_GETARG_RANGE_P(0);                                   \
    b_range   = PG_GETARG_RANGE_P(1);                                   \
    rangetypid = RangeTypeGetOid(a_range);                             \
    typcache  = lookup_type_cache(rangetypid, TYPECACHE_RANGE_INFO);   \
    a = deserialize_RangeType(a_range, typcache);                       \
    b = deserialize_RangeType(b_range, typcache);                       \
    result = internal_func (a, b);                                      \
    /* Nothing to return */                                             \
    if (result.count == 0 && !result.containsNull) {                    \
        PG_RETURN_NULL();                                               \
    }                                                                   \
    output = serialize_ArrayType(result, typcache);                     \
    if (result.ranges)                                                  \
        pfree(result.ranges);                                           \
    /*elog(INFO, "Type OID: %u", typcache->type_id);                   \
    elog(INFO, "serialize_ArrayType: set.count = %d", result.count);*/   \
    PG_RETURN_ARRAYTYPE_P(output);                                      \
}

#define DEFINE_PRUNE_SET_FUNC_LOGICAL(func_name, internal_func)         \
Datum func_name(PG_FUNCTION_ARGS)                                       \
{                                                                       \
    ArrayType *a_range, *b_range, *output;                              \
    Oid rangeTypeOID;                                                   \
    TypeCacheEntry *typcache;                                           \
    Int4RangeSet a, b, result, norm_result;                             \
                                                                        \
    if (PG_ARGISNULL(0) || PG_ARGISNULL(1)) {PG_RETURN_NULL();}         \
    a_range   = PG_GETARG_ARRAYTYPE_P(0);                               \
    b_range   = PG_GETARG_ARRAYTYPE_P(1);                               \
                                                                        \
    rangeTypeOID = ARR_ELEMTYPE(a_range);                               \
    typcache = lookup_type_cache(rangeTypeOID, TYPECACHE_RANGE_INFO);   \
    a = deserialize_ArrayType(a_range, typcache);                       \
    b = deserialize_ArrayType(b_range, typcache);                       \
    result = internal_func (a, b);                                      \
    /* Nothing to return */                                             \
    if (result.count == 0 && !result.containsNull) {                    \
        PG_RETURN_NULL();                                               \
    }                                                                   \
                                                                        \
    norm_result = normalize(result);                                    \
    output = serialize_ArrayType(norm_result, typcache);                \
    if (result.ranges)   pfree(result.ranges);                          \
    if (norm_result.ranges) pfree(norm_result.ranges);                  \
    /*elog(INFO, "Type OID: %u", typcache->type_id);                    \
    elog(INFO, "serialize_ArrayType: set.count = %d", result.count);*/  \
    PG_RETURN_ARRAYTYPE_P(output);                                      \
}


/* set- prune/cuts away impossible values based on logical condition (lt, gt, lte, gte, eq)
 * contains direction */
#define DEFINE_PRUNE_SET_FUNC_COMPARISON(func_name, internal_func)         \
Datum func_name(PG_FUNCTION_ARGS)                                       \
{                                                                       \
    ArrayType *a_range, *b_range, *output;                              \
    bool            direction;                                          \
    Oid rangeTypeOID;                                                   \
    TypeCacheEntry *typcache;                                           \
    Int4RangeSet a, b, result, norm_result;                             \
                                                                        \
    if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2)) {PG_RETURN_NULL();} \
    a_range   = PG_GETARG_ARRAYTYPE_P(0);                               \
    b_range   = PG_GETARG_ARRAYTYPE_P(1);                               \
    direction = PG_GETARG_BOOL(2);                                      \
                                                                        \
    rangeTypeOID = ARR_ELEMTYPE(a_range);                               \
    typcache = lookup_type_cache(rangeTypeOID, TYPECACHE_RANGE_INFO);   \
    a = deserialize_ArrayType(a_range, typcache);                       \
    b = deserialize_ArrayType(b_range, typcache);                       \
    result = internal_func (a, b, direction);                           \
    if (a.ranges) pfree(a.ranges);                                      \
    if (b.ranges) pfree(b.ranges);                                      \
    /* Nothing to return */                                             \
    if (result.count == 0 && !result.containsNull) {                    \
        PG_RETURN_NULL();                                               \
    }                                                                   \
    norm_result = normalize(result);                                    \
    output = serialize_ArrayType(norm_result, typcache);                \
    if (result.ranges)                                                  \
        pfree(result.ranges);                                           \
    /*elog(INFO, "Type OID: %u", typcache->type_id);                    \
    elog(INFO, "serialize_ArrayType: set.count = %d", result.count);*/  \
    PG_RETURN_ARRAYTYPE_P(output);                                      \
}

/* assign generic int64 internal either a int32 or int64 */
#define DatumGetInt64Generic(datum, typcache) {                                     \
    ((typcache)->typlen == 4 ? (int64)DatumGetInt32(datum) : DatumGetInt64(datum))  \
}

/*
* template for combining RANGE with mult. Special case on mult=[0,0]. 
* More detail in respective calls and internal_agg_min_max_combine_range_mult func
* NOTE- might need to use separate sentinel value other than isNULL aka isNeutral
*/
#define COMBINE_RANGE_MULT_MINMAX_BODY()                                        \
do {                                                                            \
    RangeType      *range_input, *mult_input, *output;                          \
    Int4Range       range, mult, result;                                        \
    TypeCacheEntry *typcacheRange, *typcacheMult;                               \
                                                                                \
    HANDLE_EITHER_ARG_ISNULL();                                                 \
                                                                                \
    range_input   = PG_GETARG_RANGE_P(0);                                      \
    mult_input    = PG_GETARG_RANGE_P(1);                                       \
    typcacheRange = lookup_type_cache(range_input->rangetypid, TYPECACHE_RANGE_INFO); \
    typcacheMult  = lookup_type_cache(mult_input->rangetypid, TYPECACHE_RANGE_INFO); \
                                                                                \
    range  = deserialize_RangeType(range_input, typcacheRange);                 \
    mult   = deserialize_RangeType(mult_input,  typcacheMult);                  \
    result = internal_agg_min_max_combine_range_mult(range, mult);             \
                                                                                \
    if (result.isNull) PG_RETURN_NULL();                                        \
    output = serialize_RangeType(result, typcacheRange);                        \
    PG_RETURN_RANGE_P(output);                                                  \
} while(0)

/* template for combining SET with mult. Special case on mult=[0,0]. 
* More detail in respective calls and internal_agg_min_max_combine_range_mult func
* NOTE- might need to use separate sentinel value other than isNULL aka isNeutral
*/
#define COMBINE_SET_MULT_MINMAX_BODY()                                          \
do {                                                                            \
    ArrayType      *input, *output;                                             \
    RangeType      *mult_input;                                                 \
    Int4RangeSet    set, result;                                                \
    Int4Range       mult;                                                       \
    TypeCacheEntry *typcache, *typcacheMult;                                    \
                                                                                \
    HANDLE_EITHER_ARG_ISNULL();                                                 \
                                                                                \
    input        = PG_GETARG_ARRAYTYPE_P(0);                                    \
    mult_input   = PG_GETARG_RANGE_P(1);                                        \
                                                                                \
    /* need a RangeType to look up typcache — get it from array element type */ \
    typcache     = lookup_type_cache(ARR_ELEMTYPE(input), TYPECACHE_RANGE_INFO); \
    typcacheMult = lookup_type_cache(mult_input->rangetypid, TYPECACHE_RANGE_INFO);\
                                                                                \
    set    = deserialize_ArrayType(input, typcache);                            \
    mult   = deserialize_RangeType(mult_input, typcacheMult);                   \
    result = internal_agg_min_max_combine_set_mult(set, mult);                  \
    /*handle empty case and null set case*/                                     \
    if (result.count == 0 || (result.count == 0 && result.containsNull)) {      \
        PG_RETURN_NULL();                                                       \
    }                                                                           \
                                                                                \
    output = serialize_ArrayType(result, typcache);                             \
    PG_RETURN_ARRAYTYPE_P(output);                                              \
} while(0)

/*
 * State Transition function for min/max aggregate over a single range.
 * Simply deserializes data, operates on it, and serializes.
 *   - State = Int4Range = [a,b)
 *   - Input = Int4Range = [c,d)
 *   - Return RangeType: [combine_fn(a,c), combine_fn(b,d))
 */
#define DEFINE_AGG_MINMAX_RANGE_TRANSFUNC(func_name, combine_fn)        \
Datum func_name(PG_FUNCTION_ARGS)                                       \
{                                                                       \
    Int4Range state_i4r, input_i4r, result_i4r;                         \
    RangeType *state, *input, *result;                                  \
    TypeCacheEntry *typcache;                                           \
                                                                        \
    /* first call: use the first input as initial state, or non null*/  \
    if (PG_ARGISNULL(0)) {                                              \
        if (PG_ARGISNULL(1)) PG_RETURN_NULL();                         \
        /*othrwise value becomes the state*/                            \
        PG_RETURN_RANGE_P(PG_GETARG_RANGE_P(1));                       \
    }                                                                   \
    /*NULL input: return current state unchanged*/                      \
    if (PG_ARGISNULL(1))                                                \
        PG_RETURN_RANGE_P(PG_GETARG_RANGE_P(0));                       \
                                                                        \
    /*compare existing min/state to the current input*/                 \
    state = PG_GETARG_RANGE_P(0);                                       \
    input = PG_GETARG_RANGE_P(1);                                       \
                                                                        \
    /*return non empty*/                                                \
    if (RangeIsEmpty(state)) PG_RETURN_RANGE_P(input);                 \
    if (RangeIsEmpty(input)) PG_RETURN_RANGE_P(state);                 \
                                                                        \
    typcache = lookup_type_cache(state->rangetypid, TYPECACHE_RANGE_INFO); \
                                                                        \
    /* deserialize, compare, serialize, return */                      \
    state_i4r  = deserialize_RangeType(state, typcache);               \
    input_i4r  = deserialize_RangeType(input, typcache);               \
    result_i4r = combine_fn(state_i4r, input_i4r);                     \
    result     = serialize_RangeType(result_i4r, typcache);            \
    PG_RETURN_RANGE_P(result);                                          \
}

/*
 * State Transition function for min/max aggregate over a rangeset/array.
 * Simply deserializes data, operates on it, and serializes.
 *   - State = ArrayType = {[a,b) ...}       //(implicit) 
 *   - Input = ArrayType = [c,d) ... }
 *   - Return ArrayType: {[min(a,c), min(b,d)) for all ranges}
 */
#define DEFINE_AGG_MINMAX_SET_TRANSFUNC(func_name, combine_fn)                  \
Datum func_name(PG_FUNCTION_ARGS)                                               \
{                                                                               \
    Int4RangeSet    state_i4r, input_i4r, n_input_i4r, result_i4r;             \
    ArrayType      *state, *input, *result;                                     \
    TypeCacheEntry *typcache;                                                   \
                                                                                \
    /* first call: use the first input as initial state, or non null*/          \
    if (PG_ARGISNULL(0)) {                                                      \
        MemoryContext aggcontext, old;                                           \
        ArrayType *copy;                                                        \
                                                                                \
        if (PG_ARGISNULL(1)) PG_RETURN_NULL();                                 \
                                                                                \
        /*othrwise value becomes the state*/                                    \
        if (!AggCheckCallContext(fcinfo, &aggcontext))                          \
            elog(ERROR, #func_name " called in non-aggregate context");         \
                                                                                \
        old  = MemoryContextSwitchTo(aggcontext);                               \
        copy = PG_GETARG_ARRAYTYPE_P_COPY(1);                                  \
        MemoryContextSwitchTo(old);                                             \
        PG_RETURN_ARRAYTYPE_P(copy);                                            \
    }                                                                           \
    /*NULL input: return current state unchanged*/                              \
    if (PG_ARGISNULL(1))                                                        \
        PG_RETURN_ARRAYTYPE_P(PG_GETARG_ARRAYTYPE_P(0));                       \
                                                                                \
    state = PG_GETARG_ARRAYTYPE_P(0);                                           \
    input = PG_GETARG_ARRAYTYPE_P(1);                                           \
    typcache = lookup_type_cache(state->elemtype, TYPECACHE_RANGE_INFO);        \
                                                                                \
    state_i4r = deserialize_ArrayType(state, typcache);                         \
    input_i4r = deserialize_ArrayType(input, typcache);                         \
    n_input_i4r = normalize(input_i4r);                                         \
                                                                                \
    result_i4r = combine_fn(state_i4r, n_input_i4r);                           \
    result  = serialize_ArrayType(result_i4r, typcache);                        \
                                                                                \
    if (state_i4r.ranges) pfree(state_i4r.ranges);                             \
    if (input_i4r.ranges) pfree(input_i4r.ranges);                             \
    if (n_input_i4r.ranges) pfree(n_input_i4r.ranges);                         \
                                                                                \
    PG_RETURN_ARRAYTYPE_P(result);                                              \
}

/*Function declarations*/
int logical_range_helper(RangeType *input1, RangeType *input2, int (*callback)(Int4Range, Int4Range) );
int logical_set_helper(ArrayType *input1, ArrayType *input2, int (*callback)(Int4RangeSet, Int4RangeSet) );
ArrayType* helperFunctions_helper( ArrayType *input, Int4RangeSet (*callback)() );
RangeType* arithmetic_range_helper(RangeType *input1, RangeType *input2, Int4Range (*callback)(Int4Range, Int4Range));
ArrayType* arithmetic_set_helper(ArrayType *input1, ArrayType *input2, Int4RangeSet (*callback)(Int4RangeSet, Int4RangeSet));

Datum get_neutral_element(Oid elementOID, MonoidOp op);

///////////////////////////////////////////////////////////////
 //   ARITHMETIC
///////////////////////////////////////////////////////////////

DEFINE_RANGE_ARITHMETIC_FUNC(range_add, range_add_internal)
DEFINE_RANGE_ARITHMETIC_FUNC(range_subtract, range_subtract_internal)
DEFINE_RANGE_ARITHMETIC_FUNC(range_multiply, range_multiply_internal)
DEFINE_RANGE_ARITHMETIC_FUNC(range_divide, range_divide_internal)

DEFINE_SET_ARITHMETIC_FUNC(set_add, range_set_add_internal)
DEFINE_SET_ARITHMETIC_FUNC(set_subtract, range_set_subtract_internal)
DEFINE_SET_ARITHMETIC_FUNC(set_multiply, range_set_multiply_internal)
DEFINE_SET_ARITHMETIC_FUNC(set_divide, range_set_divide_internal)

///////////////////////////////////////////////////////////////
 //   COMPARISON
///////////////////////////////////////////////////////////////

DEFINE_RANGE_LOGICAL_FUNC(range_gt, range_greater_than)
DEFINE_RANGE_LOGICAL_FUNC(range_gte, range_greater_than_equal)
DEFINE_RANGE_LOGICAL_FUNC(range_lt, range_less_than)
DEFINE_RANGE_LOGICAL_FUNC(range_lte, range_less_than_equal)
DEFINE_RANGE_LOGICAL_FUNC(range_eq, range_equal_internal)

DEFINE_SET_LOGICAL_FUNC(set_gt, set_greater_than)
DEFINE_SET_LOGICAL_FUNC(set_gte, set_greater_than_equal)
DEFINE_SET_LOGICAL_FUNC(set_lt, set_less_than)
DEFINE_SET_LOGICAL_FUNC(set_lte, set_less_than_equal)
DEFINE_SET_LOGICAL_FUNC(set_eq, set_equal_internal)

///////////////////////////////////////////////////////////////
 //   PRUNE FUNCTIONS
///////////////////////////////////////////////////////////////

DEFINE_PRUNE_RANGE_FUNC_COMPARISON(prune_range_lt, prune_lt_internal_range)
DEFINE_PRUNE_RANGE_FUNC_COMPARISON(prune_range_gt, prune_gt_internal_range)
DEFINE_PRUNE_RANGE_FUNC_COMPARISON(prune_range_lte, prune_lte_internal_range)
DEFINE_PRUNE_RANGE_FUNC_COMPARISON(prune_range_gte, prune_gte_internal_range)
DEFINE_PRUNE_RANGE_FUNC_LOGICAL(prune_range_eq, prune_eq_internal_range)
DEFINE_PRUNE_RANGE_FUNC_LOGICAL(prune_range_and, prune_AND_internal_range)
DEFINE_PRUNE_RANGE_FUNC_LOGICAL_OR(prune_range_or, prune_OR_internal_range) // returns a set bc potentially returns 2 ranges

DEFINE_PRUNE_SET_FUNC_COMPARISON(prune_set_lt, prune_lt_set_internal)
DEFINE_PRUNE_SET_FUNC_COMPARISON(prune_set_lte, prune_lte_set_internal)
DEFINE_PRUNE_SET_FUNC_COMPARISON(prune_set_gt, prune_gt_set_internal)
DEFINE_PRUNE_SET_FUNC_COMPARISON(prune_set_gte, prune_gte_set_internal)
DEFINE_PRUNE_SET_FUNC_LOGICAL(prune_set_eq, prune_eq_set_internal)
DEFINE_PRUNE_SET_FUNC_LOGICAL(prune_set_and, prune_AND_internal_set)
DEFINE_PRUNE_SET_FUNC_LOGICAL(prune_set_or, prune_OR_internal_set)

///////////////////////////////////////////////////////////////
 //   HELPER FUNCTIONS
///////////////////////////////////////////////////////////////

// find total num ranges in set
Datum 
array_length(PG_FUNCTION_ARGS)
{
    ArrayType *input;
    Int4RangeSet set;
    TypeCacheEntry *typcache;
    Oid elemType;
    int64 total;

    // check for NULLS. Diff from empty check
    if (PG_ARGISNULL(0)){
        PG_RETURN_NULL();
    }

    input = PG_GETARG_ARRAYTYPE_P(0);

    if (ArrayGetNItems(ARR_NDIM(input), ARR_DIMS(input)) == 0) {
        PG_RETURN_INT64(0);
    }

    elemType = ARR_ELEMTYPE(input);
    typcache = lookup_type_cache(elemType, TYPECACHE_RANGE_INFO);
    set = deserialize_ArrayType(input, typcache);
    total = set.count;
    pfree(set.ranges);
    PG_RETURN_INT64(total);

}

// sum of volume of interval
Datum
range_coverage(PG_FUNCTION_ARGS)
{
    RangeType *input;
    Int4Range r;
    TypeCacheEntry *typcache;

    if (PG_ARGISNULL(0))
        PG_RETURN_NULL();

    input = PG_GETARG_RANGE_P(0);
    typcache = lookup_type_cache(input->rangetypid, TYPECACHE_RANGE_INFO);
    r = deserialize_RangeType(input, typcache);

    if (r.isNull)
        PG_RETURN_INT64(0);

    PG_RETURN_INT64((int64)(r.upper - r.lower));
}

// sum of volume of every interval in set
Datum
set_coverage(PG_FUNCTION_ARGS)
{
    ArrayType *input;
    Int4RangeSet set;
    TypeCacheEntry *typcache;
    Oid elemType;
    int64 total;
    size_t i;

    // check for NULLS. Diff from empty check
    if (PG_ARGISNULL(0)){
        PG_RETURN_NULL();
    }

    input = PG_GETARG_ARRAYTYPE_P(0);

    if (ArrayGetNItems(ARR_NDIM(input), ARR_DIMS(input)) == 0) {
        PG_RETURN_INT64(0);
    }

    elemType = ARR_ELEMTYPE(input);
    typcache = lookup_type_cache(elemType, TYPECACHE_RANGE_INFO);
    set = deserialize_ArrayType(input, typcache);

    total=0;
    for (i = 0; i < set.count; i++) {
        if (!set.ranges[i].isNull)
            total += (int64)((set.ranges[i].upper-1) - set.ranges[i].lower);
    }

    pfree(set.ranges);
    PG_RETURN_INT64(total);
}

// x -> [x,x+1)
Datum
lift_scalar(PG_FUNCTION_ARGS)
{
    Oid rangeTypeOID;
    TypeCacheEntry *typcache;
    int x;
    Int4Range result;
    RangeBound lb, ub;
    RangeType *output;

    // check for NULLS. Diff from empty check
    if (PG_ARGISNULL(0)){
        PG_RETURN_NULL();
    }
    
    x = PG_GETARG_INT32(0);
    result = lift_scalar_local(x);
    
    // can maybe remove use of PRIMARY_DATA_TYPE and instead 
    // get OID of x, and lift that to range type for that OID (if possible ofc)
    rangeTypeOID = TypenameGetTypid(PRIMARY_DATA_TYPE);
    typcache = lookup_type_cache(rangeTypeOID, TYPECACHE_RANGE_INFO);
    
    lb = make_range_bound(result.lower, true, true);
    ub = make_range_bound(result.upper, false, false);
    output = make_range(typcache, &lb, &ub, false, NULL);

    PG_RETURN_RANGE_P(output);
}


// [a,b) -> { [a,b) }
Datum
lift_range(PG_FUNCTION_ARGS)
{
    Oid rangeTypeOID;
    TypeCacheEntry *typcache;
    RangeType* input;
    Int4Range unlifted;
    Int4RangeSet result;
    ArrayType *output;

    // check for NULLS. Diff from empty check
    if (PG_ARGISNULL(0)){
        PG_RETURN_NULL();
    }
    
    rangeTypeOID = TypenameGetTypid(PRIMARY_DATA_TYPE);
    typcache = lookup_type_cache(rangeTypeOID, TYPECACHE_RANGE_INFO);
    
    input = PG_GETARG_RANGE_P(0);
    unlifted = deserialize_RangeType(input, typcache);

    result = lift_range_local(unlifted);
    output = serialize_ArrayType(result, typcache);

    PG_RETURN_ARRAYTYPE_P(output);
}


// Reduce to num_ranges specified in PG_ARG(1).
Datum
set_reduce_size(PG_FUNCTION_ARGS)
// FIXME- might nede to fix the local code for this. need to account for NULL. should be simple fix
{
    ArrayType *inputArray;
    int32 numRangesKeep;
    Oid rangeTypeOID;
    TypeCacheEntry *typcache;
    Int4RangeSet set1;
    Int4RangeSet result;
    ArrayType *output;

    // check for NULLS. Diff from empty check
    if (PG_ARGISNULL(0) || PG_ARGISNULL(1)){
        PG_RETURN_NULL();
    }

    // get args and typcache
    inputArray = PG_GETARG_ARRAYTYPE_P(0);
    numRangesKeep = PG_GETARG_INT32(1);
    rangeTypeOID = ARR_ELEMTYPE(inputArray);
    typcache = lookup_type_cache(rangeTypeOID, TYPECACHE_RANGE_INFO);

    set1 = deserialize_ArrayType(inputArray, typcache);    
    // return NULL if sorted range == NULL. 
    if (set1.count == 0){
        PG_RETURN_NULL();
    }

    // reduce the set to numRangesKeep
    result = reduceSize(set1, numRangesKeep);
    output = serialize_ArrayType(result, typcache);
    
    pfree(set1.ranges);
    pfree(result.ranges);

    PG_RETURN_ARRAYTYPE_P(output);
}

// quicksort by LB and tie break on UB. Appends null if necessary
Datum
set_sort(PG_FUNCTION_ARGS)
{
    ArrayType *inputArray;
    ArrayType *output;
    
    // check for NULLS. Diff from empty check
    if (PG_ARGISNULL(0)){
        PG_RETURN_NULL();
    }

    inputArray = PG_GETARG_ARRAYTYPE_P(0);
    output = helperFunctions_helper(inputArray, sort);

    PG_RETURN_ARRAYTYPE_P(output);
}

// greedily merges and removed any overlap. Returns most effecient lossless representation 
Datum
set_normalize(PG_FUNCTION_ARGS)
{
    ArrayType *inputArray;
    ArrayType *output;
    
    // check for NULLS. Diff from empty check
    if (PG_ARGISNULL(0)){
        PG_RETURN_NULL();
    }

    inputArray = PG_GETARG_ARRAYTYPE_P(0);
    output = helperFunctions_helper(inputArray, normalize);

    PG_RETURN_ARRAYTYPE_P(output);
}

/*
    extendable convenience helper for creating neutral elements based on working data type.
    neutral element naturally differs for sum, min, max and is called in each combine_* helper
*/
Datum 
get_neutral_element(Oid elementOID, MonoidOp op) 
{
    switch (elementOID) {
        case INT4RANGEOID:
            switch (op) {
                case MONOID_SUM: return Int32GetDatum(0);
                case MONOID_MIN: return Int32GetDatum(INT_MAX);
                case MONOID_MAX: return Int32GetDatum(INT_MIN);
            }
            break;
        
        case INT8RANGEOID:
            switch (op) {
                case MONOID_SUM: return Int64GetDatum(0);
                case MONOID_MIN: return Int64GetDatum(PG_INT64_MAX);
                case MONOID_MAX: return Int64GetDatum(PG_INT64_MIN);
            }
            break;
        
        default:
            elog(ERROR, "Neutral element not implemented for OID %u", elementOID);
    }

    return (Datum)(0);  // impossible
}

/*
Takes in 2 parameters: Array: Int4RangeSet, and the function ptr callback: Int4RangeSet function() 
Generally called for helper functions that modify 1 Int4RangeSet param passed in

* Callback must palloc its own data
*/
ArrayType*
helperFunctions_helper(ArrayType *input, Int4RangeSet (*callback)(Int4RangeSet) )
{
    Oid rangeTypeOID;
    TypeCacheEntry *typcache;
    Int4RangeSet set1;
    Int4RangeSet result;
    ArrayType *output;

    rangeTypeOID = ARR_ELEMTYPE(input);
    typcache = lookup_type_cache(rangeTypeOID, TYPECACHE_RANGE_INFO);

    set1 = deserialize_ArrayType(input, typcache);
    result = callback(set1);
    output = serialize_ArrayType(result, typcache);
    
    pfree(set1.ranges);
    pfree(result.ranges);

    return output;
}

/*
Generic Helper for arithmetic operations on RangeTypes.
Deserializes data, performs operation on data, serializes it back to native PG RangeType.
* Parameters(3): 
    -input1 RangeType: Int4Range, NON-NULL
    -input2 RangeType: Int4Range, NON-NULL
    -function ptr callback: Int4Range function()   
* Return(1):
    -RangeType result
*/
RangeType*
arithmetic_range_helper(RangeType *input1, RangeType *input2, Int4Range (*callback)(Int4Range, Int4Range))
{   
    // assign typcache based on RangeType input
    Oid rangeTypeOID1;
    Oid rangeTypeOID2;
    TypeCacheEntry *typcache;
    Int4Range range1; 
    Int4Range range2;
    Int4Range result;
    RangeType *output;

    rangeTypeOID1 = RangeTypeGetOid(input1);
    rangeTypeOID2 = RangeTypeGetOid(input2);
    typcache = lookup_type_cache(rangeTypeOID1, TYPECACHE_RANGE_INFO);

    if (rangeTypeOID1 != rangeTypeOID2) {
        ereport(ERROR,
                (errcode(ERRCODE_DATATYPE_MISMATCH),
                errmsg("range type mismatch in arithmetic operation")));
    }

    range1 = deserialize_RangeType(input1, typcache);
    range2 = deserialize_RangeType(input2, typcache);
    
    // safety check. Should not be necessary bc postgres enforces this already
    if(!validRange(range1) || !validRange(range2)) {
        return make_empty_range(typcache);
    }

    // implemented C function
    result = callback(range1, range2);

    // convert result into RangeType
    output = serialize_RangeType(result, typcache);
    return output;
}

/*
Generic Helper for arithmetic operations on ArrayType of RangeTypes.
Deserializes data, performs operation on data, serializes it back to native PG type.
* Parameters(3): 
    -input1 ArrayType: Int4RangeSet, NON-NULL
    -input2 ArrayType: Int4RangeSet, NON-NULL
    -function ptr callback: Int4RangeSet function()   
* Return(1):
    -ArrayType result
*/
ArrayType*
arithmetic_set_helper(ArrayType *input1, ArrayType *input2, Int4RangeSet (*callback)(Int4RangeSet, Int4RangeSet))
{   
    Oid rangeTypeOID1;
    Oid rangeTypeOID2;
    TypeCacheEntry *typcache;
    Int4RangeSet set1; 
    Int4RangeSet set2;
    Int4RangeSet result;
    ArrayType *output;
    
    // assign typcache based on RangeType input
    rangeTypeOID1 = ARR_ELEMTYPE(input1);
    rangeTypeOID2 = ARR_ELEMTYPE(input2);
    typcache = lookup_type_cache(rangeTypeOID1, TYPECACHE_RANGE_INFO);
    
    if (rangeTypeOID1 != rangeTypeOID2) {
        ereport(ERROR,
                (errcode(ERRCODE_DATATYPE_MISMATCH),
                errmsg("range type mismatch in arithmetic operation")));
    }

    // convert native PG representaion to our local representation
    set1 = deserialize_ArrayType(input1, typcache);
    set2 = deserialize_ArrayType(input2, typcache);

    // handle EMPTY cases
    if(set1.containsNull && set1.count == 1) {
        output = serialize_ArrayType(set2, typcache);
        pfree(set1.ranges);
        pfree(set2.ranges);
        return output;
    }
    else if(set2.containsNull && set2.count == 1) {
        output = serialize_ArrayType(set1, typcache);
        pfree(set1.ranges);
        pfree(set2.ranges);
        return output;
    }

    // callback function in this case is an arithmetic function with params: (Int4RangeSet a, Int4RangeSet b)
    result = callback(set1, set2);
    
    // convert result back to native PG representation
    output = serialize_ArrayType(result, typcache);

    // clean local representation
    pfree(set1.ranges);
    pfree(set2.ranges);
    pfree(result.ranges);

    return output;
}

/*
Generic Helper for logical operations on RangeTypes.
Deserializes data, performs operation on data, returns int (3VL boolean) result
* Parameters(3): 
    -input1 RangeType: Int4Range, NON-NULL
    -input2 RangeType: Int4Range, NON-NULL
    -function ptr callback: Int4Range function()   
* Return(1):
    -int result (3VL)
*/
int 
logical_range_helper(RangeType *input1, RangeType *input2, int (*callback)(Int4Range, Int4Range) )
{   
    Int4Range range1;
    Int4Range range2;
    TypeCacheEntry *typcache;
    int result;
    
    typcache = lookup_type_cache(input1->rangetypid, TYPECACHE_RANGE_INFO);
    if (input1->rangetypid != input2->rangetypid) {
        ereport(ERROR,
                (errcode(ERRCODE_DATATYPE_MISMATCH),
                errmsg("range type mismatch in arithmetic operation")));
    }

    // deserialize, operate, return integer
    range1 = deserialize_RangeType(input1, typcache);
    range2 = deserialize_RangeType(input2, typcache);
    result = callback(range1, range2);
    return result;
}

/*
Generic Helper for logical operations on ArrayTypes of RangeTypes.
Deserializes data, performs operation on data, returns int (3VL boolean) result
* Parameters(3): 
    -input1 ArrayType: Int4Range[], NON-NULL
    -input2 ArrayType: Int4Range[], NON-NULL
    -function ptr callback: Int4RangeSet function()   
* Return(1):
    -int result (3VL)
*/
int 
logical_set_helper(ArrayType *input1, ArrayType *input2, int (*callback)(Int4RangeSet, Int4RangeSet) )
{   
    // assign typcache based on RangeType input
    Oid rangeTypeOID1;
    Oid rangeTypeOID2;
    TypeCacheEntry *typcache;
    Int4RangeSet set1;
    Int4RangeSet set2;
    int result;

    rangeTypeOID1 = ARR_ELEMTYPE(input1);
    rangeTypeOID2 = ARR_ELEMTYPE(input2);
    typcache = lookup_type_cache(rangeTypeOID1, TYPECACHE_RANGE_INFO);
    
    if (rangeTypeOID1 != rangeTypeOID2) {
        ereport(ERROR,
                (errcode(ERRCODE_DATATYPE_MISMATCH),
                errmsg("range type mismatch in arithmetic operation")));
    }

    // deserialize, operate, free, return integer
    set1 = deserialize_ArrayType(input1, typcache);
    set2 = deserialize_ArrayType(input2, typcache);
    result = callback(set1, set2);
    pfree(set1.ranges);
    pfree(set2.ranges);
    return result;
}

///////////////////////////////////////////////////////////////
 //   AGGREGATES
///////////////////////////////////////////////////////////////

///////////////////////
 // SUM 
///////////////////////
/*
// To be called inside a SUM aggregation call. This multiplies the Set and multiplicity together.
// Parameter: ArrayType (data col), RangeType (multiplicity)
// Returns: Combined I4R x Mult result ArrayType Datum as argument to SUM()
*/
Datum
combine_range_mult_sum(PG_FUNCTION_ARGS) 
{
    // inputs/ outputs
    RangeType *input; 
    RangeType *mult_input;
    ArrayType *output;
    
    // working type
    Int4Range input_i4r;
    Int4Range mult_i4r;
    Int4RangeSet lifted;
    Int4RangeSet result;

    // int neutral_element;
    TypeCacheEntry *typcache;
    TypeCacheEntry *typcacheMult;
    
    // handle NULL before assign
    HANDLE_EITHER_ARG_ISNULL();
    
    input = PG_GETARG_RANGE_P(0);
    mult_input = PG_GETARG_RANGE_P(1);
    
    // handle empty 
    if (RangeIsEmpty(input) || RangeIsEmpty(mult_input)) {
        PG_RETURN_NULL();
    }

    typcache = lookup_type_cache(input->rangetypid, TYPECACHE_RANGE_INFO);
    typcacheMult = lookup_type_cache(mult_input->rangetypid, TYPECACHE_RANGE_INFO);

    // deserialize, operate on, serialize, return
    input_i4r = deserialize_RangeType(input, typcache);
    mult_i4r = deserialize_RangeType(mult_input, typcacheMult);
    lifted = lift_range_local(input_i4r);

    // normalizes before return
    result = internal_agg_sum_combine_set_mult(lifted, mult_i4r);
    
    // handle empty sets and null set
    if (result.count == 0 || (result.count == 1 && result.containsNull)) {
        PG_RETURN_NULL();
    }

    output = serialize_ArrayType(result, typcache);
    PG_RETURN_ARRAYTYPE_P(output);
}

/*
// To be called inside a SUM aggregation call. This multiplies the Set and multiplicity together.
// neutral_element is the only difference between min/max implementation. This value is HARDCODED //FIXME
// Parameter: ArrayType (data col), RangeType (multiplicity)
// Returns: a ArrayType Datum as argument to SUM()
*/
Datum
combine_set_mult_sum(PG_FUNCTION_ARGS) 
{
    // inputs/ outputs
    ArrayType *set_input;
    RangeType *mult_input;
    ArrayType *output;
    
    // working type
    Int4Range mult;
    Int4RangeSet set1;
    Int4RangeSet result;
    
    // int neutral_element;
    TypeCacheEntry *typcacheSet;
    TypeCacheEntry *typcacheMult;
    
    // handle NULL before assign
    HANDLE_EITHER_ARG_ISNULL();
    
    set_input = PG_GETARG_ARRAYTYPE_P(0);
    mult_input = PG_GETARG_RANGE_P(1);

    // handle empty val or mult. NOT same as {empty} case lol
    if (ArrayGetNItems(ARR_NDIM(set_input), ARR_DIMS(set_input)) == 0 || RangeIsEmpty(mult_input)) {
        PG_RETURN_NULL();
    }

    typcacheSet = lookup_type_cache(set_input->elemtype, TYPECACHE_RANGE_INFO);
    typcacheMult = lookup_type_cache(mult_input->rangetypid, TYPECACHE_RANGE_INFO);

    // deserialize, operate on, serialize, return
    set1 = deserialize_ArrayType(set_input, typcacheSet);
    mult = deserialize_RangeType(mult_input, typcacheMult);

    // handle {empty} case != {} != NULL 
    if (set1.count == 0) {
        PG_RETURN_NULL();
    }

    // output is normalized before returned
    result = internal_agg_sum_combine_set_mult(set1, mult);
    
    // handle empty sets and null set
    if (result.count == 0 || (result.count == 1 && result.containsNull)) {
        PG_RETURN_NULL();
    }
    
    output = serialize_ArrayType(result, typcacheSet);
    
    pfree(set1.ranges);
    pfree(result.ranges);
    
    PG_RETURN_ARRAYTYPE_P(output);
}

/*
*   transition function for sum(combine_range_mult_sum(data, mult), resizetrigger, sizelimit)
*   
* Parameters [4]:
*   - RangeType: existing state
*   - RangeType: current state (result of combine_range_mult_sum(data, mult))
*   - Integer: resize trigger
*   - Integer: size limit
*   
* Returns [1]:
*   - Datum (pointer to RangeType result) 
*
* NOTE- dont think we even need reduce params here bc results always size 1 since its ranges. perhaps keep for standardization btwn set/range??
*/
Datum
agg_sum_range_transfunc(PG_FUNCTION_ARGS)
{
    RangeType *state;
    RangeType *input;
    RangeType *result;
    
    // first call: use the first input as initial state, or non null
    if (PG_ARGISNULL(0)){
        if (PG_ARGISNULL(1)){
            PG_RETURN_NULL();
        }
        // othrwise value becomes the state
        PG_RETURN_RANGE_P(PG_GETARG_RANGE_P(1));
    }

    // not first call, we do aggregate...
    // case- NULL input: return current state unchanged
    if(PG_ARGISNULL(1)) {
        PG_RETURN_RANGE_P(PG_GETARG_RANGE_P(0));
    }
    
    // otherwise, get arguments, call helper to get result, check to normalize after
    state = PG_GETARG_RANGE_P(0);
    input = PG_GETARG_RANGE_P(1);

    result = arithmetic_range_helper(state, input, range_add_internal);

    PG_RETURN_RANGE_P(result);
}

/*
*   transition function for sum(combine_set_mult_sum(data, mult), resizetrigger, sizelimit)
*   
* Parameters [4]:
*   - SumAggState: (internal type)
*   - ArrayType: current state (result of combine_set_mult_sum(data, mult))
*   - Integer: resize trigger
*   - Integer: size limit
*   
* Returns [1]:
*   - Datum (pointer to SumAggState) 
*/
Datum
agg_sum_set_transfunc(PG_FUNCTION_ARGS)
{
    MemoryContext aggcontext;
    MemoryContext oldcontext;
    SumAggState *state;
    ArrayType *currSet;
    TypeCacheEntry *typcache;
    Int4RangeSet inputSet, combined, reduced;
    
    if (!AggCheckCallContext(fcinfo, &aggcontext))
        elog(ERROR, "agg_sum_set_transfunc called in non-aggregate context");
    
    // first call, state is NULL
    if (PG_ARGISNULL(0)) {
        // check for NULL input param, or empty
        if (PG_ARGISNULL(1)) {
            PG_RETURN_NULL();
        }
        
        currSet = PG_GETARG_ARRAYTYPE_P(1);
        
        // empty set, continue on until non empty
        if (ArrayGetNItems(ARR_NDIM(currSet), ARR_DIMS(currSet)) == 0) {
            PG_RETURN_NULL();
        }
        
        // switch to aggregate memory context for persistent allocations
        oldcontext = MemoryContextSwitchTo(aggcontext);
        
        typcache = lookup_type_cache(ARR_ELEMTYPE(currSet), TYPECACHE_RANGE_INFO);
        // internal state init
        state = (SumAggState *) palloc0(sizeof(SumAggState));
        state->ranges = deserialize_ArrayType(currSet, typcache);
        // set only once on first call
        state->resizeTrigger = PG_GETARG_INT32(2);
        state->sizeLimit = PG_GETARG_INT32(3);
        state->elemTypeOID = ARR_ELEMTYPE(currSet);
        
        // return to callers context
        MemoryContextSwitchTo(oldcontext);
        
        PG_RETURN_POINTER(state);
    }
    
    // otherwise merge into existing state
    state = (SumAggState *) PG_GETARG_POINTER(0);

    if (!PG_ARGISNULL(1)) {
        currSet = PG_GETARG_ARRAYTYPE_P(1);
        typcache = lookup_type_cache(ARR_ELEMTYPE(currSet), TYPECACHE_RANGE_INFO);

        // empty check
        if (ArrayGetNItems(ARR_NDIM(currSet), ARR_DIMS(currSet)) == 0) {
            PG_RETURN_POINTER(state);
        }
        
        // deserialize input in current context (freed later)
        inputSet = deserialize_ArrayType(currSet, typcache);
        
        // agg context persists mem
        oldcontext = MemoryContextSwitchTo(aggcontext);
        combined = range_set_add_internal(state->ranges, inputSet);
        
        // free old ranges
        if (state->ranges.ranges != NULL) {
            pfree(state->ranges.ranges);
        }
        
        // check reduce size
        if (combined.count >= state->resizeTrigger) {
            reduced = reduceSize(combined, state->sizeLimit);
            pfree(combined.ranges);
            state->ranges = reduced;
        }
        else {
            state->ranges = combined;
        }
        
        MemoryContextSwitchTo(oldcontext);
        
        // free previous memory context
        pfree(inputSet.ranges);
    }
    
    PG_RETURN_POINTER(state);
}

/*
    Reduce one last time if needed and Convert Internal type(SumAggState) to ArrayType Datum.
*/
Datum
agg_sum_set_finalfunc(PG_FUNCTION_ARGS)
{
    SumAggState *state;
    ArrayType *result;
    TypeCacheEntry *typcache;
    Int4RangeSet normalized, reduced;
    Oid elemTypeOID;
    
    if (PG_ARGISNULL(0)) {
        PG_RETURN_NULL();
    }
    
    state = (SumAggState*) PG_GETARG_POINTER(0);

    elemTypeOID = state->elemTypeOID;
    typcache = lookup_type_cache(elemTypeOID, TYPECACHE_RANGE_INFO);

    // empty state
    if (state->ranges.count == 0) {
        PG_RETURN_ARRAYTYPE_P(construct_empty_array(elemTypeOID));
    }
    
    // always normalize on final call
    normalized = normalize(state->ranges);

    // optionally reduce if the normalized result is still smaller than the size limit
    if (state->ranges.count >= state->resizeTrigger) {
        reduced = reduceSize(normalized, state->sizeLimit);
        pfree(normalized.ranges);
        result = serialize_ArrayType(reduced, typcache);
        pfree(reduced.ranges);
        PG_RETURN_ARRAYTYPE_P(result);
    }
    else {
        result = serialize_ArrayType(normalized, typcache);
        pfree(normalized.ranges);
    }
    
    PG_RETURN_ARRAYTYPE_P(result);
}


/*
* ** USED FOR INTERNAL TESTING SUITE **
*   transition function for sum_metrics(combine_set_mult_sum(data, mult), resizetrigger, sizelimit)
*   
* Parameters [4]:
*   - SumAggStateMetrics: (internal type)
*   - ArrayType: current state (result of combine_set_mult_sum(data, mult))
*   - Integer: resize trigger
*   - Integer: size limit
*   
* Returns [1]:
*   - Datum (pointer to SumAggStateMetrics) 
*/
Datum
agg_sum_set_transfunc_metrics(PG_FUNCTION_ARGS)
{
    MemoryContext aggcontext;
    MemoryContext oldcontext;
    SumAggStateMetrics *state;
    ArrayType *currSet;
    TypeCacheEntry *typcache;
    Int4RangeSet inputSet, combined, normalized, newState;
    
    if (!AggCheckCallContext(fcinfo, &aggcontext))
        elog(ERROR, "agg_sum_set_transfunc_metrics called in non-aggregate context");
    
    // first call, state is NULL
    if (PG_ARGISNULL(0)) {

        // check for NULL input param, or empty
        if (PG_ARGISNULL(1)) {
            PG_RETURN_NULL();
        }
        
        currSet = PG_GETARG_ARRAYTYPE_P(1);
        typcache = lookup_type_cache(ARR_ELEMTYPE(currSet), TYPECACHE_RANGE_INFO);
        
        // empty set, continue on until non empty
        if (ArrayGetNItems(ARR_NDIM(currSet), ARR_DIMS(currSet)) == 0) {
            PG_RETURN_NULL();
        }

        // switch to aggregate memory context for persistent allocations
        oldcontext = MemoryContextSwitchTo(aggcontext);
        
        // internal state init
        state = (SumAggStateMetrics *) palloc0(sizeof(SumAggStateMetrics));
        state->ranges = deserialize_ArrayType(currSet, typcache);
        state->resizeTrigger = PG_GETARG_INT32(2);
        state->sizeLimit = PG_GETARG_INT32(3);
        state->callNormalize = PG_GETARG_BOOL(4);
        state->elemTypeOID = ARR_ELEMTYPE(currSet);
        
        state->reduceCalls = 0;
        state->combineCalls = 1;
        state->maxIntervalCount = state->ranges.count;
        state->totalIntervalCount = state->ranges.count;
        state->minEffectiveIntervalCount = 0;
        state->convergedToTotSize = 0;
        
        // need to return to callers context
        MemoryContextSwitchTo(oldcontext);
        
        PG_RETURN_POINTER(state);
    }
    
    // otherwise merge into existing state
    state = (SumAggStateMetrics *) PG_GETARG_POINTER(0);

    if (!PG_ARGISNULL(1)) {
        currSet = PG_GETARG_ARRAYTYPE_P(1);
        typcache = lookup_type_cache(ARR_ELEMTYPE(currSet), TYPECACHE_RANGE_INFO);

        // empty check
        if (ArrayGetNItems(ARR_NDIM(currSet), ARR_DIMS(currSet)) == 0) {
            PG_RETURN_POINTER(state);
        }
        
        // deserialize input in current context (freed later)
        inputSet = deserialize_ArrayType(currSet, typcache);
        
        // agg context persists data
        oldcontext = MemoryContextSwitchTo(aggcontext);
        combined = range_set_add_internal(state->ranges, inputSet);
        
        // metadata
        state->combineCalls++;
        state->totalIntervalCount += combined.count;
        if (combined.count > state->maxIntervalCount) {
            state->maxIntervalCount = combined.count;
        }

        // free old ranges
        if (state->ranges.ranges != NULL) {
            pfree(state->ranges.ranges);
        }
        
        // check reduce size
        if (combined.count >= state->resizeTrigger) {
            newState = reduceSize(combined, state->sizeLimit);
            pfree(combined.ranges);
            state->reduceCalls++;
        }
        else {
            newState = combined;
            // NOTE test normalize here and not here    // update- what???
        }

        if (state->callNormalize) {
            normalized = normalize(newState);
            if (newState.ranges != combined.ranges)
                pfree(newState.ranges);

            newState = normalized;
        }
        state->ranges = newState;
        
        // free previous memory context
        MemoryContextSwitchTo(oldcontext);        
        pfree(inputSet.ranges);
    }
    
    PG_RETURN_POINTER(state);
}

/*
* ** USED FOR INTERNAL TESTING SUITE **
    returns a composite type containing:
    * result
    * resize trigger
    * resize limit
    * number of calls to reduce
    * peak number of intervals seen
    * total intervals count
    * number of times merged new input = num rows in dataset
*/
Datum
agg_sum_set_finalfunc_metrics(PG_FUNCTION_ARGS)
{
    SumAggStateMetrics *state;
    Int4RangeSet normalized, reduced;
    Datum values[9];
    bool nulls[9] = {false,false,false,false,false,false,false,false,false};
    HeapTuple tuple;
    TupleDesc tupdesc;
    ArrayType *arr;
    Oid elemTypeOID;
    TypeCacheEntry *typcache;
    long currentSpan, currentCount;

    if (PG_ARGISNULL(0)) {
        PG_RETURN_NULL();
    }
    
    state = (SumAggStateMetrics*) PG_GETARG_POINTER(0);
    
    elemTypeOID = state->elemTypeOID;
    if (elemTypeOID == InvalidOid)
        elog(ERROR, "OID: %d type not found in catalog", elemTypeOID);
    typcache = lookup_type_cache(elemTypeOID, TYPECACHE_RANGE_INFO);

    // always normalize on final call bc why have reducndancy in output
    normalized = normalize(state->ranges);
    
    // optionally reduce further (potentially increasing cover) if the normalized result is still smaller than the users desired size limit
    if (normalized.count >= state->resizeTrigger) {
        reduced = reduceSize(normalized, state->sizeLimit);
        pfree(normalized.ranges);
        arr = serialize_ArrayType(reduced, typcache);
        currentSpan = totalSpan(reduced);
        currentCount = reduced.count;
        pfree(reduced.ranges);
    }
    else {
        arr = serialize_ArrayType(normalized, typcache);
        currentSpan = totalSpan(normalized);
        currentCount = normalized.count;
        pfree(normalized.ranges);
    }

    // track metadata
    state->minEffectiveIntervalCount = currentCount;
    state->convergedToTotSize = currentSpan;

    values[0] = PointerGetDatum(arr);
    values[1] = Int64GetDatum(state->resizeTrigger);
    values[2] = Int64GetDatum(state->sizeLimit);
    values[3] = Int64GetDatum(state->reduceCalls);
    values[4] = Int64GetDatum(state->maxIntervalCount);
    values[5] = Int64GetDatum(state->totalIntervalCount);
    values[6] = Int64GetDatum(state->combineCalls);
    values[7] = Int64GetDatum(state->minEffectiveIntervalCount);
    values[8] = Int64GetDatum(state->convergedToTotSize);

    // get the composite tuple descriptor
    if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)
        elog(ERROR, "return type must be composite");
    BlessTupleDesc(tupdesc);

    tuple = heap_form_tuple(tupdesc, values, nulls);
    return HeapTupleGetDatum(tuple);                        // cmoposite must be deserialzed again in python code to get specific metrics
}

////////////////////////////////////////////////////////////////////////////////////
//////// MIN/MAX ///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/*
* To be called inside a MIN/MAX RANGE aggregation call. If mult is [0,0], then we return NULL range == neutral value (i think)
* Returns: a RangeType Datum as argument to MIN() or MAX()
*/
Datum combine_range_mult_min(PG_FUNCTION_ARGS) { 
    COMBINE_RANGE_MULT_MINMAX_BODY(); 
}

/*
* To be called inside a MIN/MAX RANGE aggregation call. If mult is [0,0], then we return NULL range == neutral value (i think)
* Returns: a RangeType Datum as argument to MIN() or MAX()
*/
Datum combine_range_mult_max(PG_FUNCTION_ARGS) { 
    COMBINE_RANGE_MULT_MINMAX_BODY(); 
}

// range transition functions
DEFINE_AGG_MINMAX_RANGE_TRANSFUNC(agg_min_range_transfunc, min_range)
DEFINE_AGG_MINMAX_RANGE_TRANSFUNC(agg_max_range_transfunc, max_range)

/*
* To be called inside a MIN/MAX SET aggregation call. This multiplies the Set and multiplicity together.
* Args: ArrayType (data col), RangeType (multiplicity)
* Returns: a ArrayType Datum as argument to MIN() or MAX()
*/
Datum combine_set_mult_min(PG_FUNCTION_ARGS) { 
    COMBINE_SET_MULT_MINMAX_BODY(); 
}

/*
* To be called inside a MIN/MAX SET aggregation call. This multiplies the Set and multiplicity together.
* Args: ArrayType (data col), RangeType (multiplicity)
* Returns: a ArrayType Datum as argument to MIN() or MAX()
*/
Datum combine_set_mult_max(PG_FUNCTION_ARGS) { 
    COMBINE_SET_MULT_MINMAX_BODY(); 
}

// set   transition functions
DEFINE_AGG_MINMAX_SET_TRANSFUNC(agg_min_set_transfunc, min_rangeSet);
DEFINE_AGG_MINMAX_SET_TRANSFUNC(agg_max_set_transfunc, max_rangeSet);

// finalfunc simply just normalizes the result. compresses any ranges if possible (remove redundancy)
Datum 
agg_min_max_set_finalfunc(PG_FUNCTION_ARGS)
{
    ArrayType *inputArray;
    ArrayType *output;

    // check for NULLS. Diff from empty check
    if (PG_ARGISNULL(0)){
        PG_RETURN_NULL();
    }
    
    inputArray = PG_GETARG_ARRAYTYPE_P(0);

    // empty set
    if (ArrayGetNItems(ARR_NDIM(inputArray), ARR_DIMS(inputArray)) == 0) {
        PG_RETURN_NULL();
    }

    output = helperFunctions_helper(inputArray, normalize);
    PG_RETURN_ARRAYTYPE_P(output);
}


////////////////////////////////////////////////////////////////////////////////////
//////// COUNT /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/*
 * only necessary for multiplicty (RangeType). Takes in mult as param and counts the total number of possible ranges
 * basically just takes sum of multiplicity col
*/
Datum
agg_count_transfunc(PG_FUNCTION_ARGS)
{
    RangeType *state;
    RangeType *input;
    RangeType *result;
    
    // first call: use the first input as initial state, or non null
    if (PG_ARGISNULL(0)){
        if (PG_ARGISNULL(1)){
            PG_RETURN_NULL();
        }
        // othrwise value becomes the state
        PG_RETURN_RANGE_P(PG_GETARG_RANGE_P(1));
    }

    // NULL input: return current state unchanged
    if(PG_ARGISNULL(1)) {
        PG_RETURN_RANGE_P(PG_GETARG_RANGE_P(0));
    }
    
    // get arguments, call helper to get result, check to normalize after
    state = PG_GETARG_RANGE_P(0);
    input = PG_GETARG_RANGE_P(1);

    result = arithmetic_range_helper(state, input, range_add_internal);

    PG_RETURN_RANGE_P(result);
}

////////////////////////////////////////////////////////////////////////////////////
//////// AVG ///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/*
 * State Transition function for AVG aggregate over a range.
 * Tracks sum state and count state separately and then handles avg in finalfunc
 *   - State: = internal = AvgAggState_range 
 *   - Input: = RangeType val, RangeType mult
 *   - Return: State = internal = AvgAggState_range
 */
Datum 
agg_avg_range_transfunc(PG_FUNCTION_ARGS)
{
    MemoryContext aggcontext, oldcontext;
    AvgAggState_range *state;
    RangeType *data, *mult;
    Int4Range d, m, combined;
    TypeCacheEntry *typcache;

    if (!AggCheckCallContext(fcinfo, &aggcontext)) {
        elog(ERROR, "avg_range_transfunc called in non-aggregate context");
    }

    // ignore NULL rows
    if (PG_ARGISNULL(1) || PG_ARGISNULL(2))
    {
        if (PG_ARGISNULL(0)) {
            PG_RETURN_NULL();
        }
        PG_RETURN_POINTER(PG_GETARG_POINTER(0));
    }
    
    // get current states values
    data = PG_GETARG_RANGE_P(1);
    mult = PG_GETARG_RANGE_P(2);
    typcache = lookup_type_cache(data->rangetypid, TYPECACHE_RANGE_INFO);
    d = deserialize_RangeType(data, typcache);
    m = deserialize_RangeType(mult, typcache);
    combined = internal_agg_min_max_combine_range_mult(d, m);

    // first call: use the first input as initial state, or non null
    if (PG_ARGISNULL(0)){    
        // switch to aggregate memory context for persistent allocations
        oldcontext = MemoryContextSwitchTo(aggcontext);
        
        state = (AvgAggState_range *) palloc0(sizeof(AvgAggState_range));
        state->sum = combined;
        state->count = m;
        state->elemTypeOID = data->rangetypid;
        
        MemoryContextSwitchTo(oldcontext);      // return to callers context
        PG_RETURN_POINTER(state);
    }

    // otherwise:  merge current tuple into existing state (ARG(0))
    state = (AvgAggState_range *) PG_GETARG_POINTER(0);
    oldcontext = MemoryContextSwitchTo(aggcontext);
    state->sum = range_add_internal(state->sum, combined);
    state->count = range_add_internal(state->count, m);
    MemoryContextSwitchTo(oldcontext);      // return to callers context
    
    PG_RETURN_POINTER(state);
}

/*
 * Final Transition function for AVG aggregate over a range.
 * simply divides internal.sum by internal.count
 *   - State: = internal = AvgAggState_range 
 *   - Input: = RangeType val, RangeType mult
 *   - Return: State = internal = AvgAggState_range
 */
Datum 
agg_avg_range_finalfunc(PG_FUNCTION_ARGS)
{
    AvgAggState_range *state;
    TypeCacheEntry *typcache;
    Int4Range avg;
    RangeType *result;

    if (PG_ARGISNULL(0)) {
        PG_RETURN_NULL();
    }

    state = (AvgAggState_range*) PG_GETARG_POINTER(0);
    avg = range_divide_internal(state->sum, state->count);
    typcache = lookup_type_cache(state->elemTypeOID, TYPECACHE_RANGE_INFO);
    
    result = serialize_RangeType(avg, typcache);
    PG_RETURN_RANGE_P(result);
}

/*
 * State Transition function for AVG aggregate over a SET
 * Tracks sum state and count state separately and then handles avg in finalfunc
 *   - State: = internal = AvgAggState_range 
 *   - Input: = RangeType val, RangeType mult
 *   - Return: State = internal = AvgAggState_range
 */
Datum 
agg_avg_set_transfunc(PG_FUNCTION_ARGS)
{
    MemoryContext aggcontext, oldcontext;
    ArrayType *data;
    RangeType *mult;
    Int4RangeSet curr, combined;
    Int4Range m;
    AvgAggState_set *state;
    TypeCacheEntry *typcacheSet, *typcacheMult;

    if (!AggCheckCallContext(fcinfo, &aggcontext)) {
        elog(ERROR, "avg_set_transfunc called in non-aggregate context");
    }

    // ignore NULL rows
    if (PG_ARGISNULL(1) || PG_ARGISNULL(2))
    {
        if (PG_ARGISNULL(0)) {
            PG_RETURN_NULL();
        }
        PG_RETURN_POINTER(PG_GETARG_POINTER(0));
    }
    
    // get curr State values
    data = PG_GETARG_ARRAYTYPE_P(1);
    mult = PG_GETARG_RANGE_P(2);
    typcacheSet = lookup_type_cache(ARR_ELEMTYPE(data), TYPECACHE_RANGE_INFO);
    typcacheMult = lookup_type_cache(mult->rangetypid, TYPECACHE_RANGE_INFO);
    curr = deserialize_ArrayType(data, typcacheSet);
    m = deserialize_RangeType(mult, typcacheMult);
    combined = internal_agg_sum_combine_set_mult(curr, m);

    // first call: use the first input as initial state, or non null
    if (PG_ARGISNULL(0)){    
        // switch to aggregate memory context for persistent allocations
        oldcontext = MemoryContextSwitchTo(aggcontext);
        
        // create internal state type and assign it
        state = (AvgAggState_set *) palloc0(sizeof(AvgAggState_set));
        state->sum = combined;
        state->count = m;
        state->elemTypeOID = data->elemtype;
        
        MemoryContextSwitchTo(oldcontext);      // return to callers context
        PG_RETURN_POINTER(state);
    }

    // otherwise:  merge current tuple into existing state (ARG(0))
    state = (AvgAggState_set *) PG_GETARG_POINTER(0);
    oldcontext = MemoryContextSwitchTo(aggcontext);
    state->sum = range_set_add_internal(state->sum, combined);
    state->count = range_add_internal(state->count, m);
    MemoryContextSwitchTo(oldcontext);

    // free temporaries allocated in caller context
    if (curr.ranges)    pfree(curr.ranges);
    if (combined.ranges) pfree(combined.ranges);

    PG_RETURN_POINTER(state);
}

/*
 * Final Transition function for AVG aggregate over a SET.
 * simply divides internal.sum by internal.count
 *   - State: = internal = AvgAggState_range 
 *   - Input: = RangeType val, RangeType mult
 *   - Return: State = internal = AvgAggState_range
 */
Datum
agg_avg_set_finalfunc(PG_FUNCTION_ARGS)
{
    AvgAggState_set *state;
    TypeCacheEntry *typcache;
    Int4RangeSet avg, liftedCount;
    ArrayType *result;

    if (PG_ARGISNULL(0)) {
        PG_RETURN_NULL();
    }

    state = (AvgAggState_set *) PG_GETARG_POINTER(0);

    // empty state return 
    if (state->sum.count == 0) {
        PG_RETURN_NULL();
    }
        
    typcache = lookup_type_cache(state->elemTypeOID, TYPECACHE_RANGE_INFO);
    liftedCount = lift_range_local(state->count);       // count == mult i4r here
    avg = range_set_divide_internal(state->sum, liftedCount);
    result = serialize_ArrayType(avg, typcache);
    
    if (liftedCount.ranges) pfree(liftedCount.ranges);
    if (avg.ranges) pfree(avg.ranges);

    PG_RETURN_ARRAYTYPE_P(result);
}
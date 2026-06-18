-- `complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION i4r_audb_extension" to load this file. \quit

----------------------------------------------------------------------------
---------------------------Arithemtic functions-----------------------------
----------------------------------------------------------------------------

-- range_add takes 2 int4range types and returns the sum
CREATE FUNCTION range_add(a int4range, b int4range ) 
RETURNS int4range
AS 'MODULE_PATHNAME', 'range_add'
LANGUAGE c;
-- LANGUAGE c STRICT VOLATILE;

-- range_subtract takes 2 int4range types and returns the difference
CREATE FUNCTION range_subtract(a int4range, b int4range ) 
RETURNS int4range
AS 'MODULE_PATHNAME', 'range_subtract'
LANGUAGE c;

-- range_multiply takes 2 int4range types and returns the product
CREATE FUNCTION range_multiply(a int4range, b int4range ) 
RETURNS int4range
AS 'MODULE_PATHNAME', 'range_multiply'
LANGUAGE c;

-- range_divide takes 2 int4range types and returns the divded result
CREATE FUNCTION range_divide(a int4range, b int4range ) 
RETURNS int4range
AS 'MODULE_PATHNAME', 'range_divide'
LANGUAGE c;

-- set_add takes 2 arrays of int4range types and returns an array with added results
CREATE FUNCTION set_add(a int4range[], b int4range[])
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'set_add'
LANGUAGE c;

-- set_subtract takes 2 arrays of int4range types and returns an array with subtracted results
CREATE FUNCTION set_subtract(a int4range[], b int4range[])
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'set_subtract'
LANGUAGE c;

-- set_multiply takes 2 arrays of int4range types and returns an array with subtracted results
CREATE FUNCTION set_multiply(a int4range[], b int4range[])
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'set_multiply'
LANGUAGE c;

-- set_divide takes 2 arrays of int4range types and returns an array with subtracted results
CREATE FUNCTION set_divide(a int4range[], b int4range[])
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'set_divide'
LANGUAGE c;


-- ----------------------------------------------------------------------------
-- -------------------------Logical Operator functions-------------------------
-- ----------------------------------------------------------------------------

CREATE FUNCTION range_lt(a int4range, b int4range)
RETURNS boolean
AS 'MODULE_PATHNAME', 'range_lt'
LANGUAGE c;

CREATE FUNCTION range_lte(a int4range, b int4range)
RETURNS boolean
AS 'MODULE_PATHNAME', 'range_lte'
LANGUAGE c;

CREATE FUNCTION range_gt(a int4range, b int4range)
RETURNS boolean
AS 'MODULE_PATHNAME', 'range_gt'
LANGUAGE c;

CREATE FUNCTION range_gte(a int4range, b int4range)
RETURNS boolean
AS 'MODULE_PATHNAME', 'range_gte'
LANGUAGE c;

CREATE FUNCTION range_eq(a int4range, b int4range)
RETURNS boolean
AS 'MODULE_PATHNAME', 'range_eq'
LANGUAGE c;

-- set_lt takes 2 arrays of int4range types and returns bool result of logical expression
CREATE FUNCTION set_lt(a int4range[], b int4range[])
RETURNS boolean
AS 'MODULE_PATHNAME', 'set_lt'
LANGUAGE c;

-- set_lte takes 2 arrays of int4range types and returns bool result of logical expression
CREATE FUNCTION set_lte(a int4range[], b int4range[])
RETURNS boolean
AS 'MODULE_PATHNAME', 'set_lte'
LANGUAGE c;

-- set_gt takes 2 arrays of int4range types and returns bool result of logical expression
CREATE FUNCTION set_gt(a int4range[], b int4range[])
RETURNS boolean
AS 'MODULE_PATHNAME', 'set_gt'
LANGUAGE c;

-- set_gte takes 2 arrays of int4range types and returns bool result of logical expression
CREATE FUNCTION set_gte(a int4range[], b int4range[])
RETURNS boolean
AS 'MODULE_PATHNAME', 'set_gte'
LANGUAGE c;

-- equal takes 2 arrays of int4range types and returns bool result of logical expression
CREATE FUNCTION set_eq(a int4range[], b int4range[])
RETURNS boolean
AS 'MODULE_PATHNAME', 'set_eq'
LANGUAGE c;

------------------------------------------------------------------------------
--------------------------------Helper functions------------------------------
------------------------------------------------------------------------------

-- find total num ranges in set
CREATE FUNCTION array_length(set int4range[])
RETURNS int4
AS 'MODULE_PATHNAME', 'array_length'
LANGUAGE c;

-- find total volume of interval
CREATE FUNCTION range_coverage(range int4range)
RETURNS int4
AS 'MODULE_PATHNAME', 'range_coverage'
LANGUAGE c;

-- find total volume of every inteval in set
CREATE FUNCTION set_coverage(set int4range[])
RETURNS int4
AS 'MODULE_PATHNAME', 'set_coverage'
LANGUAGE c;

-- lift takes 1 int32 and returns its equivallent Int4Range 
CREATE FUNCTION lift_scalar(val int4)
RETURNS int4range
AS 'MODULE_PATHNAME', 'lift_scalar'
LANGUAGE c;

-- lift takes 1 int4range and returns its equivallent Int4RangeSet: int4range[] 
CREATE FUNCTION lift_range(a int4range)
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'lift_range'
LANGUAGE c;

-- set_reduce_size takes 1 array of int4range, and an integer and returns reduced size array of int4range
CREATE FUNCTION set_reduce_size(a int4range[], numRangesKeep integer)
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'set_reduce_size'
LANGUAGE c;

-- set_sort takes 1 array of int4range, and sorts input 
CREATE FUNCTION set_sort(a int4range[])
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'set_sort'
LANGUAGE c;

-- set_normalize takes 1 array of int4range, and merges contained ranges
CREATE FUNCTION set_normalize(a int4range[])
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'set_normalize'
LANGUAGE c;


----------------------------------------------------------------------------
-------------------------------Aggregates-----------------------------------

---------- SUM -----------

CREATE FUNCTION combine_range_mult_sum(range int4range, mult int4range) 
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'combine_range_mult_sum'
LANGUAGE c;

CREATE FUNCTION agg_sum_range_transfunc(int4range, int4range) 
RETURNS int4range
AS 'MODULE_PATHNAME', 'agg_sum_range_transfunc'
LANGUAGE c;

-- not used anymore because combine_range_mult_sum returns int4range[]. thus it makes more sense to use i4r[] sum()
create aggregate sum (int4range)
(
    stype = int4range,
    sfunc = agg_sum_range_transfunc
);

CREATE FUNCTION combine_set_mult_sum(set int4range[], mult int4range) 
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'combine_set_mult_sum'
LANGUAGE c;

CREATE FUNCTION agg_sum_set_transfunc(internal, set int4range[], resizeTrigger integer, reduceToSize integer) 
RETURNS internal
AS 'MODULE_PATHNAME', 'agg_sum_set_transfunc'
LANGUAGE c;

CREATE FUNCTION agg_sum_set_finalfunc(internal) 
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'agg_sum_set_finalfunc'
LANGUAGE c;

create aggregate sum (int4range[], resizeTrigger integer, reduceToSize integer) 
(
    stype = internal,
    sfunc = agg_sum_set_transfunc,
    finalfunc = agg_sum_set_finalfunc
);

-- used for testing suite. Tracks more detailed internal state of sum aggregate
-- and the effects of reduction
-- Int4RangeSet ranges;
-- int resizeTrigger;
-- int sizeLimit;
-- long reduceCalls;               //how many times reduceSize() fired
-- long maxIntervalCount;          //peak number of intervals seen
-- long totalIntervalCount;        //sum of counts across all agg
-- long combineCalls;              //number of times merged new input
CREATE TYPE sum_set_metrics AS (
    result int4range[],
    resizeTrigger bigint,
    sizeLimit bigint,
    reduceCalls bigint,
    maxIntervalCount bigint,
    totalIntervalCount bigint, 
    combineCalls bigint,
    minEffectiveIntervalCount bigint,
    convergedToTotSize bigint
);

CREATE FUNCTION agg_sum_set_transfunc_metrics(internal, int4range[], integer, integer, bool) 
RETURNS internal
AS 'MODULE_PATHNAME', 'agg_sum_set_transfunc_metrics'
LANGUAGE c;

CREATE FUNCTION agg_sum_set_finalfunc_metrics(internal) 
RETURNS sum_set_metrics
AS 'MODULE_PATHNAME', 'agg_sum_set_finalfunc_metrics'
LANGUAGE c;

create aggregate sum_metrics (int4range[], resizeTrigger integer, sizeLimit integer, bool) 
(
    stype = internal,
    sfunc = agg_sum_set_transfunc_metrics,
    finalfunc = agg_sum_set_finalfunc_metrics
);

---------- MIN/ MAX -----------
-- not sure if this is more functions than need be

CREATE FUNCTION combine_range_mult_min(range int4range, mult int4range) 
RETURNS int4range
AS 'MODULE_PATHNAME', 'combine_range_mult_min'
LANGUAGE c;

CREATE FUNCTION combine_range_mult_max(range int4range, mult int4range) 
RETURNS int4range
AS 'MODULE_PATHNAME', 'combine_range_mult_max'
LANGUAGE c;

CREATE FUNCTION agg_min_range_transfunc(state int4range, input int4range) 
RETURNS int4range
AS 'MODULE_PATHNAME', 'agg_min_range_transfunc'
LANGUAGE c;

CREATE FUNCTION agg_max_range_transfunc(state int4range, input int4range) 
RETURNS int4range
AS 'MODULE_PATHNAME', 'agg_max_range_transfunc'
LANGUAGE c;

create aggregate min (int4range)
(
    stype = int4range,
    sfunc = agg_min_range_transfunc
);
create aggregate max (int4range)
(
    stype = int4range,
    sfunc = agg_max_range_transfunc
);

CREATE FUNCTION combine_set_mult_min(set int4range[], mult int4range) 
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'combine_set_mult_min'
LANGUAGE c;

CREATE FUNCTION combine_set_mult_max(set int4range[], mult int4range) 
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'combine_set_mult_max'
LANGUAGE c;

CREATE FUNCTION agg_min_set_transfunc(state int4range[], input int4range[]) 
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'agg_min_set_transfunc'
LANGUAGE c;

CREATE FUNCTION agg_max_set_transfunc(state int4range[], input int4range[]) 
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'agg_max_set_transfunc'
LANGUAGE c;

CREATE FUNCTION agg_min_max_set_finalfunc(int4range[]) 
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'agg_min_max_set_finalfunc'
LANGUAGE c;

create aggregate min (int4range[])
(
    stype = int4range[],
    sfunc = agg_min_set_transfunc,
    finalfunc = agg_min_max_set_finalfunc
);
create aggregate max (int4range[])
(
    stype = int4range[],
    sfunc = agg_max_set_transfunc,
    finalfunc = agg_min_max_set_finalfunc
);

---------- Count -----------
CREATE FUNCTION agg_count_transfunc(state int4range, input int4range) 
RETURNS int4range
AS 'MODULE_PATHNAME', 'agg_count_transfunc'
LANGUAGE c;

create aggregate count (int4range)
(
    stype = int4range,
    sfunc = agg_count_transfunc
);


--------- AVERAGE ----------
CREATE FUNCTION agg_avg_range_transfunc(state internal, range int4range, mult int4range) 
RETURNS internal
AS 'MODULE_PATHNAME', 'agg_avg_range_transfunc'
LANGUAGE c;

CREATE FUNCTION agg_avg_range_finalfunc(internal) 
RETURNS int4range
AS 'MODULE_PATHNAME', 'agg_avg_range_finalfunc'
LANGUAGE c;

create aggregate avg (data int4range, mult int4range)
(
    stype = internal,   -- AvgAggState_range
    sfunc = agg_avg_range_transfunc,
    finalfunc = agg_avg_range_finalfunc
);

----------------------------------------------------------------------------
---------------------------Prune functions-----------------------------
----------------------------------------------------------------------------

CREATE FUNCTION prune_range_lt(a int4range, b int4range, direction boolean) 
RETURNS int4range
AS 'MODULE_PATHNAME', 'prune_range_lt'
LANGUAGE C STRICT;

CREATE FUNCTION prune_range_gt(a int4range, b int4range, direction boolean) 
RETURNS int4range
AS 'MODULE_PATHNAME', 'prune_range_gt'
LANGUAGE C STRICT;

CREATE FUNCTION prune_range_lte(a int4range, b int4range, direction boolean) 
RETURNS int4range
AS 'MODULE_PATHNAME', 'prune_range_lte'
LANGUAGE C STRICT;

CREATE FUNCTION prune_range_gte(a int4range, b int4range, direction boolean) 
RETURNS int4range
AS 'MODULE_PATHNAME', 'prune_range_gte'
LANGUAGE C STRICT;

CREATE FUNCTION prune_range_eq(a int4range, b int4range) 
RETURNS int4range
AS 'MODULE_PATHNAME', 'prune_range_eq'
LANGUAGE C STRICT;

CREATE FUNCTION prune_range_and(a int4range, b int4range) 
RETURNS int4range
AS 'MODULE_PATHNAME', 'prune_range_and'
LANGUAGE C STRICT;

CREATE FUNCTION prune_range_or(a int4range, b int4range) 
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'prune_range_or'
LANGUAGE C STRICT;

-- CREATE FUNCTION prune_range_not(a int4range) 
-- RETURNS int4range[]
-- AS 'MODULE_PATHNAME', 'prune_range_not'
-- LANGUAGE C STRICT;

CREATE FUNCTION prune_set_lt(a int4range[], b int4range[], direction boolean) 
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'prune_set_lt'
LANGUAGE C STRICT;

CREATE FUNCTION prune_set_lte(a int4range[], b int4range[], direction boolean) 
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'prune_set_lte'
LANGUAGE C STRICT;

CREATE FUNCTION prune_set_gt(a int4range[], b int4range[], direction boolean) 
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'prune_set_gt'
LANGUAGE C STRICT;

CREATE FUNCTION prune_set_gte(a int4range[], b int4range[], direction boolean) 
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'prune_set_gte'
LANGUAGE C STRICT;

CREATE FUNCTION prune_set_eq(a int4range[], b int4range[]) 
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'prune_set_eq'
LANGUAGE C STRICT;

CREATE FUNCTION prune_set_and(a int4range[], b int4range[]) 
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'prune_set_and'
LANGUAGE C STRICT;

CREATE FUNCTION prune_set_or(a int4range[], b int4range[]) 
RETURNS int4range[]
AS 'MODULE_PATHNAME', 'prune_set_or'
LANGUAGE C STRICT;
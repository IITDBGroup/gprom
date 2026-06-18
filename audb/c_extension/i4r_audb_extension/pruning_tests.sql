-- (1) when referring to prune_op(...), 
-- we mean calling prune_set_and(prune_op_left, prune_op_right)

-- need to handle 3 cases:
-- prune pre aggregation == WHERE
-- prune post aggregation == having
-- prune pre and post


------------------------------------------------------------
-- PRE AGGREGATE: SELECT sum(a) FROM r WHERE a < 5;
-- baseline
SELECT 
    sum(combine_set_mult_sum(val, mult), 500, 100) 
FROM r
WHERE val < cond;

-- pruned
SELECT 
    sum(combine_set_mult_sum(
        prune_set_lt(val, cond, direction),
         mult), 500, 100) 
FROM r;

-- or is it. union of left and right, right?
SELECT 
    sum(combine_set_mult_sum(
        prune_set_and(
            prune_set_lt(val, cond, false),
            prune_set_lt(val, cond, true),
        ),
        mult),500, 100)
FROM r;

------------------------------------------------------------
-- POST AGGREGATE: SELECT sum(a) FROM r HAVING sum(a) < 5;
-- baseline
SELECT 
    prune_set_lt(
        sum(combine_set_mult_sum(val, mult), 500, 100),
        cond, direction)
FROM r

SELECT 
    sum(combine_set_mult_sum(val, mult), 500, 100) res
FROM r
HAVING prune_set_lt(res, cond, direction)

------------------------------------------------------------
-- PRE AND POST AGRGEGATE: SELECT sum(a) FROM r WHERE a < 5 HAVING sum(a) > 2;
-- baseline
SELECT     
    sum(combine_set_mult_sum(val, mult), 500, 100) as res
FROM r
WHERE prune_set_lt(val, cond, direction)
HAVING prune_set_gt(res, cond direction)

-- rewrite
SELECT 
    prune_set_gt(
        sum(combine_set_mult_sum(
            prune_set_lt(val, cond_high, false),
            mult), 500, 100),
        cond_low, false)
FROM r;


-- prune using a ercentage of the total range





select 
    sum(combine_set_mult_sum(a,mult), 500, 100)
from tx_prune_basic
where a < b;

select 
    sum(combine_set_mult_sum(a,mult), 500, 100)
from tx_prune_basic
where set_lt(a,b) is not false;

select 
    sum(combine_set_mult_sum(
        prune_set_lt(a,b,false), 
        int4range(lower(mult) * case when set_lt(a,b) is NULL then 0 else 1 end, upper(mult)) as mult
    ), 500, 100)
from tx_prune_basic
where set_lt(a,b) is not false;


select a
FROm tx_prune_basic
where a < b;

select prune_set_lt(a,b,false)
from tx_prune_basic;

select sum(combine_set_mult_sum(a, mult), 500, 100)
FROm tx_prune_basic
where a < b


EXPLAIN (analyze, format json)
select 
    sum(combine_set_mult_sum(a, mult), 500, 100)
from (
    select 
        prune_set_lt(a,b,false) as a,
        mult
        -- int4range(lower(mult) * case when set_lt(a,b) is NULL then 0 else 1 end, upper(mult)) as mult
    from tx_prune_basic
) sub;


select 
    sum(combine_set_mult_sum(a, mult), 500, 100)
from (
    select 
        prune_set_lt(a,b,false) as a, 
        int4range(lower(mult) * case when set_lt(a,b) is NULL then 0 else 1 end, upper(mult)) as mult
    from tx_prune_basic
    where set_lt(a,b) is not false
) sub;


select 
    a, b, set_divide(a, array[lift_scalar(2)]),
    prune_set_lt(a, set_divide(a, array[lift_scalar(2)]), false)
from tx_prune_basic;



select 
    sum(combine_set_mult_sum(a, mult), 500, 100)
from (
    select 
        prune_set_lt(a,b,false) as a, 
        mult
        -- int4range(lower(mult) * case when set_lt(a,b) is NULL then 0 else 1 end, upper(mult)) as mult
    from tx_prune_basic
    where set_lt(a,b) is not false
) sub;




 SELECT 
                (result).result,
                (result).resizeTrigger,
                (result).sizeLimit,
                (result).reduceCalls,
                (result).maxIntervalCount,
                (result).totalIntervalCount,
                (result).combineCalls,
                (result).minEffectiveIntervalCount,
                (result).convergedToTotSize
            FROM (
                SELECT 
                    sum_metrics(combine_set_mult_sum(val, mult), 3, 1, False) as result
                FROM
                    t_s_iv_ni_nir_86390546e9
            )sub;
                FROM (
                    SELECT 
                        prune_set_lt(val, set_divide(val, array[lift_scalar(2)]), false) as val,
                        mult
                        -- prune_set_lt(val, val2, false) as val,
                    FROM t_s_iv_ni_nir_86390546e9
                ) sub1
            ) subq;


SELECT
    (result).result,
    (result).resizeTrigger,
    (result).sizeLimit,
    (result).reduceCalls,
    (result).maxIntervalCount,
    (result).totalIntervalCount,
    (result).combineCalls,
    (result).minEffectiveIntervalCount,
    (result).convergedToTotSize
FROM (
    SELECT
        sum_metrics(combine_set_mult_sum(val, mult), 500, 100, False) as result
    FROM (
        SELECT
            prune_set_lt(val, val2, false) as val,
            int4range(lower(mult) * case when set_lt(val,val2) is NULL then 0 else 1 end, upper(mult)) as mult
            -- mult
            -- prune_set_lt(val, set_divide(val2, array[lift_scalar(4)]), false) as val,
            -- prune_set_lt(val, val2, false) as val,
        FROM t_s_iv_ni_nir_3942458f30
    ) sub1
) subq;

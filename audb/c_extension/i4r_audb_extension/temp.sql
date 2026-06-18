SELECT
    set_reduce_size(
        sum(combine_set_mult_sum(val, mult), 500, 100), 100
    )
FROM t_s_iv_n_9ece6d82ac;


SELECT
    sum(combine_set_mult_sum(val, mult), 500, 100)
FROM t_s_iv_n_9ece6d82ac;


SELECT
    set_normalize(
        sum(combine_set_mult_sum(val, mult), 500, 100)
    )
FROM (
    select * from t_s_iv_n_9ece6d82ac limit 10
) sub;



------------------------------------

-- normalized
SELECT
    set_normalize(sum(combine_set_mult_sum(val, mult), 500, 100))
FROM (
    select * from t_s_iv_n_9ece6d82ac limit 100
) sub;


SELECT
    sum(combine_set_mult_sum(val, mult), 500, 100)
FROM (
    select * from t_s_iv_n_9ece6d82ac limit 100
) sub;

-- vs sumtest

SELECT
    sum_metrics(combine_set_mult_sum(val, mult), 500, 100, false)
FROM (
    select * from t_s_iv_n_9ece6d82ac limit 100
) sub;
------------------------------------



SELECT
    sumtest(combine_set_mult_sum(val, mult), 500, 100), 100
FROM t_s_iv_n_9ece6d82ac;

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
    (SELECT sumTest(combine_set_mult_sum(val, mult), 500, 100, true) as result
    FROM (SELECT * FROM t_s_iv_n_9ece6d82ac limit 10))) subq;




-- prune manual rewrite
-- select adn then agrgegation

-- tracking coverage and time vs non prune coverage and time

select sum(a) 
from r

select sum(a) 
from r
where a > 5

==

select prune_lt
select sum(a) 
from r
where a > 5

select sum(a) 
from r
where a > 5
having sum() > b

-- more carefully selected outcomes for reduce params
-- experiments with pruning for 



SELECT
    array_length(no_prune, 1) as intervals_no_prune,
    set_coverage(no_prune) as coverage_no_prune,
    array_length(pruned, 1) as intervals_pruned,
    set_coverage(pruned) as coverage_pruned
FROM (
    SELECT
        sum(combine_set_mult_sum(val, mult), 500, 100) as no_prune,
        sum(combine_set_mult_sum(
            prune_set_lt(val, array[int4range(1000, 1200)], false),
        mult), 500, 100) as pruned
    FROM t_s_iv_ni_nir_0073f377a2
) sub;

SELECT
    array_length(base, 1) as intervals_no_prune,
    set_coverage(base) as coverage_no_prune,
    array_length(bp, 1) as intervals_pruned,
    set_coverage(bp) as coverage_pruned
FROM (
    SELECT
        sum(combine_set_mult_sum(val, mult), 500, 100) as base,
        sum(combine_set_mult_sum(
            prune_set_lt(val, array[int4range(1000, 1200)], false),
        mult), 500, 100) as bp
    FROM t_s_iv_ni_nir_0073f377a2
) sub;


SELECT
    sum(combine_set_mult_sum(val, mult), 500, 100) 
from t_s_iv_ni_nir_0073f377a2 
where set_lt(val, array[int4range(1000, 1200)]);

SELECT 
    sum(combine_set_mult_sum(prune_set_lt(val, array[int4range(1000, 1200)], false), mult), 500, 100) 
FROM t_s_iv_ni_nir_0073f377a2;



SELECT 
    sum(combine_set_mult_sum(
        prune_set_and(
            prune_set_lt(val, array[int4range(1000, 1200)], false), 
            prune_set_lt(val, array[int4range(1000, 1200)], true)
        ),
    mult), 500, 100) 
FROM t_s_iv_ni_nir_0073f377a2;


select 
    prune_set_lt(val, array[int4range(1000, 1200)], false), 
    prune_set_lt(val, array[int4range(1000, 1200)], true),
    prune_set_and(
            prune_set_lt(val, array[int4range(1000, 1200)], false), 
            prune_set_lt(val, array[int4range(1000, 1200)], true)
        )
from tx_superwide;
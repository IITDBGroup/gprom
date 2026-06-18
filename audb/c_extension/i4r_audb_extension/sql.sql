
-- select range_add(prune_a, lift_scalar(3)), *
-- from (select prune_range_lt(a,b,false) as prune_a, * from tx_r_prune_basic) subq
-- where range_lt(a,b) is not false;


-- select *, 
--     prune_range_or( 
--         prune_range_lt(a,b,true),
--         prune_range_lt(a, lift_scalar(2), false)
--     ) 
-- from tx_r_prune_basic;


-- select
--      *, 
--     prune_range_gt(a, lift_scalar(17), false), 
--     prune_range_gt(a, lift_scalar(13), false) 
-- from tx_r_prune_basic;
-- -- where id = 1;


-- select 
--     *,     
--     prune_set_eq(a,b),
--     prune_set_and(a,b),
--     prune_set_or(a,b)
-- from tX_prune_basic;


-- select 
--     *, 
--     prune_set_lt(a,b, false),
--     prune_set_lt(a,b, true),
--     prune_set_lte(a,b, false),
--     prune_set_lte(a,b, true),    
--     prune_set_gt(a,b, false),
--     prune_set_gt(a,b, true),
--     prune_set_gte(a,b, false),
--     prune_set_gte(a,b, true)
-- from tX_prune_basic;
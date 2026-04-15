-- 与 GProM -uset_pruning 目标语义一致的手工 SQL（在 int 列 a,b 上）
-- 需在库中已执行 uset_pruning_pg_setup.sql（含 i4r、int_to_range_set、prune_*）
DROP TABLE IF EXISTS r_simple;
CREATE TABLE r_simple (a int, b int, u_r int);
INSERT INTO r_simple VALUES (3, 5, 1);

SELECT
  prune_and(
    prune_eq(int_to_range_set(a), int_to_range_set(3), false),
    prune_lt(int_to_range_set(a), int_to_range_set(b), false)
  ) AS a,
  prune_lt(int_to_range_set(a), int_to_range_set(b), true) AS b
FROM r_simple
WHERE set_eq(int_to_range_set(a), int_to_range_set(3))
  AND set_lt(int_to_range_set(a), int_to_range_set(b));

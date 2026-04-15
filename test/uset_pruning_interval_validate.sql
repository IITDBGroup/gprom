-- 区间列 r_interval：a、b 为 int4range[]（需先执行 uset_pruning_pg_setup.sql）
--
-- i4r：set_lt(A,B) 在区间重叠时为 NULL（unknown），不是 true。
-- 故 a=[3,4)、b=[3,5) 时 set_lt(a,b) 为 NULL，WHERE 整行为假（0 行）。
-- 行 u_r=2 使用 b=[4,6)，与 [3,4) 可判「严格在左」，set_lt 为 true。

-- 诊断：重叠行
SELECT u_r,
  set_eq(a, int_to_range_set(3)) AS eq3,
  set_lt(a, b) AS altb
FROM r_interval
WHERE u_r = 1;

-- 完整 prune（仅无重叠行）
SELECT
  prune_and(
    prune_eq(a, int_to_range_set(3), false),
    prune_lt(a, b, false)
  ) AS a_out,
  prune_lt(a, b, true) AS b_out
FROM r_interval
WHERE u_r = 2
  AND set_eq(a, int_to_range_set(3))
  AND set_lt(a, b);

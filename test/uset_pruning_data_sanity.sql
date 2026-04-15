-- 在已执行 uset_pruning_pg_setup.sql 后运行，用于核对样本行数
-- 当前种子数据下：应满足 a=3 AND a<b 的行数为 5（b ∈ {4,5,7,10,100}）
SELECT count(*) AS rows_matching_pred
FROM r
WHERE a = 3 AND a < b;

SELECT a, b, u_r
FROM r
WHERE a = 3 AND a < b
ORDER BY b;

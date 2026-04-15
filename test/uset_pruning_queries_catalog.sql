-- USET WITH PRUNING 测试语句速查（不可整文件执行；逐条复制到 gprom -sql '...'）
-- 详见 test/USET_PRUNING_TEST.md

-- S1 仅等值、单列
-- USET WITH PRUNING (SELECT a FROM r IS UADB WHERE a = 3);

-- S2 仅 set_lt
-- USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a < b);

-- S3 set_eq + set_lt
-- USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a = 3 AND a < b);

-- S4 双 set_eq
-- USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a = 3 AND b = 5);

-- S5 set_gt（需 prune_gt）
-- USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a > b);

-- S6 三合取
-- USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a = 3 AND a < b AND b > 4);

-- I1 区间列（见 uset_pruning_query_interval.sql）
-- USET WITH PRUNING (SELECT a, b FROM r_interval IS UADB WHERE a = 3 AND a < b);

-- 与「a=3 AND a<b」不同：双等值约束 a=3 AND b=5，仅命中单行 (3,5)，仍走 set_*/prune_*
USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a = 3 AND b = 5);

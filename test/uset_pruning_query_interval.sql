-- GProM：对区间列表 r_interval 使用 USET WITH PRUNING（需库中已建 r_interval）
USET WITH PRUNING (SELECT a, b FROM r_interval IS UADB WHERE a = 3 AND a < b);

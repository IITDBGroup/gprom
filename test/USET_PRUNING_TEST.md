# USET + AUDB 剪枝（Pruning）测试说明


## 1. 前置条件

1. PostgreSQL 已安装 **`i4r_audb_extension`**（含 **`prune.c`** 注册的 **`prune_set_*`**，`language c`）。可选执行本地 **`uset_pruning_pg_setup.sql`** 创建 **`int_to_range_set`**、表 **`r`** / **`r_interval`**。
2. GProM：`-backend postgres -frontend oracle`，且能连上带元数据的库（**`-Pmetadata postgres`** 等）。
3. 以下查询均使用 **`USET WITH PRUNING (...)`**，等价于开启 **`PROP_USET_PRUNING`**，**无需**再写 **`-uset_pruning`**。

---

## 2. 覆盖矩阵（标量表 `r`）

| 编号 | 说明 | 查询语句 | WHERE 重写（概念） | SELECT 剪枝（概念） |
|------|------|----------|---------------------|---------------------|
| S1 | 仅等值，**单列** | `USET WITH PRUNING (SELECT a FROM r IS UADB WHERE a = 3);` | `set_eq` | 列 **a** → **`prune_set_eq`**（**b 不出现**） |
| S2 | 仅 **`set_lt`** | `USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a < b);` | `set_lt` | **a**、**b** → **`prune_set_lt`**（`direction` 不同） |
| S3 | **`=` + `<` 合取** | `USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a = 3 AND a < b);` | `set_eq AND set_lt` | **`prune_set_and(prune_set_eq(...), prune_set_lt(...))`** 等对 **a** |
| S4 | **双等值** | `USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a = 3 AND b = 5);` | `set_eq AND set_eq` | 两列均为 **`prune_set_eq`**（**e2e 默认用例**） |
| S5 | **`>`** | `USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a > b);` | `set_gt` | **`prune_set_gt`** |
| S6 | **三个合取** | `USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a = 3 AND a < b AND b > 4);` | `set_eq AND set_lt AND set_gt` | 多约束 **`prune_set_and`** 链 |

**数据提示**：**`r`** 中有多行；**S4** 对 **(3,5)** 最典型；**S5** 需存在 **a>b** 的行（如 **(3,2)**）；**S6** 在 **`a=3` 且 `4<b` 且 `a<b`** 下有多行。



## 3. 区间列表 `r_interval`

| 编号 | 说明 | 来源文件 |
|------|------|----------|
| I1 | **`a`、`b` 为 `int4range[]`**，**`USET WITH PRUNING (SELECT a,b FROM r_interval IS UADB WHERE a = 3 AND a < b);`** | **`uset_pruning_query_interval.sql`** |



---








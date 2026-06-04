# GProM USET × i4r_audb_extension 集成测试文档

本文档记录 GProM 对 **i4r_audb_extension** 的重写能力（比较、算术、pruning、helper、聚合）的自动化测试范围、运行方式与最近一次执行结果。

**测试日期**：2026-06-02  
**GProM 二进制**：`gprom/src/command_line/gprom`  
**数据库**：`hana@localhost:5432/testdb`  
**扩展**：`i4r_audb_extension 1.1`

---

## 1. 环境准备

### 1.1 编译 GProM

```bash
cd /home/hana4/yangyun/gprom
make -j4
```

### 1.2 PostgreSQL 与扩展

```bash
export PGPASSWORD=001011   # 或 PGPASS=001011
psql -h localhost -U hana -d testdb \
  -f /home/hana4/yangyun/gprom/test/uset_pruning_pg_setup.sql
```

数据库需具备：

- **`CREATE EXTENSION i4r_audb_extension`**（**1.1+**，含 **`prune.c`** 注册的 **`prune_set_*` / `prune_range_*`**，`language c`）
- **`int_to_range_set`**、**`normalize_vals`** 等（扩展或本地 setup SQL）
- 测试表 **`r`**（整型不确定列）、**`r_interval`**（`int4range[]` 列）

可选脚本 **`uset_pruning_pg_setup.sql`**（本地路径，**未随 yangyun 远程分支提交**）会创建上述对象。**不再使用** PL/pgSQL 版 **`uset_pruning_i4r_aliases.sql`** 替代 C 剪枝。

### 1.3 一键运行全部测试

```bash
cd /home/hana4/yangyun/gprom/test
PGPASS=001011 sh run_uset_all_tests.sh
```

日志默认写入 `test/uset_all_tests.log`。

---

## 2. 测试套件总览

| 套件 | 脚本 | 用例数 | 说明 |
|------|------|--------|------|
| Pruning | `run_uset_pruning_all_cases.sh` | 7 | `USET WITH PRUNING` + `prune_set_*` |
| Helper | `run_uset_helpers_test.sh` | 2 | `-normalize` / `set_coverage` |
| Aggregate | `run_uset_agg_test.sh` | 4 | SUM / MIN·MAX / COUNT / AVG |
| 算术与比较 | `run_uset_all_tests.sh` 内联 | 3 | `set_add`、`set_lte`、`set_lt` |
| **合计** | `run_uset_all_tests.sh` | **16** | 全部通过 |

也可单独运行各子脚本：

```bash
sh run_uset_pruning_all_cases.sh
sh run_uset_helpers_test.sh
sh run_uset_agg_test.sh
```

---

## 3. 功能覆盖矩阵

### 3.1 三值比较（`set_*` / `range_*`）

| GProM 输入 | 期望生成 | 测试 |
|------------|----------|------|
| `WHERE a = 3` | `set_eq` + `prune_set_eq` | pruning 套件 |
| `WHERE a < b` | `set_lt` + `prune_set_lt` | pruning 套件 |
| `WHERE a <= 3` | `set_lte` | 算术冒烟 |
| `WHERE a > b` | `set_gt` + `prune_set_gt` | pruning 套件 |
| `r_interval` 上 `a < b` | `set_lt` | 区间 pruning + 算术冒烟 |

### 3.2 算术（`set_*` / `range_*`）

| GProM 输入 | 期望生成 | 测试 |
|------------|----------|------|
| `SELECT a + b`（`int4range[]`） | `set_add` | 算术冒烟 |
| `set_subtract/multiply/divide` | 透传 i4r | 代码路径（与 set_add 同族） |

### 3.3 投影剪枝（`prune_set_*` / `prune_range_*`）

| 场景 | 测试用例 |
|------|----------|
| 单列等值 | `SELECT a FROM r ... WHERE a = 3` |
| 合取 AND | `a = 3 AND a < b` → `prune_set_and` |
| 析取 OR | `a = 3 OR a < b`（同列）→ `prune_set_or` |
| 双等值 | `a = 3 AND b = 5` |
| 区间表 | `r_interval ... WHERE a = 3 AND a < b` |
| **聚合 SUM** | **不注入** `prune_set_*`（P/NP 生成 SQL 相同） |

### 3.4 Helper 函数

| 功能 | CLI / SQL | 期望 | 测试 |
|------|-----------|------|------|
| 归一化 | `-normalize` | `set_normalize`（非旧 `range_normalize`） | helpers |
| 覆盖率 | `set_coverage(a)` | 透传 + PG 可执行 | helpers |
| 兼容别名 | `normalize_vals` | 封装 `set_normalize` | setup SQL |

### 3.5 聚合（i4r combine + PG aggregate）

| 聚合 | GProM 输入 | 期望生成 SQL 片段 | 测试 |
|------|------------|-------------------|------|
| SUM | `SUM(a)` int 列 | `combine_set_mult_sum` + `sum(..., 50, 20)` | agg |
| MIN/MAX | `MIN(a), MAX(a)` 区间列 | `combine_set_mult_min/max` | agg |
| COUNT | `COUNT(*)` | `count(('[1,2)')::int4range)` | agg |
| AVG | `AVG(a)` int 列 | `avg(range_set_first(...), '[1,2)'::int4range)` | agg |

> **说明**：`agg_*_transfunc/finalfunc` 由 PostgreSQL `CREATE AGGREGATE` 隐式调用，GProM 不直接生成。原生 `int4range[]` 上的 AVG 暂未覆盖（i4r 尚未注册 `avg(int4range[])`）。

---

## 4. 测试用例明细

### 4.1 Pruning（7 项）

| ID | 名称 | 查询 |
|----|------|------|
| P1 | 仅等值选单列 | `USET WITH PRUNING (SELECT a FROM r IS UADB WHERE a = 3);` |
| P2 | 仅 set_lt | `USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a < b);` |
| P3 | set_eq + set_lt | `... WHERE a = 3 AND a < b` |
| P4 | 双 set_eq | `... WHERE a = 3 AND b = 5` |
| P5 | set_gt + prune_gt | `... WHERE a > b` |
| P6 | 三合取 AND | `USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a = 3 AND a < b AND b > 4);` |
| P7 | 区间列 | `USET WITH PRUNING (SELECT a, b FROM r_interval IS UADB WHERE a = 3 AND a < b );` |

### 4.2 Helper（2 项）

| ID | 名称 | 查询 / 选项 | 断言 |
|----|------|-------------|------|
| H1 | normalize | `-normalize` + `USET (SELECT a, b FROM r_interval IS UADB);` | SQL 含 `set_normalize` |
| H2 | set_coverage | `USET (SELECT set_coverage(a) FROM r_interval IS UADB);` | 透传且 psql 可执行 |

### 4.3 Aggregate（4 项）

| ID | 名称 | 查询 | 断言 |
|----|------|------|------|
| A1 | SUM | `USET (SELECT SUM(a) FROM r IS UADB);` | `combine_set_mult_sum` |
| A2 | MIN/MAX | `USET (SELECT MIN(a), MAX(a) FROM r_interval IS UADB);` | `combine_set_mult_min` |
| A3 | COUNT | `USET (SELECT COUNT(*) FROM r IS UADB);` | `count(('[1,2)')` |
| A4 | AVG | `USET (SELECT AVG(a) FROM r IS UADB);` | `range_set_first` |

### 4.4 算术与比较冒烟（3 项）

| ID | 名称 | 查询 | 断言 |
|----|------|------|------|
| E1 | set_add | `USET (SELECT a + b AS s FROM r_interval IS UADB);` | `set_add` |
| E2 | set_lte | `USET WITH PRUNING (SELECT a FROM r IS UADB WHERE a <= 3);` | `set_lte` |
| E3 | set_lt 区间 | `USET WITH PRUNING (SELECT a FROM r_interval IS UADB WHERE a < b);` | `set_lt` |

---

## 5. 生成 SQL 示例

以下为 `gprom ... -sql '...'` 实际输出片段（2026-06-02 抓取）。

### 5.1 Pruning + set_eq

```sql
SELECT prune_set_eq(F0_0."a", int_to_range_set(3)) AS "a"
FROM (...)
WHERE set_eq(F0_0."a", int_to_range_set(3));
```

（**`prune_set_eq` 为 2 参数**；勿再生成第三参数 `direction`。）

### 5.2 算术 set_add

```sql
SELECT set_add(F0_0."a", F0_0."b") AS "(a+b)"
FROM ...;
```

### 5.3 聚合 SUM

```sql
SELECT sum(combine_set_mult_sum(int_to_range_set(F0_0."a"), ('[1,2)')::int4range), 50, 20) AS "sum(a)"
FROM ...;
```

### 5.4 聚合 COUNT

```sql
SELECT count(('[1,2)')::int4range) AS "count(1)"
FROM ...;
```

### 5.5 聚合 AVG

```sql
SELECT avg(range_set_first(int_to_range_set(F0_0."a")), ('[1,2)')::int4range) AS "avg(a)"
FROM ...;
```

### 5.6 Helper set_coverage

```sql
SELECT set_coverage(F0_0."a") AS "set_coverage(a)"
FROM ...;
```

### 5.7 -normalize → set_normalize

```sql
SELECT set_normalize(F0_0."a") AS "a"
FROM ...;
```

---

## 6. 最近一次执行结果

```
======== USET / i4r 集成测试 ========
时间: 2026-06-02T12:33:34+08:00

PASS: Pruning（7 项）
PASS: Helper（normalize / set_coverage）
PASS: Aggregate（SUM/MIN/MAX/COUNT/AVG）
PASS: 算术与比较（set_add / set_lte / set_lt）

======== 全部通过 ========
```

完整日志见：`test/uset_all_tests.log`。

---

## 7. 相关文件索引

| 文件 | 用途 |
|------|------|
| `test/run_uset_all_tests.sh` | 总入口（16 项） |
| `test/run_uset_pruning_all_cases.sh` | Pruning 回归 |
| `test/run_uset_helpers_test.sh` | Helper 冒烟 |
| `test/run_uset_agg_test.sh` | 聚合冒烟 |
| `test/USET_PRUNING.md` | USET 剪枝功能总览 |
| `test/USET_I4R_PRUNE_REGISTRY.md` | 14 项剪枝对照表与架构 |
| `test/PRUNE_EFFECT_EXPERIMENT_PLAN.md` | 剪枝效果实验规划（可选） |
| `test/PRUNE_EFFECT_B-F1_RESULTS.md` | B-F1 实测与聚合路径说明 |
| `uset_i4r_prune_registry.c` / `.h` | 14 项注册表（已编入 `libur`） |
| `uncert_rewriter.c` | USET 重写、OR、`prune_set_*` 生成 |
| `expression.c` | `usetI4rIsPruneSqlFunc` |
| `uncert_rewriter.h` | `PRUNE_SET_*` / `AUDB_*` 宏 |
| `audb/.../prune.c` | i4r C 剪枝实现（与 GProM 配套） |

---

## 8. 已知限制

1. **聚合路径无投影剪枝**：`SELECT SUM(不确定 int 列)` 等不会在生成 SQL 中插入 **`prune_set_*`**，故 prune on/off 的 SUM 可能相同；见 **`PRUNE_EFFECT_B-F1_RESULTS.md`**。
2. **AVG on `int4range[]`**：原生区间列上的 AVG 跳过重写，待 i4r 注册 `avg(int4range[])` 后补充。
3. **`range_set_first`**：测试库可用 PL/pgSQL 占位（`a[1]`），生产环境宜由 i4r C 函数提供。
4. **`set_coverage` / `range_coverage`**：扩展未导出时可用 setup 占位，仅验证 GProM 透传。
5. **剪枝函数须为 C**：若 `\df prune_set_lt` 显示 `plpgsql`，需重装含 **`prune.c`** 的 **`i4r_audb_extension`**。

---

## 9. yangyun 分支已提交的 GProM 改动摘要

| 能力 | 说明 |
|------|------|
| 14 项 i4r 剪枝 | `uset_i4r_prune_registry` 映射；生成 `prune_set_*` / `prune_range_*` |
| `<=` / `>=` | WHERE：`set_lte` / `set_gte`；投影：`prune_set_lte` / `prune_set_gte` |
| OR 析取 | `flattenOrDisjuncts` → `prune_set_or` |
| `prune_set_eq` 二元 | 与 i4r C 签名一致，无 `direction` |
| `usetI4rIsPruneSqlFunc` | 模型检查识别剪枝函数 |

远程分支**不包含**部分本地测试脚本（如 `uset_pruning_i4r_aliases.sql`、`run_uset_pruning_i4r_full.sh` 等）。

---

*文档版本：2026-06，与 i4r C 剪枝 + `uset_i4r_prune_registry` 集成一致。*

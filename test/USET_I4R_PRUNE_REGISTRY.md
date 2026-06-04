# GProM × i4r_audb_extension 剪枝函数集成设计

## 1. 三层架构

```
┌─────────────────────────────────────────────────────────────┐
│  USET SQL（Oracle 前端）                                      │
│  WHERE a < b  /  a <= 3  /  a = 3 OR a < b                  │
│  USET WITH PRUNING (SELECT ...)  或  -uset_pruning            │
└───────────────────────────┬─────────────────────────────────┘
                            │ GProM 重写（uncert_rewriter.c）
┌───────────────────────────▼─────────────────────────────────┐
│  算子树：Selection(set_*) → Projection(prune_set_*)         │
│  注册表：uset_i4r_prune_registry.c（14 项对照）               │
│  OR 析取：flattenOrDisjuncts → prune_set_or                 │
└───────────────────────────┬─────────────────────────────────┘
                            │ 生成 SQL
┌───────────────────────────▼─────────────────────────────────┐
│  PostgreSQL：prune_set_* / prune_range_*（language c）      │
│  实现：audb/.../prune.c（i4r_audb_extension）                 │
└─────────────────────────────────────────────────────────────┘
```

**职责划分**：GProM **不实现**剪枝算法，只按注册表生成对 PG 函数的调用；**实际区间收缩**在数据库侧由 **i4r C 扩展**执行。

## 2. 函数对照表（14 项）

| i4r 内部 C 名 | GProM / PG SQL 名 | WHERE 比较 | 层 | PG 参数 |
|---------------|-------------------|------------|-----|---------|
| `prune_lt_internal_range` | `prune_range_lt` | `range_lt` | Range | 3（含 direction） |
| `prune_lte_internal_range` | `prune_range_lte` | `range_lte` | Range | 3 |
| `prune_gt_internal_range` | `prune_range_gt` | `range_gt` | Range | 3 |
| `prune_gte_internal_range` | `prune_range_gte` | `range_gte` | Range | 3 |
| `prune_eq_internal_range` | `prune_range_eq` | `range_eq` | Range | 2 |
| `prune_AND_internal_range` | `prune_range_and` | — | Range | 2 |
| `prune_OR_internal_range` | `prune_range_or` | — | Range | 2（返回 `int4range[]`） |
| `prune_lt_set_internal` | `prune_set_lt` | `set_lt` | Set | 3 |
| `prune_lte_set_internal` | `prune_set_lte` | `set_lte` | Set | 3 |
| `prune_gt_set_internal` | `prune_set_gt` | `set_gt` | Set | 3 |
| `prune_gte_set_internal` | `prune_set_gte` | `set_gte` | Set | 3 |
| `prune_eq_set_internal` | `prune_set_eq` | `set_eq` | Set | 2 |
| `prune_AND_internal_set` | `prune_set_and` | — | Set | 2 |
| `prune_OR_internal_set` | `prune_set_or` | — | Set | 2 |

**`usetPruneFuncUsesDirection`**（`uncert_rewriter.c`）：仅 **lt/lte/gt/gte**（及 range 层对应项）在 GProM 生成时带 **`direction`**；**eq / and / or** 为二元。

代码入口：

- `include/provenance_rewriter/uncertainty_rewrites/uset_i4r_prune_registry.h`
- `src/provenance_rewriter/uncertainty_rewrites/uset_i4r_prune_registry.c`

## 3. GProM 重写规则

| 阶段 | 输入 | 输出 |
|------|------|------|
| 比较 | `a < b`（`int4range[]`） | `set_lt(a,b)` |
| 比较 | `a < 3`（整型列） | `set_lt(int_to_range_set(a), int_to_range_set(3))` |
| 比较 | 单点 `int4range` 列 | `range_*` + `prune_range_*` |
| 投影剪枝 | `WITH PRUNING` + WHERE | 每列 `prune_set_*(col, other[, direction])` |
| 合取 AND | 多谓词同列 | `prune_set_and(p1, p2)` |
| 析取 OR | `p1 OR p2` 同列 | `prune_set_or(p1, p2)` |

**不剪枝的路径**：纯 **`SELECT SUM(不确定列)`** 等聚合（无投影列上的 `prune_set_*`）；B-F1 实验见 **`PRUNE_EFFECT_B-F1_RESULTS.md`**。

## 4. GProM 源码文件（yangyun 分支）

| 文件 | 作用 |
|------|------|
| `uset_i4r_prune_registry.h` / `.c` | 14 项注册表 + `usetI4rPruneSqlForCompare` 等 API |
| `uncert_rewriter.c` | WHERE/SELECT 重写、OR 析取、`usetPruneFuncUsesDirection` |
| `uncert_rewriter.h` | `PRUNE_SET_*` / `PRUNE_RANGE_*` / `AUDB_SET_*` 宏 |
| `expression.c` | `usetI4rIsPruneSqlFunc`、`typeOf` / `funcExists` |
| `analyze_oracle.c` | `USET WITH PRUNING` → `PROP_USET_PRUNING` |
| `uncertainty_rewrites/Makefile.am` | `uset_i4r_prune_registry.c` 编入 `libur` |

## 5. audb 扩展侧

| 文件 | 作用 |
|------|------|
| `audb/c_extension/i4r_audb_extension/prune.c` | 14 项剪枝 C 实现 |
| `audb/c_extension/i4r_audb_extension/prune.h` | 内部函数声明 |
| `audb/c_extension/i4r_audb_extension/i4r_audb_extension--1.1.sql` | `CREATE FUNCTION prune_set_* ... LANGUAGE c` |

安装扩展后，勿使用已废弃的 **`gprom/test/pruning/*.sql`** PL/pgSQL 包装。

## 6. 测试

### 6.1 GProM 生成 SQL 冒烟

```bash
cd gprom && make -j4
gprom -backend postgres -frontend oracle ... \
  -sql "USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a = 3 AND a < b);"
```

输出应含 **`set_eq` / `set_lt`** 与 **`prune_set_eq` / `prune_set_lt`**（非 `prune_eq`）。

### 6.2 PostgreSQL 直接测 14 项 C 函数（可选，本地）

若本机已安装含 `prune.c` 的扩展，可用 **`test/i4r_prune_all14_pg.sql`**（若存在）对 14 个函数逐项 `SELECT`。

### 6.3 剪枝效果实验（可选）

- 规划：**`PRUNE_EFFECT_EXPERIMENT_PLAN.md`**
- 示例结果：**`PRUNE_EFFECT_B-F1_RESULTS.md`**（`flights_rset` + `SUM`；聚合路径 P/NP SQL 相同的原因）

---

*文档版本：2026-06，与 yangyun 分支 i4r C 剪枝 + `uset_i4r_prune_registry` 一致。*

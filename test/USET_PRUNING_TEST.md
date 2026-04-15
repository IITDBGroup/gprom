# USET + AUDB 剪枝（Pruning）测试说明

本文列出**推荐测试用查询**、**覆盖点**，以及如何**批量 / 单条**运行。

---

## 1. 前置条件

1. PostgreSQL 已安装扩展 **`i4r_audb_extension`**，并执行 **`uset_pruning_pg_setup.sql`**（含 **`int_to_range_set`**、**`prune_*`**、表 **`r`** / **`r_interval`**）。
2. GProM：`-backend postgres -frontend oracle`，且能连上带元数据的库（**`-Pmetadata postgres`** 等）。
3. 以下查询均使用 **`USET WITH PRUNING (...)`**，等价于开启 **`PROP_USET_PRUNING`**，**无需**再写 **`-uset_pruning`**。

---

## 2. 覆盖矩阵（标量表 `r`）

| 编号 | 说明 | 查询语句 | WHERE 重写（概念） | SELECT 剪枝（概念） |
|------|------|----------|---------------------|---------------------|
| S1 | 仅等值，**单列** | `USET WITH PRUNING (SELECT a FROM r IS UADB WHERE a = 3);` | `set_eq` | 列 **a** → **`prune_eq`**（**b 不出现**） |
| S2 | 仅 **`set_lt`** | `USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a < b);` | `set_lt` | **a**、**b** → **`prune_lt`**（方向位不同） |
| S3 | **`=` + `<` 合取** | `USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a = 3 AND a < b);` | `set_eq AND set_lt` | **`prune_and(prune_eq(...), prune_lt(...))`** 等对 **a**；**b** 侧 **`prune_lt(..., TRUE)`** 等 |
| S4 | **双等值** | `USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a = 3 AND b = 5);` | `set_eq AND set_eq` | 两列均为 **`prune_eq`**（**`run_gprom_uset_pruning_e2e.sh` 默认用例**） |
| S5 | **`>`** | `USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a > b);` | `set_gt` | **`prune_gt`**（需重写器对 **`set_gt`** 合取拆分，见实现 **`buildPruneExprForColumn`**） |
| S6 | **三个合取** | `USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a = 3 AND a < b AND b > 4);` | `set_eq AND set_lt AND ...` 展开 | 多约束 **`prune_and`** 链更长 |

**数据提示**：**`r`** 中有多行；**S4** 对 **(3,5)** 最典型；**S5** 需存在 **a>b** 的行（如 **(3,2)**）；**S6** 在 **`a=3` 且 `4<b` 且 `a<b`** 下有多行。

---

## 2.1 每条测试语句对应的「生成过程」（GProM → 最终 SQL）

以下说明：**从输入 `USET WITH PRUNING (...)` 到 `-Pexecutor sql` 打印出的 PostgreSQL**，中间经过哪些**固定步骤**，以及 **S1–S6 / I1** 在各步**具体差别**。实现均在 **`src/provenance_rewriter/uncertainty_rewrites/uncert_rewriter.c`**（辅助函数见 **`uset_pruning_query*.sql`** 仅作人工对照，不参与编译）。

### A. 所有用例共用的流水线

| 步骤 | 做什么 | 说明 |
|------|--------|------|
| 1）解析 | **`oracle_parser.y`** | `USET WITH PRUNING ( stmt )` → **`ProvenanceStmt`**，**`options`** 含 **`PROP_USET_PRUNING`**。 |
| 2）分析 | **`analyze_*.c`** | 内层 **`SELECT ... FROM … IS UADB WHERE …`** 建成查询块；表 **`r` / `r_interval`** 列类型来自 **PostgreSQL 元数据**（**`-Pmetadata postgres`**）。 |
| 3）翻译 | **`translateProvenanceStmt`** | **`ProvenanceComputation`**；**`translateProperties`** 把 **`PROP_USET_PRUNING`** 拷到算子上。 |
| 4）重写入口 | **`rewriteUsetProvComp`** | **`g_uset_pruning_stmt_depth++`** ⇒ **`usetPruningActive()`** 为真；对子树根调用 **`rewriteUset(top)`**；结束前深度 **`--`**。 |
| 5）USET 树遍历 | **`rewriteUset`** → **`rewrite_UsetTableAccess` / `rewrite_UsetSelection` / `rewrite_UsetProjection`** | 典型顺序：先**自下而上**重写子树，再处理本层。**表访问**常被包一层投影，便于统一对列做 **`int_to_range_set`**。 |
| 6）WHERE | **`rewrite_UsetExpression`**（在 **`rewrite_UsetSelection`** 中） | 设置 **`g_uset_expr_input_op =`** 选择算子的**子**，以便 **`usetWrapArgForPredicate`** 判断列是否**已是** **`int4range[]`**（避免重复 lift）。**比较符**在 **`usetPruningActive()`** 下变为 **`set_eq` / `set_lt` / `set_gt`**；**`AND`** 递归子式，得到**布尔 SQL 与**（**不是** **`range_set_logic(..., int_to_range_set(布尔), …)`**）。 |
| 7）SELECT 列表 | **`rewrite_UsetExpression`**（在 **`rewrite_UsetProjection`** 中） | 先把标量列包成 **`int_to_range_set(列)`**（若 schema 已认定是区间类型则**透传**）。 |
| 8）剪枝替换投影 | **`applyUsetPruningToProjection`** | **`flattenAndConjuncts(where)`** 得 **`set_*` 合取列表**；对每个输出列名调用 **`buildPruneExprForColumn`**，用 **`prune_eq` / `prune_lt` / `prune_gt`** 及 **`prune_and`** 拼出**与该列相关的**收缩表达式，**替换**原 `int_to_range_set(列)`。 |
| 9）类型对齐 | **`syncAttrRefTypesFromInput`** | 消除 **`checkModel`** 父/子 **attrType** 不一致。 |
| 10）出 SQL | **SQL codegen（postgres）** | 生成带 **`WITH temp_view_0`**、子查询、`WHERE (… AND …)`、**`SELECT prune_* …`** 的语句。 |

---

### B. 各编号用例在「第 6～8 步」的差异（核心）

| 编号 | 用户 WHERE | 第 6 步之后 WHERE 的**典型结构** | 第 8 步 **flattenAndConjuncts** 得到的合取项 | 对各列 **SELECT** 的**典型**替换（概念） |
|------|------------|-----------------------------------|---------------------------------------------|--------------------------------------------|
| **S1** | `a = 3` | **`set_eq(int_to_range_set(a), int_to_range_set(3))`**（若 **a** 已是区间列则左侧为 **a**） | 仅一条 **`set_eq`** | 仅列 **a**：**`prune_eq(a侧, int_to_range_set(3), FALSE)`**；**无 b 列**故无 **`prune_*`(b)**。 |
| **S2** | `a < b` | **`set_lt(…, …)`** | 一条 **`set_lt`** | **a**（左操作数）→ **`prune_lt(..., FALSE)`**；**b**（右操作数）→ **`prune_lt(..., TRUE)`**（方向位与 **`prune_lt.sql`** 一致）。 |
| **S3** | `a = 3 AND a < b` | **`set_eq(...) AND set_lt(...)`** | **`set_eq` + `set_lt` 两条** | **列 a**：两条都含 **a** → **`prune_and( prune_eq(...), prune_lt(左,…,FALSE) )`**；**列 b**：仅 **`set_lt`** 含 **b** → **`prune_lt(..., TRUE)`**。 |
| **S4** | `a = 3 AND b = 5` | **`set_eq(a,·) AND set_eq(b,·)`**（常数经 **`int_to_range_set`**） | 两条 **`set_eq`**（分别涉及 **a** 与 **b**） | **a** 仅第一个合取项匹配 → **`prune_eq`**；**b** 仅第二个匹配 → **`prune_eq`**；一般**无** **`prune_and`**（若单列只命中一条约束）。 |
| **S5** | `a > b` | **`set_gt(…, …)`** | 一条 **`set_gt`** | **a** → **`prune_gt(..., FALSE)`**；**b** → **`prune_gt(..., TRUE)`**（**`buildPruneExprForColumn`** 中对 **`set_gt`** 分支）。 |
| **S6** | `a = 3 AND a < b AND b > 4` | **`set_eq AND set_lt AND`**（**`>`** → **`set_gt`**，左右操作数交换或按分析器产出的比较方向） | **三条 `set_*`** | **列 a**：来自 **`set_eq` + `set_lt`（及若形式为 `4 < b` 则可能另有 `set_gt`）** 的 **`prune_and` 链**；**列 b**：来自含 **b** 的合取项的 **`prune_*` + `prune_and`**（具体以生成 SQL 为准）。 |
| **I1** | 同 S3 但表 **`r_interval`** | 与 S3 相同逻辑 | 与 S3 相同 | **第 6 步**：子 schema 已为 **`int4range[]`** ⇒ **`usetWrapArgForPredicate`** 对 **`a`/`b` 常直接透传**，不包 **`int_to_range_set`**；内层投影可能不再对区间列二次 lift。**第 7～8 步**仍由 **`set_*` → `prune_*`** 拼装。 |

**注意**：上表「典型」描述与 **`loglevel 0`** 生成 SQL 一致；若元数据中列类型与预期不完全一致，**`int_to_range_set` 是否出现**以单次 **gprom** 输出为准。

---

## 3. 区间列表 `r_interval`

| 编号 | 说明 | 来源文件 |
|------|------|----------|
| I1 | **`a`、`b` 为 `int4range[]`**，**`USET WITH PRUNING (SELECT a,b FROM r_interval IS UADB WHERE a = 3 AND a < b);`** | **`uset_pruning_query_interval.sql`** |

**语义说明**：**`set_lt`** 在区间**重叠**时可能为 **NULL**（i4r 三值逻辑），详见 **`uset_pruning_interval_validate.sql`** 与 **`USET_PRUNING.md` §5.5**。

---

## 4. 一键批量测试

```bash
export PGPASS=001011   # 与本地库一致
sh /path/to/gprom/test/run_uset_pruning_all_cases.sh
```

将依次执行 **S1–S6** 与 **I1**（**GProM 生成 SQL → `psql` 执行**）。全部成功时最后一行打印 **`All cases passed.`**

**单次端到端（含结果片段断言）**：

```bash
sh /path/to/gprom/test/run_gprom_uset_pruning_e2e.sh
```

当前 **`uset_pruning_query.sql`** 内嵌查询对应 **S4**（**`a=3 AND b=5`**）。

---

## 5. 手工单条（调试 GProM 输出）

```bash
gprom -backend postgres -frontend oracle \
  -host localhost -port 5432 -user … -passwd … -db testdb \
  -Pmetadata postgres -Psqlcodegen postgres -Pexecutor sql -loglevel 0 \
  -sql 'USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a > b);'
```

将打印重写后的 **SQL**（含 **`set_gt` / `prune_gt`** 等），可复制到 **`psql`** 执行。

---

## 6. 未单独列出的能力边界（供预期对齐）

| 项目 | 说明 |
|------|------|
| **`OR` / `NOT`** | WHERE 可重写，**`flattenAndConjuncts`** 仅拆 **顶格 `AND`**；**OR** 整体**不作为**多个 **`set_*` 合取** 参与 **`applyUsetPruningToProjection` 的简单逐列匹配**。 |
| **子查询 / 聚合** | 当前回归以 **单表 + 选择 + 投影** 为主。 |
| **剪枝与 `set_*` 对齐** | **WHERE** 中 **`set_eq` / `set_lt` / `set_gt`** 会在 **SELECT** 上尝试匹配 **`prune_eq` / `prune_lt` / `prune_gt`**（见 **`uncert_rewriter.c`** **`buildPruneExprForColumn`**）。 |

---

## 7. 相关文件

| 文件 | 作用 |
|------|------|
| `test/uset_pruning_pg_setup.sql` | 库对象与 **`r` / `r_interval`** 数据 |
| `test/uset_pruning_query.sql` | e2e 默认 **S4** 语句 |
| `test/uset_pruning_query_interval.sql` | **I1** 语句 |
| `test/run_uset_pruning_all_cases.sh` | **S1–S6 + I1** 批量 |
| `test/run_gprom_uset_pruning_e2e.sh` | e2e + 结果子串检查 |
| `test/USET_PRUNING.md` | 功能与设计说明 |

---

*若增加新的谓词组合，请在本表追加一行，并在 **`run_uset_pruning_all_cases.sh`** 中加入对应 **`run_case`**。*

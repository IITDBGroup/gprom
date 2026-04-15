# GProM USET + AUDB Range-Set Pruning 说明

本文说明 **USET 不确定查询** 在启用 **Su/Oliver 式 range-set pruning** 时的设计、**代码改动位置**、**如何测试**，以及 **典型结果为何形如 `[3,4)` / `[5,6)`**。

---

## 1. 功能在做什么

### 1.1 背景

- **USET**：GProM 将整型列上的不确定性建模为 **区间集合**（在 PostgreSQL 侧对应 `int4range[]`，在 GProM 内部类型里常映射为 `DT_STRING`）。
- **Pruning（本实现）**：
  - **WHERE**：用 AUDB / i4r 的 **三值集合比较** `set_eq`、`set_lt`、`set_gt` 等，在**区间集合**上判断条件是否可能成立（返回布尔，供 SQL 过滤）。
  - **SELECT**：用 **`prune_*`** 按 WHERE 中的约束**收缩**各列对应的区间集合，使投影结果与“在 WHERE 成立前提下”的语义一致。

### 1.2 标量如何进集合

- 不再使用单独的 `lift_set(int)`；统一通过 **`int_to_range_set(x int)`**，在数据库里实现为 **`ARRAY[lift_scalar(x)]::int4range[]`**，与 AUDB 已有 **`lift_scalar`** 对齐。
- 列从标量转为区间集合时，同样使用 **`int_to_range_set(列)`**（或子查询输出已是 `int4range[]` 时**不再重复包裹**）。

### 1.3 两种启用方式（等价于“打开 pruning 模式”）

| 方式 | 含义 |
|------|------|
| 命令行 **`-uset_pruning`** | 全局选项 `OPTION_USET_PRUNING` 为真。 |
| 语法 **`USET WITH PRUNING ( ... )`** | 在 `ProvenanceStmt.options` 里写入 **`PROP_USET_PRUNING`**，翻译到 `ProvenanceComputation` 上；重写时通过深度计数 **`g_uset_pruning_stmt_depth`** 与 CLI 选项一起驱动 **`usetPruningActive()`**。 |

二者满足其一即可走 **set_*** / **prune_*** 路径。

---

## 2. 修改了哪些地方（GProM）

### 2.1 解析器（Oracle 前端）

- **`src/parser/oracle_parser.y`**  
  - 增加关键字 **`PRUNING`**。  
  - 增加产生式 **`USET WITH PRUNING '(' stmt ')'`**：与 `USET '(' stmt ')'` 相同输入类型，但 **`options`** 里放入 **`PROP_USET_PRUNING → true`**。
- **`src/parser/oracle_parser.l`**  
  - 词法识别 **`PRUNING`**。

修改解析器后需重新 **`make`**（会再生 **`oracle_parser.tab.c`**）。

### 2.2 元数据与属性

- **`include/model/query_operator/operator_property.h`**  
  - **`PROP_USET_PRUNING`**：标记“本条 USET 要求 pruning 重写”。

### 2.3 配置项

- **`include/configuration/option.h`** / **`src/configuration/option.c`**  
  - **`OPTION_USET_PRUNING`** / **`-uset_pruning`**：说明文字中描述 **`int_to_range_set`** 与 **`USET WITH PRUNING`**。

### 2.4 表达式类型（模型检查）

- **`src/model/expression/expression.c`**  
  - **`set_*` / `prune_*` / `int_to_range_set`** 的 **`typeOf` / `funcExists`**，使分析/检查阶段能识别这些函数（避免“函数不存在”类问题）。

### 2.5 核心：`uncert_rewriter.c`

文件：**`src/provenance_rewriter/uncertainty_rewrites/uncert_rewriter.c`**

主要逻辑分组如下：

| 机制 | 作用 |
|------|------|
| **`usetPruningActive()`** | `getBoolOption(OPTION_USET_PRUNING) \|\| g_uset_pruning_stmt_depth > 0`。 |
| **`rewriteUsetProvComp`** | 若算子带 **`PROP_USET_PRUNING`**，则 **`g_uset_pruning_stmt_depth++`**，退出前 **`--`**。 |
| **`usetWrapArgForPredicate`** | 为 **`set_*`** 准备参数；若输入列在**子算子 schema** 中已是 **`int4range[]`（`DT_STRING`）**，则**不再**套 **`int_to_range_set`**。 |
| **`rewriteUsetExpression`** | 比较 **`=` / `<` / `>`** 在 pruning 下改为 **`set_eq` / `set_lt` / `set_gt`**；**`AND`/`OR`/`NOT`** 在 **`usetPruningActive()`** 下改为**递归重写子表达式**并保留 **SQL 布尔运算**，避免误用 **`range_set_logic(int_to_range_set(布尔子式), ...)`**。 |
| **`g_uset_expr_input_op`** | 在 **`rewrite_UsetProjection` / `rewrite_UsetSelection`** 里指向**直接子算子**，供 **`AttributeReference`** 与 **`usetWrapArgForPredicate`** 判断“是否已是区间列”。 |
| **`syncAttrRefTypesFromInput`** | 重写后把表达式里 **`AttributeReference.attrType`** 与**子算子** **`AttributeDef.dataType`** 对齐，避免 **`checkModel`** 报 **DT_INT** vs **DT_STRING** 不一致。 |
| **`applyUsetPruningToProjection` / `buildPruneExprForColumn`** | 从 WHERE 中已重写的 **`set_*`** 合取式拆出与各输出列相关的 **`prune_eq` / `prune_lt`** 等，并可 **`prune_and`** 合并。 |

头文件 **`include/provenance_rewriter/uncertainty_rewrites/uncert_rewriter.h`** 中宏名 **`AUDB_SET_*` / `PRUNE_*`** 与实现一致（**已去掉对 `lift_set` 函数名的依赖**）。

---

## 3. 数据库与 AUDB 侧

### 3.1 测试安装脚本

- **`test/uset_pruning_pg_setup.sql`**  
  - 扩展 **`i4r_audb_extension`**。  
  - **`int_to_range_set`**（内部 **`lift_scalar`**）。  
  - **`normalize_vals`**（封装 **`set_normalize`**）。  
  - **`prune_and`**（两段区间集合逐段相交再合并）。  
  - `\i` 引用 **`audb/.../prune_equal.sql`**、**`prune_lt.sql`**、**`prune_gt.sql`**。  
  - 表 **`r(a, b, u_r)`** 及多样本 **`INSERT`**（用于多行过滤测试）。

### 3.2 路径说明

- `\i /home/hana4/audb/...` 为**本机绝对路径**。若你换机器，需改 **`uset_pruning_pg_setup.sql`** 中的 `\i` 路径，或把这三份 `prune_*.sql` 拷到可访问目录。

---

## 4. 如何测试

**每条测试 `USET WITH PRUNING` 语句在 GProM 内如何一步步生成最终 SQL**（公共流水线 + S1–S6 / I1 差异）见 **`USET_PRUNING_TEST.md` §2.1**。

### 4.1 准备数据库

```bash
export PGPASSWORD=你的密码   # 或与脚本一致使用 PGPASS
psql -h localhost -p 5432 -U hana -d testdb -v ON_ERROR_STOP=1 \
  -f /path/to/gprom/test/uset_pruning_pg_setup.sql
```

### 4.2 手工等价 SQL（验证语义）

```bash
psql ... -f /path/to/gprom/test/uset_pruning_handcrafted_validate.sql
```

表 **`r_simple`** 为单行 **(3,5)**，在 **`WHERE set_eq(...) AND set_lt(...)`** 下应能查出**一行**区间结果。

### 4.3 样本行数核对

```bash
psql ... -f /path/to/gprom/test/uset_pruning_data_sanity.sql
```

当前种子数据下，**`a = 3 AND a < b`** 应对应 **5 行**（见脚本内注释）。

### 4.4 GProM 端到端（生成 SQL + 在 PG 执行）

```bash
export PGPASS=001011   # 示例
sh /path/to/gprom/test/run_gprom_uset_pruning_e2e.sh
```

脚本会：加载 **`uset_pruning_pg_setup.sql`** → 读取 **`uset_pruning_query.sql`** 中的 **`USET WITH PRUNING (...)`** → 调用 **gprom** 输出 SQL → **psql** 执行并检查输出中是否出现预期区间片段。

### 4.5 命令行直接看重写 SQL

```bash
gprom -backend postgres -frontend oracle \
  -host localhost -port 5432 -user hana -passwd ... -db testdb \
  -Pmetadata postgres -Psqlcodegen postgres -Pexecutor sql -loglevel 0 \
  -sql "USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a = 3 AND a < b);"
```

表需 **`IS UADB`**，且元数据要求与 UADB 规则一致（例如最后一列为 **`u_r`** 等，以你当前 **analyzer** 为准）。

---

## 5. 为什么结果是 `[3,4)`、`[5,6)` 这类形式

### 5.1 整型点与半开区间

- AUDB / i4r 里 **`lift_scalar(k)`** 把整数 **k** 提升为**单点区间**，习惯写成**左闭右开** **`[k, k+1)`**（在 PostgreSQL **`int4range`** 里常见）。
- 故 **3** 对应 **`[3,4)`**，**5** 对应 **`[5,6)`**。

### 5.2 WHERE 语义（过滤）

- **`set_eq(int_to_range_set(a), int_to_range_set(3))`**：在区间集合语义下要求 **a 与标量 3 的 lift 可相等**；对点值 **a=3** 成立。
- **`set_lt(..., a, b)`**（实现细节以 **`set_lt.sql`** 为准）：表达 **a 对应集合在 b 对应集合左侧** 的三值关系；对 **a=3, b=5** 应成立。

### 5.3 SELECT 语义（prune）

- **`prune_eq` / `prune_lt`**：在 **WHERE 已给出的约束**下，对**输出列**上的区间做**收缩**（与 Su/Oliver 论文中 range-set pruning 一致）。
- 对单行 **(a,b)=(3,5)**，收缩后列 **a** 仍落在 **`[3,4)`**，列 **b** 落在 **`[5,6)`**（与手工脚本一致）。
- **`prune_and`**：对同一列上来自多个约束的 prune 结果做**区间集合上的合取**（实现为逐段求交再合并，见 **`uset_pruning_pg_setup.sql`** 中的 **`prune_and`**）。

### 5.4 多行表 `r`

- 种子数据中有多行满足 **`a=3 AND a<b`** 时，**不带 `LIMIT` 的查询会返回多行**；每行的 **`[3,4)`** 等仍来自上述 lift/prune 规则，但 **b 列**会因 **b 不同** 而得到**不同的**右半区间（例如 **`[10,11)`** 等）。

### 5.5 区间列表 `r_interval`（`int4range[]`）

- 表 **`r_interval(a, b, u_r)`** 中 **`a`、`b` 直接存 `int4range[]`**（不是标量 `int`），用于测「列本身已是区间集合」的路径。
- **`u_r=1`**：**`a = [3,4)`**，**`b = [3,5)`**（你要求的 **b 初始为 `[3,5)`**）。二者在整数 **3** 处**重叠**，i4r 的 **`set_lt(a,b)`** 为 **NULL**（三值逻辑中的 *unknown*），**不是 true**。因此 **`WHERE ... AND set_lt(a,b)`** 对该行**不成立**（结果为 **0 行**），这是**预期语义**，不是实现错误。
- **`u_r=2`**：**`a = [3,4)`**，**`b = [4,6)`**：与 **`[3,4)`** 无重叠且 **`b` 整体在 `a` 右侧**，**`set_lt(a,b)`** 为 **true**，可用来跑通 **prune** 与 **GProM** 生成 SQL（**`b` 投影收缩后为 `[4,6)`**，而非标量 **5** 时的 **`[5,6)`**）。
- 手工验证：**`test/uset_pruning_interval_validate.sql`**。GProM 样例：**`test/uset_pruning_query_interval.sql`**（`FROM r_interval IS UADB ...`）。

---

## 6. 常见问题

| 现象 | 可能原因 |
|------|----------|
| 解析失败 **`PRUNING`** | 未重新 `make`，或前端不是 Oracle 解析器。 |
| 元数据/连接错误 | **`-Pmetadata postgres`**、**`-host/-db/-user/-passwd`** 未配对。 |
| **`IS UADB` 分析失败** | 表未按 UADB 规则标注或列不符合 analyzer 假设。 |
| **`\i` 找不到 prune 文件** | 修改 **`uset_pruning_pg_setup.sql`** 里绝对路径。 |
| **脚本 `set: Illegal option`** | Shell 脚本为 **CRLF** 换行，在 Linux 上执行 **`sed -i 's/\r$//' *.sh`**。 |

---

## 7. 相关文件一览

| 路径 | 用途 |
|------|------|
| `test/uset_pruning_pg_setup.sql` | DB 扩展、函数、表 **`r`** / **`r_interval`** 种子数据 |
| `test/uset_pruning_query.sql` | 供 e2e 读取的 **`USET WITH PRUNING`** 样例（表 **`r`**，当前为 **`WHERE a=3 AND b=5`**，双 **`set_eq` + 双 `prune_eq`） |
| `test/uset_pruning_query_interval.sql` | **`USET WITH PRUNING`** 样例（表 **`r_interval`**） |
| `test/uset_pruning_interval_validate.sql` | 区间列 **`set_lt` 重叠说明** + **`u_r=2`** 的 prune 对照 |
| `test/uset_pruning_handcrafted_validate.sql` | 单行手工对照 |
| `test/uset_pruning_data_sanity.sql` | 行数与满足条件的行列表 |
| `test/run_gprom_uset_pruning_e2e.sh` | 自动化端到端 |
| `test/run_uset_pruning_all_cases.sh` | 多查询批量（S1–S6 + 区间 I1） |
| `test/USET_PRUNING_TEST.md` | **测试用例表**与批量运行说明 |
| `test/run_uset_pruning_test.sh` | 仅跑手工 validate（psql） |
| `audb/.../pruning/prune_*.sql` | **`prune_eq` / `prune_lt` / `prune_gt`** 定义 |

---

*文档随实现演进；若你改动 **`prune_*` 或 `set_*` 签名**，请同步更新本节与测试脚本中的预期。*

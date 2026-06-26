# GProM USET + audb Docker 测试环境

PostgreSQL 16 + `i4r_audb_extension` + GProM（`yangyun` 分支）的一键 Docker 封装，用于复现 **USET / USET WITH PRUNING** 实验（flights / hospital / sales 三库）。

---

## 目录

- [功能概览](#功能概览)
- [快速开始（完整测试流程）](#快速开始完整测试流程)
- [分步操作](#分步操作)
- [交互式 Shell（改写 SQL + 查询结果）](#交互式-shell改写-sql--查询结果)

---

## 功能概览


| 能力              | 说明                                                  |
| --------------- | --------------------------------------------------- |
| **postgres 服务** | PG16 + 编译安装 audb 扩展 + 初始化三库测试数据                     |
| **gprom-test**  | 自动化 smoke / 三库聚合实验                                  |
| **gprom-shell** | 交互式输入 USET，输出改写 SQL **并** 在 PostgreSQL 执行显示结果       |
| **辅助脚本**        | `pull-base-images.sh`（国内拉镜像）、`run-all.sh`（一键 smoke） |




---



## 快速开始（完整测试流程）

```bash
cd gprom/docker/uset-test


docker compose up -d postgres                              # 1. 启动 DB
docker compose --profile test run --rm gprom-test            # 2. smoke 测试
docker compose --profile shell run --rm gprom-shell          # 3. 交互 shell
```



---



## 分步操作

### 1. 构建并启动 PostgreSQL

```bash
cd gprom/docker/uset-test
docker compose build --progress=plain postgres   # 首次或 Dockerfile 变更后
docker compose up -d postgres
docker compose ps                                 # STATUS 应为 healthy
```

首次启动会自动：

- 编译安装 `i4r_audb_extension`（`test/build_i4r_audb_extension.sh`）
- 创建用户 `**hana**` / 密码 `**001011**`
- 加载 `flights_rset`、`hospital_outlier_rset`、`sales_rset`
- 创建 `int_count_set`、`ic_sum_result` 等实验函数（`test/prune_effect_metrics.sql`）

### 2. Smoke 自动化测试

```bash
docker compose --profile test build gprom-test      # 首次需编译 GProM
docker compose --profile test run --rm gprom-test
```

执行顺序：

1. `verify_audb_pg16.sh`
2. GProM USET：`SELECT SUM(act_dep) ...`
3. GProM USET WITH PRUNING：同上（带剪枝）

## 交互式 Shell（改写 SQL + 查询结果）

```bash
docker compose --profile shell build gprom-shell    
docker compose --profile shell run --rm gprom-shell
```

每条查询输出两段：

```text
-- Rewritten SQL --
WITH temp_view_0 AS ( ... )
SELECT sum(combine_set_mult_sum(prune_set_lt(...), ...)) ...

-- Query result --
 sum(act_dep)
----------------
 {"[64361,78397)"}
(1 row)
```

示例输入：

```sql
USET WITH PRUNING (SELECT SUM(act_dep) FROM flights_rset IS UADB WHERE sched_dep < act_dep);
```

命令：`\q` 退出，`\h` 查看示例。




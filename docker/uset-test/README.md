# GProM USET 测试 Docker 环境

PostgreSQL 16 + `i4r_audb_extension` + GProM（yangyun 分支），一键跑 USET / WITH PRUNING 测试。

## 前置

- Docker + Docker Compose v2
- 在 **gprom 仓库根目录** 执行（需含 `audb/`、`sales_rset.csv` 等）

## 快速开始

```bash
cd gprom/docker/uset-test

# 1. 构建并启动 PostgreSQL（含 audb 扩展 + 测试数据）
docker compose up -d postgres

# 2. 跑自动化 smoke 测试（GProM + verify 脚本）
docker compose --profile test run --rm gprom-test

# 3. 进入 GProM 交互 shell
docker compose --profile shell run --rm gprom-shell
```

## 跑完整三库实验

```bash
docker compose --profile test run --rm -e RUN_FULL=1 gprom-test
```

## 环境变量

| 变量 | 默认 | 说明 |
|------|------|------|
| `PGPORT` | 5432 | 宿主机映射端口 |
| `PGUSER` | hana | 数据库用户 |
| `PGPASS` | 001011 | 密码 |
| `PGDB` | testdb | 数据库名 |
| `RUN_FULL` | 0 | 设为 1 跑 `run_three_rset_agg_experiments.sh` |

## 架构

```
docker-compose
├── postgres   PG16 + audb 扩展 + flights/hospital/sales 数据
└── gprom-test 编译好的 gprom + test 脚本
```

- **postgres**：镜像构建时编译 audb（`test/build_i4r_audb_extension.sh`），初始化时加载 SQL/CSV。
- **gprom-test**：多阶段构建 GProM，连接 `postgres` 服务跑测试。

## 注意

- 请用 **`hana` 用户**（密码 `001011`），不要用 `postgres` peer 连接。
- 首次 `docker compose up postgres` 会初始化数据库；改数据需 `docker compose down -v` 后重建。
- 宿主机 5432 已被占用时：`PGPORT=5433 docker compose up -d postgres`

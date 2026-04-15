#!/usr/bin/env sh
# 端到端：连接 PostgreSQL -> 安装 USET pruning 对象 -> GProM 生成 SQL -> psql 执行并校验结果。
# 用法：PGHOST/PGPORT/PGUSER/PGDATABASE/PGPASS 可覆盖；默认与 run_uset_pruning_test.sh 一致。
set -eu

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PGHOST="${PGHOST:-localhost}"
PGPORT="${PGPORT:-5432}"
PGUSER="${PGUSER:-hana}"
PGDATABASE="${PGDATABASE:-testdb}"
export PGPASSWORD="${PGPASS:-001011}"

GPROM="${GPROM:-$SCRIPT_DIR/../src/command_line/gprom}"
if ! test -x "$GPROM"; then
	GPROM="$(command -v gprom || true)"
fi
if test -z "$GPROM" || ! test -x "$GPROM"; then
	echo "run_gprom_uset_pruning_e2e.sh: 找不到可执行的 gprom，请设置 GPROM=.../gprom" >&2
	exit 1
fi

psql -h "$PGHOST" -p "$PGPORT" -U "$PGUSER" -d "$PGDATABASE" -v ON_ERROR_STOP=1 \
	-f "$SCRIPT_DIR/uset_pruning_pg_setup.sql" >/dev/null 2>&1

SQL_TEXT=$(grep -E '^[[:space:]]*USET' "$SCRIPT_DIR/uset_pruning_query.sql" | tr -d '\r' | head -n1)
if test -z "$SQL_TEXT"; then
	echo "run_gprom_uset_pruning_e2e.sh: uset_pruning_query.sql 为空" >&2
	exit 1
fi

TMP="$(mktemp)"
trap 'rm -f "$TMP"' EXIT INT HUP

"$GPROM" -backend postgres -frontend oracle \
	-host "$PGHOST" -port "$PGPORT" -user "$PGUSER" -passwd "$PGPASSWORD" -db "$PGDATABASE" \
	-Pmetadata postgres -Psqlcodegen postgres -Pexecutor sql -loglevel 0 \
	-sql "$SQL_TEXT" 2>"$TMP" | sed 's/\x1b\[[0-9;]*m//g' >"$TMP.sql"

if test -s "$TMP"; then
	echo "run_gprom_uset_pruning_e2e.sh: gprom stderr:" >&2
	cat "$TMP" >&2
	exit 1
fi

OUT="$(psql -h "$PGHOST" -p "$PGPORT" -U "$PGUSER" -d "$PGDATABASE" -v ON_ERROR_STOP=1 -At -f "$TMP.sql")" \
	|| exit 1

echo "$OUT" | grep -q '\[3,4)' || {
	echo "run_gprom_uset_pruning_e2e.sh: 未在结果中见到 [3,4)，实际输出:" >&2
	echo "$OUT" >&2
	exit 1
}
echo "$OUT" | grep -q '\[5,6)' || {
	echo "run_gprom_uset_pruning_e2e.sh: 未在结果中见到 [5,6)，实际输出:" >&2
	echo "$OUT" >&2
	exit 1
}

echo "OK: GProM USET WITH PRUNING e2e on ${PGUSER}@${PGHOST}:${PGPORT}/${PGDATABASE}"

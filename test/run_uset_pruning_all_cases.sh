#!/usr/bin/env sh
# 批量跑 USET WITH PRUNING：GProM 生成 SQL + psql 执行（需 testdb 与 uset_pruning_pg_setup.sql）
# 用法：PGPASS=... sh run_uset_pruning_all_cases.sh
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
	echo "run_uset_pruning_all_cases.sh: 设置 GPROM=.../gprom" >&2
	exit 1
fi

psql -h "$PGHOST" -p "$PGPORT" -U "$PGUSER" -d "$PGDATABASE" -v ON_ERROR_STOP=1 \
	-f "$SCRIPT_DIR/uset_pruning_pg_setup.sql" >/dev/null 2>&1

run_case() {
	_name="$1"
	_sql="$2"
	_tmp="$(mktemp)"
	_err="$(mktemp)"
	printf '%s\n' "---- $_name ----"
	"$GPROM" -backend postgres -frontend oracle \
		-host "$PGHOST" -port "$PGPORT" -user "$PGUSER" -passwd "$PGPASSWORD" -db "$PGDATABASE" \
		-Pmetadata postgres -Psqlcodegen postgres -Pexecutor sql -loglevel 0 \
		-sql "$_sql" 2>"$_err" | sed 's/\x1b\[[0-9;]*m//g' >"$_tmp"
	if test -s "$_err"; then
		echo "gprom stderr:" >&2
		cat "$_err" >&2
		exit 1
	fi
	psql -h "$PGHOST" -p "$PGPORT" -U "$PGUSER" -d "$PGDATABASE" -v ON_ERROR_STOP=1 -f "$_tmp" >/dev/null \
		|| { echo "psql failed for: $_name" >&2; cat "$_tmp" >&2; rm -f "$_tmp" "$_err"; exit 1; }
	rm -f "$_tmp" "$_err"
	echo "OK"
}

run_case 'scalar: 仅等值选单列' \
	'USET WITH PRUNING (SELECT a FROM r IS UADB WHERE a = 3);'

run_case 'scalar: 仅 set_lt' \
	'USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a < b);'

run_case 'scalar: set_eq + set_lt（AND）' \
	'USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a = 3 AND a < b);'

run_case 'scalar: 双 set_eq' \
	'USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a = 3 AND b = 5);'

run_case 'scalar: set_gt + prune_gt' \
	'USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a > b);'

run_case 'scalar: 三合取 AND' \
	'USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a = 3 AND a < b AND b > 4);'

SQL_INT="$(grep -E '^[[:space:]]*USET' "$SCRIPT_DIR/uset_pruning_query_interval.sql" | tr -d '\r' | head -n1)"
run_case 'interval: r_interval（区间列）' "$SQL_INT"

echo "All cases passed."

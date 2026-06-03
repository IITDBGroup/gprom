#!/usr/bin/env sh
# i4r 剪枝 14 项 GProM 集成回归（set/range × lt/lte/gt/gte/eq + and/or）
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
	echo "run_uset_pruning_i4r_full.sh: 设置 GPROM=.../gprom" >&2
	exit 1
fi

psql -h "$PGHOST" -p "$PGPORT" -U "$PGUSER" -d "$PGDATABASE" -v ON_ERROR_STOP=1 \
	-f "$SCRIPT_DIR/uset_pruning_pg_setup.sql" >/dev/null 2>&1

run_case() {
	_name="$1"
	_sql="$2"
	_needle="$3"
	_tmp="$(mktemp)"
	_err="$(mktemp)"
	printf '%s\n' "---- $_name ----"
	_out="$("$GPROM" -backend postgres -frontend oracle \
		-host "$PGHOST" -port "$PGPORT" -user "$PGUSER" -passwd "$PGPASSWORD" -db "$PGDATABASE" \
		-Pmetadata postgres -Psqlcodegen postgres -Pexecutor sql -loglevel 0 \
		-sql "$_sql" 2>"$_err" | sed 's/\x1b\[[0-9;]*m//g')"
	if test -s "$_err"; then
		echo "gprom stderr:" >&2
		cat "$_err" >&2
		rm -f "$_tmp" "$_err"
		exit 1
	fi
	case "$_out" in
	*"$_needle"*) ;;
	*)
		echo "FAIL $_name: 期望含 '$_needle'" >&2
		echo "$_out" >&2
		rm -f "$_tmp" "$_err"
		exit 1
		;;
	esac
	printf '%s\n' "$_out" >"$_tmp"
	psql -h "$PGHOST" -p "$PGPORT" -U "$PGUSER" -d "$PGDATABASE" -v ON_ERROR_STOP=1 \
		-f "$_tmp" >/dev/null \
		|| { echo "psql failed: $_name" >&2; cat "$_tmp" >&2; rm -f "$_tmp" "$_err"; exit 1; }
	rm -f "$_tmp" "$_err"
	echo "OK"
}

# Set 层比较 + 剪枝
run_case 'set_lt' \
	'USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a < b);' \
	'prune_set_lt'

run_case 'set_lte' \
	'USET WITH PRUNING (SELECT a FROM r IS UADB WHERE a <= 3);' \
	'prune_set_lte'

run_case 'set_gt' \
	'USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a > b);' \
	'prune_set_gt'

run_case 'set_gte' \
	'USET WITH PRUNING (SELECT b FROM r IS UADB WHERE b >= 4);' \
	'prune_set_gte'

run_case 'set_eq' \
	'USET WITH PRUNING (SELECT a FROM r IS UADB WHERE a = 3);' \
	'prune_set_eq'

run_case 'set_and' \
	'USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a = 3 AND a < b);' \
	'prune_set_and'

run_case 'set_or' \
	'USET WITH PRUNING (SELECT a FROM r IS UADB WHERE a = 3 OR a < b);' \
	'prune_set_or'

# 区间列 r_interval
run_case 'interval_set_lt' \
	'USET WITH PRUNING (SELECT a, b FROM r_interval IS UADB WHERE a < b);' \
	'prune_set_lt'

echo "All i4r prune registry cases passed."

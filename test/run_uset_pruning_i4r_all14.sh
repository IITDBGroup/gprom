#!/usr/bin/env sh
# 14 项 i4r 剪枝：GProM 生成 SQL（Set 7 + 可触发的 Range）+ PG 直接 C 函数冒烟
set -eu

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PGHOST="${PGHOST:-localhost}"
PGPORT="${PGPORT:-5432}"
PGUSER="${PGUSER:-hana}"
PGDATABASE="${PGDATABASE:-testdb}"
export PGPASSWORD="${PGPASS:-001011}"

GPROM="${GPROM:-$SCRIPT_DIR/../src/command_line/gprom}"
SO="${I4R_SO:-/tmp/i4r_audb_extension.so}"

if ! test -x "$GPROM"; then
	echo "run_uset_pruning_i4r_all14.sh: 设置 GPROM=.../gprom" >&2
	exit 1
fi

# 表 r / r_interval（失败则尝试仅 PG 段）
_setup_ok=0
if psql -h "$PGHOST" -p "$PGPORT" -U "$PGUSER" -d "$PGDATABASE" -v ON_ERROR_STOP=1 \
	-f "$SCRIPT_DIR/uset_pruning_pg_setup.sql" >/dev/null 2>&1; then
	_setup_ok=1
else
	echo "WARN: uset_pruning_pg_setup 失败（多因 hana 无法 DROP postgres 拥有的 prune 函数）" >&2
	echo "      若尚未安装 C 剪枝，请运行: PGPASS=001011 $SCRIPT_DIR/install_i4r_prune_local.sh" >&2
fi

# 确保 C 剪枝可用（本地 .so）
if test -f "$SO"; then
	PGPASSWORD="${PGPASS:-001011}" psql -h "$PGHOST" -p "$PGPORT" -U postgres -d "$PGDATABASE" -v ON_ERROR_STOP=1 \
		-c "SELECT 1" >/dev/null 2>&1 && \
	sed "s|'MODULE_PATHNAME'|'$SO'|g" "${AUD_EXT:-/home/hana4/yangyun/audb/c_extension/i4r_audb_extension}/i4r_audb_extension_prune.sql" \
		2>/dev/null | psql -h "$PGHOST" -p "$PGPORT" -U postgres -d "$PGDATABASE" -v ON_ERROR_STOP=1 >/dev/null 2>&1 \
		|| true
fi

echo "======== [1/2] PostgreSQL 直接调用 14 项 C 剪枝 (prune.c) ========"
psql -h "$PGHOST" -p "$PGPORT" -U "$PGUSER" -d "$PGDATABASE" -v ON_ERROR_STOP=1 \
	-f "$SCRIPT_DIR/i4r_prune_all14_pg.sql"

if test "$_setup_ok" -ne 1; then
	if ! psql -h "$PGHOST" -p "$PGPORT" -U "$PGUSER" -d "$PGDATABASE" -tAc "SELECT 1 FROM r LIMIT 1" 2>/dev/null | grep -q 1; then
		echo "SKIP GProM 段：无表 r，请先执行 uset_pruning_pg_setup.sql" >&2
		exit 0
	fi
	echo "WARN: 使用已有表 r 继续 GProM 测试" >&2
fi

echo ""
echo "======== [2/2] GProM 生成 prune_set_* / prune_range_* 并执行 ========"

run_case() {
	_name="$1"
	_sql="$2"
	_needle="$3"
	_err="$(mktemp)"
	_tmp="$(mktemp)"
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

# Set 层（int 列 → lift 为 int4range[]，对应 prune_*_set_internal）
run_case 'gprom_set_lt'  'USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a < b);'  'prune_set_lt'
run_case 'gprom_set_lte' 'USET WITH PRUNING (SELECT a FROM r IS UADB WHERE a <= 3);'     'prune_set_lte'
run_case 'gprom_set_gt'  'USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a > b);'  'prune_set_gt'
run_case 'gprom_set_gte' 'USET WITH PRUNING (SELECT b FROM r IS UADB WHERE b >= 4);'     'prune_set_gte'
run_case 'gprom_set_eq'  'USET WITH PRUNING (SELECT a FROM r IS UADB WHERE a = 3);'     'prune_set_eq'
run_case 'gprom_set_and' 'USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a = 3 AND a < b);' 'prune_set_and'
run_case 'gprom_set_or'  'USET WITH PRUNING (SELECT a FROM r IS UADB WHERE a = 3 OR a < b);'     'prune_set_or'

# int4range[] 表仍走 set_*（非 range_*）；与注册表 Set 层一致
run_case 'gprom_interval_set_lt' \
	'USET WITH PRUNING (SELECT a, b FROM r_interval IS UADB WHERE a < b);' 'prune_set_lt'

echo ""
echo "All 14 i4r prune tests passed (PG direct 14 + GProM set-layer 8)."
echo "说明: prune_*_internal_range 在 GProM 中对应 prune_range_*，需 WHERE 两侧为 lift_scalar(int4range)；"
echo "      当前 int / int4range[] UADB 表走 set_* 路径。Range 层已由 PG 直接段覆盖。"

#!/usr/bin/env bash
set -euo pipefail

PGHOST="${PGHOST:-localhost}"
PGPORT="${PGPORT:-5432}"
PGUSER="${PGUSER:-hana}"
PGDB="${PGDB:-testdb}"
export PGPASSWORD="${PGPASS:-001011}"

psql_q() {
  psql -h "$PGHOST" -p "$PGPORT" -U "$PGUSER" -d "$PGDB" -v ON_ERROR_STOP=1 -At -c "$1"
}

echo "== PostgreSQL version =="
ver="$(psql_q "SHOW server_version;")"
echo "$ver"
case "$ver" in
  16.*) ;;
  *)
    echo "WARN: expected PostgreSQL 16.x, got $ver" >&2
    ;;
esac

echo "== i4r_audb_extension =="
psql_q "SELECT extname, extversion FROM pg_extension WHERE extname = 'i4r_audb_extension';" \
  | grep -q 'i4r_audb_extension' || {
  echo "ERROR: i4r_audb_extension not installed" >&2
  exit 1
}

echo "== core audb functions =="
for fn in set_lt prune_set_lt lift_scalar int_count_set ic_sum_result; do
  psql_q "SELECT 1 FROM pg_proc p JOIN pg_namespace n ON p.pronamespace = n.oid
          WHERE n.nspname = 'public' AND p.proname = '$fn' LIMIT 1;" | grep -q 1 \
    || { echo "ERROR: missing function $fn" >&2; exit 1; }
  echo "  OK $fn"
done

echo "== experiment tables =="
for tbl in flights_rset hospital_outlier_rset sales_rset; do
  cnt="$(psql_q "SELECT COUNT(*) FROM $tbl;")"
  echo "  $tbl: $cnt rows"
  test "$cnt" -gt 0 || { echo "ERROR: $tbl is empty" >&2; exit 1; }
done

echo "== sanity: set_lt smoke =="
psql_q "SELECT set_lt(ARRAY['[1,2)'::int4range], ARRAY['[3,4)'::int4range]);" | grep -q t

echo "verify_audb_pg16: OK"

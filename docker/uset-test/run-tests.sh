#!/bin/bash
set -euo pipefail

export PGHOST="${PGHOST:-postgres}"
export PGPORT="${PGPORT:-5432}"
export PGUSER="${PGUSER:-hana}"
export PGPASS="${PGPASS:-001011}"
export PGDB="${PGDB:-testdb}"
export PGPASSWORD="$PGPASS"

ROOT=/gprom
GPROM="${GPROM:-/usr/local/bin/gprom}"

echo "== 等待 PostgreSQL =="
for i in $(seq 1 60); do
  if psql -h "$PGHOST" -p "$PGPORT" -U "$PGUSER" -d "$PGDB" -c "SELECT 1;" >/dev/null 2>&1; then
    break
  fi
  sleep 1
done

echo "== verify_audb_pg16 =="
bash "$ROOT/test/verify_audb_pg16.sh"

echo ""
echo "== GProM USET smoke =="
"$GPROM" -backend postgres -frontend oracle \
  -host "$PGHOST" -port "$PGPORT" -user "$PGUSER" -passwd "$PGPASS" -db "$PGDB" \
  -Pmetadata postgres -Psqlcodegen postgres -Pexecutor sql -loglevel 0 \
  -sql "USET (SELECT SUM(act_dep) FROM flights_rset IS UADB WHERE sched_dep < act_dep);"

echo ""
echo "== GProM USET WITH PRUNING smoke =="
"$GPROM" -backend postgres -frontend oracle \
  -host "$PGHOST" -port "$PGPORT" -user "$PGUSER" -passwd "$PGPASS" -db "$PGDB" \
  -Pmetadata postgres -Psqlcodegen postgres -Pexecutor sql -loglevel 0 \
  -sql "USET WITH PRUNING (SELECT SUM(act_dep) FROM flights_rset IS UADB WHERE sched_dep < act_dep);"

if [ "${RUN_FULL:-0}" = "1" ]; then
  echo ""
  echo "== 三库聚合实验 =="
  GPROM="$GPROM" bash "$ROOT/test/run_three_rset_agg_experiments.sh"
fi

echo ""
echo "All tests passed."

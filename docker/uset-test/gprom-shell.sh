#!/bin/bash
# 交互式 USET shell：GProM 改写 SQL + psql 执行并显示结果
set -uo pipefail

export PGHOST="${PGHOST:-postgres}"
export PGPORT="${PGPORT:-5432}"
export PGUSER="${PGUSER:-hana}"
export PGPASS="${PGPASS:-001011}"
export PGDB="${PGDB:-testdb}"
export PGPASSWORD="$PGPASS"

GPROM="${GPROM:-/usr/local/bin/gprom}"
PROMPT="Oracle SQL - Postgres:${PGUSER}@${PGHOST}:${PGDB}\$ "

GPROM_ARGS=(
  -backend postgres -frontend oracle
  -host "$PGHOST" -port "$PGPORT" -user "$PGUSER" -passwd "$PGPASS" -db "$PGDB"
  -Pmetadata postgres -Psqlcodegen postgres -Pexecutor sql -loglevel 0
)

echo "Welcome to the GProM USET interactive shell"
echo "Each query prints rewritten SQL, then PostgreSQL results."
echo "Commands: \\q quit, \\h help"
echo "======================================================================"

run_query() {
  local sql="$1"
  local tmp err rc

  tmp="$(mktemp)"
  err="$(mktemp)"

  if ! "$GPROM" "${GPROM_ARGS[@]}" -sql "$sql" 2>"$err" | sed 's/\x1b\[[0-9;]*m//g' >"$tmp"; then
    echo "GProM error:" >&2
    cat "$err" >&2
    rm -f "$tmp" "$err"
    return 1
  fi
  if [ -s "$err" ]; then
    echo "GProM error:" >&2
    cat "$err" >&2
    rm -f "$tmp" "$err"
    return 1
  fi
  if [ ! -s "$tmp" ]; then
    echo "GProM produced empty SQL." >&2
    rm -f "$tmp" "$err"
    return 1
  fi

  echo ""
  echo "-- Rewritten SQL --"
  cat "$tmp"
  echo ""
  echo "-- Query result --"
  psql -h "$PGHOST" -p "$PGPORT" -U "$PGUSER" -d "$PGDB" -v ON_ERROR_STOP=1 -f "$tmp"
  rc=$?
  echo ""
  rm -f "$tmp" "$err"
  return "$rc"
}

while true; do
  if ! IFS= read -r -p "$PROMPT" sql; then
    echo ""
    break
  fi

  sql="${sql#"${sql%%[![:space:]]*}"}"
  sql="${sql%"${sql##*[![:space:]]}"}"
  [ -z "$sql" ] && continue

  case "$sql" in
    \\q|\\quit|quit|exit)
      break
      ;;
    \\h|\\help|help)
      cat <<'EOF'
Example queries:
  USET (SELECT SUM(act_dep) FROM flights_rset IS UADB WHERE sched_dep < act_dep);
  USET WITH PRUNING (SELECT SUM(act_dep) FROM flights_rset IS UADB WHERE sched_dep < act_dep);
  USET WITH PRUNING (SELECT SUM(Score) FROM hospital_outlier_rset IS UADB WHERE Score > 90 AND Score < 100);
EOF
      continue
      ;;
  esac

  run_query "$sql" || true
done

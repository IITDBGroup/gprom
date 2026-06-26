#!/usr/bin/env bash
# Run flagship SUM queries on flights / hospital / sales (NP vs P, ic_sum, R).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PGHOST="${PGHOST:-localhost}"
PGPORT="${PGPORT:-5432}"
PGUSER="${PGUSER:-hana}"
PGDB="${PGDB:-testdb}"
export PGPASSWORD="${PGPASS:-001011}"

GPROM="${GPROM:-/usr/local/bin/gprom}"
if ! test -x "$GPROM"; then
  GPROM="${SCRIPT_DIR}/../src/command_line/gprom"
fi
if ! test -x "$GPROM"; then
  GPROM="$(command -v gprom || true)"
fi
test -x "$GPROM" || { echo "run_three_rset_agg_experiments.sh: gprom not found" >&2; exit 1; }

OUT="${SCRIPT_DIR}/three_rset_agg_results.tsv"
: >"$OUT"
printf 'dataset\tmode\tsql\tresult\tic_sum\tR\n' >>"$OUT"

run_case() {
  local dataset="$1" mode="$2" sql="$3"
  local tmp err sqlfile out ic
  tmp="$(mktemp)"
  err="$(mktemp)"
  sqlfile="$(mktemp)"

  "$GPROM" -backend postgres -frontend oracle \
    -host "$PGHOST" -port "$PGPORT" -user "$PGUSER" -passwd "$PGPASSWORD" -db "$PGDB" \
    -Pmetadata postgres -Psqlcodegen postgres -Pexecutor sql -loglevel 0 \
    -sql "$sql" 2>"$err" | sed 's/\x1b\[[0-9;]*m//g' >"$sqlfile"

  if test -s "$err"; then
    echo "gprom stderr ($dataset $mode):" >&2
    cat "$err" >&2
    rm -f "$tmp" "$err" "$sqlfile"
    exit 1
  fi

  out="$(psql -h "$PGHOST" -p "$PGPORT" -U "$PGUSER" -d "$PGDB" -v ON_ERROR_STOP=1 -At -f "$sqlfile")"
  ic="$(psql -h "$PGHOST" -p "$PGPORT" -U "$PGUSER" -d "$PGDB" -At \
    -c "SELECT ic_sum_result('$out'::int4range[]);")"
  printf '%s\t%s\t%s\t%s\t%s\t\n' "$dataset" "$mode" "$sql" "$out" "$ic" >>"$OUT"
  echo "---- $dataset ($mode) ic_sum=$ic ----"
  echo "$out"
  rm -f "$tmp" "$err" "$sqlfile"
}

# flights flagship
NP_FL='USET (SELECT SUM(act_dep) FROM flights_rset IS UADB WHERE sched_dep < act_dep);'
P_FL='USET WITH PRUNING (SELECT SUM(act_dep) FROM flights_rset IS UADB WHERE sched_dep < act_dep);'
run_case flights NP "$NP_FL"
run_case flights P  "$P_FL"

# hospital flagship
NP_H='USET (SELECT SUM(Score) FROM hospital_outlier_rset IS UADB WHERE Score > 90 AND Score < 100);'
P_H='USET WITH PRUNING (SELECT SUM(Score) FROM hospital_outlier_rset IS UADB WHERE Score > 90 AND Score < 100);'
run_case hospital NP "$NP_H"
run_case hospital P  "$P_H"

# sales flagship
NP_S='USET (SELECT SUM(daily_sales) FROM sales_rset IS UADB WHERE daily_sales > 1000 AND daily_sales > daily_cost);'
P_S='USET WITH PRUNING (SELECT SUM(daily_sales) FROM sales_rset IS UADB WHERE daily_sales > 1000 AND daily_sales > daily_cost);'
run_case sales NP "$NP_S"
run_case sales P  "$P_S"

python3 - "$OUT" <<'PY'
import sys
from pathlib import Path

rows = []
for line in Path(sys.argv[1]).read_text().splitlines()[1:]:
    parts = line.split("\t")
    if len(parts) < 5:
        continue
    dataset, mode, _sql, _res, ic = parts[:5]
    rows.append((dataset, mode, int(ic)))

by_ds = {}
for ds, mode, ic in rows:
    by_ds.setdefault(ds, {})[mode] = ic

print("\n== R = ic_sum(P) / ic_sum(NP) ==")
for ds, m in sorted(by_ds.items()):
    np, p = m.get("NP"), m.get("P")
    if np and p:
        print(f"  {ds}: {p}/{np} = {p/np:.2f}")
PY

echo "Results written to $OUT"
echo "three_rset_agg_experiments: OK"

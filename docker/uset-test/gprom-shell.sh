#!/bin/bash
export PGHOST="${PGHOST:-postgres}"
export PGPORT="${PGPORT:-5432}"
export PGUSER="${PGUSER:-hana}"
export PGPASS="${PGPASS:-001011}"
export PGDB="${PGDB:-testdb}"

exec /usr/local/bin/gprom -backend postgres -frontend oracle \
  -host "$PGHOST" -port "$PGPORT" -user "$PGUSER" -passwd "$PGPASS" -db "$PGDB" \
  -Pmetadata postgres -Psqlcodegen postgres -Pexecutor sql -loglevel 0

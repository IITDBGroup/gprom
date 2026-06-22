#!/bin/bash
set -euo pipefail

psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname testdb <<'EOSQL'
\i /docker-entrypoint-initdb.d/sql/prune_effect_metrics.sql
\i /docker-entrypoint-initdb.d/sql/flights_rset.sql
\i /docker-entrypoint-initdb.d/sql/hospital_outlier_rset.sql
EOSQL

psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname testdb <<'EOSQL'
DROP TABLE IF EXISTS sales_rset;
CREATE TABLE sales_rset (
  store_id int, region text,
  daily_sales int4range[], daily_cost int4range[], u_r int
);
EOSQL

psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname testdb \
  -c "\copy sales_rset FROM '/docker-entrypoint-initdb.d/data/sales_rset.csv' WITH (FORMAT csv, HEADER true)"

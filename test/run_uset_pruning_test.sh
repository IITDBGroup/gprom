#!/bin/sh
PGPASSWORD=${PGPASS:-001011} psql -h localhost -U hana -d testdb -v ON_ERROR_STOP=1 -f /home/hana4/yangyun/gprom/test/uset_pruning_handcrafted_validate.sql

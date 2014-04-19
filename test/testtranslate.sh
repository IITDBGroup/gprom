#!/bin/bash
if [ $# -lt 2 ]; then
    echo "give at least two parameters, the first one is loglevel, the second one is SQL code"
    echo "testrewriters.sh 3 \"SELECT * FROM r;\""
    exit 1
fi
LOGLEVEL=$1
SQL=$2
ARGS="${*:3}"
./test/testtranslate -host ligeti.cs.iit.edu -db orcl -port 1521 -user fga_user -passwd "fga" -log -loglevel ${LOGLEVEL} -sql "${SQL}" -activate treefiy_prov_rewrite_input ${ARGS}

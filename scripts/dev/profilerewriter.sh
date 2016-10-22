#!/bin/bash
if [ $# -lt 3 ]; then
    echo "give at least three parameters, the first one is loglevel, the second one is SQL code, third one is a the CPU sampling frequency"
    echo "testrewriters.sh 3 \"SELECT * FROM r;\" 40000"
    exit 1
fi
LOGLEVEL=$1
SQL=$2
FREQ=$3
ARGS="${*:4}"
CPUPROFILE=./cpu CPUPROFILE_FREQUENCY=${FREQ} ./test/testrewriter -host ligeti.cs.iit.edu -db orcl -port 1521 -user fga_user -passwd "fga" -log -loglevel ${LOGLEVEL} -sql "${SQL}" -activate treefiy_prov_rewrite_input ${ARGS}


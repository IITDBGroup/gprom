#!/bin/bash
if [ $# -lt 2 ]; then
    echo "give at least two parameters, the first one is loglevel, the second one is SQL code"
    echo "testrewriters.sh 3 \"SELECT * FROM r;\""
    exit 1
fi
LOGLEVEL=$1
SQL=`echo "$2" | tr '\n' ' '`
ARGS="${*:3}"
valgrind --log-file=./valgrind.txt --tool=memcheck -v --track-origins=yes --leak-check=full --show-leak-kinds=all --malloc-fill=01 --free-fill=FF --leak-check=yes --dsymutil=yes --read-var-info=yes ./test/testrewriter -host ligeti.cs.iit.edu -db orcl -port 1521 -user tpch -passwd "IaDdpdr" -log -loglevel ${LOGLEVEL} -sql "${SQL}" -activate treefiy_prov_rewrite_input ${ARGS}

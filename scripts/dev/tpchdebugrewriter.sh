#!/bin/bash
if [ $# -le 1 ]; then
    echo "give at least two parameters, the first one is loglevel, the second one is SQL code"
    echo "debugrewriters.sh 3 \"SELECT * FROM r;\""
    exit 1
fi
LOGLEVEL=$1
SQL=`echo "$2" | tr '\n' ' '`
ARGS="${*:3}"
SCRIPT=debug.script

echo "run -host ligeti.cs.iit.edu -db orcl -port 1521 -user tpch -passwd "IaDdpdr" -log -loglevel ${LOGLEVEL} -activate treefiy_prov_rewrite_input -sql \"${SQL}\" ${ARGS}" > ./$SCRIPT
gdb test/testrewriter -x $SCRIPT
#rm -f $SCRIPT

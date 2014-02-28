#!/bin/bash
if [ "$#X" != "2X" ]; then
    echo "give two parameters, the first one is loglevel, the second one is SQL code"
    echo "valgrindrewriter.sh 3 \"SELECT * FROM r;\""
    exit 1
fi
LOGLEVEL=$1
SQL=$2
valgrind --log-file=./valgrind.txt --suppressions=/home/lordpretzel/scripts/valgrind.supress --tool=memcheck -v --track-origins=yes --malloc-fill=01 --free-fill=FF --leak-check=yes ./test/testrewriter -host ligeti.cs.iit.edu -db orcl -port 1521 -user fga_user -passwd "fga" -log -loglevel ${LOGLEVEL} -sql "${SQL}"

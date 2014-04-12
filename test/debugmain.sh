#!/bin/bash
if [ $# -le 1 ]; then
	LOG=""
else
	LOG="-log -loglevel $1"	
	ARGS="${*:2}"
fi
SCRIPT=debug.script

echo "run -host ligeti.cs.iit.edu -db orcl -port 1521 -user fga_user -passwd \"fga\" ${LOG} ${ARGS}" > ./$SCRIPT
gdb test/testmain -x $SCRIPT

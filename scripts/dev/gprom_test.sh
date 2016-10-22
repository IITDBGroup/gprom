#!/bin/bash
if [ "$1x" != "X" ]; then
   LOG="-log -loglevel $1"
else
   LOG=""
fi
./test/testmain -host ligeti.cs.iit.edu -db orcl -port 1521 -user fga_user -passwd "fga" ${LOG}

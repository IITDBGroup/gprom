#!/bin/bash
LOGLEVEL=$1
SQL=$2
./test/testserializer -host ligeti.cs.iit.edu -db orcl -port 1521 -user fga_user -passwd "fga" -log -loglevel ${LOGLEVEL} -sql "${SQL}"

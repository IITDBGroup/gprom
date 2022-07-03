#!/bin/bash


QUERY=$1
#PSQL=/home/oracle/anton/code/postgresql-9.6beta3-temporal/server/bin/psql -p 5400 -d tpg -h localhost -f



#/home/oracle/datasets/postgres11ps/install/bin/psql -d $DB -p 5440 -f $QUERY > /dev/null;

value=$(<$QUERY)  

#python3 ./123/test1.py
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/Users/liuziyu/ProjectLibrary/instantclient
echo "$value"
./src/command_line/gprom -host ligeti.cs.iit.edu -db pdb1 -user TPCH_1GB -passwd IaDdpdr -port 1521 -log -loglevel 4 -sql $value -Pexecutor sql -Boracle.servicename TRUE -backend oracle

#
    
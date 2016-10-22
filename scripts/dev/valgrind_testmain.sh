#!/bin/bash
if [ $# -lt 1 ]; then
    echo "give at least one parameters, the first one is loglevel"
    echo "testrewriters.sh 3"
    exit 1
fi
LOGLEVEL=$1
ARGS="${*:2}"

# SET PATH TO FIND libgprom
DIR=$(pwd)/$(dirname "${0}")
DYLD_LIBRARY_PATH="${DIR}/../src/libgprom/.libs:$DYLD_LIBRARY_PATH"
DYLD_LIBRARY_PATH=`echo "$DYLD_LIBRARY_PATH" | /sw/bin/sed 's/::*$//'`
export DYLD_LIBRARY_PATH
echo "DYLD_LIBRARY_PATH ${DYLD_LIBRARY_PATH}"

valgrind --log-file=./valgrind.txt --dsymutil=yes --tool=memcheck -v --track-origins=yes --leak-check=full --show-leak-kinds=all --malloc-fill=01 --free-fill=FF --leak-check=yes ./test/.libs/testmain -host ligeti.cs.iit.edu -db orcl -port 1521 -user fga_user -passwd "fga" -log -loglevel ${LOGLEVEL} ${ARGS}

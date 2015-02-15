#!/bin/bash
if [ $# -le 1 ]; then
	LOG=""
else
	LOG="-log -loglevel $1"	
	ARGS="${*:2}"
fi
SCRIPT=debug.script
DIR=$(pwd)/$(dirname "${0}")

# SET PATH TO FIND libgprom
DYLD_LIBRARY_PATH="${DIR}/../src/libgprom/.libs:$DYLD_LIBRARY_PATH"
DYLD_LIBRARY_PATH=`echo "$DYLD_LIBRARY_PATH" | /sw/bin/sed 's/::*$//'`
export DYLD_LIBRARY_PATH
echo "DYLD_LIBRARY_PATH ${DYLD_LIBRARY_PATH}"

echo "run -host ligeti.cs.iit.edu -db orcl -port 1521 -user fga_user -passwd \"fga\" ${LOG} ${ARGS}" > ./$SCRIPT
gdb test/.libs/testmain -x $SCRIPT

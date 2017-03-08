#!/bin/bash
########################################
# VARS
# Directory this script resides in
pushd $(dirname "${0}") > /dev/null
DIR=$(pwd -L)
popd > /dev/null
# GProM binary
CONF_FILE=.gprom
GPROM=${DIR}/../../src/command_line/gprom
GPROM_CONF=${DIR}/../../${CONF_FILE}
########################################
# READ USER CONFIGUATION
source ${DIR}/../gprom_basic.sh
########################################
if [ $# -lt 1 ]; then
    echo "give at least one parameters, a loglevel"
	echo " "
    echo "valgrind_testmain.sh 3"
    exit 1
fi
LOG="-log -loglevel $1"
ARGS="${*:2}"

# SET PATH TO FIND libgprom
DYLD_LIBRARY_PATH="${DIR}/../../src/libgprom/.libs:$DYLD_LIBRARY_PATH"
DYLD_LIBRARY_PATH=`echo "$DYLD_LIBRARY_PATH" | /sw/bin/sed 's/::*$//'`
export DYLD_LIBRARY_PATH
echo "DYLD_LIBRARY_PATH ${DYLD_LIBRARY_PATH}"

valgrind --log-file=./valgrind.txt --dsymutil=yes --tool=memcheck -v --track-origins=yes --leak-check=full --show-leak-kinds=all --malloc-fill=01 --free-fill=FF --leak-check=yes ${DIR}/../../test/testmain ${CONNECTION_PARAMS} ${LOG} ${ARGS}

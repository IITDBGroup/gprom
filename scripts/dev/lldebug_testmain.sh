#!/bin/bash
########################################
# VARS
# Directory this script resides in
pushd $(dirname "${0}") > /dev/null
DIR=$(pwd -L)
popd > /dev/null
# GProM binary
CONF_FILE=.gprom
GPROM=${DIR}/../../test/testmain
GPROM_CONF=${DIR}/../../${CONF_FILE}
########################################
# READ USER CONFIGUATION
source ${DIR}/../gprom_basic.sh
########################################
if [ $# -le 0 ]; then
	LOG="-log -loglevel 0"
	ARGS="${*}"
else
	LOG="-log -loglevel $1"	
	ARGS="${*:2}"
fi
LLDB=lldb
LOG="-log -loglevel $1"
SQL="$2"
ARGS="${*:3}"
SCRIPT=debug.script

echo "
process launch -- ${GPROM} ${LOG} ${ARGS}
"

${LLDB} process launch -- ${GPROM} ${LOG} ${ARGS}
#rm -f $SCRIPT

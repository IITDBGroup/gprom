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
########################################
# READ USER CONFIGUATION
if [ $# -le 1 ]; then
	echo "Description: use lldb to debug gprom"
	echo " "
    echo "Usage: give at least two parameters, the first one is a loglevel, the second one is a query."
    echo "lldebug_gprom.sh 3 \"SELECT * FROM r;\""
    exit 1
fi
LLDB=lldb
LOG="-log -loglevel $1"
SQL="$2"
ARGS="${*:3}"

${LLDB} ${GPROM} process launch -- ${LOG} -treeify-algebra-graphs -sql "${SQL}" ${ARGS}
#rm -f $SCRIPT

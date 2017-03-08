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
if [ $# -lt 2 ]; then
    echo "give at least two parameters, the first one is loglevel, the second one is SQL code"
	echo " "
    echo "callgrind_gprom.sh 3 \"SELECT * FROM r;\""
    exit 1
fi

LOGLEVEL=$1
SQL=$2
ARGS="${*:3}"

valgrind --tool=callgrind -v ${GPROM} ${CONNECTION_PARAMS} ${LOG} -sql "${SQL}" -treeify-algebra-graphs ${ARGS}

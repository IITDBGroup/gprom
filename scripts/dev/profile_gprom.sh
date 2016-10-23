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
if [ $# -lt 3 ]; then
    echo "give at least three parameters, the first one is loglevel, the second one is SQL code, third one is a the CPU sampling frequency"
    echo "profile_gprom.sh 3 \"SELECT * FROM r;\" 40000"
    exit 1
fi

LOG="-log -loglevel $1"
SQL=$2
FREQ=$3
ARGS="${*:4}"

CPUPROFILE=./cpu CPUPROFILE_FREQUENCY=${FREQ} ${GPROM} ${CONNECTION_PARAMS} ${LOG} -sql "${SQL}" -treeify-algebra-graphs ${ARGS}


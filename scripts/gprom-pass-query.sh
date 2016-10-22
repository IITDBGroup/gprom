#!/bin/bash
########################################
# VARS
# Directory this script resides in
pushd $(dirname "${0}") > /dev/null
DIR=$(pwd -L)
popd > /dev/null
# GProM binary
CONF_FILE=.gprom
GPROM=${DIR}/../src/command_line/gprom
GPROM_CONF=${DIR}/../${CONF_FILE}
########################################
# READ USER CONFIGUATION
source ${DIR}/gprom_basic.sh
########################################
# PARAMETERS
PROGRAM="${2}"
LOG="-log -loglevel ${1}"
ARGS="${*:3}"

if [ $# -lt 2 ]; then
    echo "Provide at least two parameters, the first one is the loglevel [0 : NONE up to 5 : TRACE] and the second one is a query."
    echo "gprom-pass-arg.sh 3 \"SELECT * FROM r;\""
    exit 1
fi

########################################
# RUN COMMAND
${GPROM} ${LOG} -sql "${PROGRAM}" ${CONNECTION_PARAMS} -treeify-algebra-graphs ${*:3}

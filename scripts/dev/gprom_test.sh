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
if [ "$1X" != "X" ]; then
   LOG="-log -loglevel $1"
else
   LOG=""
fi

echo "${DIR}/../../test/testmain ${CONNECTION_PARAMS} ${LOG}"
${DIR}/../../test/testmain ${CONNECTION_PARAMS} ${LOG}

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
# RUN COMMAND
${GPROM} ${LOG} ${CONNECTION_PARAMS} -treeify-algebra-graphs -Pmetadata sqlite -Psqlserializer sqlite ${*}

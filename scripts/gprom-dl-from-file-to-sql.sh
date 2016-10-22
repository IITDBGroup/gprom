#!/bin/bash
########################################
# VARS
# Directory this script resides in
pushd $(dirname "${0}") > /dev/null
DIR=$(pwd -L)
popd > /dev/null
# GProM script
GPROM_SCRIPT=${DIR}/gprom.sh
CONF_FILE=.gprom
GPROM_CONF=${DIR}/../${CONF_FILE}
########################################
# READ USER CONFIGUATION
source ${DIR}/gprom_basic.sh
########################################
# PARAMETERS
DLFILE="${2}"
LOG="-log -loglevel ${1}"

if [ $# -lt 2 ]; then
	echo "Description: read a Datalog program with provenance requests and regular path queries from a file and translate into SQL."
	echo " "
    echo "Usage: pass at least two parameters, the first one is the loglevel [0 : NONE up to 5 : TRACE] and the second one is the file storing the input program."
	echo " "
    echo "gprom-dl-from-file-to-sql.sh 3 myfile.dl"
    exit 1
fi

if [ ! -f ${DLFILE} ]; then
	echo "File ${DLFILE} not found!"
	exit 1
fi

########################################
# RUN COMMAND
${GPROM_SCRIPT} ${CONNECTION_PARAMS} ${LOG} ${GPROM_DL_PLUGINS} -Pexecutor sql ${*:3} -sqlfile ${DLFILE} 

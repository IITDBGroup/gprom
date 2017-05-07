#!/bin/bash
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
if [ $# -le 1 ]; then
	echo "Description: use gdb to debug gprom for Datalog queries"
	echo " "
    echo "Usage: give at least two parameters, the first one is a loglevel, the second one is a Datalog query."
    echo "debug_gprom.sh 3 \"Q(X) :- R(X,Y).;\""
    exit 1
fi
PROGRAM="${2}"
LOG="${1}"
${DIR}/debug_gprom.sh ${LOG} "${PROGRAM}" ${GPROM_DL_PLUGINS} -Pexecutor sql -treeify-algebra-graphs FALSE ${*:3} 

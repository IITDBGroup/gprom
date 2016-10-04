#!/bin/bash
########################################
# VARS
# Directory this script resides in
pushd $(dirname "${0}") > /dev/null
DIR=$(pwd -L)
popd > /dev/null
# GProM binary
GPROM=${DIR}/../src/command_line/gprom
########################################
# PARAMETERS
PROGRAM="${2}"
LOG="${1}"
ARGS="${*:3}"

if [ $# -lt 2 ]; then
    echo "give at least two parameters, the first one is the loglevel [0 : NONE up to 5 : TRACE] and the second one is a query."
    echo "gprom-pass-arg.sh 3 \"SELECT * FROM r;\""
    exit 1
fi

########################################
# RUN COMMAND
${GPROM} -log -loglevel ${LOG} -sql "${PROGRAM}" -host ligeti.cs.iit.edu -db orcl -port 1521 -user fga_user -passwd "fga" -treeify-algebra-graphs ${*:3}


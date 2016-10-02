#!/bin/bash
########################################
# VARS
# Directory this script resides in
pushd $(dirname "${0}") > /dev/null
DIR=$(pwd -L)
popd > /dev/null
# GProM script
GPROM_SCRIPT=${DIR}/gprom.sh

########################################
# PARAMETERS
DLFILE="${2}"
LOG="${1}"

if [ $# -lt 2 ]; then
	echo "Description: read a Datalog program with provenance requests and regular path queries from a file and translate into SQL."
    echo "Usage: pass at least two parameters, the first one is the loglevel [0 : NONE up to 5 : TRACE] and the second one is the file storing the input program."
    echo "gprom-dl-from-file-to-sql.sh 3 myfile.dl"
    exit 1
fi

if [ ! -f ${DLFILE} ]; then
	echo "File ${DLFILE} not found!"
	exit 1
fi

########################################
# RUN COMMAND
 ${LOG} "${PROGRAM}" -Pparser dl -Panalyzer dl -Ptranslator dl -Pexecutor sql -Cattr_reference_consistency FALSE -Cschema_consistency FALSE  -Cunique_attr_names FALSE -deactivate treefiy_prov_rewrite_input  ${*:3}
# -Cattr_reference_consistency FALSE -Cschema_consistency FALSE  -Cunique_attr_names FALSE -Oselections_move_around FALSE  -Oremove_redundant_projections FALSE  -activate optimize_operator_model

${GPROM_SCRIPT} -host ligeti.cs.iit.edu -db orcl -port 1521 -user fga_user -passwd "fga"  -log -loglevel ${LOG} -Pparser dl -Panalyzer dl -Ptranslator dl -Pexecutor sql ${*:3} < ${DLFILE} 

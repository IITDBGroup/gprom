#!/bin/bash
########################################
# VARS
# Directory this script resides in
pushd $(dirname "${0}") > /dev/null
DIR=$(pwd -L)
popd > /dev/null
# GProM script
GPROM_SCRIPT=${DIR}/gprom-pass-query.sh

########################################
# PARAMETERS
PROGRAM="${2}"
LOG="${1}"

if [ $# -lt 2 ]; then
	echo "Description: translate Datalog program with provenance requests and regular path queries into SQL."
    echo "Usage: pass at least two parameters, the first one is the loglevel [0 : NONE up to 5 : TRACE] and the second one is a query."
    echo "gprom-dl-to-sql.sh 3 \"Q(X) :- R(X,Y). WHY(Q(1)).\""
    exit 1
fi

########################################
# RUN COMMAND
${GPROM_SCRIPT} ${LOG} "${PROGRAM}" -Pparser dl -Panalyzer dl -Ptranslator dl -Pexecutor sql -Cattr_reference_consistency FALSE -Cschema_consistency FALSE  -Cunique_attr_names FALSE -deactivate treefiy_prov_rewrite_input  ${*:3}
# -Cattr_reference_consistency FALSE -Cschema_consistency FALSE  -Cunique_attr_names FALSE -Oselections_move_around FALSE  -Oremove_redundant_projections FALSE  -activate optimize_operator_model

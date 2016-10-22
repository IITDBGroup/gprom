#!/bin/bash
########################################
# VARS
# Directory this script resides in
pushd $(dirname "${0}") > /dev/null
DIR=$(pwd -L)
popd > /dev/null
# GProM script
GPROM_SCRIPT=${DIR}/gprom-pass-query.sh
CONF_FILE=.gprom
GPROM_CONF=${DIR}/../${CONF_FILE}
########################################
# READ USER CONFIGUATION
source ${DIR}/gprom_basic.sh
########################################
# PARAMETERS
PROGRAM="${1}"
OUT="${2}"
DOTFILE="${OUT}.dot"
PDFFILE="${OUT}.pdf"

if [ $# -lt 2 ]; then
	echo "Description: run a Datalog program with provenance requests, create a dot script of the resulting provenance graph, and create a pdf from this dot script using graphviz."
    echo "Usage: pass at least two parameters, the first one is the loglevel [0 : NONE up to 5 : TRACE] and the second one is a query."
    echo "gprom-dl-prov-graph.sh \"Q(X) :- R(X,Y). WHY(Q(1)).\" my_prov_graph" 
    exit 1
fi

########################################
# RUN COMMAND
##########
echo "-- compute edge relation of provenance graph"
${GPROM_SCRIPT} 0 "${1}" ${GPROM_DL_PLUGINS} -Pexecutor gp -Cattr_reference_consistency FALSE -Cunique_attr_names FALSE -Cschema_consistency FALSE ${*:3} > ${DOTFILE}
##########
echo "-- run graphviz on ${DOTFILE} to produce PDF file ${PDFFILE}"
dot -Tpdf -o ${PDFFILE} ${DOTFILE}
##########
echo "-- if running on a mac open the pdf file"
if [[ $OSTYPE == darwin* ]]; then 
	open ${PDFFILE}
fi

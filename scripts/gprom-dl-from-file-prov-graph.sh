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
DLFILE="${1}"
OUT="${2}"
DOTFILE="${OUT}.dot"
PDFFILE="${OUT}.pdf"

if [ $# -lt 2 ]; then
	echo "Description: read a Datalog program with provenance requests from an input file, run it, create a dot script of the resulting provenance graph, and create a pdf from this dot script using graphviz."
    echo "Usage: pass at least two parameters, the first one is the file storing the Datalog query and provenance request and the second one is the name of the file storing the output provenance graph."
    echo "gprom-dl-prov-graph.sh \"Q(X) :- R(X,Y). WHY(Q(1)).\" my_prov_graph myquery.dl" 
    exit 1
fi

if [ ! -f ${DLFILE} ]; then
	echo "File ${DLFILE} not found!"
	exit 1
fi

########################################
# RUN COMMAND

echo "-- compute edge relation of provenance graph for ${DLFILE}"
${GPROM_SCRIPT} -log -loglevel 0 -Pparser dl -Panalyzer dl -Ptranslator dl -Pexecutor gp -Cattr_reference_consistency FALSE -Cunique_attr_names FALSE -Cschema_consistency FALSE ${*:3} -sqlfile ${DLFILE} > ${DOTFILE}

echo "-- run graphviz on ${DOTFILE} to produce PDF file ${PDFFILE}"
dot -Tpdf -o ${PDFFILE} ${DOTFILE}

echo "-- if running on a mac open the pdf file"
if [[ $OSTYPE == darwin* ]]; then 
	open ${PDFFILE}
fi

#!/bin/bash
########################################
# PARAMETERS
INFILE="${1}"
OUTFILE="${2}"

if [ $# != 2 ]; then
    echo "Pass an example Datalog file and an output file name. This script evaluates the Datalog program (with optional provenance requests) using GProM over the example SQLite database dlexam.db in this folder. The result is then transformed into a provenance graph pdf file using graphviz (dot)."
	echo " "
    echo "genProvGraph.sh ./3hop.dl 3hop"
    exit 1
fi

########################################
# SCRIPT
SCRIPT=../../scripts/gprom-dl-from-file-prov-graph.sh

########################################
# EXECUTE SCRIPT
echo "${SCRIPT} ${INFILE} ${OUTFILE} -Psqlserializer sqlite -Pmetadata sqlite -db ./dlexam.db"
${SCRIPT} ${INFILE} ${OUTFILE} -Psqlserializer sqlite -Pmetadata sqlite -db ./dlexam.db

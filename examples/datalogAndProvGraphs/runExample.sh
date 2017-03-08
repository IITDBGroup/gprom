#!/bin/bash
########################################
# PARAMETERS
INFILE="${1}"

if [ $# != 1 ]; then
    echo "Pass an example Datalog file. This script evaluates the Datalog program (with optional provenance requests) using GProM and the example SQLite database dlexam.db in this folder."
	echo " "
    echo "runExample.sh ./3hop.dl"
    exit 1
fi

########################################
# SCRIPT
SCRIPT=../../scripts/gprom-dl-from-file-to-sql.sh

########################################
# EXECUTE SCRIPT
${SCRIPT} 0 ${INFILE} -Psqlserializer sqlite -Pmetadata sqlite -db ./dlexam.db -Pexecutor run ${2:*}

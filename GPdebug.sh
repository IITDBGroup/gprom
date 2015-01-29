#!/bin/bash
PROGRAM="${2}"
LOG="${1}"
./test/debugrewriter.sh ${LOG} "${PROGRAM}" -Pparser dl -Panalyzer dl -Ptranslator dl -Pexecutor sql -Cattr_reference_consistency FALSE -Cunique_attr_names FALSE -Cschema_consistency FALSE -activate optimize_operator_model -Oremove_redundant_projections FALSE  ${*:3}

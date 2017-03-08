#!/bin/bash
PROGRAM="${2}"
LOG="${1}"
./test/testrewriter.sh ${LOG} "${PROGRAM}" -Pparser dl -Panalyzer dl -Ptranslator dl -Pexecutor sql -Cattr_reference_consistency FALSE -Cschema_consistency FALSE  -Cunique_attr_names FALSE  ${*:3}
# -Cattr_reference_consistency FALSE -Cschema_consistency FALSE  -Cunique_attr_names FALSE -Oselections_move_around FALSE  -Oremove_redundant_projections FALSE  -activate optimize_operator_model

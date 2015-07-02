#!/bin/bash
PROGRAM="${1}"
./test/testrewriter.sh 0 "${PROGRAM}" -Pparser dl -Panalyzer dl -Ptranslator dl -Pexecutor run -Cattr_reference_consistency FALSE -Cunique_attr_names FALSE -Cschema_consistency FALSE
#-activate optimize_operator_model  -Oremove_redundant_projections FALSE -Oselections_move_around FALSE ${*:2}

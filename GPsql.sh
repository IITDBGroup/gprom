#!/bin/bash
PROGRAM="${2}"
LOG="${1}"
./test/testrewriter.sh ${LOG} "${PROGRAM}" -Pparser dl -Panalyzer dl -Ptranslator dl -Pexecutor sql -Cattr_reference_consistency FALSE -Cschema_consistency FALSE  -Cunique_attr_names FALSE  -Oremove_redundant_projections FALSE ${*:3}
# -Cattr_reference_consistency FALSE -Cschema_consistency FALSE  -Cunique_attr_names FALSE

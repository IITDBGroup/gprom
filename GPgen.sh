#!/bin/bash
PROGRAM="${1}"
OUT="${2}"
echo "-- gen prov"
./test/testrewriter.sh 0 "${1}" -Pparser dl -Panalyzer dl -Ptranslator dl -Pexecutor gp -Cattr_reference_consistency FALSE -Cunique_attr_names FALSE -Cschema_consistency FALSE ${*:3} > ${OUT}.dot
#-activate optimize_operator_model  -Oremove_redundant_projections FALSE -Oselections_move_around FALSE 
echo "-- run graphviz on ${OUT}.dot"
dot -Tpdf -o ${OUT}.pdf ${OUT}.dot
open ${OUT}.pdf

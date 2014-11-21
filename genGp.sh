#!/bin/bash
PROGRAM="${1}"
OUT="${2}"
echo "-- gen prov"
./test/testrewriter.sh 0 "${1}" -Pparser dl -Panalyzer dl -Ptranslator dl -Pexecutor gp -Cattr_reference_consistency FALSE -Cunique_attr_names FALSE -Cschema_consistency FALSE -activate optimize_operator_model > ${OUT}.dot
echo "-- run graphviz on ${OUT}.dot"
dot -Tpdf -o ${OUT}.pdf ${OUT}.dot
open ${OUT}.pdf

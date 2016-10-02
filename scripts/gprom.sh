#!/bin/bash
########################################
# VARS
# Directory this script resides in
pushd $(dirname "${0}") > /dev/null
DIR=$(pwd -L)
popd > /dev/null
# GProM binary
GPROM=${DIR}/../src/command_line/gprom
########################################
# RUN COMMAND
${GPROM} -log -loglevel 0 -host ligeti.cs.iit.edu -db orcl -port 1521 -user fga_user -passwd "fga" -activate treefiy_prov_rewrite_input ${*}



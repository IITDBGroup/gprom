#!/bin/bash
# Directory this script resides in
pushd $(dirname "${0}") > /dev/null
DIR=$(pwd -L)
popd > /dev/null
pushd ${DIR}
ant runSimpleTests
RESULT="${?}"
exit ${RESULT}

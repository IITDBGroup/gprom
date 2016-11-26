#!/bin/bash
# Directory this script resides in
pushd $(dirname "${0}") > /dev/null
DIR=$(pwd -L)
popd > /dev/null
pushd ${DIR}

# check whether oracle jdbc driver is available (otherwise tests won't work)
if [ ! -f ${DIR}/../javalib/ojdbc*.jar ];
then
	echo "Do not have ojdbc driver - do not run tests"
	exit 0
fi

# run ant tests
ant runTests
RESULT="${?}"

# return ant test result
exit ${RESULT}

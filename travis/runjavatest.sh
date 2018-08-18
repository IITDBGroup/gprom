#!/bin/bash

# move to parent directory for build
pushd $(dirname "${0}") > /dev/null
DIR=$(pwd -L)
popd > /dev/null
pushd ${DIR}/..

# copy oracle jdbc driver from container
docker run --rm --name dockerbuild -v "$(pwd)":/gprom  iitdbgroup/gprom_travis:latest  cp /usr/local/oracle/lib/oracle/12.2/client64/lib/ojdbc8.jar /gprom/build/javalib

# for now skip this
echo "SKIP TESTS UNTIL WE HAVE IMPLEMENTED CREDENTIAL PASSING FROM TRAVIS"
exit 0

# run tests
docker run --rm --name dockerbuild -e GPROM_TEST_HOST=${GPROM_TEST_HOST} -e GPROM_TEST_USER=${GPROM_TEST_USER} -e GPROM_TEST_PORT=${GPROM_TEST_PORT} -e GPROM_TEST_PASSWD=${GPROM_TEST_PASSWD} -e GPROM_TEST_REDIR_HOST=${GPROM_TEST_REDIR_HOST} -e GPROM_TEST_REDIR_USER=${GPROM_TEST_REDIR_USER} -e GPROM_TEST_REDIR_PASSWD="${GPROM_TEST_REDIR_PASSWD}" -e SSHPASS="${GPROM_TEST_REDIR_PASSWD}" --net=host -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest /bin/bash /gprom/travis/runjavatest_docker.sh
RESULT=$?

# get back to caller dir and exit with java tests exit status
popd > /dev/null
exit $RESULT


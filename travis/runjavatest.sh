#!/bin/bash

# move to parent directory for build
pushd $(dirname "${0}") > /dev/null
DIR=$(pwd -L)
popd > /dev/null
pushd ${DIR}/..

# for now skip this
exit 0

# vars
SSH_PID_FILE=${DIR}/ssh.pid

# copy oracle jdbc driver from container
docker run --rm -d --name dockerbuild iitdbgroup/gprom_travis:latest sleep 50000
docker cp dockerbuild:/usr/local/oracle/lib/oracle/12.2/client64/lib/ojdbc8.jar ./build/javalib
docker stop dockerbuild
docker rm dockerbuild

export SSHPASS="${GPROM_TEST_REDIR_PASSWD}"

# create an ssh tunnel
ssh -o 'StrictHostKeyChecking no' ${GPROM_TEST_REDIR_HOST} cat /etc/ssh/ssh_host_dsa_key.pub >> ~/.ssh/known_hosts

sshpass -e ssh -f -N -L ${GPROM_TEST_PORT}:${GPROM_TEST_HOST}:${GPROM_TEST_PORT} ${GPROM_TEST_REDIR_USER}@${GPROM_TEST_REDIR_HOST}
echo $! > ${SSH_PID_FILE}
# run tests
#TODO docker run --rm --name dockerbuild -e GPROM_TEST_HOST=${GPROM_TEST_HOST} -e GPROM_TEST_USER=${GPROM_TEST_USER} -e GPROM_TEST_PORT=${GPROM_TEST_PORT} -e GPROM_TEST_PASSWD=${GPROM_TEST_PASSWD} -v "$(pwd)":/gprom iitdbgroup/gprom_travis:latest ant -Dskipivy=true -f blackboxtests/build.xml run-test-only

# kill ssh redirect
echo "kill " `cat ${SSH_PID_FILE}`
kill `cat ${SSH_PID_FILE}`

echo "SKIP TESTS UNTIL WE HAVE IMPLEMENTED CREDENTIAL PASSING FROM TRAVIS"
RESULT=$?

popd > /dev/null
exit $RESULT


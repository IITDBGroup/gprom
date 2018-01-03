#!/bin/bash

# vars
export SSH_PID_FILE=/gprom/ssh.pid

# do not check known hosts
mkdir -p $HOME/.ssh
sshpass -e ssh -o 'StrictHostKeyChecking no' ${GPROM_TEST_REDIR_USER}@${GPROM_TEST_REDIR_HOST} cat /home/${GPROM_TEST_REDIR_USER}/.ssh/id_rsa.pub  >> ~/.ssh/known_hosts

# create SSH tunnel
sshpass -e ssh -f -N -L ${GPROM_TEST_PORT}:${GPROM_TEST_HOST}:${GPROM_TEST_PORT} ${GPROM_TEST_REDIR_USER}@${GPROM_TEST_REDIR_HOST}
ps aux | grep ssh | grep -v 'color' | awk ' { print $2 }' > ${SSH_PID_FILE}

# run the tests and store result
#echo "host: ${GPROM_TEST_HOST} user: ${GPROM_TEST_USER} pwd: ${GPROM_TEST_PASSWD} port:${GPROM_TEST_PORT}"
ant -f blackboxtests/build.xml -Dlogj2.level=debug -Dskipivy=true -DHost="${GPROM_TEST_HOST}" -DUser="${GPROM_TEST_USER}" -DPassword="${GPROM_TEST_PASSWD}" -DPort="${GPROM_TEST_PORT}" run-tests
RESULT=$?

# kill ssh redirect
echo "kill " `cat ${SSH_PID_FILE}`
kill `cat ${SSH_PID_FILE}`
rm ${SSH_PID_FILE}

echo "RESULT WAS ${RESULT}"

# return exit value of tests
exit ${RESULT}

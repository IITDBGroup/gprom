#!/bin/bash

# function to test exist status
testexit()
{
	LOG_FILE="$1"
	EXIT_STATUS=$?
	if [ "x${EXIT_STATUS}" != "x0" ]; then
	        echo "COMMAND EXITED WITH STATUS ${EXIT_STATUS}"
		echo "LOG OUTPUT:"
		cat ${LOG_FILE}
		exit ${EXIT_STATUS}
	fi
}

pushd $(dirname "${0}") > /dev/null
BASEDIR=$(pwd -L)
popd > /dev/null

echo "${BASEDIR}"


APP_NAME=gprom
VERSION=`${BASEDIR}/../configure --version | grep -o '[0-9]\+\.[0-9]\+\.[0-9]\+'`

TMP_DIR=${BASEDIR}/../dpkg
PACKAGE_FILES=${BASEDIR}/debfiles
TAR_NAME=${APP_NAME}-${VERSION}.tar.gz
TAR_PACKAGE=${BASEDIR}/../${TAR_NAME}

# Create a deb package
rm -rf ${TMP_DIR}
mkdir -p ${TMP_DIR}
cp ${TAR_PACKAGE} ${TMP_DIR}/
echo "---------- UNTAR"
tar --directory ${TMP_DIR}/ -xzf ${TMP_DIR}/${TAR_NAME} > log.debpackage >&1
testexit log.debpackage

echo "---------- PREPARE"
pushd ${TMP_DIR}/${APP_NAME}-${VERSION}/
echo "dh_make --single --copyright gpl -e bglavic@iit.edu -f ${BASEDIR}/../${TAR_NAME} -y"
dh_make --single --copyright gpl -e bglavic@iit.edu -f ${BASEDIR}/../${TAR_NAME} -y > log.debpackage >&1
testexit log.debpackage
popd
cp ${PACKAGE_FILES}/changelog ${PACKAGE_FILES}/control ${PACKAGE_FILES}/copyright ${PACKAGE_FILES}/rules ${TMP_DIR}/${APP_NAME}-${VERSION}/debian/ > log.debpackage >&1
testexit log.debpackage

#exit 0

echo "---------- BUILD PACKAGE"
pushd ${TMP_DIR}/${APP_NAME}-${VERSION}/
pwd
rm ./debian/README.Debian debian/*.ex debian/*.EX > log.debpackage >&1
testexit log.debpackage
dpkg-buildpackage -rfakeroot > log.debpackage >&1
testexit log.debpackage
popd

cp ${TMP_DIR}/*.deb ${BASEDIR}/
testexit
rm -rf ${TMP_DIR}

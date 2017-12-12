#!/bin/bash

APP_NAME=gprom
VERSION=`../configure --version | grep -o '[0-9]\+\.[0-9]\+\.[0-9]\+'`

pushd $(dirname "${0}") > /dev/null
BASEDIR=$(pwd -L)
popd > /dev/null

TMP_DIR=${BASEDIR}/../dpkg
PACKAGE_FILES=${BASEDIR}/debfiles
TAR_NAME=${APP_NAME}-${VERSION}.tar.gz
TAR_PACKAGE=${BASEDIR}/../${TAR_NAME}

# Create a deb package
rm -rf ${TMP_DIR}
mkdir -p ${TMP_DIR}
cp ${TAR_PACKAGE} ${TMP_DIR}/
echo "---------- UNTAR"
tar --directory ${TMP_DIR}/ -xzf ${TMP_DIR}/${TAR_NAME}

echo "---------- PREPARE"
pushd ${TMP_DIR}/${APP_NAME}-${VERSION}/
dh_make --single --copyright gpl -e bglavic@iit.edu -f ../${TAR_NAME} -y
popd
cp ${PACKAGE_FILES}/changelog ${PACKAGE_FILES}/control ${PACKAGE_FILES}/copyright ${PACKAGE_FILES}/rules ${TMP_DIR}/${APP_NAME}-${VERSION}/debian/

echo "---------- BUILD PACKAGE"
pushd ${TMP_DIR}/${APP_NAME}-${VERSION}/
rm debian/README.Debian debian/*.ex debian/*.EX
dpkg-buildpackage -rfakeroot
popd

#mv ../*.deb ${PWD}/packaging
#rm -r ${TMP_DIR}

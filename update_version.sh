#!/bin/bash
if [ $# != "1" ]; then
    echo "usage: update_version.sh VERSION_NUMBER"
fi

pushd $(dirname "${0}") > /dev/null
BASEDIR=$(pwd -L)

VERSION_NUMBER="${1}"

####################
echo "prepare version ${VERSION_NUMBER}"

sed -i "s/revision=\"[0-9]\+\.[0-9]\+\.[0-9]\+\"/revision=\"${VERSION_NUMBER}\"/g" ivy.xml 
sed -i "s/AC_INIT(\[GProM\],\[[0-9]\+\.[0-9]\+\.[0-9]\+\],\[bglavic@iit.edu\])/AC_INIT([GProM],[${VERSION_NUMBER}],[bglavic@iit.edu])/g" ${BASEDIR}/configure.ac

CHANGELOG_FILE=${BASEDIR}/packaging/debfiles/changelog
CHANGELOG_ENTRY="gprom (${VERSION_NUMBER}-1) unstable; urgency=low

  * github tag ${VERSION_NUMBER}

 -- Boris Glavic <bglavic@iit.edu>  $(date)
"

HAS_VERSION=`grep -c -e "${VERSION_NUMBER}" ${CHANGELOG_FILE}`

if [ "X${HAS_VERSION}" == "X0" ]; then
    echo "add changelog entry"
    echo "${CHANGELOG_ENTRY}" > ${BASEDIR}/changelog.tmp
    cat ${CHANGELOG_FILE} >> ${BASEDIR}/changelog.tmp
    cp ${BASEDIR}/changelog.tmp ${CHANGELOG_FILE}
    rm ${BASEDIR}/changelog.tmp
fi

GIT_TAG="v${VERSION_NUMBER}"
echo "check whether git version has tag ${GIT_TAG}"
HAS_TAG=`git show-ref --tags | grep -c "${GIT_TAG}"`

if [ "X${HAS_TAG}" == "X0" ]; then
    echo "tag not present, create it"
    git commit -am 'commit updating reference to version in files to ${GIT_TAG}'
    git tag -am 'GProM version ${GIT_TAG}' ${GIT_TAG}
fi

#sed -i -e 's/VERSION/${VERSION}/g' ${TMP_DIR}/${APP_NAME}-${VERSION}/debian/control
popd > /dev/null

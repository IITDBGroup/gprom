#!/bin/bash
if [ $# != "1" ]; then
    echo "usage: update_version.sh VERSION_NUMBER"
	echo "e.g., update_version.sh 2.1.5"
fi

pushd $(dirname "${0}") > /dev/null
BASEDIR=$(pwd -L)
MYDATE=`date "+%Y-%m-%d"`
VERSION_NUMBER="${1}"

####################
echo "prepare version ${VERSION_NUMBER}"

# update man page
gsed -i "1s/.*/.TH gprom 1 \"${MYDATE}\" \"version ${VERSION_NUMBER}\"/" ${BASEDIR}/doc/gprom.man

# create changelog entry
gsed -i "s/revision=\"[0-9]\+\.[0-9]\+\.[0-9]\+\"/revision=\"${VERSION_NUMBER}\"/g" ivy.xml
gsed -i "s/AC_INIT(\[GProM\],\[[0-9]\+\.[0-9]\+\.[0-9]\+\],\[bglavic@uic.edu\])/AC_INIT([GProM],[${VERSION_NUMBER}],[bglavic@uic.edu])/g" ${BASEDIR}/configure.ac

CHANGELOG_FILE=${BASEDIR}/packaging/debfiles/changelog
CHANGELOG_ENTRY="gprom (${VERSION_NUMBER}-1) unstable; urgency=low

  * github tag ${VERSION_NUMBER}

 -- Boris Glavic <bglavic@uic.edu>  $(date)
"

# create git tag
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

# create tar.gz archive
make -j dist

# create tag and push it
if [ "X${HAS_TAG}" == "X0" ]; then
    echo "tag not present, create it"
    git commit -am "commit updating reference to version in files to ${GIT_TAG}"
    git tag -am "GProM version ${GIT_TAG}" ${GIT_TAG}
    git push origin
    git push --tags origin
fi

# create github release
gh release create v${VERSION_NUMBER} --verify-tag -t "v${VERSION_NUMBER}" --notes-from-tag gprom-${VERSION_NUMBER}.tar.gz

# update debian package control file
gsed -i -e 's/VERSION/${VERSION}/g' ${TMP_DIR}/${APP_NAME}-${VERSION}/debian/control
popd > /dev/null

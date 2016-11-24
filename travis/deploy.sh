#!/bin/bash


# EXIT if not linux build on master
if [ "$TRAVIS_BRANCH" != "master" ]
then
  echo "This commit was made against the $TRAVIS_BRANCH and not the master! No deploy!"
  exit 0
fi
if [[ $TRAVIS_OS_NAME == 'osx' ]]
then
  echo "This build is not our default deployment build! No deploy!"
  exit 0
fi

# Find directory
pushd $(dirname "${0}") > /dev/null
DIR=$(pwd -L)
popd > /dev/null
CONFIGFILE="${DIR}/../config.h"

# check whether this is a tag
IS_TAG=`git describe --exact-match HEAD | grep -o -e '[0-9]\+.[0-9]\+.[0-9]\+'`

if [[ "X$IS_TAG" == "X" ]]
then
	echo "not a tag, do not push release to github"
	exit 0
fi

# grab version
if [[ ! -f ${CONFIGFILE} ]]
then
	echo "config.h not found"
	exit 1
fi

VERSION=`cat ${CONFIGFILE} | grep -o -e ' VERSION "[0-9]*.[0-9]*.[0-9]*"' | grep -o -e '[0-9]\+.[0-9]\+.[0-9]\+'`
API_JSON=$(printf '{"tag_name": "%s","target_commitish": "master","name": "%s","body": "Release of version %s","draft": false,"prerelease": false}' $VERSION $VERSION $VERSION)
echo "${API_JSON}"
curl --data "$API_JSON" https://api.github.com/repos/iitdbgroup/gprom/releases?access_token="${GH_OAUTH}"



# deploy:
  # provider: releases
  # api_key:
  # file: perm-0.1-src.tar.gz
  # on:
  #   repo: IITDBGroup/perm



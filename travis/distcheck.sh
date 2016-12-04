#!/bin/bash

# Directory this script resides in
pushd $(dirname "${0}") > /dev/null
DIR=$(pwd -L)
popd > /dev/null
pushd ${DIR}/..

CONF_OPTIONS=--disable-postgres
 
# Mac OS X
if [[ $TRAVIS_OS_NAME == 'osx' ]]; then
	make dist
	#make distcheck
# LINUX	
else
	make dist
	#make distcheck
fi

popd > /dev/null

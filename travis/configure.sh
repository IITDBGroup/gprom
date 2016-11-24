#!/bin/bash

# Directory this script resides in
pushd $(dirname "${0}") > /dev/null
DIR=$(pwd -L)
popd > /dev/null
pushd ${DIR}/..

# Mac OS X
if [[ $TRAVIS_OS_NAME == 'osx' ]]; then
	YACC="/usr/local/bin/bison -y" LEX=/usr/local/bin/flex ./configure --disable-oracle --disable-postgres
# LINUX	
else
	./configure --disable-oracle --disable-postgres
fi

popd > /dev/null

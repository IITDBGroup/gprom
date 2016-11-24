#!/bin/bash

# Directory this script resides in
pushd $(dirname "${0}") > /dev/null
DIR=$(pwd -L)
popd > /dev/null
pushd ${DIR}/..

# MAC OS X
if [[ $TRAVIS_OS_NAME == 'osx' ]]; then
	glibtoolize
	autoreconf --force --install
# LINUX	
else
	cd ${DIR}/..
	make -f Makefile.maintainer
fi

popd > /dev/null

#!/bin/bash
# MAC OS X
if [[ $TRAVIS_OS_NAME == 'osx' ]]; then
	# Directory this script resides in
	pushd $(dirname "${0}") > /dev/null
	DIR=$(pwd -L)
	popd > /dev/null
	cd ${DIR}/..
	glibtoolize
	autoreconf --force --install
# LINUX	
else
	cd ${DIR}/..
	make -f Makefile.maintainer
fi

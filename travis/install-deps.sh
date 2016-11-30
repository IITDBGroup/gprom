#!/bin/bash

# MAC OS X
if [[ $TRAVIS_OS_NAME == 'osx' ]]; then
	# Directory this script resides in
	pushd $(dirname "${0}") > /dev/null
	DIR=$(pwd -L)
	popd > /dev/null
	# install dependencies
	brew update
	brew upgrade bison
	brew upgrade flex
	brew outdated ant || brew upgrade ant
	brew outdated libtool || brew upgrade libtool
	brew outdated bison || brew upgrade bison
	brew outdated flex || brew upgrade flex
	brew outdated sqlite || brew upgrade sqlite
	brew outdated readline || brew upgrade readline
	# fix libtoolize and libtool
	brew reinstall libtool
	brew unlink bison
	brew link bison --force
	#sudo ln -s `which glibtoolize` /usr/bin/libtoolize
# LINUX	
else
	sudo apt-get -qq update
	sudo apt-get install -y ant autotools-dev autoconf bison flex git libsqlite3-dev libtool libreadline6 libreadline6-dev sqlite3 gnuplot
fi

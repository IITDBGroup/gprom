#!/bin/bash

# MAC OS X
if [[ $TRAVIS_OS_NAME == 'osx' ]]; then
	brew update
	brew outdated ant || brew upgrade ant
	brew outdated libtool || brew upgrade libtool
	brew outdated libsqlite3-dev || brew upgrade libsqlite3-dev
	brew outdated bison || brew upgrade bison
	brew outdated flex || brew upgrade flex
	brew outdated autotools-dev || brew upgrade autotools-dev
	brew outdated sqlite3 || brew upgrade sqlite3
	brew outdated libreadline6 || brew upgrade libreadline6
	brew outdated libreadline6-dev || brew upgrade libreadline6-dev
# LINUX	
else
	sudo apt-get -qq update
	sudo apt-get install -y ant autotools-dev autoconf bison flex git libsqlite3-dev libtool libreadline6 libreadline6-dev sqlite3 gnuplot
fi

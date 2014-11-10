#!/bin/bash
make -f Makefile.maintainer
./mconf.sh
make -j 32

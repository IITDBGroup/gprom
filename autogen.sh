#!/bin/bash
libtoolize
autoreconf --force --install
./configure $@
make

#!/bin/bash
autoreconf --force --install
./configure $@
make
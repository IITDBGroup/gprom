#!/bin/bash
./test/testanalysis -host fourier.cs.iit.edu -db orcl -port 1521 -user tprov -passwd "XA<w67onz" -sql "SELECT * FROM (SELECT a, sum(b) FROM r);" -loglevel 3

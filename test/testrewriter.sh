#!/bin/bash
./test/testrewriter -host fourier.cs.iit.edu -db orcl -port 1521 -user tprov -passwd "XA<w67onz" -sql "PROVENANCE OF (SELECT b FROM r;);" -loglevel 3

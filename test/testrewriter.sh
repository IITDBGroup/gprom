#!/bin/bash
./test/testrewriter -host fourier.cs.iit.edu -db orcl -port 1521 -user tprov -passwd "XA<w67onz" -sql "PROVENANCE AS OF SCN 123 OF(SELECT sum(b) FROM r GROUP BY a,b;);" -loglevel 3

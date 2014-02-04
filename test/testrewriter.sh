#!/bin/bash
./test/testrewriter -host fourier.cs.iit.edu -db orcl -port 1521 -user tprov -passwd "XA<w67onz" -sql "PROVENANCE AS OF SCN 123 OF(SELECT sum(a) FROM r GROUP BY b; );" -loglevel 3

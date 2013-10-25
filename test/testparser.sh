test/testparser -loglevel 3 -sql "SELECT DISTINCT sum(d)*4 AS x, a AS x1,c AS x2 FROM r R,s S WHERE e=5 GROUP BY f, ff ORDER BY T LIMIT 3;"

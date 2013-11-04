test/testparser -loglevel 3 -sql "SELECT DISTINCT sum(d)*4 AS x, a AS x1,c AS x2, d FROM r R,s S, (SELECT c FROM d) C, A JOIN B ON d=e WHERE e=5 GROUP BY f, ff HAVING sum(d)>3 ORDER BY T LIMIT 3;"

test/testparser -log -loglevel 3 -sql "SELECT a FROM (SELECT c FROM d) A JOIN B ON d=e JOIN C ON e=f;"

test/testparser -log -loglevel 3 -sql "SELECT a FROM (SELECT c FROM d) A NATURAL JOIN B;"

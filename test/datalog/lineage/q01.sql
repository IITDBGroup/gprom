CREATE TABLE rtpcq01 AS
  SELECT F0.L_RETURN AS A0, F0.L_LINESTAT AS A1, sum(F0.L_QUANT) AS A2, sum(F0.L_EXTP) AS A3, sum((F0.L_EXTP * (CAST (1 AS DOUBLE) - F0.L_DISC))) AS A4, sum(((F0.L_EXTP * (CAST (1 AS DOUBLE) - F0.L_DISC)) * (CAST (1 AS DOUBLE) + F0.L_TAX))) AS A5, avg(F0.L_QUANT) AS A6, avg(F0.L_EXTP) AS A7, avg(F0.L_DISC) AS A8, count(F0.X1) AS A9 FROM (SELECT F0.L_ORDERKEY AS X1, F0.L_PARTKEY AS X2, F0.L_SUPPKEY AS X3, F0.L_LINENUMBER AS X4, F0.L_QUANTITY AS L_QUANT, F0.L_EXTENDEDPRICE AS L_EXTP, F0.L_DISCOUNT AS L_DISC, F0.L_TAX AS L_TAX, F0.L_RETURNFLAG AS L_RETURN, F0.L_LINESTATUS AS L_LINESTAT, F0.L_SHIPDATE AS L_SHIPDATE, F0.L_COMMITDATE AS X5, F0.L_RECEIPTDATE AS X6, F0.L_SHIPINSTRUCT AS X7, F0.L_SHIPMODE AS X8, F0.L_COMMENT AS X9 FROM LINEITEM AS F0) F0 WHERE (F0.L_SHIPDATE <= '1998-09-01') GROUP BY F0.L_RETURN, F0.L_LINESTAT
   LIMIT 1;
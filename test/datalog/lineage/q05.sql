CREATE TABLE rtpcq05 AS
SELECT F4.N_N AS A0, sum((F2.L_EP * (CAST (1 AS DOUBLE) - F2.L_D))) AS A1
FROM ((((((
SELECT F0.C_CUSTKEY AS C_CK, F0.C_NAME AS C1, F0.C_ADDRESS AS C2, F0.C_NATIONKEY AS C_NK, F0.C_PHONE AS C3, F0.C_ACCTBAL AS C4, F0.C_MKTSEGMENT AS C5, F0.C_COMMENT AS C6
FROM CUSTOMER AS F0) F0 JOIN (
SELECT F0.O_ORDERKEY AS O_OK, F0.O_CUSTKEY AS C_CK, F0.O_ORDERSTATUS AS O1, F0.O_TOTALPRICE AS O2, F0.O_ORDERDATE AS O_OD, F0.O_ORDERPRIORITY AS O3, F0.O_CLERK AS O4, F0.O_SHIPPRIORITY AS O5, F0.O_COMMENT AS O6
FROM ORDERS AS F0) F1 ON ((F0.C_CK = F1.C_CK))) JOIN (
SELECT F0.L_ORDERKEY AS O_OK, F0.L_PARTKEY AS L1, F0.L_SUPPKEY AS L_SK, F0.L_LINENUMBER AS L2, F0.L_QUANTITY AS L3, F0.L_EXTENDEDPRICE AS L_EP, F0.L_DISCOUNT AS L_D, F0.L_TAX AS L5, F0.L_RETURNFLAG AS L6, F0.L_LINESTATUS AS L7, F0.L_SHIPDATE AS L8, F0.L_COMMITDATE AS L9, F0.L_RECEIPTDATE AS L10, F0.L_SHIPINSTRUCT AS L11, F0.L_SHIPMODE AS L12, F0.L_COMMENT AS L13
FROM LINEITEM AS F0) F2 ON ((F1.O_OK = F2.O_OK))) JOIN (
SELECT F0.S_SUPPKEY AS L_SK, F0.S_NAME AS S1, F0.S_ADDRESS AS S2, F0.S_NATIONKEY AS C_NK, F0.S_PHONE AS S3, F0.S_ACCTBAL AS S4, F0.S_COMMENT AS S5
FROM SUPPLIER AS F0) F3 ON (((F2.L_SK = F3.L_SK) AND (F0.C_NK = F3.C_NK)))) JOIN (
SELECT F0.N_NATIONKEY AS C_NK, F0.N_NAME AS N_N, F0.N_REGIONKEY AS N_RK, F0.N_COMMENT AS N1
FROM NATION AS F0) F4 ON (((F0.C_NK = F4.C_NK) AND (F3.C_NK = F4.C_NK)))) JOIN (
SELECT F0.R_REGIONKEY AS N_RK, F0.R_NAME AS C_5_0, F0.R_COMMENT AS R2
FROM REGION AS F0
WHERE (F0.R_NAME = 'ASIA')) F5 ON ((F4.N_RK = F5.N_RK)))
WHERE ((F1.O_OD >= '1994-01-01') AND (F1.O_OD < '1995-01-01'))
 GROUP BY F4.N_N
 LIMIT 1;
CREATE TABLE rtpcq02 AS
  SELECT  DISTINCT F0.S_ACCOUNT_BAL AS A0, F0.S_NAME AS A1, F0.N_NAME AS A2, F0.P_KEY AS A3, F0.S_COST AS A4, F0.P_MANU AS A5, F0.S_ADDR AS A6, F0.S_PHONE AS A7, F0.S_COMMENT AS A8
FROM ((
SELECT F0.A0 AS S_ACCOUNT_BAL, F0.A1 AS S_NAME, F0.A2 AS N_NAME, F0.A3 AS P_KEY, F0.A4 AS S_COST, F0.A5 AS P_MANU, F0.A6 AS S_ADDR, F0.A7 AS S_PHONE, F0.A8 AS S_COMMENT
FROM (
SELECT  DISTINCT F2.S_ACCOUNT_BAL AS A0, F2.S_NAME AS A1, F0.N_NAME AS A2, F3.P_KEY AS A3, F4.S_COST AS A4, F3.P_MANU AS A5, F2.S_ADDR AS A6, F2.S_PHONE AS A7, F2.S_COMMENT AS A8
FROM (((((
SELECT F0.N_NATIONKEY AS N_KEY, F0.N_NAME AS N_NAME, F0.N_REGIONKEY AS R_KEY, F0.N_COMMENT AS X1
FROM NATION AS F0) F0 JOIN (
SELECT F0.R_REGIONKEY AS R_KEY, F0.R_NAME AS C_1_0, F0.R_COMMENT AS X2
FROM REGION AS F0
WHERE (F0.R_NAME = 'EUROPE')) F1 ON ((F0.R_KEY = F1.R_KEY))) JOIN (
SELECT F0.S_SUPPKEY AS S_KEY, F0.S_NAME AS S_NAME, F0.S_ADDRESS AS S_ADDR, F0.S_NATIONKEY AS N_KEY, F0.S_PHONE AS S_PHONE, F0.S_ACCTBAL AS S_ACCOUNT_BAL, F0.S_COMMENT AS S_COMMENT
FROM SUPPLIER AS F0) F2 ON ((F0.N_KEY = F2.N_KEY))) CROSS JOIN (
SELECT F0.P_PARTKEY AS P_KEY, F0.P_NAME AS X3, F0.P_MFGR AS P_MANU, F0.P_BRAND AS X4, F0.P_TYPE AS P_TYPE, F0.P_SIZE AS C_3_0, F0.P_CONTAINER AS X5, F0.P_RETAILPRICE AS X6, F0.P_COMMENT AS X7
FROM PART AS F0
WHERE (F0.P_SIZE = 15)) F3) JOIN (
SELECT F0.PS_PARTKEY AS P_KEY, F0.PS_SUPPKEY AS S_KEY, F0.PS_AVAILQTY AS X8, F0.PS_SUPPLYCOST AS S_COST, F0.PS_COMMENT AS X9
FROM PARTSUPP AS F0) F4 ON (((F3.P_KEY = F4.P_KEY) AND (F2.S_KEY = F4.S_KEY))))) F0) F0 JOIN (
SELECT F0.A0 AS P_KEY, F0.A1 AS S_COST
FROM (
SELECT /*+ materialize */ F0.A3 AS A0, min(F0.A4) AS A1
FROM (
SELECT  DISTINCT F2.S_ACCOUNT_BAL AS A0, F2.S_NAME AS A1, F0.N_NAME AS A2, F3.P_KEY AS A3, F4.S_COST AS A4, F3.P_MANU AS A5, F2.S_ADDR AS A6, F2.S_PHONE AS A7, F2.S_COMMENT AS A8
FROM (((((
SELECT F0.N_NATIONKEY AS N_KEY, F0.N_NAME AS N_NAME, F0.N_REGIONKEY AS R_KEY, F0.N_COMMENT AS X1
FROM NATION AS F0) F0 JOIN (
SELECT F0.R_REGIONKEY AS R_KEY, F0.R_NAME AS C_1_0, F0.R_COMMENT AS X2
FROM REGION AS F0
WHERE (F0.R_NAME = 'EUROPE')) F1 ON ((F0.R_KEY = F1.R_KEY))) JOIN (
SELECT F0.S_SUPPKEY AS S_KEY, F0.S_NAME AS S_NAME, F0.S_ADDRESS AS S_ADDR, F0.S_NATIONKEY AS N_KEY, F0.S_PHONE AS S_PHONE, F0.S_ACCTBAL AS S_ACCOUNT_BAL, F0.S_COMMENT AS S_COMMENT
FROM SUPPLIER AS F0) F2 ON ((F0.N_KEY = F2.N_KEY))) CROSS JOIN (
SELECT F0.P_PARTKEY AS P_KEY, F0.P_NAME AS X3, F0.P_MFGR AS P_MANU, F0.P_BRAND AS X4, F0.P_TYPE AS P_TYPE, F0.P_SIZE AS C_3_0, F0.P_CONTAINER AS X5, F0.P_RETAILPRICE AS X6, F0.P_COMMENT AS X7
FROM PART AS F0
WHERE (F0.P_SIZE = 15)) F3) JOIN (
SELECT F0.PS_PARTKEY AS P_KEY, F0.PS_SUPPKEY AS S_KEY, F0.PS_AVAILQTY AS X8, F0.PS_SUPPLYCOST AS S_COST, F0.PS_COMMENT AS X9
FROM PARTSUPP AS F0) F4 ON (((F3.P_KEY = F4.P_KEY) AND (F2.S_KEY = F4.S_KEY))))) F0
 GROUP BY F0.A3) F0) F1 ON (((F0.P_KEY = F1.P_KEY) AND (F0.S_COST = F1.S_COST))))
                         LIMIT 1;

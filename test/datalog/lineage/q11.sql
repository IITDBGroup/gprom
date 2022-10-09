CREATE TABLE rtpcq11 AS
  SELECT  DISTINCT F0.C1 AS A0, F0.C2 AS A1
    FROM ((
      SELECT F0.A0 AS C1, F0.A1 AS C2
        FROM (
          SELECT /*+ materialize */ F0.PS_PK AS A0, sum((F0.PS_SC * CAST (F0.PS_AQ AS DOUBLE))) AS A1
            FROM (((
              SELECT F0.PS_PARTKEY AS PS_PK, F0.PS_SUPPKEY AS PS_SK, F0.PS_AVAILQTY AS PS_AQ, F0.PS_SUPPLYCOST AS PS_SC, F0.PS_COMMENT AS PS2
                FROM PARTSUPP AS F0) F0 JOIN (
                  SELECT F0.S_SUPPKEY AS PS_SK, F0.S_NAME AS S1, F0.S_ADDRESS AS S2, F0.S_NATIONKEY AS S_NK, F0.S_PHONE AS S3, F0.S_ACCTBAL AS S4, F0.S_COMMENT AS S5
                    FROM SUPPLIER AS F0) F1 ON ((F0.PS_SK = F1.PS_SK))) JOIN (
                      SELECT F0.N_NATIONKEY AS S_NK, F0.N_NAME AS C_2_0, F0.N_REGIONKEY AS N1, F0.N_COMMENT AS N2
                        FROM NATION AS F0
                       WHERE (F0.N_NAME = 'GERMANY')) F2 ON ((F1.S_NK = F2.S_NK)))
           GROUP BY F0.PS_PK) F0) F0 CROSS JOIN (
             SELECT F0.A0 AS C3
               FROM (
                 SELECT /*+ materialize */ sum((F0.A1 * 0.000100)) AS A0
                   FROM (
                     SELECT /*+ materialize */ F0.PS_PK AS A0, sum((F0.PS_SC * CAST (F0.PS_AQ AS DOUBLE))) AS A1
                       FROM (((
                         SELECT F0.PS_PARTKEY AS PS_PK, F0.PS_SUPPKEY AS PS_SK, F0.PS_AVAILQTY AS PS_AQ, F0.PS_SUPPLYCOST AS PS_SC, F0.PS_COMMENT AS PS2
                           FROM PARTSUPP AS F0) F0 JOIN (
                             SELECT F0.S_SUPPKEY AS PS_SK, F0.S_NAME AS S1, F0.S_ADDRESS AS S2, F0.S_NATIONKEY AS S_NK, F0.S_PHONE AS S3, F0.S_ACCTBAL AS S4, F0.S_COMMENT AS S5
                               FROM SUPPLIER AS F0) F1 ON ((F0.PS_SK = F1.PS_SK))) JOIN (
                                 SELECT F0.N_NATIONKEY AS S_NK, F0.N_NAME AS C_2_0, F0.N_REGIONKEY AS N1, F0.N_COMMENT AS N2
                                   FROM NATION AS F0
                                  WHERE (F0.N_NAME = 'GERMANY')) F2 ON ((F1.S_NK = F2.S_NK)))
                      GROUP BY F0.PS_PK) F0) F0) F1)
   WHERE (F0.C2 > F1.C3)
   LIMIT 1;

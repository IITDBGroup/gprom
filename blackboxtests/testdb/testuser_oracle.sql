CREATE USER testuser IDENTIFIED BY testuser_man;

--------------------------------------------------------------------------------
-- TABLESPACE settings
ALTER USER testuser TEMPORARY TABLESPACE TEMP_SMALL_FGA;

CREATE FLASHBACK ARCHIVE TESTUSER_FBA_ARCHIVE
TABLESPACE TEST_FLASHBACK_TABLESPACE_1 QUOTA 12G
RETENTION 12 MONTH;


--------------------------------------------------------------------------------
-- GRANT
-- access tables, create tables
GRANT CREATE SESSION TO testuser;
GRANT CREATE TABLE TO testuser;
GRANT ALTER ANY TABLE TO testuser;
GRANT CREATE ANY MATERIALIZED VIEW
   , ALTER ANY MATERIALIZED VIEW
   , DROP ANY MATERIALIZED VIEW
   , QUERY REWRITE
   , GLOBAL QUERY REWRITE
   TO testuser;
GRANT CREATE SEQUENCE TO testuser;


-- explain privileges
GRANT ADMINISTER SQL TUNING SET TO testuser;
GRANT ADVISOR TO testuser;
GRANT SELECT ANY DICTIONARY TO testuser;

-- unlimited space
GRANT UNLIMITED TABLESPACE TO testuser;

--FBA
GRANT FLASHBACK ARCHIVE ON TESTUSER_FBA_ARCHIVE TO testuser;

--FGA
GRANT EXECUTE ON dbms_fga TO testuser;
GRANT SELECT ON dba_audit_policies TO testuser;
GRANT SELECT ON dba_fga_audit_trail TO testuser;
GRANT SELECT ON fga_log$ TO testuser;

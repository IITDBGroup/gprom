GRANT CREATE TABLE TO fga_user;
GRANT EXECUTE ON dbms_fga TO fga_user;
GRANT select ON dba_audit_policies TO fga_user;
GRANT select ON dba_fga_audit_trail TO fga_user;
GRANT select ON fga_log$ TO fga_user;
GRANT UNLIMITED TABLESPACE TO fga_user;

begin
   dbms_fga.add_policy (
      object_schema=>'fga_user',
      object_name=>'r',
      policy_name=>'r_TRACK',
      audit_condition=> NULL,
      audit_column=> NULL,
      statement_types => 'SELECT,INSERT,UPDATE,DELETE'
  );
end;

-- 测试用户（与 verify_audb_pg16.sh / GProM 脚本一致）
DO $$
BEGIN
  IF NOT EXISTS (SELECT 1 FROM pg_roles WHERE rolname = 'hana') THEN
    CREATE ROLE hana WITH LOGIN PASSWORD '001011' SUPERUSER;
  END IF;
END $$;

GRANT ALL PRIVILEGES ON DATABASE testdb TO hana;

\c testdb

CREATE EXTENSION IF NOT EXISTS i4r_audb_extension;

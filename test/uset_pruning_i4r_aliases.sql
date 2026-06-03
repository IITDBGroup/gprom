-- 剪枝函数全部由 i4r_audb_extension 的 C 实现提供（见 audb 扩展包 i4r_audb_extension_prune.sql）
-- 不再加载 gprom/test/pruning/*.sql 或 audb PL/pgSQL 包装

\i /home/hana4/yangyun/audb/c_extension/i4r_audb_extension/i4r_audb_extension_prune.sql

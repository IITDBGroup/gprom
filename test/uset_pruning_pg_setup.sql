-- AUDB / GProM USET pruning test: requires i4r_audb_extension
CREATE EXTENSION IF NOT EXISTS i4r_audb_extension;

DROP FUNCTION IF EXISTS int_to_range_set(int);
DROP FUNCTION IF EXISTS int_to_range_set(integer);

CREATE OR REPLACE FUNCTION int_to_range_set(x int)
RETURNS int4range[]
LANGUAGE sql
IMMUTABLE
STRICT
AS $$
  SELECT ARRAY[lift_scalar(x)]::int4range[];
$$;

DROP FUNCTION IF EXISTS normalize_vals(int4range[]);

CREATE OR REPLACE FUNCTION normalize_vals(a int4range[])
RETURNS int4range[]
LANGUAGE sql
IMMUTABLE
AS $$
  SELECT set_normalize(a);
$$;

DROP FUNCTION IF EXISTS prune_and(int4range[], int4range[]);

CREATE OR REPLACE FUNCTION prune_and(set1 int4range[], set2 int4range[])
RETURNS int4range[]
LANGUAGE plpgsql
AS $$
DECLARE
    rv int4range[] := '{}';
    A int4range;
    B int4range;
    pr int4range;
BEGIN
    IF set1 IS NULL OR set2 IS NULL THEN
        RETURN '{}';
    END IF;
    FOREACH A IN ARRAY set1 LOOP
        FOREACH B IN ARRAY set2 LOOP
            pr := A * B;
            IF NOT isempty(pr) THEN
                rv := rv || pr;
            END IF;
        END LOOP;
    END LOOP;
    RETURN rv;
END;
$$;

DROP FUNCTION IF EXISTS prune_eq(int4range[], int4range[], boolean);
DROP FUNCTION IF EXISTS prune_lt(int4range[], int4range[], boolean);
DROP FUNCTION IF EXISTS prune_gt(int4range[], int4range[], boolean);

\i /home/hana4/audb/c_extension/plpgsql_implementation/pruning/prune_equal.sql
\i /home/hana4/audb/c_extension/plpgsql_implementation/pruning/prune_lt.sql
\i /home/hana4/audb/c_extension/plpgsql_implementation/pruning/prune_gt.sql

DROP TABLE IF EXISTS r;
CREATE TABLE r (a int, b int, u_r int);

-- 多样本：便于测 USET / set_* / prune_*（GProM 与手工 SQL）
-- u_r：任意行标识；a,b 为不确定整型列（IS UADB 语义下由元数据使用）
INSERT INTO r (a, b, u_r) VALUES
  (3, 5, 1),      -- 满足 a=3 AND a<b（与原先单条一致）
  (3, 10, 2),     -- 满足
  (3, 100, 3),    -- 满足，b 较大
  (3, 7, 10),     -- 满足
  (3, 3, 4),      -- a=b，不满足 a<b
  (3, 2, 5),      -- a>b，不满足 a<b
  (1, 5, 6),      -- a≠3
  (4, 9, 7),      -- a≠3
  (2, 8, 8),      -- a≠3
  (-1, 10, 9),    -- a≠3，负值
  (0, 3, 11),     -- a≠3
  (3, 4, 12);     -- 满足（紧邻边界 a<b）

-- 区间列：a、b 存 int4range[]（不确定集合）；例：a 为点 3 的 lift [3,4)，b 初始为区间 [3,5)
DROP TABLE IF EXISTS r_interval;
CREATE TABLE r_interval (
  a int4range[] NOT NULL,
  b int4range[] NOT NULL,
  u_r int NOT NULL
);

-- u_r=1：b 为 [3,5)（与 a=[3,4) 在 3 处重叠）→ i4r 的 set_lt(a,b) 为 NULL，整行 WHERE 不成立
-- u_r=2：b 为 [4,6)（与 [3,4) 无重叠且整体在右侧）→ set_lt 为 true，可测完整 prune 输出
INSERT INTO r_interval (a, b, u_r) VALUES
  (
    ARRAY['[3,4)'::int4range],
    ARRAY['[3,5)'::int4range],
    1
  ),
  (
    ARRAY['[3,4)'::int4range],
    ARRAY['[4,6)'::int4range],
    2
  );

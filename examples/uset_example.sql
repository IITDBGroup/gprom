-- USET示例：演示如何将uset(select a+b from testdb)改写为range_set_add(a,b)

-- 首先创建测试表
CREATE TABLE testdb (
    id INTEGER PRIMARY KEY,
    a INTEGER,
    b INTEGER
);

-- 插入测试数据
INSERT INTO testdb VALUES (1, 10, 20);
INSERT INTO testdb VALUES (2, 15, 25);
INSERT INTO testdb VALUES (3, 30, 40);

-- 创建range_set_add函数（PostgreSQL语法）
CREATE OR REPLACE FUNCTION range_set_add(set1 int4range[], set2 int4range[])
RETURNS int4range[] AS $$
DECLARE
    result int4range[] := '{}';
    i int4range;
    j int4range;
BEGIN
    FOR i IN (SELECT unnest(set1)) LOOP
        FOR j IN (SELECT unnest(set2)) LOOP
            result := array_append(result, int4range((lower(i) + lower(j)), (upper(i) + upper(j)) + 1));
        END LOOP;
    END LOOP;
    RETURN result;
END;
$$ LANGUAGE plpgsql;

-- 原始查询：uset(select a+b from testdb)
-- 这个查询应该被GProM中间件重写为：
-- SELECT range_set_add(a, b) FROM testdb;

-- 测试查询
SELECT * FROM testdb;

-- 预期的重写结果应该是：
-- SELECT range_set_add(a, b) as result FROM testdb;

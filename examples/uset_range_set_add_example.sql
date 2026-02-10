-- USET模式与range_set_add功能示例
-- 演示如何将 uset(select a+b from testdb) 改写为 range_set_add(a,b)

-- 1. 首先创建测试数据库和表
CREATE DATABASE IF NOT EXISTS testdb;
USE testdb;

-- 创建测试表
CREATE TABLE IF NOT EXISTS testdb (
    id INT PRIMARY KEY,
    a INT,
    b INT,
    c INT
);

-- 插入测试数据
INSERT INTO testdb VALUES (1, 10, 20, 30);
INSERT INTO testdb VALUES (2, 15, 25, 35);
INSERT INTO testdb VALUES (3, 20, 30, 40);

-- 2. 创建range_set_add函数（如果不存在）
-- 注意：这个函数需要在PostgreSQL中创建
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

-- 3. 设置USET模式
-- 在GProM中，可以通过以下方式设置USET模式：
-- SET PROPERTY USET_MODE = TRUE;

-- 4. 执行USET查询示例

-- 示例1：简单的加法操作
-- 原始查询：uset(select a+b from testdb)
-- 重写后：select range_set_add(a, b) from testdb
SELECT range_set_add(a, b) as result FROM testdb;

-- 示例2：多个加法操作
-- 原始查询：uset(select a+b+c from testdb)
-- 重写后：select range_set_add(range_set_add(a, b), c) from testdb
SELECT range_set_add(range_set_add(a, b), c) as result FROM testdb;

-- 示例3：带条件的查询
-- 原始查询：uset(select a+b from testdb where a > 10)
-- 重写后：select range_set_add(a, b) from testdb where a > 10
SELECT range_set_add(a, b) as result FROM testdb WHERE a > 10;

-- 示例4：聚合查询
-- 原始查询：uset(select sum(a+b) from testdb)
-- 重写后：select sum(range_set_add(a, b)) from testdb
SELECT sum(range_set_add(a, b)) as result FROM testdb;

-- 示例5：复杂表达式
-- 原始查询：uset(select (a+b)*c from testdb)
-- 重写后：select range_set_add(a, b) * c from testdb
SELECT range_set_add(a, b) * c as result FROM testdb;

-- 5. 验证结果
-- 查看原始数据和重写后的结果
SELECT 
    id,
    a,
    b,
    c,
    a + b as original_sum,
    range_set_add(a, b) as uset_sum
FROM testdb;

-- 6. 清理
-- DROP TABLE IF EXISTS testdb;
-- DROP DATABASE IF EXISTS testdb;


-- PostgreSQL 函数：处理跨行约束的CTable条件解析
-- 这个函数会收集所有行中对同一变量的约束，然后合并解析
-- 关键：只收集对特定变量的约束，不收集其他变量的约束

CREATE OR REPLACE FUNCTION parse_ctable_condition_cross_row(
    table_name TEXT,
    var_name TEXT,
    current_row_c_conf TEXT
)
RETURNS TEXT AS $$
DECLARE
    combined_constraints TEXT;
    constraint_parts TEXT[];
    filtered_parts TEXT[];
    part TEXT;
    var_pattern TEXT;
    current_row_constraints TEXT;
    other_constraints TEXT;
BEGIN
    -- 如果当前行的c_conf是'TRUE'，直接返回变量名
    IF UPPER(TRIM(current_row_c_conf)) = 'TRUE' THEN
        RETURN var_name;
    END IF;
    
    -- 构建变量匹配模式：变量名前后应该是非字母数字字符或字符串边界
    -- 例如：匹配'X'但不匹配'XY'或'AX'
    var_pattern := '(^|[^a-zA-Z0-9])' || var_name || '([^a-zA-Z0-9]|$)';
    
    -- 首先从当前行的c_conf中提取只包含该变量的约束部分
    -- 例如：如果var_name='X'，c_conf='X>9000&&Y!=20000'，应该只提取'X>9000'
    constraint_parts := string_to_array(current_row_c_conf, '&&');
    filtered_parts := ARRAY[]::TEXT[];
    
    -- 过滤当前行的约束，只保留包含该变量的部分
    FOREACH part IN ARRAY constraint_parts
    LOOP
        part := trim(part);
        -- 检查是否包含该变量（使用正则表达式精确匹配）
        IF part ~ var_pattern THEN
            filtered_parts := array_append(filtered_parts, part);
        END IF;
    END LOOP;
    
    -- 如果当前行没有该变量的约束，直接返回变量名
    IF array_length(filtered_parts, 1) IS NULL THEN
        RETURN var_name;
    END IF;
    
    -- 保存当前行的过滤后约束
    current_row_constraints := array_to_string(filtered_parts, '&&');
    
    -- 收集其他行中对同一变量的约束
    -- 使用动态SQL查询所有行中c_conf包含该变量的约束
    -- 注意：只收集包含该变量的约束部分，不是整个c_conf
    EXECUTE format('
        WITH constraint_parts AS (
            SELECT unnest(string_to_array(c_conf, ''&&'')) AS part
            FROM %I
            WHERE UPPER(TRIM(c_conf)) != ''TRUE''
        ),
        filtered_parts AS (
            SELECT DISTINCT trim(part) AS part
            FROM constraint_parts
            WHERE trim(part) ~ (''(^|[^a-zA-Z0-9])'' || %L || ''([^a-zA-Z0-9]|$)'')
              AND trim(part) != ''''
        )
        SELECT string_agg(part, ''&&'')
        FROM filtered_parts
    ', table_name, var_name) INTO other_constraints;
    
    -- 合并当前行约束和其他行约束（去重）
    IF other_constraints IS NULL OR other_constraints = '' THEN
        -- 只有当前行的约束
        combined_constraints := current_row_constraints;
    ELSE
        -- 合并当前行和其他行的约束
        constraint_parts := string_to_array(current_row_constraints, '&&') || string_to_array(other_constraints, '&&');
        filtered_parts := ARRAY[]::TEXT[];
        FOREACH part IN ARRAY constraint_parts
        LOOP
            part := trim(part);
            IF part != '' AND NOT (part = ANY(filtered_parts)) THEN
                filtered_parts := array_append(filtered_parts, part);
            END IF;
        END LOOP;
        combined_constraints := array_to_string(filtered_parts, '&&');
    END IF;
    
    -- 调用原有的解析函数
    RETURN parse_ctable_condition_z3_sympy(combined_constraints, var_name);
END;
$$ LANGUAGE plpgsql;

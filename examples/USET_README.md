# USET功能使用说明

## 概述

USET（Uncertainty Set）是GProM中间件的一个扩展功能，用于处理不确定性数据的集合运算。当您输入`uset(select a+b from testdb)`时，GProM会自动将其重写为`range_set_add(a,b)`，其中`range_set_add`是一个自定义的PostgreSQL函数，用于处理范围集合的加法运算。

## 功能特性

1. **自动重写**：将SQL中的算术运算自动重写为相应的集合运算函数
2. **不确定性处理**：支持处理具有不确定性的数据范围
3. **扩展性**：可以轻松添加新的集合运算函数

## 使用方法

### 1. 基本语法

```sql
-- 原始查询
uset(select a+b from testdb)

-- 重写后的查询
SELECT range_set_add(a, b) FROM testdb;
```

### 2. 支持的运算

目前支持以下运算的重写：

- **加法（+）**：`a + b` → `range_set_add(a, b)`
- **减法（-）**：`a - b` → `range_set_sub(a, b)`（待实现）
- **乘法（*）**：`a * b` → `range_set_mul(a, b)`（待实现）

### 3. 数据库函数要求

在使用USET功能之前，需要在数据库中创建相应的集合运算函数。以PostgreSQL为例：

```sql
-- 创建range_set_add函数
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
```

## 实现原理

### 1. 重写流程

1. **解析阶段**：GProM解析SQL查询，识别USET操作
2. **标记阶段**：为相关表达式添加USET_MODE属性
3. **重写阶段**：将算术运算重写为集合运算函数调用
4. **生成阶段**：生成最终的SQL查询

### 2. 核心函数

- `rewriteUset()`：主要的USET重写入口函数
- `rewriteUsetOperation()`：处理USET操作
- `rewriteUsetExpr()`：重写表达式
- `rewriteUsetOp()`：重写操作符
- `rewriteUsetFun()`：重写函数调用

### 3. 配置选项

在GProM配置中可以设置以下选项：

```c
// 启用USET功能
#define ENABLE_USET_REWRITE true

// USET函数名
#define USET_FUNC_NAME "USET"
#define RANGE_SET_ADD_FUNC_NAME "range_set_add"
```

## 示例

### 示例1：基本加法运算

```sql
-- 输入
uset(select a+b from testdb)

-- 输出
SELECT range_set_add(a, b) FROM testdb;
```

### 示例2：复杂表达式

```sql
-- 输入
uset(select (a+b)*c from testdb)

-- 输出（需要实现乘法重写）
SELECT range_set_mul(range_set_add(a, b), c) FROM testdb;
```

## 扩展开发

### 添加新的运算支持

1. 在`uncert_rewriter.c`中添加新的重写逻辑
2. 在数据库中创建对应的集合运算函数
3. 更新常量定义和函数声明

### 示例：添加减法支持

```c
// 在rewriteUsetOp函数中添加
if (strcmp(expr->name, "-") == 0) {
    // 重写逻辑
    Node *ret = (Node *)createFunctionCall("range_set_sub", args);
    return ret;
}
```

## 注意事项

1. **数据库兼容性**：确保目标数据库支持自定义函数
2. **数据类型**：注意输入和输出数据类型的一致性
3. **性能考虑**：集合运算可能比普通算术运算更耗时
4. **错误处理**：确保在重写失败时有适当的回退机制

## 故障排除

### 常见问题

1. **函数未找到**：确保数据库中已创建相应的集合运算函数
2. **类型不匹配**：检查数据类型是否与函数定义一致
3. **重写失败**：查看GProM日志以获取详细错误信息

### 调试方法

1. 启用详细日志：设置日志级别为DEBUG
2. 检查中间结果：查看重写过程中的中间表达式
3. 验证SQL语法：确保生成的SQL语法正确

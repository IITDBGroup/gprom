# GProM – Add Functions

## 1. Range Set Operations

### 1.1 range_set_add

#### Purpose
Adds two range sets by combining each range from the first set with each range from the second.

#### PostgreSQL UDF

```sql
CREATE OR REPLACE FUNCTION range_set_add(I int[][], r int[][])
RETURNS int[][] AS $$
DECLARE
  result int[][];
  i int;
  j int;
BEGIN
  result := '{}';

  FOR i IN array_lower(I, 1)..array_upper(I, 1) LOOP
    FOR j IN array_lower(r, 1)..array_upper(r, 1) LOOP
      result := result || ARRAY[
        ARRAY[I[i][1] + r[j][1], I[i][2] + r[j][2]]
      ];
    END LOOP;
  END LOOP;

  RETURN result;
END;
$$ LANGUAGE plpgsql;
```

#### GProM Usage

```sql
uset(SELECT array1 + array2 FROM test_arrays);
```

#### Example Output

```
{{6,8},{8,10},{8,10},{10,12}}
{{4,6},{6,8},{6,8},{8,10}}
{{3,3},{3,3},{3,3},{3,3}}
```

### 1.2 range_set_multiply

#### Purpose
Multiplies two range sets, computing the minimum and maximum possible products for each range combination.

#### PostgreSQL UDF

```sql
CREATE OR REPLACE FUNCTION range_set_multiply(I int[][], r int[][])
RETURNS int[][] AS $$
DECLARE
  result int[][];
  i int;
  j int;
BEGIN
  result := '{}';

  FOR i IN array_lower(I, 1)..array_upper(I, 1) LOOP
    FOR j IN array_lower(r, 1)..array_upper(r, 1) LOOP
      result := result || ARRAY[
        ARRAY[
          LEAST(
            I[i][1]*r[j][1],
            I[i][1]*r[j][2],
            I[i][2]*r[j][1],
            I[i][2]*r[j][2]
          ),
          GREATEST(
            I[i][1]*r[j][1],
            I[i][1]*r[j][2],
            I[i][2]*r[j][1],
            I[i][2]*r[j][2]
          )
        ]
      ];
    END LOOP;
  END LOOP;

  RETURN result;
END;
$$ LANGUAGE plpgsql;
```

#### GProM Usage

```sql
uset(SELECT array1 * array2 FROM test_arrays);
```

#### Example Output

```
{{5,12},{7,16},{15,24},{21,32}}
{{0,5},{0,7},{8,15},{12,21}}
{{2,2},{2,2},{2,2},{2,2}}
```

### 1.3 range_set_subtract

#### Purpose
Subtracts the second range set from the first.

#### PostgreSQL UDF

```sql
CREATE OR REPLACE FUNCTION range_set_subtract(I int[][], r int[][])
RETURNS int[][] AS $$
DECLARE
  result int[][];
  i int;
  j int;
BEGIN
  result := '{}';

  FOR i IN array_lower(I, 1)..array_upper(I, 1) LOOP
    FOR j IN array_lower(r, 1)..array_upper(r, 1) LOOP
      result := result || ARRAY[
        ARRAY[I[i][1] - r[j][2], I[i][2] - r[j][1]]
      ];
    END LOOP;
  END LOOP;

  RETURN result;
END;
$$ LANGUAGE plpgsql;
```

#### GProM Usage

```sql
uset(SELECT I - r FROM test_range_set_subtract);
```

#### Example Output

```
{{-2,3},{-8,-2},{7,18},{1,13}}
{{-9,2},{-1,10}}
{{98,199}}
```

### 1.4 range_set_logic

#### Purpose
Performs logical operations (AND, OR, NOT) on range sets.

#### PostgreSQL UDF

```sql
CREATE OR REPLACE FUNCTION range_set_logic(
  set1 int4range[],
  set2 int4range[],
  operator text
)
RETURNS int4range[] AS $$
DECLARE
  result int4range[];
BEGIN
  result := ARRAY[]::int4range[];

  FOR i IN 1..array_length(set1, 1) LOOP
    IF operator = 'AND' THEN
      result := array_append(result, set1[i] * set2[i]);
    ELSIF operator = 'OR' THEN
      result := array_append(result, set1[i] + set2[i]);
    ELSIF operator = 'NOT' THEN
      -- Inverse logic applied
    ELSE
      RAISE EXCEPTION 'Invalid operator. Use AND or OR.';
    END IF;
  END LOOP;

  RETURN result;
END;
$$ LANGUAGE plpgsql;
```

#### GProM Usage

```sql
uset(SELECT set1 & set2 FROM test_range_set_logic);  -- AND
uset(SELECT set1 | set2 FROM test_range_set_logic);  -- OR
uset(SELECT !set2 FROM test_range_set_logic);        -- NOT
```

### 1.5 range_set_comparison

#### Purpose
Compares range sets using relational operators such as <, >, and =.

#### Less Than

```sql
CREATE OR REPLACE FUNCTION range_set_smallerthan(
  set1 int4range[],
  set2 int4range[]
)
RETURNS boolean AS $$
DECLARE
  lowest_a int;
  greatest_b int;
BEGIN
  lowest_a := lower(set1[1]);
  greatest_b := upper(set2[1]);
  RETURN lowest_a < greatest_b;
END;
$$ LANGUAGE plpgsql;
```

#### Greater Than

```sql
CREATE OR REPLACE FUNCTION range_set_largerthan(
  set1 int4range[],
  set2 int4range[]
)
RETURNS boolean AS $$
DECLARE
  lowest_a int;
  greatest_b int;
BEGIN
  lowest_a := lower(set1[1]);
  greatest_b := upper(set2[1]);
  RETURN lowest_a > greatest_b;
END;
$$ LANGUAGE plpgsql;
```

#### Equality

```sql
CREATE OR REPLACE FUNCTION range_set_equal(
  set1 int4range[],
  set2 int4range[]
)
RETURNS int4range[] AS $$
DECLARE
  pointer1 int := 1;
  pointer2 int := 1;
  result int4range[] := '{}';
  a int4range[];
  b int4range[];
BEGIN
  a := range_normalize(set1);
  b := range_normalize(set2);
  -- pointer-based comparison logic
  RETURN result;
END;
$$ LANGUAGE plpgsql;
```

#### GProM Usage

```sql
uset(SELECT set1 > set2 FROM test_range_cmp);
uset(SELECT set1 < set2 FROM test_range_cmp);
uset(SELECT set1 = set2 FROM test_range_cmp);
```

## 2. USET Uncertainty Table Types

GProM 的 USET 模式支持三种不确定性表类型。

### 2.1 TIP — Independent Tuple Probability

#### Usage

```sql
uset(SELECT * FROM table_name IS TIP(prob));
```

#### Characteristics

- 每个元组具有独立概率 prob
- prob < 0.5 的元组自动过滤
- 输出包含不确定性标记（U_attr, U_R）

#### Example

```sql
CREATE TABLE items (
  id int,
  name VARCHAR(50),
  value int,
  prob float
);

INSERT INTO items VALUES
(1, 'Item1', 10, 0.8),
(2, 'Item2', 20, 0.3);

uset(SELECT * FROM items IS TIP(prob));
```

#### Output

```
id | name  | value | ub | lb
1  | Item1 | 10    | 1  | 1
```

### 2.2 XTABLE — Intra-Group Probability Distribution

#### Usage

```sql
uset(SELECT * FROM table_name IS XTABLE(group_id, prob));
```

#### Characteristics

- 相同 group_id 的行表示同一实体的不同可能值
- 输出按组聚合（如 {50000,60000}）

#### Example

```sql
uset(SELECT * FROM employee_salaries IS XTABLE(emp_id, prob));
```

#### Output

```
emp_id | name  | salary        | ub | lb
1      | Alice | {50000,60000} | 1  | 1
2      | Bob   | {70000}       | 1  | 1
```

### 2.3 CTABLE — Conditional Constraint Uncertainty

#### Usage

```sql
uset(SELECT * FROM table_name IS CTABLE(c_conf));
```

#### Characteristics

- 使用条件约束（如 X > 10 && X < 20）
- 约束由 Z3 和 SymPy 解析
- 输出为区间字符串或区间并集

#### Example

```sql
CREATE TABLE products (
  product VARCHAR(50),
  price VARCHAR(50),
  c_conf VARCHAR(200)
);

INSERT INTO products VALUES
('Apple', 'X', 'X > 10 && X < 20'),
('Banana', 'Y', 'Y > 20 || Y < 2');

uset(SELECT * FROM products IS CTABLE(c_conf));
```

#### Output

```
product | price            | ub | lb
Apple   | (10,20)          | 1  | 0
Banana  | (-∞,2) U (20,+∞) | 1  | 0
```

# CTable Uncertainty Table Rewriting Functionality - Complete User Guide

## Overview

CTable (Confidence Table) is a mechanism in GProM for handling uncertain data. It allows storing uncertain values (represented by variables such as X, Y, Z) in database tables and specifies constraints for these variables through the `c_conf` column.

### Core Features

1. **Uncertain Value Representation**: Use variables (X, Y, Z, etc.) to represent unknown values
2. **Constraint Parsing**: Parse conditional expressions in the `c_conf` column (e.g., `X>9000 && Y!=20000`)
3. **Interval Calculation**: Convert constraint conditions into numeric intervals (e.g., `(9000,+∞)`)
4. **Cross-Row Dependencies**: Support constraints from different rows affecting each other
5. **Uncertainty Annotation**: Automatically add `lb` (lower bound) and `ub` (upper bound) columns

### Rewriting Process

CTable rewriting consists of two steps:

1. **Step 1: Scan the c_conf column and extract variable constraints**
   - Iterate through all rows' `c_conf` columns
   - Parse conditional expressions and extract variables (X, Y, Z, etc.)
   - Calculate constraint intervals for each variable
   - Variables not mentioned default to `[-∞,+∞]`

2. **Step 2: Rewrite the original table based on constraints**
   - Iterate through all columns (skip `id` and `c_conf` columns)
   - If a column's value is a variable (e.g., X, Y), convert it to an interval or value set based on `c_conf` conditions
   - Add `lb` and `ub` columns to represent row uncertainty
     - `ub` is always 1
     - If a row contains no unknown values (`c_conf='TRUE'`), then `lb=1`; otherwise `lb=0`

---

#### PostgreSQL Functions
- **`parse_ctable_condition_z3_sympy`**: Base parsing function that uses the Z3 solver to parse conditions
- **`parse_ctable_condition_cross_row`**: Cross-row constraint function that collects constraints for the same variable from all rows

---

## Installation and Configuration

### Prerequisites

1. **PostgreSQL 14+**
2. **PL/Python3u extension**
3. **Z3 Python bindings**

### Installation Steps

#### Step 1: Install PostgreSQL Extension

```sql
-- Connect to database
psql -h localhost -U postgres -d your_database

-- Create PL/Python3u extension
CREATE EXTENSION IF NOT EXISTS plpython3u;
```

#### Step 2: Install Z3 Python Bindings

```bash
# Ubuntu/Debian
sudo apt-get install python3-z3

# Or using pip
pip3 install z3-solver
```

#### Step 3: Load PostgreSQL Functions

```bash
cd /home/user/gprom

# Load base parsing function
cat parse_ctable_condition_z3_sympy.sql | sudo -u postgres psql -h localhost -d your_database

# Load cross-row constraint function
cat parse_ctable_condition_cross_row.sql | sudo -u postgres psql -h localhost -d your_database
```

#### Step 4: Compile GProM

```bash
cd /home/user/gprom
./autogen.sh
./configure
make
sudo make install
```

## Syntax

### c_conf Conditional Expression Syntax

#### Supported Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `>` | Greater than | `X>9000` |
| `<` | Less than | `Y<40000` |
| `>=` | Greater than or equal | `X>=1000` |
| `<=` | Less than or equal | `Y<=50000` |
| `!=` or `<>` | Not equal | `Y!=20000` |
| `=` | Equal (for sets) | `X={1000,2000,3000}` |
| `&&` | Logical AND | `X>9000&&Y!=20000` |
| `\|\|` | Logical OR | `X>9000\|\|X<1000` |

#### Special Values

- **`TRUE`** or **`true`**: Indicates that all values in this row are certain, with no uncertainty
- **`false`**: Indicates that constraint conditions are contradictory, the row does not exist (returns NULL)

#### Set Syntax

Use curly braces to represent value sets:

```
X={1000,2000,3000}
```

This means X can be any one of 1000, 2000, or 3000.

#### Interval Notation

Parsing results use interval notation:

- `(a,b)`: Open interval, a < x < b
- `[a,b]`: Closed interval, a ≤ x ≤ b
- `(-∞,a)`: Negative infinity to a
- `(a,+∞)`: a to positive infinity
- `{v1,v2,...}`: Value set
- `(-∞,a) U (a,b)`: Union

---

## How the Parser Works

### How to Parse Unknown Numbers in Uncertain Databases

#### 1. Variable Identification

The system identifies variables in the following way:

```c
// Check if column value is contained in c_conf (as a variable)
Node *colValueInConf = createFunctionCall("strpos", 
    LIST_MAKE(c_confRef, origExpr));
```

For example, if `salary = "X"` and `c_conf = "X>9000"`, then `strpos(c_conf, salary) > 0` is true, indicating that the salary column contains variable X.

#### 2. Constraint Parsing Flow

```
Input: c_conf = "X>9000&&Y!=20000", var_name = "X"
  ↓
Step 1: Split conditions (split by '&&')
  → ["X>9000", "Y!=20000"]
  ↓
Step 2: Variable filtering (keep only constraints containing target variable)
  → ["X>9000"]  (filter out "Y!=20000")
  ↓
Step 3: Parse single constraint
  → op: ">", value: 9000
  ↓
Step 4: Build Z3 constraint and verify satisfiability
  → Z3: X > 9000
  ↓
Step 5: Calculate interval
  → (9000,+∞)
```

#### 3. Cross-Row Constraint Collection

When handling cross-row dependencies:

```sql
-- Internal logic of parse_ctable_condition_cross_row function
WITH constraint_parts AS (
    SELECT unnest(string_to_array(c_conf, '&&')) AS part
    FROM table_name
    WHERE UPPER(TRIM(c_conf)) != 'TRUE'
),
filtered_parts AS (
    SELECT DISTINCT trim(part) AS part
    FROM constraint_parts
    WHERE trim(part) ~ ('(^|[^a-zA-Z0-9])' || var_name || '([^a-zA-Z0-9]|$)')
)
SELECT string_agg(part, '&&')
FROM filtered_parts
```

#### 4. Z3 Solver Usage

The system uses the Z3 solver to verify constraint satisfiability:

```python
from z3 import Real, Solver, unsat

var = Real(var_name)
solver = Solver()
solver.add(constraint)
if solver.check() == unsat:
    return 'false'  # Contradictory condition
```

#### 5. Interval Calculation

Calculate intervals based on constraint conditions:

- **Single constraint**: `X>9000` → `(9000,+∞)`
- **Multiple constraints**: `X>9000 && X<40000` → `(9000,40000)`
- **!= operator**: `Y!=20000 && Y<40000` → `(-∞,20000) U (20000,40000)`
- **Set syntax**: `X={1000,2000,3000}` → `{1000,2000,3000}`

---

## Test Cases and Results

### Test Case 1: Basic Single Variable Constraint

**Input Data**:
```sql
CREATE TABLE test1 (
    id INT PRIMARY KEY,
    name VARCHAR(50),
    salary VARCHAR(50),
    c_conf TEXT
);

INSERT INTO test1 VALUES 
    (1, 'Alice', 'X', 'X>9000'),
    (2, 'Bob', 'Y', 'Y<40000'),
    (3, 'Eve', '20000', 'TRUE');
```

**Query**:
```sql
uset(SELECT * FROM test1 IS CTABLE(c_conf));
```

**Expected Results**:

| name  | salary      | ub | lb |
|-------|-------------|----|----|
| Alice | (9000,+∞)   | 1  | 0  |
| Bob   | (-∞,40000)  | 1  | 0  |
| Eve   | 20000       | 1  | 1  |

**Actual Test Results**: ✅ Passed

---

### Test Case 2: Cross-Row Dependencies

**Input Data**:
```sql
CREATE TABLE test2 (
    id INT PRIMARY KEY,
    name VARCHAR(50),
    salary VARCHAR(50),
    c_conf TEXT
);

INSERT INTO test2 VALUES 
    (1, 'Alice', 'X', 'X>9000&&Y!=20000'),
    (2, 'Bob', 'Y', 'Y<40000'),
    (3, 'Eve', '20000', 'TRUE');
```

**Query**:
```sql
uset(SELECT * FROM test2 IS CTABLE(c_conf));
```

**Expected Results**:

| name  | salary                          | ub | lb |
|-------|---------------------------------|----|----|
| Alice | (9000,+∞)                       | 1  | 0  |
| Bob   | (-∞,20000) U (20000, 40000)     | 1  | 0  |
| Eve   | 20000                            | 1  | 1  |

**Explanation**:
- Alice's salary only contains X constraints: `X>9000` → `(9000,+∞)`
- Bob's salary merges `Y<40000` from Bob's row and `Y!=20000` from Alice's row → `(-∞,20000) U (20000, 40000)`
---

### Test Case 3: Set Syntax

**Input Data**:
```sql
CREATE TABLE test3 (
    id INT PRIMARY KEY,
    name VARCHAR(50),
    salary VARCHAR(50),
    c_conf TEXT
);

INSERT INTO test3 VALUES 
    (1, 'Alice', 'X', 'X>9000&&X={10000,20000}'),
    (2, 'Bob', 'Y', 'Y={5000,15000,25000}');
```

**Query**:
```sql
uset(SELECT * FROM test3 IS CTABLE(c_conf));
```

**Expected Results**:

| name  | salary      | ub | lb |
|-------|-------------|----|----|
| Alice | {10000,20000} | 1  | 0  |
| Bob   | {5000,15000,25000} | 1  | 0  |

**Explanation**:
- Alice's constraints: `X>9000 && X={10000,20000}`, take intersection → `{10000,20000}`


---

### Test Case 4: Contradictory Conditions

**Input Data**:
```sql
CREATE TABLE test4 (
    id INT PRIMARY KEY,
    name VARCHAR(50),
    salary VARCHAR(50),
    c_conf TEXT
);

INSERT INTO test4 VALUES 
    (1, 'Alice', 'X', 'X>9000&&X={1000,2000}');
```

**Query**:
```sql
uset(SELECT * FROM test4 IS CTABLE(c_conf));
```

**Expected Results**:

| name  | salary | ub | lb |
|-------|--------|----|----|
| Alice | NULL    | 1  | 1  |

**Explanation**:
- Constraint `X>9000 && X={1000,2000}` is contradictory (neither 1000 nor 2000 is greater than 9000)
- Z3 solver returns `unsat`, function returns `'false'`
- System converts `'false'` to NULL
- `lb=1` (because contradictory condition indicates certainty of non-existence)


---

### Test Case 5: != Operator

**Input Data**:
```sql
CREATE TABLE test5 (
    id INT PRIMARY KEY,
    name VARCHAR(50),
    salary VARCHAR(50),
    c_conf TEXT
);

INSERT INTO test5 VALUES 
    (1, 'Alice', 'Y', 'Y!=20000'),
    (2, 'Bob', 'Y', 'Y<40000&&Y!=20000');
```

**Query**:
```sql
uset(SELECT * FROM test5 IS CTABLE(c_conf));
```

**Expected Results**:

| name  | salary                    | ub | lb |
|-------|---------------------------|----|----|
| Alice | (-∞,20000) U (20000,+∞)   | 1  | 0  |
| Bob   | (-∞,20000) U (20000,40000)| 1  | 0  |

**Explanation**:
- `Y!=20000` means exclude 20000, forming two intervals: `(-∞,20000)` and `(20000,+∞)`
- After merging with `Y<40000`: `(-∞,20000) U (20000,40000)`


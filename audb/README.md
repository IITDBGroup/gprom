# GPROM- AUDB Extension

## Overview
PostgreSQL extension for range-based uncertainty arithmetic and aggregates. Part of the [GPROM](https://github.com/IITDBGroup/gprom) project for provenance and uncertainty management.

## Prerequisites
- PostgreSQL 12+ (tested on 14, 15, 16)
- Build tools: `gcc`, `make`, `pg_config`
- Docker (optional, containerized development)

## Installation

### Option 1: Docker (preferred)

1. Clone the repository:
```sh
   git clone <repository_url>
   cd audb/c_extension/i4r_audb_extension
```

2. Start the Docker container:
```sh
   docker compose up -d
```

3. Enter the container:
```sh
   docker exec -it audb_test bash
```

4. Build and install inside the container:
```sh
   cd [....]/audb/c_extension/i4r_audb_extension
   make clean
   make && make install
```

5. Connect to the database:
```sh
   psql -U postgres -d audb_test
```
```sql
   CREATE EXTENSION i4r_audb_extension;
```
### Option 2: Local Installation
0. Ensure dependencies are installed and proper version is used.
1. Clone the repository:
```sh
   git clone <repository_url>
   cd audb/c_extension/i4r_audb_extension
```

2. Build and install:
```sh
   make clean
   make
   sudo make install
```

3. Connect to PostgreSQL and create the extension:
```sh
   psql -U postgres -d your_database
```
```sql
   CREATE EXTENSION i4r_audb_extension;
```

## Quick Start
```sql
-- Create extension
CREATE EXTENSION i4r_audb_extension;

-- Range arithmetic
SELECT range_add('[1,5)', '[3,7)');                             -- [4,11) -- Note Upper Bound (UB) is exclusive
SELECT range_multiply(int4range(2,4), '[3,5)'::int4range);      -- [6,13) 
-- Note int4range(a, b) == '[a, b)' == '[a,b)'::int4range syntax

-- Set arithmetic
SELECT set_add(                                                 --  {"[3,6)","[7,10)","[7,10)","[11,14)"}
    ARRAY[int4range(1,3), int4range(5,7)],
    ARRAY[int4range(2,4), int4range(6,8)]
);

-- Uncertainty-aware aggregates
CREATE TABLE sales (
    amount int4range,
    multiplicity int4range  -- [lower, upper) occurrences
);

INSERT INTO sales VALUES 
    ('[100,150)', '[1,3)'),   -- Certain: occurred 1-2 times
    ('[200,250)', '[0,2)'),   -- Uncertain: may not have occurred
    ('[50,75)', '[2,4)');     -- Certain: occurred 2-3 times

-- Aggregates ignore rows with multiplicity starting at 0 due to necessary underapproximation.
SELECT min(combine_range_mult_min(amount, multiplicity)) FROM sales;
-- Returns: [50,75) (ignores uncertain row)

SELECT sum(combine_range_mult_sum(amount, multiplicity)) FROM sales;
-- Returns: [150,224) (ignores uncertain row)
```



## Available Functions 
### Arithmetic: 
* range_add,  range_subtract, range_multiply, range_divide
* set_add, set_subtract, set_multiply, set_divide

### Logical Operators
* range_lt, range_lte, range_gt, range_gte, range_eq
* set_lt, set_lte, set_gt, set_gte, set_eq

### Aggregates
- MIN, MAX, COUNT, AVG, SUM for ranges and sets
- `min(combine_set_mult_min(int4range[], int4range)) → int4range[]`
- `max(combine_set_mult_max(int4range[], int4range)) → int4range[]`
- `sum(combine_range_mult_sum(int4range, int4range)) → int4range`
- `count(combine_range_mult_sum(int4range, int4range)) → int4range`

### Helper Functions
set_normalize, set_sort, set_reduce_size, lift, array_length, range_coverage, set_coverage

### Pruning Operators
* prune_range_lt, prune_range_lte, prune_range_gt, prune_range_gte, prune_range_and, prune_range_or, prune_range_eq
* prune_set_lt, prune_set_lte, prune_set_gt, prune_set_gte, prune_set_and, prune_set_or, prune_set_eq


TODO add function declarations
<!-- 
- `range_add(int4range, int4range) → int4range`
- `range_subtract(int4range, int4range) → int4range`
- `range_multiply(int4range, int4range) → int4range`
- `range_divide(int4range, int4range) → int4range`

### Set Arithmetic
- `set_add(int4range[], int4range[]) → int4range[]`
- `set_subtract(int4range[], int4range[]) → int4range[]`
- `set_multiply(int4range[], int4range[]) → int4range[]`
- `set_divide(int4range[], int4range[]) → int4range[]`

### Aggregates 
- `min(combine_range_mult_min(int4range, int4range)) → int4range`
- `max(combine_range_mult_max(int4range, int4range)) → int4range`
- `sum(combine_range_mult_sum(int4range, int4range)) → int4range`
- `count(combine_range_mult_sum(int4range, int4range)) → int4range`

### Set Aggregates
- `min(combine_set_mult_min(int4range[], int4range)) → int4range[]`
- `max(combine_set_mult_max(int4range[], int4range)) → int4range[]`
- `sum(combine_range_mult_sum(int4range, int4range)) → int4range`
- `count(combine_range_mult_sum(int4range, int4range)) → int4range`

### Logical Operators
- `range_lt(int4range, int4range) → boolean`
- `range_lte(int4range, int4range) → boolean`
- `range_gt(int4range, int4range) → boolean`
- `range_gte(int4range, int4range) → boolean` -->

## Testing

### Run Regression Test Suite
```sh
psql -U postgres -d audb_test
```
```sql
\i i4r_audb_extension_test--1.1.sql
```

### Run Specific Tests
- [Python Experiment Runner Suite](https://github.com/asxvi/audb/tree/main/experimentRunner)


## License
This project is licensed under the Apache 2.0 License. See the `LICENSE` file for more details.

## Related Projects

- [GPROM](https://github.com/IITDBGroup/gprom) - Provenance and uncertainty management system
- [PostgreSQL](https://www.postgresql.org/) - Open source relational database
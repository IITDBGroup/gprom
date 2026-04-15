[![analytics](http://www.google-analytics.com/collect?v=1&t=pageview&_s=1&dl=https%3A%2F%2Fgithub.com%2FIITDBGroup%2Fgprom%2Fmain&_u=MAC~&cid=123456789&tid=UA-92255635-2)]()
[![Build Status](https://travis-ci.org/IITDBGroup/gprom.svg?branch=master)](https://travis-ci.org/IITDBGroup/gprom)

# Overview

**GProM** is a database middleware that adds provenance support to multiple database backends. Provenance is information about how data was produced by database operations. That is, for a row in the database or returned by a query we capture from which rows it was derived and by which operations. The system compiles declarative queries with provenance requests into SQL code and executes this SQL code on a backend database system. GProM supports provenance capture for SQL queries and transactions, and produces provenance graphs explaining existing and missing answers for Datalog queries. Provenance is captured on demand by using a compilation technique called *instrumentation*. Instrumentation rewrites an SQL query (or past transaction) into a query that returns rows paired with their provenance. The output of the instrumentation process is a regular SQL query that can be executed using any standard relational database. The instrumented query generated from a provenance request returns a standard relation that maps rows to their provenance. Provenance for transactions is captured retroactively using a declarative replay technique called *reenactment* that we have developed at IIT. GProM extends multiple frontend languages (e.g., SQL and Datalog) with provenance requests and can produce code for multiple backends (currently Oracle). For information about the research behind GProM have a look at the IIT DBGroup's [webpage](http://www.cs.iit.edu/%7edbgroup/research/gprom.php). 

GProM provides an interactive shell `gprom`, a C library `libgprom`, and a JDBC driver.

# Online Demos

* [Online Demo for PUG Provenance Graph Explorer](http://ec2-18-218-236-30.us-east-2.compute.amazonaws.com:5000/)

# Documentation (Wiki Links)

* [Installation Instructions](https://github.com/IITDBGroup/gprom/wiki/installation)
* [Tutorial](https://github.com/IITDBGroup/gprom/wiki/tutorial)
* [GProM Commandline Shell Manual](https://github.com/IITDBGroup/gprom/blob/master/doc/gprom_man.md)
* Provenance Language Features
  * [SQL](https://github.com/IITDBGroup/gprom/wiki/sql_extensions)
  * [Datalog](https://github.com/IITDBGroup/gprom/wiki/lang_datalog)
* [Docker containers](https://github.com/IITDBGroup/gprom/wiki/docker)
* [Optimization](https://github.com/IITDBGroup/gprom/wiki/research_optimization)
* [Reenactment](https://github.com/IITDBGroup/gprom/wiki/research_reenactment)
* [Provenance Graphs for Datalog](https://github.com/IITDBGroup/gprom/wiki/datalog_prov)
* [CTable Uncertainty Table Rewriting](CTable.md) - Handle uncertain data with variable constraints
* [USET + AUDB Range-Set Pruning](test/USET_PRUNING.md) - Optional Su/Oliver-style range-set pruning for `IS UADB` queries (`-uset_pruning` or `USET WITH PRUNING`)

# Features

+ Flexible on-demand provenance capture and querying for SQL queries using language-level instrumentation, i.e., by running SQL queries.
+ Retroactive provenance capture for transactions using reenactment. Notably, our approach requires no changes to the transactional workload and underlying DBMS
+ Produce provenance graphs for Datalog queries that explain why (provenance) or why-not (missing answers) a tuple is in the result of a Datalog query
+ Heuristic and cost-based optimization for queries instrumented for provenance capture
+ Export of database provenance into the WWW PROV standard format
+ **CTable Uncertainty Table Rewriting**: Handle uncertain data by storing variables (X, Y, Z) and constraint conditions, automatically converting them to intervals or value sets with uncertainty bounds
+ **USET + AUDB range-set pruning**: For tables declared **`IS UADB`**, optionally rewrite **`USET`** queries so **WHERE** uses AUDB **`set_*`** comparisons on **`int4range[]`** semantics and **SELECT** applies **`prune_*`** to shrink range sets per constraints (CLI **`-uset_pruning`** or syntax **`USET WITH PRUNING ( ... )`**)

# Usage #

To use **gprom**, the interactive shell of GProM, you will need to have one of the supported backend databases installed. For casual use cases, you can stick to SQLite. However, to fully exploit the features of GProM, you should use Oracle. We also provide several docker containers with gprom preinstalled (see [here](https://github.com/IITDBGroup/gprom/wiki/docker)) When starting gprom, you have to specify connection parameters to the database. For example, using one of the convenience wrapper scripts that ship with GProM, you can connected to a test SQLite database included in the repository by running the following command in the main source folder after installation:

```
gprom -backend sqlite -db ./examples/test.db
```

will start the shell connecting to an SQLite database `./examples/test.db`. If GProM is able to connect to the database, then this will spawn a shell like this:

```
GProM Commandline Client
Please input a SQL command, '\q' to exit the program, or '\h' for help
======================================================================

Oracle SQL - SQLite:./examples/test.db$
```

In this shell you can enter SQL and utility commands. The shell in turn will show you query results (just like your favorite DB shell). However, the main use of GProM is on-demand capture of provenance for database operations. You can access this functionality through several new SQL language constructs supported by GProM. Importantly, these language constructs behave like queries and, thus, can be used as part of more complex queries. Assume you have a table `R(A,B)`, let us ask our first provenance query.

```
Oracle SQL - SQLite:./examples/test.db$ SELECT * FROM R;
 A | B |
--------
 1 | 1 |
 2 | 3 |

Oracle SQL - SQLite:./examples/test.db$ PROVENANCE OF (SELECT A FROM r);

 A | PROV_R_A | PROV_R_B |
--------------------------
 1 | 1        | 1        |
 2 | 2        | 3        |
```

As you can see, `PROVENANCE OF (q)` returns the same answer as query `q`, but adds additional *provenance* attributes. These attributes store for each result row of the query the input row(s) which where used to compute the output row. For example, the query result `(1)` was derived from row `(1,1)` in table `R`. For now let us close the current session using the `\q` utility command:

```
Oracle SQL - SQLite:./examples/test.db$ \q
```

Provenance for SQL queries is only one of the features supported by GProM. A full list of SQL language extensions supported by GProM can be found in the [wiki](https://github.com/IITDBGroup/gprom/wiki/). See the [man page](https://github.com/IITDBGroup/gprom/blob/master/doc/gprom_man.md) of gprom for further information how to use the CLI of the system.

## CTable Uncertainty Table Rewriting

GProM now supports **CTable** (Confidence Table) functionality for handling uncertain data in database tables. This feature allows you to:

- Store uncertain values using variables (X, Y, Z, etc.)
- Specify constraint conditions in a `c_conf` column
- Automatically convert variables to intervals or value sets based on constraints
- Support cross-row dependencies where constraints from different rows affect each other
- Add uncertainty bounds (`lb` and `ub` columns) to query results

### USET IS Declaration Requirement

**Important:** `uset()` queries require at least one table in the FROM clause to have an **IS** declaration (e.g., `IS CTABLE(c_conf)`) to specify the uncertainty format. Queries without any IS declaration will report an error during semantic analysis.

```sql
-- Correct: table has IS CTABLE(c_conf)
uset(SELECT * FROM employee IS CTABLE(c_conf));

-- Error: no IS declaration
uset(SELECT * FROM employee);  -- Will fail with semantic error
```

### NORMALIZE Option

You can add a top-level `range_normalize()` projection to merge overlapping intervals in each column. Enable it in two ways:

**1. Keyword** – Add `NORMALIZE` after the CTABLE declaration:
```sql
uset(SELECT * FROM employee IS CTABLE(c_conf) NORMALIZE);
```

**2. Command-line option** – Use `-normalize` or `-option normalize=true`:
```bash
gprom -normalize -query "uset(SELECT * FROM employee IS CTABLE(c_conf));"
# or
gprom -option normalize=true -query "uset(SELECT * FROM employee IS CTABLE(c_conf));"
```

**Prerequisite:** Create a PostgreSQL `range_normalize()` function. For `int4range[]` columns, use the version that merges overlapping ranges. For `text`/`varchar` output from CTable, add an overload:

```sql
CREATE OR REPLACE FUNCTION range_normalize(rangeset text) RETURNS text AS $$
BEGIN RETURN rangeset; END;
$$ LANGUAGE plpgsql;
```

### Example Usage

```sql
-- Create a table with uncertain values
CREATE TABLE employee (
    id INT PRIMARY KEY,
    name VARCHAR(50),
    salary VARCHAR(50),
    c_conf TEXT
);

INSERT INTO employee VALUES 
    (1, 'Alice', 'X', 'X>9000'),
    (2, 'Bob', 'Y', 'Y<40000'),
    (3, 'Eve', '20000', 'TRUE');

-- Query with CTable rewriting (IS CTABLE is required)
uset(SELECT name, salary, ub, lb FROM employee IS CTABLE(c_conf));
```

**Result:**
```
name  | salary      | ub | lb
------|-------------|----|----
Alice | (9000,+∞)   | 1  | 0
Bob   | (-∞,40000)  | 1  | 0
Eve   | 20000       | 1  | 1
```

### Supported Constraint Operators

- Comparison: `>`, `<`, `>=`, `<=`, `!=` (or `<>`)
- Set syntax: `X={1000,2000,3000}`
- Logical: `&&` (AND), `||` (OR)
- Special values: `TRUE` (all values certain), `false` (contradictory constraints)

### Prerequisites

- PostgreSQL 14+ with PL/Python3u extension
- Z3 Python bindings (`pip3 install z3-solver` or `apt-get install python3-z3`)
- Load PostgreSQL functions: `parse_ctable_condition_z3_sympy.sql` and `parse_ctable_condition_cross_row.sql`

For detailed documentation and examples, see [CTable.md](CTable.md) in the repository.

## USET + AUDB Range-Set Pruning

This mode targets **uncertain integer** columns modeled as **range sets** (PostgreSQL **`int4range[]`** / AUDB **i4r**). The rewriter:

- Replaces comparisons in **WHERE** with **`set_eq`**, **`set_lt`**, **`set_gt`**, etc., so filtering matches **three-valued** set semantics on intervals.
- Replaces projections with **`prune_*`** calls so each output column’s range set is **tightened** under the active **WHERE** constraints (Su/Oliver-style range-set pruning).

**Enable** (either option is sufficient):

1. **CLI:** **`-uset_pruning`** (option **`OPTION_USET_PRUNING`**).
2. **Syntax:** **`USET WITH PRUNING ( SELECT ... FROM ... IS UADB WHERE ... )`**, which sets property **`PROP_USET_PRUNING`** on that statement.

Scalars are lifted in SQL via **`int_to_range_set`**; if backend metadata already types a column as **`int4range[]`**, the generated SQL avoids double-lifting where the rewriter can tell (**`-Pmetadata postgres`** recommended)

### Example Usage

**Prerequisite:** install the **i4r / AUDB** extension and load **`test/uset_pruning_pg_setup.sql`** (edit embedded **`\i`** paths to your **AUDB** checkout if needed). That script defines **`int_to_range_set`**, **`prune_*`**, and seeds **`r`**. The fragment below matches the **minimal** table shape; you can **`CREATE`** only if you already loaded the extension and helper functions.

```sql
-- Uncertain integer columns a, b; u_r is row metadata (often not SELECT-visible)
CREATE TABLE r (a int, b int, u_r int);

INSERT INTO r (a, b, u_r) VALUES (3, 5, 1);

-- IS UADB is required. Pruning is on because of WITH PRUNING (or use -uset_pruning).
USET WITH PRUNING (SELECT a, b FROM r IS UADB WHERE a = 3 AND b = 5);
```

**Result:** after GProM compiles the request to SQL and you run it on PostgreSQL, each integer is lifted to a **half-open** point range (**`3` → `[3,4)`**, **`5` → `[5,6)`**). One matching row yields one result line; **`int4range[]`** columns may print as an array literal.

```
  a        | b
----------+----------
 {"[3,4)"} | {"[5,6)"}
```


**Limitation:** For **`IS UADB`**, **`u_r`** is often treated as internal metadata; it may **not** be available as an ordinary **SELECT** column. If analysis fails, select only the data attributes you need (**e.g.** **`a`**, **`b`**).

# Installation

The [wiki](https://github.com/IITDBGroup/gprom/wiki/installation) has detailed installation instructions. In a nutshell, GProM can be compiled with support for different database backends and is linked against the C client libraries of these database backends. The installation follows the standard procedure using GNU build tools. Checkout the git repository, install all dependencies and run:

```
./autogen.sh
./configure
make
sudo make install
```

## CTable Feature Setup

To use the CTable uncertainty table rewriting feature, you need to:

1. **Install PostgreSQL extensions:**
   ```sql
   CREATE EXTENSION IF NOT EXISTS plpython3u;
   ```

2. **Install Z3 Python bindings:**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install python3-z3
   # Or using pip
   pip3 install z3-solver
   ```

3. **Load PostgreSQL functions:**
   ```bash
   cat parse_ctable_condition_z3_sympy.sql | sudo -u postgres psql -d your_database
   cat parse_ctable_condition_cross_row.sql | sudo -u postgres psql -d your_database
   ```

4. **Create `range_normalize()` for NORMALIZE support (optional):**
   ```sql
   -- For int4range[] (if your columns use that type)
   -- See your existing range_normalize(int4range[]) implementation.

   -- For text/varchar (CTable output):
   CREATE OR REPLACE FUNCTION range_normalize(rangeset text) RETURNS text AS $$
   BEGIN RETURN rangeset; END;
   $$ LANGUAGE plpgsql;
   ```

See [CTable.md](CTable.md) for detailed setup instructions and usage examples.

# Research and Background

The functionality of GProM is based on a long term research effort by the [IIT DBGroup](http://www.cs.iit.edu/~dbgroup/) studying how to capture provenance on-demand using instrumentation. Links to [publications](http://www.cs.iit.edu/~dbgroup/publications) and more research oriented descriptions of the techniques implemented in GProM can be found at [http://www.cs.iit.edu/~dbgroup/research](http://www.cs.iit.edu/~dbgroup/research).


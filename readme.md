## Overview ##

**GProM** is a database middleware that adds provenance support to multiple database backends. Provenance is this respect refers to information about how data is produced by database operations. That is, for a row in the database or returned by a query we capture from which rows it was derived and by which operations. The system compiles declarative queries with provenance requests into SQL code and executes this SQL code on a backend database system. GProM supports provenance capture for SQL queries and transactions, and produces provenance graphs explaining existing and missing answers for Datalog queries. Provenance is captured on demand by using a compilation technique called instrumentation. Instrumentation rewrites an SQL query (or past transaction) into a query that returns rows paired with their provenance. The output of the instrumentation process is a regular SQL query that can be executed using any standard relational database. The instrumented query generated from a provenance request returns a standard relation that maps rows to their provenance. GProM extends multiple frontend languages (e.g., SQL and Datalog) with provenance requests and can produce code for multiple backends (currently Oracle).

GProM provides an interactive shell `gprom`, a C library `libgprom`, and a JDBC driver (currently not functional).

# Architecture



# Installation

The [wiki](https://github.com/lordpretzel/gprom/wiki/installation) has detailed installation instructions. In a nutshell, GProM can be compiled with support for different database backends and is linked against the C client libraries of these database backends. The installation follows the standard procedure using GNU build tools. Checkout the git repository, install all dependencies and run:

```
./autogen.sh
./configure
make
sudo make install
```


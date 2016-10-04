# Overview

**GProM** is a database middleware that adds provenance support to multiple database backends. Provenance is information about how data was produced by database operations. That is, for a row in the database or returned by a query we capture from which rows it was derived and by which operations. The system compiles declarative queries with provenance requests into SQL code and executes this SQL code on a backend database system. GProM supports provenance capture for SQL queries and transactions, and produces provenance graphs explaining existing and missing answers for Datalog queries. Provenance is captured on demand by using a compilation technique called *instrumentation*. Instrumentation rewrites an SQL query (or past transaction) into a query that returns rows paired with their provenance. The output of the instrumentation process is a regular SQL query that can be executed using any standard relational database. The instrumented query generated from a provenance request returns a standard relation that maps rows to their provenance. Provenance for transactions is captured retroactively using a declarative replay technique called *reenactment* that we have developed at IIT. GProM extends multiple frontend languages (e.g., SQL and Datalog) with provenance requests and can produce code for multiple backends (currently Oracle). For information about the research behind GProM have a look at the IIT DBGroup's [webpage](http://www.cs.iit.edu/%7edbgroup/research/gprom.php). 

GProM provides an interactive shell `gprom`, a C library `libgprom`, and a JDBC driver (currently not fully functional).

# Features

+ Flexible on-demand provenance capture and querying for SQL queries using language-level instrumentation, i.e., by running SQL queries.
+ Retroactive provenance capture for transactions using reenactment. Notably, our approach requires no changes to the transactional workload and underlying DBMS
+ Produce provenance graphs for Datalog queries that explain why (provenance) or why-not (missing answers) a tuple is in the result of a Datalog query
+ Heuristic and cost-based optimization for queries instrumented for provenance capture
+ Export of database provenance into the WWW PROV standard format

# Usage #

To use **gprom**, the interactive shell of GProM, you will need a working Oracle database (currently the only fully supported backend). When starting gprom, you have to specify connection parameters to the database. For example:

```
gprom -host 1.1.1.1 -user usr -passwd mypass -port 1521 -db orcl
```

will start the shell connecting to an Oracle server with IP `1.1.1.1` and SID `orcl` listening at port `1521` using user name `usr` and password `mypass`. If GProM is able to connect to the database, then this will spawn a shell like this:

```
GProM Commandline Client
Please input a SQL command, '\q' to exit the program, or '\h' for help
======================================================================

Oracle SQL - Oracle:usr@1.1.1.1$
```

In this shell you can enter SQL and utility commands and the shell will show you query results (just like your favorite DB shell). However, the main use of GProM is on-demand capture of provenance for database operations. You can access this functionality through several new SQL language constructs supported by GProM. Importantly, these language constructs behave like queries and, thus, can be used as part of more complex queries. Assume you have a table `R(A,B)`, let us ask our first provenance query.

```
Oracle SQL - Oracle:usr@1.1.1.1$ SELECT * FROM r;
A|B|
----------------------------------------
1|1|
2|3|

Oracle SQL - Oracle:usr@1.1.1.1$ PROVENANCE OF (SELECT a FROM r);

A|PROV_R_A|PROV_R_B|
----------------------------------------
1|1|1|
2|2|3|
```

As you can see, `PROVENANCE OF (q)` returns the same answer as query `q`, but adds additional *provenance* attributes. These attributes store for each result row of the query the input row(s) which where used to compute the output row. For example, the query result `(1)` was derived from row `(1,1)` in table `R`. For now let us close the current session using the `\q` utility command:

```
Oracle SQL - Oracle:usr@1.1.1.1$ \q
```

Provenance for SQL queries is only one of the features supported by GProM. A full list of SQL language extensions supported by GProM can be found in the [wiki](https://github.com/lordpretzel/gprom/wiki/). See the [man page](https://github.com/lordpretzel/gprom/blob/master/doc/gprom_man.md) of gprom for further information how to use the CLI of the system. 

# Installation

The [wiki](https://github.com/lordpretzel/gprom/wiki/installation) has detailed installation instructions. In a nutshell, GProM can be compiled with support for different database backends and is linked against the C client libraries of these database backends. The installation follows the standard procedure using GNU build tools. Checkout the git repository, install all dependencies and run:

```
./autogen.sh
./configure
make
sudo make install
```

# Research and Background

The functionality of GProM is based on a long term research effort by the [IIT DBGroup](http://www.cs.iit.edu/~dbgroup/) studying how to capture provenance on-demand using instrumentation. Links to [publications](http://www.cs.iit.edu/~dbgroup/publications) and more research oriented descriptions of the techniques implemented in GProM can be found at [http://www.cs.iit.edu/~dbgroup/research](http://www.cs.iit.edu/~dbgroup/research).





[Table of Contents](#toc)

[Name](#toc0)
-------------

**gprom** - a command line interface for the GProM provenance database middleware

[Synopsis](#toc1)
-----------------

**gprom** *\[connection\_options\]*

**gprom -query** *query* *\[connection\_options\]*

**gprom -queryfile** *file* *\[connection\_options\]*

**gprom -help**

**gprom -languagehelp *language***

[Description](#toc2)
--------------------

**GProM** is a database middleware that adds provenance support to multiple database backends. Provenance is information about how data was produced by database operations. That is, for a row in the database or returned by a query we capture from which rows it was derived and by which operations. The system compiles declarative queries with provenance requests into SQL code and executes this SQL code on a backend database system. GProM supports provenance capture for SQL queries and transactions, and produces provenance graphs explaining existing and missing answers for Datalog queries. Provenance is captured on demand by using a compilation technique called instrumentation. Instrumentation rewrites an SQL query (or past transaction) into a query that returns rows paired with their provenance. The output of the instrumentation process is a regular SQL query that can be executed using any standard relational database. The instrumented query generated from a provenance request returns a standard relation that maps rows to their provenance. GProM extends multiple frontend languages (e.g., SQL and Datalog) with provenance requests and can produce code for multiple backends (currently Oracle).

**gprom** is a command line interface for GProM. gprom can be called in several ways as shown above in the synopsis. The first form starts an interactive shell where the user runs SQL and utility commands. The second form evaluates a single query given as parameter *query*. The third form runs all SQL commands from file *file*. The last form describes the provenance extensions that GProM supports for a particular frontend language, e.g., *oracle* for Oracle SQL. See discussion on parser plugins below for a comprehensive list of supported frontend languages. See the **EXAMPLES** section for some typical usage examples.

[Options](#toc3)
----------------

Boolean options take a single argument, e.g., **-time\_queries** *TRUE* activates timing of queries and **-time\_queries** *FALSE* deactivates this options.

### [Help](#toc4)

Options to get help on GProM usage.

**-help**   
show help message and quit

**-languagehelp** ***language***   
show help for the selected language (see section on parser plugins below for a list of supported languages)

### [Input](#toc5)

These options determine whether GProM operates as an interactive shell or just processes one query. If none of the options below is set, then an interactive shell is created.

**-query** ***query***   
process *query*

**-queryFile** ***file***   
read query to be processed from *file*

### [Output and Timing Queries](#toc6)

These options control what information GProM is printing for an executed query.

**-time\_queries**   
measure runtimes of executing rewritten queries. This option is ignored for all executor plugins except for *run*.

**-time\_query\_format**   
if *-time\_queries* is activated, then this format is used for printing query runtimes. The format is printf compatible and should contain exactly on *%f* element (additional formating such as *%12f* is ok).

**-repeat\_query\_count**   
execute each query this many times. This is mainly useful for timing queries.

**-show\_result**   
show query result (default if *true*). This option is ignored for all executor plugins except for *run*. The main use case for deactivating this is to measure query runtimes without spending time on serializing query results.

### [Logging and Debug](#toc7)

Set logging and debugging options.

**-log**   
activate logging

**-loglevel** ***level***   
set minimum level of log messages to be shown. Valid settings for *level* are **0 = NONE**, **1 = FATAL**, **2 = ERROR**, **3 = INFO**, **4 = DEBUG**, **5 = TRACE**.

**-timing**   
measure how much time is spend in each component of the system and output this information at exit

**-memdebug**   
measure how much memory is allocated and output memory statistics at exit

**-show\_graphviz**   
Print graphviz scripts for relational algebra expressions created by GProM internally to *stdout*

**-graphviz\_details**   
Show operator parameters in graphviz scripts. Is ignored unless **-show\_graphviz** is activated

**-aggressive\_model\_checking**   
Aggressively check validity of generated relational algebra graphs after each processing step

**-C***check\_option*   
Activate *check\_option* where *check\_option* is one of *schema\_consistency*, *unique\_attr\_names*, *parent\_child\_links*, or *attr\_reference\_consistency*.

### [Plugins](#toc8)

Configure plugins. Plugins determine mosts of GProM’s behavior including selecting frontend languages and database backends.

**-P*plugin\_type*** ***plugin\_name***   
Select *plugin\_name* as the active plugin for *plugin\_type*. Most components in GProM are pluggable. See the section on plugins below.

**-backend** ***backend\_name***   
Select *backend* as the active database backend. This overwrites most other plugin options. Individual plugin options take precedence over this options enabling plugins to be customized.

**-frontend** ***frontend\_name***   
Select the *frontend\_name* as the active frontend language. This is the language in which the user communicates with GProM. For instance, *oracle* corresponds to Oracle’s SQL dialect extended with provenance features. This overwrites several other plugin options. Individual plugin options take precedence over this options enabling plugins to be customized. *frontend* takes precedence over *backend*.

### [Connection Options](#toc9)

Configure the connection to the backend database system.

**-host** ***host***   
Host IP address for backend DB connection. Default value: *ligeti.cs.iit.edu*.

**-db** ***orcl***   
Database name for the backend DB connection. For Oracle connections this determines *SID* or *SERVICE\_NAME*. Default value: *orcl*

**-user** ***user***   
User for the backend DB connection. Default value: *fga\_user*

**-passwd** ***password***   
Use password *password* for the backend DB connection.

**-port** ***port***   
The TPC/IP network port to use for the backend DB connection.

### [Provenance Features](#toc10)

GProM main purpose is to provide provenance support for relational databases by instrumenting operations for provenance capture. These options control certain aspects of provenance instrumentation.

**-treeify-algebra-graphs**   
Transform relational algebra graphs into trees before performing provenance instrumentation. Currently, this option activated by default since GProM’s provenance instrumentation cannot deal with graphs yet.

**-prov\_reenact\_update\_with\_case**   
When reenacting update operations use **CASE** instead of **UNION** to simulate an update

**-prov\_instrument\_agg\_window**   
When instrumenting an aggregation operator for provenance capture use window function to perform the instrumentation instead of using joins (the default)

**-prov\_reenact\_only\_updated\_rows\_use\_conditions**   
If the user requests the provenance of a transaction restricted to rows that where updated by the transaction, then use the conditions of update statements for this transaction to filter out rows from the input of reenactment that where not updated by the transaction

**-prov\_reenact\_only\_updated\_rows\_use\_hist\_join**   
If the user requests the provenance of a transaction restricted to rows that where updated by the transaction, then use a temporal join between the table at transaction commit and the table at transaction begin to filter out rows from the input of reenactment that where not updated by the transaction

**-prov\_use\_composable**   
Use composable version of provenance instrumentation that adds additional columns which enumerate duplicates of result rows introduced by provenance instrumentation

### [Temporal Features](#toc11)

GProM also implements a form of temporal queries called sequenced semantics over interval-timestamped data. These options control the application of normalization operations applied by the rewrites for sequenced semantics.

**-temporal\_use\_coalesce**   
If activated (the default), then GProM applies coalescing to the result of a temporal query to produce a unique interval-timestamped representation of a temporal query result.

**-temporal\_use\_normalization**   
If activated (the default), then GProM normalizes the input to temporal aggregation and set difference operators. The correctness of results depends on normalization. Thus, this option should only be deactivated for testing.

**-temporal\_use\_normalization\_window**   
GProM supports two implementations of temporal normalization one based on joins and one which uses analytical functions (window functions). If this option is activated, then GProM applies the window based implementation.

### [Optimization](#toc12)

GProM features a heuristic and cost-based optimizer for relational algebra and provenance instrumentation. These options control the optimizer. Additional options are described in the **OPTIMIZATION** section below.

**-heuristic\_opt**   
Apply heuristic application of relational algebra optimization rules. Default value: *FALSE*.

**-cbo**   
Apply cost-based optimization. Default value: *FALSE*.

**-O*optimization\_option***   
Activate optimization option. Most options correspond to equivalence preserving relational algebra transformations. -O*optimization\_option* activates the option. To deactivate an option use -O*optimization\_option* *FALSE*. For example, **-Omerge\_ops** activates a rule that merges adjacent selections and projections in a query. See section **OPTIMIZATION** below for a full list of supported *optimization\_option* values.

[Plugins](#toc13)
-----------------

Most components in GProM are pluggable and can be replaced. The following components are realized as plugins:

### [parser](#toc14)

The parser plugin determines what input language is used.

**orcle** - Oracle SQL dialect   

**dl** - Datalog   

### [executor](#toc15)

GProM translates statements in an input language with provenance features into a language understood by a database backend (this process is called instrumentation). The executor plugin determines what is done with the instrumented query produced by GProM.

**sql** - Print the generated query to *stdout*   

**run** - Run the generated query and show its result   

**dl** - Output a datalog program (only works if *dl* analyzer, translator, and parser plugins have been chosen   

### [analyzer](#toc16)

This plugin checks the output of the parser for semantic correctness.

**oracle** - Assumes the input is an SQL query written in Oracle’s SQL dialect   

**dl** - Analyses Datalog inputs   

### [translator](#toc17)

This plugin translates the input language into **relational algebra** which is used as an internal code representation by GProM.

**oracle** - Translates Oracle SQL into relational algebra   

**dl** -` ` ` ` Translates Datalog into relational algebra   

**dummy** - Do not translate the input (this can be used to produce an output language other than SQL to circumvent the limitations of GProM’s relational algebra model, e.g., we currently do not support recursion)   

### [metadatalookup](#toc18)

The metadata lookup plugin handles communication with the backend database. This involves 1) running queries over the catalog of the backend to do, e.g., semantic analysis and 2) executing queries instrumented for provenance capture to compute the results of provenance requests submitted by the user. To be able to do this, the plugin manages a connection to the backend database using the C library of the backend DBMS. The type of metadata lookup plugin determines how connection parameters will be interpreted.

**oracle** - This plugin manages communication with an Oracle database backend. We use Oracle’s *OCI* interface wrapped by the open source library *OCILIB*.   

**postgres** - This plugin manages communication with a PostgreSQL database backend. We use PostgreSQL’s *libpq* library.   

**sqlite** - This plugin manages communication with a SQLite database backend. We use the *sqlite3-dev* library.   

**monetdb** - This plugin manages communication with a MonetDB database backend. We use the MonetDB’s *mapi* library.   

### [sqlcodegen](#toc19)

This plugin translates GProM’s internal relational algebra model of queries into queries written in a backend’s SQL dialect.

**dl** - Output a Datalog program   

**oracle** - Output SQL code written in Oracle’s SQL dialect   

**postgres** - Output SQL code written in PostgreSQL’s SQL dialect   

**sqlite** - Output SQL code written in SQLite’s SQL dialect   

### [cbo](#toc20)

Select search strategy of the cost-based optimizer

**exhaustive** - enumerate all options   

**balance** - stop optimization after optimization time exceeds estimated runtime of best plan found so far   

**sim\_ann** - use simmulated annealing meta-heuristic   

[Optimization](#toc21)
----------------------

As mentioned above GProM features a cost-based and heuristic optimization for relational algebra expressions. Heuristic optimization rules are mostly relational algebra equivalences. Cost-base optimization chooses between alternative options for instrumenting a query for provenance capture and controls the application of some of the algebraic equivalence rules we support.

### [Relational algebra transformations](#toc22)

GProM currently implement the following transformation rules that are activated with **-O*rule***:

**merge\_ops** - merge adjacent projection and selection operators. Selections will always be merged. However, merging projections can lead to an explosion of projection expression size. We actively check for such cases and avoid merging if this would increase the expression size dramatically. For example, consider a projection **A + A AS B** followed by a projection **B + B AS C**. Merging these two projections would result in the projection expression **A + A + A + A AS C** which has double the number of **A** references as the original projection. This optimization is important when computing transaction provenance. For a thorough explanation see the publications referenced on the GProM webpage.   

**factor\_attrs** - try to factor attributes in projection expressions to reduce the number of references to attributes. We currently support addition and multiplication expressions in **CASE** constructs. For example, **CASE WHEN *cond **THEN A + 2 ELSE A END AS A***** can be refactored into **A + CASE WHEN *cond **THEN 2 ELSE 0 END AS A***** to reduce the number of references to attribute **A** from 2 to 1.   

**materialize\_unsafe\_proj** - Force the backend database to materialize projections that could lead to uncontrolled expression growth if they would be merged with adjacent projections (as explained above for **merge\_ops**).   

**remove\_redundant\_projections** - Removes projections that are unnecessary from a query, e.g., a projection on **A, B** over a table **R(A,B)** is redundant and should be removed to simplify the query.   

**remove\_redundant\_duplicate\_removals** - Removes duplicate removal operators if the application of duplicate removal has no effect on the query result. We check for two cases here: 1) if the input relation has at least one candidate key, then there are no duplicates and the operator has no effect and 2) if the result of the duplicate removal is later subjected to duplicate removal by a downstream operator and none of the operators on the path to this downstream operator are sensitive to the number of duplicates then the operator can be safely removed.   

**remove\_redundant\_window\_operators** - Remove window operators (corresponding to SQL **OVER** clause expressions) which produce an output that is not used by any downstream operators.   

**remove\_unnecessary\_columns** - Based on an analysis of which columns of the relation produced by an operator are used by downstream operators, we add additional projections to remove unused columns.   

**pullup\_duplicate\_removals** - This optimization tries to pull up duplicate removal operators.   

**pullup\_prov\_projections** - The provenance instrumentation used by GProM duplicates attributes of input tables using projection and propagates them to produce results annotated with provenance. This optimization tries to pull up such projections to delay the increase of schema sized caused by duplicating attributes.   

**selection\_move\_around** - This optimization applies standard selection move-around techniques.   

### [Cost-based optimization options](#toc23)

The following options control the behavior of GProM’s cost-based optimizer:

**-cbo\_choice\_point\_remove\_duplicate\_removal** - makes a cost-based choice of whether to remove a duplicate removal operator when possible   

**-cbo\_max\_considered\_plans** *num\_plans* - stop cost-based optimization after *num\_plans* have been considered.   

**-cbo\_sim\_ann\_const** *c* - Set the constant *c* used by the simulated annealing search strategy to calculate ap, e.g., c = 10, 20, 50 or 100   

**-cbo\_sim\_ann\_cooldown\_rate** -   
Set the cooling down rate used by simulated annealing. Value has to be between 0.1 and 0.9.

**-cbo\_num\_heuristic\_opt\_iterations** *num\_iter* - Apply each heuristic optimization rule *num\_iter* times.   

[Examples](#toc24)
------------------

**Example 1.** Connect to an Oracle database (*oracle*) at IP *1.1.1.1* with SID *orcl* using user *usr* and password *mypass* at port *1521* and start an interactive session:

    gprom -backend oracle -host 1.1.1.1 -user usr -passwd mypass -port 1521 -db orcl

**Example 2.** Same as above, but output instrumented SQL queries to *stdout* instead of executing them:

    gprom -backend oracle -host 1.1.1.1 -user usr -passwd mypass -port 1521 -db orcl
    -Pexecutor sql

**Example 3.** Using the same database as in examples 1 and 2, return an SQL Query that captures provenance for the query **SELECT a FROM r**:

    gprom -backend oracle -host 1.1.1.1 -user usr -passwd mypass -port 1521 -db orcl
    -Pexecutor sql \
          -query "PROVENANCE OF (SELECT a FROM r);"

**Example 4.** Connect to SQLite database test.db and return provenance for the query **SELECT a FROM r**:

    gprom -backend sqlite -db test.db \
          -query "PROVENANCE OF (SELECT a FROM r);"

**Example 5.** Connect to SQLite database test.db and return results of the Datalog query **Q(X) :- R(X,Y).**:

    gprom -backend sqlite -frontend dl -db test.db \
          -query "Q(X) :- R(X,Y)."

[Authors](#toc25)
-----------------

**Bahareh Arab** (*barab@hawk.iit.edu*)   

**Su Feng** (*sfeng@hawk.iit.edu*)   

**Boris Glavic** (*bglavic@iit.edu*)   

**Seokki Lee** (*slee195@hawk.iit.edu*)   

**Xing Niu** (*xniu7@hawk.iit.edu*)   

[Bugs](#toc26)
--------------

To see a list of current bugs or to report a new bug: [*https://github.com/IITDBGroup/gprom/issues*](https://github.com/IITDBGroup/gprom/issues)   

[See Also](#toc27)
------------------

To learn more about the research behind GProM see [*http://www.cs.iit.edu/%7edbgroup/research/gprom.php*](http://www.cs.iit.edu/%7edbgroup/research/gprom.php)   

------------------------------------------------------------------------

[**Table of Contents**]()

[Name](#sect0)

[Synopsis](#sect1)

[Description](#sect2)

[Options](#sect3)

-   [Help](#sect4)
-   [Input](#sect5)
-   [Output and Timing Queries](#sect6)
-   [Logging and Debug](#sect7)
-   [Plugins](#sect8)
-   [Connection Options](#sect9)
-   [Provenance Features](#sect10)
-   [Temporal Features](#sect11)
-   [Optimization](#sect12)

[Plugins](#sect13)

-   [parser](#sect14)
-   [executor](#sect15)
-   [analyzer](#sect16)
-   [translator](#sect17)
-   [metadatalookup](#sect18)
-   [sqlcodegen](#sect19)
-   [cbo](#sect20)

[Optimization](#sect21)

-   [Relational algebra transformations](#sect22)
-   [Cost-based optimization options](#sect23)

[Examples](#sect24)

[Authors](#sect25)

[Bugs](#sect26)

[See Also](#sect27)

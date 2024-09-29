# NAME

**gprom** - a command line interface for the GProM provenance database
middleware

# SYNOPSIS

**gprom** *\[connection_options\]*

**gprom -query** *query* *\[connection_options\]*

**gprom -queryfile** *file* *\[connection_options\]*

**gprom -help**

**gprom -languagehelp *language***

# DESCRIPTION

**GProM** is a database middleware that adds provenance support to
multiple database backends. Provenance is information about how data was
produced by database operations. That is, for a row in the database or
returned by a query we capture from which rows it was derived and by
which operations. The system compiles declarative queries with
provenance requests into SQL code and executes this SQL code on a
backend database system. GProM supports provenance capture for SQL
queries and transactions, and produces provenance graphs explaining
existing and missing answers for Datalog queries. Provenance is captured
on demand by using a compilation technique called instrumentation.
Instrumentation rewrites an SQL query (or past transaction) into a query
that returns rows paired with their provenance. The output of the
instrumentation process is a regular SQL query that can be executed
using any standard relational database. The instrumented query generated
from a provenance request returns a standard relation that maps rows to
their provenance. GProM extends multiple frontend languages (e.g., SQL
and Datalog) with provenance requests and can produce code for multiple
backends (currently Oracle).

**gprom** is a command line interface for GProM. gprom can be called in
several ways as shown above in the synopsis. The first form starts an
interactive shell where the user enters queries (in SQL or Datalog) and
utility commands. The second form evaluates a single query given as
parameter *query*. The third form runs all SQL commands from file
*file*. The last form describes the provenance extensions that GProM
supports for a particular frontend language, e.g., *oracle* for Oracle
SQL. See discussion on parser plugins below for a comprehensive list of
supported frontend languages. See the **EXAMPLES** section for some
typical usage examples.

# OPTIONS

Boolean options take a single argument, e.g., **-time_queries** *TRUE*
activates timing of queries and **-time_queries** *FALSE* deactivates
this option.

## HELP

Options to get help on GProM usage.

**-help**  
show help message and quit

**-languagehelp** ***language***  
show help for the selected language (see section on parser plugins below
for a list of supported languages)

## INPUT

These options determine whether GProM operates as an interactive shell
or just processes one query. If none of the options below is set, then
an interactive shell is created.

**-query** ***query***  
process *query*

**-queryFile** ***file***  
read query to be processed from *file*

## OUTPUT AND TIMING QUERIES

These options control what information GProM is printing for an executed
query.

**-time_queries**  
measure runtimes of executing rewritten queries. This option is ignored
for all executor plugins except for *run* (see the section on *plugins*
below).

**-time_query_format**  
if *-time_queries* is activated, then this format is used for printing
query runtimes. The format is printf compatible and should contain
exactly on *%f* element (additional formating such as *%12f* is ok). Any
occurrence of **"\n"** in the format string is replaced with a newline.
For instance, "query took\n%f\nmsecs\n" will print the time of a query
as three lines: "query took", a second line with just the time itself,
and a third line "msecs".

**-repeat_query_count**  
execute each query this many times. This is mainly useful for timing
queries.

**-show_result**  
show query result (default if *true*). This option is ignored for all
executor plugins except for *run*. The main use case for deactivating
this is to measure query runtimes without spending time on serializing
query results.

## LOGGING AND DEBUG

Set logging and debugging options.

**-log**  
activate logging

**-loglevel** ***level***  
set minimum level of log messages to be shown. Valid settings for
*level* are **0 = NONE**, **1 = FATAL**, **2 = ERROR**, **3 = INFO**,
**4 = DEBUG**, **5 = TRACE**.

**-timing**  
measure how much time is spend in each component of the system and
output this information at exit

**-memdebug**  
measure how much memory is allocated and output memory statistics at
exit

**-show_graphviz**  
Print graphviz scripts for relational algebra expressions created by
GProM internally to *stdout*

**-graphviz_details**  
Show operator parameters in graphviz scripts. Is ignored unless
**-show_graphviz** is activated

**-aggressive_model_checking**  
Aggressively check validity of generated relational algebra graphs after
each processing step

**-C***check_option*  
Activate *check_option* where *check_option* is one of
*schema_consistency*, *unique_attr_names*, *parent_child_links*, or
*attr_reference_consistency*.

## PLUGINS

Configure plugins. Plugins determine mosts of GProM's behavior including
selecting frontend languages and database backends.

**-P*plugin_type*** ***plugin_name***  
Select *plugin_name* as the active plugin for *plugin_type*. Most
components in GProM are pluggable. See the section on plugins below.

**-backend** ***backend_name***  
Select *backend* as the active database backend. This overwrites most
other plugin options. Individual plugin options take precedence over
this option enabling plugins to be customized even when a backend is
selected.

**-frontend** ***frontend_name***  
Select the *frontend_name* as the active frontend language. This is the
language in which the user communicates with GProM. For instance,
*oracle* corresponds to a subset of Oracle's SQL dialect extended with
provenance features. This overwrites several other plugin options.
Individual plugin options take precedence over this options enabling
plugins to be customized. *frontend* takes precedence over *backend*.
Use **-languagehelp** to get a brief description of the language
features.

## CONNECTION OPTIONS

Configure the connection to the backend database system.

**-host** ***host***  
Host IP address for backend DB connection, e.g., *127.0.0.1* or
*myhost.com*.

**-db** ***orcl***  
Database name for the backend DB connection. The meaning of this option
is backend specific. For Oracle connections this determines *SID* or
*SERVICE_NAME*. For PostgreSQL databases it is the database name. For
SQLite databases this is a path to the database file. Default value:
*orcl*

**-user** ***user***  
User for the backend DB connection. Default value: *fga_user*

**-passwd** ***password***  
Use password *password* for the backend DB connection.

**-port** ***port***  
The TPC/IP network port to use for the backend DB connection.

## PROVENANCE FEATURES

GProM main purpose is to provide provenance support for relational
databases by instrumenting operations for provenance capture. These
options control certain aspects of provenance instrumentation.

**-treeify-algebra-graphs**  
Transform relational algebra graphs into trees before performing
provenance instrumentation. Currently, this option activated by default
since GProM's provenance instrumentation cannot deal with graphs yet.

**-prov_reenact_update_with_case**  
When reenacting update operations use **CASE** instead of **UNION** to
simulate an update

**-prov_instrument_agg_window**  
When instrumenting an aggregation operator for provenance capture use
window function to perform the instrumentation instead of using joins
(the default)

**-prov_reenact_only_updated_rows_use_conditions**  
If the user requests the provenance of a transaction restricted to rows
that where updated by the transaction, then use the conditions of update
statements for this transaction to filter out rows from the input of
reenactment that where not updated by the transaction

**-prov_reenact_only_updated_rows_use_hist_join**  
If the user requests the provenance of a transaction restricted to rows
that where updated by the transaction, then use a temporal join between
the table at transaction commit and the table at transaction begin to
filter out rows from the input of reenactment that where not updated by
the transaction

**-prov_use_composable**  
Use composable version of provenance instrumentation that adds
additional columns which enumerate duplicates of result rows introduced
by provenance instrumentation

## TEMPORAL FEATURES

GProM also implements a form of temporal queries called sequenced
semantics over interval-timestamped data. These options control the
application of normalization operations applied by the rewrites for
sequenced semantics.

**-temporal_use_coalesce**  
If activated (the default), then GProM applies coalescing to the result
of a temporal query to produce a unique interval-timestamped
representation of a temporal query result.

**-temporal_use_normalization**  
If activated (the default), then GProM normalizes the input to temporal
aggregation and set difference operators. The correctness of results
depends on normalization. Thus, this option should only be deactivated
for testing.

**-temporal_use_normalization_window**  
GProM supports two implementations of temporal normalization one based
on joins and one which uses analytical functions (window functions). If
this option is activated, then GProM applies the window based
implementation.

## OPTIMIZATION

GProM features a heuristic and cost-based optimizer for relational
algebra and provenance instrumentation. These options control the
optimizer. Additional options are described in the **OPTIMIZATION**
section below.

**-heuristic_opt**  
Apply heuristic application of relational algebra optimization rules.
Default value: *FALSE*.

**-cbo**  
Apply cost-based optimization. Default value: *FALSE*.

**-O*optimization_option***  
Activate optimization option. Most options correspond to equivalence
preserving relational algebra transformations. -O*optimization_option*
activates the option. To deactivate an option use
-O*optimization_option* *FALSE*. For example, **-Omerge_ops** activates
a rule that merges adjacent selections and projections in a query. See
section **OPTIMIZATION** below for a full list of supported
*optimization_option* values.

# PLUGINS

Most components in GProM are pluggable and can be replaced. The
components shown below are realized as plugins. Currently, the sequence
of components that process a query are hardcoded. An incoming query is
first parsed by the **parser** plugin, then GProM applies semantic
analysis (**analyzer** plugin), uses the **translator** plugin to
translate the query into an intermediate representation (in almost all
cases that is relational algebra). Afterwards, any provenance or other
extended features are processed by rewriting the query using
instrumentation. The result of this step is then translate into
backend-specific code (e.g., SQL) using the **sqlcodegen** plugin. The
**metadatalookup** plugin provides backend-specific access to schema
information.

## parser

The parser plugin determines what input language is used.

> **orcle** - Oracle SQL dialect

> **dl** - Datalog

## executor

GProM translates statements in an input language with provenance
features into a language understood by a database backend (this process
is called instrumentation). The executor plugin determines what is done
with the instrumented query produced by GProM.

> **sql** - Print the generated query to *stdout*

> **run** - Run the generated query and show its result

> **dl** - Output a datalog program (only works if *dl* analyzer,
> translator, and parser plugins have been chosen

## analyzer

This plugin checks the output of the parser for semantic correctness.

> **oracle** - Assumes the input is an SQL query written in Oracle's SQL
> dialect

> **dl** - Analyses Datalog inputs

## translator

This plugin translates the input language into **relational algebra**
which is used as an internal code representation by GProM.

> **oracle** - Translates Oracle SQL into relational algebra

> **dl** - Translates Datalog into relational algebra

> **dummy** - Do not translate the input (this can be used to produce an
> output language other than SQL to circumvent the limitations of
> GProM's relational algebra model, e.g., we currently do not support
> recursion)

## metadatalookup

The metadata lookup plugin handles communication with the backend
database. This involves 1) running queries over the catalog of the
backend to do, e.g., semantic analysis and 2) executing queries
instrumented for provenance capture to compute the results of provenance
requests submitted by the user. To be able to do this, the plugin
manages a connection to the backend database using the C library of the
backend DBMS. The type of metadata lookup plugin determines how
connection parameters will be interpreted.

> **oracle** - This plugin manages communication with an Oracle database
> backend. We use Oracle's *OCI* interface wrapped by the open source
> library *OCILIB*.

> **postgres** - This plugin manages communication with a PostgreSQL
> database backend. We use PostgreSQL's *libpq* library.

> **sqlite** - This plugin manages communication with a SQLite database
> backend. We use the *sqlite3-dev* library.

> **monetdb** - This plugin manages communication with a MonetDB
> database backend. We use the MonetDB's *mapi* library.

## sqlcodegen

This plugin translates GProM's internal relational algebra model of
queries into queries written in a backend's SQL dialect.

> **dl** - Output a Datalog program

> **oracle** - Output SQL code written in Oracle's SQL dialect

> **postgres** - Output SQL code written in PostgreSQL's SQL dialect

> **sqlite** - Output SQL code written in SQLite's SQL dialect

## cbo

Select search strategy of the cost-based optimizer

> **exhaustive** - enumerate all options

> **balance** - stop optimization after optimization time exceeds
> estimated runtime of best plan found so far

> **sim_ann** - use simmulated annealing meta-heuristic

# OPTIMIZATION

As mentioned above GProM features a cost-based and heuristic
optimization for relational algebra expressions. Heuristic optimization
rules are mostly relational algebra equivalences. Cost-base optimization
chooses between alternative options for instrumenting a query for
provenance capture and controls the application of some of the algebraic
equivalence rules we support.

## Relational algebra transformations

GProM currently implement the following transformation rules that are
activated with **-O***rule*:

> **merge_ops** - merge adjacent projection and selection operators.
> Selections will always be merged. However, merging projections can
> lead to an explosion of projection expression size. We actively check
> for such cases and avoid merging if this would increase the expression
> size dramatically. For example, consider a projection **A + A AS B**
> followed by a projection **B + B AS C**. Merging these two projections
> would result in the projection expression **A + A + A + A AS C** which
> has double the number of **A** references as the original projection.
> This optimization is important when computing transaction provenance.
> For a thorough explanation see the publications referenced on the
> GProM webpage.

> **factor_attrs** - try to factor attributes in projection expressions
> to reduce the number of references to attributes. We currently support
> addition and multiplication expressions in **CASE** constructs. For
> example, **CASE WHEN** *cond* **THEN A + 2 ELSE A END AS A** can be
> refactored into **A + CASE WHEN** *cond* **THEN 2 ELSE 0 END AS A** to
> reduce the number of references to attribute **A** from 2 to 1.

> **materialize_unsafe_proj** - Force the backend database to
> materialize projections that could lead to uncontrolled expression
> growth if they would be merged with adjacent projections (as explained
> above for **merge_ops**).

> **remove_redundant_projections** - Removes projections that are
> unnecessary from a query, e.g., a projection on **A, B** over a table
> **R(A,B)** is redundant and should be removed to simplify the query.

> **remove_redundant_duplicate_removals** - Removes duplicate removal
> operators if the application of duplicate removal has no effect on the
> query result. We check for two cases here: 1) if the input relation
> has at least one candidate key, then there are no duplicates and the
> operator has no effect and 2) if the result of the duplicate removal
> is later subjected to duplicate removal by a downstream operator and
> none of the operators on the path to this downstream operator are
> sensitive to the number of duplicates then the operator can be safely
> removed.

> **remove_redundant_window_operators** - Remove window operators
> (corresponding to SQL **OVER** clause expressions) which produce an
> output that is not used by any downstream operators.

> **remove_unnecessary_columns** - Based on an analysis of which columns
> of the relation produced by an operator are used by downstream
> operators, we add additional projections to remove unused columns.

> **pullup_duplicate_removals** - This optimization tries to pull up
> duplicate removal operators.

> **pullup_prov_projections** - The provenance instrumentation used by
> GProM duplicates attributes of input tables using projection and
> propagates them to produce results annotated with provenance. This
> optimization tries to pull up such projections to delay the increase
> of schema sized caused by duplicating attributes.

> **selection_move_around** - This optimization applies standard
> selection move-around techniques.

## Cost-based optimization options

The following options control the behavior of GProM's cost-based
optimizer:

> **-cbo_choice_point_remove_duplicate_removal** - makes a cost-based
> choice of whether to remove a duplicate removal operator when possible

> **-cbo_max_considered_plans** *num_plans* - stop cost-based
> optimization after *num_plans* have been considered.

> **-cbo_sim_ann_const** *c* - Set the constant *c* used by the
> simulated annealing search strategy to calculate ap, e.g., c = 10, 20,
> 50 or 100

> **-cbo_sim_ann_cooldown_rate** - Set the cooling down rate used by
> simulated annealing. Value has to be between 0.1 and 0.9.

> **-cbo_num_heuristic_opt_iterations** *num_iter* - Apply each
> heuristic optimization rule *num_iter* times.

# EXAMPLES

**Example 1.** Connect to an Oracle database (*oracle*) at IP *1.1.1.1*
with SID *orcl* using user *usr* and password *mypass* at port *1521*
and start an interactive session:

    gprom -backend oracle -host 1.1.1.1 -user usr -passwd mypass -port 1521 -db orcl

**Example 2.** Same as above, but output instrumented SQL queries to
*stdout* instead of executing them:

    gprom -backend oracle -host 1.1.1.1 -user usr -passwd mypass -port 1521 -db orcl -Pexecutor sql

**Example 3.** Using the same database as in examples 1 and 2, return an
SQL Query that captures provenance for the query **SELECT a FROM r**:

    gprom -backend oracle -host 1.1.1.1 -user usr -passwd mypass -port 1521 -db orcl -Pexecutor sql \
          -query "PROVENANCE OF (SELECT a FROM r);"

**Example 4.** Connect to SQLite database test.db and return provenance
for the query **SELECT a FROM r**:

    gprom -backend sqlite -db test.db \
          -query "PROVENANCE OF (SELECT a FROM r);"

**Example 5.** Connect to SQLite database test.db and return results of
the Datalog query **Q(X) :- R(X,Y).**:

    gprom -backend sqlite -frontend dl -db test.db \
          -query "Q(X) :- R(X,Y)."

# AUTHORS

> **Bahareh Arab** (*barab@hawk.iit.edu*)

> **Su Feng** (*sfeng@hawk.iit.edu*)

> **Boris Glavic** (*bglavic@iit.edu*)

> **Seokki Lee** (*slee195@hawk.iit.edu*)

> **Xing Niu** (*xniu7@hawk.iit.edu*)

# BUGS

> To see a list of current bugs or to report a new bug:
> *https://github.com/IITDBGroup/gprom/issues*

# SEE ALSO

> To learn more about the research behind GProM see
> *http://www.cs.iit.edu/%7edbgroup/research/gprom.php*

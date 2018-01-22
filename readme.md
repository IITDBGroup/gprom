[![analytics](http://www.google-analytics.com/collect?v=1&t=pageview&_s=1&dl=https%3A%2F%2Fgithub.com%2FIITDBGroup%2Fgprom%2Fmain&_u=MAC~&cid=123456789&tid=UA-92255635-2)]()
[![Build Status](https://travis-ci.org/IITDBGroup/gprom.svg?branch=master)](https://travis-ci.org/IITDBGroup/gprom)

# Overview

**PUG** is a provenance middleware that generates (summarized) explanations for why and why-not provenance questions. PUG is implemented as an extension of 
[GProM](https://github.com/IITDBGroup/gprom), a database middleware that adds provenance support to multiple database backends. 

PUGS currently supports the following client interfaces: GProM's interactive shell `gprom`, a C library `libgprom`, a JDBC driver, and a python-based web viewer for provenance graphs.


# Online Demo

* [Online Demo for PUGs Provenance Graph Explorer](http://ec2-35-164-188-60.us-west-2.compute.amazonaws.com:5000)

# Documentation

* [PUG Tutorial (with pictures!)](https://github.com/IITDBGroup/gprom/wiki/datalog_prov)
* [PUG's Web Explorer Usage and Installation](https://github.com/IITDBGroup/gprom/wiki/pgview)
* [PUG's Datalog Dialect](https://github.com/IITDBGroup/gprom/wiki/lang_datalog)
* [Docker containers](https://github.com/IITDBGroup/gprom/wiki/docker)
* [Installation Instructions](https://github.com/IITDBGroup/gprom/wiki/installation)
* [GProM CLI Manual](https://github.com/IITDBGroup/gprom/blob/master/doc/gprom_man.md)

# Features

+ Creates provenance graphs that explain missing and existing answers of queries
+ Automatically generates concise summaries of provenance graphs

# Usage #

PUG currently provides two client interface, **gprom**, the interactive shell of GProM, and a python-based webfront end. 

# Research and Background

The functionality of PUG and GProM are based on long term research efforts by the [IIT DBGroup](http://www.cs.iit.edu/~dbgroup/) studying how to capture provenance on-demand using instrumentation, how to unify why and why-not provenance, and how to efficiently summarize provenance information. Links to [publications](http://www.cs.iit.edu/~dbgroup/publications) and more research oriented descriptions of the techniques implemented in PUG and GProM can be found at [http://www.cs.iit.edu/~dbgroup/research](http://www.cs.iit.edu/~dbgroup/research) or on the project pages for [PUGS](http://www.cs.iit.edu/%7edbgroup/research/pug.php) and [GProM](http://www.cs.iit.edu/%7edbgroup/research/gprom.php). 

Provenance for relational queries records how results of a query depend on the query’s inputs. This type of information can be used to explain why (and how) a result is derived by a query over a given database. Recently, approaches have been developed that use provenance-like techniques to explain why a tuple (or a set of tuples described declaratively by a pattern) is missing from the query result. However, the two problems of computing provenance and explaining missing answers have been treated mostly in isolation.

With **PUG** (**Provenance Unification through Graphs**), we have developed a unified approach for efficiently computing provenance (why) and missing answers (why-not). This approach is based on the observation that in provenance model for queries with unsafe negation, why-not questions can be translated into why questions and vice versa. Typically only a part of the provenance, which we call explanation, is actually relevant for answering the user's provenance question about the existence or absence of a result. We have developed an approach that tries to restrict provenance capture to what is relevant to explain the outcome of interest specified by the user. 

While explanations are useful for reducing the size of provenance, the result may still overwhelm users with too much information and waste computational resources. In particular for why-not, where the provenance explains all failed ways of how a result could have been derived using the rules of a query, provenance graphs may be too large to compute even for small datasets. We address the computational and usability challenges of large provenance graphs by creating summaries based on structural commonalities that exist in the provenance. Importantly, our approach computes summaries based on a sample of the provenance only and, thus, avoids the computationally infeasible step of generating the full why-not provenance for a user question. We have implemented these techniques in our **PUGS** (**Provenance Unification through Graphs with Summarization**) extension of PUG. 

Being based on GProM, PUG and PUGS compile Datalog queries with provenance requests into SQL code and then executes this SQL code on a backend database system and produces a provenance graph explaining the existing and missing answers of interest. Provenance is captured on demand by using a compilation technique called *instrumentation*. Instrumentation rewrites a declarative frontend language with provenance semantics into SQL code. The output of the instrumentation process is a regular SQL query that can be executed using any standard relational database. In the case of PUG, the instrumented query generated from a provenance request returns a the edge relation of a provenance graph. 

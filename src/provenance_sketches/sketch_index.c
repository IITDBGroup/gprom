/*
 *------------------------------------------------------------------------------
 *
 * sketch_index.c - Index structure for storing provenance sketches
 *
 *     This is implements an index structure for storing provenance sketches and
 *     functions to store / load this types of index to / from a database
 *     backend. Currently the index is a hashtable mapping parameterized queries
 *     (query templates) to lists of provenance sketches for certain parameter
 *     bindings for such a template.
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2021-03-03
 *        SUBDIR: src/provenance_sketches/
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "log/logger.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"
#include "metadata_lookup/metadata_lookup.h"
#include "provenance_sketches/sketch_index.h"



void
loadSketchIndexFromDB(char *tablename)
{

}

void
storeSketchIndexToDB(char *tablename)
{

}

/*
 *------------------------------------------------------------------------------
 *
 * metadata_lookup_postgres_plans.h - Functions for translating JSON query plans
 * produced by Postgres's explain command into GProM relational algebra trees
 * (Query Operator model) with annotations for Postgres physical operators and
 * costs.
 *
 *     Postgres's EXPLAIN command can return query plans in JSON. The functions
 *     defined here translate such a JSON representation into GProM internal
 *     relational algebra graphs. We store information about physical operators
 *     and costs as "properties" of the operators in the produced query tree.
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2019-08-25
 *        SUBDIR: include/metadata_lookup/
 *
 *-----------------------------------------------------------------------------
 */

#ifndef _METADATA_LOOKUP_POSTGRES_PLANS_H_
#define _METADATA_LOOKUP_POSTGRES_PLANS_H_

#include "model/node/nodetype.h"

extern Node *translateJSONplanToRA (char *json);

#endif /* _METADATA_LOOKUP_POSTGRES_PLANS_H_ */

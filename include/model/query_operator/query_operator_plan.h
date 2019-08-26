/*
 *------------------------------------------------------------------------------
 *
 * query_operator_plan.h - Functions and definitions for dealing with query operator trees (relational algegra) that encode query plans.
 *
 *     We store plan information in the properties of an operator. Metadata lookup plugins implement functionality to retrieve a plan from a backend.
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2019-08-11
 *        SUBDIR: include/model/query_operator/
 *
 *-----------------------------------------------------------------------------
 */

#ifndef _QUERY_OPERATOR_PLAN_H_
#define _QUERY_OPERATOR_PLAN_H_

// macros that define properties we use to store plan information
#define QO_PLAN_BYTES_PER_ROW "PROP_QOP_BYTES_PER_ROW"
#define QO_PLAN_COST "PROP_QOP_COST"
#define QO_PLAN_LOOPS "PROP_QOP_LOOPS"
#define QO_PLAN_PHYSICAL_OP "PROP_QOP_PHYS_OP"
#define QO_PLAN_ROWS "PROP_QOP_ROWS"
#define QO_PLAN_STARTUP_TIME "PROP_QOP_STARTUP_TIME"
#define QO_PLAN_TIME "PROP_QOP_TIME"
#define QO_PLAN_PAGES_READ_FROM_BUFFER "PROP_QOP_PAGES_READ_FROM_BUFFER"
#define QO_PLAN_PAGES_READ_FROM_DISK "PROP_QOP_PAGES_READ_FROM_DISK"

#endif /* _QUERY_OPERATOR_PLAN_H_ */

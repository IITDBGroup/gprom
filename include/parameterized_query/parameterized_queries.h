/*
 *------------------------------------------------------------------------------
 *
 * parameterized_queries.h - Functions to deal with parameterized queries.
 *
 *     Parameterized queries index and methods to applying binds and creating
 *     parameterized queries by "templatizing" a non-parameterized query.
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2021-04-03
 *        SUBDIR: include/parameterized_queries/
 *
 *-----------------------------------------------------------------------------
 */

#ifndef _PARAMETERIZED_QUERIES_H_
#define _PARAMETERIZED_QUERIES_H_

#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"

extern void setupParameterizedQueryMap();
extern void shutdownParameterizedQueryMap();
extern ParameterizedQuery *getParameterizedQuery(char *name);
extern boolean parameterizedQueryExists(char *name);
extern void createParameterizedQuery(char *name, ParameterizedQuery *q);
extern QueryOperator *parameterizedQueryApplyBinds(char *paramQ, List *binds);
extern ParameterizedQuery *queryToTemplate(QueryOperator *root);


#endif /* _PARAMETERIZED_QUERIES_H_ */

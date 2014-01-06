/*-----------------------------------------------------------------------------
 *
 *  prov_rewriter.h
 *      Main functions of the provenance rewriter.
 *
 *      AUTHOR: Boris Glavic
 *
 *      The provenance rewriter rewrites input queries into queries with
 *      provenance.
 *
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#ifndef PROV_REWRITTER_H_
#define PROV_REWRITTER_H_

#include "model/list/list.h"
#include "model/query_operator/query_operator.h"

extern Node *provRewriteQBModel (Node *);
extern List *provRewriteQueryList (List *list);
extern QueryOperator *provRewriteQuery (QueryOperator *input);

#endif /* PROV_REWRITTER_H_ */

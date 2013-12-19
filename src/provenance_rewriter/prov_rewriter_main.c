/*-----------------------------------------------------------------------------
 *
 * prov_rewriter_main.c
 *		Main entry point to the provenance rewriter.
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "log/logger.h"

#include "provenance_rewriter/prov_rewriter.h"
#include "provenance_rewriter/pi_cs_rewrites/pi_cs_main.h"
#include "provenance_rewriter/transformation_rewrites/transformation_prov_main.h"

#include "model/query_operator/query_operator.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"

/* function declarations */
static QueryOperator *findProvenanceComputations (QueryOperator *op);
static QueryOperator *rewriteProvenanceComputation (ProvenanceComputation *op);

/* function definitions */
Node *
provRewriteQBModel (Node *qbModel)
{
    if (isA(qbModel, List))
        return (Node *) provRewriteQueryList((List *) qbModel);
    else if (isA(qbModel, QueryOperator))
        return (Node *) provRewriteQuery((QueryOperator *) qbModel);

    FATAL_LOG("cannot rewrite node <%s>", nodeToString(qbModel));

    return NULL;
}

List *
provRewriteQueryList (List *list)
{
    FOREACH(QueryOperator,q,list)
        q_his_cell->data.ptr_value = provRewriteQuery(q);

    return list;
}


QueryOperator *
provRewriteQuery (QueryOperator *input)
{
    return findProvenanceComputations(input);
}


QueryOperator *
findProvenanceComputations (QueryOperator *op)
{
    // is provenance computation? then rewrite
    if (isA(op, ProvenanceComputation))
        return rewriteProvenanceComputation((ProvenanceComputation *) op);

    // else search for children with provenance
    FOREACH(QueryOperator, x, op->inputs)
        findProvenanceComputations(x);

    return op;
}

QueryOperator *
rewriteProvenanceComputation (ProvenanceComputation *op)
{
    switch(op->provType)
    {
        case PI_CS:
            return rewritePI_CS(op);
        case TRANSFORMATION:
            return rewriteTransformationProvenance((QueryOperator *) op);
    }
    return NULL;
}

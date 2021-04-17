/*
 *------------------------------------------------------------------------------
 *
 * parameterized_queries.c - functions for dealing with parameterized queries
 *
 *     Functions for dealing with parameterized queries and an index mapping
 *     names to parameterized queries.
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2021-04-03
 *        SUBDIR: src/parameterized_queries/
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "analysis_and_translate/parameter.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/set/hashmap.h"
#include "parameterized_query/parameterized_queries.h"

#define PARAM_MAP_CONTEXT_NAME "parameterized_queries"

#define ENSURE_PARAM_EXISTS \
	do { \
		if (paramQueries == NULL) \
		{ \
			setupParameterizedQueryMap(); \
		} \
	} while(0)


typedef struct QueryToTemplateContext {
	List *vals;
	boolean inParameterizableExpr;
} QueryToTemplateContext;

static HashMap *paramQueries = NULL;
static MemContext *paramContext = NULL;
static boolean queryToTemplateVisitor(Node *node, QueryToTemplateContext *state);
static Node *queryToTemplateMutator(Node *n, QueryToTemplateContext *state);

void
setupParameterizedQueryMap()
{
	paramContext = NEW_LONGLIVED_MEMCONTEXT(PARAM_MAP_CONTEXT_NAME);
	ACQUIRE_MEM_CONTEXT(paramContext);
	paramQueries = NEW_MAP(Constant,ParameterizedQuery);
	RELEASE_MEM_CONTEXT();
}

void
shutdownParameterizedQueryMap()
{
	FREE_MEM_CONTEXT(paramContext);
	paramQueries = NULL;
	paramContext = NULL;
}

ParameterizedQuery *
getParameterizedQuery(char *name)
{
	ENSURE_PARAM_EXISTS;
	return (ParameterizedQuery *) getMapString(paramQueries, name);
}

boolean
parameterizedQueryExists(char *name)
{
	ENSURE_PARAM_EXISTS;
	return MAP_HAS_STRING_KEY(paramQueries, name);
}

void
createParameterizedQuery(char *name, ParameterizedQuery *q)
{
	ENSURE_PARAM_EXISTS;
	MAP_ADD_STRING_KEY(paramQueries, name, q);
}

QueryOperator *
parameterizedQueryApplyBinds(char *paramQ, List *binds)
{
	ParameterizedQuery *p = getParameterizedQuery(paramQ);
	return (QueryOperator *) applyBinds(p, binds);
}

ParameterizedQuery *
queryToTemplate(QueryOperator *root)
{
	ParameterizedQuery *result = makeNode(ParameterizedQuery);
	List *paramValues = NIL;
	QueryToTemplateContext *context = NEW(QueryToTemplateContext);
	QueryOperator *parameterizedQ = copyObject(root);

	queryToTemplateVisitor((Node *) parameterizedQ, context);
	result->parameters = paramValues;

	return result;
}

static boolean
queryToTemplateVisitor(Node *node, QueryToTemplateContext *state)
{
	if (node == NULL)
		return TRUE;

	if (isA(node, SelectionOperator))
	{
		SelectionOperator *s = (SelectionOperator *) node;
		s->cond = queryToTemplateMutator(node, state);
	}
	if (isA(node, JoinOperator))
	{
		JoinOperator *j = (JoinOperator *) node;
		j->cond = queryToTemplateMutator(node, state);
	}
	//TODO which else?

	return visit(node, queryToTemplateVisitor, state);
}

static Node *
queryToTemplateMutator(Node *node, QueryToTemplateContext *state)
{
	if (node == NULL)
        return NULL;

	if (isA(node, Constant))
    {
		// only replace in parameterizable expresison e.g., selection condition
        SQLParameter *p = (SQLParameter *) node;
        p->position = LIST_LENGTH(state->vals) + 1;
        Node *val = copyObject(node);

		state->vals = appendToTailOfList(state->vals, val);
        DEBUG_LOG("replaced constant <%s> with parameter <%s>",
				  nodeToString(val), nodeToString(p));

        return copyObject(val);
    }

    return mutate(node, queryToTemplateMutator, state);
}

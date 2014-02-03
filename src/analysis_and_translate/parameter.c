/*-----------------------------------------------------------------------------
 *
 * parameter.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"

#include "model/node/nodetype.h"
#include "model/list/list.h"

// static functions
static boolean findParamVisitor(Node *node, List **state);
static Node *replaceParamMutator (Node *node, List *state);

Node *
setParameterValues (Node *qbModel, List *values)
{
    List *params = findParameters (qbModel);

    return replaceParamMutator(qbModel, values);
}

static Node *
replaceParamMutator (Node *node, List *state)
{
    if (node == NULL)
        return NULL;

    if (isA(node, SQLParameter))
    {
        SQLParameter *p = (SQLParameter *) node;
        int pos = p->position;
        assert(pos > 0 && pos <= LIST_LENGTH(state));

        return copyObject(getNthOfListP(state, pos - 1));
    }

    return mutate(node, replaceParamMutator, state);
}


List *
findParameters (Node *qbModel)
{
    List *result = NIL;

    findParamVisitor(qbModel, &result);

    return result;
}

static boolean
findParamVisitor(Node *node, List **state)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, SQLParameter))
        *state = appendToTailOfList(*state, node);

    return visit(node, findParamVisitor, (void *) state);
}

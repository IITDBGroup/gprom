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

#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "model/expression/expression.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "analysis_and_translate/parameter.h"

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
        Node *val;

        assert(pos > 0 && pos <= LIST_LENGTH(state));

        val = getNthOfListP(state, pos - 1);
        DEBUG_LOG("replaced parameter <%s> with <%s>", nodeToString(p), nodeToString(val));

        return copyObject(val);
    }

    return mutate(node, replaceParamMutator, state);
}


List *
findParameters (Node *qbModel)
{
    List *result = NIL;

    findParamVisitor(qbModel, &result);

    DEBUG_LOG("parameters are: %s", nodeToString(result));

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

List *
oracleBindToConsts (char *binds)
{
    List *result = NIL;
    char *pos = binds;

    while(*pos != '\0')
    {
        int param;
        int len;
        char *value;
        int retVal;
        int read;

        // do not advance after last position of input string
        assert(pos - binds <= strlen(binds) + 1);

        // read parameter number and length. The format is #PARAM_NUM(LENGTH):
        retVal = sscanf(pos,"#%u(%u):%n", &param, &len, &read);
        DEBUG_LOG("#%u(%u): of length %u", param, len, read);
        if (retVal != 2)
            FATAL_LOG("String <%s> at <%s> did not match #Param(length)", binds, pos);
        pos += read;
        DEBUG_LOG("remaining parse is: <%s>", pos);
        //TODO check: can parameter list be different from 1, 2, 3, 4, ...

        // read value
        value = CALLOC(len + 1, 1);
        strncpy(value, pos, len);
        value[len] = '\0';
        pos += len;
        DEBUG_LOG("remaining parse is: <%s>", pos);

        result = appendToTailOfList(result, (Node *) createConstString(value));
        DEBUG_LOG("parameters parsed so far <%s>", nodeToString(result));
        FREE(value);
    }

    return result;
}

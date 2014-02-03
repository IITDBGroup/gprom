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

#define CONSUME(str,expt) \
    do { \
        char *_intExpt = (expt); \
        if(strncmp(str, _intExpt, strlen(expt)) != 0) \
           FATAL("expected %s, but was %s", _intExpt, str); \
        str += strlen(_intExpt); \
    } while(0)

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
        assert(pos - binds <= strlen(binds));

        // read parameter number and length. The format is #PARAM_NUM(LENGTH):
        retVal = sscanf(pos,"#%u(%u):%n", &param, &len, &read);
        if (retVal != 2)
            FATAL_LOG("String <%s> at <%s> did not match #Param(length)", binds, pos);
        pos += read;
        //TODO check: can parameter list be different from 1, 2, 3, 4, ...

        // read value
        value = CALLOC(len + 1, 1);
        strncpy(value, pos, len);
        value[len] = '\0';
        pos += len;

        result = appendToTailOfList(result, (Node *) createConstString(value));
        FREE(value);
    }

    return result;
}

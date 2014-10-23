/*-----------------------------------------------------------------------------
 *
 * datalog_model.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"

#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "model/expression/expression.h"
#include "model/datalog/datalog_model.h"

static List *getAtomVars(DLAtom *a);
static List *getComparisonVars(DLComparison *a);

DLAtom *
createDLAtom (char *rel, List *args, boolean negated)
{
    DLAtom *result = makeNode(DLAtom);

    result->rel = rel;
    result->args = args;
    result->negated = negated;

    return result;
}

DLVar *
createDLVar (char *vName, DataType vType)
{
    DLVar *result = makeNode(DLVar);

    result->name = vName;
    result->dt = vType;

    return result;
}

boolean
isConstAtom (DLAtom *a)
{
    FOREACH(Node,arg,a->args)
    {
        if (!isA(arg, Constant))
            return FALSE;
    }

    return TRUE;
}

DLRule *
createDLRule (DLAtom *head, List *body)
{
    DLRule *result = makeNode(DLRule);

    result->head = head;
    result->body = body;

    return result;
}


DLProgram *
createDLProgram (List *dlRules, List *facts, char *ans)
{
    DLProgram *result = makeNode(DLProgram);

    result->rules = dlRules;
    result->facts = facts;
    result->ans = ans;

    return result;
}

DLComparison *
createDLComparison (char *op, Node *lArg, Node *rArg)
{
    DLComparison *result = makeNode(DLComparison);

    result->opExpr = createOpExpr(op, LIST_MAKE(lArg, rArg));

    return result;
}

char *
getHeadPredName(DLRule *r)
{
    DLAtom *h = r->head;
    return h->rel;
}

List *
getRuleVars (DLRule *r)
{
    List *result = NIL;

    result = CONCAT_LISTS(result, getAtomVars(r->head));
    result = CONCAT_LISTS(result, getBodyVars(r));

    return result;
}

List *
getBodyVars (DLRule *r)
{
    List *result = NIL;

    FOREACH(Node,a,r->body)
    {
        if (isA(a, DLAtom))
            result = CONCAT_LISTS(result,
                    getAtomVars((DLAtom *) a));
        else if (isA(a,DLComparison))
            result = CONCAT_LISTS(result,
                    getComparisonVars((DLComparison *) a));
    }

    return result;
}

List *
getBodyPredVars (DLRule *r)
{
    List *result = NIL;

    FOREACH(Node,a,r->body)
    {
        if (isA(a, DLAtom))
            result = CONCAT_LISTS(result,
                    getAtomVars((DLAtom *) a));
    }

    return result;
}


List *
getHeadVars (DLRule *r)
{
    return getAtomVars(r->head);
}

List *
getVarNames (List *vars)
{
    List *result = NIL;

    FOREACH(DLVar,v,vars)
        result = appendToTailOfList(result, strdup(v->name));

    return result;
}

static List *
getAtomVars(DLAtom *a)
{
    List *result = NIL;

    FOREACH(Node,arg,a->args)
    {
        if(isA(arg, DLVar))
            result = appendToTailOfList(result, arg);
    }

    return result;
}

static List *
getComparisonVars(DLComparison *a)
{
    DLVar *l = getNthOfListP(a->opExpr->args,0);
    DLVar *r = getNthOfListP(a->opExpr->args,1);

    return LIST_MAKE(l,r);
}

Node *
getDLProp(DLNode *n, char *key)
{
    if (n->properties == NULL)
        return NULL;

    return MAP_GET_STRING(n->properties, key);
}

void
setDLProp(DLNode *n, char *key, Node *value)
{
    if (n->properties == NULL)
        n->properties = NEW_MAP(Node,Node);

    MAP_ADD_STRING_KEY(n->properties, key, value);
}

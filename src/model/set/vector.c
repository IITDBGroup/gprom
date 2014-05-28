/*-----------------------------------------------------------------------------
 *
 * vector.c
 *			  
 *		- Basic Vector data structure
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
#include "model/set/vector.h"

#define INITIAL_VEC_SIZE 16

static void extendVector(Vector *v);

Vector *
makeVector(VectorType type, NodeTag nodeType)
{
    Vector *result = makeNode(Vector);

    result->elType = type;
    result->elNodeType = nodeType;
    result->length = 0;
    result->maxLength = INITIAL_VEC_SIZE;

    switch(type)
    {
        case VECTOR_INT:
            result->data = MALLOC(sizeof(int) * INITIAL_VEC_SIZE);
            break;
        default:
            result->data = MALLOC(sizeof(void *) * INITIAL_VEC_SIZE);
            break;
    }

    return result;
}


void
vecAppend(Vector *v, Node *el)
{
    if (v->maxLength == v->length)
        extendVector(v);

    ARR(v,Node)[v->length++] = el;
}

void
vecAppendInt(Vector *v, int el)
{
    if (v->maxLength == v->length)
        extendVector(v);

    VEC_TO_IA(v)[v->length++] = el;
}

static void
extendVector(Vector *v)
{
    char *newData;
    size_t typeSize;

    switch(v->elType)
    {
        case VECTOR_INT:
            typeSize = sizeof(int);
            break;
        default:
            typeSize = sizeof(void *);
            break;
    }

    newData = MALLOC(typeSize * v->maxLength * 2);
    memcpy(newData, v->data, typeSize * v->length);

    v->data = newData;
    v->maxLength *= 2;
}

Node *
getVec(Vector *v, int pos)
{
    ASSERT(pos >= 0 && pos < VEC_LENGTH(v));

    return VEC_TO_ARR(v,Node)[pos];
}

int
getVecInt(Vector *v, int pos)
{
    ASSERT(pos >= 0 && pos < VEC_LENGTH(v));

    return VEC_TO_IA(v)[pos];
}

boolean
findVecElem(Vector *v, Node *el)
{
    FOREACH_VEC(Node,n,v)
        if (equal(el,n))
            return TRUE;

    return FALSE;
}

boolean
findVecInt(Vector *v, int el)
{
    FOREACH_VEC(int,e,v)
       if (*e == el)
           return TRUE;

    return FALSE;
}

void
freeVec (Vector *v)
{
    FREE(v->data);
    FREE(v);
}


void
deepFreeVec (Vector *v)
{
    FOREACH_VEC(void,el,v)
    {
        FREE(el);
    }
    FREE(v->data);
    FREE(v);
}

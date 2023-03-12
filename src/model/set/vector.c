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
#include "instrumentation/timing_instrumentation.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/vector.h"

#define INITIAL_VEC_SIZE 16

static void extendVector(Vector *v);

Vector *
makeVector(VectorType type, NodeTag nodeType)
{
    return makeVectorOfSize(type, nodeType, INITIAL_VEC_SIZE);
}

Vector *
makeVectorOfSize(VectorType type, NodeTag nodeType, int numElem)
{
    Vector *result = makeNode(Vector);

        result->elType = type;
        result->elNodeType = nodeType;
        result->length = 0;
        result->maxLength = numElem;

        switch(type)
        {
            case VECTOR_INT:
                result->data = MALLOC(sizeof(int) * numElem);
                break;
            case VECTOR_LONG:
            	result->data = MALLOC(sizeof(gprom_long_t) * numElem);
            	break;
            case VECTOR_FLOAT:
				result->data = MALLOC(sizeof(double) * numElem);
            	break;
            default:
                result->data = MALLOC(sizeof(void *) * numElem);
                break;
        }

        return result;
}

Vector *
makeVectorFromElem(VectorType type, NodeTag nodeType, void *elem, ...)
{
    Vector *result = makeVector(type, nodeType);
    va_list args;
    void *arg;

    va_start(args, elem);

    for(arg = elem; arg != NULL; arg = va_arg(args,void*))
    {
        TRACE_LOG("add pointer %p to vector", arg);
        vecAppendNode(result, arg);
    }

    va_end(args);

    return result;
}

Vector *
makeVectorIntFromElem(int elem, ...)
{
	Vector *result = makeVector(VECTOR_INT, T_Invalid);
    va_list args;
    int arg;

    va_start(args, elem);

    for(arg = elem; arg >= 0; arg = va_arg(args,int))
    {
        TRACE_LOG("add int %d to set", arg);
        vecAppendInt(result, arg);
    }

    va_end(args);

    return result;
}

Vector *
makeVectorIntSeq (int start, int length, int step)
{
    Vector *result = makeVectorOfSize(VECTOR_INT, T_Invalid, length);
    int *a = VEC_TO_IA(result);

    for(int i = 0, val = start; i < length; i++, val +=step)
        a[i] = val;
    result->length = length;

    return result;
}

Vector *
makeVectorFromList (List *input)
{
    if (input == NULL)
        return NULL;

    VectorType t = isA(input,IntList) ? VECTOR_INT : VECTOR_NODE;
    Vector *result = makeVectorOfSize(t, T_Invalid, LIST_LENGTH(input));

    FOREACH(Node,n,input)
        vecAppendNode(result,n);

    return result;
}
Vector *
makeVectorLongFromElem(gprom_long_t elem, ...)
{
	Vector *result = makeVector(VECTOR_LONG, T_Invalid);
    va_list args;
    gprom_long_t arg;

    va_start(args, elem);

    for(arg = elem; arg >= 0; arg = va_arg(args,gprom_long_t))
    {
        TRACE_LOG("add long %ld to set", arg);
        vecAppendLong(result, arg);
    }

    va_end(args);

    return result;
}

Vector *
makeVectorFloatFromElem(double elem, ...)
{
	Vector *result = makeVector(VECTOR_FLOAT, T_Invalid);
    va_list args;
    double arg;

    va_start(args, elem);

    for(arg = elem; arg >= 0; arg = va_arg(args,double))
    {
        TRACE_LOG("add int %f to set", arg);
        vecAppendFloat(result, arg);
    }

    va_end(args);

    return result;
}

size_t
getVecElemSize(Vector *v)
{
    switch(v->elType)
    {
        case VECTOR_INT:
            return sizeof(int);
        case VECTOR_LONG:
        	return sizeof(gprom_long_t);
        case VECTOR_FLOAT:
        	return sizeof(double);
        default:
            return sizeof(void *);
    }
}

size_t
getVecDataSize(Vector *v)
{
    return v->maxLength * getVecElemSize(v);
}

void
vecAppendNode(Vector *v, Node *el)
{
    if (v->maxLength == v->length)
        extendVector(v);

    VEC_TO_ARR(v,Node)[v->length++] = el;
}

void
vecAppendInt(Vector *v, int el)
{
    if (v->maxLength == v->length)
        extendVector(v);

    VEC_TO_IA(v)[v->length++] = el;
}

void
vecAppendLong(Vector *v, gprom_long_t el)
{
	if (v->maxLength == v->length)
		extendVector(v);

	VEC_TO_LA(v)[v->length++] = el;
}

void
vecAppendFloat(Vector *v, double el)
{
	if (v->maxLength == v->length)
		extendVector(v);

	VEC_TO_FA(v)[v->length++] = el;
}

void
vecAppendString(Vector *v, char *el)
{
	if (v->maxLength == v->length)
		extendVector(v);

	VEC_TO_ARR(v,char)[v->length++] = el;
}

static void
extendVector(Vector *v)
{
    char *newData;
    size_t typeSize = getVecElemSize(v);

    newData = MALLOC(typeSize * v->maxLength * 2);
    memcpy(newData, v->data, typeSize * v->length);

    v->data = newData;
    v->maxLength *= 2;
}

Node *
getVecNode(Vector *v, int pos)
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

char *
getVecString(Vector *v, int pos)
{
	ASSERT(pos >=0 && pos < VEC_LENGTH(v));

	return VEC_TO_ARR(v,char)[pos];
}

gprom_long_t
getVecLong(Vector *v, int pos)
{
	ASSERT(pos >=0 && pos < VEC_LENGTH(v));

	return VEC_TO_LA(v)[pos];
}

double
getVecFloat(Vector *v, int pos)
{
	ASSERT(pos >=0 && pos < VEC_LENGTH(v));

	return VEC_TO_FA(v)[pos];
}

boolean
findVecNode(Vector *v, Node *el)
{
    FOREACH_VEC(Node,n,v)
    {
    	if (equal(el,n))
            return TRUE;
    }

    return FALSE;
}

boolean
findVecInt(Vector *v, int el)
{
    FOREACH_VEC_INT(e,v)
	{
       if (e == el)
           return TRUE;
	}

    return FALSE;
}

boolean
findVecString(Vector *v, char *el)
{
	FOREACH_VEC(char,s,v)
	{
		if (streq(s, el))
		{
			return TRUE;
		}
	}

	return FALSE;
}

boolean
findVecLong(Vector *v, gprom_long_t el)
{
	FOREACH_VEC_LONG(e, v)
	{
		if (e == el)
		{
			return TRUE;
		}
	}

	return FALSE;
}

boolean
findVecFloat(Vector *v, double el)
{
	FOREACH_VEC_FLOAT(e, v)
	{
		if (e == el)
		{
			return TRUE;
		}
	}
	return FALSE;
}

boolean
shrinkVector(Vector *v, int newSize)
{
    ASSERT(newSize >= 0);

    if (VEC_LENGTH(v) <= newSize)
        return FALSE;

    v->length = newSize;

    return TRUE;
}

int
popVecInt(Vector *v)
{
    ASSERT(v->length > 0);

    int result = getVecInt(v, v->length - 1);
    v->length--;

    return result;
}

Node *
popVecNode(Vector *v)
{
    ASSERT(v->length > 0);

    Node *result = getVecNode(v, v->length - 1);
    v->length--;

    return result;
}

char *
popVecString(Vector *v)
{
	ASSERT(v->length > 0);

	char *result = getVecString(v, v->length - 1);
	v->length--;

	return result;
}

gprom_long_t
popVecLong(Vector *v)
{
	ASSERT(v->length > 0);

	gprom_long_t result = getVecLong(v, v->length - 1);
	v->length--;

	return result;
}

double
popVecFloat(Vector *v)
{
	ASSERT(v->length > 0);

	double result = getVecFloat(v, v->length - 1);
	v->length--;

	return result;
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

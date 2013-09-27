/*************************************
 *         copy.c
 *    Author: Hao Guo
 *    copy function for tree nodes
 *
 *
 *
 **************************************/

#include <string.h>

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/expression/expression.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"

/* functions to copy specific node types */
static List *deepCopyList(List *from);
static AttributeReference *copyAttributeReference(AttributeReference *from);
static QueryBlock *copyQueryBlock(QueryBlock *from);
static SelectItem *copySelectItem(SelectItem *from);


/*use the Macros(the varibles are 'new' and 'from')*/

/* creates a new pointer to a node and allocated mem */
#define COPY_INIT(type) \
		type *new; \
		new = makeNode(type);

/*copy a simple scalar field(int, bool, float, etc)*/
#define COPY_SCALAR_FIELD(fldname)  \
		new->fldname = from->fldname

/*copy a field that is a pointer to Node or Node tree*/
#define COPY_NODE_FIELD(fldname)  \
		new->fldname = (copyObject(from->fldname))

/*copy a field that is a pointer to C string or NULL*/
#define COPY_STRING_FIELD(fldname) \
		new->fldname = (from->fldname != NULL ? strcpy((char *) MALLOC(strlen(from->fldname) + 1), \
				from->fldname) : NULL)

/*deep copy for List operation*/
static List *
deepCopyList(List *from)
{
    COPY_INIT(List);

    assert(getListLength(from) >= 1);
    COPY_SCALAR_FIELD(length);
    COPY_SCALAR_FIELD(type); // if it is an Int_List

    if (from->type == T_IntList)
    {
        FOREACH_INT(i, from)
            new = appendToTailOfListInt(new, i);
    }
    else
    {
        FOREACH(Node,n,from)
            new = appendToTailOfList(new, copyObject(n));
    }

    return new;
}

static AttributeReference *
copyAttributeReference(AttributeReference *from)
{
    COPY_INIT(AttributeReference);

    COPY_STRING_FIELD(name);

    return new;
}

static QueryBlock *
copyQueryBlock(QueryBlock *from)
{
    COPY_INIT(QueryBlock);

    COPY_NODE_FIELD(selectClause);
    COPY_NODE_FIELD(distinct);
    COPY_NODE_FIELD(fromClause);
    COPY_NODE_FIELD(whereClause);
    COPY_NODE_FIELD(havingClause);

    return new;
}

static SelectItem *
copySelectItem(SelectItem *from)
{
    COPY_INIT(SelectItem);

    COPY_STRING_FIELD(alias);
    COPY_NODE_FIELD(expr);

    return new;
}

/*copyObject copy of a Node tree or list and all substructure copied too */
/*this is a deep copy & with recursive*/

void *copyObject(void *from)
{
    void *retval;

    if(from == NULL)
        return NULL;

    switch(nodeTag(from))
    {
        /*different type nodes*/

        /*list nodes*/
        case T_List:
        case T_IntList:
            retval = deepCopyList(from);
            break;
        case T_AttributeReference:
            retval = copyAttributeReference(from);
            break;
            //T_QueryOperator
            //T_SelectionOperator
            //T_ProjectionOperator
            //T_JoinOperator
            //T_AggregationOperator
            //T_ProvenanceComputation
            //T_QueryBlock model
        case T_QueryBlock:
            retval = copyQueryBlock(from);
            break;
        case T_SelectItem:
            retval = copySelectItem(from);
            break;
        default:
            retval = NULL;
            break;
    }

    return retval;
}

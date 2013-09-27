/*************************************
 *         to_string.c
 *    Author: Hao Guo
 * Implement a function that given a query
 * tree or expression tree that returns a
 * string representing the tree.
 *
 **************************************/

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/expression/expression.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"

/* functions to output specific node types */
static void outList(StringInfo str, List *node);
static void outNode(StringInfo, void *node);
static void outQueryBlock (StringInfo str, QueryBlock *node);

/*define macros*/
/*label for the node type*/
#define booltostr(a) \
		((a) ? "true" : "false")

#define WRITE_NODE_TYPE(nodelabel)  \
		appendStringInfoString(str,  CppAsString(nodelabel));

#define CppAsString(token) #token

/*int field*/
#define WRITE_INT_FIELD(fldname)  \
		appendStringInfo(str, ":" CppAsString(fldname) "%d", node->fldname)

/*long-int field*/
#define WRITE_LONG_FIELD(fldname)  \
		appendStringInfo(str, ":" CppAsString(fldname) "%ld", node->fldname)

/*char field*/
#define WRITE_CHAR_FIELD(fldname)  \
		appendStringInfo(str, ":" CppAsString(fldname) "%c", node->fldname)

/*enum-type field as integer*/
#define WRITE_ENUM_FIELD(fldname, enumtype)  \
		appendStringInfo(str, ":" CppAsString(fldname) "%d", (int)node->fldname)

/*float field*/
#define WRITE_FLOAT_FIELD(fldname, format)  \
		appendStringInfo(str, ":" CppAsString(fldname) " " format, node->fldname)

/*bool field*/
#define WRITE_BOOL_FIELD(fldname)  \
		appendStringInfo(str, ":" CppAsString(fldname) "%s", booltostr(node->fldname))

/*node field*/
#define WRITE_NODE_FIELD(fldname)  \
		appendStringInfoString(str, ":" CppAsString(fldname) " "); outNode(str, node->fldname);

/*outNode from node append it to string*/
static void
outList(StringInfo str, List *node)
{
    appendStringInfo(str, "(");

    if(isA(node, IntList))
    {
        FOREACH_INT(i, node)
        {
            appendStringInfo(str, "i%d", i);
            if (i_his_cell->next)
                appendStringInfoString(str, " ");
        }
    }
    else
    {
        FOREACH(Node,n,node)
        {
            outNode(str, n);
            if (n_his_cell->next)
                appendStringInfoString(str, " ");
        }
    }
    appendStringInfo(str, ")");
}

static void
outQueryBlock (StringInfo str, QueryBlock *node)
{
    WRITE_NODE_TYPE(QUERYBLOCK);

    WRITE_NODE_FIELD(distinct);
    WRITE_NODE_FIELD(selectClause);
    WRITE_NODE_FIELD(fromClause);
    WRITE_NODE_FIELD(whereClause);
    WRITE_NODE_FIELD(havingClause);
}

//typedef struct Value
//{
//   NodeTag type;
//
//   union ValUnion
//     {
//        long   ival; /*integer*/
//        char   *str;   /*string*/
//
//     }val;
//}Value;

//static void outValue(StringInfo str, Value *value)
//{
//        switch (value->type)
//           {
////              case T_Integer:
////                   appendStringInfo(str, "%ld", value->val.ival);
////                   break;
////
////              case T_NULL:
////                   appendStringInfoString(str, "NULL");
////                   break;
//
//              default :
//                   break;
//           }
//}

void outNode(StringInfo str, void *obj)
{
    if(obj == NULL)
        appendStringInfoString(str, "<>");
    else if(isA(obj, List) || isA(obj, IntList))
        outList(str, obj);
    else
    {
        appendStringInfoString(str, "{");
        switch (nodeTag(obj))
        {
            case T_List:
            case T_IntList:
                outList(str, (List *) obj);
                break;
            case T_QueryBlock:
                outQueryBlock(str, (QueryBlock *) obj);
                break;
            //different case
            //query operator model nodes
            //T_QueryOperator,
            //T_SelectionOperator,
            //T_ProjectionOperator,
            //T_JoinOperator,
            //T_AggregationOperator,
            //T_ProvenanceComputation

            default :
                //outNode(str, obj);
                break;
        }
        appendStringInfoString(str, "}");
    }
}

/*nodeToString return the node as string*/

char *nodeToString(void *obj)
{
    StringInfo str;
    str = makeStringInfo();
    outNode(str, obj);
    return str->data;
}

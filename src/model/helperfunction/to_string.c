/*************************************
 *         to_string.c
 *    Author: Hao Guo
 * Implement a function that given a query
 * tree or expression tree that returns a
 * string representing the tree.
 *
 **************************************/

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/expression/expression.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"

/* functions to output specific node types */
static void outList(StringInfo str, List *node);
static void outNode(StringInfo, void *node);
static void outQueryBlock (StringInfo str, QueryBlock *node);
static void outConstant (StringInfo str, Constant *node);
static void outSelectItem (StringInfo str, SelectItem *node);
static void writeCommonFromItemFields(StringInfo str, FromItem *node);
static void outFromTableRef (StringInfo str, FromTableRef *node);
static void outAttributeReference (StringInfo str, AttributeReference *node);
static void indentString(StringInfo str, int level);

/*define macros*/
/*label for the node type*/
#define booltostr(a) \
		((a) ? "true" : "false")

#define WRITE_NODE_TYPE(nodelabel)  \
		appendStringInfoString(str,  CppAsString(nodelabel));

#define CppAsString(token) #token

/*int field*/
#define WRITE_INT_FIELD(fldname)  \
		appendStringInfo(str, ":" CppAsString(fldname) "|%d", node->fldname)

/*long-int field*/
#define WRITE_LONG_FIELD(fldname)  \
		appendStringInfo(str, ":" CppAsString(fldname) "|%ld", node->fldname)

/*char field*/
#define WRITE_CHAR_FIELD(fldname)  \
		appendStringInfo(str, ":" CppAsString(fldname) "|%c", node->fldname)

/*string field*/
#define WRITE_STRING_FIELD(fldname)  \
        appendStringInfo(str, ":" CppAsString(fldname) "|\"%s\"", node->fldname)


/*enum-type field as integer*/
#define WRITE_ENUM_FIELD(fldname, enumtype)  \
		appendStringInfo(str, ":" CppAsString(fldname) "|%d", (int)node->fldname)

/*float field*/
#define WRITE_FLOAT_FIELD(fldname, format)  \
		appendStringInfo(str, ":" CppAsString(fldname) "|" format, node->fldname)

/*bool field*/
#define WRITE_BOOL_FIELD(fldname)  \
		appendStringInfo(str, ":" CppAsString(fldname) "|%s", booltostr(node->fldname))

/*node field*/
#define WRITE_NODE_FIELD(fldname)  \
		appendStringInfoString(str, ":" CppAsString(fldname) "|"); outNode(str, node->fldname);

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
    appendStringInfoString(str, ")");
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

void
outConstant (StringInfo str, Constant *node)
{
    WRITE_NODE_TYPE(CONSTANT);

    WRITE_ENUM_FIELD(constType, DataType);
    appendStringInfoString(str, ":value ");
    switch(node->constType)
    {
        case DT_INT:
            appendStringInfo(str, "%u", *((int *) node->value));
            break;
        case DT_FLOAT:
            appendStringInfo(str, "%f", *((double *) node->value));
            break;
        case DT_STRING:
            appendStringInfoString(str, (char *) node->value);
            break;
        case DT_BOOL:
            appendStringInfo(str, "%s", *((boolean *) node->value) == TRUE ? "TRUE" : "FALSE");
            break;
    }
}

static void
outSelectItem (StringInfo str, SelectItem *node)
{
    WRITE_NODE_TYPE(SELECT_ITEM);

    WRITE_STRING_FIELD(alias);
    WRITE_NODE_FIELD(expr);
}

static void
writeCommonFromItemFields(StringInfo str, FromItem *node)
{
    WRITE_STRING_FIELD(name);
    WRITE_NODE_FIELD(attrNames);
}

static void
outFromTableRef (StringInfo str, FromTableRef *node)
{
    WRITE_NODE_TYPE(FROM_TABLE_REF);

    writeCommonFromItemFields(str, (FromItem *) node);
    WRITE_STRING_FIELD(tableId);
}

static void
outAttributeReference (StringInfo str, AttributeReference *node)
{
    WRITE_NODE_TYPE(ATTRIBUTE_REFERENCE);

    WRITE_STRING_FIELD(name);
}

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
            case T_Constant:
                outConstant(str, (Constant *) obj);
                break;
            case T_SelectItem:
                outSelectItem(str, (SelectItem *) obj);
                break;
            case T_FromTableRef:
                outFromTableRef(str, (FromTableRef *) obj);
                break;
            case T_AttributeReference:
                outAttributeReference(str, (AttributeReference *) obj);
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

char *
beatify(char *input)
{
    StringInfo str = makeStringInfo();
    char *result;
    int indentation = 0;
    boolean inString = FALSE;

    if(input == NULL)
        return NULL;

    for(; *input != '\0'; input++)
    {
        char c = *input;
        if (inString)
        {
            switch(c)
            {
                case '"':
                    inString = FALSE;
                default:
                    appendStringInfoChar (str, c);
            }
        }
        else
        {
            switch (c)
            {
                case '(':
                    indentString(str, indentation);
                    indentation++;
                    appendStringInfoString(str, "(");
                    break;
                case '{':
                    appendStringInfoChar(str, '\n');
                    indentString(str, indentation);
                    indentation++;
                    appendStringInfoString(str, "{");
                    break;
                case ')':
                case '}':
                    indentation--;
                    appendStringInfoChar(str, '\n');
                    indentString(str, indentation);
                    appendStringInfo(str, "%c",c);
                    break;
                case ':':
                    appendStringInfoString(str,"\n");
                    indentString(str, indentation);
                    break;
                case '|':
                    appendStringInfoString(str,": ");
                    break;
                case '"':
                    inString = TRUE;
                default:
                    appendStringInfoChar (str, c);
            }
        }
    }

    result = str->data;
    FREE(str);
    return result;
}

static void
indentString(StringInfo str, int level)
{
   while(level-- > 0)
       appendStringInfoChar(str, '\t');
}

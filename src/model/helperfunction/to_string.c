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
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/expression/expression.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"

/* functions to output specific node types */
static void outPointerList (StringInfo str, List *node);
static void outList(StringInfo str, List *node);
static void outStringList (StringInfo str, List *node);
static void outNode(StringInfo, void *node);
static void outQueryBlock (StringInfo str, QueryBlock *node);
static void outConstant (StringInfo str, Constant *node);
static void outFunctionCall (StringInfo str, FunctionCall *node);
static void outSelectItem (StringInfo str, SelectItem *node);
static void writeCommonFromItemFields(StringInfo str, FromItem *node);
static void outFromTableRef (StringInfo str, FromTableRef *node);
static void outFromJoinExpr (StringInfo str, FromJoinExpr *node);
static void outFromSubquery (StringInfo str, FromSubquery *node);
static void outAttributeReference (StringInfo str, AttributeReference *node);
static void outFunctionCall (StringInfo str, FunctionCall *node);
static void outSchema (StringInfo str, Schema *node);
static void outSchemaFromLists(StringInfo str, Schema *node);
static void outAttributeDef (StringInfo str, AttributeDef *node);
static void outSetQuery (StringInfo str, SetQuery *node);
static void outOperator (StringInfo str, Operator *node);
static void indentString(StringInfo str, int level);
static void outQueryOperator(StringInfo str, QueryOperator *node);
static void outProjectionOperator(StringInfo str, ProjectionOperator *node);
static void outSelectionOperator(StringInfo str, SelectionOperator *node);
static void outJoinOperator(StringInfo str, JoinOperator *node);
static void outAggregationOperator(StringInfo str, AggregationOperator *node);
static void outProvenanceComputation(StringInfo str, ProvenanceComputation *node);
static void outTableAccessOperator(StringInfo str, TableAccessOperator *node);
static void outSetOperator(StringInfo str, SetOperator *node);
static void outDuplicateRemoval(StringInfo str, DuplicateRemoval *node);
static void outNestedSubquery(StringInfo str, NestedSubquery *node);
static void outInsert(StringInfo str, Insert *node);
static void outDelete(StringInfo str, Delete *node);
static void outUpdate(StringInfo str, Update *node);
static void outTransactionStmt(StringInfo str, TransactionStmt *node);

/*define macros*/
/*label for the node type*/
#define booltostr(a) \
		((a) ? "true" : "false")

#define WRITE_NODE_TYPE(nodelabel)  \
		appendStringInfoString(str,  CppAsString(nodelabel));

#define CppAsString(token) #token

/* int field*/
#define WRITE_INT_FIELD(fldname)  \
		appendStringInfo(str, ":" CppAsString(fldname) "|%d", node->fldname)

/* long-int field*/
#define WRITE_LONG_FIELD(fldname)  \
		appendStringInfo(str, ":" CppAsString(fldname) "|%ld", node->fldname)

/* char field*/
#define WRITE_CHAR_FIELD(fldname)  \
		appendStringInfo(str, ":" CppAsString(fldname) "|%c", node->fldname)

/* string field*/
#define WRITE_STRING_FIELD(fldname)  \
        appendStringInfo(str, ":" CppAsString(fldname) "|\"%s\"", \
                node->fldname ? node->fldname : "(null)")


/* enum-type field as integer*/
#define WRITE_ENUM_FIELD(fldname, enumtype)  \
		appendStringInfo(str, ":" CppAsString(fldname) "|%d", (int)node->fldname)

/* float field*/
#define WRITE_FLOAT_FIELD(fldname, format)  \
		appendStringInfo(str, ":" CppAsString(fldname) "|" format, node->fldname)

/* bool field*/
#define WRITE_BOOL_FIELD(fldname)  \
		appendStringInfo(str, ":" CppAsString(fldname) "|%s", booltostr(node->fldname))

/* node field*/
#define WRITE_NODE_FIELD(fldname)  \
		do { \
		    appendStringInfoString(str, ":" CppAsString(fldname) "|"); \
            outNode(str, node->fldname); \
        } while(0);

/*node field*/
#define WRITE_STRING_LIST_FIELD(fldname)  \
		do { \
			appendStringInfoString(str, ":" CppAsString(fldname) "|"); \
			outStringList(str, node->fldname); \
		} while(0);

/* write the pointer address of a node used for debugging operator model graphs */
#define WRITE_NODE_ADDRESS() \
        appendStringInfo(str, ":ADDRESS|%p", node)

/* write a field that contains a list of pointers to other nodes */
#define WRITE_POINTER_LIST_FIELD(fldname) \
		do { \
		    appendStringInfoString(str, ":" CppAsString(fldname) "|"); \
		    outPointerList(str, (List *) node->fldname); \
		} while(0)

/* out pointer list */
static void
outPointerList (StringInfo str, List *node)
{
    appendStringInfo(str, "(");

    if (node != NIL)
    {
        FOREACH(void,p,node)
        {
            appendStringInfo(str, "%p", p);
            if (p_his_cell->next)
                appendStringInfoString(str, " ");
        }
    }

    appendStringInfoString(str, ")");
}

/* outNode from node append it to string*/
static void
outList(StringInfo str, List *node)
{
    appendStringInfo(str, "(");

    if (node != NIL)
    {
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
    }

    appendStringInfoString(str, ")");
}

char *
stringListToString (List *node)
{
    StringInfo str = makeStringInfo();

    outStringList(str, node);

    return str->data;
}

static void
outStringList (StringInfo str, List *node)
{
    appendStringInfo(str, "(");

    FOREACH(char,s,node)
    {
        appendStringInfo(str, "\"%s\"", s);
        if (s_his_cell->next)
            appendStringInfoString(str, " ");
    }

    appendStringInfoString(str, ")");
}

static void
outInsert(StringInfo str, Insert *node)
{
    WRITE_NODE_TYPE(INSERT);
    WRITE_STRING_FIELD(tableName);
    WRITE_NODE_FIELD(attrList);
    WRITE_NODE_FIELD(query);
}

static void 
outDelete(StringInfo str, Delete *node)
{
    WRITE_NODE_TYPE(DELETE);
    WRITE_STRING_FIELD(nodeName);
    WRITE_NODE_FIELD(cond);
}

static void 
outUpdate(StringInfo str, Update *node)
{
    WRITE_NODE_TYPE(UPDATE);
    WRITE_STRING_FIELD(nodeName);
    WRITE_NODE_FIELD(selectClause);
    WRITE_NODE_FIELD(cond);
}

static void
outTransactionStmt(StringInfo str, TransactionStmt *node)
{
    WRITE_NODE_TYPE(TRANSACTIONSTMT);
    WRITE_ENUM_FIELD(stmtType, TransactionStmtType);
}

static void
outQueryBlock (StringInfo str, QueryBlock *node)
{
    WRITE_NODE_TYPE(QUERYBLOCK);

    WRITE_NODE_FIELD(distinct);
    WRITE_NODE_FIELD(selectClause);
    WRITE_NODE_FIELD(fromClause);
    WRITE_NODE_FIELD(whereClause);
    WRITE_NODE_FIELD(groupByClause);
    WRITE_NODE_FIELD(havingClause);
    WRITE_NODE_FIELD(orderByClause);
    WRITE_NODE_FIELD(limitClause);
}

static void
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
            appendStringInfo(str, "'%s'", (char *) node->value);
            break;
        case DT_BOOL:
            appendStringInfo(str, "%s", *((boolean *) node->value) == TRUE ? "TRUE" : "FALSE");
            break;
    }
}

static void
outFunctionCall (StringInfo str, FunctionCall *node)
{
    WRITE_NODE_TYPE(FUNCTIONCALL);

    WRITE_STRING_FIELD(functionname);
    WRITE_NODE_FIELD(args);
    WRITE_BOOL_FIELD(isAgg);
}

static void
outSetQuery (StringInfo str, SetQuery *node)
{
    WRITE_NODE_TYPE(SETQUERY);

    WRITE_ENUM_FIELD(setOp, SetOpType);
    WRITE_BOOL_FIELD(all);
    WRITE_NODE_FIELD(selectClause);
    WRITE_NODE_FIELD(lChild);
    WRITE_NODE_FIELD(rChild);
}

static void
outOperator (StringInfo str, Operator *node)
{
    WRITE_NODE_TYPE(OPERATOR);

    WRITE_STRING_FIELD(name);
    WRITE_NODE_FIELD(args);
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
    WRITE_STRING_LIST_FIELD(attrNames);
}

static void
outFromTableRef (StringInfo str, FromTableRef *node)
{
    WRITE_NODE_TYPE(FROM_TABLE_REF);

    writeCommonFromItemFields(str, (FromItem *) node);
    WRITE_STRING_FIELD(tableId);
}

static void
outFromJoinExpr (StringInfo str, FromJoinExpr *node)
{
    WRITE_NODE_TYPE(FROMJOINEXPR);

    writeCommonFromItemFields(str, (FromItem *) node);
    WRITE_NODE_FIELD(left);
    WRITE_NODE_FIELD(right);
    WRITE_ENUM_FIELD(joinType, JoinType);
    WRITE_ENUM_FIELD(joinCond, JoinConditionType);

    // USING is string list
    if (node->joinCond == JOIN_COND_USING)
    {
        appendStringInfoString(str, ":cond|");
        outStringList(str, (List *) node->cond);
    }
    else
        WRITE_NODE_FIELD(cond);
}

static void
outFromSubquery (StringInfo str, FromSubquery *node)
{
    WRITE_NODE_TYPE(FROMSUBQUERY);

    writeCommonFromItemFields(str, (FromItem *) node);
    WRITE_NODE_FIELD(subquery);
}

static void
outNestedSubquery (StringInfo str, NestedSubquery *node)
{
    WRITE_NODE_TYPE(NESTEDSUBQUERY);

    WRITE_ENUM_FIELD(nestingType, NestingExprType);
    WRITE_NODE_FIELD(expr);
    WRITE_STRING_FIELD(comparisonOp);
    WRITE_NODE_FIELD(query);
}

static void
outAttributeReference (StringInfo str, AttributeReference *node)
{
    WRITE_NODE_TYPE(ATTRIBUTE_REFERENCE);

    WRITE_STRING_FIELD(name);
    WRITE_INT_FIELD(fromClauseItem);
    WRITE_INT_FIELD(attrPosition);
}

static void 
outSchema (StringInfo str, Schema *node)
{
    WRITE_NODE_TYPE(SCHEMA);

    WRITE_STRING_FIELD(name);
    WRITE_NODE_FIELD(attrDefs);
}

//static void
//outSchemaFromLists (StringInfo str, Schema *node)
//{
//    WRITE_NODE_TYPE(SCHEMA);
//
//    WRITE_NODE_FIELD(name);
//    WRITE_NODE_FIELD(attrDefs);
//}

static void
outAttributeDef (StringInfo str, AttributeDef *node)
{
    WRITE_NODE_TYPE(ATTRIBUTE_DEF);

    WRITE_ENUM_FIELD(dataType, DataType);
    WRITE_STRING_FIELD(attrName);
    WRITE_INT_FIELD(pos); 
}

#define WRITE_QUERY_OPERATOR() outQueryOperator(str, (QueryOperator *) node)

static void
outQueryOperator (StringInfo str, QueryOperator *node)
{
    WRITE_NODE_ADDRESS();
    WRITE_POINTER_LIST_FIELD(parents);
    WRITE_NODE_FIELD(schema);
    WRITE_NODE_FIELD(provAttrs);
    WRITE_NODE_FIELD(inputs);
}

static void 
outProjectionOperator(StringInfo str, ProjectionOperator *node)
{
    WRITE_NODE_TYPE(PROJECTION_OPERATOR);
    WRITE_QUERY_OPERATOR();
    
    WRITE_NODE_FIELD(projExprs); // projection expressions, Expression type
}
static void 
outSelectionOperator (StringInfo str, SelectionOperator *node)
{
    WRITE_NODE_TYPE(SELECTION_OPERATOR);
    WRITE_QUERY_OPERATOR();

    WRITE_NODE_FIELD(cond); //  condition expression, Expr type
}

static void 
outJoinOperator(StringInfo str, JoinOperator *node)
{
    WRITE_NODE_TYPE(JOIN_OPERATOR);
    WRITE_QUERY_OPERATOR();

    WRITE_ENUM_FIELD(joinType, JoinType);
    WRITE_NODE_FIELD(cond);
}

static void 
outAggregationOperator(StringInfo str, AggregationOperator *node)
{
    WRITE_NODE_TYPE(AGGREGATION_OPERATOR);
    WRITE_QUERY_OPERATOR();

    WRITE_NODE_FIELD(aggrs);
    WRITE_NODE_FIELD(groupBy);
}

static void 
outProvenanceComputation(StringInfo str, ProvenanceComputation *node)
{
    WRITE_NODE_TYPE(PROVENANCE_COMPUTATION);
    WRITE_QUERY_OPERATOR();

    WRITE_ENUM_FIELD(provType,ProvenanceType);
}

static void outTableAccessOperator(StringInfo str, TableAccessOperator *node)
{
    WRITE_NODE_TYPE(TABLE_ACCESS_OPERATOR);
    WRITE_QUERY_OPERATOR();

    WRITE_STRING_FIELD(tableName);
}

static void outSetOperator(StringInfo str, SetOperator *node)
{
    WRITE_NODE_TYPE(SET_OPERATOR);
    WRITE_QUERY_OPERATOR();

    WRITE_ENUM_FIELD(setOpType,SetOpType);
}

static void outDuplicateRemoval(StringInfo str, DuplicateRemoval *node)
{
    WRITE_NODE_TYPE(DUPLICATE_REMOVAL);
    WRITE_QUERY_OPERATOR();

    WRITE_NODE_FIELD(attrs); // attributes that need duplicate removal, AttributeReference type

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
            case T_Operator:
                outOperator(str, (Operator *) obj);
                break;
            case T_SetQuery:
                outSetQuery (str, (SetQuery *) obj);
                break;
            case T_FromTableRef:
                outFromTableRef(str, (FromTableRef *) obj);
                break;
            case T_FromJoinExpr:
                outFromJoinExpr(str, (FromJoinExpr *) obj);
                break;
            case T_FromSubquery:
                outFromSubquery(str, (FromSubquery*) obj);
                break;
            case T_NestedSubquery:
                outNestedSubquery(str, (NestedSubquery*) obj);
                break;
            case T_AttributeReference:
                outAttributeReference(str, (AttributeReference *) obj);
                break;
            case T_FunctionCall:
                outFunctionCall(str, (FunctionCall *) obj);
                break;
            case T_Schema:
                outSchema(str, (Schema *) obj);
                break;
            case T_AttributeDef:
                outAttributeDef(str, (AttributeDef *) obj);
                break;
            case T_Insert:
                outInsert(str, (Insert *) obj);
                break;
            case T_Delete:
                outDelete(str, (Delete *) obj);
                break;
            case T_Update:
                outUpdate(str, (Update *) obj);
                break;
            case T_TransactionStmt:
                outTransactionStmt(str, (TransactionStmt *) obj);
                break;

            //query operator model nodes
            case T_QueryOperator:
                outQueryOperator(str, (QueryOperator *) obj);
                break;
            case T_SelectionOperator:
                outSelectionOperator(str, (SelectionOperator *) obj);
                break;
            case T_ProjectionOperator:
                outProjectionOperator(str, (ProjectionOperator *) obj);
                break;
            case T_JoinOperator:
                outJoinOperator(str, (JoinOperator *) obj);
                break;
            case T_AggregationOperator:
                outAggregationOperator(str, (AggregationOperator *) obj);
                break;
            case T_ProvenanceComputation:
                outProvenanceComputation(str, (ProvenanceComputation *) obj);
                break;
            case T_TableAccessOperator:
                outTableAccessOperator(str, (TableAccessOperator *) obj);
                break;
            case T_SetOperator:
                outSetOperator(str, (SetOperator *) obj);
                break;
            case T_DuplicateRemoval:
                outDuplicateRemoval(str, (DuplicateRemoval *) obj);
                break;
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
    boolean inStringConst = FALSE;

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
        else if (inStringConst)
        {
            switch(c)
            {
                case '\'':
                {
                    inStringConst = FALSE;
                    appendStringInfoChar (str, c);
                }
                break;
                case '\\':
                {
                    if ((c + 1) == '\'')
                    {
                        c++;
                        appendStringInfoString(str, "\\'");
                    }
                    else
                        appendStringInfoChar (str, c);
                }
                break;
                default:
                    appendStringInfoChar (str, c);
            }
        }
        else
        {
            switch (c)
            {
                case '(':
                    indentation++;
                    appendStringInfoString(str, "(");
                    if (input[1] == '"') // string list
                    {
                        appendStringInfoChar(str, '\n');
                        indentString(str, indentation);
                    }
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
                    appendStringInfoChar (str, c);
                    break;
                case '\'':
                    inStringConst = TRUE;
                    appendStringInfoChar (str, c);
                    break;
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

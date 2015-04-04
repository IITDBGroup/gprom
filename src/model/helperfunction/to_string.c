/*************************************
 *         to_string.c
 *    Author: Hao Guo
 * Implement a function that given a query
 * tree or expression tree that returns a
 * string representing the tree.
 *
 **************************************/


#include "common.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/set/vector.h"
#include "model/set/hashmap.h"
#include "model/expression/expression.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/datalog/datalog_model.h"
#include "utility/string_utils.h"

/* functions to output specific node types */
static void outNode(StringInfo, void *node);

// collection types
static void outPointerList (StringInfo str, List *node);
static void outList(StringInfo str, List *node);
static void outStringList (StringInfo str, List *node);
static void outSet(StringInfo str, Set *node);
static void outVector(StringInfo str, Vector *node);
static void outHashMap(StringInfo str, HashMap *node);

// expression types
static void outConstant (StringInfo str, Constant *node);
static void outFunctionCall (StringInfo str, FunctionCall *node);
static void outAttributeReference (StringInfo str, AttributeReference *node);
static void outSQLParameter (StringInfo str, SQLParameter *node);
static void outOperator (StringInfo str, Operator *node);
static void outKeyValue (StringInfo str, KeyValue *node);
static void outCaseExpr (StringInfo str, CaseExpr *node);
static void outCaseWhen (StringInfo str, CaseWhen *node);
static void outIsNullExpr (StringInfo str, IsNullExpr *node);
static void outWindowBound (StringInfo str, WindowBound *node);
static void outWindowFrame (StringInfo str, WindowFrame *node);
static void outWindowDef (StringInfo str, WindowDef *node);
static void outWindowFunction (StringInfo str, WindowFunction *node);
static void outRowNumExpr (StringInfo str, RowNumExpr *node);
static void outOrderExpr (StringInfo str, OrderExpr *node);
static void outCastExpr (StringInfo str, CastExpr *node);

// query block model
static void outQueryBlock (StringInfo str, QueryBlock *node);
static void outSetQuery (StringInfo str, SetQuery *node);
static void outProvenanceStmt (StringInfo str, ProvenanceStmt *node);
static void outProvenanceTransactionInfo (StringInfo str,
        ProvenanceTransactionInfo *node);
static void outInsert(StringInfo str, Insert *node);
static void outDelete(StringInfo str, Delete *node);
static void outUpdate(StringInfo str, Update *node);
static void outNestedSubquery(StringInfo str, NestedSubquery *node);
static void outTransactionStmt(StringInfo str, TransactionStmt *node);
static void outWithStmt(StringInfo str, WithStmt *node);

static void outSelectItem (StringInfo str, SelectItem *node);
static void writeCommonFromItemFields(StringInfo str, FromItem *node);
static void outDistinctClause(StringInfo str, DistinctClause *node);
static void outFromProvInfo (StringInfo str, FromProvInfo *node);
static void outFromTableRef (StringInfo str, FromTableRef *node);
static void outFromJoinExpr (StringInfo str, FromJoinExpr *node);
static void outFromSubquery (StringInfo str, FromSubquery *node);

// operator model
static void outSchema (StringInfo str, Schema *node);
static void outAttributeDef (StringInfo str, AttributeDef *node);
static void outQueryOperator(StringInfo str, QueryOperator *node);
static void outProjectionOperator(StringInfo str, ProjectionOperator *node);
static void outSelectionOperator(StringInfo str, SelectionOperator *node);
static void outJoinOperator(StringInfo str, JoinOperator *node);
static void outAggregationOperator(StringInfo str, AggregationOperator *node);
static void outProvenanceComputation(StringInfo str, ProvenanceComputation *node);
static void outTableAccessOperator(StringInfo str, TableAccessOperator *node);
static void outSetOperator(StringInfo str, SetOperator *node);
static void outDuplicateRemoval(StringInfo str, DuplicateRemoval *node);
static void outConstRelOperator(StringInfo str, ConstRelOperator *node);
static void outNestingOperator(StringInfo str, NestingOperator *node);
static void outWindowOperator(StringInfo str, WindowOperator *node);
static void outOrderOperator(StringInfo str, OrderOperator *node);

//json
static void outFromJsonTable(StringInfo str, FromJsonTable *node);
static void outFromJsonColInfoItem(StringInfo str, JsonColInfoItem *node);
static void outJsonTableOperator(StringInfo str, JsonTableOperator *node);
static void outJsonPath(StringInfo str, JsonPath *node);

// datalog model
static void outDLAtom(StringInfo str, DLAtom *node);
static void outDLVar(StringInfo str, DLVar *node);
static void outDLRule(StringInfo str, DLRule *node);
static void outDLProgram(StringInfo str, DLProgram *node);
static void outDLComparison(StringInfo str, DLComparison *node);

static void indentString(StringInfo str, int level);

// create overview string for an operator tree
static int compareOpInfos (const void *l, const void *r);
static void operatorToOverviewInternal(StringInfo str, QueryOperator *op, int indent, HashMap *map);
static void datalogToStrInternal(StringInfo str, Node *n, int indent);


/*define macros*/
#define OP_ID_STRING "OP_ID"

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
		appendStringInfo(str, ":" CppAsString(fldname) "|%s - %d", enumtype ## ToString(node->fldname), (int)node->fldname)

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
outSet(StringInfo str, Set *node)
{
//    appendStringInfo(str, "{");

    switch(node->setType)
    {
        case SET_TYPE_INT:
            FOREACH_SET(int,i,node)
                appendStringInfo(str, "%d%s", *i, i_his_el->hh.next ? ", " : "");
            break;
        case SET_TYPE_STRING:
            FOREACH_SET(char,el,node)
                appendStringInfo(str, "%s%s", el, el_his_el->hh.next ? ", " : "");
            break;
        case SET_TYPE_POINTER:
            FOREACH_SET(void,el,node)
                appendStringInfo(str, "%p%s", el, el_his_el->hh.next ? ", " : "");
            break;
        case SET_TYPE_NODE:
            FOREACH_SET(void,el,node)
            {
                outNode(str, el);
                appendStringInfo(str, "%s", el_his_el->hh.next ? ", " : "");
            }
            break;
        default:
            FATAL_LOG("not implemented yet");
            break;
    }

//    appendStringInfo(str, "}");
}

static void
outVector(StringInfo str, Vector *node)
{
    appendStringInfo(str, "[");

    switch(node->elType)
    {
        case VECTOR_INT:
        {
            int j = 0;
            FOREACH_VEC_INT(i,node)
            {
                appendStringInfo(str, "%s", itoa(*i));
                appendStringInfo(str, "%s", VEC_LENGTH(node) > ++j ? ", " : "");
            }
        }
            break;
        case VECTOR_NODE:
            FOREACH_VEC(Node,n,node)
            {
                outNode(str, *n);
                appendStringInfo(str, "%s", VEC_IS_LAST(*n,node) ? ", " : "");
            }
            break;
        case VECTOR_STRING:
            FATAL_LOG("TODO");
            break;
    }

    appendStringInfo(str, "]");
}

static void
outHashMap(StringInfo str, HashMap *node)
{
    List *entryStrings = NIL;
    List *sortEntries = NIL;

    appendStringInfo(str, "{");

    // create list of serializations for each hash entry
    FOREACH_HASH_ENTRY(el,node)
    {
        StringInfo hashStr = makeStringInfo();
        outNode(hashStr, el->key);
        appendStringInfoString(hashStr," => ");
        outNode(hashStr, el->value);
        entryStrings = appendToTailOfList(entryStrings, strdup(hashStr->data));
    }

    // sort entries lexigraphically (deterministic output)
    sortEntries = sortList(entryStrings,
            (int (*) (const void *, const void *)) strCompare);

    // append entries to output
    FOREACH(char,s,sortEntries)
    {
        appendStringInfoString(str,s);
        appendStringInfo(str, "%s", s_his_cell->next ? ", " : "");
    }

    appendStringInfo(str, "}");
}

// datalog model
static void
outDLAtom(StringInfo str, DLAtom *node)
{
    WRITE_NODE_TYPE(DLATOM);

    WRITE_STRING_FIELD(rel);
    WRITE_NODE_FIELD(args);
    WRITE_BOOL_FIELD(negated);
    WRITE_NODE_FIELD(n.properties);
}

static void
outDLVar(StringInfo str, DLVar *node)
{
    WRITE_NODE_TYPE(DLVAR);

    WRITE_STRING_FIELD(name);
    WRITE_ENUM_FIELD(dt,DataType);
    WRITE_NODE_FIELD(n.properties);
}

static void
outDLRule(StringInfo str, DLRule *node)
{
    WRITE_NODE_TYPE(DLRULE);

    WRITE_NODE_FIELD(head);
    WRITE_NODE_FIELD(body);
    WRITE_NODE_FIELD(n.properties);
}

static void
outDLProgram(StringInfo str, DLProgram *node)
{
    WRITE_NODE_TYPE(DLPROGRAM);

    WRITE_NODE_FIELD(rules);
    WRITE_NODE_FIELD(facts);
    WRITE_STRING_FIELD(ans);
    WRITE_NODE_FIELD(n.properties);
}

static void
outDLComparison(StringInfo str, DLComparison *node)
{
    WRITE_NODE_TYPE(DLCOMPARISON);

    WRITE_NODE_FIELD(opExpr);
    WRITE_NODE_FIELD(n.properties);
}

static void
outFromJsonTable(StringInfo str, FromJsonTable *node)
{
    WRITE_NODE_TYPE(JSONTABLE);
    writeCommonFromItemFields(str, (FromItem *) node);
    WRITE_NODE_FIELD(columns);
    WRITE_STRING_FIELD(documentcontext);
    WRITE_NODE_FIELD(jsonColumn);
    WRITE_STRING_FIELD(jsonTableIdentifier);
}

static void
outFromJsonColInfoItem(StringInfo str, JsonColInfoItem *node)
{
	WRITE_NODE_TYPE(JSONCOLINFOITEM);

    WRITE_STRING_FIELD(attrName);
    WRITE_STRING_FIELD(path);
    WRITE_STRING_FIELD(attrType);

    WRITE_STRING_FIELD(format);
    WRITE_STRING_FIELD(wrapper);
    WRITE_NODE_FIELD(nested);
    WRITE_STRING_FIELD(forOrdinality);
}

static void
outInsert(StringInfo str, Insert *node)
{
    WRITE_NODE_TYPE(INSERT);
    WRITE_STRING_FIELD(tableName);
    WRITE_STRING_LIST_FIELD(attrList);
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
outWithStmt(StringInfo str, WithStmt *node)
{
    WRITE_NODE_TYPE(WITH_STMT);
    WRITE_NODE_FIELD(withViews);
    WRITE_NODE_FIELD(query);
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

    if (node->isNull)
        appendStringInfoString(str, "NULL");
    else
        switch(node->constType)
        {
            case DT_INT:
                appendStringInfo(str, "%d", *((int *) node->value));
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
            case DT_LONG:
                appendStringInfo(str, "%ld", *((long *) node->value));
                break;
            case DT_VARCHAR2:
	        appendStringInfo(str, "'%s'", (char *) node->value);
	        break;
        }

    WRITE_BOOL_FIELD(isNull);
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
    WRITE_STRING_LIST_FIELD(selectClause);
    WRITE_NODE_FIELD(lChild);
    WRITE_NODE_FIELD(rChild);
}

static void
outProvenanceStmt (StringInfo str, ProvenanceStmt *node)
{
    WRITE_NODE_TYPE(PROVENANCESTMT);

    WRITE_NODE_FIELD(query);
    WRITE_STRING_LIST_FIELD(selectClause);
    WRITE_ENUM_FIELD(provType,ProvenanceType);
    WRITE_ENUM_FIELD(inputType,ProvenanceInputType);
    WRITE_NODE_FIELD(transInfo);
    WRITE_NODE_FIELD(asOf);
    WRITE_NODE_FIELD(options);
}

static void
outProvenanceTransactionInfo (StringInfo str, ProvenanceTransactionInfo *node)
{
    WRITE_NODE_TYPE(PROVENANCETRANSACTIONINFO);

    WRITE_ENUM_FIELD(transIsolation, IsolationLevel);
    WRITE_STRING_LIST_FIELD(updateTableNames);
    WRITE_NODE_FIELD(originalUpdates);
    WRITE_NODE_FIELD(scns);
    WRITE_NODE_FIELD(commitSCN);
}

static void
outOperator (StringInfo str, Operator *node)
{
    WRITE_NODE_TYPE(OPERATOR);

    WRITE_STRING_FIELD(name);
    WRITE_NODE_FIELD(args);
}

static void
outKeyValue (StringInfo str, KeyValue *node)
{
    WRITE_NODE_TYPE(KEYVALUE);

    WRITE_NODE_FIELD(key);
    WRITE_NODE_FIELD(value);
}

static void
outCaseExpr (StringInfo str, CaseExpr *node)
{
    WRITE_NODE_TYPE(CASE_EXPR);

    WRITE_NODE_FIELD(expr);
    WRITE_NODE_FIELD(whenClauses);
    WRITE_NODE_FIELD(elseRes);
}

static void
outCaseWhen (StringInfo str, CaseWhen *node)
{
    WRITE_NODE_TYPE(CASE_WHEN);

    WRITE_NODE_FIELD(when);
    WRITE_NODE_FIELD(then);
}

static void
outIsNullExpr (StringInfo str, IsNullExpr *node)
{
    WRITE_NODE_TYPE(IS_NULL_EXPR);

    WRITE_NODE_FIELD(expr);
}

static void
outWindowBound (StringInfo str, WindowBound *node)
{
    WRITE_NODE_TYPE(WINDOW_BOUND);

    WRITE_ENUM_FIELD(bType, WindowBoundType);
    WRITE_NODE_FIELD(expr);
}

static void
outWindowFrame (StringInfo str, WindowFrame *node)
{
    WRITE_NODE_TYPE(WINDOW_FRAME);

    WRITE_ENUM_FIELD(frameType, WinFrameType);
    WRITE_NODE_FIELD(lower);
    WRITE_NODE_FIELD(higher);
}

static void
outWindowDef (StringInfo str, WindowDef *node)
{
    WRITE_NODE_TYPE(WINDOW_DEF);

    WRITE_NODE_FIELD(partitionBy);
    WRITE_NODE_FIELD(orderBy);
    WRITE_NODE_FIELD(frame);
}

static void
outWindowFunction (StringInfo str, WindowFunction *node)
{
    WRITE_NODE_TYPE(WINDOW_FUNCTION);

    WRITE_NODE_FIELD(f);
    WRITE_NODE_FIELD(win);
}

static void
outRowNumExpr (StringInfo str, RowNumExpr *node)
{
    WRITE_NODE_TYPE(ROWNUMEXPR);
}

static void
outOrderExpr (StringInfo str, OrderExpr *node)
{
    WRITE_NODE_TYPE(ORDER_EXPR);

    WRITE_NODE_FIELD(expr);
    WRITE_ENUM_FIELD(order,SortOrder);
    WRITE_ENUM_FIELD(nullOrder,SortNullOrder);
}

static void
outCastExpr (StringInfo str, CastExpr *node)
{
    WRITE_NODE_TYPE(CAST_EXPR);

    WRITE_ENUM_FIELD(resultDT,DataType);
    WRITE_NODE_FIELD(expr);
}

static void
outSelectItem (StringInfo str, SelectItem *node)
{
    WRITE_NODE_TYPE(SELECT_ITEM);

    WRITE_STRING_FIELD(alias);
    WRITE_NODE_FIELD(expr);
}

static void
outDistinctClause(StringInfo str, DistinctClause *node)
{
    WRITE_NODE_TYPE(DISTINCT_CLAUSE);

    WRITE_NODE_FIELD(distinctExprs);
}

static void
writeCommonFromItemFields(StringInfo str, FromItem *node)
{
    WRITE_STRING_FIELD(name);
    WRITE_STRING_LIST_FIELD(attrNames);
    WRITE_NODE_FIELD(provInfo);
    WRITE_NODE_FIELD(dataTypes);
}

static void
outFromProvInfo (StringInfo str, FromProvInfo *node)
{
    WRITE_NODE_TYPE(FROMPROVINFO);

    WRITE_BOOL_FIELD(baserel);
    WRITE_BOOL_FIELD(intermediateProv);
    WRITE_STRING_LIST_FIELD(userProvAttrs);
}

static void
outFromTableRef (StringInfo str, FromTableRef *node)
{
    WRITE_NODE_TYPE(FROMTABLEREF);

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
    WRITE_INT_FIELD(outerLevelsUp);
    WRITE_ENUM_FIELD(attrType,DataType);
}

static void
outSQLParameter (StringInfo str, SQLParameter *node)
{
    WRITE_NODE_TYPE(SQL_PARAMETER);

    WRITE_STRING_FIELD(name);
    WRITE_INT_FIELD(position);
    WRITE_ENUM_FIELD(parType, DataType);
}

static void 
outSchema (StringInfo str, Schema *node)
{
    WRITE_NODE_TYPE(SCHEMA);

    WRITE_STRING_FIELD(name);
    WRITE_NODE_FIELD(attrDefs);
}

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
    WRITE_NODE_FIELD(properties);
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
    WRITE_ENUM_FIELD(inputType,ProvenanceInputType);
    WRITE_NODE_FIELD(transactionInfo);
    WRITE_NODE_FIELD(asOf);
}

static void
outTableAccessOperator(StringInfo str, TableAccessOperator *node)
{
    WRITE_NODE_TYPE(TABLE_ACCESS_OPERATOR);
    WRITE_QUERY_OPERATOR();

    WRITE_NODE_FIELD(asOf);
    WRITE_STRING_FIELD(tableName);
}

static void
outSetOperator(StringInfo str, SetOperator *node)
{
    WRITE_NODE_TYPE(SET_OPERATOR);
    WRITE_QUERY_OPERATOR();

    WRITE_ENUM_FIELD(setOpType,SetOpType);
}

static void
outDuplicateRemoval(StringInfo str, DuplicateRemoval *node)
{
    WRITE_NODE_TYPE(DUPLICATE_REMOVAL);
    WRITE_QUERY_OPERATOR();

    WRITE_NODE_FIELD(attrs); // attributes that need duplicate removal, AttributeReference type

}

static void
outConstRelOperator(StringInfo str, ConstRelOperator *node)
{
	WRITE_NODE_TYPE(CONST_REL_OPERATOR);
	WRITE_QUERY_OPERATOR();

	WRITE_NODE_FIELD(values);
}

static void
outNestingOperator(StringInfo str, NestingOperator *node)
{
	WRITE_NODE_TYPE(NESTING_OPERATOR);
	WRITE_QUERY_OPERATOR();

	WRITE_ENUM_FIELD(nestingType,NestingExprType);
	WRITE_NODE_FIELD(cond);
}

static void
outWindowOperator(StringInfo str, WindowOperator *node)
{
    WRITE_NODE_TYPE(WINDOW_OPERATOR);
    WRITE_QUERY_OPERATOR();

    WRITE_NODE_FIELD(partitionBy);
    WRITE_NODE_FIELD(orderBy);
    WRITE_NODE_FIELD(frameDef);
    WRITE_STRING_FIELD(attrName);
    WRITE_NODE_FIELD(f);
}

static void
outOrderOperator(StringInfo str, OrderOperator *node)
{
    WRITE_NODE_TYPE(ORDER_OPERATOR);
    WRITE_QUERY_OPERATOR();

    WRITE_NODE_FIELD(orderExprs);
}

static void
outJsonTableOperator(StringInfo str, JsonTableOperator *node)
{
    WRITE_NODE_TYPE(JSONTABLEOPERATOR);

    WRITE_QUERY_OPERATOR();
    WRITE_NODE_FIELD(columns);
    WRITE_STRING_FIELD(documentcontext);
    WRITE_NODE_FIELD(jsonColumn);
    WRITE_STRING_FIELD(jsonTableIdentifier);
}

static void
outJsonPath(StringInfo str, JsonPath *node)
{
	 WRITE_NODE_TYPE(JSONPATH);

	 WRITE_STRING_FIELD(path);
}

void
outNode(StringInfo str, void *obj)
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
            case T_Set:
                outSet(str, (Set *) obj);
                break;
            case T_Vector:
                outVector(str, (Vector *) obj);
                break;
            case T_HashMap:
                outHashMap(str, (HashMap *) obj);
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
            case T_KeyValue:
                outKeyValue(str, (KeyValue *) obj);
                break;
            case T_CaseExpr:
                outCaseExpr(str, (CaseExpr *) obj);
                break;
            case T_CaseWhen:
                outCaseWhen(str, (CaseWhen *) obj);
                break;
            case T_IsNullExpr:
                outIsNullExpr(str, (IsNullExpr *) obj);
                break;
            case T_WindowBound:
                outWindowBound(str, (WindowBound *) obj);
                break;
            case T_WindowFrame:
                outWindowFrame(str, (WindowFrame *) obj);
                break;
            case T_WindowDef:
                outWindowDef(str, (WindowDef *) obj);
                break;
            case T_WindowFunction:
                outWindowFunction(str, (WindowFunction *) obj);
                break;
            case T_RowNumExpr:
                outRowNumExpr(str, (RowNumExpr *) obj);
                break;
            case T_OrderExpr:
                outOrderExpr(str, (OrderExpr *) obj);
                break;
            case T_CastExpr:
                outCastExpr(str, (CastExpr *) obj);
                break;
            case T_SetQuery:
                outSetQuery (str, (SetQuery *) obj);
                break;
            case T_ProvenanceStmt:
                outProvenanceStmt (str, (ProvenanceStmt *) obj);
                break;
            case T_ProvenanceTransactionInfo:
                outProvenanceTransactionInfo (str, (ProvenanceTransactionInfo *) obj);
                break;
            case T_DistinctClause:
                outDistinctClause (str, (DistinctClause *) obj);
                break;
            case T_FromProvInfo:
                outFromProvInfo(str, (FromProvInfo *) obj);
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
            case T_SQLParameter:
                outSQLParameter(str, (SQLParameter *) obj);
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
            case T_WithStmt:
                outWithStmt(str, (WithStmt *) obj);
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
            case T_ConstRelOperator:
            	outConstRelOperator(str, (ConstRelOperator *) obj);
            	break;
            case T_NestingOperator:
            	outNestingOperator(str, (NestingOperator *) obj);
            	break;
            case T_WindowOperator:
                outWindowOperator(str, (WindowOperator *) obj);
                break;
            case T_OrderOperator:
                outOrderOperator(str, (OrderOperator *) obj);
                break;
            /* datalog stuff */
            case T_DLAtom:
                outDLAtom(str, (DLAtom *) obj);
                break;
            case T_DLVar:
                outDLVar(str, (DLVar *) obj);
                break;
            case T_DLProgram:
                outDLProgram(str, (DLProgram *) obj);
                break;
            case T_DLRule:
                outDLRule(str, (DLRule *) obj);
                break;
            case T_DLComparison:
                outDLComparison(str, (DLComparison *) obj);
                break;
            /* Json stuff */
            case T_FromJsonTable:
                outFromJsonTable(str, (FromJsonTable *)obj);
                break;
            case T_JsonColInfoItem:
                outFromJsonColInfoItem(str, (JsonColInfoItem *)obj);
                break;
            case T_JsonTableOperator:
                outJsonTableOperator(str, (JsonTableOperator *) obj);
                break;
            case T_JsonPath:
            	outJsonPath(str, (JsonPath *) obj);
            	break;
            default :
                FATAL_LOG("do not know how to output node of type %d", nodeTag(obj));
                //outNode(str, obj);
                break;
        }
        appendStringInfoString(str, "}");
    }
}

/*nodeToString return the node as string*/

char *
nodeToString(void *obj)
{
    StringInfo str;
    char *result;

    str = makeStringInfo();
    outNode(str, obj);
    result = str->data;
    TRACE_LOG("output is of length <%u> of <%u>", str->len, str->maxlen);
    FREE(str);

    return result;
}

//int
//hashObject(void *a)
//{
//    StringInfo str;
//    int hash;
//
//    str = makeStringInfo();
//    outNode(str, obj);
//
//}

/*
 *
 */
char *
beatify(char *input)
{
    StringInfo str = makeStringInfo();
    char *result;
    int indentation = 0;
    boolean inString = FALSE;
    boolean inStringConst = FALSE;

    TRACE_LOG("beatify string of len <%u>\n%s", strlen(input), input);

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
                    break;
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

char *
itoa(int value)
{
    StringInfo str = makeStringInfo();
    char *result;

    appendStringInfo(str, "%d", value);
    result = str->data;
    FREE(str);

    return result;
}

char *
datalogToOverviewString(Node *n)
{
    StringInfo str = makeStringInfo();

    if (n == NULL)
        return "";

    datalogToStrInternal(str, n, 0);

    return str->data;
}

static void
datalogToStrInternal(StringInfo str, Node *n, int indent)
{
    if (n == NULL)
        return;

    switch(n->type)
    {
        case T_DLAtom:
        {
            DLAtom *a = (DLAtom *) n;
            int i = 1;
            int len = LIST_LENGTH(a->args);

            if (DL_HAS_PROP(a,DL_WON))
                appendStringInfoString(str, "+");
            if (DL_HAS_PROP(a,DL_LOST))
                appendStringInfoString(str, "-");
            if (DL_HAS_PROP(a,DL_UNDER_NEG_WON))
                appendStringInfoString(str, "*+");
            if (DL_HAS_PROP(a,DL_UNDER_NEG_LOST))
                appendStringInfoString(str, "*-");

            if (DL_HAS_PROP(a,DL_IS_IDB_REL))
                appendStringInfoString(str, "@");

            if (a->negated)
                appendStringInfoString(str, "not ");
            appendStringInfo(str, "%s(", a->rel);
            FOREACH(Node,arg,a->args)
            {
                datalogToStrInternal(str, arg, indent);
                if (i++ < len)
                    appendStringInfoString(str, ",");
            }
            appendStringInfoString(str, ")");
        }
        break;
        case T_DLRule:
        {
            DLRule *r = (DLRule *) n;
            int i = 1;
            int len = LIST_LENGTH(r->body);

            indentString(str,indent);
            // add rule id if set
            if (DL_HAS_PROP(r,DL_RULE_ID))
                appendStringInfo(str, "r%u: ",
                        INT_VALUE(DL_GET_PROP(r,DL_RULE_ID)));
            datalogToStrInternal(str, (Node *) r->head, indent);
            appendStringInfoString(str, " :- ");
            FOREACH(Node,a,r->body)
            {
                datalogToStrInternal(str, a, indent);
                if (i++ < len)
                    appendStringInfoString(str, ",");
            }

            appendStringInfoString(str, ".\n");
        }
        break;
        case T_DLComparison:
        {
            DLComparison *c = (DLComparison *) n;

            appendStringInfo(str, "(%s)", exprToSQL((Node *) c->opExpr));
        }
        break;
        case T_DLVar:
        {
            DLVar *v = (DLVar *) n;

            appendStringInfo(str, "%s", v->name);
        }
        break;
        case T_DLProgram:
        {
            DLProgram *p = (DLProgram *) n;
            appendStringInfoString(str, "PROGRAM:\n");
            FOREACH(Node,r,p->rules)
            {
                if (isA(r,Constant))
                    appendStringInfoString(str, "ANSWER RELATION:\n\t");
                datalogToStrInternal(str,(Node *) r, 4);
            }
            if (p->ans)
                appendStringInfo(str, "ANSWER RELATION:\n\t%s\n",
                        p->ans);
            if (DL_HAS_PROP(p,DL_PROV_WHY) || DL_HAS_PROP(p,DL_PROV_WHYNOT))
            {
                char *prop = DL_HAS_PROP(p,DL_PROV_WHY) ? DL_PROV_WHY : DL_PROV_WHYNOT;
                Node *question = DL_GET_PROP(p,prop);

                appendStringInfo(str, "%s:\n\t", prop);
                datalogToStrInternal(str,(Node *) question, 4);
            }
        }
        break;
        case T_Constant:
            appendStringInfo(str, "%s",
                    CONST_TO_STRING(n));
        break;
        // provenance
        case T_KeyValue:
        {
            KeyValue *kv = (KeyValue *) n;
            appendStringInfo(str, "COMPUTE PROVENANCE: %s[%s]",
                    STRING_VALUE(kv->key), datalogToOverviewString(kv->value));
        }
        break;
        case T_List:
        {
            List *l = (List *) n;
            FOREACH(Node,el,l)
                datalogToStrInternal(str,el, indent + 4);
        }
        break;
        default:
        {
            if (IS_EXPR(n))
                appendStringInfo(str, "%s", exprToSQL(n));
            else
                FATAL_LOG("should have never come here, datalog program should"
                        " not have nodes like this: %s",
                        beatify(nodeToString(n)));
        }
        break;
    }
}

char *
operatorToOverviewString(Node *op)
{
    StringInfo str = makeStringInfo();
    HashMap *m;
    List *reusedSubtrees = NIL;

    if (op == NULL)
        return "";

    TRACE_LOG("input was:\n%s", nodeToString(op));

    if (isA(op,List))
    {
        FOREACH(QueryOperator,o,(List *) op)
        {
            m = NEW_MAP(Constant,List);
            MAP_ADD_STRING_KEY(m, OP_ID_STRING, createConstInt(0));
            operatorToOverviewInternal(str,(QueryOperator *) o, 0, m);

            removeMapElem(m, (Node *) createConstString(OP_ID_STRING));
            reusedSubtrees = sortList(getEntries(m), (int (*)(const void *, const void *))compareOpInfos);

            FOREACH(KeyValue,k,reusedSubtrees)
            {
                List *opInfo = (List *) k->value;
                int opId = INT_VALUE(getNthOfListP(opInfo, 0));
                StringInfo inner = (StringInfo) LONG_VALUE(getNthOfListP(opInfo, 1));

                appendStringInfo(str, "\n\n-----------------------\n@%u\n%s", opId, inner->data);
            }

            appendStringInfoString(str, "\n");
        }
    }
    else
    {
        m = NEW_MAP(Constant,List);
        MAP_ADD_STRING_KEY(m, OP_ID_STRING, createConstInt(0));

        operatorToOverviewInternal(str,(QueryOperator *) op, 0, m);

        removeMapElem(m, (Node *) createConstString(OP_ID_STRING));
        reusedSubtrees = sortList(getEntries(m), (int (*)(const void *, const void *))compareOpInfos);

        FOREACH(KeyValue,k,reusedSubtrees)
        {
            List *opInfo = (List *) k->value;
            int opId = INT_VALUE(getNthOfListP(opInfo, 0));
            StringInfo inner = (StringInfo) LONG_VALUE(getNthOfListP(opInfo, 1));

            appendStringInfo(str, "\n\n-----------------------\n@%u\n%s", opId, inner->data);
        }
    }

    return str->data;
}

static int
compareOpInfos (const void *l, const void *r)
{
    List *lList = (List *) (*((KeyValue **) l))->value;
    List *rList = (List *) (*((KeyValue **) r))->value;
    int lOpId = INT_VALUE(getNthOfListP(lList, 0));
    int rOpId = INT_VALUE(getNthOfListP(rList, 0));

    return lOpId - rOpId;
}

static void
operatorToOverviewInternal(StringInfo str, QueryOperator *op, int indent, HashMap *map)
{
    // if operator has more than one parents then we outsource
    if (LIST_LENGTH(op->parents) > 1)
    {
        List *opInfo = (List *) MAP_GET_LONG(map, (long) op); // info is: [id, stringRep]
        int opId;

        indentString(str, indent);

        if (opInfo == NIL)
        {
            StringInfo opStr;
            Constant *curId = (Constant *) MAP_GET_STRING(map, OP_ID_STRING);
            opId = INT_VALUE(curId);
            int *idVal = (int *) curId->value;
            (*idVal)++;
            opStr = makeStringInfo();
            opInfo = LIST_MAKE(createConstInt(opId), createConstLong((long) opStr));
            MAP_ADD_LONG_KEY(map, (long) op, opInfo);

            // append link
            appendStringInfo(str, "@%u\n", opId);

            // add to separate stringinfo
            str = opStr;
            indent = 0;
        }
        else
        {
            opId = INT_VALUE(getNthOfListP(opInfo, 0));
            appendStringInfo(str, "@%u\n", opId);

            return;
        }
    }
    else
        indentString(str, indent);


    // output specific operator things
    switch(op->type)
    {
        case T_ProjectionOperator:
        {
            ProjectionOperator *o = (ProjectionOperator *) op;
            WRITE_NODE_TYPE(Projection);

            appendStringInfoString(str, " [");
            FOREACH(Node,expr,o->projExprs)
            {
                appendStringInfo(str, "%s ", exprToSQL(expr));
            }
            appendStringInfoChar(str, ']');
        }
            break;
        case T_SelectionOperator:
            WRITE_NODE_TYPE(Selection);
            appendStringInfoString(str, " [");
            appendStringInfoString(str, exprToSQL(((SelectionOperator *) op)->cond));
            appendStringInfoChar(str, ']');
            break;
        case  T_JoinOperator:
        {
            JoinOperator *o =  (JoinOperator *) op;
            switch(o->joinType) {
                case JOIN_INNER:
                    WRITE_NODE_TYPE(Join);
                    break;
                case JOIN_CROSS:
                    WRITE_NODE_TYPE(CrossProduct);
                    break;
                case JOIN_LEFT_OUTER:
                    WRITE_NODE_TYPE(LeftOuterJoin);
                    break;
                case JOIN_RIGHT_OUTER:
                    WRITE_NODE_TYPE(RightOuterJoin);
                    break;
                case JOIN_FULL_OUTER:
                    WRITE_NODE_TYPE(FullOuterJoin);
                    break;
            }
            appendStringInfoString(str, " [");
            appendStringInfoString(str, exprToSQL(o->cond));
            appendStringInfoChar(str, ']');
        }
            break;
        case T_AggregationOperator:
        {
            AggregationOperator *o = (AggregationOperator *) op;
            WRITE_NODE_TYPE(Aggregation);
            appendStringInfoString(str, " [");
            appendStringInfoString(str, exprToSQL((Node *) o->aggrs));
            appendStringInfoString(str, o->groupBy ? "] GROUP BY [" : "");
            appendStringInfoString(str, exprToSQL((Node *) o->groupBy));
            appendStringInfoChar(str, ']');
        }
            break;
        case T_ProvenanceComputation:
            WRITE_NODE_TYPE(ProvenanceComputation);
            break;
        case T_TableAccessOperator:
            WRITE_NODE_TYPE(TableAccess);
            appendStringInfoString(str, " [");
            appendStringInfoString(str, ((TableAccessOperator *) op)->tableName);
            appendStringInfoChar(str, ']');
            break;
        case T_SetOperator:
        {
            SetOperator *o = (SetOperator *) op;
            switch(o->setOpType)
            {
                case SETOP_UNION:
                    WRITE_NODE_TYPE(Union);
                    break;
                case SETOP_INTERSECTION:
                    WRITE_NODE_TYPE(Intersection);
                    break;
                case SETOP_DIFFERENCE:
                    WRITE_NODE_TYPE(SetDifference);
                    break;
            }

        }
        break;
        case T_DuplicateRemoval:
            WRITE_NODE_TYPE(DuplicateRemoval);
            break;
        case T_ConstRelOperator:
        {
            ConstRelOperator *o = (ConstRelOperator *) op;

            WRITE_NODE_TYPE(ConstRelOperator);
            appendStringInfoString(str, " [");
            appendStringInfoString(str, exprToSQL((Node *) o->values));
            appendStringInfoChar(str, ']');
        }
        break;
        case T_NestingOperator:
        {
            NestingOperator *o = (NestingOperator *) op;
            const char *nestingType = (o->nestingType == NESTQ_EXISTS) ? "EXISTS" :
                    ((o->nestingType == NESTQ_ANY) ? "ANY" :
                    ((o->nestingType == NESTQ_ALL) ? "ALL" :
                    ((o->nestingType == NESTQ_UNIQUE) ? "UNIQUE" :
                    ((o->nestingType == NESTQ_SCALAR) ? "SCALAR" : "")
                    )));

            WRITE_NODE_TYPE(NestingOperator);
            appendStringInfo(str, "[%s] [%s]", nestingType, o->cond ? exprToSQL(o->cond) : "");
        }
        break;
        case T_WindowOperator:
        {
            WindowOperator *o = (WindowOperator *) op;
            WRITE_NODE_TYPE(WindowOperator);

            appendStringInfo(str, "[%s] ", exprToSQL(o->f));

            appendStringInfoString(str, "[");
            FOREACH(Node,part,o->partitionBy)
                appendStringInfo(str, "%s ", exprToSQL(part));
            appendStringInfoString(str, "] ");

            appendStringInfoString(str, "[");
            FOREACH(Node,part,o->orderBy)
                appendStringInfo(str, "%s ", exprToSQL(part));
            appendStringInfoString(str, "] ");

            //exprToSQL((Node *) o->frameDef);
        }
        break;
        case T_OrderOperator:
        {
            OrderOperator *o = (OrderOperator *) op;
            WRITE_NODE_TYPE(OrderOperator);
            appendStringInfo(str, "%s", exprToSQL((Node *) o->orderExprs));
        }
        break;
        case T_JsonTableOperator:
            WRITE_NODE_TYPE(JsonTable);
            appendStringInfoString(str, " [");
            appendStringInfoString(str, ((JsonTableOperator *) op)->jsonTableIdentifier);
            appendStringInfoChar(str, ']');
            break;
        default:
            FATAL_LOG("not a query operator:\n%s", op);
            break;
    }

    // output name
    appendStringInfo(str, " [%s] ", op->schema->name);

    // output attribute names
    appendStringInfoString(str, "(");
    int pos = 0;
    FOREACH(AttributeDef,a,op->schema->attrDefs)
        appendStringInfo(str, "%s%s ", a->attrName,
        		searchListInt(op->provAttrs, pos++) ? "*" : "");
    appendStringInfoString(str, ")");

    // output address and parent addresses
    appendStringInfo(str, " [%p]", op);

    appendStringInfoString(str, "(");
    FOREACH(QueryOperator,parent,op->parents)
    	appendStringInfo(str, "%p ", parent);
    appendStringInfoString(str, ")\n");

    FOREACH(QueryOperator,child,op->inputs)
        operatorToOverviewInternal(str, child, indent + 1, map);
}

static void
indentString(StringInfo str, int level)
{
    while(level-- > 0)
        appendStringInfoString(str, "  ");
}

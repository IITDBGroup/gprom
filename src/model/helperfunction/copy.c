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
#include "instrumentation/timing_instrumentation.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/set/hashmap.h"
#include "model/set/vector.h"
#include "model/expression/expression.h"
#include "model/query_block/query_block.h"
#include "model/datalog/datalog_model.h"
#include "model/query_operator/query_operator.h"

/* data structures for copying operator nodes */
typedef struct OperatorMap
{
    QueryOperator *from; // the original operator
    QueryOperator *copy; // the copied operator
    UT_hash_handle hh;
} OperatorMap;

#define ADD_OP_TO_MAP(_old,_new) \
    do { \
        OperatorMap *el = NEW(OperatorMap); \
        el->from = (QueryOperator *) _old; \
        el->copy = (QueryOperator *) _new; \
        HASH_ADD_PTR(*opMap, from, el); \
    } while(0)

#define MAP_HAS_OP(_old) \
    mapHasOp((QueryOperator *) _old, opMap)

#define MAP_GET_OP(_old) \
    mapGetOp((QueryOperator *) _old, opMap)

/* operator map functions */
static boolean mapHasOp(QueryOperator *op, OperatorMap **opMap);
static QueryOperator *mapGetOp(QueryOperator *op, OperatorMap **opMap);

/* internal copy entry point that creates the operator map */
static void *copyInternal(void *from, OperatorMap **opMap);

/* functions to copy collection types */
static List *deepCopyList(List *from, OperatorMap **opMap);
static Set *deepCopySet(Set *from, OperatorMap **opMap);
static HashMap *deepCopyHashMap(HashMap *from, OperatorMap **opMap);
static Vector *deepCopyVector(Vector *from, OperatorMap **opMap);

/* functions to copy expression node types */
static FunctionCall *copyFunctionCall(FunctionCall *from, OperatorMap **opMap);
static KeyValue *copyKeyValue(KeyValue *from, OperatorMap **opMap);
static AttributeReference *copyAttributeReference(AttributeReference *from, OperatorMap **opMap);
static Operator *copyOperator(Operator *from, OperatorMap **opMap);
static SQLParameter *copySQLParameter(SQLParameter *from, OperatorMap **opMap);
static CaseExpr *copyCaseExpr(CaseExpr *from, OperatorMap **opMap);
static CaseWhen *copyCaseWhen(CaseWhen *from, OperatorMap **opMap);
static IsNullExpr *copyIsNullExpr(IsNullExpr *from, OperatorMap **opMap);
static WindowBound *copyWindowBound(WindowBound *from, OperatorMap **opMap);
static WindowFrame *copyWindowFrame(WindowFrame *from, OperatorMap **opMap);
static WindowDef *copyWindowDef(WindowDef *from, OperatorMap **opMap);
static WindowFunction *copyWindowFunction(WindowFunction *from, OperatorMap **opMap);
static RowNumExpr *copyRowNumExpr(RowNumExpr *from, OperatorMap **opMap);
static OrderExpr *copyOrderExpr(OrderExpr *from, OperatorMap **opMap);
static CastExpr *copyCastExpr(CastExpr *from, OperatorMap **opMap);

/*schema helper functions*/
static AttributeDef *copyAttributeDef(AttributeDef *from, OperatorMap **opMap);
static Schema *copySchema(Schema *from, OperatorMap **opMap);

/*functions to copy query_operator*/
static QueryOperator *copyQueryOperator(QueryOperator *from, QueryOperator *new, OperatorMap **opMap);
static TableAccessOperator *copyTableAccessOperator(TableAccessOperator *from, OperatorMap **opMap);
static JsonTableOperator *copyJsonTableOperator(JsonTableOperator *from, OperatorMap **opMap);
static JsonPath *copyJsonPath(JsonPath *from, OperatorMap **opMap);
static SelectionOperator *copySelectionOperator(SelectionOperator *from, OperatorMap **opMap);
static ProjectionOperator *copyProjectionOperator(ProjectionOperator *from, OperatorMap **opMap);
static JoinOperator *copyJoinOperator(JoinOperator *from, OperatorMap **opMap);
static AggregationOperator *copyAggregationOperator(AggregationOperator *from, OperatorMap **opMap);
static SetOperator *copySetOperator(SetOperator *from, OperatorMap **opMap);
static DuplicateRemoval *copyDuplicateRemoval(DuplicateRemoval *from, OperatorMap **opMap);
static ProvenanceComputation *copyProvenanceComputation(ProvenanceComputation *from, OperatorMap **opMap);
static ConstRelOperator *copyConstRelOperator(ConstRelOperator *from, OperatorMap **opMap);
static NestingOperator *copyNestingOperator(NestingOperator *from, OperatorMap **opMap);
static WindowOperator *copyWindowOperator(WindowOperator *from, OperatorMap **opMap);
static OrderOperator *copyOrderOperator(OrderOperator *from, OperatorMap **opMap);
static FromJsonTable *copyFromJsonTable(FromJsonTable *from, OperatorMap **opMap);
static JsonColInfoItem *copyJsonColInfoItem(JsonColInfoItem *from,OperatorMap ** opMap);

/*functions to copy query_block*/
static SetQuery *copySetQuery(SetQuery *from, OperatorMap **opMap);
//static SetOp *copySetOp(SetOp *from, OperatorMap **opMap);
static QueryBlock *copyQueryBlock(QueryBlock *from, OperatorMap **opMap);
static Constant *copyConstant(Constant *from, OperatorMap **opMap);
static NestedSubquery *copyNestedSubquery(NestedSubquery *from, OperatorMap **opMap);
static ProvenanceStmt *copyProvenanceStmt(ProvenanceStmt *from, OperatorMap **opMap);
static ProvenanceTransactionInfo *copyProvenanceTransactionInfo (
        ProvenanceTransactionInfo *from, OperatorMap **opMap);
static SelectItem *copySelectItem(SelectItem  *from, OperatorMap **opMap);
static void copyFromItem (FromItem *from, FromItem *to);
static FromTableRef *copyFromTableRef(FromTableRef *from, OperatorMap **opMap);
static FromSubquery *copyFromSubquery(FromSubquery *from, OperatorMap **opMap);
static FromJoinExpr *copyFromJoinExpr(FromJoinExpr *from, OperatorMap **opMap);
static DistinctClause *copyDistinctClause(DistinctClause *from, OperatorMap **opMap);
static Insert *copyInsert(Insert *from, OperatorMap **opMap);
static Delete *copyDelete(Delete *from, OperatorMap **opMap);
static Update *copyUpdate(Update *from, OperatorMap **opMap);
static TransactionStmt *copyTransactionStmt(TransactionStmt *from, OperatorMap **opMap);
static FromProvInfo *copyFromProvInfo(FromProvInfo *from, OperatorMap **opMap);
static WithStmt *copyWithStmt(WithStmt *from, OperatorMap **opMap);

/* functions to copy datalog model elements */
static DLAtom *copyDLAtom(DLAtom *from, OperatorMap **opMap);
static DLVar *copyDLVar(DLVar *from, OperatorMap **opMap);
static DLComparison *copyDLComparison(DLComparison *from, OperatorMap **opMap);
static DLRule *copyDLRule(DLRule *from, OperatorMap **opMap);
static DLProgram *copyDLProgram(DLProgram *from, OperatorMap **opMap);

/*use the Macros(the varibles are 'new' and 'from')*/

/* creates a new pointer to a node and allocated mem */
#define COPY_INIT(type) \
		type *new; \
		new = makeNode(type);

/*copy a simple scalar field(int, bool, float, etc)*/
#define COPY_SCALAR_FIELD(fldname)  \
		(new->fldname = from->fldname)

/*copy a field that is a pointer to Node or Node tree*/
#define COPY_NODE_FIELD(fldname)  \
		(new->fldname) = (copyInternal(from->fldname, opMap))

/*copy a field that is a pointer to C string or NULL*/
#define COPY_STRING_FIELD(fldname) \
         new->fldname = (from->fldname !=NULL ? strdup(from->fldname) : NULL)
/* copy a field that is a list of strings */
#define COPY_STRING_LIST_FIELD(fldname) \
		 new->fldname = deepCopyStringList((List *) from->fldname)


/*deep copy for List operation*/
static List *
deepCopyList(List *from, OperatorMap **opMap)
{
    List *new = NIL;

    if(from == NIL)
        return NIL;

    if (from->type == T_IntList)
    {
        FOREACH_INT(i, from)
            new = appendToTailOfListInt(new, i);
    }
    else
    {
        FOREACH(Node,n,from)
            new = appendToTailOfList(new, copyInternal(n, opMap));
    }

    return new;
}

static Set *
deepCopySet(Set *from, OperatorMap **opMap)
{
    Set *new = newSet(from->setType, from->typelen, from->eq, from->cpy);

    switch(from->setType)
    {
        case SET_TYPE_INT:
            FOREACH_SET_INT(a,from)
                addIntToSet(new, a);
        break;
        case SET_TYPE_NODE:
            FOREACH_SET(Node,a,from)
                addToSet(new, copyObject(a));
            break;
        case SET_TYPE_POINTER:
            FOREACH_SET(void,a,from)
                addToSet(new, a);
            break;
        case SET_TYPE_STRING:
            FOREACH_SET(char,a,from)
                addToSet(new, strdup(a));
            break;
    }

    return new;
}

static HashMap *
deepCopyHashMap(HashMap *from, OperatorMap **opMap)
{
    HashMap *new = newHashMap(from->keyType, from->valueType, NULL, NULL);

    FOREACH_HASH_ENTRY(n,from)
        ADD_TO_MAP(new,copyObject(n));

    return new;
}

static Vector *
deepCopyVector(Vector *from, OperatorMap **opMap)
{
    Vector *new = makeVector(from->elType, from->elNodeType);

    FREE(new->data);
    COPY_SCALAR_FIELD(length);
    COPY_SCALAR_FIELD(maxLength);
    new->data = MALLOC(getVecDataSize(from));

    switch(from->elType)
    {
        case VECTOR_INT:
            memcpy(new->data, from->data, getVecDataSize(from));
            break;
        case VECTOR_NODE:
            new->length = 0;
            FOREACH_VEC(Node,n,from)
                VEC_ADD_NODE(from,copyObject(n));
            break;
        case VECTOR_STRING:
            break;
    }

    return new;
}

static DLAtom *
copyDLAtom(DLAtom *from, OperatorMap **opMap)
{
    COPY_INIT(DLAtom);

    COPY_STRING_FIELD(rel);
    COPY_NODE_FIELD(args);
    COPY_SCALAR_FIELD(negated);
    COPY_NODE_FIELD(n.properties);

    return new;
}

static DLVar *
copyDLVar(DLVar *from, OperatorMap **opMap)
{
    COPY_INIT(DLVar);

    COPY_STRING_FIELD(name);
    COPY_SCALAR_FIELD(dt);
    COPY_NODE_FIELD(n.properties);

    return new;
}

static DLComparison *
copyDLComparison(DLComparison *from, OperatorMap **opMap)
{
    COPY_INIT(DLComparison);

    COPY_NODE_FIELD(opExpr);
    COPY_NODE_FIELD(n.properties);

    return new;
}

static DLRule *
copyDLRule(DLRule *from, OperatorMap **opMap)
{
    COPY_INIT(DLRule);

    COPY_NODE_FIELD(head);
    COPY_NODE_FIELD(body);
    COPY_NODE_FIELD(n.properties);

    return new;
}

static DLProgram *
copyDLProgram(DLProgram *from, OperatorMap **opMap)
{
    COPY_INIT(DLProgram);

    COPY_NODE_FIELD(rules);
    COPY_NODE_FIELD(facts);
    COPY_STRING_FIELD(ans);
    COPY_NODE_FIELD(n.properties);

    return new;
}

static AttributeReference *
copyAttributeReference(AttributeReference *from, OperatorMap **opMap)
{
    COPY_INIT(AttributeReference);
    COPY_STRING_FIELD(name);
    COPY_SCALAR_FIELD(fromClauseItem);
    COPY_SCALAR_FIELD(attrPosition);
    COPY_SCALAR_FIELD(outerLevelsUp);
    COPY_SCALAR_FIELD(attrType);

    return new;
}

static FunctionCall *
copyFunctionCall(FunctionCall *from, OperatorMap **opMap)
{ 
    COPY_INIT(FunctionCall);
    COPY_STRING_FIELD(functionname);
    COPY_NODE_FIELD(args);
    COPY_SCALAR_FIELD(isAgg);

    return new;
}

static KeyValue *
copyKeyValue(KeyValue *from, OperatorMap **opMap)
{
    COPY_INIT(KeyValue);

    COPY_NODE_FIELD(key);
    COPY_NODE_FIELD(value);

    return new;
}

static Operator *
copyOperator(Operator *from, OperatorMap **opMap)
{ 
    COPY_INIT(Operator);
    COPY_STRING_FIELD(name);
    COPY_NODE_FIELD(args);

    return new;
}

static SQLParameter *
copySQLParameter(SQLParameter *from, OperatorMap **opMap)
{
    COPY_INIT(SQLParameter);
    COPY_STRING_FIELD(name);
    COPY_SCALAR_FIELD(position);
    COPY_SCALAR_FIELD(parType);

    return new;
}

static CaseExpr *
copyCaseExpr(CaseExpr *from, OperatorMap **opMap)
{
    COPY_INIT(CaseExpr);
    COPY_NODE_FIELD(expr);
    COPY_NODE_FIELD(whenClauses);
    COPY_NODE_FIELD(elseRes);

    return new;
}

static CaseWhen *
copyCaseWhen(CaseWhen *from, OperatorMap **opMap)
{
    COPY_INIT(CaseWhen);
    COPY_NODE_FIELD(when);
    COPY_NODE_FIELD(then);

    return new;
}

static IsNullExpr *
copyIsNullExpr(IsNullExpr *from, OperatorMap **opMap)
{
    COPY_INIT(IsNullExpr);
    COPY_NODE_FIELD(expr);

    return new;
}

static WindowBound *
copyWindowBound(WindowBound *from, OperatorMap **opMap)
{
    COPY_INIT(WindowBound);

    COPY_SCALAR_FIELD(bType);
    COPY_NODE_FIELD(expr);

    return new;
}

static WindowFrame *
copyWindowFrame(WindowFrame *from, OperatorMap **opMap)
{
    COPY_INIT(WindowFrame);

    COPY_SCALAR_FIELD(frameType);
    COPY_NODE_FIELD(lower);
    COPY_NODE_FIELD(higher);

    return new;
}

static WindowDef *
copyWindowDef(WindowDef *from, OperatorMap **opMap)
{
    COPY_INIT(WindowDef);

    COPY_NODE_FIELD(partitionBy);
    COPY_NODE_FIELD(orderBy);
    COPY_NODE_FIELD(frame);

    return new;
}

static WindowFunction *
copyWindowFunction(WindowFunction *from, OperatorMap **opMap)
{
    COPY_INIT(WindowFunction);

    COPY_NODE_FIELD(f);
    COPY_NODE_FIELD(win);

    return new;
}

static RowNumExpr *
copyRowNumExpr(RowNumExpr *from, OperatorMap **opMap)
{
    COPY_INIT(RowNumExpr);

    return new;
}

static OrderExpr *
copyOrderExpr(OrderExpr *from, OperatorMap **opMap)
{
    COPY_INIT(OrderExpr);

    COPY_NODE_FIELD(expr);
    COPY_SCALAR_FIELD(order);
    COPY_SCALAR_FIELD(nullOrder);

    return new;
}

static CastExpr *
copyCastExpr(CastExpr *from, OperatorMap **opMap)
{
    COPY_INIT(CastExpr);

    COPY_SCALAR_FIELD(resultDT);
    COPY_NODE_FIELD(expr);

    return new;
}

static AttributeDef *
copyAttributeDef(AttributeDef *from, OperatorMap **opMap)
{
    COPY_INIT(AttributeDef);
    COPY_SCALAR_FIELD(dataType);
    COPY_STRING_FIELD(attrName);
    COPY_SCALAR_FIELD(pos);

    return new;
}

static Schema *
copySchema(Schema *from, OperatorMap **opMap)
{
    COPY_INIT(Schema);
    COPY_STRING_FIELD(name);
    COPY_NODE_FIELD(attrDefs);
    
    return new;
}

/*functions to copy query_operator*/
static QueryOperator *
copyQueryOperator(QueryOperator *from, QueryOperator *new, OperatorMap **opMap)
{
    // record that new is a copy of from
    ADD_OP_TO_MAP(from, new);

    // copy regular fields
    COPY_NODE_FIELD(schema);
    COPY_NODE_FIELD(provAttrs);
    COPY_NODE_FIELD(properties);

    // cannot set parents, because not all parents may have been copied yet
    new->parents = NIL;

    // copy children only if they have not been copied before
    FOREACH(QueryOperator,child,from->inputs)
    {
        QueryOperator *newChild;
        // for existing children retrieve them from opMap
        if(MAP_HAS_OP(child))
             newChild = MAP_GET_OP(child);
        // else copy the child
        else
            newChild = (QueryOperator *) copyInternal(child, opMap);

        // ... and adapt newChilds their parents list and add them to inputs
        new->inputs = appendToTailOfList(new->inputs, newChild);
        newChild->parents = appendToTailOfList(newChild->parents, new);
    }

    return new;
}

#define COPY_OPERATOR() copyQueryOperator((QueryOperator *) from, (QueryOperator *) new, opMap)

static TableAccessOperator *
copyTableAccessOperator(TableAccessOperator *from, OperatorMap **opMap)
{
    COPY_INIT(TableAccessOperator);
    COPY_OPERATOR();
    COPY_STRING_FIELD(tableName);
    COPY_NODE_FIELD(asOf);

    return new;
}

static JsonTableOperator *
copyJsonTableOperator(JsonTableOperator *from, OperatorMap **opMap)
{
    COPY_INIT(JsonTableOperator);
    COPY_OPERATOR();

    COPY_NODE_FIELD(columns);

    COPY_STRING_FIELD(documentcontext);
    COPY_NODE_FIELD(jsonColumn);
    COPY_STRING_FIELD(jsonTableIdentifier);

    return new;
}

static JsonPath *
copyJsonPath(JsonPath *from, OperatorMap **opMap)
{
	COPY_INIT(JsonPath);
	COPY_STRING_FIELD(path);

	return new;
}

static SelectionOperator *
copySelectionOperator(SelectionOperator *from, OperatorMap **opMap)
{
    COPY_INIT(SelectionOperator);
    COPY_OPERATOR();
    COPY_NODE_FIELD(cond);

    return new;
}

static ProjectionOperator *
copyProjectionOperator(ProjectionOperator *from, OperatorMap **opMap)
{
    COPY_INIT(ProjectionOperator);
    COPY_OPERATOR();
    COPY_NODE_FIELD(projExprs);

    return new;
}

static JoinOperator *
copyJoinOperator(JoinOperator *from, OperatorMap **opMap)
{
    COPY_INIT(JoinOperator);
    COPY_OPERATOR();
    COPY_SCALAR_FIELD(joinType);
    COPY_NODE_FIELD(cond);

    return new;
}

static AggregationOperator *
copyAggregationOperator(AggregationOperator *from, OperatorMap **opMap)
{
    COPY_INIT(AggregationOperator);
    COPY_OPERATOR();
    COPY_NODE_FIELD(aggrs);
    COPY_NODE_FIELD(groupBy);

    return new;
}

static SetOperator *
copySetOperator(SetOperator *from, OperatorMap **opMap)
{
    COPY_INIT(SetOperator);
    COPY_OPERATOR();
    COPY_SCALAR_FIELD(setOpType);

    return new;
}

static DuplicateRemoval *
copyDuplicateRemoval(DuplicateRemoval *from, OperatorMap **opMap)
{
    COPY_INIT(DuplicateRemoval);
    COPY_OPERATOR();
    COPY_NODE_FIELD(attrs);

    return new;
}

static ProvenanceComputation *
copyProvenanceComputation(ProvenanceComputation *from, OperatorMap **opMap)
{
    COPY_INIT(ProvenanceComputation);
    COPY_OPERATOR();
    COPY_SCALAR_FIELD(provType);
    COPY_SCALAR_FIELD(inputType);
    COPY_NODE_FIELD(transactionInfo);
    COPY_NODE_FIELD(asOf);

    return new;
}

static ConstRelOperator *
copyConstRelOperator(ConstRelOperator *from, OperatorMap **opMap)
{
    COPY_INIT(ConstRelOperator);
    COPY_OPERATOR();
    COPY_NODE_FIELD(values);

    return new;
}

static NestingOperator *
copyNestingOperator(NestingOperator *from, OperatorMap **opMap)
{
    COPY_INIT(NestingOperator);
    COPY_OPERATOR();
    COPY_SCALAR_FIELD(nestingType);
    COPY_NODE_FIELD(cond);

    return new;
}

static WindowOperator *
copyWindowOperator(WindowOperator *from, OperatorMap **opMap)
{
    COPY_INIT(WindowOperator);
    COPY_OPERATOR();
    COPY_NODE_FIELD(partitionBy);
    COPY_NODE_FIELD(orderBy);
    COPY_NODE_FIELD(frameDef);
    COPY_STRING_FIELD(attrName);
    COPY_NODE_FIELD(f);

    return new;
}

static OrderOperator *
copyOrderOperator(OrderOperator *from, OperatorMap **opMap)
{
    COPY_INIT(OrderOperator);
    COPY_OPERATOR();

    COPY_NODE_FIELD(orderExprs);

    return new;
}

static JsonColInfoItem *
copyJsonColInfoItem(JsonColInfoItem *from, OperatorMap **opMap)
{
    COPY_INIT(JsonColInfoItem);

    COPY_STRING_FIELD(attrName);
    COPY_STRING_FIELD(path);
    COPY_STRING_FIELD(attrType);

    COPY_STRING_FIELD(format);
    COPY_STRING_FIELD(wrapper);
    COPY_NODE_FIELD(nested);
    COPY_STRING_FIELD(forOrdinality);

    return new;
}

/*functions to copy query_block*/
static SetQuery *
copySetQuery(SetQuery *from, OperatorMap **opMap)
{
    COPY_INIT(SetQuery);
    COPY_SCALAR_FIELD(setOp);
    COPY_SCALAR_FIELD(all);
    COPY_STRING_LIST_FIELD(selectClause);
    COPY_NODE_FIELD(lChild);
    COPY_NODE_FIELD(rChild);

    return new;
}

static QueryBlock *
copyQueryBlock(QueryBlock *from, OperatorMap **opMap)
{
    COPY_INIT(QueryBlock);
    COPY_NODE_FIELD(selectClause);
    COPY_NODE_FIELD(distinct);
    COPY_NODE_FIELD(fromClause);
    COPY_NODE_FIELD(whereClause);
    COPY_NODE_FIELD(groupByClause);
    COPY_NODE_FIELD(havingClause);
    COPY_NODE_FIELD(orderByClause);
    COPY_NODE_FIELD(limitClause);
    
    return new;
}

static Insert *
copyInsert(Insert *from, OperatorMap **opMap)
{
    COPY_INIT(Insert);
    COPY_STRING_FIELD(tableName);
    COPY_STRING_LIST_FIELD(attrList);
    COPY_NODE_FIELD(query);

    return new;
}

static Delete *
copyDelete(Delete *from, OperatorMap **opMap)
{
    COPY_INIT(Delete);
    COPY_STRING_FIELD(nodeName);
    COPY_NODE_FIELD(cond);

    return new;
}

static Update *
copyUpdate(Update *from, OperatorMap **opMap)
{
    COPY_INIT(Update);
    COPY_STRING_FIELD(nodeName);
    COPY_NODE_FIELD(selectClause);
    COPY_NODE_FIELD(cond);

    return new;
}

static TransactionStmt *
copyTransactionStmt(TransactionStmt *from, OperatorMap **opMap)
{
    COPY_INIT(TransactionStmt);
    COPY_SCALAR_FIELD(stmtType);

    return new;
}

static FromProvInfo *
copyFromProvInfo(FromProvInfo *from, OperatorMap **opMap)
{
    COPY_INIT(FromProvInfo);
    COPY_SCALAR_FIELD(baserel);
    COPY_SCALAR_FIELD(intermediateProv);
    COPY_STRING_LIST_FIELD(userProvAttrs);

    return new;
}

static WithStmt *
copyWithStmt(WithStmt *from, OperatorMap **opMap)
{
    COPY_INIT(WithStmt);
    COPY_NODE_FIELD(withViews);
    COPY_NODE_FIELD(query);

    return new;
}

static NestedSubquery *
copyNestedSubquery(NestedSubquery *from, OperatorMap **opMap)
{
    COPY_INIT(NestedSubquery);
    COPY_SCALAR_FIELD(nestingType);
    COPY_NODE_FIELD(expr);
    COPY_STRING_FIELD(comparisonOp);
    COPY_NODE_FIELD(query);

    return new;
}

static ProvenanceStmt *
copyProvenanceStmt(ProvenanceStmt *from, OperatorMap **opMap)
{
    COPY_INIT(ProvenanceStmt);
    COPY_NODE_FIELD(query);
    COPY_NODE_FIELD(selectClause);
    COPY_SCALAR_FIELD(provType);
    COPY_SCALAR_FIELD(inputType);
    COPY_NODE_FIELD(transInfo);    
    COPY_NODE_FIELD(asOf);
    COPY_NODE_FIELD(options);

    return new;
}

static ProvenanceTransactionInfo *
copyProvenanceTransactionInfo (ProvenanceTransactionInfo *from,
        OperatorMap **opMap)
{
    COPY_INIT(ProvenanceTransactionInfo);
    COPY_SCALAR_FIELD(transIsolation);
    COPY_STRING_LIST_FIELD(updateTableNames);
    COPY_NODE_FIELD(originalUpdates);
    COPY_NODE_FIELD(scns);
    COPY_NODE_FIELD(commitSCN);

    return new;
}

static SelectItem *
copySelectItem(SelectItem  *from, OperatorMap **opMap)
{
    COPY_INIT(SelectItem);
    COPY_STRING_FIELD(alias);
    COPY_NODE_FIELD(expr);

    return new;
}

static void
copyFromItem (FromItem *from, FromItem *to)
{
    to->name = strdup(from->name);
    to->attrNames = deepCopyStringList(from->attrNames);
    to->provInfo = copyObject(from->provInfo);
    to->dataTypes = copyObject(from->dataTypes);
}

#define COPY_FROM() copyFromItem((FromItem *) from, (FromItem *) new);

static FromTableRef *
copyFromTableRef(FromTableRef *from, OperatorMap **opMap)
{
    COPY_INIT(FromTableRef);
    COPY_FROM();
    COPY_STRING_FIELD(tableId);

    return new;
}

static FromJsonTable*
copyFromJsonTable(FromJsonTable *from,OperatorMap **opMap)
{
    COPY_INIT(FromJsonTable);

    COPY_FROM();

    COPY_NODE_FIELD(columns);
    COPY_STRING_FIELD(documentcontext);
    COPY_NODE_FIELD(jsonColumn);
    COPY_STRING_FIELD(jsonTableIdentifier);

    return new;
}

static Constant *
copyConstant(Constant *from, OperatorMap **opMap)
{
      COPY_INIT(Constant);
      COPY_SCALAR_FIELD(constType); 
	  COPY_SCALAR_FIELD(isNull);
	  if (from->isNull)
	  {
	      new->value = NULL;
	      return new;
	  }

	  switch (from->constType)
	  {
	      case DT_INT:
	          new->value = NEW(int);
	          *((int *) new->value) = *((int *) from->value);
	          break;
	      case DT_FLOAT:
	          new->value = NEW(double);
	          *((double *) new->value) = *((double *) from->value);
	          break;
	      case DT_BOOL:
	          new->value = NEW(boolean);
	          *((boolean *) new->value) = *((boolean *) from->value);
	          break;
	      case DT_STRING:
	          new->value = strdup(from->value);
	          break;
	      case DT_LONG:
	          new->value = NEW(long);
	          *((long *) new->value) = *((long *) from->value);
	          break;
	      case DT_VARCHAR2:
	    	  new->value = strdup(from->value);
	          break;
	  }
	  return new;
}

static FromSubquery *
copyFromSubquery(FromSubquery *from, OperatorMap **opMap)
{
    COPY_INIT(FromSubquery);
    COPY_FROM();
    COPY_NODE_FIELD(subquery);
    
    return new;
}

static FromJoinExpr *
copyFromJoinExpr(FromJoinExpr *from, OperatorMap **opMap)
{
    COPY_INIT(FromJoinExpr);
    COPY_FROM();
    COPY_NODE_FIELD(left);
    COPY_NODE_FIELD(right);
    COPY_SCALAR_FIELD(joinType);
    COPY_SCALAR_FIELD(joinCond);
    if (from->joinCond == JOIN_COND_USING)
        new->cond = (Node *) deepCopyStringList((List *) from->cond);
    else
        COPY_NODE_FIELD(cond);

    return new;
}

static DistinctClause *
copyDistinctClause(DistinctClause *from, OperatorMap **opMap)
{
    COPY_INIT(DistinctClause);
    COPY_NODE_FIELD(distinctExprs);

    return new;
}
/*copyObject copy of a Node tree or list and all substructure copied too */
/*this is a deep copy & with recursive*/
static void *
copyInternal(void *from, OperatorMap **opMap)
{
    void *retval;

    if(from == NULL)
        return NULL;

    /* different type nodes */
    switch(nodeTag(from))
    {
        /* collection type nodes*/
        case T_List:
        case T_IntList:
            retval = deepCopyList(from, opMap);
            break;
        case T_Set:
            retval = deepCopySet(from, opMap);
            break;
        case T_HashMap:
            retval = deepCopyHashMap(from, opMap);
            break;
        case T_Vector:
            retval = deepCopyVector(from, opMap);
            break;

        /* expression model */
        case T_AttributeReference:
            retval = copyAttributeReference(from, opMap);
            break;
        case T_FunctionCall:
            retval = copyFunctionCall(from, opMap);
            break;
        case T_KeyValue:
            retval = copyKeyValue(from, opMap);
            break;
        case T_Operator:
            retval = copyOperator(from, opMap);
            break;
        case T_Schema:
            retval = copySchema(from, opMap);
            break;
        case T_AttributeDef:
            retval = copyAttributeDef(from, opMap);
            break;
        case T_SQLParameter:
            retval = copySQLParameter(from, opMap);
            break;
        case T_CaseExpr:
            retval = copyCaseExpr(from, opMap);
            break;
        case T_CaseWhen:
            retval = copyCaseWhen(from, opMap);
            break;
        case T_IsNullExpr:
            retval = copyIsNullExpr(from, opMap);
            break;
        case T_WindowBound:
            retval = copyWindowBound(from, opMap);
            break;
        case T_WindowFrame:
            retval = copyWindowFrame(from, opMap);
            break;
        case T_WindowDef:
            retval = copyWindowDef(from, opMap);
            break;
        case T_WindowFunction:
            retval = copyWindowFunction(from, opMap);
            break;
        case T_RowNumExpr:
            retval = copyRowNumExpr(from, opMap);
            break;
        case T_OrderExpr:
            retval = copyOrderExpr(from, opMap);
            break;
        case T_CastExpr:
            retval = copyCastExpr(from, opMap);
            break;
            /* query block model nodes */
//        case T_SetOp:
//            retval = copySetOp(from, opMap);
//            break;
        case T_SetQuery:
            retval = copySetQuery(from, opMap);
            break;
        case T_ProvenanceStmt:
            retval = copyProvenanceStmt(from, opMap);
            break;
        case T_ProvenanceTransactionInfo:
            retval = copyProvenanceTransactionInfo(from, opMap);
            break;
        case T_QueryBlock:
            retval = copyQueryBlock(from, opMap);
            break;
        case T_WithStmt:
            retval = copyWithStmt(from, opMap);
            break;
        case T_SelectItem:
            retval = copySelectItem(from, opMap);
            break;
        case T_Constant:
            retval = copyConstant(from, opMap);
            break;
        case T_NestedSubquery:
            retval = copyNestedSubquery(from, opMap);
            break;
        case T_FromTableRef:
            retval = copyFromTableRef(from, opMap);
            break;
        case T_FromSubquery:
            retval = copyFromSubquery(from, opMap);
            break;
        case T_FromJoinExpr:
            retval = copyFromJoinExpr(from, opMap);
            break;
        case T_FromProvInfo:
            retval = copyFromProvInfo(from, opMap);
            break;
        case T_DistinctClause:
            retval = copyDistinctClause(from, opMap);
            break;
        case T_Insert:
            retval = copyInsert(from, opMap);
            break;
        case T_Delete:
            retval = copyDelete(from, opMap);
            break;
        case T_Update:
            retval = copyUpdate(from, opMap);
            break;
        case T_TransactionStmt:
            retval = copyTransactionStmt(from, opMap);
            break;

             /* query operator model nodes */
        case T_SelectionOperator:
            retval = copySelectionOperator(from, opMap);
            break;
        case T_ProjectionOperator:
            retval = copyProjectionOperator(from, opMap);
            break;
        case T_JoinOperator:
            retval = copyJoinOperator(from, opMap);
            break;
        case T_AggregationOperator:
            retval = copyAggregationOperator(from, opMap);
            break;
        case T_ProvenanceComputation:
            retval = copyProvenanceComputation(from, opMap);
            break;
        case T_TableAccessOperator:
            retval = copyTableAccessOperator(from, opMap);
            break;
        case T_SetOperator:
            retval = copySetOperator(from, opMap);
            break;
        case T_DuplicateRemoval:
            retval = copyDuplicateRemoval(from, opMap);
            break;
        case T_ConstRelOperator:
            retval = copyConstRelOperator(from, opMap);
            break;
        case T_NestingOperator:
            retval = copyNestingOperator(from, opMap);
            break;
        case T_WindowOperator:
            retval = copyWindowOperator(from, opMap);
            break;
        case T_OrderOperator:
            retval = copyOrderOperator(from, opMap);
            break;
        case T_FromJsonTable:
            retval = copyFromJsonTable(from, opMap);
            break;
        case T_JsonColInfoItem:
	    retval = copyJsonColInfoItem(from, opMap);
	    break;
            /* datalog model nodes */
        case T_DLAtom:
            retval = copyDLAtom(from, opMap);
            break;
        case T_DLVar:
            retval = copyDLVar(from, opMap);
            break;
        case T_DLRule:
            retval = copyDLRule(from, opMap);
            break;
        case T_DLProgram:
            retval = copyDLProgram(from, opMap);
            break;
        case T_DLComparison:
            retval = copyDLComparison(from, opMap);
            break;
        case T_JsonTableOperator:
        	retval = copyJsonTableOperator(from, opMap);
        	break;
        case T_JsonPath:
        	retval = copyJsonPath(from, opMap);
        	break;
        default:
            retval = NULL;
            break;
    }

    return retval;
}

void *copyObject(void *from)
{
    OperatorMap *opMap = NULL;
    void *result = NULL;

    result = copyInternal(from, &opMap);

    HASH_CLEAR(hh,opMap);

    return result;
}

static boolean
mapHasOp(QueryOperator *op, OperatorMap **opMap)
{
    OperatorMap *result;

    HASH_FIND_PTR(*opMap,&op,result);

    return result != NULL;
}

static QueryOperator *
mapGetOp(QueryOperator *op, OperatorMap **opMap)
{
    OperatorMap *result;

    HASH_FIND_PTR(*opMap,&op,result);
    ASSERT(result != NULL);

    return result->copy;
}

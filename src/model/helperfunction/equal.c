/*************************************
 *         equal.c
 *    Author: Hao Guo
 *    One-line description
 *
 *
 *
 **************************************/

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "instrumentation/timing_instrumentation.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/set/vector.h"
#include "model/set/hashmap.h"
#include "model/bitset/bitset.h"
#include "model/graph/graph.h"
#include "model/expression/expression.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/datalog/datalog_model.h"
#include "model/integrity_constraints/integrity_constraints.h"
#include "model/rpq/rpq_model.h"
#include "log/logger.h"

#define EQUAL_CONTEXT_NAME "EQUAL_CONTEXT"

/* internal equal function */
static boolean equalInternal(void *a, void *b, HashMap *seenOps, MemContext *c);

/* equal function for collection types */
static boolean equalList(List *a, List *b, HashMap *seenOps, MemContext *c);
static boolean equalStringListInternal (List *a, List *b, HashMap *seenOps, MemContext *c);
static boolean equalSet (Set *a, Set *b, HashMap *seenOps, MemContext *c);
static boolean equalKeyValue (KeyValue *a, KeyValue *b, HashMap *seenOps, MemContext *c);
static boolean equalHashMap (HashMap *a, HashMap *b, HashMap *seenOps, MemContext *c);
static boolean equalVector (Vector *a, Vector *b, HashMap *seenOps, MemContext *c);
static boolean equalBitset (BitSet *a, BitSet *b, HashMap *seenOps, MemContext *c);
static boolean equalGraph (Graph *a, Graph *b, HashMap *seenOps, MemContext *c);

/* equal functions for expression types */
static boolean equalFunctionCall(FunctionCall *a, FunctionCall *b, HashMap *seenOps, MemContext *c);
static boolean equalAttributeReference (AttributeReference *a,
        AttributeReference *b, HashMap *seenOps, MemContext *c);
static boolean equalSQLParameter (SQLParameter *a, SQLParameter* b, HashMap *seenOps, MemContext *c);
static boolean equalOperator (Operator *a, Operator *b, HashMap *seenOps, MemContext *c);
static boolean equalConstant (Constant *a, Constant *b, HashMap *seenOps, MemContext *c);
static boolean equalCaseExpr (CaseExpr *a, CaseExpr *b, HashMap *seenOps, MemContext *c);
static boolean equalCastExpr (CastExpr *a, CastExpr *b, HashMap *seenOps, MemContext *c);
static boolean equalCaseWhen (CaseWhen *a, CaseWhen *b, HashMap *seenOps, MemContext *c);
static boolean equalIsNullExpr (IsNullExpr *a, IsNullExpr *b, HashMap *seenOps, MemContext *c);
static boolean equalWindowBound (WindowBound *a, WindowBound *b, HashMap *seenOps, MemContext *c);
static boolean equalWindowFrame (WindowFrame *a, WindowFrame *b, HashMap *seenOps, MemContext *c);
static boolean equalWindowDef (WindowDef *a, WindowDef *b, HashMap *seenOps, MemContext *c);
static boolean equalWindowFunction (WindowFunction *a, WindowFunction *b, HashMap *seenOps, MemContext *c);
static boolean equalRowNumExpr (RowNumExpr *a, RowNumExpr *b, HashMap *seenOps, MemContext *c);
static boolean equalOrderExpr (OrderExpr *a, OrderExpr *b, HashMap *seenOps, MemContext *c);

/* equal functions for integrity constraints */
static boolean equalFD(FD *a, FD *b, HashMap *seenOps, MemContext *c);
static boolean equalFOdep(FOdep *a, FOdep *b, HashMap *seenOps, MemContext *c);

// equal functions for query_operator
static boolean equalSchema(Schema *a, Schema *b, HashMap *seenOps, MemContext *c);
//static boolean equalSchemaFromLists(Schema *a, Schema *b, HashMap *seenOps, MemContext *c);
static boolean equalAttributeDef(AttributeDef *a, AttributeDef *b, HashMap *seenOps, MemContext *c);
static boolean equalQueryOperator(QueryOperator *a, QueryOperator *b, HashMap *seenOps, MemContext *c);
static boolean equalTableAccessOperator(TableAccessOperator *a, TableAccessOperator *b, HashMap *seenOps, MemContext *c);
static boolean equalSampleClauseOperator(SampleClauseOperator *a, SampleClauseOperator *b, HashMap *seenOps, MemContext *c);
static boolean equalSelectionOperator(SelectionOperator *a, SelectionOperator *b, HashMap *seenOps, MemContext *c);
static boolean equalProjectionOperator(ProjectionOperator *a, ProjectionOperator *b, HashMap *seenOps, MemContext *c);
static boolean equalJoinOperator(JoinOperator *a, JoinOperator *b, HashMap *seenOps, MemContext *c);
static boolean equalAggregationOperator(AggregationOperator *a, AggregationOperator *b, HashMap *seenOps, MemContext *c);
static boolean equalSetOperator(SetOperator *a, SetOperator *b, HashMap *seenOps, MemContext *c);
static boolean equalDuplicateRemoval(DuplicateRemoval *a, DuplicateRemoval *b, HashMap *seenOps, MemContext *c);
static boolean equalProvenanceComputation(ProvenanceComputation *a, ProvenanceComputation *b, HashMap *seenOps, MemContext *c);
static boolean equalConstRelOperator(ConstRelOperator *a, ConstRelOperator *b, HashMap *seenOps, MemContext *c);
static boolean equalNestingOperator(NestingOperator *a, NestingOperator *b, HashMap *seenOps, MemContext *c);
static boolean equalWindowOperator(WindowOperator *a, WindowOperator *b, HashMap *seenOps, MemContext *c);
static boolean equalOrderOperator(OrderOperator *a, OrderOperator *b, HashMap *seenOps, MemContext *c);
static boolean equalLimitOperator(LimitOperator *a, LimitOperator *b, HashMap *seenOps, MemContext *c);

// Json
static boolean equalFromJsonTable(FromJsonTable *a, FromJsonTable *b, HashMap *seenOps, MemContext *c);
static boolean equalJsonColInfoItem(JsonColInfoItem *a, JsonColInfoItem *b, HashMap *seenOps, MemContext *c);
static boolean equalJsonTableOperator(JsonTableOperator *a, JsonTableOperator *b, HashMap *seenOps, MemContext *c);
static boolean equalJsonPath(JsonPath *a, JsonPath *b, HashMap *seenOps, MemContext *c);

// equal functions for query_block
static boolean equalQueryBlock(QueryBlock *a, QueryBlock *b, HashMap *seenOps, MemContext *c);
static boolean equalSetQuery(SetQuery *a, SetQuery *b, HashMap *seenOps, MemContext *c);
static boolean equalNestedSubquery (NestedSubquery *a, NestedSubquery *b, HashMap *seenOps, MemContext *c);
static boolean equalProvenanceStmt(ProvenanceStmt *a, ProvenanceStmt *b, HashMap *seenOps, MemContext *c);
static boolean equalProvenanceTransactionInfo(ProvenanceTransactionInfo *a,
        ProvenanceTransactionInfo *b, HashMap *seenOps, MemContext *c);
static boolean equalWithStmt(WithStmt *a, WithStmt *b, HashMap *seenOps, MemContext *c);
static boolean equalSelectItem(SelectItem *a, SelectItem *b, HashMap *seenOps, MemContext *c);
static boolean equalFromItem(FromItem *a, FromItem *b, HashMap *seenOps, MemContext *c);
static boolean equalFromTableRef(FromTableRef *a, FromTableRef *b, HashMap *seenOps, MemContext *c);
static boolean equalFromSubquery(FromSubquery *a, FromSubquery *b, HashMap *seenOps, MemContext *c);
static boolean equalFromJoinExpr(FromJoinExpr *a, FromJoinExpr *b, HashMap *seenOps, MemContext *c);
static boolean equalDistinctClause(DistinctClause *a,  DistinctClause *b, HashMap *seenOps, MemContext *c);
static boolean equalInsert(Insert *a, Insert *b, HashMap *seenOps, MemContext *c);
static boolean equalDelete(Delete *a, Delete *b, HashMap *seenOps, MemContext *c);
static boolean equalUpdate(Update *a, Update *b, HashMap *seenOps, MemContext *c);
static boolean equalTransactionStmt(TransactionStmt *a, TransactionStmt *b, HashMap *seenOps, MemContext *c);
static boolean equalFromProvInfo (FromProvInfo *a, FromProvInfo *b, HashMap *seenOps, MemContext *c);
static boolean equalCreateTable (CreateTable *a, CreateTable *b, HashMap *seenOps, MemContext *c);
static boolean equalAlterTable (AlterTable *a, AlterTable *b, HashMap *seenOps, MemContext *c);

// equal functions for datalog model
static boolean equalDLAtom (DLAtom *a, DLAtom *b, HashMap *seenOps, MemContext *c);
static boolean equalDLVar (DLVar *a, DLVar *b, HashMap *seenOps, MemContext *c);
static boolean equalDLRule (DLRule *a, DLRule *b, HashMap *seenOps, MemContext *c);
static boolean equalDLProgram (DLProgram *a, DLProgram *b, HashMap *seenOps, MemContext *c);
static boolean equalDLComparison (DLComparison *a, DLComparison *b, HashMap *seenOps, MemContext *c);
static boolean equalDLDomain (DLDomain *a, DLDomain *b, HashMap *seenOps, MemContext *c);

// equal functions for regex model
static boolean equalRegex (Regex *a, Regex *b, HashMap *seenOps, MemContext *c);
static boolean equalRPQQuery (RPQQuery *a, RPQQuery *b, HashMap *seenOps, MemContext *c);

/* use these macros to compare fields */

/* compare one simple scalar field(int, boolean, float, etc)*/
#define COMPARE_SCALAR_FIELD(fldname)  \
		do{  \
			if (a->fldname != b->fldname)  \
			return FALSE;  \
		} while (0)

/* compare a field pointer to Node tree*/
#define COMPARE_NODE_FIELD(fldname)  \
		do{  \
			if(!equalInternal(a->fldname, b->fldname, seenOps, c))  \
			return FALSE;  \
		} while (0)

/* compare a field pointer to a string list*/
#define COMPARE_STRING_LIST_FIELD(fldname)  \
        do{  \
            if(!equalStringListInternal((List *) a->fldname, (List *) b->fldname, seenOps, c))  \
            return FALSE;  \
        } while (0)

/* compare a field that is a pointer to a C string or maybe NULL*/
#define COMPARE_STRING_FIELD(fldname)  \
		do{  \
			if(!equalstr(a->fldname, b->fldname))  \
			return FALSE;  \
		} while (0)

/* compare the common query operator fields */
#define COMPARE_QUERY_OP() \
		do { \
			if(!equalQueryOperator((QueryOperator *) &a->op, (QueryOperator *) &b->op, seenOps, c)) \
			return FALSE; \
		} while (0)

/*compare a string field that maybe NULL*/
#define equalstr(a, b)  \
		(((a) != NULL && (b) != NULL) ? (strcmp(a, b) == 0) : (a) == (b))

/* datalog model comparisons */
static boolean
equalDLAtom (DLAtom *a, DLAtom *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_STRING_FIELD(rel);
    COMPARE_NODE_FIELD(args);
    COMPARE_SCALAR_FIELD(negated);
    COMPARE_NODE_FIELD(n.properties);

    return TRUE;
}

static boolean
equalDLVar (DLVar *a, DLVar *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_STRING_FIELD(name);
    COMPARE_SCALAR_FIELD(dt);
    COMPARE_NODE_FIELD(n.properties);

    return TRUE;
}

static boolean
equalDLRule (DLRule *a, DLRule *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_NODE_FIELD(head);
    COMPARE_NODE_FIELD(body);
    COMPARE_NODE_FIELD(n.properties);

    return TRUE;
}

static boolean
equalDLProgram (DLProgram *a, DLProgram *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_NODE_FIELD(rules);
    COMPARE_NODE_FIELD(facts);
    COMPARE_STRING_FIELD(ans);
    COMPARE_NODE_FIELD(doms);
    COMPARE_NODE_FIELD(n.properties);
    COMPARE_NODE_FIELD(comp);
    COMPARE_NODE_FIELD(func);
    COMPARE_NODE_FIELD(sumOpts);

    return TRUE;
}

static boolean
equalDLComparison (DLComparison *a, DLComparison *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_NODE_FIELD(opExpr);
    COMPARE_NODE_FIELD(n.properties);

    return TRUE;
}

static boolean
equalDLDomain (DLDomain *a, DLDomain *b, HashMap *seenOps, MemContext *c)
{
	COMPARE_STRING_FIELD(rel);
	COMPARE_STRING_FIELD(attr);
	COMPARE_STRING_FIELD(name);
    COMPARE_NODE_FIELD(n.properties);

    return TRUE;
}

static boolean
equalRegex (Regex *a, Regex *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_NODE_FIELD(children);
    COMPARE_SCALAR_FIELD(opType);
    COMPARE_STRING_FIELD(label);

    return TRUE;
}

static boolean
equalRPQQuery (RPQQuery *a, RPQQuery *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_NODE_FIELD(q);
    COMPARE_SCALAR_FIELD(t);
    COMPARE_STRING_FIELD(edgeRel);
    COMPARE_STRING_FIELD(resultRel);

    return TRUE;
}

/* */
static boolean
equalAttributeReference (AttributeReference *a,
        AttributeReference *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_STRING_FIELD(name);
    COMPARE_SCALAR_FIELD(fromClauseItem);
    COMPARE_SCALAR_FIELD(attrPosition);
    COMPARE_SCALAR_FIELD(outerLevelsUp);
    COMPARE_SCALAR_FIELD(attrType);

    return TRUE;
}

static boolean
equalSQLParameter (SQLParameter *a, SQLParameter *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_STRING_FIELD(name);
    COMPARE_SCALAR_FIELD(position);
    COMPARE_SCALAR_FIELD(parType);

    return TRUE;
}

static boolean
equalOperator (Operator *a, Operator *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_STRING_FIELD(name);
    COMPARE_NODE_FIELD(args);

    return TRUE;
}

static boolean
equalConstant (Constant *a, Constant *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_SCALAR_FIELD(constType);

    // if both are NULL they are considered equal
    if (a->isNull && b->isNull)
        return TRUE;

    // only one of them is NULL return FALSE
    if (a->isNull || b->isNull)
        return FALSE;

    switch(a->constType)
    {
        case DT_INT:
            return INT_VALUE(a) == INT_VALUE(b);
        case DT_FLOAT:
            return FLOAT_VALUE(a) == FLOAT_VALUE(b);
        case DT_BOOL:
            return BOOL_VALUE(a) == BOOL_VALUE(b);
        case DT_STRING:
            return strcmp(STRING_VALUE(a), STRING_VALUE(b)) == 0;
        case DT_LONG:
            return LONG_VALUE(a) == LONG_VALUE(b);
        case DT_VARCHAR2:
	    return strcmp(STRING_VALUE(a), STRING_VALUE(b)) == 0;
    }

    COMPARE_SCALAR_FIELD(isNull);

    return TRUE;
}

static boolean
equalCaseExpr (CaseExpr *a, CaseExpr *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_NODE_FIELD(expr);
    COMPARE_NODE_FIELD(whenClauses);
    COMPARE_NODE_FIELD(elseRes);

    return TRUE;
}

static boolean
equalCaseWhen (CaseWhen *a, CaseWhen *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_NODE_FIELD(when);
    COMPARE_NODE_FIELD(then);

    return TRUE;
}

static boolean
equalCastExpr (CastExpr *a, CastExpr *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_SCALAR_FIELD(resultDT);
    COMPARE_NODE_FIELD(expr);

    return TRUE;
}

static boolean
equalIsNullExpr (IsNullExpr *a, IsNullExpr *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_NODE_FIELD(expr);

    return TRUE;
}

static boolean
equalWindowBound (WindowBound *a, WindowBound *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_SCALAR_FIELD(bType);
    COMPARE_NODE_FIELD(expr);

    return TRUE;
}

static boolean
equalWindowFrame (WindowFrame *a, WindowFrame *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_SCALAR_FIELD(frameType);
    COMPARE_NODE_FIELD(lower);
    COMPARE_NODE_FIELD(higher);

    return TRUE;
}

static boolean
equalWindowDef (WindowDef *a, WindowDef *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_NODE_FIELD(partitionBy);
    COMPARE_NODE_FIELD(orderBy);
    COMPARE_NODE_FIELD(frame);

    return TRUE;
}

static boolean
equalWindowFunction (WindowFunction *a, WindowFunction *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_NODE_FIELD(f);
    COMPARE_NODE_FIELD(win);

    return TRUE;
}

static boolean
equalRowNumExpr (RowNumExpr *a, RowNumExpr *b, HashMap *seenOps, MemContext *c)
{
    return TRUE;
}

static boolean
equalOrderExpr (OrderExpr *a, OrderExpr *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_NODE_FIELD(expr);
    COMPARE_SCALAR_FIELD(order);
    COMPARE_SCALAR_FIELD(nullOrder);

    return TRUE;
}

static boolean
equalFD(FD *a, FD *b, HashMap *seenOps, MemContext *c)
{
	COMPARE_STRING_FIELD(table);
	COMPARE_NODE_FIELD(lhs);
	COMPARE_NODE_FIELD(rhs);

	return TRUE;
}

static boolean
equalFOdep(FOdep *a, FOdep *b, HashMap *seenOps, MemContext *c)
{
	COMPARE_NODE_FIELD(lhs);
	COMPARE_NODE_FIELD(rhs);

	return TRUE;
}


static boolean
equalQuantifiedComparison (QuantifiedComparison *a, QuantifiedComparison *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_NODE_FIELD(checkExpr);
    COMPARE_NODE_FIELD(exprList);
    COMPARE_SCALAR_FIELD(qType);
    COMPARE_STRING_FIELD(opName);

    return TRUE;
}

/* */
static boolean
equalFunctionCall(FunctionCall *a, FunctionCall *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_STRING_FIELD(functionname);
    COMPARE_NODE_FIELD(args);
    COMPARE_SCALAR_FIELD(isAgg);
    COMPARE_SCALAR_FIELD(isDistinct);

    return TRUE;
}

/*equal list fun */
static boolean
equalList(List *a, List *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_SCALAR_FIELD(type);
    COMPARE_SCALAR_FIELD(length);

    // lists have same type and length
    ASSERT(LIST_LENGTH(a) > 0 && LIST_LENGTH(b) > 0);

    switch(a->type)
    {
        case T_List:
        {
            FORBOTH(Node,l,r,a,b)
            {
                if (!equal(l,r))
                    return FALSE;
            }
        }
        break;
        case T_IntList:
        {
            FORBOTH_INT(i,j,a,b)
            {
                if (i != j)
                    return FALSE;
            }
        }
        break;
        default:
            return FALSE;
    }

    return TRUE;
}

static boolean
equalStringListInternal (List *a, List *b, HashMap *seenOps, MemContext *c)
{
    if (a == NULL && b == NULL)
        return TRUE;
    if (a == NULL || b == NULL)
        return FALSE;

    COMPARE_SCALAR_FIELD(type);
    COMPARE_SCALAR_FIELD(length);

    // lists have same type and length
    ASSERT(LIST_LENGTH(a) > 0 && LIST_LENGTH(b) > 0);

    FORBOTH(char,s1,s2,a,b)
    {
        if (strcmp(s1,s2) != 0)
            return FALSE;
    }

    return TRUE;
}

boolean
equalStringList (List *a, List *b)
{
    return equalStringListInternal(a,b,NULL, NULL);
}

static boolean
equalSet (Set *a, Set *b, HashMap *seenOps, MemContext *c)
{
    if (a->setType != b->setType)
        return FALSE;
    if (setSize(a) != setSize(b))
        return FALSE;

    if (a->setType == SET_TYPE_INT)
    {
        FOREACH_SET(int,i,a)
        {
            if (!hasSetIntElem(b,*i))
                return FALSE;
        }

        FOREACH_SET(int,i,b)
        {
            if (!hasSetIntElem(a,*i))
                return FALSE;
        }

        return TRUE;
    }

    if (a->setType == SET_TYPE_LONG)
    {
        FOREACH_SET(gprom_long_t,i,a)
        {
            if (!hasSetLongElem(b,*i))
                return FALSE;
        }

        FOREACH_SET(gprom_long_t,i,b)
        {
            if (!hasSetLongElem(a,*i))
                return FALSE;
        }

        return TRUE;
    }

    FOREACH_SET(void,el,a)
    {
        if (!hasSetElem(b,el))
            return FALSE;
    }
    FOREACH_SET(void,el,b)
    {
        if (!hasSetElem(a,el))
            return FALSE;
    }

    return TRUE;
}

static boolean
equalKeyValue (KeyValue *a, KeyValue *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_NODE_FIELD(key);
    COMPARE_NODE_FIELD(value);

    return TRUE;
}

static boolean
equalHashMap (HashMap *a, HashMap *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_SCALAR_FIELD(keyType);
    COMPARE_SCALAR_FIELD(valueType);

    FOREACH_HASH_KEY(Node,k,a)
        if(!hasMapKey(b,k) || !equal(getMap(a,k),getMap(b,k)))
            return FALSE;

    FOREACH_HASH_KEY(Node,k,b)
        if(!hasMapKey(a,k))
            return FALSE;

    return TRUE;
}

static boolean
equalVector (Vector *a, Vector *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_SCALAR_FIELD(elType);
    COMPARE_SCALAR_FIELD(length);
//    COMPARE_SCALAR_FIELD(elNodeType);

    switch(a->elType)
    {
        case VECTOR_INT:
        {
            int *aA, *bA;
            aA = VEC_TO_IA(a);
            bA = VEC_TO_IA(b);

            for(int i = 0; i < VEC_LENGTH(a); i++)
                if (aA[i] != bA[i])
                    return FALSE;
        }
        break;
        case VECTOR_STRING:
        {
            char **aA, **bA;
            aA = VEC_TO_ARR(a,char);
            bA = VEC_TO_ARR(b,char);

            for(int i = 0; i < VEC_LENGTH(a); i++)
                if (strcmp(aA[i],bA[i]) != 0)
                    return FALSE;
        }
        break;
        case VECTOR_NODE:
        {
            Node **aA, **bA;
            aA = VEC_TO_ARR(a,Node);
            bA = VEC_TO_ARR(b,Node);

            for(int i = 0; i < VEC_LENGTH(a); i++)
                if (!equal(aA[i],bA[i]))
                    return FALSE;
        }
        break;
    }

    return TRUE;
}

static boolean
equalBitset(BitSet *a, BitSet *b, HashMap *seenOps, MemContext *c)
{
	return bitsetEquals(a,b);
}

static boolean
equalGraph(Graph *a, Graph *b, HashMap *seenOps, MemContext *c)
{
	COMPARE_NODE_FIELD(nodes);
	COMPARE_NODE_FIELD(edges);

	return TRUE;
}

static boolean
equalSchema(Schema *a, Schema *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_STRING_FIELD(name);
    COMPARE_NODE_FIELD(attrDefs);

    return TRUE;
}

//static boolean
//equalSchemaFromLists(Schema *a, Schema *b, HashMap *seenOps, MemContext *c)
//{
//    COMPARE_STRING_FIELD(name);
//    COMPARE_NODE_FIELD(attrNames);
//    COMPARE_NODE_FIELD(dataTypes);
//
//    return TRUE;
//}

static boolean
equalAttributeDef(AttributeDef *a, AttributeDef *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_STRING_FIELD(attrName);
    //COMPARE_SCALAR_FIELD(dataType);

    return TRUE;
}

static boolean
equalQueryOperator(QueryOperator *a, QueryOperator *b, HashMap *seenOps, MemContext *c)
{
    gprom_long_t aAddr = (gprom_long_t) a;
    gprom_long_t bAddr = (gprom_long_t) b;

    if (c == NULL)
    {
        c = NEW_MEM_CONTEXT(EQUAL_CONTEXT_NAME);
        ACQUIRE_MEM_CONTEXT(c);
        seenOps = NEW_MAP(Constant,Constant);
    }

    if (MAP_HAS_LONG_KEY(seenOps, aAddr))
        return LONG_VALUE(MAP_GET_LONG(seenOps, aAddr)) == bAddr;
//    COMPARE_NODE_FIELD(inputs);

    COMPARE_NODE_FIELD(schema);
    //COMPARE_NODE_FIELD(parents); //TODO implement compare one node
    COMPARE_NODE_FIELD(provAttrs);
    COMPARE_NODE_FIELD(properties);

    // store mapping in hashmap
    MAP_ADD_LONG_KEY(seenOps,aAddr,createConstLong(bAddr));

    return TRUE;
}

static boolean
equalTableAccessOperator(TableAccessOperator *a, TableAccessOperator *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_QUERY_OP();
    COMPARE_STRING_FIELD(tableName);
    COMPARE_NODE_FIELD(asOf);
//    COMPARE_NODE_FIELD(sampClause);

    return TRUE;
}

static boolean
equalSampleClauseOperator(SampleClauseOperator *a, SampleClauseOperator *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_QUERY_OP();
	COMPARE_NODE_FIELD(sampPerc);

	return TRUE;
}

static boolean
equalSelectionOperator(SelectionOperator *a, SelectionOperator *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_QUERY_OP();
    COMPARE_NODE_FIELD(cond);

    return TRUE;
}

static boolean
equalProjectionOperator(ProjectionOperator *a, ProjectionOperator *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_QUERY_OP();
    COMPARE_NODE_FIELD(projExprs);

    return TRUE;
}

static boolean
equalJoinOperator(JoinOperator *a, JoinOperator *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_QUERY_OP();
    COMPARE_SCALAR_FIELD(joinType);
    COMPARE_NODE_FIELD(cond);

    return TRUE;
}

static boolean
equalAggregationOperator(AggregationOperator *a, AggregationOperator *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_QUERY_OP();
    COMPARE_NODE_FIELD(aggrs);
    COMPARE_NODE_FIELD(groupBy);

    return TRUE;
}

static boolean
equalSetOperator(SetOperator *a, SetOperator *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_QUERY_OP();
    COMPARE_SCALAR_FIELD(setOpType);

    return TRUE;
}

static boolean
equalDuplicateRemoval(DuplicateRemoval *a, DuplicateRemoval *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_QUERY_OP();
    COMPARE_NODE_FIELD(attrs);

    return TRUE;
}

static boolean
equalProvenanceComputation(ProvenanceComputation *a,  ProvenanceComputation *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_QUERY_OP();
    COMPARE_SCALAR_FIELD(provType);
    COMPARE_SCALAR_FIELD(inputType);
    COMPARE_NODE_FIELD(transactionInfo);
    COMPARE_NODE_FIELD(asOf);

    return TRUE;
}

static boolean
equalConstRelOperator(ConstRelOperator *a, ConstRelOperator *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_QUERY_OP();
    COMPARE_NODE_FIELD(values);

    return TRUE;
}

static boolean
equalNestingOperator(NestingOperator *a, NestingOperator *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_QUERY_OP();
    COMPARE_SCALAR_FIELD(nestingType);
    COMPARE_NODE_FIELD(cond);

    return TRUE;
}

static boolean
equalWindowOperator (WindowOperator *a, WindowOperator *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_QUERY_OP();
    COMPARE_NODE_FIELD(partitionBy);
    COMPARE_NODE_FIELD(orderBy);
    COMPARE_NODE_FIELD(frameDef);
    COMPARE_STRING_FIELD(attrName);
    COMPARE_NODE_FIELD(f);

    return TRUE;
}

static boolean
equalOrderOperator(OrderOperator *a, OrderOperator *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_QUERY_OP();
    COMPARE_NODE_FIELD(orderExprs);

    return TRUE;
}

static boolean
equalLimitOperator(LimitOperator *a, LimitOperator *b, HashMap *seenOps, MemContext *c)
{
	COMPARE_QUERY_OP();
	COMPARE_NODE_FIELD(limitExpr);
	COMPARE_NODE_FIELD(offsetExpr);

	return TRUE;
}

static boolean
equalFromJsonTable(FromJsonTable *a, FromJsonTable *b, HashMap *seenOps, MemContext *c)
{
	COMPARE_NODE_FIELD(columns);
    COMPARE_STRING_FIELD(documentcontext);
    COMPARE_NODE_FIELD(jsonColumn);
    COMPARE_STRING_FIELD(jsonTableIdentifier);
    COMPARE_STRING_FIELD(forOrdinality);

    return TRUE;
}

static boolean
equalJsonColInfoItem(JsonColInfoItem *a, JsonColInfoItem *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_STRING_FIELD(attrName);
    COMPARE_STRING_FIELD(path);
    COMPARE_STRING_FIELD(attrType);

    COMPARE_STRING_FIELD(format);
    COMPARE_STRING_FIELD(wrapper);
    COMPARE_NODE_FIELD(nested);
    COMPARE_STRING_FIELD(forOrdinality);

    return TRUE;
}

static boolean
equalJsonTableOperator(JsonTableOperator *a, JsonTableOperator *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_QUERY_OP();

    COMPARE_NODE_FIELD(columns);
    COMPARE_STRING_FIELD(documentcontext);
    COMPARE_NODE_FIELD(jsonColumn);
    COMPARE_STRING_FIELD(jsonTableIdentifier);
    COMPARE_STRING_FIELD(forOrdinality);

    return TRUE;
}

static boolean
equalJsonPath(JsonPath *a, JsonPath *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_STRING_FIELD(path);

    return TRUE;
}

// equal functions for query_block
static boolean
equalQueryBlock(QueryBlock *a, QueryBlock *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_NODE_FIELD(selectClause);
    COMPARE_NODE_FIELD(distinct);
    COMPARE_NODE_FIELD(fromClause);
    COMPARE_NODE_FIELD(whereClause);
    COMPARE_NODE_FIELD(groupByClause);
    COMPARE_NODE_FIELD(havingClause);
    COMPARE_NODE_FIELD(orderByClause);
    COMPARE_NODE_FIELD(limitClause);
    COMPARE_NODE_FIELD(offsetClause);

    return TRUE;
}

static boolean
equalSetQuery(SetQuery *a, SetQuery *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_SCALAR_FIELD(setOp);
    COMPARE_SCALAR_FIELD(all);
    COMPARE_STRING_LIST_FIELD(selectClause);
    COMPARE_NODE_FIELD(lChild);
    COMPARE_NODE_FIELD(rChild);

    return TRUE;
}

static boolean
equalNestedSubquery (NestedSubquery *a, NestedSubquery *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_SCALAR_FIELD(nestingType);
    COMPARE_NODE_FIELD(expr);
    COMPARE_STRING_FIELD(comparisonOp);
    COMPARE_NODE_FIELD(query);

    return TRUE;
}


static boolean
equalInsert(Insert *a, Insert *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_NODE_FIELD(schema);
    COMPARE_STRING_FIELD(insertTableName);
    COMPARE_NODE_FIELD(query);

    return TRUE;

}

static boolean
equalDelete(Delete *a, Delete *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_NODE_FIELD(schema);
    COMPARE_STRING_FIELD(deleteTableName);
    COMPARE_NODE_FIELD(cond);

    return TRUE;

}

static boolean
equalUpdate(Update *a, Update *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_NODE_FIELD(schema);
    COMPARE_STRING_FIELD(updateTableName);
    COMPARE_NODE_FIELD(selectClause);
    COMPARE_NODE_FIELD(cond);

    return TRUE;

}

static boolean
equalTransactionStmt(TransactionStmt *a, TransactionStmt *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_SCALAR_FIELD(stmtType);

    return TRUE;
}

static boolean
equalFromProvInfo (FromProvInfo *a, FromProvInfo *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_SCALAR_FIELD(baserel);
    COMPARE_SCALAR_FIELD(intermediateProv);
    COMPARE_STRING_LIST_FIELD(userProvAttrs);
    COMPARE_NODE_FIELD(provProperties);
    return TRUE;
}

static boolean
equalCreateTable (CreateTable *a, CreateTable *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_STRING_FIELD(tableName);
    COMPARE_NODE_FIELD(tableElems);
    COMPARE_NODE_FIELD(constraints);
    COMPARE_NODE_FIELD(query);

    return TRUE;
}

static boolean
equalAlterTable (AlterTable *a, AlterTable *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_STRING_FIELD(tableName);
    COMPARE_SCALAR_FIELD(cmdType);
    COMPARE_STRING_FIELD(columnName);
    COMPARE_SCALAR_FIELD(newColDT);
    COMPARE_NODE_FIELD(schema);
    COMPARE_NODE_FIELD(beforeSchema);

    return TRUE;
}


static boolean
equalProvenanceStmt(ProvenanceStmt *a, ProvenanceStmt *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_NODE_FIELD(query);
    COMPARE_STRING_LIST_FIELD(selectClause);
    COMPARE_NODE_FIELD(dts);
    COMPARE_SCALAR_FIELD(provType);
    COMPARE_SCALAR_FIELD(inputType);
    COMPARE_NODE_FIELD(transInfo);
    COMPARE_NODE_FIELD(asOf);
    COMPARE_NODE_FIELD(options);
    COMPARE_NODE_FIELD(sumOpts);

    return TRUE;
}

static boolean
equalProvenanceTransactionInfo(ProvenanceTransactionInfo *a,
        ProvenanceTransactionInfo *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_SCALAR_FIELD(transIsolation);
    COMPARE_STRING_LIST_FIELD(updateTableNames);
    COMPARE_NODE_FIELD(originalUpdates);
    COMPARE_NODE_FIELD(scns);
    COMPARE_NODE_FIELD(commitSCN);


    return TRUE;
}

static boolean
equalWithStmt(WithStmt *a, WithStmt *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_NODE_FIELD(withViews);
    COMPARE_NODE_FIELD(query);

    return TRUE;
}

static boolean
equalSelectItem(SelectItem *a, SelectItem *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_STRING_FIELD(alias);
    COMPARE_NODE_FIELD(expr);

    return TRUE;
}

static boolean
equalFromItem(FromItem *a, FromItem *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_STRING_FIELD(name);
    COMPARE_STRING_LIST_FIELD(attrNames);
    COMPARE_NODE_FIELD(provInfo);
    COMPARE_NODE_FIELD(dataTypes);

    return TRUE;
}

static boolean
equalFromTableRef(FromTableRef *a, FromTableRef *b, HashMap *seenOps, MemContext *c)
{
    if (!equalFromItem((FromItem *) a, (FromItem *) b, seenOps, c))
        return FALSE;
    COMPARE_STRING_FIELD(tableId);

    return TRUE;
}

static boolean
equalFromSubquery(FromSubquery *a, FromSubquery *b, HashMap *seenOps, MemContext *c)
{
    if (!equalFromItem((FromItem *) a, (FromItem *) b, seenOps, c))
        return FALSE;
    COMPARE_NODE_FIELD(subquery);

    return TRUE;
}

static boolean
equalFromJoinExpr(FromJoinExpr *a, FromJoinExpr *b, HashMap *seenOps, MemContext *c)
{
    if (!equalFromItem((FromItem *) a, (FromItem *) b, seenOps, c))
        return FALSE;
    COMPARE_NODE_FIELD(left);
    COMPARE_NODE_FIELD(right);
    COMPARE_SCALAR_FIELD(joinType);
    COMPARE_SCALAR_FIELD(joinCond);
    if (a->joinCond == JOIN_COND_USING)
        COMPARE_STRING_LIST_FIELD(cond);
    else
        COMPARE_NODE_FIELD(cond);

    return TRUE;
}

static boolean
equalDistinctClause(DistinctClause *a,  DistinctClause *b, HashMap *seenOps, MemContext *c)
{
    COMPARE_NODE_FIELD(distinctExprs);

    return TRUE;
}

/* returns true if two pointers point to the same memory location */
boolean
ptrEqual(void *a, void *b)
{
    return a == b;
}

/* external equal function that does a quick check to determine whether to call internal function that allocates mem context */
boolean
equal(void *a, void *b)
{
    if (a == b)
        return TRUE;

    if (a == NULL || b == NULL)
        return FALSE;

    if (nodeTag(a) !=nodeTag(b))
        return FALSE;

    boolean result = equalInternal(a, b, NULL, NULL);

    if(streq(getCurMemContext()->contextName, EQUAL_CONTEXT_NAME))
        FREE_AND_RELEASE_CUR_MEM_CONTEXT();

    return result;
}

/*equalfun returns  whether two nodes are equal*/
boolean
equalInternal(void *a, void *b, HashMap *seenOps, MemContext *c)
{
    boolean retval;
    if (a == b)
        return TRUE;

    if (a == NULL || b == NULL)
        return FALSE;

    if (nodeTag(a) !=nodeTag(b))
        return FALSE;

    TRACE_LOG("same node types \n<%s>\nand\n<%s>", nodeToString(a), nodeToString(b));

    switch(nodeTag(a))
    {
        case T_List:
        case T_IntList:
            retval = equalList(a,b, seenOps, c);
            break;
        case T_Set:
            retval = equalSet(a,b, seenOps, c);
            break;
        case T_HashMap:
            retval = equalHashMap(a,b, seenOps, c);
            break;
        case T_Vector:
            retval = equalVector(a,b, seenOps, c);
            break;
	    case T_BitSet:
			retval = equalBitset(a, b, seenOps, c);
			break;
	    case T_Graph:
			retval = equalGraph(a, b, seenOps, c);
			break;
        case T_FunctionCall:
            retval = equalFunctionCall(a,b, seenOps, c);
            break;
        case T_AttributeReference:
            retval = equalAttributeReference(a,b, seenOps, c);
            break;
        case T_SQLParameter:
            retval = equalSQLParameter(a,b, seenOps, c);
            break;
        case T_Operator:
            retval = equalOperator(a,b, seenOps, c);
            break;
        case T_Constant:
            retval = equalConstant(a,b, seenOps, c);
            break;
        case T_CaseExpr:
            retval = equalCaseExpr(a,b, seenOps, c);
            break;
        case T_CaseWhen:
            retval = equalCaseWhen(a,b, seenOps, c);
            break;
        case T_CastExpr:
            retval = equalCastExpr(a,b, seenOps, c);
            break;
        case T_IsNullExpr:
            retval = equalIsNullExpr(a,b, seenOps, c);
            break;
        case T_WindowBound:
            retval = equalWindowBound(a,b, seenOps, c);
            break;
        case T_WindowFrame:
            retval = equalWindowFrame(a,b, seenOps, c);
            break;
        case T_WindowDef:
            retval = equalWindowDef(a,b, seenOps, c);
            break;
        case T_WindowFunction:
            retval = equalWindowFunction(a,b, seenOps, c);
            break;
        case T_RowNumExpr:
            retval = equalRowNumExpr(a,b, seenOps, c);
            break;
        case T_OrderExpr:
            retval = equalOrderExpr(a,b, seenOps, c);
            break;
        case T_FD:
            retval = equalFD(a,b, seenOps, c);
            break;
        case T_FOdep:
            retval = equalFOdep(a,b, seenOps, c);
            break;
	    case T_QuantifiedComparison:
            retval = equalQuantifiedComparison(a,b, seenOps, c);
            break;
            /*something different cases this, and we have*/
            /*different types of T_Node       */
        case T_QueryOperator:
            retval = equalQueryOperator(a,b, seenOps, c);
            break;
        case T_Schema:
            retval = equalSchema(a,b, seenOps, c);
            break;
        case T_KeyValue:
            retval = equalKeyValue(a,b, seenOps, c);
            break;
        case T_AttributeDef:
            retval = equalAttributeDef(a,b, seenOps, c);
            break;
        case T_TableAccessOperator:
            retval = equalTableAccessOperator(a,b, seenOps, c);
            break;
        case T_SampleClauseOperator:
        	retval = equalSampleClauseOperator(a,b, seenOps, c);
			break;
        case T_SelectionOperator:
            retval = equalSelectionOperator(a,b, seenOps, c);
            break;
        case T_ProjectionOperator:
            retval = equalProjectionOperator(a,b, seenOps, c);
            break;
        case T_JoinOperator:
            retval = equalJoinOperator(a,b, seenOps, c);
            break;
        case T_AggregationOperator:
            retval = equalAggregationOperator(a,b, seenOps, c);
            break;
        case T_SetOperator:
            retval = equalSetOperator(a,b, seenOps, c);
            break;
        case T_DuplicateRemoval:
            retval = equalDuplicateRemoval(a,b, seenOps, c);
            break;
        case T_ProvenanceComputation:
            retval = equalProvenanceComputation(a,b, seenOps, c);
            break;
        case T_ConstRelOperator:
            retval = equalConstRelOperator(a,b, seenOps, c);
            break;
        case T_QueryBlock:
            retval = equalQueryBlock(a,b, seenOps, c);
            break;
        case T_SetQuery:
            retval = equalSetQuery(a,b, seenOps, c);
            break;
        case T_NestedSubquery:
            retval = equalNestedSubquery(a,b, seenOps, c);
            break;
        case T_ProvenanceStmt:
            retval = equalProvenanceStmt(a,b, seenOps, c);
            break;
        case T_ProvenanceTransactionInfo:
            retval = equalProvenanceTransactionInfo(a,b, seenOps, c);
            break;
        case T_WithStmt:
            retval = equalWithStmt(a,b, seenOps, c);
            break;
        case T_SelectItem:
            retval = equalSelectItem(a,b, seenOps, c);
            break;
        case T_FromItem:
            retval = equalFromItem(a,b, seenOps, c);
            break;
        case T_FromProvInfo:
            retval = equalFromProvInfo(a,b, seenOps, c);
            break;
        case T_FromTableRef:
            retval = equalFromTableRef(a,b, seenOps, c);
            break;
        case T_FromSubquery:
            retval = equalFromSubquery(a,b, seenOps, c);
            break;
        case T_FromJoinExpr:
            retval = equalFromJoinExpr(a,b, seenOps, c);
            break;
        case T_DistinctClause:
            retval = equalDistinctClause(a,b, seenOps, c);
            break;
        case T_Insert:
            retval = equalInsert(a,b, seenOps, c);
            break;
        case T_Delete:
            retval = equalDelete(a,b, seenOps, c);
            break;
        case T_Update:
            retval = equalUpdate(a,b, seenOps, c);
            break;
        case T_TransactionStmt:
            retval = equalTransactionStmt(a,b, seenOps, c);
            break;
        case T_CreateTable:
            retval = equalCreateTable(a,b, seenOps, c);
            break;
        case T_AlterTable:
            retval = equalAlterTable(a,b, seenOps, c);
            break;
        case T_NestingOperator:
            retval = equalNestingOperator(a,b, seenOps, c);
            break;
        case T_WindowOperator:
            retval = equalWindowOperator(a,b, seenOps, c);
            break;
        case T_OrderOperator:
            retval = equalOrderOperator(a,b, seenOps, c);
            break;
        case T_LimitOperator:
            retval = equalLimitOperator(a,b, seenOps, c);
            break;
        case T_FromJsonTable:
            retval = equalFromJsonTable(a,b, seenOps, c);
            break;
        case T_JsonColInfoItem:
        	retval = equalJsonColInfoItem(a,b, seenOps, c);
        	break;
        case T_JsonTableOperator:
        	retval = equalJsonTableOperator(a,b, seenOps, c);
        	break;
        case T_JsonPath:
        	retval = equalJsonPath(a,b, seenOps, c);
        	break;
        /* datalog model */
        case T_DLAtom:
            retval = equalDLAtom(a,b, seenOps, c);
            break;
        case T_DLVar:
            retval = equalDLVar(a,b, seenOps, c);
            break;
        case T_DLRule:
            retval = equalDLRule(a,b, seenOps, c);
            break;
        case T_DLProgram:
            retval = equalDLProgram(a,b, seenOps, c);
            break;
        case T_DLComparison:
            retval = equalDLComparison(a,b, seenOps, c);
            break;
        case T_DLDomain:
            retval = equalDLDomain(a,b, seenOps, c);
            break;
        case T_Regex:
            retval = equalRegex(a,b, seenOps, c);
            break;
        case T_RPQQuery:
            retval = equalRPQQuery(a,b, seenOps, c);
            break;
        default:
            retval = FALSE;
            break;
    }

//    printf("equals: %p, %p, %s\n%s -- %s\n", a, b, (a == b) ? "TRUE" : "FALSE", nodeToString(a), nodeToString(b));

    if (!retval)
        TRACE_LOG("not equals \n%s\n\n%s", nodeToString(a), nodeToString(b));

    return retval;
}

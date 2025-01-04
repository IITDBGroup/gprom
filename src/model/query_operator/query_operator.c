/*-------------------------------------------------------------------------
 *
 * query_operator.c - Datastructures to encode relational algebra graphs
 *    Author: Ying Ni yni6@hawk.iit.edu
 *
 *
 *-------------------------------------------------------------------------
 */

#include "common.h"
#include "log/logger.h"
#include "metadata_lookup/metadata_lookup.h"
#include "model/expression/expression.h"
#include "model/query_operator/query_operator.h"
#include "mem_manager/mem_mgr.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "provenance_rewriter/prov_utility.h"
#include "model/set/set.h"
#include "model/query_operator/operator_property.h"
#include "operator_optimizer/optimizer_prop_inference.h"
#include "utility/string_utils.h"
#include "model/query_operator/query_operator_model_checker.h"

typedef struct CorrelatedAttrsState {
	int curDepth;
	Set *result;
	boolean corrInsideSubquery;
	boolean childrenCanBeKeptCorrelated;
    boolean seenNormalization;
} CorrelatedAttrsState;

static Schema *schemaFromExpressions (char *name, List *attributeNames, List *exprs, List *inputs);
static KeyValue *getProp (QueryOperator *op, Node *key);
static unsigned numOpsInTreeInternal (QueryOperator *q, unsigned int *count);
static boolean countUniqueOpsVisitor(QueryOperator *op, void *context);
static boolean internalVisitQOGraph (QueryOperator *q, TraversalOrder tOrder,
        boolean (*visitF) (QueryOperator *op, void *context), void *context,
        Set *haveSeen);
static boolean findCorrelatedAttrsVisitor(Node *n, CorrelatedAttrsState *state);
static boolean canStayCorrelatedVisitor(Node *n, CorrelatedAttrsState *state);


QueryOperator *
findNestingOperator (QueryOperator *op, int levelsUp)
{
    QueryOperator *result = op;

    FOREACH(QueryOperator, p, result->parents)
    {
            if(isA(p, NestingOperator))
            {
                levelsUp --;
                if(levelsUp == 0)
                    return p;
                else
                    return findNestingOperator(p, levelsUp);
            }
            else
                return findNestingOperator(p, levelsUp);
    }

    return result;
}

char *
getNestingAttrPrefix()
{
	return backendifyIdentifier("nesting_eval_");
}

char *
getNestingResultAttribute(int number)
{
	return backendifyIdentifier(CONCAT_STRINGS("nesting_eval_", gprom_itoa(number)));
}

boolean
isNestingAttribute(char *attr)
{
	char *prefix = getNestingAttrPrefix();
	return isPrefix(attr, prefix)
		|| isPrefix(attr, CONCAT_STRINGS("\"", prefix));
}

Schema *
createSchema(char *name, List *attrDefs)
{
    Schema *s = NEW(Schema);
    s->name = name;
    s->attrDefs = attrDefs;
    return s;
}

AttributeDef *
createAttributeDef (char *name, DataType dt)
{
    AttributeDef *result = makeNode(AttributeDef);

    result->dataType = dt;
    result->attrName = name;

    return result;
}

Schema *
createSchemaFromLists (char *name, List *attrNames, List *dataTypes)
{
    Schema *result = makeNode(Schema);

    result->name = strdup(name);
    result->attrDefs = NIL;

    if (dataTypes == NIL)
    {
        FOREACH(char,n,attrNames)
        {
            AttributeDef *a = makeNode(AttributeDef);
            a->attrName = strdup(n);
            a->dataType = DT_STRING;

            result->attrDefs = appendToTailOfList(result->attrDefs, a);
        }
    }
    else
    {
        FORBOTH_LC(n,dt,attrNames,dataTypes)
        {
            AttributeDef *a = makeNode(AttributeDef);
            a->attrName = strdup(LC_P_VAL(n));
            a->dataType = LC_INT_VAL(dt);

            result->attrDefs = appendToTailOfList(result->attrDefs, a);
        }
    }
    return result;
}

void
setAttrDefDataTypeBasedOnBelowOp(QueryOperator *op1, QueryOperator *op2)
{
	FOREACH(AttributeDef,a1,op1->schema->attrDefs)
	{
	      FOREACH(AttributeDef, a2, op2->schema->attrDefs)
		  {
	    	    if(streq(a1->attrName,a2->attrName))
	    	    {
	    	           a1->dataType = a2->dataType;
	    	           break;
	    	    }
		  }
	}
}

static Schema *
schemaFromExpressions(char *name, List *attributeNames, List *exprs, List *inputs)
{
    List *dataTypes = NIL;

	// if attributeNames is NIL, then generate attribute names
	if(attributeNames == NIL)
	{
		FOREACH(Node,n,exprs)
		{
			char *name = exprToSQL(n, NULL, FALSE); //TODO is that right for this usage?
			attributeNames = appendToTailOfList(attributeNames, name);
		}
	}

    FOREACH(Node,n,exprs)
	{
        dataTypes = appendToTailOfListInt(dataTypes, typeOf(n));
	}

    return createSchemaFromLists(name, attributeNames, dataTypes);
}

void
addAttrToSchema(QueryOperator *op, char *name, DataType dt)
{
    AttributeDef *a;

    a = createAttributeDef(strdup(name), dt);
    op->schema->attrDefs = appendToTailOfList(op->schema->attrDefs, a);
}

void
deleteAttrFromSchemaByName(QueryOperator *op, char *name, boolean adaptProvAttrs)
{
	int pos = -1;
	boolean found = FALSE;

    FOREACH(AttributeDef,a,op->schema->attrDefs)
    {
		pos++;
        if (streq(a->attrName,name))
        {
            op->schema->attrDefs = REMOVE_FROM_LIST_PTR(op->schema->attrDefs, a);
			found = TRUE;
            break;
        }
    }

	if(adaptProvAttrs && found)
	{
		op->provAttrs = removeFromListInt(op->provAttrs, pos);
		FOREACH_LC(lc, op->provAttrs)
		{
		    if(lc->data.int_value > pos)
			{
				lc->data.int_value = lc->data.int_value - 1;
			}
		}
	}
}

void
deleteAttrRefFromProjExprs(ProjectionOperator *op, int pos)
{
    int i = 0;

    FOREACH_LC(lc, op->projExprs)
    {
        if(i == pos)
        {
            op->projExprs = REMOVE_FROM_LIST_PTR(op->projExprs, LC_P_VAL(lc));
            break;
        }
        i++;
    }
}

void
reSetPosOfOpAttrRefBaseOnBelowLayerSchema(QueryOperator *op2, List *attrRefs)
{
	int cnt;
	DEBUG_LOG("resetAttrRefs %s\nbased references: %s",
			  NodeTagToString(op2->type),
			  beatify(nodeToString(attrRefs)));
    DEBUG_OP_LOG("tree", op2);
	FOREACH(AttributeReference,a1,attrRefs)
	{
	    cnt = 0;
        FOREACH(AttributeDef, a2, op2->schema->attrDefs)
        {

            if(strpeq(a1->name, a2->attrName))
            {
                a1->attrPosition = cnt;
                DEBUG_LOG("set attr %s position to %d", a1->name, cnt);
                break;
            }
            cnt++;
        }
	}
}

void
resetPosOfAttrRefBaseOnBelowLayerSchema(QueryOperator *parent, QueryOperator *child)
{
	List *attrRefs = NIL;

	// collect attribute references in parent and adapt them based on child
	if (isA(child,JoinOperator) || isA(child,ProjectionOperator))
    {
        if(!checkUniqueAttrNames(child))
        {
            makeAttrNamesUnique(child);
            DEBUG_OP_LOG("join or projection attributes are not unique", child);
        }
    }

	if(isA(parent, ProjectionOperator))
	{
		attrRefs = getAttrReferences((Node *) ((ProjectionOperator *)parent)->projExprs);
	}
	else if(isA(parent, SelectionOperator))
	{
		Node *cond = ((SelectionOperator *)parent)->cond;
		attrRefs = getAttrReferences(cond);
	}
	else if (isA(parent,JoinOperator))
	{
		Node *cond = ((JoinOperator *)parent)->cond;
		attrRefs = getAttrReferences(cond);
	}
	else if (isA(parent,AggregationOperator))
	{
		AggregationOperator *agg = (AggregationOperator *) parent;
		List *aggrs = getAttrReferences((Node *) agg->aggrs);
		List *groupBy = getAttrReferences((Node *) agg->groupBy);
		attrRefs = concatTwoLists(aggrs, groupBy);
	}
	else if (isA(parent,DuplicateRemoval))
	{
		attrRefs = getAttrReferences((Node *) ((DuplicateRemoval *) parent)->attrs);
	}
	else if (isA(parent,NestingOperator))
	{
		attrRefs = getAttrReferences((Node *) ((NestingOperator *) parent)->cond);
	}
	else if (isA(parent,WindowOperator))
	{
        WindowOperator *w = (WindowOperator *) parent;
        List *partitionBy = getAttrReferences((Node *) w->partitionBy);
        List *orderBy = getAttrReferences((Node *) w->orderBy);
        List *frameDef = getAttrReferences((Node *) w->frameDef);
        List *f = getAttrReferences((Node *) w->f);
        attrRefs = concatTwoLists(partitionBy, orderBy);
        attrRefs = concatTwoLists(attrRefs, frameDef);
        attrRefs = concatTwoLists(attrRefs, f);
        DEBUG_LOG("WINATTR %p:", (void *) parent);
        DEBUG_NODE_BEATIFY_LOG("", attrRefs);
	}
	else if (isA(parent,OrderOperator))
	{
		attrRefs = getAttrReferences((Node *) ((OrderOperator *) parent)->orderExprs);
	}
    reSetPosOfOpAttrRefBaseOnBelowLayerSchema(child, attrRefs);
}

/*List *
unionEqualElemOfTwoSetList(List *listEqlOp, List *listSet)
{

    FOREACH_LC(lc, listEqlOp)
    {
        if(streq(((Operator *)LC_P_VAL(lc))->name,OPNAME_EQ))
        {
            ListCell *lc1 = getHeadOfList(((Operator *)LC_P_VAL(lc))->args);
            ListCell *lc2 = getTailOfList(((Operator *)LC_P_VAL(lc))->args);
            Node *n1 = (Node *)LC_P_VAL(lc1);
            Node *n2 = (Node *)LC_P_VAL(lc2);

            listSet = addOneEqlOpAttrToListSet(n1,n2,listSet);
        }
    }

    return listSet;
}

List *
addOneEqlOpAttrToListSet(Node *n1,Node *n2,List *listSet)
{
    Node *tempn1, *tempn2;
    if(isA(n1, Constant))
        tempn1 = n1;
    else
        tempn1 = (Node *)(((AttributeReference *)n1)->name);

    if(isA(n2, Constant))
        tempn2 = n2;
    else
        tempn2 = (Node *)(((AttributeReference *)n2)->name);

    Set *tempSet1;
    Set *tempSet2;
    boolean flag1, flag2;

    flag1 = flag2 = FALSE;
    if(!isA(tempn1,Constant))
    {
        FOREACH(Set, s1, listSet)
        {
            FOREACH_SET(Node, sn1, s1)
            {
                if(!isA(sn1,Constant))
                {
                    if(streq((char *)sn1,(char *)tempn1))
                    {
                        flag1 = TRUE;
                        tempSet1 = s1;
                        break;
                    }
                }
            }

        }
    }

    if(isA(tempn2,Constant))
    {
        FOREACH(Set, s2, listSet)
        {
            FOREACH_SET(Node, sn2, s2)
            {
                if(isA(sn2,Constant))
                {
                    int *a = ((Constant *)sn2)->value;
                    int *b = ((Constant *)tempn2)->value;

                    if(*a == *b)
                    {
                        flag2 = TRUE;
                        tempSet2 = s2;
                        break;
                    }
                }
            }
	}
    }
    else
    {
        FOREACH(Set, s2, listSet)
        {
            FOREACH_SET(Node, sn2, s2)
            {
                if(streq((char *)sn2, (char *)tempn2))
                {
                    flag2 = TRUE;
                    tempSet2 = s2;
                    break;
                }
            }
        }
    }

    if(flag1 == TRUE && flag2 == FALSE)
    {
        addToSet(tempSet1,tempn2);
    }
    else if(flag1 == TRUE && flag2 == TRUE)
    {
	Set *uSet = unionSets(tempSet1,tempSet2);
	listSet = REMOVE_FROM_LIST_PTR(listSet, tempSet2);
	listSet = REMOVE_FROM_LIST_PTR(listSet, tempSet1);
	listSet = appendToTailOfList(listSet,uSet);
    }

    return listSet;
}*/

List*
getCondOpList(List *l1, List *l2)
{
    boolean flag1;
    List *newOpList = NIL;

    FOREACH(Operator, o, l2)
    {
        flag1 = FALSE;

        FOREACH(AttributeDef, a, l1)
        {
        	if(isA(getHeadOfListP(o->args),Constant))
        	{
        		flag1 = TRUE;
        		break;
        	}
        	else if(streq(((AttributeReference *)getHeadOfListP(o->args))->name, a->attrName))
        	{
        		flag1 = TRUE;
        		break;
        	}
        }

        if(flag1 == TRUE)
	{
            if(isA(getTailOfListP(o->args),Constant))
	        {
                DEBUG_LOG("test compare constant");
                newOpList = appendToTailOfList(newOpList, o);
            }
            else if(isA(getTailOfListP(o->args),AttributeReference))
            {
                FOREACH(AttributeDef, a, l1)
                {
                    if(streq(((AttributeReference *)getTailOfListP(o->args))->name, a->attrName))
                    {
                        newOpList = appendToTailOfList(newOpList, o);
                        break;
                    }
                }
            }
        }
    }

    return newOpList;
}


List *
getDataTypes(Schema *schema)
{
    return getAttrDataTypes(schema->attrDefs);
}



List *
getAttrNames(Schema *schema)
{
    return getAttrDefNames(schema->attrDefs);
}

List *
getAttrDefNames(List *defs)
{
    List *result = NIL;

    FOREACH(AttributeDef,a,defs)
        result = appendToTailOfList(result, a->attrName);

    return result;
}


List *
getAttrDataTypes(List *defs)
{
    List *result = NIL;

    FOREACH(AttributeDef,a,defs)
        result = appendToTailOfListInt(result, a->dataType);

    return result;
}

List *
inferOpResultDTs (QueryOperator *op)
{
    List *resultDTs = NIL;

    switch(op->type)
    {
        case T_ProjectionOperator:
        {
            ProjectionOperator *o = (ProjectionOperator *) op;
            FOREACH(Node,e,o->projExprs)
            {
                resultDTs = appendToTailOfListInt(resultDTs, typeOf(e));
            }
        }
        break;
        case T_JoinOperator:
        {
            resultDTs = CONCAT_LISTS(getDataTypes(GET_OPSCHEMA(OP_LCHILD(op))), getDataTypes(GET_OPSCHEMA(OP_RCHILD(op))));
        }
        break;
        case T_AggregationOperator:
        {
            AggregationOperator *o = (AggregationOperator *) op;
            FOREACH(Node,e,o->aggrs)
            {
                resultDTs = appendToTailOfListInt(resultDTs, typeOf(e));
            }
            FOREACH(Node,e,o->groupBy)
            {
                resultDTs = appendToTailOfListInt(resultDTs, typeOf(e));
            }
        }
        break;
        case T_WindowOperator:
        {
            WindowOperator *o = (WindowOperator *) op;
            resultDTs = getDataTypes(GET_OPSCHEMA(OP_LCHILD(op)));
            resultDTs = appendToTailOfListInt(resultDTs, typeOf(o->f));
        }
        break;
        case T_SelectionOperator:
        case T_OrderOperator:
        case T_DuplicateRemoval:
        case T_SetOperator:
        {
            resultDTs = getDataTypes(GET_OPSCHEMA(OP_LCHILD(op)));
        }
        break;
                // Check Attribute that we use as Json Column should be from/should exist in child
        case T_JsonTableOperator:
        {
            JsonTableOperator *o = (JsonTableOperator *)op;
            resultDTs = getDataTypes(GET_OPSCHEMA(OP_LCHILD(op)));
            for(int i = 0; i < LIST_LENGTH(o->columns); i++)
                resultDTs = appendToTailOfListInt(resultDTs, DT_VARCHAR2); //TODO until more types supported
        }
        break;
        case T_TableAccessOperator:
        {
            resultDTs = getDataTypes(GET_OPSCHEMA(op));
        }
        break;
        case T_SampleClauseOperator:
        {
            resultDTs = getDataTypes(GET_OPSCHEMA(op));
        }
        break;
        case T_ProvenanceComputation:
        {
            resultDTs = getDataTypes(GET_OPSCHEMA(op));//TODO
        }
        break;
        case T_NestingOperator:
        {
            NestingOperator *n = (NestingOperator *) op;
            DataType nType;

            resultDTs = getDataTypes(GET_OPSCHEMA(OP_LCHILD(op)));

            switch(n->nestingType)
            {
                case NESTQ_EXISTS:
                case NESTQ_ANY:
                case NESTQ_ALL:
                case NESTQ_UNIQUE:
                {
                    nType = DT_BOOL;
                }
                case NESTQ_SCALAR:
                case NESTQ_LATERAL:
                {
                    nType = DT_STRING; //TODO
                }
                break;
            }

            resultDTs = appendToTailOfListInt(resultDTs, nType);
        }
        break;
        case T_ConstRelOperator:
        {
            ConstRelOperator *c = (ConstRelOperator *) op;
            FOREACH(Node,v,c->values)
            {
                resultDTs = appendToTailOfListInt(resultDTs, typeOf(v));
            }
        }
        break;
        default:
            FATAL_LOG("needs to be implemented!");
            break;
    }
    return resultDTs;
}


QueryOperator *
shallowCopyQueryOperator(QueryOperator *op)
{
	QueryOperator *result;
	List *parents = op->parents;
	List *children = op->inputs;

	op->parents = NIL;
	op->inputs = NIL;

	result = copyObject(op);

	op->parents = parents;
	op->inputs = children;

	return result;
}

TableAccessOperator *
createTableAccessOp(char *tableName, Node *asOf, char *alias, List *parents,
        List *attrNames, List *dataTypes)
{
    TableAccessOperator *ta = makeNode(TableAccessOperator);

    ta->tableName = tableName;
    ta->asOf = asOf;
    ta->op.inputs = NULL;
    ta->op.schema = createSchemaFromLists(alias, attrNames, dataTypes);
    ta->op.parents = parents;
    ta->op.provAttrs = NIL;
//    ta->sampClause = sampClause;

    return ta;
}

SampleClauseOperator *
createSampleClauseOp(QueryOperator *input, Node *sampPerc, List *attrNames, List *dataTypes)
{
	SampleClauseOperator *sc = makeNode(SampleClauseOperator);

	sc->sampPerc = sampPerc;
	sc->op.inputs = singleton(input);
	sc->op.schema = createSchemaFromLists(strdup("SAMPLE"), attrNames, dataTypes);

	return sc;
}

JsonTableOperator *
createJsonTableOperator(FromJsonTable *fjt)
{
    JsonTableOperator *jt = makeNode(JsonTableOperator);

    jt->op.inputs = NULL;
    jt->op.schema = createSchemaFromLists(fjt->from.name, fjt->from.attrNames, fjt->from.dataTypes);
    jt->op.parents = NIL;
    jt->op.provAttrs = NIL;

    jt->columns = fjt->columns;
    jt->documentcontext = fjt->documentcontext;
    jt->jsonColumn = fjt->jsonColumn;
    jt->jsonTableIdentifier = fjt->jsonTableIdentifier;

    return jt;
}

SelectionOperator *
createSelectionOp(Node *cond, QueryOperator *input, List *parents,
        List *attrNames)
{
    SelectionOperator *sel = makeNode(SelectionOperator);

    sel->cond = copyObject(cond);
    if (input != NULL)
        sel->op.inputs = singleton(input);
    else
        sel->op.inputs = NIL;

    if (attrNames == NIL && input)
        attrNames = getQueryOperatorAttrNames(input);

    sel->op.schema = createSchemaFromLists("SELECT", attrNames,
            input ? getDataTypes(input->schema) : NIL);

    sel->op.parents = parents;
    sel->op.provAttrs = NIL;

    return sel;
}

ProjectionOperator *
createProjectionOp(List *projExprs, QueryOperator *input, List *parents,
        List *attrNames)
{
    ProjectionOperator *prj = makeNode(ProjectionOperator);

	prj->projExprs = copyObject(projExprs);

    if (input != NULL)
        prj->op.inputs = singleton(input);
    else
        prj->op.inputs = NIL;
    prj->op.schema = schemaFromExpressions("PROJECTION", attrNames, projExprs,
            singleton(input));

    prj->op.parents = parents;
    prj->op.provAttrs = NIL;

    return prj;
}

JoinOperator *
createJoinOp(JoinType joinType, Node *cond, List *inputs, List *parents,
        List *attrNames)
{
    JoinOperator *join = makeNode(JoinOperator);

    join->cond = copyObject(cond);
    join->joinType = joinType;
    join->op.inputs = inputs;
    /* get data types from inputs and attribute names from parameter to create
     * schema */
    List *lDT, *rDT;
    lDT = getDataTypes(OP_LCHILD(join)->schema);
    rDT = getDataTypes(OP_RCHILD(join)->schema);

    // if no attribute names are given use attributes from children
    if (attrNames == NIL)
    {
        attrNames = getAttrNames(OP_LCHILD(join)->schema);
        attrNames = CONCAT_LISTS(attrNames, getAttrNames(OP_RCHILD(join)->schema));
    }

    join->op.schema = createSchemaFromLists("JOIN", attrNames, concatTwoLists(lDT, rDT));

    join->op.parents = parents;
    join->op.provAttrs = NULL;

    return join;
}

AggregationOperator *
createAggregationOp(List *aggrs, List *groupBy, QueryOperator *input,
        List *parents, List *attrNames)
{
    AggregationOperator *aggr = makeNode(AggregationOperator);

    FOREACH(Node, func, aggrs)
    {
        aggr->aggrs = appendToTailOfList(aggr->aggrs, copyObject(func));
    }
    FOREACH(Node, expr, groupBy)
    {
        aggr->groupBy = appendToTailOfList(aggr->groupBy, copyObject(expr));
    }
    if (input != NULL)
        aggr->op.inputs = singleton(input);
    else
        aggr->op.inputs = NIL;

    aggr->op.schema = schemaFromExpressions("AGG", attrNames,
            concatTwoLists(copyList(aggrs),copyList(groupBy)), singleton(input));
    aggr->op.parents = parents;
    aggr->op.provAttrs = NULL;

    return aggr;
}

SetOperator *
createSetOperator(SetOpType setOpType, List *inputs, List *parents,
        List *attrNames)
{
    SetOperator *set = makeNode(SetOperator);
    QueryOperator *lChild;

    set->setOpType = setOpType;
    set->op.inputs = inputs;
    lChild = OP_LCHILD(set);
    set->op.schema = createSchemaFromLists("SET", attrNames,
            lChild ? getDataTypes(lChild->schema) : NIL);
    set->op.parents = parents;
    set->op.provAttrs = NULL;

    return set;
}

DuplicateRemoval *
createDuplicateRemovalOp(List *attrs, QueryOperator *input, List *parents,
        List *attrNames)
{
    DuplicateRemoval *dr = makeNode(DuplicateRemoval);

    if (attrNames == NIL)
        attrNames = getQueryOperatorAttrNames(input);

    dr->attrs = attrs;
    dr->op.inputs = singleton(input);
    dr->op.schema = createSchemaFromLists("DUPREM", attrNames, getDataTypes(input->schema));
    dr->op.parents = parents;
    dr->op.provAttrs = NULL;

    return dr;
}

ProvenanceComputation *
createProvenanceComputOp(ProvenanceType provType, List *inputs, List *parents, List *attrNames, List *dts, Node *asOf)
{
    ProvenanceComputation *p = makeNode(ProvenanceComputation);

    p->op.parents = parents;
    p->op.inputs = inputs;
    p->op.schema = createSchemaFromLists("PROVENANCE", attrNames, dts);
    p->provType = provType;
    p->asOf = asOf;

    return p;
}

ConstRelOperator *
createConstRelOp(List *values, List *parents, List *attrNames, List *dataTypes)
{
    ConstRelOperator *co = NEW(ConstRelOperator);

    co->values=values;
    co->op.type=T_ConstRelOperator;
    co->op.inputs=NULL;

    if (dataTypes == NIL)
    {
        FOREACH(Node,v, values)
        {
            dataTypes = appendToTailOfListInt(dataTypes, typeOf(v));
        }
    }

    co->op.schema= createSchemaFromLists("ConstRel", attrNames, dataTypes);
    co->op.parents=parents;
    co->op.provAttrs=NIL;

    return co;
}

NestingOperator *
createNestingOp(NestingExprType nestingType, Node *cond, List *inputs, List *parents, List *attrNames, List *dts)
{
    NestingOperator *no = makeNode(NestingOperator);
    no->nestingType = nestingType;
    no->cond = copyObject(cond);
    no->op.type = T_NestingOperator;
    no->op.inputs = inputs;
    no->op.schema = createSchemaFromLists("NESTING", attrNames, dts);
    no->op.parents = parents;
    no->op.provAttrs = NIL;

    return no;
}

WindowOperator *
createWindowOp(Node *fCall, List *partitionBy, List *orderBy,
        WindowFrame *frameDef, char *attrName, QueryOperator *input,
        List *parents)
{
    WindowOperator *wo = makeNode(WindowOperator);
    List *inputAttrs = getQueryOperatorAttrNames(input);
    List *inputDTs = getDataTypes(input->schema);

    wo->partitionBy = partitionBy;
    wo->orderBy = orderBy;
    wo->frameDef = frameDef;
    wo->attrName = attrName;
    wo->f = (Node *) fCall;
    wo->op.type = T_WindowOperator;
    wo->op.inputs = singleton(input);
    wo->op.schema = createSchemaFromLists("WINDOW",
            CONCAT_LISTS(inputAttrs, singleton(attrName)),
            CONCAT_LISTS(inputDTs, singletonInt(typeOf(wo->f))));
    wo->op.parents = parents;
    wo->op.provAttrs = NIL;

    return wo;
}

OrderOperator *
createOrderOp(List *orderExprs, QueryOperator *input, List *parents)
{
    OrderOperator *o = makeNode(OrderOperator);
    List *inputAttrs = getQueryOperatorAttrNames(input);
    List *dts = getDataTypes(input->schema);

    o->orderExprs = orderExprs;
    o->op.type = T_OrderOperator;
    o->op.inputs = singleton(input);
    o->op.schema =  createSchemaFromLists("ORDER", inputAttrs, dts);
    o->op.parents = parents;
    o->op.provAttrs = NIL;

    return o;
}

LimitOperator *
createLimitOp(Node *limitExpr, Node *offsetExpr, QueryOperator *input, List *parents)
{
	LimitOperator *o = makeNode(LimitOperator);

	o->limitExpr = limitExpr;
	o->offsetExpr = offsetExpr;
	if (input != NULL)
	{
		List *inputAttrs = getQueryOperatorAttrNames(input);
		List *dts = getDataTypes(input->schema);
		o->op.inputs = singleton(input);
		o->op.schema = createSchemaFromLists("LIMIT", inputAttrs, dts);
	}
	o->op.parents = parents;

	return o;
}

QueryOperator *
getFirstRoot(QueryOperator *op)
{
	if(LIST_LENGTH(op->parents) > 0)
	{
		return getFirstRoot(getHeadOfListP(op->parents));
	}
	return op;
}

void
setProperty (QueryOperator *op, Node *key, Node *value)
{
    if (op->properties == NULL)
    {
        op->properties = (Node *) NEW_MAP(Node,Node);
    }

    KeyValue *val = getProp(op, key);

    if (val)
    {
        val->value = value;
        return;
    }

    addToMap((HashMap *) op->properties, key, value);
}

Node *
getProperty (QueryOperator *op, Node *key)
{
    KeyValue *kv = getProp(op, key);

    return kv ? kv->value : NULL;
}

void
setStringProperty (QueryOperator *op, char *key, Node *value)
{
    setProperty(op, (Node *) createConstString(key), value);
}

Node *
getStringProperty (QueryOperator *op, char *key)
{
    if (op->properties == NULL)
        op->properties = (Node *) NEW_MAP(Node,Node);
    return getMapString((HashMap *) op->properties, key);
}

void
removeStringProperty (QueryOperator *op, char *key)
{
    removeMapStringElem((HashMap *) op->properties, key);
}

List *
appendToListProperty(QueryOperator *op, Node *key, Node *newTail)
{
	List *cur = (List *) getProp(op, key);

	cur = appendToTailOfList(cur, newTail);
	setProperty(op, key, (Node *) cur);

	return cur;
}

List *
appendToListStringProperty(QueryOperator *op, char *key, Node *newTail)
{
	List *cur = (List *) getStringProperty(op, key);

	cur = appendToTailOfList(cur, newTail);
	setStringProperty(op, key, (Node *) cur);

	return cur;
}


static KeyValue *
getProp (QueryOperator *op, Node *key)
{
    if (op->properties == NULL)
    {
        op->properties = (Node *) NEW_MAP(Node,Node);
    }

    return getMapEntry((HashMap *) op->properties, key);
//    if (mapHasKey(op->properties, key))
//        return mpa
//    FOREACH(KeyValue,p,(List *) op->properties)
//    {
//        if (equal(p->key,key))
//            return p;
//    }

//    return NULL;
}

void
addChildOperator (QueryOperator *parent, QueryOperator *child)
{
    parent->inputs = appendToTailOfList(parent->inputs, child);
    child->parents = appendToTailOfList(child->parents, parent);
}

void
addParent (QueryOperator *child, QueryOperator *parent)
{
    if (!searchList(child->parents, parent))
        child->parents = appendToTailOfList(child->parents, parent);
}

void
removeParent (QueryOperator *child, QueryOperator *parent)
{
    child->parents = REMOVE_FROM_LIST_PTR(child->parents, parent);
}

int
getChildPosInParent (QueryOperator *parent, QueryOperator *child)
{
    int i = 0;

    FOREACH(QueryOperator,o,parent->inputs)
    {
        if (o == child)
            return i;
        i++;
    }

    return -1;
}

List *
getProvenanceAttrs(QueryOperator *op)
{
    return op ? op->provAttrs : NIL;
}

List *
getProvenanceAttrDefs(QueryOperator *op)
{
    List *result = NIL;

    FOREACH_INT(i,op->provAttrs)
    {
        //DEBUG_LOG("prov attr at <%u> is <%s>", i, nodeToString(getNthOfListP(op->schema->attrDefs, i)));
        result = appendToTailOfList(result, getNthOfListP(op->schema->attrDefs, i));
    }

    return result;
}

List *
getProvenanceAttrReferences(ProjectionOperator *op, QueryOperator *op1)
{
    List *result = NIL;

    FOREACH_INT(i,op1->provAttrs)
    {
        //DEBUG_LOG("prov attr at <%u> is <%s>", i, nodeToString(getNthOfListP(op->projExprs, i)));
        result = appendToTailOfList(result, getNthOfListP(op->projExprs, i));
    }
    return result;
}

List *
getOpProvenanceAttrNames(QueryOperator *op)
{
    List *provDefs = getProvenanceAttrDefs(op);
    List *result = NIL;

    FOREACH(AttributeDef,a,provDefs)
    result = appendToTailOfList(result, strdup(a->attrName));

    return result;
}

int
getNumProvAttrs(QueryOperator *op)
{
    return LIST_LENGTH(op->provAttrs);
}

List *
getNormalAttrs(QueryOperator *op)
{
    if(op == NULL || op->schema == NULL || op->schema->attrDefs == NIL)
        return NIL;

    List *result = NIL;
    int pos = 0;

    FOREACH(AttributeDef, a, op->schema->attrDefs)
    {
        if(!searchListInt(op->provAttrs, pos))
            result = appendToTailOfList(result, a);
        pos++;
    }

    return result;
}

List *
getNormalAttrReferences(ProjectionOperator *op, QueryOperator *op1)
{
    List *result = NIL;
    int pos = 0;

    FOREACH(AttributeReference, a, op->projExprs)
    {
        if(!searchListInt(op1->provAttrs, pos))
            result = appendToTailOfList(result, a);
        pos++;
    }

    return result;
}

List *
getNormalAttrNames(QueryOperator *op)
{
    List *defs = getNormalAttrs(op);
    List *result = NIL;

    FOREACH(AttributeDef, a, defs)
    	result = appendToTailOfList(result, strdup(a->attrName));

    return result;
}

List *
getAttrRefNames(ProjectionOperator *op)
{
   List *result = NIL;

   FOREACH(AttributeReference, a, op->projExprs)//FIXME will break if not just attribute references
      result = appendToTailOfList(result, strdup(a->name));

   return result;
}

//e.g. op = a + (b+2), then aNameOpList = {a,b}
List *
getAttrNameFromOpExpList(List *aNameOpList, Operator *opExpList)
{
	Node *left  = (Node *)getHeadOfListP(opExpList->args);
	Node *right = (Node *)getTailOfListP(opExpList->args);

	if(isA(left, Operator))
	{
		aNameOpList = getAttrNameFromOpExpList(aNameOpList, (Operator *)left);
	}
	else if(isA(left, AttributeReference))
	{
		aNameOpList = appendToTailOfList(aNameOpList, ((AttributeReference *)left)->name);
	}

	if(isA(right, Operator))
	{
		aNameOpList = getAttrNameFromOpExpList(aNameOpList, (Operator *)right);
	}
	else if(isA(right, AttributeReference))
	{
		aNameOpList = appendToTailOfList(aNameOpList, ((AttributeReference *)right)->name);
	}

	return aNameOpList;

}

/*
 * dif with getAttrRefNames, which contains rename such as
 * A+B+2 AS X, So the list will contain {A,B}
 * But contain duplicate, such as
 * A+B AS X, A, then {A, B, A}, but if change List to set, duplicate should be removed
 */
List *
getAttrRefNamesContainOps(ProjectionOperator *op)
{
   List *result = NIL;

   FOREACH(Node, a, op->projExprs)
   {
	   if(isA(a, Operator))
	   {
		   result = getAttrNameFromOpExpList(result, (Operator *)a);
	   }
	   else if(isA(a, AttributeReference))
	   {
		   result = appendToTailOfList(result, strdup(((AttributeReference *)a)->name));
	   }
   }

   return result;
}

int
getNumNormalAttrs(QueryOperator *op)
{
    return getNumAttrs(op) - LIST_LENGTH(op->provAttrs);
}


List *
getQueryOperatorAttrNames (QueryOperator *op)
{
    List *result = NIL;

    FOREACH(AttributeDef,a,op->schema->attrDefs)
    result = appendToTailOfList(result, strdup(a->attrName));

    return result;
}

int
getNumAttrs(QueryOperator *op)
{
    return LIST_LENGTH(op->schema->attrDefs);
}

int
getAttrPos(QueryOperator *op, char *attr)
{
    int i = 0;
    FOREACH(AttributeDef,a,op->schema->attrDefs)
    {
        if (strcmp(a->attrName, attr) == 0)
            return i;
        i++;
    }

    return -1;
}

AttributeDef *
getAttrDefByName(QueryOperator *op, char *attr)
{
    FOREACH(AttributeDef,a,op->schema->attrDefs)
    {
        if (strcmp(a->attrName, attr) == 0)
            return a;
    }

    return NULL;
}

AttributeDef *
getAttrDefByPos(QueryOperator *op, int pos)
{
    ASSERT(pos >= 0 && pos < LIST_LENGTH(op->schema->attrDefs));

    return (AttributeDef *) getNthOfListP(op->schema->attrDefs, pos);
}

char *
getAttrNameByPos(QueryOperator *op, int pos)
{
    return getAttrDefByPos(op, pos)->attrName;
}

AttributeReference *
getAttrRefByPos (QueryOperator *op, int pos)
{
    AttributeDef *d = getAttrDefByPos(op, pos);

    ASSERT(d != NULL);

    AttributeReference *res = createFullAttrReference(strdup(d->attrName), 0,
                pos,
                INVALID_ATTR,
                d->dataType);

    return res;
}

AttributeReference *
getAttrRefByName(QueryOperator *op, char *attr)
{
    AttributeDef *d = getAttrDefByName(op, attr);

    ASSERT_WITH_MESSAGE(d != NULL,
                        "did for not attr %s in op <%s>",
                        attr,
                        singleOperatorToOverview(op));

    AttributeReference *res = createFullAttrReference(strdup(d->attrName), 0,
            getAttrPos(op, attr),
            0,
            d->dataType);

    return res;
}

List *
getAttrRefsInOperator(QueryOperator *op)
{
    List *refs = NIL;

    switch(op->type)
    {
        case T_TableAccessOperator:
            break;
        case T_ProjectionOperator:
        {
            ProjectionOperator *p = (ProjectionOperator *) op;
            refs = getAttrReferences((Node *)p->projExprs);
        }
        break;
        case T_SelectionOperator:
        {
            SelectionOperator *p = (SelectionOperator *) op;
            refs = getAttrReferences((Node *)p->cond);
        }
        break;
        case T_JoinOperator:
        {
            JoinOperator *p = (JoinOperator *) op;
            refs = getAttrReferences((Node *)p->cond);
        }
        break;
        case T_AggregationOperator:
        {
            AggregationOperator *p = (AggregationOperator *) op;
            refs = CONCAT_LISTS(getAttrReferences((Node *)p->aggrs),
                    getAttrReferences((Node *)p->groupBy));
        }
        break;
        case T_DuplicateRemoval:
        {
            DuplicateRemoval *p = (DuplicateRemoval *) op;
            refs = getAttrReferences((Node *)p->attrs);
        }
        break;
        case T_WindowOperator:
        {
            WindowOperator *p = (WindowOperator *) op;
            refs = CONCAT_LISTS(getAttrReferences((Node *)p->f),
                    getAttrReferences((Node *)p->partitionBy),
                    getAttrReferences((Node *)p->orderBy),
                    getAttrReferences((Node *)p->frameDef));
        }
        break;
        case T_NestingOperator:
		{
            //TODO do not traverse into query operator
			NestingOperator *n = (NestingOperator *) op;
			refs = getAttrReferences(n->cond);
		}
		break;
        case T_OrderOperator:
        {
            OrderOperator *p = (OrderOperator *) op;
            refs = getAttrReferences((Node *)p->orderExprs);
        }
        break;
        case T_ConstRelOperator:
        case T_SetOperator:
        default:
            break;
    }

    return refs;
}

boolean
opReferencesAttr(QueryOperator *op, char *a)
{
	List *refs = getAttrRefsInOperator(op);

	FOREACH(AttributeReference,ar,refs)
	{
		if(streq(ar->name, a))
		{
			return TRUE;
		}
	}

	return FALSE;
}

List *
aggOpGetGroupByAttrNames(AggregationOperator *op)
{
    List *result = getQueryOperatorAttrNames((QueryOperator *) op);

    return sublist(result, LIST_LENGTH(op->aggrs), LIST_LENGTH(op->aggrs) + LIST_LENGTH(op->groupBy) - 1);
}

List *
aggOpGetAggAttrNames(AggregationOperator *op)
{
    List *result = getQueryOperatorAttrNames((QueryOperator *) op);

    if (LIST_LENGTH(op->aggrs) == 0)
        return NIL;

    return sublist(result, 0, LIST_LENGTH(op->aggrs) - 1);
}

List *
aggOpGetGroupByAttrDefs(AggregationOperator *op)
{
    List *result = (List *) copyObject(((QueryOperator *) op)->schema->attrDefs);

    return sublist(result, LIST_LENGTH(op->aggrs), LIST_LENGTH(op->aggrs) + LIST_LENGTH(op->groupBy) - 1);
}

List *
aggOpGetAggAttrDefs(AggregationOperator *op)
{
	List *result = (List *) copyObject(((QueryOperator *) op)->schema->attrDefs);

    if (LIST_LENGTH(op->aggrs) == 0)
        return NIL;

    return sublist(result, 0, LIST_LENGTH(op->aggrs) - 1);
}


WindowFunction *
winOpGetFunc (WindowOperator *op)
{
    return createWindowFunction(copyObject(op->f),
            (WindowDef *) copyObject(createWindowDef(
                    op->partitionBy, op->orderBy, op->frameDef)));
}

List *
getProjExprsForAttrNames(QueryOperator *op, List *names)
{
	List *result = NIL;

	FOREACH(char,name,names)
	{
		result = appendToTailOfList(result,
									getAttrRefByName(op, name));
	}

	return result;
}

List *
getProjExprsForAllAttrs(QueryOperator *op)
{
	List *attrNames = getQueryOperatorAttrNames(op);
	return getProjExprsForAttrNames(op, attrNames);
}


List *
getProjResultAttrNamesForProjExpr(ProjectionOperator *op, Node *expr)
{
    List *result;
	List *names = getQueryOperatorAttrNames((QueryOperator *) op);
	int pos = 0;

	FOREACH(Node,pe,op->projExprs)
	{
		if(equal(pe, expr))
		{
			result = appendToTailOfList(result, getNthOfListP(names, pos));
		}
		pos++;
	}

	return result;
}


List *
getNestingResultAttributeNames(NestingOperator *op)
{
	List *attrs = getQueryOperatorAttrNames((QueryOperator *) op);
	int numOuterAttrs = getNumAttrs(OP_LCHILD(op));

	// return result attributes that are not from the outer (left input)
	attrs = sublist(attrs, numOuterAttrs, -1);

	return attrs;
}

char *
getSingleNestingResultAttribute(NestingOperator *op)
{
	ASSERT(getNumAttrs(OP_LCHILD(op)) + 1 == getNumAttrs((QueryOperator *) op));
	return getTailOfListP(getQueryOperatorAttrNames((QueryOperator *) op));
}


/**
 * @brief Determine all correlated attributes from the outer query.
 *
 * The default is to only look at correlatations with the op's outer query.
 *
 * @param op the nesting operator
 * @param corrInSubquery if TRUE, then also find correlation within the nested query itself
 * @return set of correlated attribute names
 */

Set *
getNestingCorrelatedAttributes(NestingOperator *op, boolean corrInSubquery) // boolean traverseIntoNestingOperators
{
	Set *result = STRSET();
	CorrelatedAttrsState state = { 1, result, corrInSubquery };

	findCorrelatedAttrsVisitor((Node *) OP_RCHILD(op), &state);

	return result;
}

Set *
getCorrelatedAttributes(Node *op, boolean corrInSubquery) // boolean traverseIntoNestingOperators
{
	Set *result = STRSET();
	CorrelatedAttrsState state = { 1, result, corrInSubquery };

	findCorrelatedAttrsVisitor((Node *)op, &state);

	return result;
}

boolean
noCorrelationBelowNormalization(Node *op, boolean corrInSubquery) // boolean traverseIntoNestingOperators
{
	Set *result = STRSET();

	CorrelatedAttrsState state = { 1, result, corrInSubquery, TRUE };

	canStayCorrelatedVisitor((Node *)op, &state);

	return state.childrenCanBeKeptCorrelated;
}


static boolean
findCorrelatedAttrsVisitor(Node *n, CorrelatedAttrsState *state)
{
	if (n == NULL)
		return TRUE;

	//TODO
	// rules: N can be kept if (i) all of N's childresn can be kept, if (ii) all  of N's correlated attributes are one level up only, and (iii) we can push normalization below N's correlated attributes
	//   N [CAN KEEP SUBQUERY] depends on CAN KEEP SUBQUERY
	//      SELECTION[R.A = R.C] -- R is outer
	//         JOIN
	//             N' [CAN KEEP SUBQUERY] correlated attribute
	//                SELECTION[S.B[1] = T.C[0]] -- T is inner most
	//                     T
	//                X
	//             S
	//      R
	/* List *attrRefs = queryOperatorGetAttrRefs(op); // --> current operator */
	/* FOREACH(AttributeReference,a,attrRefs) */
	/* { */
	/* 	if(a->outerLevelsUp != 0) */
	/* 	{ */
	/* 		addToSet(state->result, a); */
	/* 		//TODO append to state */
	/* 	} */
	/* } */

	/* visitQOGraph(op, TRAVERSAL_PRE, findCorrelatedAttrsVisitor , state); */

	if(isA(n, AttributeReference))
	{
		AttributeReference *a = (AttributeReference *) n;

		if(a->outerLevelsUp == state->curDepth
		   || (a->outerLevelsUp > 0 && state->corrInsideSubquery))
		{
			addToSet(state->result, strdup(a->name));
		}
	}
	if(isA(n, NestingOperator))
	{
		NestingOperator *no = (NestingOperator *) n;
		CorrelatedAttrsState newState = *state;
		newState.curDepth++;

		visit(no->cond, findCorrelatedAttrsVisitor, state);
        visit((Node *) OP_LCHILD(no), findCorrelatedAttrsVisitor, state);
		return visit((Node *) OP_RCHILD(no), findCorrelatedAttrsVisitor, &newState);
	}

	return visit(n, findCorrelatedAttrsVisitor, state);
}


static boolean
canStayCorrelatedVisitor(Node *n, CorrelatedAttrsState *state)
{
	if (n == NULL)
		return TRUE;

	//TODO
	// rules: N can be kept if (i) all of N's childresn can be kept,
    // if (ii) all  of N's correlated attributes are one level up only,
    // and (iii) we can push normalization below N's correlated attributes
    //
	//   N [CAN KEEP SUBQUERY] depends on CAN KEEP SUBQUERY
	//      SELECTION[R.A = R.C] -- R is outer
	//         JOIN
	//             N' [CAN KEEP SUBQUERY] correlated attribute
	//                SELECTION[S.B[1] = T.C[0]] -- T is inner most
	//                     T
	//                X
	//             S
	//      R
	/* List *attrRefs = queryOperatorGetAttrRefs(op); // --> current operator */
	/* FOREACH(AttributeReference,a,attrRefs) */
	/* { */
	/* 	if(a->outerLevelsUp != 0) */
	/* 	{ */
	/* 		addToSet(state->result, a); */
	/* 		//TODO append to state */
	/* 	} */
	/* } */

	/* visitQOGraph(op, TRAVERSAL_PRE, findCorrelatedAttrsVisitor , state); */

	if(isA(n, NestingOperator))
	{
		NestingOperator *no = (NestingOperator *) n;
		CorrelatedAttrsState newState = *state;
		newState.curDepth++;
        newState.seenNormalization = FALSE; // Reset as we descend into children nesting operators

        if(no->cond) {
            visit(no->cond, canStayCorrelatedVisitor, state);
        }
        visit((Node *) OP_LCHILD(no), canStayCorrelatedVisitor, state);
		visit((Node *) OP_RCHILD(no), canStayCorrelatedVisitor, &newState);
        state->childrenCanBeKeptCorrelated &= newState.childrenCanBeKeptCorrelated;
        return TRUE;
	}
    else if(IS_OP(n)) {
        QueryOperator *q = (QueryOperator *) n;

        if(isA(q->properties, HashMap) && MAP_HAS_STRING_KEY((HashMap *)(q->properties), "normalize")) {
            state->seenNormalization = TRUE;
            FOREACH(QueryOperator, a, q->inputs) {
                visit((Node *)a, canStayCorrelatedVisitor, state);
            }
            state->seenNormalization = FALSE;
            return TRUE;
        }
    }
	else if(isA(n, AttributeReference))
	{
		AttributeReference *a = (AttributeReference *) n;

		if(a->outerLevelsUp > 0)
		{
			addToSet(state->result, strdup(a->name));
            // We assume that at the time of calling the normalization has already been pushed down as far as it can go (assuming this part of the state is relevant)
            state->childrenCanBeKeptCorrelated = (!state->seenNormalization && a->outerLevelsUp <= 1) && state->childrenCanBeKeptCorrelated;
		}
	}


	return visit(n, canStayCorrelatedVisitor, state);
}


void
treeify(QueryOperator *op)
{
    FOREACH(QueryOperator,child,op->inputs)
		treeify(child);

    // if operator has more than one parent, then we need to duplicate the subtree under this operator
    if (LIST_LENGTH(op->parents) > 1)
    {
        INFO_LOG("operator has more than one parent %s", operatorToOverviewString((Node *) op));
		Set *parentSet = NODESET();

        FOREACH(QueryOperator,parent,op->parents)
		{
			addToSet(parentSet, parent);
		}

        FOREACH_SET(QueryOperator,parent,parentSet)
        {
			// check for special case where a binary parent has the operators as both of its inputs
			if(equal(parent->inputs,LIST_MAKE(op, op)))
			{
				QueryOperator *cp1 = copyUnrootedSubtree(op);
				QueryOperator *cp2 = copyUnrootedSubtree(op);
				cp1->parents = singleton(parent);
				cp2->parents = singleton(parent);
				parent->inputs = LIST_MAKE(cp1, cp2);
			}
			else
			{
				QueryOperator *copy = copyUnrootedSubtree(op);
				replaceNode(parent->inputs, op, copy);
				copy->parents = singleton(parent);
			}
        }
        op->parents = NIL;
    }
}

boolean
visitQOGraph (QueryOperator *q, TraversalOrder tOrder,
        boolean (*visitF) (QueryOperator *op, void *context), void *context)
{
    boolean result = FALSE;
    NEW_AND_ACQUIRE_MEMCONTEXT("QO_GRAPH_VISITOR_CONTEXT");
    Set *haveSeen = PSET();
    result = internalVisitQOGraph(q, tOrder, visitF, context, haveSeen);
    FREE_AND_RELEASE_CUR_MEM_CONTEXT();
    return result;
}

static boolean
internalVisitQOGraph (QueryOperator *q, TraversalOrder tOrder,
        boolean (*visitF) (QueryOperator *op, void *context), void *context, Set *haveSeen)
{
    if (tOrder == TRAVERSAL_PRE && !visitF(q, context))
        return FALSE;

    FOREACH(QueryOperator,c,q->inputs)
    {
        if (!hasSetElem(haveSeen, c))
        {
            boolean result = FALSE;
            MemContext *ctxt;
            addToSet(haveSeen, c);
            ctxt = RELEASE_MEM_CONTEXT();
            result = internalVisitQOGraph(c, tOrder, visitF, context, haveSeen);
            ACQUIRE_MEM_CONTEXT(ctxt);
            if (!result)
                return FALSE;
        }
    }

    if (tOrder == TRAVERSAL_POST && !visitF(q, context))
        return FALSE;

    return TRUE;
}


unsigned int
numOpsInGraph (QueryOperator *root)
{
    List *ctx = LIST_MAKE(createConstInt(0), PSET());

    visitQOGraph(root, TRAVERSAL_PRE, countUniqueOpsVisitor, ctx);
    Constant *c = (Constant *) getNthOfListP(ctx, 0);
    return INT_VALUE(c);
}


static boolean
countUniqueOpsVisitor(QueryOperator *op, void *context)
{
    List *l = (List *) context;
    Constant *c = (Constant *) getNthOfListP(l, 0);
    Set *s = (Set *) getNthOfListP(l, 1);

    if (!hasSetElem(s,op))
    {
        addToSet(s,op);
        (INT_VALUE(c))++;
    }
    return TRUE;
}

#define PROP_CHILD_COUNT "CC"

unsigned int
numOpsInTree (QueryOperator *root)
{
    unsigned int result = 0;
    NEW_AND_ACQUIRE_MEMCONTEXT("QO_GRAPH_VISITOR_CONTEXT");
    numOpsInTreeInternal(root, &result);
    removeProp(root, PROP_CHILD_COUNT);
    FREE_AND_RELEASE_CUR_MEM_CONTEXT();
    return result;
}

static unsigned int
numOpsInTreeInternal (QueryOperator *q, unsigned int *count)
{
    unsigned int opC = 1;
    if (HAS_STRING_PROP(q, PROP_CHILD_COUNT))
    {
        opC = INT_VALUE(GET_STRING_PROP(q, PROP_CHILD_COUNT));
        (*count) += opC;
        return opC;
    }

    FOREACH(QueryOperator,c,q->inputs)
        opC += numOpsInTreeInternal(c, count);
    SET_STRING_PROP(q, PROP_CHILD_COUNT, createConstInt(opC));
    return opC;
}

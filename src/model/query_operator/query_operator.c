/*-------------------------------------------------------------------------
 *
 * query_operator.c
 *    Author: Ying Ni yni6@hawk.iit.edu
 *    One-line description
 *
 *        Here starts the more detailed description where we
 *        explain in more detail how this works.
 *
 *-------------------------------------------------------------------------
 */

#include "common.h"
#include "log/logger.h"
#include "model/query_operator/query_operator.h"
#include "mem_manager/mem_mgr.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "provenance_rewriter/prov_utility.h"
#include "model/set/set.h"
#include "model/query_operator/operator_property.h"


//static Schema *mergeSchemas (List *inputs);
static Schema *schemaFromExpressions (char *name, List *attributeNames, List *exprs, List *inputs);
static KeyValue *getProp (QueryOperator *op, Node *key);

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
    result->pos = 0;

    return result;
}

Schema *
createSchemaFromLists (char *name, List *attrNames, List *dataTypes)
{
    Schema *result = makeNode(Schema);

    result->name = strdup(name);
    result->attrDefs = NIL;

    if (dataTypes == NULL)
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

static Schema *
schemaFromExpressions (char *name, List *attributeNames, List *exprs, List *inputs)
{
    List *dataTypes = NIL;

    FOREACH(Node,n,exprs)
        dataTypes = appendToTailOfListInt(dataTypes, typeOf(n));

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
deleteAttrFromSchemaByName(QueryOperator *op, char *name)
{
    FOREACH(AttributeDef,a,op->schema->attrDefs)
    {
        if (streq(a->attrName,name))
        {
            op->schema->attrDefs = REMOVE_FROM_LIST_PTR(op->schema->attrDefs, a);
            break;
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
resetPosOfAttrRefBaseOnBelowLayerSchema(ProjectionOperator *op1, QueryOperator *op2)
{
    int cnt = 0;
    FOREACH(AttributeDef, a2, op2->schema->attrDefs)
    {
        FOREACH(AttributeReference, a1, op1->projExprs)
        {
            if(streq(a1->name, a2->attrName))
            {
                a1->attrPosition = cnt;
            }
        }
        cnt++;
    }
}

void
resetPosOfAttrRefBaseOnBelowLayerSchemaOfSelection(SelectionOperator *op1,QueryOperator *op2)
{
    Operator *o = (Operator *)(op1->cond);
    int cnt = 0;

    if(!streq(o->name,"AND"))
    {
        FOREACH(AttributeDef, a2, op2->schema->attrDefs)
        {
            FOREACH_LC(lc, (o->args))
            {
                if(isA(LC_P_VAL(lc), AttributeReference))
                {
                    AttributeReference *a1 = (AttributeReference *)LC_P_VAL(lc);
                    //DEBUG_LOG("Test Def: %s, Ref: %s",
                    //a2->attrName,a1->name);
                    if(streq(a1->name,a2->attrName))
		    {
                        a1->attrPosition = cnt;
                    }
                }
            }
            cnt++;
        }
    }
    else
    {
        Operator *o2;
        Operator *o1;
        FOREACH(AttributeDef, a2, op2->schema->attrDefs)
        {
            o2 = o;
            while(streq(o2->name,"AND"))
            {
                o1  = (Operator *)(getTailOfListP(o2->args));
                o2 = (Operator *)(getHeadOfListP(o2->args));

                FOREACH_LC(lc,(o1->args))
                {
                    if(isA(LC_P_VAL(lc), AttributeReference))
                    {
                        AttributeReference *a1 = (AttributeReference *)LC_P_VAL(lc);
                        //DEBUG_LOG("Test Def: %s, Ref: %s",
                        //a2->attrName,a1->name);
			if(streq(a1->name,a2->attrName))
			{
                            a1->attrPosition = cnt;
                        }
                    }
                }
            }

            //The last one operator which without AND
            FOREACH_LC(lc,(o2->args))
	    {
                if(isA(LC_P_VAL(lc), AttributeReference))
                {
                    AttributeReference *a1 = (AttributeReference *)LC_P_VAL(lc);
                    //DEBUG_LOG("Test Def: %s, Ref: %s",
                    //a2->attrName,a1->name);
		    if(streq(a1->name,a2->attrName))
                    {
                        a1->attrPosition = cnt;
                    }
                }
            }

            cnt++;
        }
    }
}


void
setMoveAroundListSetProperityForWholeTree(QueryOperator *root)
{
    if(root->inputs != NULL)
    {
        FOREACH(QueryOperator, op, root->inputs)
            setMoveAroundListSetProperityForWholeTree(op);
    }

    if(root != NULL)
    {
        if(isA(root, TableAccessOperator))
        {
            List *setList = NIL;
            FOREACH(AttributeDef,a, root->schema->attrDefs)
            {
                Set *s = MAKE_SET_PTR(a->attrName);
                setList = appendToTailOfList(setList, s);
            }

            setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_LIST_SET_SELECTION_MOVE_AROUND), (Node *)setList);
        }

        if(isA(root, SelectionOperator))
        {
            List *opList = NIL;

            Operator *c = (Operator *)(((SelectionOperator *)root)->cond);
            if(streq(c->name,"AND"))
            {
                opList = getSelectionCondOperatorList(opList, c);
            }
            else
                opList = appendToHeadOfList(opList, c);

	  QueryOperator *child = OP_LCHILD(root);
          Node *n1 = getProperty((QueryOperator *)child, (Node *) createConstString(PROP_STORE_LIST_SET_SELECTION_MOVE_AROUND));

          List *l1 = (List *)n1;
          List *l11 = NIL;
          List *setList = NIL;

          FOREACH(Set, s1, l1)
          {
              Set *s = unionSets(s1,s1);
              l11 = appendToTailOfList(l11, s);
          }

          setList = UnionEqualElemOfTwoSetList(opList, l11);
          setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_LIST_SET_SELECTION_MOVE_AROUND), (Node *)setList);
        }

        if(isA(root, JoinOperator))
        {
            QueryOperator *newRoot = root;
            QueryOperator *parent = getHeadOfListP(root->parents);
            List *opList = NIL;
            List *setList = NIL;
            List *helpList = NIL;

            Operator *c = (Operator *)(((SelectionOperator *)parent)->cond);

            if(streq(c->name,"AND"))
            {
                opList = getSelectionCondOperatorList(opList, c);
            }
            else
                opList = appendToHeadOfList(opList, c);

	    FOREACH(QueryOperator, op, root->inputs)
            {
                Node *n1 = getProperty(op, (Node *) createConstString(PROP_STORE_LIST_SET_SELECTION_MOVE_AROUND));
                List *l1 = (List *)n1;

                List *templ1 = NIL;
                FOREACH(Set, s1, l1)
                {
                    Set *s = unionSets(s1,s1);
                    templ1 = appendToTailOfList(templ1, s);
                }

                helpList = concatTwoLists(helpList,templ1);
            }

	    setList = UnionEqualElemOfTwoSetList(opList, helpList);
            setProperty((QueryOperator *)newRoot, (Node *) createConstString(PROP_STORE_LIST_SET_SELECTION_MOVE_AROUND), (Node *)setList);
        }
    }
}


List *
UnionEqualElemOfTwoSetList(List *listEqlOp, List *listSet)
{

    FOREACH_LC(lc, listEqlOp)
    {
        if(streq(((Operator *)LC_P_VAL(lc))->name,"="))
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
    //DEBUG_LOG("test n1: %s", nodeToString(n1));
    //DEBUG_LOG("test n2: %s", nodeToString(n2));

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
            /*if(hasSetElem(s1,(char *)tempn1))
              {
                  flag1 = TRUE;
                  tempSet1 = s1;
                  break;
              }*/

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
            /*if(hasSetElem(s2,tempn2))
              {
                  flag2 = TRUE;
                  tempSet2 = s2;
                  break;
              }*/

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
}


void
reSetMoveAroundListSetProperityForWholeTree(QueryOperator *root)
{
    if(isA(root, JoinOperator))
    {
        Node *n1 = getProperty(root, (Node *) createConstString(PROP_STORE_LIST_SET_SELECTION_MOVE_AROUND));
        List *l1 = (List *)n1;
        FOREACH(QueryOperator, op, root->inputs)
        {
            while(!(isA(op,SelectionOperator) || isA(op,JoinOperator) || isA(op,TableAccessOperator)))
            {
                op = OP_LCHILD(op);
            }

            List *attrName = getAttrNames((Schema *)(op->schema));

            List *newl1 = NIL;
            FOREACH(Set,s,l1)
            {
                Set *tempSet = unionSets(s,s);
                boolean flag;
                FOREACH_SET(Node,n,s)
                {
                    flag = FALSE;
                    if(!isA(n,Constant))
                    {
                        FOREACH(char,nme,attrName)
                        {
                            if(streq((char *)n,nme))
                            {
                                flag = TRUE;
                            }
                        }
                    }
                    else
                        flag = TRUE;

                    if(flag == FALSE)
                    {
                        removeSetElem(tempSet,n);
                    }
                }
                if(setSize(tempSet) != 0)
                {
                    newl1 = appendToTailOfList(newl1,tempSet);
                }
            }

            setProperty(op, (Node *) createConstString(PROP_STORE_LIST_SET_SELECTION_MOVE_AROUND),(Node *)newl1);
        }
    }

    FOREACH(QueryOperator, o, root->inputs)
        reSetMoveAroundListSetProperityForWholeTree(o);
}


void
introduceSelection(QueryOperator *root)
{
    if(root->inputs != NULL)
    {
        FOREACH(QueryOperator, op, root->inputs)
        {
            introduceSelection(op);
        }
    }

    if(isA(root,JoinOperator))
    {
        FOREACH(QueryOperator,qo,root->inputs)
        {
            Node *n1 = getProperty(qo, (Node *) createConstString(PROP_STORE_LIST_SET_SELECTION_MOVE_AROUND));
            List *l1 = (List *)n1;
            List *opList = NIL;

            FOREACH(Set, s1, l1)
            {
                if(setSize(s1) == 2)
                {
                    List *argList = NIL;
                    AttributeReference *a;
                    FOREACH_SET(Node,selem,s1)
                    {
                        if(!isA(selem,Constant))
                        {
                            FOREACH(AttributeDef,attrDef,qo->schema->attrDefs)
                            {
                                if(streq((char *)selem,attrDef->attrName))
                                {
                                    a = createFullAttrReference((char *)selem , 0, 0, 0, attrDef->dataType);
                                    break;
                                }
                            }
                            argList = appendToHeadOfList(argList,a);
                        }
                        else
                            argList = appendToTailOfList(argList,selem);
                    }

                    Operator *o = createOpExpr("=", argList);
                    opList = appendToTailOfList(opList, o);
                }

                if(setSize(s1) == 3)
                {
                    List *argList = NIL;
                    AttributeReference *a;
                    AttributeReference *b;
                    int flagFst = FALSE;

                    FOREACH_SET(Node,selem,s1)
                    {
                        if(flagFst == FALSE)
                        {
                            flagFst = TRUE;
                            if(!isA(selem,Constant))
                            {
                                FOREACH(AttributeDef,attrDef,qo->schema->attrDefs)
                                {
                                    if(streq((char *)selem,attrDef->attrName))
                                    {
                                        a = createFullAttrReference((char *)selem , 0, 0, 0, attrDef->dataType);
                                        break;
                                    }
                                }
                                argList = appendToHeadOfList(argList,a);
                            }
                            else
                            {
                                argList = appendToTailOfList(argList,selem);
                            }
                        }
                        else
                        {
                            if(!isA(selem,Constant))
                            {
                                FOREACH(AttributeDef,attrDef,qo->schema->attrDefs)
                                {
                                    if(streq((char *)selem,attrDef->attrName))
                                    {
                                        b = createFullAttrReference((char *)selem , 0, 0, 0, attrDef->dataType);
                                        break;
                                    }
                                }
                                argList = appendToHeadOfList(argList,b);

				Operator *o = createOpExpr("=", argList);
                                opList = appendToTailOfList(opList, o);

				argList = REMOVE_FROM_LIST_PTR(argList,b);
                            }
                            else
                            {
                                argList = appendToTailOfList(argList,selem);
				Operator *o = createOpExpr("=", argList);
                                opList = appendToTailOfList(opList, o);

                                argList = REMOVE_FROM_LIST_PTR(argList,selem);
                            }
                        }
                    }
                }
            }

            if(opList != NIL)
            {
                if(isA(qo, SelectionOperator))
                {
                    List *originalOpList = NIL;
                    Operator *originalCondOp = (Operator *)(((SelectionOperator *)qo)->cond);

                    if(streq(originalCondOp->name,"AND"))
                    {
                        originalOpList = getSelectionCondOperatorList(originalOpList, originalCondOp);
                    }
                    else
                        originalOpList = appendToHeadOfList(originalOpList, originalCondOp);

                    FOREACH(Operator,condOp,originalOpList)
                    {
                        if(!streq(condOp->name,"="))
                        {
                            opList = appendToHeadOfList(opList,condOp);
                        }
                    }

                    Node *opCond = changeListOpToAnOpNode(opList);
                    ((SelectionOperator *)qo)->cond = opCond;
                    QueryOperator *child = getHeadOfListP(qo->inputs);
                    resetPosOfAttrRefBaseOnBelowLayerSchemaOfSelection((SelectionOperator *)qo,(QueryOperator *)child);
                }
                else
                {
                    Node *opCond = changeListOpToAnOpNode(opList);
                    SelectionOperator *newSo1 = createSelectionOp(opCond, NULL, NIL,getAttrNames(qo->schema));

                    // Switch the subtree with this newly created projection
                    // operator.
		    switchSubtrees((QueryOperator *) qo, (QueryOperator *) newSo1);

                    // Add child to the newly created projections operator,
                    addChildOperator((QueryOperator *) newSo1, (QueryOperator *) qo);

                    //reset the attr_ref position
                    resetPosOfAttrRefBaseOnBelowLayerSchemaOfSelection((SelectionOperator *)newSo1,(QueryOperator *)qo);
                }
            }
        }
    }
}


Node *
changeListOpToAnOpNode(List *l1)
{
    List *helpList;
    Node *opNode1;

    if (LIST_LENGTH(l1) == 2)
        opNode1 = (Node *) createOpExpr("AND", (List *) l1);
    else if(LIST_LENGTH(l1) > 2)
    {
        int i;
        helpList = NIL;
        Operator *helpO1 = getHeadOfListP(l1);
        l1 = REMOVE_FROM_LIST_PTR(l1, helpO1);
        Operator *helpO2 = getHeadOfListP(l1);
        l1 = REMOVE_FROM_LIST_PTR(l1, helpO2);
        helpList = appendToTailOfList(helpList, helpO1);
        helpList = appendToTailOfList(helpList, helpO2);

	Operator *helpO = createOpExpr("AND", (List *) helpList);
        int length_l1 = LIST_LENGTH(l1);

        for(i=0; i<length_l1; i++)
        {
            helpList = NIL;
            helpList = appendToTailOfList(helpList, helpO);
            helpO = getHeadOfListP(l1);
            l1 = REMOVE_FROM_LIST_PTR(l1, helpO);
            helpList = appendToTailOfList(helpList, helpO);
            helpO =  createOpExpr("AND", (List *) helpList);
        }
        opNode1 = (Node *)helpO;
    }
    else
        opNode1 = (Node *) getHeadOfListP(l1);

    return opNode1;
}

List *
getSelectionCondOperatorList(List *opList, Operator *op)
{
    Operator *o2 = (Operator *)(getTailOfListP(op->args));
    if(!streq(o2->name, "AND"))
    {
        opList = appendToHeadOfList(opList, o2);
    }

    Operator *o1 = (Operator *)(getHeadOfListP(op->args));
    if(streq(o1->name,"AND"))
    {
        getSelectionCondOperatorList(opList, o1);
    }
    else
    {
        opList = appendToHeadOfList(opList, o1);
    }

    return opList;
}


List*
getCondOpList(List *l1, List *l2)
{
    boolean flag1;
    List *newOpList = NIL;

    FOREACH(Operator, o, l2)
    {
        DEBUG_LOG("test list: %s", nodeToString(o));
        flag1 = FALSE;

	FOREACH(AttributeDef, a, l1)
        {
            if(streq(((AttributeReference *)getHeadOfListP(o->args))->name, a->attrName))
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
                DEBUG_LOG("test compare AttributeReference");
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
getDataTypes (Schema *schema)
{
    List *result = NIL;

    FOREACH(AttributeDef,a,schema->attrDefs)
    result = appendToTailOfListInt(result, a->dataType);

    return result;
}

List *
getAttrNames(Schema *schema)
{
    List *result = NIL;

    FOREACH(AttributeDef,a,schema->attrDefs)
    result = appendToTailOfList(result, a->attrName);

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

    return ta;
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

    FOREACH(Node, expr, projExprs)
    prj->projExprs = appendToTailOfList(prj->projExprs, (Node *) copyObject(expr));

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
    QueryOperator *lChild = OP_LCHILD(set);

    set->setOpType = setOpType;
    set->op.inputs = inputs;
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

    dr->attrs = attrs;
    dr->op.inputs = singleton(input);
    dr->op.schema = createSchemaFromLists("DUPREM", attrNames, getDataTypes(input->schema));
    dr->op.parents = parents;
    dr->op.provAttrs = NULL;

    return dr;
}

ProvenanceComputation *
createProvenanceComputOp(ProvenanceType provType, List *inputs, List *parents, List *attrNames, Node *asOf)
{
    ProvenanceComputation *p = makeNode(ProvenanceComputation);

    p->op.parents = parents;
    p->op.inputs = inputs;
    p->op.schema = createSchemaFromLists("PROVENANCE", attrNames, NULL);
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
    co->op.schema= createSchemaFromLists("ConstRel", attrNames, dataTypes);
    co->op.parents=parents;
    co->op.provAttrs=NIL;

    return co;
}

NestingOperator *
createNestingOp(NestingExprType nestingType, Node *cond, List *inputs, List *parents, List *attrNames)
{
    NestingOperator *no = makeNode(NestingOperator);
    no->nestingType = nestingType;
    no->cond = copyObject(cond);
    no->op.type = T_NestingOperator;
    no->op.inputs = inputs;
    no->op.schema = createSchemaFromLists("NESTING", attrNames, NIL);
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

    wo->partitionBy = partitionBy;
    wo->orderBy = orderBy;
    wo->frameDef = frameDef;
    wo->attrName = attrName;
    wo->f = (Node *) fCall;
    wo->op.type = T_WindowOperator;
    wo->op.inputs = singleton(input);
    wo->op.schema = createSchemaFromLists("WINDOW", CONCAT_LISTS(inputAttrs, singleton(attrName)), NIL);
    wo->op.parents = parents;
    wo->op.provAttrs = NIL;

    return wo;
}

OrderOperator *
createOrderOp(List *orderExprs, QueryOperator *input, List *parents)
{
    OrderOperator *o = makeNode(OrderOperator);
    List *inputAttrs = getQueryOperatorAttrNames(input);

    o->orderExprs = orderExprs;
    o->op.type = T_OrderOperator;
    o->op.inputs = singleton(input);
    o->op.schema =  createSchemaFromLists("ORDER", inputAttrs, NIL);
    o->op.parents = parents;
    o->op.provAttrs = NIL;

    return o;
}

void
setProperty (QueryOperator *op, Node *key, Node *value)
{
    KeyValue *val = getProp(op, key);

    if (val)
    {
        val->value = value;
        return;
    }

    val = createNodeKeyValue(key, value);
    op->properties =  (Node *) appendToTailOfList((List *) op->properties, val);
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
    KeyValue *kv = getProp(op, (Node *) createConstString(key));

    return kv ? kv->value : NULL;
}

static KeyValue *
getProp (QueryOperator *op, Node *key)
{
    FOREACH(KeyValue,p,(List *) op->properties)
    {
        if (equal(p->key,key))
            return p;
    }

    return NULL;
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
        DEBUG_LOG("prov attr at <%u> is <%s>", i, nodeToString(getNthOfListP(op->schema->attrDefs, i)));
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
        DEBUG_LOG("prov attr at <%u> is <%s>", i, nodeToString(getNthOfListP(op->projExprs, i)));
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

List *
getAttrRefsInOperator (QueryOperator *op)
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
            //TODO do not traverse into query operator
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

List *
aggOpGetGroupByAttrNames(AggregationOperator *op)
{
    List *result = getQueryOperatorAttrNames((QueryOperator *) op);

    return sublist(result, LIST_LENGTH(op->aggrs), LIST_LENGTH(op->aggrs) + LIST_LENGTH(op->groupBy));
}

List *
aggOpGetAggAttrNames(AggregationOperator *op)
{
    List *result = getQueryOperatorAttrNames((QueryOperator *) op);

    return sublist(result, 0, LIST_LENGTH(op->aggrs));
}

WindowFunction *
winOpGetFunc (WindowOperator *op)
{
    return createWindowFunction(copyObject(op->f),
            (WindowDef *) copyObject(createWindowDef(
                    op->partitionBy, op->orderBy, op->frameDef)));
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

        FOREACH(QueryOperator,parent,op->parents)
        {
            QueryOperator *copy = copyUnrootedSubtree(op);
            replaceNode(parent->inputs, op, copy);
            copy->parents = singleton(parent);
        }
        op->parents = NIL;
    }
}

//static Schema *
//mergeSchemas (List *inputs)
//{
//    Schema *result = NULL;
//
//    FOREACH(QueryOperator,O,inputs)
//    {
//        if (result == NULL)
//            result = (Schema *) copyObject(O->schema);
//        else
//            result->attrDefs = concatTwoLists(result->attrDefs, copyObject(O->schema->attrDefs));
//    }
//
//    return result;
//}

/*-----------------------------------------------------------------------------
 *
 * optimizer_prop_inference.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "model/node/nodetype.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "operator_optimizer/optimizer_prop_inference.h"
#include "metadata_lookup/metadata_lookup.h"
#include "include/log/logger.h"
#include "include/model/query_operator/operator_property.h"


static List *attrRefListToStringList (List *input);

//TODO using a static variable is a problem here, because you just have the last assigned key for a base rleation available which is not sufficient.
// you need the keys of an operators inputs to determine the operator keys. Since you store them as a property, why not get them from there?
//static List *keyList = NIL;

//TODO keylist should be a list/set of string sets, because an operator may have more than one candidate key each of which may consist of multiple attributes
void
computeKeyProp (QueryOperator *root)
{
    List *keyList = NIL;
    List *rKeyList = NIL;

    if (root == NULL)
        return;

    // compute key properties of children first
    if(root->inputs != NULL)
        FOREACH(QueryOperator, op, root->inputs)
            computeKeyProp(op);

    // table acces operator or constant relation operators have predetermined keys
    if(isA(root, TableAccessOperator))
    {
        TableAccessOperator *rel = (TableAccessOperator *) root;
        keyList = getKeyInformation(rel->tableName);
        setStringProperty((QueryOperator *)root, PROP_STORE_LIST_KEY, (Node *)keyList);
        DEBUG_LOG("operator %s keys are {%s}", root->schema->name, stringListToString(keyList));
        return;
    }
    else if (isA(root, ConstRelOperator))
    {
        FOREACH(AttributeDef, a, root->schema->attrDefs)
            keyList = appendToTailOfList(keyList, strdup(a->attrName));
        setStringProperty((QueryOperator *)root, PROP_STORE_LIST_KEY, (Node *)keyList);
        DEBUG_LOG("operator %s keys are {%s}", root->schema->name, stringListToString(keyList));
        return;
    }

    // get keys of children
    keyList = (List *) getStringProperty(OP_LCHILD(root), PROP_STORE_LIST_KEY);

    if (IS_BINARY_OP(root))
    {
        List *newKeyList = NIL;
        rKeyList = (List *) getStringProperty(OP_RCHILD(root), PROP_STORE_LIST_KEY);
        newKeyList = concatTwoLists(keyList, rKeyList);
        setStringProperty((QueryOperator *)root, PROP_STORE_LIST_KEY, (Node *)newKeyList);
    }
    // deal with different operator types

    // here we could use the ECs to determine new keys, e.g., if input has keys {{A}, {C}} and we have selection condition B = C, then we have a new key {{A}, {B}, {C}}
    if (isA(root, SelectionOperator))
        setStringProperty((QueryOperator *)root, PROP_STORE_LIST_KEY, (Node *)keyList);

    if (isA(root, ProjectionOperator))
    {
        List *l1 = ((ProjectionOperator *)root)->projExprs;
        List *l2 = NIL;

        FOREACH(AttributeReference, op1, l1)
            l2 = appendToTailOfList(l2, op1->name);

        FOREACH(char, op, keyList)
        {
            if(!searchListString(l2, op))
            {
                setStringProperty((QueryOperator *)root, PROP_STORE_LIST_KEY, NULL);
                break;
            }

            setStringProperty((QueryOperator *)root, PROP_STORE_LIST_KEY, (Node *)keyList);
        }
    }

    // dup removal operator has a key {all attributes} if the input does not have a key
    if (isA(root, DuplicateRemoval))
    {
    	//List *l1 = getQueryOperatorAttrNames(OP_LCHILD(root));
    	//TODO Get the child's key property and Append it to above list and set it as property of duplicate operator

        setStringProperty((QueryOperator *)root, PROP_STORE_LIST_KEY, (Node *)keyList);
    }

    DEBUG_LOG("operator %s keys are {%s}", root->schema->name, stringListToString(keyList));
}

void
computeECProp (QueryOperator *root)
{
    INFO_LOG("\n************************************************************\n"
            "    PORPERTY INFERENCE STEP: ECs\n"
            "************************************************************\n"
            "%s\n"
            "************************************************************",
            operatorToOverviewString((Node *) root));
    DEBUG_LOG("*********EC**********\n\tStart bottom-up traversal");
	computeECPropBottomUp(root);
	printECPro(root);

	DEBUG_LOG("*********EC**********\n\tStart top-down traversal");
	computeECPropTopDown(root);
	printECPro(root);
}

void
printSingleECList(List *l)
{
	DEBUG_LOG("SET LIST: %s, SIZE LIST %d", nodeToString(l), LIST_LENGTH(l));
	FOREACH(KeyValue, kv, l)
	{
	    Set *s = (Set *) kv->key;
	    Constant *c = (Constant *) kv->value;
		DEBUG_LOG("Set: ");
		FOREACH_SET(char, n, s)
        {
		    DEBUG_LOG("%s", (char *)n);
        }
		if (c != NULL)
		    DEBUG_LOG("%s", exprToSQL((Node *) c));
		DEBUG_LOG("\n");
	}
}

void
printECPro(QueryOperator *root)
{
    StringInfo str = makeStringInfo ();

	appendStringInfoString(str, NodeTagToString(root->type));
	appendStringInfo(str, " (%p)", root);

	Node *nRoot = getProperty(root, (Node *) createConstString(PROP_STORE_SET_EC));
	List *list = (List *)nRoot;
	appendStringInfo(str, "\nList size %d\n", LIST_LENGTH(list));

	FOREACH(KeyValue, kv, list)
	{
	    Set *s = (Set *) kv->key;
	    Constant *c = (Constant *) kv->value;

		appendStringInfoString(str,"{");
        FOREACH_SET(char, n, s)
        {
            appendStringInfo(str,"%s%s", (char *)n, FOREACH_SET_HAS_NEXT(n) ? " " : "");
        }
        if (c != NULL)
            appendStringInfo(str," %s", exprToSQL((Node *) c));
		appendStringInfoString(str, "} ");
	}
	appendStringInfoString(str, "\n");
	DEBUG_LOG("EC %s", str->data);

	if(root->inputs != NULL)
	{
		FOREACH(QueryOperator, op, root->inputs)
		         printECPro(op);
	}
 }

//generate a List of Sets by bottom up(here uses ptr set)
//for each set, (e.g. {{a,d,5},{c}})a is the pointer point to char a,b, 5 is the pointer point to a constant structure
void
computeECPropBottomUp (QueryOperator *root)
{
	if(root->inputs != NULL)
	{
		FOREACH(QueryOperator, op, root->inputs)
        				computeECPropBottomUp(op);
	}

	if(root != NULL)
	{
		if(isA(root, TableAccessOperator))
		{
			List *EC = NIL;
			FOREACH(AttributeDef,a, root->schema->attrDefs)
			{
			    KeyValue *kv;
				Set *s = MAKE_STR_SET(a->attrName);
				kv = createNodeKeyValue((Node *) s, NULL);
				EC = appendToTailOfList(EC, kv);
			}

			setStringProperty((QueryOperator *)root, PROP_STORE_SET_EC, (Node *)EC);
		}

		else if(isA(root, SelectionOperator))
		{
			QueryOperator *childOp = OP_LCHILD(root);
			Node *nChild = getStringProperty(childOp, PROP_STORE_SET_EC);
			List *childEC = (List *) copyObject(nChild); // use same pointers as in child which is unsafe if you

			List *CondEC = NIL;
			Node *op = ((SelectionOperator *)root)->cond;
			CondEC = GenerateCondECSetListUsedInBottomUp(op);

			//Union the child's EC list with the Cond EC list
			List *EC = concatTwoLists(childEC, CondEC);

			//remove the Duplicate set in the list (which has the same element)
			EC = CombineDuplicateElemSetInECList(EC);
			setStringProperty((QueryOperator *)root, PROP_STORE_SET_EC, (Node *)EC);
		}

		else if(isA(root, ProjectionOperator))
		{
			ProjectionOperator *pj = (ProjectionOperator *)root;
			//TODO this is like a copyList, e.g., a shallow copy, if this is waht you wanted then replace with copyList otherwise use copyObject
			//get list (contains attrRef or Op) from project op projExprs
			List *attrA = NIL;
			List *attrB = NIL;
			attrA = pj->projExprs;
			attrB = pj->op.schema->attrDefs;

			//get child EC property
			Node *nChildECSetList = getStringProperty(OP_LCHILD(root), PROP_STORE_SET_EC);
			List *childECSetList = (List *)copyObject(nChildECSetList);

			List *setList = NIL;
			setList = SCHAtoBUsedInBomUp(setList, childECSetList, attrA, attrB);
			setList = CombineDuplicateElemSetInECList(setList);
			setStringProperty((QueryOperator *)root, PROP_STORE_SET_EC, (Node *)setList);
		}

		else if(isA(root, JoinOperator))
		{
			List *EC = NIL;
			List *lChildEC = (List *) getStringProperty(OP_LCHILD(root), PROP_STORE_SET_EC);
			List *rChildEC = (List *) getStringProperty(OP_RCHILD(root), PROP_STORE_SET_EC);

			if (((JoinOperator*)root)->joinType == JOIN_INNER)
			{
				//1, Get cond set
				List *condEC = NIL;
                Node *op = ((JoinOperator*)root)->cond; //TODO not safe may be,e.g., function
                condEC = GenerateCondECSetListUsedInBottomUp(op);

                //2, union it with EC(Rchild) and EC(Lchild)
    			EC = concatTwoLists(copyObject(lChildEC), copyObject(rChildEC));
    			EC = concatTwoLists(EC, copyObject(condEC));

    			//3, Duplicate remove
    			EC = CombineDuplicateElemSetInECList(EC);
    			setStringProperty((QueryOperator *)root, PROP_STORE_SET_EC, (Node *)EC);
			}

			if (((JoinOperator*)root)->joinType == JOIN_CROSS)
			{
				EC = concatTwoLists(copyObject(lChildEC), copyObject(rChildEC));
				setStringProperty((QueryOperator *)root, PROP_STORE_SET_EC, (Node *)EC);
			}

			if (((JoinOperator*)root)->joinType == JOIN_LEFT_OUTER)
			{
				List *condList = NIL;
				List *newCondList = NIL;
				getSelectionCondOperatorList(((JoinOperator *)root)->cond, &condList);
				boolean flag = FALSE;

				FOREACH(Operator, o, condList)
				{
					flag = FALSE;
					if(streq(o->name,"="))
					{
						List *rattrNames = getQueryOperatorAttrNames(OP_RCHILD(root));
						Node *n1 = getHeadOfListP(o->args);
						Node *n2 = getTailOfListP(o->args);

						if(isA(n1, AttributeReference))
						{
							char *name1 = ((AttributeReference *)n1)->name;
							if(searchListString(rattrNames, name1))
								flag = TRUE;
						}

						if(isA(n2, AttributeReference))
						{
							char *name2 = ((AttributeReference *)n2)->name;
							if(searchListString(rattrNames, name2))
								flag = TRUE;
						}

					}

					if(flag == FALSE)
						newCondList = appendToTailOfList(newCondList, o);
				}
				List *newEC = NIL;
				newEC = GenerateCondECBasedOnCondOp(newCondList);

				List *rEC = NIL;
				FOREACH(AttributeDef,a, ((QueryOperator *)OP_RCHILD(root))->schema->attrDefs)
				{
				    KeyValue *kv;
					Set *s = MAKE_STR_SET(a->attrName);
					kv = createNodeKeyValue((Node *) s, NULL);
					rEC = appendToTailOfList(rEC, kv);
				}
				EC = concatTwoLists(copyObject(lChildEC), rEC);
				EC = concatTwoLists(EC, newEC);
    			EC = CombineDuplicateElemSetInECList(EC);
    			setStringProperty((QueryOperator *)root, PROP_STORE_SET_EC, (Node *)EC);
			}

			if (((JoinOperator*)root)->joinType == JOIN_RIGHT_OUTER)
			{
				List *condList = NIL;
				List *newCondList = NIL;
				getSelectionCondOperatorList(((JoinOperator *)root)->cond, &condList);
				boolean flag = FALSE;

				FOREACH(Operator, o, condList)
				{
					flag = FALSE;
					if(streq(o->name,"="))
					{
						List *lattrNames = getQueryOperatorAttrNames(OP_LCHILD(root));
						Node *n1 = getHeadOfListP(o->args);
						Node *n2 = getTailOfListP(o->args);

						if(isA(n1, AttributeReference))
						{
							char *name1 = ((AttributeReference *)n1)->name;
							if(searchListString(lattrNames, name1))
								flag = TRUE;
						}

						if(isA(n2, AttributeReference))
						{
							char *name2 = ((AttributeReference *)n2)->name;
							if(searchListString(lattrNames, name2))
								flag = TRUE;
						}
					}

					if(flag == FALSE)
						newCondList = appendToTailOfList(newCondList, o);
				}
				List *newEC = NIL;
				newEC = GenerateCondECBasedOnCondOp(newCondList);

				List *lEC = NIL;
				FOREACH(AttributeDef,a, ((QueryOperator *)OP_LCHILD(root))->schema->attrDefs)
				{
				    KeyValue *kv;
					Set *s = MAKE_STR_SET(a->attrName);
					kv = createNodeKeyValue((Node *) s, NULL);
					lEC = appendToTailOfList(lEC, kv);
				}
				EC = concatTwoLists(copyObject(rChildEC), lEC);
				EC = concatTwoLists(EC, newEC);
    			EC = CombineDuplicateElemSetInECList(EC);
    			setStringProperty((QueryOperator *)root, PROP_STORE_SET_EC, (Node *)EC);
			}

			if (((JoinOperator*)root)->joinType == JOIN_FULL_OUTER)
			{
				List *rEC = NIL;
				FOREACH(AttributeDef,a, ((QueryOperator *)OP_RCHILD(root))->schema->attrDefs)
				{
				    KeyValue *kv;
					Set *s = MAKE_STR_SET(a->attrName);
					kv = createNodeKeyValue((Node *) s, NULL);
					rEC = appendToTailOfList(rEC, kv);
				}

				List *lEC = NIL;
				FOREACH(AttributeDef,a, ((QueryOperator *)OP_LCHILD(root))->schema->attrDefs)
				{
				    KeyValue *kv;
					Set *s = MAKE_STR_SET(a->attrName);
					kv = createNodeKeyValue((Node *) s, NULL);
					lEC = appendToTailOfList(lEC, kv);
				}

				List *EC = NIL;
				EC = concatTwoLists(rEC, lEC);
				setStringProperty((QueryOperator *)root, PROP_STORE_SET_EC, (Node *)EC);
			}
		}

		else if(isA(root, AggregationOperator))
		{
            AggregationOperator *agg = (AggregationOperator *)root;

			List *childECSetList = (List *) getStringProperty(OP_LCHILD(root), PROP_STORE_SET_EC);
			List *setList = NIL;

			List *aggAndGB = concatTwoLists(copyList(agg->aggrs), copyList(agg->groupBy));
			List *cmpGrByADef = copyList(agg->op.schema->attrDefs);
//			popHeadOfListP(cmpGrByADef); //TODO works only if there is only one aggregation function!
			//change attrRef name in Group By to attrDef in Schema
			setList = SCHAtoBUsedInBomUp(setList, childECSetList, aggAndGB, cmpGrByADef);

			setStringProperty((QueryOperator *)root, PROP_STORE_SET_EC, (Node *)setList);
		}

		else if(isA(root, DuplicateRemoval))
		{
			Node *childECP = getProperty(OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_EC));
			List *setList = (List *)childECP;
			setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_SET_EC), (Node *)setList);
		}

		else if(isA(root,SetOperator))
		{
			//get EC of left child and right child
			Node *lChildECN = getProperty(OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_EC));
			List *lECSetList = (List *)copyObject(lChildECN);
			Node *rChildECP = getProperty(OP_RCHILD(root), (Node *) createConstString(PROP_STORE_SET_EC));
			List *rECSetList = (List *)copyObject(rChildECP);

			//get schema list of left child and right child
			List *lattrDefs = getQueryOperatorAttrNames(OP_LCHILD(root));
			List *rattrDefs = getQueryOperatorAttrNames(OP_RCHILD(root));

			if(((SetOperator *)root)->setOpType == SETOP_UNION)
			{
			    //step 1, SCH(S)/SCH(R)
				List *rSetList = NIL;
				rSetList = LSCHtoRSCH(rSetList,rECSetList,lattrDefs,rattrDefs);

				//step 2, intersect each set
				Set *tempSet;
				KeyValue *tempKv;
				List *setList = NIL;

				FOREACH(KeyValue, kv1, lECSetList)
				{
				    Set *s1 = (Set *) kv1->key;
				    Constant *c1 = (Constant *) kv1->value;

					FOREACH(KeyValue, kv2, rSetList)
		            {
					    Set *s2 = (Set *) kv2->key;
					    Constant *c2 = (Constant *) kv2->value;

						tempSet = intersectSets(copyObject(s1),copyObject(s2));
						if(setSize(tempSet) != 0) {
                        //TODO deal with conflicting constants
						    // the output is only constant if both inputs are equal to the same constant
//						    tempKv = createNodeKeyValue((Node *) tempSet,
//						            (Node *) ((c1 != c2) ? copyObject(c1) : NULL));
							//Should be if c1 = c2, output constant = c1; else, output constant = NULL;
							tempKv = createNodeKeyValue((Node *) tempSet,
							        (Node *) ((equal(c1,c2)) ? copyObject(c1) : NULL));
							setList = appendToTailOfList(setList, tempKv);
						}
		            }
				}

				setStringProperty((QueryOperator *)root, PROP_STORE_SET_EC, (Node *)setList);
			}

			if(((SetOperator *)root)->setOpType == SETOP_INTERSECTION)
			{
			    //SCH(S)/SCH(R)
				List *setList = NIL;
				setList = LSCHtoRSCH(setList,rECSetList,lattrDefs,rattrDefs);

                //setList = concatTwoLists(setList,copyObject(lECSetList));
                setList = concatTwoLists(setList,copyObject(lECSetList));
                setList = CombineDuplicateElemSetInECList(setList);
				setStringProperty((QueryOperator *)root, PROP_STORE_SET_EC, (Node *)setList);
			}

			if(((SetOperator *)root)->setOpType == SETOP_DIFFERENCE)
			{
				Node *childECN = getStringProperty(OP_LCHILD(root), PROP_STORE_SET_EC);
				List *EC = (List *) copyObject(childECN);
				setStringProperty((QueryOperator *)root, PROP_STORE_SET_EC, (Node *)EC);
				setStringProperty((QueryOperator *)root, PROP_STORE_SET_EC, (Node *)EC);
			}

		}
		else if(isA(root,WindowOperator))
		{
			List *childEC = (List *)getStringProperty(OP_LCHILD(root), PROP_STORE_SET_EC);
			List *EC = copyObject(childEC);

			WindowOperator *wOp = (WindowOperator *)root;
			char *f = wOp->attrName;

			//deal with the case: if window op like: sum(a)... AS X, sum(a)... AS Y
			//test case: "SELECT X,C FROM (SELECT SUM(A) OVER(PARTITION BY B ORDER BY B desc) AS X, SUM(A) OVER(PARTITION BY B ORDER BY B desc) AS Y,C FROM R JOIN S ON A=C) sbu WHERE X=3 AND C=4;
			if(isA(OP_LCHILD(root), WindowOperator))
			{
				WindowOperator *wChild = (WindowOperator *)OP_LCHILD(root);
				if(equal(wChild->f, wOp->f))
				{
                    FOREACH(KeyValue, kv, EC)
		            {
                    	Set *s = (Set *)kv->key;
                    	if(hasSetElem(s, wChild->attrName))
                    	{
                    		addToSet(s, strdup(wOp->attrName));
                    	}
		            }
				}
				else
				{
					Set *s = MAKE_STR_SET(strdup(f));
					KeyValue *kv;
					kv = createNodeKeyValue((Node *) s, NULL);
					EC = appendToTailOfList(EC, kv);
				}
			}
			else
			{
				Set *s = MAKE_STR_SET(strdup(f));
				KeyValue *kv;
				kv = createNodeKeyValue((Node *) s, NULL);
				EC = appendToTailOfList(EC, kv);
			}
			setStringProperty((QueryOperator *)root, PROP_STORE_SET_EC, (Node *)EC);
		}
		else
		{
			List *childEC = (List *)getStringProperty(OP_LCHILD(root), PROP_STORE_SET_EC);
			List *EC = copyObject(childEC);
			setStringProperty((QueryOperator *)root, PROP_STORE_SET_EC, (Node *)EC);
		}
	}
}

void
computeECPropTopDown (QueryOperator *root)
{

	if(isA(root, SelectionOperator))
	{
		Node *nRoot = getStringProperty(root, PROP_STORE_SET_EC);
		QueryOperator *childOp = OP_LCHILD(root);

		setStringProperty((QueryOperator *)childOp, PROP_STORE_SET_EC, nRoot);
	}

	else if(isA(root, ProjectionOperator))
	{
		List *rList = (List *) getStringProperty(root, PROP_STORE_SET_EC);
		List *cList = (List *) getStringProperty((QueryOperator *)(OP_LCHILD(root)), PROP_STORE_SET_EC);
		// this is just a deep copy
		List *setList = copyObject(rList);

		ProjectionOperator *pj = (ProjectionOperator *)root;
		List *attrDefs = copyObject(pj->op.schema->attrDefs);
		List *attrRefs = copyList(pj->projExprs); //TODO this is not safe because projection may have a + b, you need to check, see below I fixed that

		SCHBtoAUsedInTopBom(&setList, attrRefs, attrDefs);

		cList = concatTwoLists(cList, setList);
		cList = CombineDuplicateElemSetInECList(cList);
		setStringProperty((QueryOperator *)(OP_LCHILD(root)), PROP_STORE_SET_EC, (Node *)cList);
	}

	//contains join inner and join cross
	else if(isA(root, JoinOperator))
	{
		//Join operator EC
		List *rootECSetList = (List *) getStringProperty(root, PROP_STORE_SET_EC);

		//SCH(Left Child)
		Set *lSchemaSet = STRSET();
		FOREACH(AttributeDef,a, ((QueryOperator *)(OP_LCHILD(root)))->schema->attrDefs)
		        addToSet(lSchemaSet,a->attrName);

		//SCH(Right Child)
		Set *rSchemaSet = STRSET();
		FOREACH(AttributeDef,a, ((QueryOperator *)(OP_RCHILD(root)))->schema->attrDefs)
		        addToSet(rSchemaSet,a->attrName);

		//get EC(left)
		QueryOperator *lChildOp = OP_LCHILD(root);
		Set *tempSet;
		List *lSetList = (List *) getStringProperty(lChildOp, PROP_STORE_SET_EC);
        FOREACH(KeyValue, kv, rootECSetList)
		{
            Set *s = (Set *) kv->key;
            Constant *c = (Constant *) kv->value;
        	tempSet =   setDifference(copyObject(s), rSchemaSet);

        	if(setSize(tempSet) > 1 || c != NULL) {
        	    KeyValue *newKV = createNodeKeyValue((Node*) tempSet, copyObject(c));
        		lSetList = appendToTailOfList(lSetList, newKV);
        	}
		}
        lSetList = CombineDuplicateElemSetInECList(lSetList);
		setStringProperty(lChildOp, PROP_STORE_SET_EC, (Node *)lSetList);

        //get EC(right)
        QueryOperator *rChildOp = OP_RCHILD(root);
        List *rSetList = (List *) getStringProperty(rChildOp, PROP_STORE_SET_EC);
        FOREACH(KeyValue, kv, rootECSetList)
        {
            Set *s = (Set *) kv->key;
            Constant *c = (Constant *) kv->value;
            tempSet = setDifference(copyObject(s), lSchemaSet);

            if(setSize(tempSet) > 1 || c != NULL) {
                KeyValue *newKV = createNodeKeyValue((Node*) tempSet, copyObject(c));
                rSetList = appendToTailOfList(rSetList, newKV);
            }
        }
        rSetList = CombineDuplicateElemSetInECList(rSetList);
        setStringProperty(rChildOp, PROP_STORE_SET_EC, (Node *)rSetList);
	}

	else if(isA(root, AggregationOperator))
	{
		List *rList = (List *) getStringProperty(root, PROP_STORE_SET_EC);
		List *cList = (List *) getStringProperty((QueryOperator *)(OP_LCHILD(root)), PROP_STORE_SET_EC);
		List *setList = copyObject(rList);

		AggregationOperator *agg = (AggregationOperator *)root;
		List *attrDefs = copyObject(agg->op.schema->attrDefs);
		List *aggAndGB = concatTwoLists(copyList(agg->aggrs), copyList(agg->groupBy));
		SCHBtoAUsedInTopBom(&setList, aggAndGB, attrDefs);

		cList = concatTwoLists(cList, setList);
		cList = CombineDuplicateElemSetInECList(cList);

		setStringProperty((QueryOperator *)(OP_LCHILD(root)), PROP_STORE_SET_EC, (Node *)cList);
	}

	else if(isA(root, DuplicateRemoval))
	{
	    //TODO this is not correct
		List *rootEC = (List *) getStringProperty(root, PROP_STORE_SET_EC);
		List *EC = copyObject(rootEC);
		setStringProperty((QueryOperator *)OP_LCHILD(root), PROP_STORE_SET_EC, (Node *)EC);
	}

	else if(isA(root,SetOperator))
	{
		//get schema list of left child and right child
		List *lattrDefs = getQueryOperatorAttrNames(OP_LCHILD(root));
		List *rattrDefs = getQueryOperatorAttrNames(OP_RCHILD(root));

		List *rootEC = (List *) getStringProperty(root, PROP_STORE_SET_EC);
		List *lEC = (List *) getStringProperty(OP_LCHILD(root), PROP_STORE_SET_EC);
		List *rEC = (List *) getStringProperty(OP_RCHILD(root), PROP_STORE_SET_EC);

		if(((SetOperator *)root)->setOpType == SETOP_UNION)
		{
            //set left child's EC
			List *lSetList = concatTwoLists(copyObject(rootEC), copyObject(lEC));
			lSetList = CombineDuplicateElemSetInECList(lSetList);
			setStringProperty((QueryOperator *)OP_LCHILD(root), PROP_STORE_SET_EC, (Node *)lSetList);

			//SCH(R)/SCH(S)
			List *newRootEC = NIL;
			newRootEC = LSCHtoRSCH(newRootEC,rootEC,rattrDefs,lattrDefs);

			//set right childs' EC
			List *rSetList = NIL;
			rSetList = concatTwoLists(copyObject(newRootEC), copyObject(rEC));
			rSetList = CombineDuplicateElemSetInECList(rSetList);
			setStringProperty((QueryOperator *)OP_RCHILD(root), PROP_STORE_SET_EC, (Node *)rSetList);
		}

		if(((SetOperator *)root)->setOpType == SETOP_INTERSECTION)
		{
			//set left child's EC
			setStringProperty((QueryOperator *)OP_LCHILD(root), PROP_STORE_SET_EC, (Node *)copyObject(rootEC));

			//SCH(R)/SCH(S)
			List *rootSetList = NIL;
			rootSetList = LSCHtoRSCH(rootSetList,rootEC,rattrDefs,lattrDefs);

			//set right child's EC
			setStringProperty((QueryOperator *)OP_RCHILD(root), PROP_STORE_SET_EC, (Node *)copyObject(rootSetList));
		}
		if(((SetOperator *)root)->setOpType == SETOP_DIFFERENCE)
		{
			List *lResultEC = copyObject(rootEC);
			setStringProperty((QueryOperator *)OP_LCHILD(root), PROP_STORE_SET_EC, (Node *)lResultEC);

			List *rResultEC = NIL;
			rResultEC = LSCHtoRSCH(rResultEC,rootEC,rattrDefs,lattrDefs);
			rResultEC = concatTwoLists(rResultEC, copyObject(rEC));
			rResultEC= CombineDuplicateElemSetInECList(rResultEC);
			setStringProperty((QueryOperator *)OP_RCHILD(root), PROP_STORE_SET_EC, (Node *)rResultEC);
		}
	}
	else if(isA(root,WindowOperator))
	{
		List *rootEC = (List *)getStringProperty(root, PROP_STORE_SET_EC);
        List *newRootEC = copyObject(rootEC);

		WindowOperator *wOp = (WindowOperator *)root;
		char *f = wOp->attrName;

		List *EC = NIL;
		FOREACH(KeyValue, kv, newRootEC)
		{
			Set *s = (Set *)kv->key;
			if(hasSetElem(s, f))
			{
				removeSetElem(s,f);
				if(setSize(s) != 0)
				    EC = appendToTailOfList(EC, kv);
			}
			else
				EC = appendToTailOfList(EC, kv);
		}

		setStringProperty(OP_LCHILD(root), PROP_STORE_SET_EC, (Node *)EC);
	}
	else if(isA(root, OrderOperator))
	{
		List *rootEC = (List *) getStringProperty(root, PROP_STORE_SET_EC);
		List *EC = copyObject(rootEC);
		setStringProperty(OP_LCHILD(root), PROP_STORE_SET_EC, (Node *)EC);
	}

	if(root->inputs != NULL)
	{
		FOREACH(QueryOperator, op, root->inputs)
	        				computeECPropTopDown(op);
	}
}

void
SCHBtoAUsedInTopBom(List **setList, List *attrRefs, List *attrDefs)
{
	FOREACH(KeyValue, kv, *setList)
	{
	    Set *s = (Set *) kv->key;
	    FORBOTH_LC(ar, ad, attrRefs, attrDefs)
        {
	        char *d = ((AttributeDef *)LC_P_VAL(ad))->attrName;
	        if(hasSetElem(s,d))
	        {
	            Node *pExpr = LC_P_VAL(ar);
	            removeSetElem (s,d);
	            if (isA(pExpr,AttributeReference)) {
	                char *r = strdup(((AttributeReference *) pExpr)->name);
                    addToSet(s,r);
	            }
	        }
        }
	}
}

List *
SCHAtoBUsedInBomUp(List *setList, List *childECSetList, List *attrA, List *attrB)
{
    List *tempList = childECSetList;
    //HashMap *childAToParent = NEW_MAP(Constant,Constant);
    HashMap *childAToParent = NEW_MAP(Constant,Node);

    // create map from child attrname to parent attrname
    // key is attrname, value is a set of attrname, eg. if A->X,A-Y then key:A, value:{X,Y}
    FORBOTH(Node,a,b,attrA,attrB)
    {
        AttributeDef *aDef = (AttributeDef *) b;
        if (isA(a, AttributeReference))
        {
            AttributeReference *aRef = (AttributeReference *) a;
            if(MAP_HAS_STRING_KEY(childAToParent, aRef->name))
            {
                Set *set = (Set *)getMapString(childAToParent, aRef->name);
                addToSet(set, strdup(aDef->attrName));
            }
            else
            {
            	Set *set = MAKE_STR_SET(strdup(aDef->attrName));
            	//MAP_ADD_STRING_KEY(childAToParent, aRef->name, createConstString(aDef->attrName));
            	MAP_ADD_STRING_KEY(childAToParent, aRef->name, (Node *)set);
            }
            DEBUG_LOG("map %s to %s", aRef->name, aDef->attrName);
        }
    }

    // translate attribute references in the sets
    FOREACH(KeyValue, kv, tempList)
    {
        Set *s = (Set *) kv->key;
        Constant *c = (Constant *) kv->value;
        Set *newEC = STRSET();
        KeyValue *newKv;

        FOREACH_SET(char,attr,s)
        {
            if(MAP_HAS_STRING_KEY(childAToParent, attr))
            {
            	Set *set = (Set *)getMapString(childAToParent, attr);
            	FOREACH_SET(char, a, set)
            	       addToSet(newEC,strdup(a));
            }
        }

        DEBUG_LOG("mapped EC <%s> to <%s>", nodeToString(s), nodeToString(newEC));

        if(setSize(newEC) != 0) {
            newKv = createNodeKeyValue ((Node *) newEC, (Node *) c);
            setList = appendToTailOfList(setList, newKv);
        }
    }

    // create ECs for attributes that are not simple references to child attributes
    FORBOTH(Node,a,b,attrA,attrB)
    {
        AttributeDef *aDef = (AttributeDef *) b;
        if (!isA(a, AttributeReference))
        {
            Set *newEC = MAKE_STR_SET(strdup(aDef->attrName));
            KeyValue *kv = createNodeKeyValue((Node *) newEC, NULL);
            setList = appendToTailOfList(setList,
                    (Node *) kv);
            DEBUG_LOG("added new EC <%s>", nodeToString(newEC));
        }
    }

    DEBUG_LOG("adapted EC list for projection is %s", nodeToString(setList));

    return setList;
}

List *
LSCHtoRSCH(List *setList, List *rECSetList, List *lSchemaList, List *rSchemaList)
{
	setList = copyObject(rECSetList);
	FOREACH(KeyValue, kv, setList)
	{
	    Set *s = (Set *) kv->key;

		FORBOTH(char,l1,l2, lSchemaList, rSchemaList)
		{
			if(hasSetElem(s, l2))
			{
				removeSetElem(s, l2);
				addToSet(s, l1);
			}
		}
	}

	return setList;
}

List *
CombineDuplicateElemSetInECList(List *DupECList)
{
		boolean change = FALSE;
		//do a deep copy so we have the original list around
		List *list1 = copyList(DupECList);

		 //Main loop1 of fix point computation that unions overlapping sets
		do{
		    List *list2 = copyList(list1);
		    list2 = removeFromHead(list2);
		    change = FALSE;

		    //Main loop2
		    FOREACH(KeyValue, kv1, list1)
		    {
		        Set *s1 = (Set *) kv1->key;
		        Constant *c1 = (Constant *) kv1->value;
                //Second list
                FOREACH(KeyValue, kv2, list2)
                {
                    Set *s2 = (Set *) kv2->key;
                    Constant *c2 = (Constant *) kv2->value;

                    if (overlapsSet(s1,s2) || (equal(c1,c2) && c1 != NULL))
                    {
                        //1, change flag
                        change = TRUE;

                        //2, union two sets
                        Set *newSet = unionSets(s1, s2);
                        Constant *newC = (c1 != NULL) ? c1 : c2;
                        KeyValue *newKv = createNodeKeyValue((Node *) newSet, (Node *) newC);
                        //TODO check for conflicting constants
                        DupECList = REMOVE_FROM_LIST_PTR(DupECList, kv1);
                        DupECList = replaceNode(DupECList, kv2, newKv);

                        break;
                    }
                }

		        if(change == TRUE)
		            break;
		        else
		            list2 = removeFromHead(list2);
		    }
		    list1 = copyList(DupECList);
		} while(change == TRUE);

	    //filter the empty sets
	    List *DupECList1 = NIL;
	    FOREACH(KeyValue, kv, DupECList)
	    {
	        Set *s = (Set *) kv->key;
	    	if(setSize(s) != 0)
	    		DupECList1 = appendToTailOfList(DupECList1, kv);
	    }
		return DupECList1;
}


// If cond = (a=5) AND (b=c)
// COndECSetList = {{a,5},{b,c}} which is a List of Sets
// Use the ptr set, a is the pointer point to char a, 5 is the pointer point to a constant structure
List *
GenerateCondECSetListUsedInBottomUp(Node *op)
{

	List *condList = NIL;
	getSelectionCondOperatorList(op, &condList);
    condList = GenerateCondECBasedOnCondOp(condList);

	return condList;
}

List *
GenerateCondECBasedOnCondOp(List *condList)
{
    List *result = NIL;
	FOREACH(Operator, o, condList)
	{
		if(streq(o->name,"="))
		{
			Set *s = STRSET();
			Constant *c = NULL;

			if(isA(getHeadOfListP(o->args), AttributeReference))
			{
	           char *arName1 = ((AttributeReference *)(getHeadOfListP(o->args)))->name;
	           addToSet(s,arName1);
			}
			else if(isA(getHeadOfListP(o->args), Constant))
			{
			   c =  (Constant *)getHeadOfListP(o->args);
//			   addToSet(s,c1);
			}
			else
			{
				//TODO: return error
			}

			if(isA(getTailOfListP(o->args), AttributeReference))
			{
	           char *arName2 = ((AttributeReference *)(getTailOfListP(o->args)))->name;
	           addToSet(s,arName2);
			}
			else if(isA(getTailOfListP(o->args), Constant))
			{
			   Constant *c2 =  (Constant *)getTailOfListP(o->args);
			   // if two unequal constants are compared then we add a NULL-constant to the EC to indicate that there is a contradiction
			   if (c != NULL && !equal(c,c2))
			       c = createNullConst(c->constType);
			   else
			       c = c2;
			}
			else
			{
				//TODO: return error
			}

			KeyValue *kv = createNodeKeyValue((Node *) s, (Node *) c);
			result = appendToTailOfList(result, kv);
		}
	}

	return result;
}

void initializeSetProp(QueryOperator *root)
{
	SET_BOOL_STRING_PROP((QueryOperator *)root, PROP_STORE_BOOL_SET);

	FOREACH(QueryOperator, o, root->inputs)
	{
		initializeSetProp(o);
	}
}

void
computeSetProp (QueryOperator *root)
{
	if (isA(root, ProjectionOperator) || isA(root, SelectionOperator))
	{
		QueryOperator *lChild = OP_LCHILD(root);

		boolean rootprop = GET_BOOL_STRING_PROP(root, PROP_STORE_BOOL_SET);
		if (lChild)
		{
			boolean childprop = GET_BOOL_STRING_PROP(lChild, PROP_STORE_BOOL_SET);
			boolean finalchildprop = rootprop && childprop;
			setStringProperty((QueryOperator *) lChild, PROP_STORE_BOOL_SET, (Node *) createConstBool(finalchildprop));
		}
	}

	if (isA(root, DuplicateRemoval))
	{
		QueryOperator *lChild = OP_LCHILD(root);

		if(lChild)
		{
			boolean childprop = GET_BOOL_STRING_PROP(lChild, PROP_STORE_BOOL_SET);
			boolean finalchildprop = TRUE && childprop;
			setStringProperty((QueryOperator *) lChild, PROP_STORE_BOOL_SET, (Node *) createConstBool(finalchildprop));
		}
	}

	if (isA(root, JoinOperator))
	{
		QueryOperator *lChild = OP_LCHILD(root);
		QueryOperator *rChild = OP_RCHILD(root);

		boolean rootprop = GET_BOOL_STRING_PROP(root, PROP_STORE_BOOL_SET);

		if (lChild && rChild)
		{
			boolean leftchildprop = GET_BOOL_STRING_PROP(lChild, PROP_STORE_BOOL_SET);
			boolean rightchildprop = GET_BOOL_STRING_PROP(lChild, PROP_STORE_BOOL_SET);
			boolean finalleftchildprop = rootprop && leftchildprop;
			boolean finalrightchildprop = rootprop && rightchildprop;
			setStringProperty((QueryOperator *) lChild, PROP_STORE_BOOL_SET, (Node *) createConstBool(finalleftchildprop));
			setStringProperty((QueryOperator *) lChild, PROP_STORE_BOOL_SET, (Node *) createConstBool(finalrightchildprop));
		}
	}

	FOREACH(QueryOperator, o, root->inputs)
	{
		computeSetProp(o);
	}

}




Set *
AddAttrOfSelectCondToSet(Set *set, Operator *op)
{
   if(op->name != NULL)
   {
	   Node *n1 = (Node *)getHeadOfListP(op->args);
	   Node *n2 = (Node *)getTailOfListP(op->args);
	   //Left
       if(isA(n1,Operator))
       {
    	   set = AddAttrOfSelectCondToSet(set, (Operator *)n1);
       }

       //Right
       if(isA(n2,Operator))
       {
    	   set = AddAttrOfSelectCondToSet(set, (Operator *)n2);
       }

       if(isA(n1,AttributeReference))
       {
    	   char * attrName = ((AttributeReference *)n1)->name;
    	   addToSet(set, strdup(attrName));
       }

       if(isA(n2,AttributeReference))
       {
    	   char * attrName = ((AttributeReference *)n2)->name;
    	   addToSet(set, strdup(attrName));
       }

   }

   return set;
}

void initializeIColProp(QueryOperator *root)
{
	Set *icols;
	List *icolsList = NIL;
	icolsList = getQueryOperatorAttrNames(root);
	icols = makeStrSetFromList(icolsList);
	setStringProperty((QueryOperator *)root, PROP_STORE_SET_ICOLS, (Node *)icols);
}

/*
 * Compute which set of output columns produced by the operator are need upstream.
 * Will be used to remove unneeded attributes early on. In particular useful for
 * removing the ROWNUM and other window operators introduced by PI-CS composable
 * rewrites.
 */
void
computeReqColProp (QueryOperator *root)
{
	/*
	 * Get root's icols set which can be used in following each operator
	 */
	Set *icols = (Set*)getProperty(root, (Node *) createConstString(PROP_STORE_SET_ICOLS));

	if(isA(root, SelectionOperator))
	{
		//icols = (Set*)getProperty(root, (Node *) createConstString(PROP_STORE_SET_ICOLS));
		//Set *childicols = (Set*)getProperty(OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_ICOLS));

		//Get the conditions of Selection Operator and add it to Set
		Operator *condOp = (Operator *)(((SelectionOperator *)root)->cond);
		Set *condicols = STRSET();
		condicols = AddAttrOfSelectCondToSet(condicols,condOp);
		DEBUG_LOG("length of set: %d \n",setSize(condicols));

		/*
		 * Reset itself's property which should union the condition set
		 * Same with its child's icols property
		 */
		//icols = unionSets(icols,condicols);
		//setStringProperty((QueryOperator *) root, PROP_STORE_SET_ICOLS, (Node *)icols);

		//Union this two sets and set it as its child icols property
		Set *eicols;
		eicols = unionSets(icols, condicols);
		setStringProperty((QueryOperator *) OP_LCHILD(root), PROP_STORE_SET_ICOLS, (Node *)eicols);
	}

	else if(isA(root, ProjectionOperator))
	{
		//Set *icols = (Set*)getProperty(root, (Node *) createConstString(PROP_STORE_SET_ICOLS));
		//Set *childicols = (Set*)getProperty(OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_ICOLS));

		//Get Reference Attribute Names and put it into a set
        Schema *opSchema = ((ProjectionOperator *)root)->op.schema;
        List *attrDefNames = getAttrNames(opSchema);
        List *attrRefList = ((ProjectionOperator *)root)->projExprs;
        List *eicolsList = NIL;

        //e.g. Project(A,X) from Project(A->A, B+C->X, D->D) from R{A,B,C,D}, set icols(R)
        //icols = {A,X}
        //schema = {A,X,D}
        //eicols = {A,B,C}
        //if AttrDefName in icols, get correspond AttrRef name (need to check if it is operator)
        FORBOTH_LC(a,ar, attrDefNames,attrRefList)
        {
            //TODO this should deal with any other type of expression
        	if(hasSetElem(icols,LC_P_VAL(a)))
        	{
        	    List *attrRefs = getAttrReferences(LC_P_VAL(ar));
//        		if(isA(LC_P_VAL(ar), Operator))
//        			eicolsList = getAttrNameFromOpExpList(eicolsList, (Operator *)(LC_P_VAL(ar)));
//        		else if(isA(LC_P_VAL(ar), AttributeReference))
//        			eicolsList = appendToTailOfList(eicolsList, strdup(((AttributeReference *)(LC_P_VAL(ar)))->name));
        		eicolsList = CONCAT_LISTS(eicolsList, attrRefListToStringList(attrRefs));
        	}
        }

        Set *eicols = makeStrSetFromList(eicolsList);
        setStringProperty((QueryOperator *) OP_LCHILD(root), PROP_STORE_SET_ICOLS, (Node *)eicols);
	}

	else if(isA(root, DuplicateRemoval))
	{
		Set *eicols = copyObject(icols);
		setStringProperty((QueryOperator *) OP_LCHILD(root), PROP_STORE_SET_ICOLS, (Node *)eicols);
	}

	else if (isA(root, JoinOperator))
	{
		List *l1 = getQueryOperatorAttrNames(OP_LCHILD(root));
		List *l2 = getQueryOperatorAttrNames(OP_RCHILD(root));
		Set *s1 = makeStrSetFromList(l1);
		Set *s2 = makeStrSetFromList(l2);

		if(((JoinOperator*)root)->joinType == JOIN_CROSS)
		{
			// Set icols for left child
			Set *e1icols = intersectSets(icols, s1);
			setStringProperty((QueryOperator *) OP_LCHILD(root), PROP_STORE_SET_ICOLS, (Node *)e1icols);

			// Set icols for right child
			Set *e2icols = intersectSets(icols, s2);
			setStringProperty((QueryOperator *) OP_RCHILD(root), PROP_STORE_SET_ICOLS, (Node *)e2icols);
		}

		if (((JoinOperator*)root)->joinType == JOIN_INNER)
		{
			Operator *condOp = (Operator *)(((JoinOperator *)root)->cond);
			//TODO Initialize Set without having some value
			Set *condicols = STRSET();
			condicols = AddAttrOfSelectCondToSet(condicols,condOp);

			/*
			 * Reset itself's property which should union the condition set
			 */
			icols = unionSets(icols,condicols);

			// Set icols for left child
			Set *e1icols = intersectSets(icols, s1);
			setStringProperty((QueryOperator *) OP_LCHILD(root), PROP_STORE_SET_ICOLS, (Node *)e1icols);

			//Set icols for right child
			Set *e2icols = intersectSets(icols, s2);
			setStringProperty((QueryOperator *) OP_RCHILD(root), PROP_STORE_SET_ICOLS, (Node *)e2icols);


		}
	}

	else if(isA(root, AggregationOperator))
	{
		AggregationOperator *agg = (AggregationOperator *)root;
		Set *set = STRSET();

		//e.g. add sum(A) to set
		FOREACH(FunctionCall, a, agg->aggrs)
		{
			//TODO: ar should get from list args, not only the head one
			AttributeReference *ar = (AttributeReference *)(getHeadOfListP(a->args));
			addToSet(set,strdup(ar->name));
		}

		//e.g. add group by B into set
		FOREACH(AttributeReference, a, agg->groupBy)
		{
		    addToSet(set,strdup(a->name));
		}

		setStringProperty((QueryOperator *) OP_LCHILD(root), PROP_STORE_SET_ICOLS, (Node *)set);
	}

	else if(isA(root,OrderOperator))
	{
		//Get attributes from order by
		Set *ordSet = STRSET();
		List *ordList = ((OrderOperator *)root)->orderExprs;
		FOREACH(OrderExpr, o, ordList)
		{
            AttributeReference *ar = (AttributeReference *)(o->expr);
			addToSet(ordSet,strdup(ar->name));
		}

		/*
		 * Reset itself's property which should union the condition set
		 * Same with its child's icols property
		 */

		Set *eicols = unionSets(ordSet, icols);
		setStringProperty((QueryOperator *) OP_LCHILD(root), PROP_STORE_SET_ICOLS, (Node *)eicols);
		setStringProperty((QueryOperator *) root, PROP_STORE_SET_ICOLS, (Node *)eicols);
	}

	else if(isA(root,WindowOperator))
	{
		/*
		 * Get need attribute name in window function, such as the attributes
		 * in FunctionCall, PartitionBy and OrderBy
		 */

		//(1)FunctionalCall, e.g. SUM(A), add A to set
        Set *winSet = STRSET();
        List *funList = ((FunctionCall *)(((WindowOperator *)root)->f))->args;
        FOREACH(AttributeReference, ar, funList)
        {
        	addToSet(winSet,strdup(ar->name));
        }

        //(2)PartitionBy
        List *parList = ((WindowOperator *)root)->partitionBy;
        if(parList != NIL)
        {
        	FOREACH(AttributeReference, ar, parList)
        	{
        		addToSet(winSet,strdup(ar->name));
        	}
        }
        //(3)OrderBy
        List *ordList = ((WindowOperator *)root)->orderBy;
        if(ordList != NIL)
        {
        	FOREACH(AttributeReference, ar, ordList)
        	{
        		addToSet(winSet,strdup(ar->name));
        	}
        }
        /*
         * Get child's schema as schemaSet, then
         * e.icols = (icols union winSet) intersect with (schemaSet)
         * Need intersect get rid of winf_0, union because icols has C, need add C
         * e.g.   proj(X,C) AS (X,C)                          icols(X,C)
         *        proj(winf_0,C) AS (X,C)                     icols(X,C)
         *        window[SUM(A)][B][]  schema[A,B,C,D,winf_0] icols(winf_0,C) can remove D
         *        R(A,B,C,D)                                  icols(A,B,C) add proj(A,B,C)
         */
        //QueryOperator *child = OP_LCHILD(root->inputs);
        QueryOperator *child = OP_LCHILD(&((WindowOperator *)root)->op);
        List *schemaList = getQueryOperatorAttrNames(child);
        Set *schemaSet = makeStrSetFromList(schemaList);
        Set *eicols;
        eicols = unionSets(winSet, icols);
        eicols = intersectSets(eicols, schemaSet);
		setStringProperty((QueryOperator *) OP_LCHILD(root), PROP_STORE_SET_ICOLS, (Node *)eicols);

	}


	FOREACH(QueryOperator, o, root->inputs)
	{
		computeReqColProp(o);
	}
}

void
printIcols(QueryOperator *root)
{
	Set *icols = (Set*)getProperty(root, (Node *) createConstString(PROP_STORE_SET_ICOLS));
	DEBUG_LOG("icols:%s\n ",nodeToString(icols));

	FOREACH(QueryOperator, o, root->inputs)
	{
		printIcols(o);
	}
}

static List *
attrRefListToStringList (List *input)
{
    List *result = NIL;

    FOREACH(AttributeReference,a,input)
        result = appendToTailOfList(result,strdup(a->name));

    return result;
}

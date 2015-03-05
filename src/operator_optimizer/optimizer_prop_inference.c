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
#include "metadata_lookup/metadata_lookup_oracle.h"
#include "include/log/logger.h"
#include "include/model/query_operator/operator_property.h"


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
        keyList = getKeyInformation(root);
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
	computeECPropBottomUp(root);
	DEBUG_LOG("*********EC**********");
	printECPro(root);
	computeECPropTopDown(root);
	DEBUG_LOG("*********EC**********");
	printECPro(root);
}

void
printSingleECList(List *l)
{
	DEBUG_LOG("SET LIST: %s, SIZE LIST %d", nodeToString(l), l->length);
	FOREACH(Set, s, l)
	{
		DEBUG_LOG("Set: ");
		FOREACH_SET(Node, n, s)
        {
			if(isA(n, Constant))
			{
				DEBUG_LOG("%s ", nodeToString(n));
			}
			else
			   DEBUG_LOG("%s ", (char *)n);
        }
		DEBUG_LOG("\n");
	}
}

void
printECPro(QueryOperator *root)
{
	if(isA(root, ProjectionOperator))
		printf("Projection Operator: ");
	if(isA(root, TableAccessOperator))
		printf("Table Access Operator: ");
	if(isA(root, AggregationOperator))
		printf("Aggregation Operator: ");
	if(isA(root, SelectionOperator))
		printf("Selection Operator: ");
	if(isA(root, JoinOperator))
		printf("Join Operator: ");

	Node *nRoot = getProperty(root, (Node *) createConstString(PROP_STORE_SET_EC));
	List *list = (List *)nRoot;
	DEBUG_LOG("List size %d", list->length);
	FOREACH(Set, s, list)
	{
		printf("{");
		FOREACH_SET(Node, n, s)
		{
			if(isA(n, Constant))
			{
				printf(" %s ", nodeToString(n));
			}
			else
				printf(" %s ", (char *)n);
		}
		printf("} ");
	}
	printf("\n");

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
			List *setList = NIL;
			FOREACH(AttributeDef,a, root->schema->attrDefs)
			{
				Set *s = MAKE_SET_PTR(a->attrName);
				setList = appendToTailOfList(setList, s);
			}

			setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_SET_EC), (Node *)setList);
		}

		else if(isA(root, SelectionOperator))
		{
			QueryOperator *childOp = OP_LCHILD(root);
			Node *nChild = getProperty(childOp, (Node *) createConstString(PROP_STORE_SET_EC));
			List *childECSetList = copyList((List *)nChild);

			List *CondECSetList = NIL;
			Operator *op = (Operator *)(((SelectionOperator *)root)->cond);
			CondECSetList = GenerateCondECSetListUsedInBottomUp(CondECSetList, op);

			//Union the child's EC list with the Cond EC list
			List *tempList = concatTwoLists(childECSetList, CondECSetList);

			//remove the Duplicate set in the list (which has the same element)
			List *setList = CombineDuplicateElemSetInECList(tempList);
			setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_SET_EC), (Node *)setList);
		}

		else if(isA(root, ProjectionOperator))
		{
			ProjectionOperator *pj = (ProjectionOperator *)root;
			//get list (contains attrRef or Op) from project op projExprs
			List *attrA = NIL;
			FOREACH_LC(l, pj->projExprs)
                 attrA =  appendToTailOfList(attrA, LC_P_VAL(l));

            //get attrDef list from project op schema
			List *attrB = NIL;
			FOREACH_LC(l, pj->op.schema->attrDefs)
                 attrB =  appendToTailOfList(attrB, LC_P_VAL(l));

			//get child EC property
			Node *nChildECSetList = getProperty(OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_EC));
			List *childECSetList = (List *)copyObject(nChildECSetList);

			List *setList = NIL;
			setList = SCHAtoBUsedInProJ(setList, childECSetList, attrA, attrB);
			setList = CombineDuplicateElemSetInECList(setList);
			setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_SET_EC), (Node *)setList);
		}

		else if(isA(root, JoinOperator))
		{
			List *setList = NIL;
			if (((JoinOperator*)root)->joinType == JOIN_INNER)
			{
				//1, Get cond set
                Operator *op = (Operator *)(((JoinOperator*)root)->cond);
                Set *set = PSET();

                if(streq(op->name,"="))
                {
                	if(isA(getHeadOfListP(op->args), AttributeReference))
                	{
                		char *lChildName = ((AttributeReference *)getHeadOfListP(op->args))->name;
                		addToSet(set,lChildName);
                	}
                	else
                	{
                		//TODO: return error
                	}

                	if(isA(getTailOfListP(op->args), AttributeReference))
                	{
                		char *rChildName = ((AttributeReference *)getTailOfListP(op->args))->name;
                		addToSet(set,rChildName);
                	}
                	else
                	{
                		//TODO: return error
                	}
                }
                List *condList = NIL;
                condList = appendToTailOfList(condList, set);

                //2, union it with EC(Rchild) and EC(Lchild)
        		QueryOperator *lChild = OP_LCHILD(root);
        		QueryOperator *rChild = OP_RCHILD(root);

    			Node *lNChild = getProperty(lChild, (Node *) createConstString(PROP_STORE_SET_EC));
    			Node *rNChild = getProperty(rChild, (Node *) createConstString(PROP_STORE_SET_EC));

    			List *lChildECSetList = (List *)lNChild;
    			List *rChildECSetList = (List *)rNChild;

    			setList = concatTwoLists(copyList(lChildECSetList), copyList(rChildECSetList));
    			setList = concatTwoLists(setList, copyList(condList));

    			//3, Duplicate remove
    			setList = CombineDuplicateElemSetInECList(setList);
    			setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_SET_EC), (Node *)setList);
			}

			if (((JoinOperator*)root)->joinType == JOIN_CROSS)
			{
				QueryOperator *lChild = OP_LCHILD(root);
				QueryOperator *rChild = OP_RCHILD(root);

				Node *lNChild = getProperty(lChild, (Node *) createConstString(PROP_STORE_SET_EC));
				Node *rNChild = getProperty(rChild, (Node *) createConstString(PROP_STORE_SET_EC));

				List *lChildECSetList = (List *)lNChild;
				List *rChildECSetList = (List *)rNChild;

				setList = concatTwoLists(copyList(lChildECSetList), copyList(rChildECSetList));
				setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_SET_EC), (Node *)setList);
			}
		}

		else if(isA(root, AggregationOperator))
		{
			Set *setGroupBy = PSET();
            AggregationOperator *agg = (AggregationOperator *)root;

			//step 1, get GroupBy set
			FOREACH(AttributeReference, ar, agg->groupBy)
                   addToSet(setGroupBy, ar->name);

			//step 2
			Node *childECP = getProperty(OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_EC));
			List *childECSetList = (List *)childECP;
			Set *newSet = NULL;;
			List *setList = NIL;
			List *setList1 = NIL;

			FOREACH(Set, s, childECSetList)
			{
				newSet = intersectPtrSets(setGroupBy, s);
				if(setSize(newSet) != 0)
					setList1 = appendToTailOfList(setList1, newSet);
			}

			List *cmpGrByADef = copyList(agg->op.schema->attrDefs);
			popHeadOfListP(cmpGrByADef);
			//change attrRef name in Group By to attrDef in Schema
			setList = SCHAtoBUsedInProJ(setList, setList1, agg->groupBy, cmpGrByADef);

			//step 3
			//get SUM(A) and append to tail of list setList, then set property
            int size = LIST_LENGTH(agg->aggrs);
            Set *tempSet;
            int i = 0;
            FOREACH(AttributeDef, ad, agg->op.schema->attrDefs)
            {
            	tempSet = MAKE_SET_PTR(ad->attrName);
            	setList = appendToTailOfList(setList, tempSet);
            	i++;
            	if(i == size)
            		break;
            }

			setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_SET_EC), (Node *)setList);
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
			Node *lChildECP = getProperty(OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_EC));
			List *lECSetList = (List *)copyObject(lChildECP);
			Node *rChildECP = getProperty(OP_RCHILD(root), (Node *) createConstString(PROP_STORE_SET_EC));
			List *rECSetList = (List *)copyObject(rChildECP);

			//get schema list of left child and right child
			List *lSchemaList = NIL;
			FOREACH(AttributeDef,a, ((QueryOperator *)(OP_LCHILD(root)))->schema->attrDefs)
					lSchemaList = appendToTailOfList(lSchemaList, a->attrName);

			List *rSchemaList = NIL;
			FOREACH(AttributeDef,a, ((QueryOperator *)(OP_RCHILD(root)))->schema->attrDefs)
				    rSchemaList = appendToTailOfList(rSchemaList, a->attrName);

			if(((SetOperator *)root)->setOpType == SETOP_UNION)
			{
			    //step 1, SCH(S)/SCH(R)
				List *rSetList = NIL;
				rSetList = LSCHtoRSCH(rSetList,rECSetList,lSchemaList,rSchemaList);

				//step 2, intersect each set
				Set *tempSet;
				List *setList = NIL;
				FOREACH(Set, s1, lECSetList)
				{
					FOREACH(Set, s2, rSetList)
		            {
						tempSet = intersectPtrSets(copyObject(s1),copyObject(s2));
						if(setSize(tempSet) != 0)
							setList = appendToTailOfList(setList, tempSet);
		            }
				}

				setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_SET_EC), (Node *)setList);
			}

			if(((SetOperator *)root)->setOpType == SETOP_INTERSECTION)
			{
			    //SCH(S)/SCH(R)
				List *setList = NIL;
				setList = LSCHtoRSCH(setList,rECSetList,lSchemaList,rSchemaList);

                setList = concatTwoLists(setList,lECSetList);
                setList = concatTwoLists(copyList(setList),copyList(lECSetList));
				setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_SET_EC), (Node *)setList);
			}

			if(((SetOperator *)root)->setOpType == SETOP_DIFFERENCE)
			{
				Node *childECP = getProperty(OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_EC));
				List *setList = copyList((List *)childECP);
				setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_SET_EC), (Node *)setList);
			}

		}
		else
		{
			Node *childECP = getProperty(OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_EC));
			List *setList = (List *)childECP;
			setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_SET_EC), (Node *)setList);
		}
	}
}

void
computeECPropTopDown (QueryOperator *root)
{

	if(isA(root, SelectionOperator))
	{
		Node *nRoot = getProperty(root, (Node *) createConstString(PROP_STORE_SET_EC));

		QueryOperator *childOp = OP_LCHILD(root);
		setProperty((QueryOperator *)childOp, (Node *) createConstString(PROP_STORE_SET_EC ), nRoot);
	}

	else if(isA(root, ProjectionOperator))
	{
		Node *nRoot = getProperty(root, (Node *) createConstString(PROP_STORE_SET_EC));
		List *rList = (List *)nRoot;

		List *setList = NIL;
		FOREACH(Set, s, rList)
		{
			//Set *s = unionSets(s1,s1);
			Set *s1 = copyObject(s);
			setList = appendToTailOfList(setList, s1);
		}

		ProjectionOperator *pj = (ProjectionOperator *)root;
		List *schemaList1 = copyList(pj->op.schema->attrDefs);
		List *attrRefList1 = copyList(pj->projExprs);

		FOREACH(Set, s, setList)
		{
			FORBOTH_LC(a1, s1, attrRefList1, schemaList1)
            		{
				char *d1 = ((AttributeDef *)LC_P_VAL(s1))->attrName;
				if(hasSetPtrElem(s,d1))
				{
					char *r1 = ((AttributeReference *)LC_P_VAL(a1))->name;
					removeSetPtrElem (s,d1);
					addToSet(s,r1);
				}
            		}
		}

		setProperty((QueryOperator *)(OP_LCHILD(root)), (Node *) createConstString(PROP_STORE_SET_EC ), (Node *)setList);

	}

	//contains join inner and join cross
	else if(isA(root, JoinOperator))
	{
		//Join operator EC
		Node *nRoot = getProperty(root, (Node *) createConstString(PROP_STORE_SET_EC));
		List *rootECSetList = (List *)nRoot;

		//SCH(Left Child)
		Set *lSchemaSet = PSET();
		FOREACH(AttributeDef,a, ((QueryOperator *)(OP_LCHILD(root)))->schema->attrDefs)
		        addToSet(lSchemaSet,a->attrName);

		//SCH(Right Child)
		Set *rSchemaSet = PSET();
		FOREACH(AttributeDef,a, ((QueryOperator *)(OP_RCHILD(root)))->schema->attrDefs)
		        addToSet(rSchemaSet,a->attrName);

		//get EC(left)
		Set *tempSet;
		List *lSetList = NIL;
        FOREACH(Set, s, rootECSetList)
		{
        	tempSet = setPtrDifference(s, rSchemaSet);
        	if(setSize(tempSet) > 1)
        		lSetList = appendToTailOfList(lSetList, tempSet);
        	else if(setSize(tempSet) == 1 && !isA(tempSet->elem->data,Constant))
        		lSetList = appendToTailOfList(lSetList, tempSet);
		}
        QueryOperator *lChildOp = OP_LCHILD(root);
		setProperty(lChildOp, (Node *) createConstString(PROP_STORE_SET_EC ), (Node *)lSetList);

        //get EC(right)
		List *rSetList = NIL;
        FOREACH(Set, s, rootECSetList)
		{
        	tempSet = setPtrDifference(s, lSchemaSet);
        	if(setSize(tempSet) > 1)
        		rSetList = appendToTailOfList(rSetList, tempSet);
        	else if(setSize(tempSet) == 1 && !isA(tempSet->elem->data,Constant))
        		rSetList = appendToTailOfList(rSetList, tempSet);
		}
        QueryOperator *rChildOp = OP_RCHILD(root);
		setProperty(rChildOp, (Node *) createConstString(PROP_STORE_SET_EC ), (Node *)rSetList);

	}

	else if(isA(root, AggregationOperator))
	{
		//get EC(Root)
		Node *nRoot = getProperty(root, (Node *) createConstString(PROP_STORE_SET_EC));
		List *rootECSetList = (List *)nRoot;

		//get EC(R)
		QueryOperator *childOp = OP_LCHILD(root);
		Node *nChild = getProperty(childOp, (Node *) createConstString(PROP_STORE_SET_EC));
		List *childECSetList = (List *)nChild;

		//get SCH(R)
		Set *childSchemaSet = PSET();
		FOREACH(AttributeDef,a, ((QueryOperator *)(OP_LCHILD(root)))->schema->attrDefs)
		       addToSet(childSchemaSet,a->attrName);

		List *rootBtoAECSetList = NIL;
		List *childSchemaSetList1 = copyObject(rootECSetList);
		List *cmpGrByADef = copyList(root->schema->attrDefs);
		popHeadOfListP(cmpGrByADef);
		List *groupBy = ((AggregationOperator *)root)->groupBy;
		rootBtoAECSetList = SCHBtoAUsedInAgg(rootBtoAECSetList, childSchemaSetList1, cmpGrByADef, groupBy);

       //main steps
		List *setList = NIL;
		Set *tempSet;
		FOREACH(Set, s, rootBtoAECSetList)
		{
			tempSet = intersectPtrSets(s, childSchemaSet);
            setList = appendToTailOfList(setList, tempSet);
		}

		setList = concatTwoLists(setList, childECSetList);
		setList = CombineDuplicateElemSetInECList(setList);
		setProperty((QueryOperator *)(OP_LCHILD(root)), (Node *) createConstString(PROP_STORE_SET_EC ), (Node *)setList);

	}

	else if(isA(root, DuplicateRemoval))
	{
		Node *rootECP = getProperty(root, (Node *) createConstString(PROP_STORE_SET_EC));
		setProperty((QueryOperator *)OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_EC), rootECP);
	}

	else if(isA(root,SetOperator))
	{
		//get schema list of left child and right child
		List *lSchemaList = NIL;
		FOREACH(AttributeDef,a, ((QueryOperator *)(OP_LCHILD(root)))->schema->attrDefs)
		      lSchemaList = appendToTailOfList(lSchemaList, a->attrName);

		List *rSchemaList = NIL;
		FOREACH(AttributeDef,a, ((QueryOperator *)(OP_RCHILD(root)))->schema->attrDefs)
			  rSchemaList = appendToTailOfList(rSchemaList, a->attrName);

		Node *rootECP = getProperty(root, (Node *) createConstString(PROP_STORE_SET_EC));
		List *rootECSetList = (List *)rootECP;

		Node *lECP = getProperty(OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_EC));
		List *lECSetList = (List *)lECP;

		Node *rECP = getProperty(OP_RCHILD(root), (Node *) createConstString(PROP_STORE_SET_EC));
		List *rECSetList = (List *)rECP;

		if(((SetOperator *)root)->setOpType == SETOP_UNION)
		{
            //set left child's EC
			List *lSetList = concatTwoLists(copyList(rootECSetList), copyList(lECSetList));
			lSetList = CombineDuplicateElemSetInECList(lSetList);
			setProperty((QueryOperator *)OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_EC), (Node *)lSetList);

			//SCH(R)/SCH(S)
			List *rootSetList = NIL;
			rootSetList = LSCHtoRSCH(rootSetList,rootECSetList,rSchemaList,lSchemaList);

			//set right childs' EC
			rootSetList = concatTwoLists(copyList(rootSetList), copyList(rECSetList));
			rootSetList = CombineDuplicateElemSetInECList(rootSetList);
			setProperty((QueryOperator *)OP_RCHILD(root), (Node *) createConstString(PROP_STORE_SET_EC), (Node *)rootSetList);
		}

		if(((SetOperator *)root)->setOpType == SETOP_INTERSECTION || ((SetOperator *)root)->setOpType == SETOP_DIFFERENCE)
		{
			//set left child's EC
			setProperty((QueryOperator *)OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_EC), rootECP);

			//SCH(R)/SCH(S)
			List *rootSetList = NIL;
			rootSetList = LSCHtoRSCH(rootSetList,rootECSetList,rSchemaList,lSchemaList);

			//set right child's EC
			setProperty((QueryOperator *)OP_RCHILD(root), (Node *) createConstString(PROP_STORE_SET_EC), (Node *)rootSetList);

		}
	}

	if(root->inputs != NULL)
	{
		FOREACH(QueryOperator, op, root->inputs)
	        				computeECPropTopDown(op);
	}
}


List *
SCHAtoBUsedInProJ(List *setList, List *childECSetList, List *attrA, List *attrB)
{
	List *tempList = copyList(childECSetList);
		FOREACH(Set, s, tempList)
		{

					FORBOTH_LC(a,b,attrA,attrB)
			        {
                        //TODO: Operator in Projection attribute reference

						if(isA(LC_P_VAL(a),AttributeReference))
						{
							char *tempName = ((AttributeReference *)LC_P_VAL(a))->name;
							if(hasSetPtrElem(s,tempName))
							{
								removeSetPtrElem(s,tempName);
								addToSet(s,((AttributeDef *)LC_P_VAL(b))->attrName);
							}
						}
			        }

				if(setSize(s) != 0)
					setList = appendToTailOfList(setList, copyObject(s));
		}

		return setList;
}

List *
SCHBtoAUsedInAgg(List *setList, List *childECSetList, List *attrA, List *attrB)
{
	List *tempList = copyList(childECSetList);
	FOREACH(Set, s, tempList)
	{
		FORBOTH_LC(a,b,attrA,attrB)
		{
			if(isA(LC_P_VAL(a),AttributeDef))
			{
				char *tempName = ((AttributeDef *)LC_P_VAL(a))->attrName;
				if(hasSetPtrElem(s,tempName))
				{
					removeSetPtrElem(s,tempName);
					addToSet(s,((AttributeReference *)LC_P_VAL(b))->name);
				}
			}
		 }
		if(setSize(s) != 0)
			setList = appendToTailOfList(setList, copyObject(s));
	}

	return setList;
}

List *
LSCHtoRSCH(List *setList, List *rECSetList, List *lSchemaList, List *rSchemaList)
{
	setList = copyList(rECSetList);
	FOREACH(Set, s, setList)
	{
		FORBOTH(char,l1,l2, lSchemaList, rSchemaList)
		{
			if(hasSetPtrElem(s, l2))
			{
				removeSetPtrElem(s, l2);
				addToSet(s, l1);
			}
		}
	}

	return setList;
}

List *
CombineDuplicateElemSetInECList(List *DupECList)
{
		boolean flag = FALSE;
		List *list1 = copyList(DupECList);

	    //Main loop1
	    do{
	        List *list2 = copyList(list1);
	        list2 = removeFromHead(list2);
	    	flag = FALSE;

	    	//Main loop2
	    	FOREACH(Set, s1, list1)
	    	{
	    		FOREACH_SET(Node, n1, s1)
				{
	    			//Second list
	    			FOREACH(Set, s2, list2)
			    	{
	    				if(hasSetPtrElem(s2,n1))
	    				{
	    					//1, change flag
	    					flag = TRUE;

	    					//2, union two sets
	    					Set *newSet = unionPtrSets(s1, s2);
	    					DupECList = REMOVE_FROM_LIST_PTR(DupECList, s1);
	    					DupECList = replaceNode(DupECList, s2, newSet);
	    					break;
	    				}
			    	}
	    			if(flag == TRUE)
	    				break;
				}
	    		if(flag == TRUE)
	    			break;
	    		else
	    		{
	    			list2 = removeFromHead(list2);
	    		}
	    	}
	    	list1 = copyList(DupECList);
	    }while(flag == TRUE);

	    //filter the empty sets
	    List *DupECList1 = NIL;
	    FOREACH(Set, s, DupECList)
	    {
	    	if(setSize(s) != 0)
	    		DupECList1 = appendToTailOfList(DupECList1, s);
	    }
		return DupECList1;
}


// If cond = (a=5) AND (b=c)
// COndECSetList = {{a,5},{b,c}} which is a List of Sets
// Use the ptr set, a is the pointer point to char a, 5 is the pointer point to a constant structure
List *
GenerateCondECSetListUsedInBottomUp(List *CondECSetList, Operator *op)
{

	List *condList = NIL;
	if(streq(op->name,"AND"))
		condList = getSelectionCondOperatorList(condList,op);
	else if(streq(op->name, "="))
		condList = appendToTailOfList(condList,op);

	FOREACH(Operator, o, condList)
	{
		if(streq(o->name,"="))
		{
			Set *s = PSET();
			if(isA(getHeadOfListP(o->args), AttributeReference))
			{
	           char *arName1 = ((AttributeReference *)(getHeadOfListP(o->args)))->name;
	           addToSet(s,arName1);
			}
			else if(isA(getHeadOfListP(o->args), Constant))
			{
			   Constant *c1 =  (Constant *)getHeadOfListP(o->args);
			   addToSet(s,c1);
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
			   addToSet(s,c2);
			}
			else
			{
				//TODO: return error
			}

			CondECSetList = appendToTailOfList(CondECSetList, s);
		}
	}

	return CondECSetList;
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
        	if(hasSetElem(icols,LC_P_VAL(a)))
        	{
        		if(isA(LC_P_VAL(ar), Operator))
        		{
        			eicolsList = getAttrNameFromOpExpList(eicolsList, (Operator *)(LC_P_VAL(ar)));
        		}
        		else if(isA(LC_P_VAL(ar), AttributeReference))
        		{
        			eicolsList = appendToTailOfList(eicolsList, strdup(((AttributeReference *)(LC_P_VAL(ar)))->name));
        		}
        	}
        }

        Set *eicols = makeStrSetFromList(eicolsList);
        setStringProperty((QueryOperator *) OP_LCHILD(root), PROP_STORE_SET_ICOLS, (Node *)eicols);
	}

	else if(isA(root, DuplicateRemoval))
	{
		//Set *icols = (Set*)getProperty(root, (Node *) createConstString(PROP_STORE_SET_ICOLS));
		//Set *childicols = (Set*)getProperty(OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_ICOLS));

		//Union both sets and set it as its child icols property
		//Set *eicols;
		//eicols = unionSets(rooticols, childicols);
		Set *eicols = copyObject(icols);
		setStringProperty((QueryOperator *) OP_LCHILD(root), PROP_STORE_SET_ICOLS, (Node *)eicols);

	}

	else if (isA(root, JoinOperator))
	{
		List *l1 = getQueryOperatorAttrNames(OP_LCHILD(root));
		List *l2 = getQueryOperatorAttrNames(OP_RCHILD(root));

		Set *s1 = makeStrSetFromList(l1);
		Set *s2 = makeStrSetFromList(l2);

		//Set *icols = (Set*)getProperty(root, (Node *) createConstString(PROP_STORE_SET_ICOLS));
		if(((JoinOperator*)root)->joinType == JOIN_CROSS)
		{
			// Set icols for left child
			Set *e1icols = intersectSets(icols, s1);
			//Set *s4 = unionSets(s3, leftchildicols);
			setStringProperty((QueryOperator *) OP_LCHILD(root), PROP_STORE_SET_ICOLS, (Node *)e1icols);

			// Set icols for right child
			Set *e2icols = intersectSets(icols, s2);
			//Set *s6 = unionSets(s5, rightchildicols);
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
			//setStringProperty((QueryOperator *)root, PROP_STORE_SET_ICOLS, (Node *)icols);

			// Set icols for left child
			//Set *s3 = unionSets(icols, condicols);
			Set *e1icols = intersectSets(icols, s1);
			//Set *s5 = unionSets(s4, leftchildicols);
			setStringProperty((QueryOperator *) OP_LCHILD(root), PROP_STORE_SET_ICOLS, (Node *)e1icols);

			//Set icols for right child
			Set *e2icols = intersectSets(icols, s2);
			//Set *s8 = unionSets(s7, rightchildicols);
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

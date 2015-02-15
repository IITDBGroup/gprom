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
	computeECPropTopDown(root);
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
				//Set *s = MAKE_SET_PTR(a->attrName);
				char *aDName = copyObject(a->attrName);
				Set *s = MAKE_SET_PTR(aDName);
				setList = appendToTailOfList(setList, s);
			}

			setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_SET_EC ), (Node *)setList);
		}

		else if(isA(root, SelectionOperator))
		{
			QueryOperator *childOp = OP_LCHILD(root);
			Node *nChild = getProperty(childOp, (Node *) createConstString(PROP_STORE_SET_EC));
			List *childECSetList = (List *)nChild;

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
			//get list (contains attrRef or Op) from project op projExprs
			List *attrA = NIL;
			ProjectionOperator *pj = (ProjectionOperator *)root;
			FOREACH_LC(l, pj->projExprs)
                 attrA =  appendToTailOfList(attrA, LC_P_VAL(l));

            //get attrDef list from project op schema
			List *attrB = NIL;
			FOREACH_LC(l, pj->op.schema->attrDefs)
                 attrB =  appendToTailOfList(attrA, LC_P_VAL(l));

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

			if (((JoinOperator*)root)->joinType == JOIN_INNER)
			{
				//1, Get cond set
                Operator *op = (Operator *)(((JoinOperator*)root)->cond);
                Set *set;

                if(streq(op->name,"="))
                {
                	if(isA(getHeadOfListP(op->args), AttributeReference))
                	{
                		char *lChildName = (char *)getHeadOfListP(op->args);
                		set = MAKE_SET_PTR(lChildName);
                	}
                	else
                	{
                		//TODO: return error
                	}

                	if(isA(getTailOfListP(op->args), AttributeReference))
                	{
                		char *rChildName = (char *)getTailOfListP(op->args);
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

    			List *setList = concatTwoLists(lChildECSetList, rChildECSetList);
    			setList = concatTwoLists(setList, condList);

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

				List *setList = concatTwoLists(lChildECSetList, rChildECSetList);

				setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_SET_EC), (Node *)setList);
			}
		}

		else if(isA(root, AggregationOperator))
		{
			boolean flag = FALSE;
			Set *setGroupBy;
            AggregationOperator *agg = (AggregationOperator *)root;

			//step 1, get GroupBy set
			FOREACH(AttributeReference, ar, agg->groupBy)
			{
				char *arName = copyObject(ar->name);
				if(flag == FALSE)
				{
					setGroupBy = MAKE_SET_PTR(arName);
					flag = TRUE;
				}
				else
				{
					addToSet(setGroupBy, arName);
				}
			}

			//step 2
			Node *childECP = getProperty(OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_EC));
			List *childECSetList = (List *)childECP;
			Set *newSet;
			List *setList = NIL;

			FOREACH(Set, s, childECSetList)
			{
				newSet = intersectSets(setGroupBy, s);
				setList = appendToTailOfList(setList, newSet);
			}

			//step 3
			//get SUM(A) and append to tail of list setList, then set property
            int size = LIST_LENGTH(agg->aggrs);
            Set *tempSet;
            int i = 0;
            FOREACH(AttributeDef, ad, agg->op.schema->attrDefs)
            {
            	tempSet = MAKE_SET_PTR(copyObject(ad->attrName));
            	setList = appendToTailOfList(setList, copyObject(tempSet));
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
					lSchemaList = appendToTailOfList(lSchemaList, copyObject(a->attrName));

			List *rSchemaList = NIL;
			FOREACH(AttributeDef,a, ((QueryOperator *)(OP_RCHILD(root)))->schema->attrDefs)
				rSchemaList = appendToTailOfList(rSchemaList, copyObject(a->attrName));

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
					    tempSet = intersectSets(copyObject(s1),copyObject(s2));
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
                setList = CombineDuplicateElemSetInECList(setList);
				setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_SET_EC), (Node *)setList);
			}

			if(((SetOperator *)root)->setOpType == SETOP_DIFFERENCE)
			{
				Node *childECP = getProperty(OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_EC));
				List *setList = (List *)childECP;
				setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_SET_EC), (Node *)setList);
			}

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

		ProjectionOperator *pj = (ProjectionOperator *)root;
        List *setList = NIL;

        List *schemaList1 = copyObject(pj->op.schema->attrDefs);
        List *schemaList2 = copyObject(pj->op.schema->attrDefs);
        List *attrRefList1 = copyObject(pj->projExprs);
        List *attrRefList2 = copyObject(pj->projExprs);

        //step 1
        Set *tempSet;
        FORBOTH(Node, a1, s1, attrRefList1, schemaList1)
        {
        	 FORBOTH(Node, a2, s2, attrRefList2, schemaList2)
		     {
        		 FOREACH(Set, s, rList)
		         {
        			 char *d1 = ((AttributeDef *)s1)->attrName;
        			 char *d2 = ((AttributeDef *)s2)->attrName;
        			 if(hasSetElem(s,d1) && hasSetElem(s,d2) && !streq(d1,d2))
        			 {
        				 char *r1 = copyObject(((AttributeReference *)a1)->name);
        				 char *r2 = copyObject(((AttributeReference *)a2)->name);
                         tempSet = MAKE_SET_PTR(r1,r2);
                         setList = appendToTailOfList(setList, tempSet);
                         break;
        			 }
		         }
		     }
        }

        //step 2
        setList = concatTwoLists(setList, rList);
        setList = CombineDuplicateElemSetInECList(setList);
		setProperty((QueryOperator *)(OP_LCHILD(root)), (Node *) createConstString(PROP_STORE_SET_EC ), (Node *)setList);

	}

	//contains join inner and join cross
	else if(isA(root, JoinOperator))
	{
		//Join operator EC
		Node *nRoot = getProperty(root, (Node *) createConstString(PROP_STORE_SET_EC));
		List *rootECSetList = (List *)nRoot;

		//SCH(Left Child)
        boolean flag = FALSE;
		Set *lSchemaSet;
		FOREACH(AttributeDef,a, ((QueryOperator *)(OP_LCHILD(root)))->schema->attrDefs)
		{
			if(flag == FALSE)
			{
				lSchemaSet = MAKE_SET_PTR(copyObject(a->attrName));
				flag = TRUE;
			}
			else
				addToSet(lSchemaSet,copyObject(a->attrName));
		}

		//SCH(Right Child)
		flag = FALSE;
		Set *rSchemaSet;
		FOREACH(AttributeDef,a, ((QueryOperator *)(OP_RCHILD(root)))->schema->attrDefs)
		{
			if(flag == FALSE)
			{
				rSchemaSet = MAKE_SET_PTR(copyObject(a->attrName));
				flag = TRUE;
			}
			else
				addToSet(rSchemaSet,copyObject(a->attrName));
		}

		//get EC(left)
		Set *tempSet;
		List *lSetList = NIL;
        FOREACH(Set, s, rootECSetList)
		{
        	tempSet = setDifference(s, rSchemaSet);
        	lSetList = appendToTailOfList(lSetList, tempSet);
		}
        QueryOperator *lChildOp = OP_LCHILD(root);
		setProperty(lChildOp, (Node *) createConstString(PROP_STORE_SET_EC ), (Node *)lSetList);

        //get EC(right)
		List *rSetList = NIL;
        FOREACH(Set, s, rootECSetList)
		{
        	tempSet = setDifference(s, lSchemaSet);
        	rSetList = appendToTailOfList(rSetList, tempSet);
		}
        QueryOperator *rChildOp = OP_RCHILD(root);
		setProperty(rChildOp, (Node *) createConstString(PROP_STORE_SET_EC ), (Node *)rSetList);

	}

	else if(isA(root, AggregationOperator))
	{
		//get EC(R)
		QueryOperator *childOp = OP_LCHILD(root);
		Node *nChild = getProperty(childOp, (Node *) createConstString(PROP_STORE_SET_EC));
		List *childECSetList = (List *)nChild;

		//get EC(Root)
		Node *nRoot = getProperty(root, (Node *) createConstString(PROP_STORE_SET_EC));
		List *rootECSetList = (List *)nRoot;

		//get SCH(R)
        boolean flag = FALSE;
		Set *childSchemaSet;
		FOREACH(AttributeDef,a, ((QueryOperator *)(OP_LCHILD(root)))->schema->attrDefs)
		{
			if(flag == FALSE)
			{
				childSchemaSet = MAKE_SET_PTR(copyObject(a->attrName));
				flag = TRUE;
			}
			else
				addToSet(childSchemaSet,copyObject(a->attrName));
		}

       //main steps
		List *setList = NIL;
		Set *tempSet;
		FOREACH(Set, s, rootECSetList)
		{
			tempSet = intersectSets(s, childSchemaSet);
            setList = appendToTailOfList(setList, tempSet);
		}

		setList = concatTwoLists(setList, childECSetList);
		setList = CombineDuplicateElemSetInECList(setList);
		setProperty((QueryOperator *)(OP_LCHILD(root)), (Node *) createConstString(PROP_STORE_SET_EC ), (Node *)setList);

	}

	else if(isA(root, DuplicateRemoval))
	{
		Node *rootECP = getProperty(root, (Node *) createConstString(PROP_STORE_SET_EC));
		//List *setList = (List *)childECP;
		setProperty((QueryOperator *)OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_EC), rootECP);
	}

	else if(isA(root,SetOperator))
	{
		//get schema list of left child and right child
		List *lSchemaList = NIL;
		FOREACH(AttributeDef,a, ((QueryOperator *)(OP_LCHILD(root)))->schema->attrDefs)
				lSchemaList = appendToTailOfList(lSchemaList, copyObject(a->attrName));

		List *rSchemaList = NIL;
		FOREACH(AttributeDef,a, ((QueryOperator *)(OP_RCHILD(root)))->schema->attrDefs)
			rSchemaList = appendToTailOfList(rSchemaList, copyObject(a->attrName));

		Node *rootECP = getProperty(root, (Node *) createConstString(PROP_STORE_SET_EC));
		List *rootECSetList = (List *)rootECP;

		if(((SetOperator *)root)->setOpType == SETOP_UNION)
		{
            //set left child's EC
			List *lSetList = concatTwoLists(rootECSetList, lSchemaList);
			lSetList = CombineDuplicateElemSetInECList(lSetList);
			setProperty((QueryOperator *)OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_EC), (Node *)lSetList);

			//SCH(R)/SCH(S)
			List *rootSetList = NIL;
			rootSetList = LSCHtoRSCH(rootSetList,rootECSetList,rSchemaList,lSchemaList);

			//set right childs' EC
			rootSetList = concatTwoLists(rootSetList, rSchemaList);
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

List *
SCHAtoBUsedInProJ(List *setList, List *childECSetList, List *attrA, List *attrB)
{
	List *aNameOpList = NIL;
	FOREACH(Set, s, childECSetList)
	{
		//FOREACH_SET(Node, se, s)
		//{
				FORBOTH_LC(a,b,attrA,attrB)
		        {
					if(isA(LC_P_VAL(a),Operator))
					{
						//attrOpList = {a,b,c}
						aNameOpList = getAttrNameFromOpExpList(aNameOpList,(Operator *)LC_P_VAL(a));
						FOREACH(char, n, aNameOpList)
						{
							//if(!isA(se,Constant))
							//{
								//if(streq(n,(char *)se))
							    if(hasSetElem(s,n))
								{

									removeSetElem(s,n);
									Set *tempSet = MAKE_SET_PTR(copyObject(n));
									setList = appendToTailOfList(setList, tempSet);
									//break;
								}
							//}
						}
					}

					if(isA(a,AttributeReference))
					{
						//if(!isA(se,Constant))
						//{
							//if(streq(((AttributeReference *)a)->name,(char *)se))
							if(hasSetElem(s,((AttributeReference *)a)->name))
						    {
								removeSetElem(s,((AttributeReference *)a)->name);
								addToSet(s,((AttributeDef *)b)->attrName);
                                //se = ((AttributeReference *)b)->name;
                                //break;
							}
						//}
					}
		        }

			if(setSize(s) != 0)
				setList = appendToTailOfList(setList, copyObject(s));
		//}
	}

	return setList;
}

List *
LSCHtoRSCH(List *setList, List *rECSetList, List *lSchemaList, List *rSchemaList){

	Set *set;
	boolean flag;
	//List *setList = NIL;
	FOREACH(Set, s, rECSetList)
	{
		flag = FALSE;
		FOREACH_SET(Node, se, s)
		{
			if(flag == FALSE)
			{
				if(!isA(se,Constant))
				{
					FORBOTH(char,l1,l2, lSchemaList, rSchemaList)
			        {

						if(streq((char *)se,l2))
						{
							set = MAKE_SET_PTR(copyObject(l1));
							flag = TRUE;
							break;
						}
			         }
				}
				else
					set = MAKE_SET_PTR((Constant *)copyObject(se));
			}
			else
			{
				if(!isA(se,Constant))
				{
					FORBOTH(char,l1,l2, lSchemaList, rSchemaList)
			        {

						if(streq((char *)se,l2))
						{
							addToSet(set,copyObject(l1));
							flag = TRUE;
							break;
						}
			        }
				}
				else
					addToSet(set,(Constant *)copyObject(se));
			}

		}

		setList = appendToTailOfList(setList, set);
	}

	return setList;
}

List *
CombineDuplicateElemSetInECList(List *DupECList)
{
	boolean flag;
	List *list1 = copyObject(DupECList);

    //Main loop1
    do{
    	//initial
    	//List *list1 = copyObject(DupECList);
        List *list2 = copyObject(list1);
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
    				if(hasSetElem(s2,n1))
    				{
    					//1, change flag
    					flag = TRUE;
    					//2, union two sets
    					Set *newSet = unionSets(s1, s2);
    					list1 = REMOVE_FROM_LIST_PTR(list1, s2);
    					list1 = replaceNode(list1, s2, newSet);
    					//break the loop redo it by new list (list1)
    					break;
    				}
		    	}

    			if(flag == TRUE)
    				break;
			}

    		if(flag == TRUE)
    			break;
    		else
    			list2 = removeFromHead(list2);
    	}

    }while(flag == TRUE);

	return DupECList;
}


// If cond = (a=5) AND (b=c)
// COndECSetList = {{a,5},{b,c}} which is a List of Sets
// Use the ptr set, a is the pointer point to char a, 5 is the pointer point to a constant structure
List *
GenerateCondECSetListUsedInBottomUp(List *CondECSetList, Operator *op)
{

	if(streq(op->name,"AND"))
	{
		Operator *o2 = (Operator *)(getTailOfListP(op->args));
		CondECSetList = getSelectionCondOperatorList(CondECSetList,o2);

		Operator *o1 = (Operator *)(getHeadOfListP(op->args));
		CondECSetList = getSelectionCondOperatorList(CondECSetList,o1);

	}
	else if(streq(op->name,"="))
	{
		Set *s;
		if(isA(getHeadOfListP(op->args), AttributeReference))
		{
           char *arName = copyObject(((AttributeReference *)(LC_P_VAL(getHeadOfList(op->args))))->name);
           s = MAKE_SET_PTR(arName);
		}
		else if(isA(getHeadOfListP(op->args), Constant))
		{
		   Constant *c =  copyObject((Constant *)LC_P_VAL(getHeadOfList(op->args)));
		   s = MAKE_SET_PTR(c);
		}
		else
		{
			//TODO: return error
		}

		if(isA(LC_P_VAL(getTailOfList(op->args)), AttributeReference))
		{
           char *arName = copyObject(((AttributeReference *)(LC_P_VAL(getHeadOfList(op->args))))->name);
           addToSet(s,arName);
		}
		else if(isA(LC_P_VAL(getTailOfList(op->args)), Constant))
		{
		   Constant *c =  copyObject((Constant *)LC_P_VAL(getHeadOfList(op->args)));
		   addToSet(s,c);
		}
		else
		{
			//TODO: return error
		}

		CondECSetList = appendToTailOfList(CondECSetList, s);
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


void initializeIColProp(QueryOperator *root, Set *seticols)
{
	setStringProperty((QueryOperator *)root, PROP_STORE_SET_ICOLS, (Node *)seticols);

	FOREACH(QueryOperator, o, root->inputs)
	{
		initializeIColProp(o, seticols);
	}
}

Set *
AddAttrOfSelectCondToSet(Set *set, Operator *op)
{
   //Operator *CondOp = (Operator *)(op->name);
   if(op->name != NULL)
   {
	   //Left
       if(isA(getHeadOfList(op->args),Operator))
       {
    	   set = AddAttrOfSelectCondToSet(set, (Operator *)getHeadOfList(op->args));
       }
       else if(isA(getHeadOfList(op->args),AttributeReference))
       {
    	   char * attrName = ((AttributeReference *)getHeadOfList(op->args))->name;
    	   //TODO: add attrName to set
    	   addToSet(set, attrName);
       }
//       else if(isA(getHeadOfList(op->args),Constant))
//       {
//    	   //TODO: add attrName to set
//    	   addToSet(set, (char *)getHeadOfList(op->args));
//       }

       //Right
       if(isA(getTailOfList(op->args),Operator))
       {
    	   set = AddAttrOfSelectCondToSet(set, (Operator *)getTailOfList(op->args));
       }
       else if(isA(getTailOfList(op->args),AttributeReference))
       {
    	   char * attrName = ((AttributeReference *)getTailOfList(op->args))->name;
    	   //TODO: add attrName to set
    	   addToSet(set, attrName);
       }
//       else if(isA(getTailOfList(op->args),Constant))
//       {
//    	   //TODO: add attrName to set
//    	   addToSet(set, (char *)getTailOfList(op->args));
//       }
   }

   return set;
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
	if (isA(root, SelectionOperator))
	{
		Set *rooticols = (Set*)getProperty(root, (Node *) createConstString(PROP_STORE_SET_ICOLS));
		Set *childicols = (Set*)getProperty(OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_ICOLS));

		// TODO Get the conditions of Selection Operator and add it to Set
		Operator *condOp = (Operator *)(((SelectionOperator *)root)->cond);

		//TODO Initialize Set without having some value
		Set *condicols = MAKE_STR_SET(strdup("EMPTY"));

		condicols = AddAttrOfSelectCondToSet(condicols,condOp);

		// TODO Union all three sets and set it as its child icols property
		Set *eicols;
		eicols = unionSets(rooticols, childicols);
		eicols = unionSets(eicols, condicols);
		setStringProperty((QueryOperator *) OP_LCHILD(root), PROP_STORE_SET_ICOLS, (Node *)eicols);
	}

	if(isA(root, ProjectionOperator))
	{
		Set *rooticols = (Set*)getProperty(root, (Node *) createConstString(PROP_STORE_SET_ICOLS));
		Set *childicols = (Set*)getProperty(OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_ICOLS));

		//TODO Get Reference Attribute Names and put it into a set
		List *opAttrNames = getAttrRefNames((ProjectionOperator *)root);
		Set *setopAttrNames = makeStrSetFromList(opAttrNames);

		//TODO if Reference Attribute name belongs to the Intersection of the above set with rooticols, then we add the corresponding schema attribute's name to a set.
		Set *s1 = intersectSets(rooticols, setopAttrNames);

		List *l1 = getQueryOperatorAttrNames(root);

		//TODO Initialize Set without any argument
		Set *s2 = MAKE_STR_SET(strdup("EMPTY"));

		FORBOTH(char, a1, b1, opAttrNames, l1)
		{
			if(hasSetElem(s1, a1))
			{
				addToSet(s2, b1);
			}
		}
		//TODO Union this set with childicols and set it as its child icols property
		Set *s3 = unionSets(childicols, s2);
		setStringProperty((QueryOperator *) OP_LCHILD(root), PROP_STORE_SET_ICOLS, (Node *)s3);
	}

	if(isA(root, DuplicateRemoval))
	{
		Set *rooticols = (Set*)getProperty(root, (Node *) createConstString(PROP_STORE_SET_ICOLS));
		Set *childicols = (Set*)getProperty(OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_ICOLS));

		//Union both sets and set it as its child icols property
		Set *eicols;
		eicols = unionSets(rooticols, childicols);
		setStringProperty((QueryOperator *) OP_LCHILD(root), PROP_STORE_SET_ICOLS, (Node *)eicols);

	}

	if (isA(root, JoinOperator))
	{
		if(((JoinOperator*)root)->joinType == JOIN_CROSS)
		{
			Set *rooticols = (Set*)getProperty(root, (Node *) createConstString(PROP_STORE_SET_ICOLS));
			Set *leftchildicols = (Set*)getProperty(OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_ICOLS));
			Set *rightchildicols = (Set*)getProperty(OP_RCHILD(root), (Node *) createConstString(PROP_STORE_SET_ICOLS));

			List *l1 = getQueryOperatorAttrNames(OP_LCHILD(root));
			List *l2 = getQueryOperatorAttrNames(OP_RCHILD(root));

			Set *s1 = makeStrSetFromList(l1);
			Set *s2 = makeStrSetFromList(l2);

			// Set icols for left child
			Set *s3 = intersectSets(rooticols, s1);
			Set *s4 = unionSets(s3, leftchildicols);
			setStringProperty((QueryOperator *) OP_LCHILD(root), PROP_STORE_SET_ICOLS, (Node *)s4);

			// Set icols for right child
			Set *s5 = intersectSets(rooticols, s2);
			Set *s6 = unionSets(s5, rightchildicols);
			setStringProperty((QueryOperator *) OP_RCHILD(root), PROP_STORE_SET_ICOLS, (Node *)s6);
		}

		if (((JoinOperator*)root)->joinType == JOIN_INNER)
		{
			Set *rooticols = (Set*)getProperty(root, (Node *) createConstString(PROP_STORE_SET_ICOLS));
			Set *leftchildicols = (Set*)getProperty(OP_LCHILD(root), (Node *) createConstString(PROP_STORE_SET_ICOLS));
			Set *rightchildicols = (Set*)getProperty(OP_RCHILD(root), (Node *) createConstString(PROP_STORE_SET_ICOLS));

			Operator *condOp = (Operator *)(((JoinOperator *)root)->cond);
			//TODO Initialize Set without having some value
			Set *condicols = MAKE_STR_SET(strdup("EMPTY"));
			condicols = AddAttrOfSelectCondToSet(condicols,condOp);

			List *l1 = getQueryOperatorAttrNames(OP_LCHILD(root));
			List *l2 = getQueryOperatorAttrNames(OP_RCHILD(root));

			Set *s1 = makeStrSetFromList(l1);
			Set *s2 = makeStrSetFromList(l2);

			// Set icols for left child
			Set *s3 = unionSets(rooticols, condicols);
			Set *s4 = intersectSets(s3, s1);
			Set *s5 = unionSets(s4, leftchildicols);
			setStringProperty((QueryOperator *) OP_LCHILD(root), PROP_STORE_SET_ICOLS, (Node *)s5);

			// Set icols for right child
			Set *s7 = intersectSets(s3, s2);
			Set *s8 = unionSets(s7, rightchildicols);
			setStringProperty((QueryOperator *) OP_RCHILD(root), PROP_STORE_SET_ICOLS, (Node *)s8);
		}
	}

	FOREACH(QueryOperator, o, root->inputs)
	{
		computeReqColProp(o);
	}
}

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
       else if(isA(getHeadOfList(op->args),Constant))
       {
    	   //TODO: add attrName to set
    	   addToSet(set, (char *)getHeadOfList(op->args));
       }

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
       else if(isA(getTailOfList(op->args),Constant))
       {
    	   //TODO: add attrName to set
    	   addToSet(set, (char *)getTailOfList(op->args));
       }
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

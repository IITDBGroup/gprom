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

#include "operator_optimizer/optimizer_prop_inference.h"
#include "metadata_lookup/metadata_lookup_oracle.h"
#include "include/log/logger.h"
#include "include/model/query_operator/operator_property.h"

static List *keyList = NIL;

void
computeKeyProp (QueryOperator *root)
{
    if(root->inputs != NULL)
    {
        FOREACH(QueryOperator, op, root->inputs)
            computeKeyProp(op);
    }

    if (root != NULL)
    {
        if(isA(root, TableAccessOperator))
        {
            keyList = getKeyInformation(root);
            setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_LIST_KEY), (Node *)keyList);
        }

        if (isA(root, SelectionOperator))
        {
            setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_LIST_KEY), (Node *)keyList);
        }

        if (isA(root, ProjectionOperator))
        {
            List *l1 = ((ProjectionOperator *)root)->projExprs;
            List *l2 = NIL;

            FOREACH(AttributeReference, op1, l1)
            {
                l2 = appendToTailOfList(l2, op1->name);
            }

            FOREACH(char, op, keyList)
            {
                if(!searchListString(l2, op))
                {
                    setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_LIST_KEY), NULL);
                    break;
                }

                setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_LIST_KEY), (Node *)keyList);
            }
        }

        if (isA(root, DuplicateRemoval))
        {
            setProperty((QueryOperator *)root, (Node *) createConstString(PROP_STORE_LIST_KEY), (Node *)keyList);
        }
    }
}

void
computeECProp (QueryOperator *root)
{

}

void
computeSetProp (QueryOperator *root)
{

}

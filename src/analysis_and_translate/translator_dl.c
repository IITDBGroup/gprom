/*-----------------------------------------------------------------------------
 *
 * translator_dl.c
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
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/datalog/datalog_model.h"
#include "model/set/hashmap.h"

Node *
translateParseDL(Node *q)
{
    HashMap *idbRels;   // keep track of partial algebra expressions for idb rules

    return NULL;
}

QueryOperator *
translateQueryDL(Node *node)
{
    return NULL;
}

/*-----------------------------------------------------------------------------
 *
 * prov_schema.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"
#include "provenance_rewriter/prov_schema.h"

#include "uthash.h"

/* consts */
#define PROV_ATTR_PREFIX "prov_"

/* data types */
typedef struct RelCount {
    char *relName;
    int count;
    UT_hash_handle hh;
} RelCount;

typedef struct ProvSchemaInfo
{
    List *provAttrs;
    RelCount *rels;
} ProvSchemaInfo;

/* function declarations */
static boolean findBaserelationsVisitor (Node *node, ProvSchemaInfo *status);
#define CONCAT(...) concat(__VA_ARGS__, NULL)
static char *concat(char *first, ...);
static int getRelCount(ProvSchemaInfo *info, char *tableName);


/* definitions */
List *
getProvenanceAttributes(QueryOperator *q, ProvenanceType type)
{
    switch(type) {
        case PI_CS:
        {
            ProvSchemaInfo *pSchema= NEW(ProvSchemaInfo);

            findBaserelationsVisitor((Node *) q, pSchema);
            return pSchema->provAttrs;
        }
        case TRANSFORMATION:
            return singleton(strdup("tprov"));
    }
}

char *
getProvenanceAttrName (char *table, char *attr, int count)
{
    char *countStr = NEW(128);
    return CONCAT(PROV_ATTR_PREFIX, table, "_", attr, sprintf(countStr,"%u", count));
}

static char *
concat(char *first, ...)
{
    va_list args;
    StringInfo str = makeStringInfo();
    char *string;

    va_start(args, first);
    while((string = va_arg(args, char*)) != NULL)
        appendStringInfoString(str, string);
    va_end(args);

    return str->data;
}

static boolean
findBaserelationsVisitor (Node *node, ProvSchemaInfo *status)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, TableAccessOperator))
    {
        TableAccessOperator *t = (TableAccessOperator *) node;

        FOREACH(AttributeDef,a,t->op.schema->attrDefs)
            status->provAttrs = appendToTailOfList(status->provAttrs,
                    getProvenanceAttrName(t->tableName,a->attrName,
                            getRelCount(status, t->tableName)));
    }

    return visit(node, findBaserelationsVisitor, status);
}

static int
getRelCount(ProvSchemaInfo *info, char *tableName)
{
    RelCount *relCount;

    HASH_FIND_STR(info->rels,tableName,relCount);
    if (relCount == NULL)
    {
        relCount = NEW(RelCount);
        relCount->count = 0;
        relCount->relName = strdup(tableName);
        HASH_ADD_STR(info->rels,relName,relCount);
    }
    else
    {
        relCount->count++;
    }

    return relCount->count;
}

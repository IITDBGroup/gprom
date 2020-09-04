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

#include "log/logger.h"

#include "uthash.h"

#include "mem_manager/mem_mgr.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"
#include "model/query_block/query_block.h"
#include "provenance_rewriter/prov_schema.h"
#include "provenance_rewriter/semiring_combiner/sc_main.h"
#include "provenance_rewriter/uncertainty_rewrites/uncert_rewriter.h"

/* consts */
#define PROV_ATTR_PREFIX "PROV_"

/* data types */
typedef struct ProvSchemaInfo
{
    List *provAttrs;
    List *dts;
    RelCount *rels;
} ProvSchemaInfo;

/* function declarations */
static boolean findBaserelationsVisitor (Node *node, ProvSchemaInfo *status);
static int getRelCount(ProvSchemaInfo *info, char *tableName);
static boolean findTablerefVisitor (Node *node, ProvSchemaInfo *status);
static boolean findTablerefVisitorForCoarse (Node *node, ProvSchemaInfo *status);
static char *escapeUnderscore (char *str);

/* definitions */
List *
getProvenanceAttributes(QueryOperator *q, ProvenanceType type)
{
    switch(type) {
        case PROV_PI_CS:
        {
            ProvSchemaInfo *pSchema= NEW(ProvSchemaInfo);

            findBaserelationsVisitor((Node *) q, pSchema);
            return pSchema->provAttrs;
        }
        case PROV_TRANSFORMATION:
            return singleton(strdup("tprov"));
        case PROV_XML:
            return singleton(strdup("xmlprov"));
        case PROV_NONE:
        {
            return NIL;
        }
        case CAP_USE_PROV_COARSE_GRAINED:
        {
            return NIL;
        }
        case PROV_COARSE_GRAINED:
        {
            return NIL;
        }
        case USE_PROV_COARSE_GRAINED:
        {
            return NIL;
        }
        case USE_PROV_COARSE_GRAINED_BIND:
        {
            return NIL;
        }
    }
    return NIL; //keep compiler quiet
}

List *
getProvenanceAttrNames (char *table, List *attrs, int count)
{
     List *result = NIL;

     FOREACH(char,a,attrs)
         result = appendToTailOfList(result, getProvenanceAttrName(table, a, count));

     return result;
}

char *
getProvenanceAttrName (char *table, char *attr, int count)
{
    char *countStr = CALLOC(1,128);
    if (count > 0)
        sprintf(countStr,"_%u", count);
    return CONCAT_STRINGS(PROV_ATTR_PREFIX, escapeUnderscore(table), countStr, "_",
            escapeUnderscore(attr));
}

static char *
escapeUnderscore (char *str)
{
    int len = strlen(str);
    int newLen = len;
    char *result;

    for(char *s = str; *s != '\0'; s++, newLen = newLen + (*s == '_' ? 1 : 0));

    result = (char *) MALLOC(newLen + 1);

    for(int i = 0, j = 0; i <= len; i++, j++)
    {
        if (str[i] == '_')
        {
            result[j++] = '_';
            result[j] = '_';
        }
        else
            result[j] = str[i];
    }

    return result;
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
    return getRelNameCount(&(info->rels), tableName);
//
//
//
//    HASH_FIND_STR(info->rels,tableName,relCount);
//    if (relCount == NULL)
//    {
//        relCount = NEW(RelCount);
//        relCount->count = 0;
//        relCount->relName = strdup(tableName);
//        HASH_ADD_STR(info->rels,relName,relCount);
//    }
//    else
//        relCount->count++;
//
//    return relCount->count;
}

int
getCurRelNameCount(RelCount **relCount, char *tableName)
{
    RelCount *relC = NULL;

    HASH_FIND_STR((*relCount), tableName, relC);
    if (relC == NULL)
    {
        relC = NEW(RelCount);
        relC->count = 0;
        relC->relName = strdup(tableName);
        HASH_ADD_KEYPTR(hh, (*relCount), relC->relName, strlen(relC->relName),
                relC);
    }

    return relC->count;
}

int
getRelNameCount(RelCount **relCount, char *tableName)
{
    RelCount *relC = NULL;

    HASH_FIND_STR((*relCount), tableName, relC);
    if (relC == NULL)
    {
        relC = NEW(RelCount);
        relC->count = 0;
        relC->relName = strdup(tableName);
        HASH_ADD_KEYPTR(hh, (*relCount), relC->relName, strlen(relC->relName),
                relC);
    }
    else
        relC->count++;

    return relC->count;
}

void
getQBProvenanceAttrList (ProvenanceStmt *stmt, List **attrNames, List **dts)
{
    if(stmt->provType == PROV_PI_CS && stmt->inputType == PROV_INPUT_QUERY)
    {
        ProvSchemaInfo *pSchema= NEW(ProvSchemaInfo);

        pSchema->provAttrs = NIL;
        pSchema->dts = NIL;
        findTablerefVisitor((Node *) stmt->query, pSchema);

        if (LIST_LENGTH(pSchema->dts) == 0)
        {
            THROW(SEVERITY_RECOVERABLE, "%s", "cannot apply semiring combiner to query that has no provenance attributes.");
        }

        //semiring combiner check
        if(isSemiringCombinerActivatedPs(stmt)){
            *attrNames = singleton("PROV");
            *dts = singletonInt(getSemiringCombinerDatatype(stmt,pSchema->dts));
            return;
        }

        *attrNames = pSchema->provAttrs;
        *dts = pSchema->dts;

        return;
    }
    if (stmt->provType == PROV_COARSE_GRAINED
    			|| stmt->provType == USE_PROV_COARSE_GRAINED
			|| stmt->provType == CAP_USE_PROV_COARSE_GRAINED
			|| stmt->provType == USE_PROV_COARSE_GRAINED_BIND)
    {
        //TODO create list of prov attributes PROV_R, PROV_S, .... and their DTs
        ProvSchemaInfo *pSchema= NEW(ProvSchemaInfo);

        pSchema->provAttrs = NIL;
        pSchema->dts = NIL;
        findTablerefVisitorForCoarse((Node *) stmt->query, pSchema);

        //semiring combiner check
        if(isSemiringCombinerActivatedPs(stmt)){
            *attrNames = singleton("PROV");
            *dts = singletonInt(getSemiringCombinerDatatype(stmt,pSchema->dts));
            return;
        }

        *attrNames = pSchema->provAttrs;
        *dts = pSchema->dts;


        return;
    }
    if (stmt->provType == PROV_XML)
    {
        *attrNames = appendToTailOfList(*attrNames, "PROV");
        *dts = appendToTailOfListInt(*dts, DT_STRING);

        return;
    }
    if (stmt->inputType == PROV_INPUT_UNCERTAIN_QUERY)
    {
        List *qAttrName =  getQBAttrNames(stmt->query);

        // add attribute uncertainty attributes
        FOREACH(char,n,qAttrName)
        {
            char *uName = getUncertString(n);
            *dts = appendToTailOfListInt(*dts, DT_INT);
            *attrNames = appendToTailOfList(*attrNames, strdup(uName));
        }

        // add row uncertainty attribute
        *dts = appendToTailOfListInt(*dts, DT_INT);
        *attrNames = appendToTailOfList(*attrNames, getUncertString(UNCERTAIN_ROW_ATTR));
    }
}

static boolean
findTablerefVisitor (Node *node, ProvSchemaInfo *status)
{
    if (node == NULL)
        return TRUE;

    if (isFromItem(node))
    {
        FromItem *f = (FromItem *) node;

        // if user specified provenance attribute, then use them
        if (f->provInfo)
        {
            char *tableName = f->name;
            int curRelCount = getRelCount(status, tableName);

            // is base relation provenance or USE PROVENANCE
            if (f->provInfo->baserel && LIST_EMPTY(f->provInfo->userProvAttrs))
            {
                FOREACH(char,attrName,f->attrNames)
                {
                    status->provAttrs = appendToTailOfList(status->provAttrs,
                            getProvenanceAttrName(tableName, attrName,
                                    curRelCount));
                }
                FOREACH_INT(dt,f->dataTypes)
                    status->dts = appendToTailOfListInt(status->dts, dt);
            }
            // show intermediate provenacne
            else if (f->provInfo->intermediateProv)
            {
                // first get child provenance attributes
                boolean result = visit(node, findTablerefVisitor, status);
                //TODO is that correct?
                FOREACH(char,attrName,f->attrNames)
                {
                    status->provAttrs = appendToTailOfList(status->provAttrs,
                            getProvenanceAttrName(tableName, attrName,
                                    curRelCount));
                }
                FOREACH_INT(dt,f->dataTypes)
                    status->dts = appendToTailOfListInt(status->dts, dt);

                return result;
            }
            // user provided provenance attributes
            else
            {
                FOREACH(char,attrName,f->provInfo->userProvAttrs)
                {
                    DataType dt;

                    status->provAttrs = appendToTailOfList(status->provAttrs,
                            getProvenanceAttrName(tableName, attrName,
                                    curRelCount));
                    dt = getNthOfListInt(f->dataTypes, listPosString(f->attrNames, attrName));
                    status->dts = appendToTailOfListInt(status->dts, dt);
                }
            }
            return TRUE;
        }
        else if (isA(node, FromTableRef))
        {
            FromTableRef *r = (FromTableRef *) node;
            char *tableName = r->tableId;
            int curRelCount = getRelCount(status, tableName);

            FOREACH(char,a,r->from.attrNames)
                status->provAttrs = appendToTailOfList(status->provAttrs,
                        getProvenanceAttrName(tableName,a, curRelCount));
            FOREACH_INT(dt,r->from.dataTypes)
                status->dts = appendToTailOfListInt(status->dts, dt);
        }
    }

    return visit(node, findTablerefVisitor, status);
}


static boolean
findTablerefVisitorForCoarse (Node *node, ProvSchemaInfo *status)
{
    if (node == NULL)
        return TRUE;

    if (isFromItem(node))
    {
        if (isA(node, FromTableRef))
        {
            FromTableRef *r = (FromTableRef *) node;
            char *tableName = r->tableId;

            status->provAttrs = appendToTailOfList(status->provAttrs,
                		 CONCAT_STRINGS("PROV_", strdup(tableName)));

            status->dts = appendToTailOfListInt(status->dts, DT_STRING);
        }
    }

    return visit(node, findTablerefVisitorForCoarse, status);
}

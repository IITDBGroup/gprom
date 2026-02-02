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

#include "model/expression/expression.h"
#include "model/set/hashmap.h"
#include "uthash.h"

#include "mem_manager/mem_mgr.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/query_block/query_block.h"
#include "provenance_rewriter/prov_schema.h"
#include "provenance_rewriter/semiring_combiner/sc_main.h"
#include "provenance_rewriter/uncertainty_rewrites/uncert_rewriter.h"

/* data types */
typedef struct ProvSchemaInfo
{
    List *provAttrs;
    List *dts;
    HashMap *rels;
	HashMap *views;
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

			pSchema->rels = NEW_MAP(Constant,Constant);

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
getProvenanceAttrNames(char *table, List *attrs, int count)
{
     List *result = NIL;

     FOREACH(char,a,attrs)
         result = appendToTailOfList(result, getProvenanceAttrName(table, a, count));

     return result;
}

char *
getProvenanceAttrName(char *table, char *attr, int count)
{
    char *countStr = CALLOC(1,128);
    if (count > 0)
        sprintf(countStr,"_%u", count);
    return CONCAT_STRINGS(PROV_ATTR_PREFIX, escapeUnderscore(table), countStr, "_",
            escapeUnderscore(attr));
}

List *
getCoarseGrainedAttrNames(char *table, List *attrs, int count)
{
     List *result = NIL;

     FOREACH(char,a,attrs)
         result = appendToTailOfList(result, getCoarseGrainedAttrName(table, a, count));

     return result;
}

char *
getCoarseGrainedAttrName(char *table, char *attr, int count)
{
    return CONCAT_STRINGS(PROV_ATTR_PREFIX, strdup(table), "_", strdup(attr), gprom_itoa(count));
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
    return increaseRefCount(info->rels, tableName);
}

int
getCurRelNameCount(HashMap *relCount, char *tableName)
{
	if (!MAP_HAS_STRING_KEY(relCount, tableName))
	{
		MAP_ADD_STRING_KEY(relCount, tableName, createConstInt(0));
	}

    return INT_VALUE(MAP_GET_STRING(relCount, tableName));
}

int
increaseRefCount(HashMap *provCounts, char *prefix)
{
	int cnt;

	if (MAP_HAS_STRING_KEY(provCounts, prefix))
	{
		Constant *cntC = (Constant *) MAP_GET_STRING(provCounts, prefix);
		INT_VALUE(cntC) = INT_VALUE(cntC) + 1;
	}
	else {
		MAP_ADD_STRING_KEY(provCounts, prefix, createConstInt(0));
	}

	cnt = INT_VALUE(MAP_GET_STRING(provCounts, prefix));

	DEBUG_LOG("count for <%s> is <%u>", prefix, cnt);

	return cnt;
}

List *
opGetProvAttrInfo(QueryOperator *op)
{
	return (List *) getStringProperty(op, PROP_PROVENANCE_TABLE_ATTRS);
}

void
copyProvInfo(QueryOperator *to, QueryOperator *from)
{
	SET_STRING_PROP(
		to,
		PROP_PROVENANCE_TABLE_ATTRS,
		GET_STRING_PROP(from, PROP_PROVENANCE_TABLE_ATTRS));
}

void
getQBProvenanceAttrList (ProvenanceStmt *stmt, List **attrNames, List **dts)
{
    if(stmt->provType == PROV_PI_CS && stmt->inputType == PROV_INPUT_QUERY)
    {
        ProvSchemaInfo *pSchema= NEW(ProvSchemaInfo);

        pSchema->provAttrs = NIL;
        pSchema->dts = NIL;
		pSchema->views = NEW_MAP(Constant,Node);
		pSchema->rels = NEW_MAP(Constant,Constant);

        findTablerefVisitor((Node *) stmt->query, pSchema);
        /*
         * if stmt->query is WithStmt
         * WithStmt *ws = (WithStmt *)(stmt->query);
         * findTablerefVisitor((Node *) ws->query, pSchema);
         * fixed in findTablerefVisitor
         */


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
		pSchema->views = NEW_MAP(Constant,Node);
		pSchema->rels = NEW_MAP(Constant,Constant);
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
        List *qAttrName = getQBAttrNames(stmt->query);

        // add attribute uncertainty attributes
        FOREACH(char,n,qAttrName)
        {
            char *uName = backendifyIdentifier(getUncertString(n));
            *dts = appendToTailOfListInt(*dts, DT_INT);
            *attrNames = appendToTailOfList(*attrNames, strdup(uName));
        }

        // add row uncertainty attribute
        *dts = appendToTailOfListInt(*dts, DT_INT);
        *attrNames = appendToTailOfList(*attrNames, getUncertString(UNCERTAIN_ROW_ATTR));
    }
	if (stmt->inputType == PROV_INPUT_UNCERTAIN_TUPLE_QUERY)
	{
        // add row uncertainty attribute
		List *qAttrDef =  getQBAttrDefs(stmt->query);
		AttributeDef *nd = (AttributeDef *)getTailOfListP(qAttrDef);
        *dts = appendToTailOfListInt(*dts, nd->dataType);
        *attrNames = appendToTailOfList(*attrNames, getUncertString(UNCERTAIN_ROW_ATTR));
	}
    if (stmt->inputType == PROV_INPUT_RANGE_QUERY)
    {
    	List *qAttrDef =  getQBAttrDefs(stmt->query);
    	INFO_LOG("=======================%s", stringListToString(*attrNames));
    	// add attribute range attributes
    	FOREACH(Node,n,qAttrDef)
    	{
            char *ubName = backendifyIdentifier(getUBString(((AttributeDef *)n)->attrName));
            char *lbName = backendifyIdentifier(getLBString(((AttributeDef *)n)->attrName));
            *dts = appendToTailOfListInt(*dts, ((AttributeDef *)n)->dataType);
            *dts = appendToTailOfListInt(*dts, ((AttributeDef *)n)->dataType);
            *attrNames = appendToTailOfList(*attrNames, strdup(ubName));
            *attrNames = appendToTailOfList(*attrNames, strdup(lbName));
        }

            // add row Range attribute
        *dts = appendToTailOfListInt(*dts, DT_INT);
        *dts = appendToTailOfListInt(*dts, DT_INT);
        *dts = appendToTailOfListInt(*dts, DT_INT);
        *attrNames = appendToTailOfList(*attrNames, backendifyIdentifier(ROW_CERTAIN));
        *attrNames = appendToTailOfList(*attrNames, backendifyIdentifier(ROW_BESTGUESS));
        *attrNames = appendToTailOfList(*attrNames, backendifyIdentifier(ROW_POSSIBLE));
    }
}

static boolean
findTablerefVisitor(Node *node, ProvSchemaInfo *status)
{
    if (node == NULL)
        return TRUE;


    if(isA(node,WithStmt))
    {
    	WithStmt *ws = (WithStmt *) node;
		HashMap *views = status->views;

		// need to first map table references refering to CTEs to queries
		FOREACH(KeyValue,v,ws->withViews)
		{
			addToMap(views,v->key,v->value);
		}

		status->views = views;

		// just pass on the CTE and search in the main query
		return findTablerefVisitor(ws->query, status);
    }

    if (isFromItem(node))
    {
        FromItem *f = (FromItem *) node;

        // if user specified provenance attribute, then use them
        if (f->provInfo)
        {
            char *tableName = f->name;
            int curRelCount = getRelCount(status, tableName);

            // is base relation provenance or USE PROVENANCE
            if (f->provInfo->baserel && MY_LIST_EMPTY(f->provInfo->userProvAttrs))
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

			// if the table is a CTE, search for table references in the view defintion
			if (MAP_HAS_STRING_KEY(status->views, tableName))
			{
				Node *viewDef = MAP_GET_STRING(status->views, tableName);

				// find table references in the view
				visit(viewDef, findTablerefVisitor, status);
			}
			// table is not a view, generate its provenance attributes
			else
			{
				int curRelCount = getRelCount(status, tableName);

				FOREACH(char,a,r->from.attrNames)
					status->provAttrs = appendToTailOfList(status->provAttrs,
														   getProvenanceAttrName(tableName,a, curRelCount));
				FOREACH_INT(dt,r->from.dataTypes)
					status->dts = appendToTailOfListInt(status->dts, dt);
			}
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
                		 CONCAT_STRINGS(PROV_ATTR_PREFIX, strdup(tableName)));

            status->dts = appendToTailOfListInt(status->dts, DT_STRING);
        }
    }

    return visit(node, findTablerefVisitorForCoarse, status);
}

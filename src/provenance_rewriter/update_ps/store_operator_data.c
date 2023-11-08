/*
 * Header files
 */
#include "common.h"
#include "configuration/option.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"

#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "model/bitset/bitset.h"
#include "model/set/vector.h"

#include "provenance_rewriter/pi_cs_rewrites/pi_cs_main.h"
#include "provenance_rewriter/update_ps/update_ps_main.h"
#include "provenance_rewriter/prov_rewriter.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/coarse_grained/coarse_grained_rewrite.h"
#include "provenance_rewriter/uncertainty_rewrites/uncert_rewriter.h"

#include "sql_serializer/sql_serializer.h"
#include "metadata_lookup/metadata_lookup.h"
#include "metadata_lookup/metadata_lookup_postgres.h"
#include "instrumentation/timing_instrumentation.h"
#include "provenance_rewriter/update_ps/table_compress.h"
#include "provenance_rewriter/update_ps/update_ps_incremental.h"
#include "provenance_rewriter/update_ps/update_ps_build_state.h"
#include "provenance_rewriter/update_ps/rbtree.h"
#include "operator_optimizer/optimizer_prop_inference.h"
#include "operator_optimizer/operator_optimizer.h"

#include "provenance_rewriter/update_ps/bloom.h"
#include "provenance_rewriter/update_ps/store_operator_data.h"

static void storeOperatorDataInternal(QueryOperator *op);
static void storeAggregationData(AggregationOperator *op);
static void storeProvenanceComputationData(ProvenanceComputation *op);
static void storeDuplicateRemovalData(DuplicateRemoval *op);
static void storePSMapData(PSMap *psMap, int opNum);
static void storeGBACSsData(GBACSs *acs, int opNum, char *acsName);
static void storeJoinOperatorData(JoinOperator *op);
static void storeBloom(HashMap *blooms, int opNum, char* branch);
static void storeBloomInfo(HashMap *leftInfos, HashMap *rightInfos, int opNum);

boolean
checkQueryInfoStored(char *queryName)
{
	boolean isMetaTableExisted = postgresCatalogTableExists(QUERY_LIST_META);
	if (isMetaTableExisted) {
		StringInfo str = makeStringInfo();
        appendStringInfo(str, "SELECT %s FROM %s WHERE %s = '%s';", QUERY_HAS_STATE, QUERY_LIST_META, QUERY_NAME, queryName);
		Relation *rel = postgresExecuteQuery(str->data);
        freeStringInfo(str);
		int nTups = rel->tuples->length;
		if (nTups < 1) {
			return FALSE;
		}

		int val = atoi((char *) getVecString((Vector *) getVecNode(rel->tuples, 0), 0));
		if (val == 0) {
			return FALSE;
		}
		return TRUE;
	}

	// table not exists create;
    StringInfo str = makeStringInfo();
    appendStringInfo(str, "CREATE TABLE %s (%s varchar, %s int);", QUERY_LIST_META, QUERY_NAME, QUERY_HAS_STATE);
	postgresExecuteStatement(str->data);
	return FALSE;
}
void
setInfoStored(char *queryName)
{
    StringInfo str = makeStringInfo();
    appendStringInfo(str, "INSERT INTO %s VALUES('%s', 1); ", QUERY_LIST_META, queryName);
    postgresExecuteStatement(str->data);
    freeStringInfo(str);

}

void
storeOperatorData(QueryOperator *op)
{
    storeOperatorDataInternal(op);
}

static void
storeOperatorDataInternal(QueryOperator *op)
{
    if (isA(op, TableAccessOperator)) {
        return;
    }

    if (isA(op, AggregationOperator)) {
        storeAggregationData((AggregationOperator *) op);
    } else if (isA(op, ProvenanceComputation)) {
        storeProvenanceComputationData((ProvenanceComputation *) op);
    } else if (isA(op, DuplicateRemoval)) {
        storeDuplicateRemovalData((DuplicateRemoval *) op);
    } else if (isA(op, JoinOperator)) {
        boolean usingBF = getBoolOption(OPTION_UPDATE_PS_JOIN_USING_BF);
        if (usingBF) {
            storeJoinOperatorData((JoinOperator *) op);
        }
    }


    FOREACH(QueryOperator, q, op->inputs) {
        storeOperatorDataInternal(q);
    }
}

static void
storeJoinOperatorData(JoinOperator *op) {
    HashMap *joinState = (HashMap *) getStringProperty((QueryOperator *) op, PROP_DATA_STRUCTURE_JOIN);
    // DEBUG_NODE_BEATIFY_LOG("join state for store", joinState);
    HashMap *leftBloom = (HashMap *) MAP_GET_STRING(joinState, JOIN_LEFT_BLOOM);
    HashMap *leftBloomMapping = (HashMap *) MAP_GET_STRING(joinState, JOIN_LEFT_BLOOM_ATT_MAPPING);
    HashMap *rightBloom = (HashMap *) MAP_GET_STRING(joinState, JOIN_RIGHT_BLOOM);
    HashMap *rightBloomMapping = (HashMap *) MAP_GET_STRING(joinState, JOIN_RIGHT_BLOOM_ATT_MAPPING);

    // DEBUG_NODE_BEATIFY_LOG("left bloom", leftBloom);
    // DEBUG_NODE_BEATIFY_LOG("left mapping", leftBloomMapping);
    // DEBUG_NODE_BEATIFY_LOG("right bloom", rightBloom);
    // DEBUG_NODE_BEATIFY_LOG("right mapping", rightBloomMapping);
    int opNum = INT_VALUE((Constant *) GET_STRING_PROP((QueryOperator *) op, PROP_OPERATOR_NUMBER));
    storeBloom(leftBloom, opNum, "left");
    storeBloom(rightBloom, opNum, "right");
    storeBloomInfo(leftBloomMapping, rightBloomMapping, opNum);
}

static void
storeBloom(HashMap *blooms, int opNum, char *branch)
{
    char *qName = getStringOption(OPTION_UPDATE_PS_QUERY_NAME);
    FOREACH_HASH_KEY(Constant, c, blooms) {
        StringInfo name = makeStringInfo();
        appendStringInfo(name, "%s_%s_%s_%s", qName, gprom_itoa(opNum), branch, STRING_VALUE(c));
        Bloom *bloom = (Bloom *) MAP_GET_STRING(blooms, STRING_VALUE(c));
        DEBUG_NODE_BEATIFY_LOG("BLOOM", bloom);
        bloom_save(bloom, name->data);
    }
}

static void
storeBloomInfo(HashMap *leftInfos, HashMap *rightInfos, int opNum) {
    StringInfo infos = makeStringInfo();
    int idx = 0;
    FOREACH_HASH_KEY(Constant, c, leftInfos) {
        appendStringInfo(infos, "L:%s-", STRING_VALUE(c));
        Vector *v = (Vector *) MAP_GET_STRING(leftInfos, STRING_VALUE(c)) ;
        FOREACH_VEC(char, att, v) {
            if (idx > 0) {
                appendStringInfoChar(infos, ',');
            }
            appendStringInfo(infos, "%s", att);
        }
        appendStringInfo(infos, "%s", "\n");
        idx++;
    }
    idx = 0;
    FOREACH_HASH_KEY(Constant, c, rightInfos) {
        appendStringInfo(infos, "R:%s-", STRING_VALUE(c));
        Vector *v = (Vector *) MAP_GET_STRING(rightInfos, STRING_VALUE(c)) ;
        FOREACH_VEC(char, att, v) {
            if (idx > 0) {
                appendStringInfoChar(infos, ',');
            }
            appendStringInfo(infos, "%s", att);
        }
        appendStringInfo(infos, "%s", "\n");
        idx++;
    }
    char *qName = getStringOption(OPTION_UPDATE_PS_QUERY_NAME);
    StringInfo infoName = makeStringInfo();
    appendStringInfo(infoName, "BLInfos_%s_%s", qName, gprom_itoa(opNum));
    FILE *file = fopen(infoName->data, "w");
    fprintf(file, "%s", infos->data);
    fclose(file);
}

static void
storeAggregationData(AggregationOperator *op)
{
    HashMap *dataStructures = NULL;
    if (HAS_STRING_PROP((QueryOperator *) op, PROP_DATA_STRUCTURE_STATE)) {
        dataStructures = (HashMap *) GET_STRING_PROP((QueryOperator *) op, PROP_DATA_STRUCTURE_STATE);
    } else {
        return;
    }

    int opNum = INT_VALUE((Constant *) GET_STRING_PROP((QueryOperator *) op, PROP_OPERATOR_NUMBER));

    // each hash map is a hash map: a psmap or data map;
    FOREACH_HASH_KEY(Constant, c, dataStructures) {
        if(strcmp(STRING_VALUE(c), PROP_PROV_SKETCH_AGG) == 0) {
            storePSMapData((PSMap *) MAP_GET_STRING(dataStructures, STRING_VALUE(c)), opNum);
        } else {

            if (strncmp(backendifyIdentifier(STRING_VALUE(c)), backendifyIdentifier("avg"), 3) == 0
            || strncmp( backendifyIdentifier(STRING_VALUE(c)), backendifyIdentifier("sum"), 3) == 0
            || strncmp( backendifyIdentifier(STRING_VALUE(c)), backendifyIdentifier("cou"), 3) == 0) {
                storeGBACSsData((GBACSs *) MAP_GET_STRING(dataStructures, STRING_VALUE(c)), opNum, STRING_VALUE(c));
            } else {

            }
        }
    }
    // get ps map;
}

static void
storeProvenanceComputationData(ProvenanceComputation *op)
{
    HashMap *dataStructures = NULL;
    if (HAS_STRING_PROP((QueryOperator *) op, PROP_DATA_STRUCTURE_STATE)) {
        dataStructures = (HashMap *) GET_STRING_PROP((QueryOperator *) op, PROP_DATA_STRUCTURE_STATE);
    } else {
        return;
    }

    int opNum = INT_VALUE((Constant *) GET_STRING_PROP((QueryOperator *) op, PROP_OPERATOR_NUMBER));
    storePSMapData((PSMap *) dataStructures, opNum);
}

static void
storeDuplicateRemovalData(DuplicateRemoval *op)
{
    HashMap *dataStructures = NULL;
    if (HAS_STRING_PROP((QueryOperator *) op, PROP_DATA_STRUCTURE_STATE)) {
        dataStructures = (HashMap *) GET_STRING_PROP((QueryOperator *) op, PROP_DATA_STRUCTURE_STATE);
    } else {
        return;
    }

    int opNum = INT_VALUE((Constant *) GET_STRING_PROP((QueryOperator *) op, PROP_OPERATOR_NUMBER));
    Node * n = (Node *) MAP_GET_STRING(dataStructures, PROP_PROV_SKETCH_DUP);
    DEBUG_NODE_BEATIFY_LOG("PSMAP IN DUP", n);
    storePSMapData((PSMap *) MAP_GET_STRING(dataStructures, PROP_PROV_SKETCH_DUP), opNum);
    storeGBACSsData((GBACSs *) MAP_GET_STRING(dataStructures, PROP_DATA_STRUCTURE_DUP_DATA), opNum, "count_dup");
}

static void
storePSMapData(PSMap *psMap, int opNum)
{
    INFO_LOG("START STORE PSMAP");
    char *qName = getStringOption(OPTION_UPDATE_PS_QUERY_NAME);
    StringInfo meta = makeStringInfo();
    appendStringInfo(meta, "%s_op_%s_psmapinfo", qName, gprom_itoa(opNum));
    boolean isInfo = postgresCatalogTableExists(meta->data);
    if (!isInfo) {
        postgresExecuteStatement(CONCAT_STRINGS("CREATE TABLE ", meta->data, "(psname varchar, pslen int, psisint int)"));
    } else {
        postgresExecuteStatement(CONCAT_STRINGS("TRUNCATE ", meta->data, ";"));
    }
    FOREACH_HASH_KEY(Constant, c, psMap->provSketchs) {
        StringInfo str = makeStringInfo();
        // append ps name;
        appendStringInfo(str, "'%s'", STRING_VALUE(c));

        // append ps len;
        int sketchLen = INT_VALUE((Constant *) MAP_GET_STRING(psMap->provLens, STRING_VALUE(c)));
        appendStringInfo(str, ",%s", gprom_itoa(sketchLen));

        // append isIntsketch;
        boolean isIntSketch = FALSE;
        if (MAP_HAS_STRING_KEY(psMap->isIntSketch, STRING_VALUE(c))) {
            isIntSketch = BOOL_VALUE((Constant *) MAP_GET_STRING(psMap->isIntSketch, STRING_VALUE(c)));
        }
        appendStringInfo(str, ",%s", gprom_itoa(isIntSketch));

        postgresExecuteStatement(CONCAT_STRINGS("INSERT INTO ", meta->data, " VALUES(", str->data, ");"));

    }

    // store ps and frag cnt;
    if (opNum == 0) {
        StringInfo info = makeStringInfo();
        appendStringInfo(info, "%s_op_%s_psinfo", qName, gprom_itoa(opNum));
        boolean isTableExists = postgresCatalogTableExists(info->data);
        if (isTableExists) {
            postgresExecuteStatement(CONCAT_STRINGS("TRUNCATE ", info->data, ";"));
        } else {
            postgresExecuteStatement(CONCAT_STRINGS("CREATE TABLE ", info->data, "(psname varchar, provsketch varchar);"));
        }

        FOREACH_HASH_KEY(Constant, c, psMap->provSketchs) {
            BitSet *bitset = (BitSet *) MAP_GET_STRING(psMap->provSketchs, STRING_VALUE(c));
            postgresExecuteStatement(CONCAT_STRINGS("INSERT INTO ", info->data, " VALUES('", STRING_VALUE(c), "','", bitSetToString(bitset), "');"));
        }

        info = makeStringInfo();
        appendStringInfo(info, "%s_op_%s_fragcnt", qName, gprom_itoa(opNum));
        isTableExists = postgresCatalogTableExists(info->data);
        if (isTableExists) {
            postgresExecuteStatement(CONCAT_STRINGS("TRUNCATE ", info->data, ";"));
        } else {
            postgresExecuteStatement(CONCAT_STRINGS("CREATE TABLE ", info->data, "(psname varchar, fragno int, fragcnt int);"));
        }

        FOREACH_HASH_KEY(Constant, c, psMap->fragCount) {
            HashMap *fragCnt = (HashMap *) MAP_GET_STRING(psMap->fragCount, STRING_VALUE(c));
            FOREACH_HASH_KEY(Constant, no, fragCnt) {
                Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, INT_VALUE(no));

                postgresExecuteStatement(CONCAT_STRINGS("INSERT INTO ", info->data, " VALUES('", STRING_VALUE(c), "',", gprom_itoa(INT_VALUE(no)), ",", gprom_itoa(INT_VALUE(cnt)), ");"));
            }
        }
    } else {
        StringInfo info = makeStringInfo();
        appendStringInfo(info, "%s_op_%s_psinfo", qName, gprom_itoa(opNum));
        boolean isTableExists = postgresCatalogTableExists(info->data);
        if (isTableExists) {
            postgresExecuteStatement(CONCAT_STRINGS("TRUNCATE ", info->data, ";"));
        } else {
            postgresExecuteStatement(CONCAT_STRINGS("CREATE TABLE ", info->data, "(psname varchar, groupby varchar, provsketch varchar);"));
        }

        StringInfo q = makeStringInfo();
        appendStringInfo(q, "insert into %s (psname, groupby, provsketch) values ", info->data);
        boolean hasValue = FALSE;
        FOREACH_HASH_KEY(Constant, c, psMap->provSketchs) {
            HashMap *provSketches = (HashMap *) MAP_GET_STRING(psMap->provSketchs, STRING_VALUE(c));
            boolean isIntThisPS = BOOL_VALUE((Constant *) MAP_GET_STRING(psMap->isIntSketch, STRING_VALUE(c)));

            if (isIntThisPS) {
                FOREACH_HASH_KEY(Constant, gb, provSketches) {
                    hasValue = TRUE;
                    int sketch = INT_VALUE((Constant *) MAP_GET_STRING(provSketches, STRING_VALUE(gb)));
                    appendStringInfo(q, "('%s', '%s', '%s'),", STRING_VALUE(c), STRING_VALUE(gb), gprom_itoa(sketch));
                }
            } else {
                FOREACH_HASH_KEY(Constant, gb, provSketches) {
                    hasValue = TRUE;
                    BitSet *sketch = (BitSet *) MAP_GET_STRING(provSketches, STRING_VALUE(gb));
                    appendStringInfo(q, "('%s', '%s', '%s'),", STRING_VALUE(c), STRING_VALUE(gb), bitSetToString(sketch));
                }
            }
        }
        q->data[q->len - 1] = ';';

        if (hasValue) {
            postgresExecuteStatement(q->data);
        }

        // fragcnt;
        info = makeStringInfo();
        appendStringInfo(info, "%s_op_%s_fragcnt", qName, gprom_itoa(opNum));
        isTableExists = postgresCatalogTableExists(info->data);
        if (isTableExists) {
            postgresExecuteStatement(CONCAT_STRINGS("TRUNCATE ", info->data, ";"));
        } else {
            postgresExecuteStatement(CONCAT_STRINGS("CREATE TABLE ", info->data, "(psname varchar, groupby varchar, fragno int, fragcnt int);"));
        }
        q = makeStringInfo();
        appendStringInfo(q, "insert into %s (psname, groupby, fragno, fragcnt) values ", info->data);

        hasValue = FALSE;
        FOREACH_HASH_KEY(Constant, c, psMap->fragCount) {
            HashMap *gbFragCnt = (HashMap *) MAP_GET_STRING(psMap->fragCount, STRING_VALUE(c));
            FOREACH_HASH_KEY(Constant, gb, gbFragCnt) {
                hasValue = TRUE;
                HashMap *fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, STRING_VALUE(gb));
                FOREACH_HASH_KEY(Constant, no, fragCnt) {
                    Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, INT_VALUE(no));
                    appendStringInfo(q, "('%s', '%s', %s, %s),", STRING_VALUE(c), STRING_VALUE(gb), gprom_itoa(INT_VALUE(no)),gprom_itoa(INT_VALUE(cnt)));
                }
            }
        }
        q->data[q->len - 1] = ';';
        if (hasValue) {
            postgresExecuteStatement(q->data);
        }
    }
}

static void
storeGBACSsData(GBACSs *acs, int opNum, char *acsName)
{
    char *qName = getStringOption(OPTION_UPDATE_PS_QUERY_NAME);

    // get function type;
    // count ->1, sum ->2, avg->3;
    int type = 0;
    if (strncmp(backendifyIdentifier(acsName), backendifyIdentifier("avg"), 3) == 0) {
        type = 3;
    } else if (strncmp(backendifyIdentifier(acsName), backendifyIdentifier("sum"), 3) == 0) {
        type = 2;
    } else if (strncmp(backendifyIdentifier(acsName), backendifyIdentifier("cou"), 3) == 0) {
        type = 1;
    }


    StringInfo meta = makeStringInfo();
    appendStringInfo(meta, "%s_op_%s_%s", qName, gprom_itoa(opNum), acsName);
    boolean isInfo = postgresCatalogTableExists(meta->data);

    if (!isInfo) {
        if (type == 3){
            postgresExecuteStatement(CONCAT_STRINGS("CREATE TABLE ", meta->data, "(groupby varchar, avg float, sum float, cnt bigint)"));
        } else if (type == 2) {
            postgresExecuteStatement(CONCAT_STRINGS("CREATE TABLE ", meta->data, "(groupby varchar, sum float, cnt bigint)"));
        } else if (type == 1) {
            postgresExecuteStatement(CONCAT_STRINGS("CREATE TABLE ", meta->data, "(groupby varchar, cnt bigint)"));
        }

    } else {
        postgresExecuteStatement(CONCAT_STRINGS("TRUNCATE ", meta->data, ";"));
    }

    StringInfo q = makeStringInfo();
    if (type == 1) {
        appendStringInfo(q, "insert into %s (groupby, cnt) values ", meta->data);
        FOREACH_HASH_KEY(Constant, c, acs->map) {
            Vector *v = (Vector *) MAP_GET_STRING(acs->map, STRING_VALUE(c));
            appendStringInfo(q, "('%s', %s),", STRING_VALUE(c), gprom_ltoa(LONG_VALUE((Constant *) getVecNode(v, 0))));
        }
    } else if (type == 2) {
        appendStringInfo(q, "insert into %s (groupby, sum, cnt) values ", meta->data);
        FOREACH_HASH_KEY(Constant, c, acs->map) {
            Vector *v = (Vector *) MAP_GET_STRING(acs->map, STRING_VALUE(c));
            appendStringInfo(q, "('%s', %s, %s),", STRING_VALUE(c), gprom_ftoa(FLOAT_VALUE((Constant *) getVecNode(v, 0))), gprom_ltoa((LONG_VALUE((Constant *) getVecNode(v, 1)))));
        }
    } else if (type == 3) {
        appendStringInfo(q, "insert into %s (groupby, avg, sum, cnt) values ", meta->data);
        FOREACH_HASH_KEY(Constant, c, acs->map) {
            Vector *v = (Vector *) MAP_GET_STRING(acs->map, STRING_VALUE(c));
            appendStringInfo(q, "('%s', %s, %s, %s),", STRING_VALUE(c), gprom_ftoa(FLOAT_VALUE((Constant *) getVecNode(v, 0))), gprom_ftoa(FLOAT_VALUE((Constant *) getVecNode(v, 1))), gprom_ltoa(LONG_VALUE((Constant *) getVecNode(v, 2))));
        }
    }

    q->data[q->len - 1] = ';';
    postgresExecuteStatement(q->data);
}

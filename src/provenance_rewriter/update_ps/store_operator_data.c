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

#if HAVE_POSTGRES_BACKEND
#include "libpq-fe.h"
#endif

static void storeOperatorDataInternal(QueryOperator *op);
static void storeAggregationData(AggregationOperator *op);
static void storeProvenanceComputationData(ProvenanceComputation *op);
static void storeDuplicateRemovalData(DuplicateRemoval *op);
static void storePSMapData(PSMap *psMap, int opNum);
static void storeGBACSsData(GBACSs *acs, int opNum, char *acsName);
static void storeJoinOperatorData(JoinOperator *op);
static void storeBloom(HashMap *blooms, int opNum, char* branch);
static void storeBloomInfo(HashMap *leftInfos, HashMap *rightInfos, int opNum);
static void storeOrderOperatorData(OrderOperator *op);
static void storeOrderByMetadata(RBTRoot *root, char *qName, int qNum);
static void storeOrderByValues(OrderOperator *op, RBTRoot *root, char* qName, int opNum);


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
    } else if (isA(op, OrderOperator)) {
        storeOrderOperatorData((OrderOperator *) op);
    }


    FOREACH(QueryOperator, q, op->inputs) {
        storeOperatorDataInternal(q);
    }
}

static void
storeOrderOperatorData(OrderOperator *op)
{
    // just dealing with orderby-limit; single order operator will be ignored;
    // QueryOperator *parent = (QueryOperator *) getNthOfListP(((QueryOperator *) op)->parents, 0);

    boolean hasLimitAbove = hasLimitOpAbove((QueryOperator *) op);
    // if (!isA(parent, LimitOperator)) {
        // return;
    // }
    INFO_LOG("HAS LIMIT ABOVE %d", hasLimitAbove);
    if (!hasLimitAbove) {
        return;
    }

    HashMap * statedata = (HashMap *) getStringProperty((QueryOperator *) op, PROP_DATA_STRUCTURE_STATE);
    DEBUG_NODE_BEATIFY_LOG("ORDER STATE", statedata);

    RBTRoot * root = (RBTRoot *) MAP_GET_STRING(statedata, PROP_DATA_STRUCTURE_ORDER_BY);
    DEBUG_NODE_BEATIFY_LOG("RBT ROOT", root);

    char *qName = getStringOption(OPTION_UPDATE_PS_QUERY_NAME);
    int opNum = INT_VALUE((Constant *) GET_STRING_PROP((QueryOperator *) op, PROP_OPERATOR_NUMBER));
    storeOrderByMetadata(root, qName, opNum);
    storeOrderByValues(op, root, qName, opNum);
}

static void
storeOrderByValues(OrderOperator *op, RBTRoot *root, char* qName, int opNum)
{
    StringInfo tname = makeStringInfo();
    appendStringInfo(tname, "%s_%s_order_values", qName, gprom_itoa(opNum));

    StringInfo createTbl = makeStringInfo();
    appendStringInfo(createTbl, "create table %s (", tname->data);

    int idx = 0;
    FOREACH(AttributeDef, ad, ((QueryOperator *) op)->schema->attrDefs) {
        if (idx > 0) {
            appendStringInfo(createTbl, "%s", ",");
        }
        appendStringInfo(createTbl, "%s", strdup(ad->attrName));
        switch (ad->dataType) {
            case DT_INT:
            case DT_BOOL:
            {
                appendStringInfo(createTbl, " %s", "int");
            }
            break;
            case DT_LONG:
            {
                appendStringInfo(createTbl, " %s", "bigint");
            }
            break;
            case DT_FLOAT:
            {
                appendStringInfo(createTbl, " %s", "float");
            }
            break;
            case DT_VARCHAR2:
            case DT_STRING:
            {
                appendStringInfo(createTbl, " %s", "varchar");
            }
            break;
        }
        idx++;
    }

    Vector *vPSAttr = makeVector(VECTOR_STRING, T_Vector);
    HashMap * psIsInt = (HashMap *) MAP_GET_STRING(root->metadata, ORDER_BY_IS_PS_INT);
    FOREACH_HASH_KEY(Constant, c, psIsInt) {
        vecAppendString(vPSAttr, STRING_VALUE(c));
        appendStringInfo(createTbl, ", %s varchar", STRING_VALUE(c));
    }
    appendStringInfo(createTbl, "%s;", ")");

    Vector * vec = RBTInorderTraverse(root);

    if (vec == NULL || vec->length < 1) {
        return;
    }

    DEBUG_NODE_BEATIFY_LOG("RBT VEC:", vec);
    INFO_LOG("size of vec: %d", vec->length);

    int attrsLen = LIST_LENGTH(((QueryOperator *) op)->schema->attrDefs);

    StringInfo allTuples = makeStringInfo();
    // int tupNum = 0;
    FOREACH_VEC(RBTNode, node, vec) {
        HashMap *map = (HashMap *) node->val;
        FOREACH_HASH_KEY(Vector, n, map) {
            DEBUG_NODE_BEATIFY_LOG("RBT VAL KEY:", n);
            StringInfo vals = makeStringInfo();
            for (int idx = 0; idx < attrsLen; idx++) {
                char *format = NULL;
                if (idx > 0) {
                    format = "\t%s";
                } else {
                    format ="%s";
                }
                Constant *c = (Constant *) getVecNode(n, idx);
                switch (c->constType) {
                    case DT_INT:
                    {
                        appendStringInfo(vals, format, gprom_itoa(INT_VALUE(c)));
                    }
                    break;
                    case DT_BOOL:
                    {
                        appendStringInfo(vals, format, (BOOL_VALUE(c) == TRUE ? gprom_itoa(1) : gprom_itoa(0)));
                    }
                    break;
                    case DT_LONG:
                    {
                        appendStringInfo(vals, format, gprom_ltoa(LONG_VALUE(c)));
                    }
                    break;
                    case DT_FLOAT:
                    {
                        appendStringInfo(vals, format, gprom_ftoa(FLOAT_VALUE(c)));
                    }
                    break;
                    case DT_VARCHAR2:
                    case DT_STRING:
                    {
                        appendStringInfo(vals, format , STRING_VALUE(c));
                    }
                    break;
                }

            }
            DEBUG_NODE_BEATIFY_LOG("PSs", getVecNode(n, attrsLen));
            HashMap *allPS = (HashMap *) getVecNode(n, attrsLen);
            FOREACH_VEC(char, psAttr, vPSAttr) {
                Node * psVal = MAP_GET_STRING(allPS, psAttr);

                DEBUG_NODE_BEATIFY_LOG("PS", psVal);
                boolean isThisInt = BOOL_VALUE((Constant *) MAP_GET_STRING(psIsInt, psAttr));
                if (isThisInt) {
                    // appendStringInfo(vals, ",%s", gprom_itoa(INT_VALUE((Constant *) psVal)));
                    appendStringInfo(vals, "\t%s", gprom_itoa(INT_VALUE((Constant *) psVal)));
                } else {
                    // appendStringInfo(vals, ",%s", (bitSetToString((BitSet *) psVal)));
                    appendStringInfo(vals, "\t%s", (bitSetToString((BitSet *) psVal)));

                }
            }

            INFO_LOG("data sql %s", vals->data);
            int rep = INT_VALUE((Constant *) getMap(map, (Node *) n));
            for (int i = 0; i < rep; i++) {
                appendStringInfo(allTuples, "%s\r", vals->data);
            }
        }
    }
    INFO_LOG("ALL TUPLES %s", allTuples->data);
    postgresCopyToDB(createTbl, allTuples, tname->data);
}

static void
storeOrderByMetadata(RBTRoot *root, char *qName, int opNum)
{
    StringInfo infos = makeStringInfo();
    HashMap *metadata = (HashMap *) root->metadata;
    HashMap *psInt = (HashMap *) MAP_GET_STRING(metadata, ORDER_BY_IS_PS_INT);
    HashMap *psLen = (HashMap *) MAP_GET_STRING(metadata, ORDER_BY_PS_LENS);
    FOREACH_HASH_KEY(Constant, c, psInt) {
        boolean isInt = BOOL_VALUE((Constant *) MAP_GET_STRING(psInt, STRING_VALUE(c)));
        int len = INT_VALUE((Constant *) MAP_GET_STRING(psLen, STRING_VALUE(c)));
        appendStringInfo(infos, "%s,%s,%s\n", STRING_VALUE(c), gprom_itoa(len), gprom_itoa(isInt));
    }
    StringInfo infoName = makeStringInfo();
    appendStringInfo(infoName, "ORDERInfos_%s_%s", qName, gprom_itoa(opNum));
    FILE *file = fopen(infoName->data, "w");
    fprintf(file, "%s", infos->data);
    fclose(file);
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
    FOREACH_HASH_KEY(Constant, c, leftInfos) {
        appendStringInfo(infos, "L:%s-", STRING_VALUE(c));
        Vector *v = (Vector *) MAP_GET_STRING(leftInfos, STRING_VALUE(c)) ;
        int idx = 0;
        FOREACH_VEC(char, att, v) {
            if (idx > 0) {
                appendStringInfoChar(infos, ',');
            }
            appendStringInfo(infos, "%s", att);
            idx++;
        }
        appendStringInfo(infos, "%s", "\n");
    }
    FOREACH_HASH_KEY(Constant, c, rightInfos) {
        appendStringInfo(infos, "R:%s-", STRING_VALUE(c));
        Vector *v = (Vector *) MAP_GET_STRING(rightInfos, STRING_VALUE(c)) ;
        int idx = 0;
        FOREACH_VEC(char, att, v) {
            if (idx > 0) {
                appendStringInfoChar(infos, ',');
            }
            appendStringInfo(infos, "%s", att);
            idx++;
        }
        appendStringInfo(infos, "%s", "\n");
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
    // the top operator;
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

        StringInfo stmt = makeStringInfo() ;
        char *stmtName = CONCAT_STRINGS("STMT_", info->data);
        appendStringInfo(stmt, "INSERT INTO %s VALUES($1::varchar, $2::int, $3::int);", info->data);
        postgresPrepareUpdatePS(stmt->data, stmtName, 3);

        FOREACH_HASH_KEY(Constant, c, psMap->fragCount) {
            HashMap *fragCnt = (HashMap *) MAP_GET_STRING(psMap->fragCount, STRING_VALUE(c));
            FOREACH_HASH_KEY(Constant, no, fragCnt) {
                Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, INT_VALUE(no));

                char ** params = CALLOC(sizeof(char *), 3);
                params[0] = STRING_VALUE(c);
                params[1] = gprom_itoa(INT_VALUE(no));
                params[2] = gprom_itoa(INT_VALUE(cnt));
                postgresExecPrepareUpdatePS(NULL, stmtName, 3, params);
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
        appendStringInfo(q, "INSERT INTO %s VALUES ($1::varchar, $2::varchar, $3::varchar)", info->data);
        char *stmtName = CONCAT_STRINGS("STMT_", info->data);

        postgresPrepareUpdatePS(q->data, stmtName, 3);
        // PGconn *conn = getPostgresConnection();

        // boolean hasValue = FALSE;
        FOREACH_HASH_KEY(Constant, c, psMap->provSketchs) {
            HashMap *provSketches = (HashMap *) MAP_GET_STRING(psMap->provSketchs, STRING_VALUE(c));
            boolean isIntThisPS = BOOL_VALUE((Constant *) MAP_GET_STRING(psMap->isIntSketch, STRING_VALUE(c)));

            if (isIntThisPS) {
                FOREACH_HASH_KEY(Constant, gb, provSketches) {
                    // hasValue = TRUE;
                    char **params = CALLOC(sizeof(char *) , 3);
                    params[0] = (char *) STRING_VALUE(c);

                    // translate gb to bytea;
                    // size_t to_length;
                    // unsigned char *bytea = PQescapeByteaConn(conn, (unsigned char *) STRING_VALUE(gb), strlen(STRING_VALUE(gb)), &to_length);
                    // params[1] = (char *) bytea;
                    params[1] = (char *) STRING_VALUE(gb);

                    // get sketch;
                    int sketch = INT_VALUE((Constant *) MAP_GET_STRING(provSketches, STRING_VALUE(gb)));
                    params[2] = gprom_itoa(sketch);
                    postgresExecPrepareUpdatePS(NULL, stmtName, 3, params);

                }
            } else {
                FOREACH_HASH_KEY(Constant, gb, provSketches) {
                    // hasValue = TRUE;
                    char **params = CALLOC(sizeof(char *), 3);
                    params[0] = STRING_VALUE(c);

                    // translate gb to bytea;
                    // size_t to_length;
                    // unsigned char *bytea = PQescapeByteaConn(conn, (unsigned char *) STRING_VALUE(gb), strlen(STRING_VALUE(gb)), &to_length);
                    // params[1] = (char *) bytea;
                    params[1] = (char *) STRING_VALUE(gb);

                    BitSet *sketch = (BitSet *) MAP_GET_STRING(provSketches, STRING_VALUE(gb));

                    params[2] = (char *) bitSetToString(sketch);
                    // appendStringInfo(q, "('%s', '%s', '%s'),", STRING_VALUE(c), STRING_VALUE(gb), bitSetToString(sketch));

                    postgresExecPrepareUpdatePS(NULL, stmtName, 3, params);
                }
            }
        }

        // fragcnt;
        info = makeStringInfo();
        appendStringInfo(info, "%s_op_%s_fragcnt", qName, gprom_itoa(opNum));
        isTableExists = postgresCatalogTableExists(info->data);
        if (isTableExists) {
            postgresExecuteStatement(CONCAT_STRINGS("TRUNCATE ", info->data, ";"));
        } else {
            postgresExecuteStatement(CONCAT_STRINGS("CREATE TABLE ", info->data, "(psname varchar, groupby bytea, fragno int, fragcnt int);"));
        }
        q = makeStringInfo();
        // appendStringInfo(q, "insert into %s (psname, groupby, fragno, fragcnt) values ", info->data);
        // appendStringInfo(q, "INSERT INTO %s VALUES ($1::varchar, $2::bytea, $3::int, $4::int);", info->data);
        appendStringInfo(q, "INSERT INTO %s VALUES ($1::varchar, $2::varchar, $3::int, $4::int);", info->data);
        stmtName = CONCAT_STRINGS("STMT_", info->data);

        postgresPrepareUpdatePS(q->data, stmtName, 4);

        // hasValue = FALSE;
        FOREACH_HASH_KEY(Constant, c, psMap->fragCount) {
            HashMap *gbFragCnt = (HashMap *) MAP_GET_STRING(psMap->fragCount, STRING_VALUE(c));
            FOREACH_HASH_KEY(Constant, gb, gbFragCnt) {
                // hasValue = TRUE;
                HashMap *fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, STRING_VALUE(gb));
                FOREACH_HASH_KEY(Constant, no, fragCnt) {
                    char ** params = CALLOC(sizeof(char *), 4);
                    params[0] = STRING_VALUE(c);

                    // bytea;
                    // size_t to_length;
                    // unsigned char *bytea = PQescapeByteaConn(conn, (unsigned char *) STRING_VALUE(gb), strlen(STRING_VALUE(gb)), &to_length);
                    // params[1] = (char *) bytea;
                    params[1] = (char *) STRING_VALUE(gb);

                    params[2] = gprom_itoa(INT_VALUE(no));

                    Constant *cnt = (Constant *) MAP_GET_INT(fragCnt, INT_VALUE(no));

                    params[3] = gprom_itoa(INT_VALUE(cnt));
                    postgresExecPrepareUpdatePS(NULL, stmtName, 4, params);
                    // appendStringInfo(q, "('%s', '%s', %s, %s),", STRING_VALUE(c), STRING_VALUE(gb), gprom_itoa(INT_VALUE(no)),gprom_itoa(INT_VALUE(cnt)));
                }
            }
        }
        // q->data[q->len - 1] = ';';
        // if (hasValue) {
            // postgresExecuteStatement(q->data);
        // }
    }
}

static void
storeGBACSsData(GBACSs *acs, int opNum, char *acsName)
{
    // PGconn *conn = getPostgresConnection();
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
    char *stmtName;
    if (type == 1) {
        // appendStringInfo(q, "insert into %s (groupby, cnt) values ", meta->data);
        appendStringInfo(q, "INSERT INTO %s VALUES ($1::varchar, $2::bigint);", meta->data);
        stmtName = CONCAT_STRINGS("STMT_", meta->data);
        postgresPrepareUpdatePS(q->data, stmtName, 2);
        FOREACH_HASH_KEY(Constant, c, acs->map) {
            Vector *v = (Vector *) MAP_GET_STRING(acs->map, STRING_VALUE(c));

            char ** params = CALLOC(sizeof(char *), 2);
            //bytea;
            // size_t to_length;
            // unsigned char *bytea = PQescapeByteaConn(conn, (unsigned char *) STRING_VALUE(c), strlen(STRING_VALUE(c)), &to_length);
            // params[0] = (char *) bytea;
            params[0] = STRING_VALUE(c);
            params[1] = gprom_ltoa(LONG_VALUE((Constant *) getVecNode(v, 0)));
            // appendStringInfo(q, "('%s', %s),", STRING_VALUE(c), gprom_ltoa(LONG_VALUE((Constant *) getVecNode(v, 0))));
            postgresExecPrepareUpdatePS(NULL, stmtName, 2, params);
        }
    } else if (type == 2) {
        appendStringInfo(q, "INSERT INTO %s VALUES ($1::varchar, $2::float,$3::bigint);", meta->data);
        stmtName = CONCAT_STRINGS("STMT_", meta->data);
        postgresPrepareUpdatePS(q->data, stmtName, 3);
        // appendStringInfo(q, "insert into %s (groupby, sum, cnt) values ", meta->data);
        FOREACH_HASH_KEY(Constant, c, acs->map) {
            char ** params = CALLOC(sizeof(char *), 3);

            // size_t to_length;
            // unsigned char *bytea = PQescapeByteaConn(conn, (unsigned char *) STRING_VALUE(c), strlen(STRING_VALUE(c)), &to_length);
            // params[0] = (char *) bytea;
            params[0] = (char *) STRING_VALUE(c);

            Vector *v = (Vector *) MAP_GET_STRING(acs->map, STRING_VALUE(c));
            params[1] = gprom_ftoa(FLOAT_VALUE((Constant *) getVecNode(v, 0)));
            params[2] = gprom_ltoa(LONG_VALUE((Constant *) getVecNode(v, 1)));
            postgresExecPrepareUpdatePS(NULL, stmtName, 3, params);
            // appendStringInfo(q, "('%s', %s, %s),", STRING_VALUE(c), gprom_ftoa(FLOAT_VALUE((Constant *) getVecNode(v, 0))), gprom_ltoa((LONG_VALUE((Constant *) getVecNode(v, 1)))));
        }
    } else if (type == 3) {
        // appendStringInfo(q, "insert into %s (groupby, avg, sum, cnt) values ", meta->data);
        appendStringInfo(q, "INSERT INTO %s VALUES ($1::varchar, $2::float, $3::float, $4::bigint);", meta->data);
        stmtName = CONCAT_STRINGS("STMT_", meta->data);
        postgresPrepareUpdatePS(q->data, stmtName, 4);
        FOREACH_HASH_KEY(Constant, c, acs->map) {
            char ** params = CALLOC(sizeof(char *), 4);

            // size_t to_length;
            // unsigned char *bytea = PQescapeByteaConn(conn, (unsigned char *) STRING_VALUE(c), strlen(STRING_VALUE(c)), &to_length);
            // params[0] = (char *) bytea;
            params[1] = (char *) STRING_VALUE(c);

            Vector *v = (Vector *) MAP_GET_STRING(acs->map, STRING_VALUE(c));

            params[1] = gprom_ftoa(FLOAT_VALUE((Constant *) getVecNode(v, 0)));
            params[2] = gprom_ftoa(FLOAT_VALUE((Constant *) getVecNode(v, 1)));
            params[3] = gprom_ltoa(LONG_VALUE((Constant *) getVecNode(v, 2)));
            postgresExecPrepareUpdatePS(NULL, stmtName, 4, params);
            // appendStringInfo(q, "('%s', %s, %s, %s),", STRING_VALUE(c), gprom_ftoa(FLOAT_VALUE((Constant *) getVecNode(v, 0))), gprom_ftoa(FLOAT_VALUE((Constant *) getVecNode(v, 1))), gprom_ltoa(LONG_VALUE((Constant *) getVecNode(v, 2))));
        }
    }

    // q->data[q->len - 1] = ';';
    // postgresExecuteStatement(q->data);
}

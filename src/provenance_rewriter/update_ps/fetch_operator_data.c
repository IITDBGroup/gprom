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
#include "provenance_rewriter/update_ps/fetch_operator_data.h"

// fetch
static void fetchOperatorDataInternal(QueryOperator *op);
static void fetchAggregationData(AggregationOperator *op);
static void fetchProvenanceComputationData(ProvenanceComputation * op);
static void fetchDuplicateRemovalData(DuplicateRemoval *op);
static PSMap *fetchPSMapData(int opNum);
static GBACSs *fetchGBACSsData(int opNum, char* acsName, int type);
static void fetchJoinOperatorData(JoinOperator *op);
static void fetchBloomInfo(HashMap *joinState, int opNum);
static void fetchBlooms(HashMap *joinState, int opNum);
static void fetchOrderOperatorData(OrderOperator *op);
static void fetchOrderByMetadata(OrderOperator * op, RBTRoot * root, char *qName, int opNum);
static void fetchOrderByValues(OrderOperator *op, RBTRoot *root, char *qName, int opNum);




void
fetchOperatorData(QueryOperator *op)
{
    fetchOperatorDataInternal(op);
}

static void
fetchOperatorDataInternal(QueryOperator *op)
{
    if (isA(op, TableAccessOperator)) {
        return;
    }

    if (isA(op, AggregationOperator)) {
        fetchAggregationData((AggregationOperator *) op);
    } else if (isA(op, ProvenanceComputation)) {
        fetchProvenanceComputationData((ProvenanceComputation *) op);
    } else if (isA(op, DuplicateRemoval)) {
        fetchDuplicateRemovalData((DuplicateRemoval *) op);
    } else if (isA(op, JoinOperator)) {
        boolean usingBF = getBoolOption(OPTION_UPDATE_PS_JOIN_USING_BF);
        if (usingBF) {
            fetchJoinOperatorData((JoinOperator *) op);
        }
    } else if (isA(op, OrderOperator)) {
        fetchOrderOperatorData((OrderOperator *) op);
    }

    FOREACH(QueryOperator, q, op->inputs) {
        fetchOperatorDataInternal(q);
    }
}

static void fetchOrderOperatorData(OrderOperator *op)
{
    QueryOperator *parent = (QueryOperator *) getNthOfListP(((QueryOperator *) op)->parents, 0);
    if (!isA(parent, LimitOperator)) {
        return;
    }
    RBTRoot *root = makeRBT(RBT_ORDER_BY, TRUE);
    char *qName = getStringOption(OPTION_UPDATE_PS_QUERY_NAME);
    int opNum = INT_VALUE((Constant *) GET_STRING_PROP((QueryOperator *) op, PROP_OPERATOR_NUMBER));
    fetchOrderByMetadata(op, root, qName, opNum);
    fetchOrderByValues(op, root, qName, opNum);

    HashMap *dataStructures = NEW_MAP(Constant, Node);
	addToMap(dataStructures, (Node *) createConstString(PROP_DATA_STRUCTURE_ORDER_BY), (Node *) root);
    DEBUG_NODE_BEATIFY_LOG("order fetch state", dataStructures);
    SET_STRING_PROP(op, PROP_DATA_STRUCTURE_STATE, dataStructures);
}

static void
fetchOrderByValues(OrderOperator *op, RBTRoot *root, char *qName, int opNum)
{
    StringInfo fetchSQL = makeStringInfo();
    appendStringInfo(fetchSQL, "select * from %s_%s_order_values;", qName, gprom_itoa(opNum));
    Relation *rel = executeQuery(fetchSQL->data);
    Vector *tuples = rel->tuples;

    List *attAds = ((QueryOperator *) op)->schema->attrDefs;
    // deal with normal attributes;
    int rows = tuples->length;
    int cols = LIST_LENGTH(attAds);

    Vector *allKeys = makeVector(VECTOR_NODE, T_Vector);
    Vector *allVals = makeVector(VECTOR_NODE, T_Vector);
    for (int row = 0; row < rows; row++) {
        vecAppendNode(allKeys, (Node *) makeVector(VECTOR_NODE, T_Vector));
        vecAppendNode(allVals, (Node *) makeVector(VECTOR_NODE, T_Vector));
    }

    Set *allOrderByAttrs = (Set *) MAP_GET_STRING(root->metadata, ORDER_BY_ATTRS);
    for (int col = 0; col < cols; col++) {
        AttributeDef *ad = getNthOfListP(attAds, col);
        boolean isThisAOrderByAttr = FALSE;
        if (hasSetElem(allOrderByAttrs, (void *) ad->attrName)) {
            isThisAOrderByAttr = TRUE;
        }
        switch (ad->dataType) {
            case DT_BOOL:
            {
                for (int row = 0; row < rows; row++) {
                    char * val = (char *) getVecString((Vector *) getVecNode(tuples, row), col);

                    Constant *cVal = createConstInt(atoi(val));
                    vecAppendNode((Vector *) getVecNode(allVals, row), (Node *) cVal);
                    if (isThisAOrderByAttr) {
                        vecAppendNode((Vector *) getVecNode(allKeys, row), (Node *) cVal);
                    }
                }
            }
            break;
            case DT_INT:
            {
                for (int row = 0; row < rows; row++) {
                    char * val = (char *) getVecString((Vector *) getVecNode(tuples, row), col);

                    Constant *cVal = createConstInt(atoi(val));
                    vecAppendNode((Vector *) getVecNode(allVals, row), (Node *) cVal);
                    if (isThisAOrderByAttr) {
                        vecAppendNode((Vector *) getVecNode(allKeys, row), (Node *) cVal);
                    }
                }
            }
            break;
            case DT_LONG:
            {
                for (int row = 0; row < rows; row++) {
                    char * val = (char *) getVecString((Vector *) getVecNode(tuples, row), col);

                    Constant *cVal = createConstLong(atol(val));
                    vecAppendNode((Vector *) getVecNode(allVals, row), (Node *) cVal);
                    if (isThisAOrderByAttr) {
                        vecAppendNode((Vector *) getVecNode(allKeys, row), (Node *) cVal);
                    }
                }
            }
            break;
            case DT_FLOAT:
            {
                for (int row = 0; row < rows; row++) {
                    char * val = (char *) getVecString((Vector *) getVecNode(tuples, row), col);

                    Constant *cVal = createConstFloat(atof(val));
                    vecAppendNode((Vector *) getVecNode(allVals, row), (Node *) cVal);
                    if (isThisAOrderByAttr) {
                        vecAppendNode((Vector *) getVecNode(allKeys, row), (Node *) cVal);
                    }
                }
            }
            break;
            case DT_VARCHAR2:
            case DT_STRING:
            {
                for (int row = 0; row < rows; row++) {
                    char * val = (char *) getVecString((Vector *) getVecNode(tuples, row), col);

                    Constant *cVal = createConstString(val);
                    vecAppendNode((Vector *) getVecNode(allVals, row), (Node *) cVal);
                    if (isThisAOrderByAttr) {
                        vecAppendNode((Vector *) getVecNode(allKeys, row), (Node *) cVal);
                    }

                }
            }
            break;
        }
    }

    HashMap *isPSInt = (HashMap *) MAP_GET_STRING(root->metadata, ORDER_BY_IS_PS_INT);
    for (int col = cols; col < LIST_LENGTH(rel->schema); col++) {
        char *psName = (char *) getNthOfListP(rel->schema, col);
        boolean isInt = BOOL_VALUE((Constant *) MAP_GET_STRING(isPSInt, psName));

        for (int row = 0; row < rows; row++) {
            if (col == cols) {
                vecAppendNode((Vector *) getVecNode(allVals, row), (Node *) NEW_MAP(Constant, Node));
            }

            char *psVal = (char *) getVecString((Vector *) getVecNode(tuples, row), col);
            HashMap *map = (HashMap *) getVecNode((Vector *) getVecNode(allVals, row), cols);
            if (isInt) {
                addToMap(map, (Node *) createConstString(psName), (Node *) createConstInt(atoi(psVal)));
            } else {
                addToMap(map, (Node *) createConstString(psName), (Node *) stringToBitset(psVal));
            }
        }
    }
    INFO_LOG("VEC LEN IN ORDER BY %d", allKeys->length);
    for (int row = 0; row < rows; row++) {
        RBTInsert(root, (Node *) getVecNode(allKeys, row), (Node *) getVecNode(allVals, row));
    }
    INFO_LOG("order result attrs %s", rel->schema);
}

static void
fetchOrderByMetadata(OrderOperator * op, RBTRoot * root, char *qName, int opNum)
{
    StringInfo infoName = makeStringInfo();
    appendStringInfo(infoName, "ORDERInfos_%s_%s", qName, gprom_itoa(opNum));
    FILE *file = fopen(infoName->data, "r");
    if (!file) {
        return;
    }

    HashMap *psIsInt = NEW_MAP(Constant, Constant);
    HashMap *psLen = NEW_MAP(Constant, Constant);

    char line[1000];
    int LINELEN = 1000;
    while (fgets(line, LINELEN, file) != NULL) {
        int pos = 0;
        int comma = 0;
        int linelen = strlen(line);
        char psAttr[1000];
        char isInt[10];
        char len[100];
        for (int i = 1; i < linelen; i++) {
            // line[linelen - 1] = '\n'
            if (line[i] == ',' || i == linelen - 1) {
                if (comma == 0) {
                    strncpy(psAttr, line + pos, i - pos);
                    psAttr[i - pos] = '\0';
                    pos = i + 1;
                } else if (comma == 1) {
                    strncpy(len, line + pos, i - pos);
                    len[i - pos] = '\0';
                    pos = i + 1;
                } else if (comma == 2) {
                    strncpy(isInt, line + pos, i - pos);
                    isInt[i - pos] = '\0';
                    pos = i + 1;
                }
                comma++;
            }
        }
        // INFO_LOG("attr: %s, len: %d, isInt: %d", psAttr, atoi(len), atoi(isInt));
        addToMap(psLen, (Node *) createConstString(strdup(psAttr)), (Node *) createConstInt(atoi(len)));
        addToMap(psIsInt, (Node *) createConstString(strdup(psAttr)), (Node *) createConstBool(atoi(isInt)));
    }
    fclose(file);

    addToMap(root->metadata, (Node *) createConstString(ORDER_BY_IS_PS_INT), (Node *) psIsInt);
	addToMap(root->metadata, (Node *) createConstString(ORDER_BY_PS_LENS), (Node *) psLen);


	addToMap(root->metadata, (Node *) createConstString(ORDER_BY_ATTR_NUMS), (Node *) createConstInt(LIST_LENGTH(((OrderOperator *) op)->orderExprs)));
    // addToMap(root->metadata, )
    // DEBUG_NODE_BEATIFY_LOG("PSISINT", psIsInt);
    // DEBUG_NODE_BEATIFY_LOG("PSLEN", psLen);

    Vector *orderByASCs = makeVector(VECTOR_INT, T_Vector);
    Set *orderAttrNameSet = STRSET();
    FOREACH(OrderExpr, oe, op->orderExprs) {
        if (oe->order == SORT_ASC) {
            vecAppendInt(orderByASCs, 1);
        } else {
            vecAppendInt(orderByASCs, -1);
        }
        addToSet(orderAttrNameSet, strdup(((AttributeReference *) oe->expr)->name));
    }

    addToMap(root->metadata, (Node *) createConstString(ORDER_BY_ASCS), (Node *) orderByASCs);
	addToMap(root->metadata, (Node *) createConstString(ORDER_BY_ATTRS), (Node *) orderAttrNameSet);
}

static void
fetchJoinOperatorData(JoinOperator *op) {
    HashMap *joinState = NEW_MAP(Constant, Node);
    int opNum = INT_VALUE((Constant *) GET_STRING_PROP((QueryOperator*) op, PROP_OPERATOR_NUMBER));

    fetchBloomInfo(joinState, opNum);
    fetchBlooms(joinState, opNum);
    setStringProperty((QueryOperator *) op, PROP_DATA_STRUCTURE_JOIN, (Node *) joinState);
    DEBUG_NODE_BEATIFY_LOG("join state", joinState);
}

static void
fetchBlooms(HashMap *joinState, int opNum)
{
    char *qName = getStringOption(OPTION_UPDATE_PS_QUERY_NAME);

    HashMap *leftInfos = (HashMap *) MAP_GET_STRING(joinState, JOIN_LEFT_BLOOM_ATT_MAPPING);
    HashMap *leftBlooms = NEW_MAP(Constant, Node);
    FOREACH_HASH_KEY(Constant, c, leftInfos) {
        StringInfo bloomName = makeStringInfo();
        appendStringInfo(bloomName, "%s_%s_left_%s", qName, gprom_itoa(opNum), STRING_VALUE(c));
        Bloom *bloom = createBloomFilter();
        bloom_load(bloom, bloomName->data);
        addToMap(leftBlooms, (Node *) copyObject(c), (Node *) bloom);
    }

    HashMap *rightInfos = (HashMap *) MAP_GET_STRING(joinState, JOIN_RIGHT_BLOOM_ATT_MAPPING);
    HashMap *rightBlooms = NEW_MAP(Constant, Node);
    FOREACH_HASH_KEY(Constant, c, rightInfos) {
        StringInfo bloomName = makeStringInfo();
        appendStringInfo(bloomName, "%s_%s_right_%s", qName, gprom_itoa(opNum), STRING_VALUE(c));
        Bloom *bloom = createBloomFilter();
        bloom_load(bloom, bloomName->data);
        addToMap(rightBlooms, (Node *) copyObject(c), (Node *) bloom);
    }

    addToMap(joinState, (Node *) createConstString(JOIN_LEFT_BLOOM), (Node *) leftBlooms);
    addToMap(joinState, (Node *) createConstString(JOIN_RIGHT_BLOOM), (Node *) rightBlooms);
}

static void
fetchBloomInfo(HashMap *joinState, int opNum)
{
    char *qName = getStringOption(OPTION_UPDATE_PS_QUERY_NAME);
    StringInfo infoName = makeStringInfo();
    appendStringInfo(infoName, "BLInfos_%s_%s", qName, gprom_itoa(opNum));
    FILE *file = fopen(infoName->data, "r");
    if (!file) {
        return;
    }

    char line[1000];
    int LINELEN = 1000;
    HashMap *leftMapping = NEW_MAP(Constant, Node);
    HashMap *rightMapping = NEW_MAP(Constant, Node);

    while (fgets(line, LINELEN, file) != NULL) {
        char branch = line[0] ;
        int pos = 2;
        int lineLen = strlen(line);
        char fromAtt[1000];
        Vector *vec = makeVector(VECTOR_STRING, T_Vector);
        for (int i = 3; i < lineLen; i++) {
            if (line[i] == '-') {
                strncpy(fromAtt, line + pos, i - pos);
                fromAtt[i - pos] = '\0';
                pos = i + 1;
            } else if (line[i] == ',' || i == lineLen - 1) {
                // NOTE: line[lineLen - 1] == '\n'
                INFO_LOG("I_POS LEN line: '%s', lenLen: '%d', pos: '%d', char: '%c'", line, lineLen, i - pos, line[i]);
                char att[1000];
                strncpy(att, line + pos, i - pos);
                att[i - pos] = '\0';
                vecAppendString(vec, strdup(att));
                pos = i + 1;
            }
        }

        if (branch == 'L') {
            addToMap(leftMapping, (Node *) createConstString(fromAtt), (Node *) vec);
        } else if (branch == 'R') {
            addToMap(rightMapping, (Node *) createConstString(fromAtt), (Node *) vec);
        }
    }
    fclose(file);

    addToMap(joinState, (Node *) createConstString(JOIN_LEFT_BLOOM_ATT_MAPPING), (Node *) leftMapping);
    addToMap(joinState, (Node *) createConstString(JOIN_RIGHT_BLOOM_ATT_MAPPING), (Node *) rightMapping);
    DEBUG_NODE_BEATIFY_LOG("JOIN STATE FETCH", joinState);
}

static void
fetchAggregationData(AggregationOperator *op)
{

    int opNum = INT_VALUE((Constant *) GET_STRING_PROP((QueryOperator *) op, PROP_OPERATOR_NUMBER));
    HashMap *dataStructures = NEW_MAP(Constant, Node);
    PSMap * psMap = fetchPSMapData(opNum);
    addToMap(dataStructures, (Node *) createConstString(PROP_PROV_SKETCH_AGG), (Node *) psMap);


    FOREACH(FunctionCall, fc, op->aggrs) {
        AttributeReference *ar = (AttributeReference *) getNthOfListP(fc->args, 0);
        char *acsName = CONCAT_STRINGS(fc->functionname, "_", ar->name);
        if (strcmp(fc->functionname, AVG_FUNC_NAME) == 0) {
            GBACSs *acs = fetchGBACSsData(opNum, acsName, 3);
            addToMap(dataStructures, (Node *) createConstString(acsName), (Node *) acs);
        } else if (strcmp(fc->functionname, SUM_FUNC_NAME) == 0) {
            GBACSs *acs = fetchGBACSsData(opNum, acsName, 2);
            addToMap(dataStructures, (Node *) createConstString(acsName), (Node *) acs);
        } else if (strcmp(fc->functionname, COUNT_FUNC_NAME) == 0) {
            GBACSs *acs = fetchGBACSsData(opNum, acsName, 1);
            addToMap(dataStructures, (Node *) createConstString(acsName), (Node *) acs);
        } else {

        }
    }

    setStringProperty((QueryOperator *) op, PROP_DATA_STRUCTURE_STATE, (Node *) dataStructures);
}

static void
fetchProvenanceComputationData(ProvenanceComputation *op) {
    int opNum = INT_VALUE((Constant *) GET_STRING_PROP((QueryOperator *) op, PROP_OPERATOR_NUMBER));
    PSMap * psMap = fetchPSMapData(opNum);
    setStringProperty((QueryOperator *) op, PROP_DATA_STRUCTURE_STATE, (Node *) psMap);
}

static void
fetchDuplicateRemovalData(DuplicateRemoval *op)
{
    HashMap *dataStructures = NEW_MAP(Constant, Node);
    int opNum = INT_VALUE((Constant *) GET_STRING_PROP((QueryOperator *) op, PROP_OPERATOR_NUMBER));
    PSMap * psMap = fetchPSMapData(opNum);
    addToMap(dataStructures, (Node *) createConstString(PROP_PROV_SKETCH_DUP), (Node *) psMap);
    GBACSs *acs = fetchGBACSsData(opNum, "count_dup", 1);
    addToMap(dataStructures, (Node *) createConstString(PROP_DATA_STRUCTURE_DUP_DATA), (Node *) acs);
    setStringProperty((QueryOperator *) op, PROP_DATA_STRUCTURE_STATE, (Node *) dataStructures);
}

static PSMap *
fetchPSMapData(int opNum)
{
    PSMap *psMap = makePSMap();
    char *qName = getStringOption(OPTION_UPDATE_PS_QUERY_NAME);
    StringInfo meta = makeStringInfo();
    appendStringInfo(meta, "SELECT * FROM %s_op_%s_psmapinfo;", qName, gprom_itoa(opNum));

    // fetch ps map info;
    Relation *rel = postgresExecuteQuery(meta->data);

    FOREACH_VEC(Vector, v, rel->tuples) {
        char *name = (char *) getVecString(v, 0);
        int psLen = atoi((char *) getVecString(v, 1));
        int isPSInt = atoi((char *) getVecString(v, 2));
        boolean isInt = (isPSInt == 1 ? TRUE : FALSE);
        addToMap(psMap->isIntSketch, (Node *) createConstString(name), (Node *) createConstBool(isInt));
        addToMap(psMap->provLens, (Node *) createConstString(name), (Node *) createConstInt(psLen));
    }

    // fetch ps and fragcnt;
    if (opNum == 0) {
        // ps
        meta = makeStringInfo();
        appendStringInfo(meta, "select * from %s_op_%s_psinfo;", qName, gprom_itoa(opNum));
        rel = postgresExecuteQuery(meta->data);
        FOREACH_VEC(Vector, v, rel->tuples) {
            char *name = (char *) getVecString(v, 0);
            char *bitStr = (char *) getVecString(v, 1);
            BitSet *bitSet = (BitSet *) stringToBitset(bitStr);
            addToMap(psMap->provSketchs, (Node *) createConstString(name), (Node *) bitSet);
        }

        // frag cnt;
        meta = makeStringInfo();
        appendStringInfo(meta, "select * from %s_op_%s_fragcnt;", qName, gprom_itoa(opNum));
        rel = postgresExecuteQuery(meta->data);

        FOREACH_VEC(Vector, v, rel->tuples) {
            char *name = (char *) getVecString(v, 0);
            int no = atoi((char *) getVecString(v, 1));
            int cnt = atoi((char *) getVecString(v, 2));

            HashMap *fragCnt = (HashMap *) MAP_GET_STRING(psMap->fragCount, name);
            if (fragCnt == NULL) {
                fragCnt = NEW_MAP(Constant, Node);
                addToMap(psMap->fragCount, (Node *) createConstString(name), (Node *) fragCnt);
            }
            addToMap(fragCnt, (Node *) createConstInt(no), (Node *) createConstInt(cnt));
        }
    } else {
        // ps
        meta = makeStringInfo();
        appendStringInfo(meta, "select * from %s_op_%s_psinfo;", qName, gprom_itoa(opNum));
        rel = postgresExecuteQuery(meta->data);
        FOREACH_VEC(Vector, v, rel->tuples) {
            char *name = (char *) getVecString(v, 0);
            char *groupby = (char *) getVecString(v, 1);
            char *bitStr = (char *) getVecString(v, 2);

            HashMap *gbPS = (HashMap *) MAP_GET_STRING(psMap->provSketchs, name);
            if (gbPS == NULL) {
                gbPS = NEW_MAP(Constant, Node);
                addToMap(psMap->provSketchs, (Node *) createConstString(name), (Node *) gbPS);
            }

            if (BOOL_VALUE((Constant *) MAP_GET_STRING(psMap->isIntSketch, name)) == TRUE) {
                int ps = atoi(bitStr);
                addToMap(gbPS, (Node *) createConstString(groupby), (Node *) createConstInt(ps));
            } else {
                BitSet *bitSet = (BitSet *) stringToBitset(bitStr);
                addToMap(gbPS, (Node *) createConstString(groupby), (Node *) bitSet);
            }
        }

        // fragCnt;
        meta = makeStringInfo();
        appendStringInfo(meta, "select * from %s_op_%s_fragcnt", qName, gprom_itoa(opNum));
        rel = postgresExecuteQuery(meta->data);
        FOREACH_VEC(Vector, v, rel->tuples) {
            char *name = (char *) getVecString(v, 0);
            char *groupby = (char *) getVecString(v, 1);
            int no = atoi((char *) getVecString(v, 2));
            int cnt = atoi((char *) getVecString(v, 3));

            HashMap *gbFragCnt = (HashMap *) MAP_GET_STRING(psMap->fragCount, name);
            if (gbFragCnt == NULL) {
                gbFragCnt = NEW_MAP(Constant, Node);
                addToMap(psMap->fragCount, (Node *) createConstString(name), (Node *) gbFragCnt);
            }

            HashMap *fragCnt = (HashMap *) MAP_GET_STRING(gbFragCnt, groupby);
            if (fragCnt == NULL) {
                fragCnt = NEW_MAP(Constant, Constant);
                addToMap(gbFragCnt, (Node *) createConstString(groupby), (Node *) fragCnt);
            }

            addToMap(fragCnt, (Node *) createConstInt(no), (Node *) createConstInt(cnt));
        }
    }
    return psMap;
}

static GBACSs *
fetchGBACSsData(int opNum, char* acsName, int type)
{
    GBACSs *acs = makeGBACSs();
    char *qName = getStringOption(OPTION_UPDATE_PS_QUERY_NAME);
    StringInfo meta = makeStringInfo();
    appendStringInfo(meta, "SELECT * FROM %s_op_%s_%s; ", qName, gprom_itoa(opNum), acsName);

    Relation *rel = postgresExecuteQuery(meta->data) ;
    if (type == 3) {
        FOREACH_VEC(Vector, v, rel->tuples) {
            char *groupby = (char *) getVecString(v, 0);
            double avg = atof((char *) getVecString(v, 1));
            double sum = atof((char *) getVecString(v, 2));
            gprom_long_t cnt = atol((char *) getVecString(v, 3));
            Vector *vals = makeVector(VECTOR_NODE, T_Vector);
            vecAppendNode(vals, (Node *) createConstFloat(avg));
            vecAppendNode(vals, (Node *) createConstFloat(sum));
            vecAppendNode(vals, (Node *) createConstLong(cnt));
            addToMap(acs->map, (Node *) createConstString(groupby), (Node *) vals);
        }
    } else if (type == 2) {
        FOREACH_VEC(Vector, v, rel->tuples) {
            char *groupby = (char *) getVecString(v, 0);
            double sum = atof((char *) getVecString(v, 1));
            gprom_long_t cnt = atol((char *) getVecString(v, 2));
            Vector *vals = makeVector(VECTOR_NODE, T_Vector);
            vecAppendNode(vals, (Node *) createConstFloat(sum));
            vecAppendNode(vals, (Node *) createConstLong(cnt));
            addToMap(acs->map, (Node *) createConstString(groupby), (Node *) vals);
        }
    } else if (type == 1) {
        FOREACH_VEC(Vector, v, rel->tuples) {
            char *groupby = (char *) getVecString(v, 0);
            gprom_long_t cnt = atol((char *) getVecString(v, 1));
            Vector *vals = makeVector(VECTOR_NODE, T_Vector);
            vecAppendNode(vals, (Node *) createConstLong(cnt));
            addToMap(acs->map, (Node *) createConstString(groupby), (Node *) vals);
        }
    }

    return acs;
}

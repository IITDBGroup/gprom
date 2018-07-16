/*-----------------------------------------------------------------------------
 *
 * translator_oracle.c
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
#include "mem_manager/mem_mgr.h"
#include "analysis_and_translate/translator_oracle.h"
#include "analysis_and_translate/analyzer.h"
#include "analysis_and_translate/parameter.h"
#include "analysis_and_translate/translate_update.h"
#include "instrumentation/timing_instrumentation.h"
#include "metadata_lookup/metadata_lookup.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/operator_property.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "model/set/hashmap.h"
#include "parser/parser.h"
#include "provenance_rewriter/prov_utility.h"
#include "utility/string_utils.h"

// data types
typedef struct ReplaceGroupByState {
    List *expressions;
    List *attrNames;
    int attrOffset;
} ReplaceGroupByState;


static List *attrsOffsetsList = NIL;

#define PROP_PROJ_RENAMED_ATTRS "RenamedProjAttrs"

// function declarations
static Node *translateGeneral(Node *node);
//static Node *translateSummary(Node *input, Node *node);
static void adaptSchemaFromChildren(QueryOperator *o);

/* Three branches of translating a Query */
static QueryOperator *translateSetQuery(SetQuery *sq);
static QueryOperator *translateQueryBlock(QueryBlock *qb);
static QueryOperator *translateProvenanceStmt(ProvenanceStmt *prov);
static void markTableAccessForRowidProv (QueryOperator *o);
static void getAffectedTableAndOperationType (Node *stmt,
        ReenactUpdateType *stmtType, char **tableName, Node **updateCond);
static void translateProperties(QueryOperator *q, List *properties);
static QueryOperator *translateWithStmt(WithStmt *with);

/* Functions of translating from clause in a QueryBlock */
static QueryOperator *translateFromClause(List *fromClause);
static QueryOperator *buildJoinTreeFromOperatorList(List *opList);
static List *translateFromClauseToOperatorList(List *fromClause);
//static void addPrefixToAttrNames (List *str, char *prefix);
static List *getAttrsOffsets(List *fromClause);
static inline QueryOperator *createTableAccessOpFromFromTableRef(
        FromTableRef *ftr);
static QueryOperator *translateFromJoinExpr(FromJoinExpr *fje);
static QueryOperator *translateFromSubquery(FromSubquery *fsq);
static QueryOperator *translateFromJsonTable(FromJsonTable *fjt);
static QueryOperator *translateFromProvInfo(QueryOperator *op, FromItem *f);

/* Functions of translating nested subquery in a QueryBlock */
static QueryOperator *translateNestedSubquery(QueryBlock *qb,
        QueryOperator *joinTreeRoot, List *attrsOffsets);
extern boolean findNestedSubqueries(Node *node, List **state);
static List *getListOfNestedSubqueries(QueryBlock *qb);
static void replaceAllNestedSubqueriesWithAuxExprs(QueryBlock *qb, HashMap *qToAttr);
static Node *replaceNestedSubqueryWithAuxExpr(Node *node, HashMap *qToAttr);

/* Functions of translating where clause in a QueryBlock */
static QueryOperator *translateWhereClause(Node *whereClause,
        QueryOperator *nestingOp, List *attrsOffsets);
static boolean visitAttrRefToSetNewAttrPos(Node *n, List *state);

/* Functions of translating simple select clause in a QueryBlock */
static QueryOperator *translateSelectClause(List *selectClause,
        QueryOperator *select, List *attrsOffsets, boolean hasAgg);
static QueryOperator *translateDistinct(DistinctClause *distinctClause,
        QueryOperator *input);

/* Functions of translating aggregations, having and group by */
static QueryOperator *translateHavingClause(Node *havingClause,
        QueryOperator *input, List *attrsOffsets);
static QueryOperator *translateAggregation(QueryBlock *qb, QueryOperator *input,
        List *attrsOffsets);
static QueryOperator *translateWindowFuncs(QueryBlock *qb, QueryOperator *input,
        List *attrsOffsets);

static QueryOperator *translateOrderBy(QueryBlock *qb, QueryOperator *input,
        List *attrsOffsets);

/* helpers */
static Node *replaceAggsAndGroupByMutator(Node *node,
        ReplaceGroupByState *state);
static QueryOperator *createProjectionOverNonAttrRefExprs(List **selectClause,
        Node **havingClause, List **groupByClause, QueryOperator *input,
        List *attrsOffsets);
static List *getListOfNonAttrRefExprs(List *selectClause, Node *havingClause,
        List *groupByClause);
static List *getListOfAggregFunctionCalls(List *selectClause,
        Node *havingClause);
static boolean visitAggregFunctionCall(Node *n, List **aggregs);
static boolean visitFindWindowFuncs(Node *n, List **wfs);
static boolean replaceWithViewRefsMutator(Node *node, List *views);

static boolean visitAttrRefToSetNewAttrPosList(Node *n, List *offsetsList);

static char *summaryType = NULL;
static Node *prop = NULL;


Node *
translateParseOracle (Node *q)
{
    Node *result;

    INFO_NODE_BEATIFY_LOG("translate QB model", q);

    result = translateGeneral(q);

    DEBUG_NODE_BEATIFY_LOG("result of translation is:", result);
    INFO_OP_LOG("result of translation overview is", result);
    ASSERT(equal(result, copyObject(result)));

    return result;
}



QueryOperator *
translateQueryOracle (Node *node)
{
    DEBUG_LOG("translate query <%s>", nodeToString(node));

    switch(node->type)
    {
        case T_QueryBlock:
            return translateQueryBlock((QueryBlock *) node);
        case T_SetQuery:
            return translateSetQuery((SetQuery *) node);
        case T_ProvenanceStmt:
            return translateProvenanceStmt((ProvenanceStmt *) node);
        case T_Insert:
        case T_Update:
        case T_Delete:
            return translateUpdate(node);
        case T_WithStmt:
            return translateWithStmt((WithStmt *) node);
        case T_CreateTable:
            return translateCreateTable((CreateTable *) node);
        case T_AlterTable:
            return translateAlterTable((AlterTable *) node);
        default:
            ASSERT(FALSE);
            return NULL;
    }
}

static Node *
translateGeneral (Node *node)
{
    Node *result;
    QueryOperator *r;

    if (isA(node, List))
    {
        result = (Node *) copyList((List *) node);
        FOREACH(Node,stmt,(List *) result)
        {
            if (isA(stmt, ProvenanceStmt))
            {
                ProvenanceStmt *prov = (ProvenanceStmt *) stmt;

                FOREACH(Node,n,prov->sumOpts)
                {
                    if(isA(n,List))
                    {
                        List *sumOpts = (List *) n;

                        FOREACH(KeyValue,sn,sumOpts)
                        if(streq(STRING_VALUE(sn->key),"sumtype"))
                            summaryType = STRING_VALUE(sn->value);
                    }
                }

                if(summaryType == NULL)
                    stmt_his_cell->data.ptr_value = (Node *) translateQueryOracle(stmt);
                else
                {
                    r = translateQueryOracle(stmt);
                    r->properties = copyObject(prop);
                    stmt_his_cell->data.ptr_value = (Node *) r;
                }
            }
            else
            {
                stmt_his_cell->data.ptr_value = (Node *) translateQueryOracle(stmt);
            }
        }
    }
    else
    {
        if (isA(node, ProvenanceStmt))
        {
            ProvenanceStmt *prov = (ProvenanceStmt *) node;

            FOREACH(Node,n,prov->sumOpts)
            {
                if(isA(n,List))
                {
                    List *sumOpts = (List *) n;

                    FOREACH(KeyValue,sn,sumOpts)
                    if(streq(STRING_VALUE(sn->key),"sumtype"))
                        summaryType = STRING_VALUE(sn->value);
                }
            }

            if(summaryType == NULL)
                result = (Node *) translateQueryOracle(node);
            else
            {
                r = translateQueryOracle(node);
                r->properties = copyObject(prop);
                result = (Node *) r;
            }
        }
        else
        {
            result = (Node *) translateQueryOracle(node);
        }
    }

    Set *done = PSET();
    disambiguiteAttrNames(result, done);
	
    return result;
}


boolean
disambiguiteAttrNames(Node *node, Set *done)
{
    QueryOperator *op;
    boolean changed = FALSE;

    if (hasSetElem(done, node))
        return FALSE;

    if (isA(node, List))
    {
        FOREACH(QueryOperator,q,(List *) node)
        {
            changed |= disambiguiteAttrNames((Node *) q, done);
        }
        return changed;
    }

    op = (QueryOperator *) node;

    FOREACH(Node,child,op->inputs)
    {
        changed |= disambiguiteAttrNames(child, done);
    }

    // if children have changed we need to fix operator's attributes
    if (changed)
    {
        DEBUG_OP_LOG("child operator's schema has changed", op);
        // first adapt attribute references
        List *attrRefs = getAttrRefsInOperator(op);
        //TODO keep track of what attributes are renamed by a projection and store this to not override renaming
        if (isA(op, ProjectionOperator))
        {
            ProjectionOperator *p = (ProjectionOperator *) op;
            Set *renamedAttrs = STRSET();

            // find renamed attributes
            FORBOTH(Node,projExpr,attr,p->projExprs, op->schema->attrDefs)
            {
                AttributeReference *aRef;
                AttributeDef *aDef = (AttributeDef *) attr;

                // only consider the case A AS B
                if(isA(projExpr, AttributeReference))
                {
                    aRef = (AttributeReference *) projExpr;
                    if (!streq(aRef->name, aDef->attrName))
                       addToSet(renamedAttrs, strdup(aDef->attrName));
                }
            }
            SET_STRING_PROP(op, PROP_PROJ_RENAMED_ATTRS, renamedAttrs);
        }

        FOREACH(AttributeReference,a,attrRefs)
        {
            QueryOperator *child;
            AttributeDef *childA;
            int input = a->fromClauseItem;
            int attrPos = a->attrPosition;

            child = (QueryOperator *) getNthOfListP(op->inputs, input);
            childA = getAttrDefByPos(child, attrPos);
            if (!strpeq(a->name,childA->attrName))
            {
                a->name = strdup(childA->attrName);
            }
        }
        // adapt schema based on changed attributes
        adaptSchemaFromChildren(op);
    }

    //TODO What other ops to consider
    if (isA(node,JoinOperator) || isA(node,ProjectionOperator))
    {
        if(!checkUniqueAttrNames(op))
        {
            makeAttrNamesUnique(op);
            changed = TRUE;
            DEBUG_OP_LOG("join or projection attributes are not unique", op);
        }
    }

    addToSet(done, node);
    return changed;
}

static void
adaptSchemaFromChildren(QueryOperator *o)
{
    switch(o->type)
    {
        case T_SelectionOperator:
        case T_SetOperator:
        case T_DuplicateRemoval:
        case T_OrderOperator:
        {
            o->schema->attrDefs = copyObject(OP_LCHILD(o)->schema->attrDefs);
        }
        break;
        case T_ProjectionOperator: //TODO do not rename attribute if this is already a rename
        {
            ProjectionOperator *p = (ProjectionOperator *) o;
            Set *renamedAttrs = (Set *) GET_STRING_PROP(o, PROP_PROJ_RENAMED_ATTRS);

            FORBOTH(Node,proj,a,p->projExprs,o->schema->attrDefs)
            {
                AttributeDef *aDef = (AttributeDef *) a;
                if (isA(proj,AttributeReference))
                {
                    AttributeReference *ref = (AttributeReference *) proj;
                    if (!strpeq(aDef->attrName, ref->name) && !hasSetElem(renamedAttrs, aDef->attrName))
                    {
                        aDef->attrName = strdup(ref->name);
                    }
                }
            }
        }
        break;
        case T_JoinOperator:
        {
            List *lAttrs = copyObject(OP_LCHILD(o)->schema->attrDefs);
            List *rAttrs = copyObject(OP_RCHILD(o)->schema->attrDefs);
            o->schema->attrDefs = CONCAT_LISTS(lAttrs, rAttrs);
        }
        break;
        case T_NestingOperator:
        case T_WindowOperator:
        {
            Node *lastOne = getTailOfListP(o->schema->attrDefs);
            o->schema->attrDefs = copyObject(OP_LCHILD(o)->schema->attrDefs);
            o->schema->attrDefs = appendToTailOfList(o->schema->attrDefs, lastOne);
        }
        break;
        case T_JsonTableOperator:
        {
            List *childAttr = OP_LCHILD(o)->schema->attrDefs;
            FORBOTH(AttributeDef,a,childA,o->schema->attrDefs,childAttr)
            {
                if (!strpeq(a->attrName, childA->attrName))
                {
                    a->attrName = strdup(childA->attrName);
                }
            }
        }
        break;
        case T_ProvenanceComputation:
        {
            //TODO should never end up here?
        }
        break;
        default:
            break;
    }
}

static QueryOperator *
translateSetQuery(SetQuery *sq)
{
    QueryOperator *left = NULL;
    QueryOperator *right = NULL;
    QueryOperator *result = NULL;

    DEBUG_LOG("translate set query");

    if (sq->lChild)
        left = translateQueryOracle(sq->lChild);
    if (sq->rChild)
        right = translateQueryOracle(sq->rChild);
    ASSERT(left && right);

    // set children of the set operator node
    List *inputs = LIST_MAKE(left, right);

    // create set operator node
    SetOperator *so = createSetOperator(sq->setOp, inputs, NIL,
            sq->selectClause);

    // set the parent of the operator's children
    OP_LCHILD(so)->parents = OP_RCHILD(so)->parents = singleton(so);

    //if not "all" then add duplicate removal operators
    if (!sq->all)
    {
        switch(sq->setOp)
        {
            case SETOP_UNION:
            case SETOP_INTERSECTION:

                result = (QueryOperator *) createDuplicateRemovalOp(
                        getNormalAttrProjectionExprs((QueryOperator *) so),
                        (QueryOperator *) so,
                        NIL, getAttrNames(GET_OPSCHEMA(so)));
                ((QueryOperator *) so)->parents = singleton(result);
                break;
            case SETOP_DIFFERENCE:
                {
                    QueryOperator *lD, *rD;

                    lD = (QueryOperator *) createDuplicateRemovalOp(
                            getNormalAttrProjectionExprs(left),
                            (QueryOperator *) left,
                            NIL, getAttrNames(GET_OPSCHEMA(left)));

                    rD = (QueryOperator *) createDuplicateRemovalOp(
                            getNormalAttrProjectionExprs(right),
                            (QueryOperator *) right,
                            NIL, getAttrNames(GET_OPSCHEMA(right)));
                    switchSubtrees(left, lD);
                    switchSubtrees(right, rD);
                    left->parents = singleton(lD);
                    right->parents = singleton(rD);

                    result = (QueryOperator *) so;
                }
                break;
        }
    }
    // is "all"
    else
        result = (QueryOperator *) so;

    DEBUG_LOG("translated set query is %s", operatorToOverviewString((Node *) result));

    return result;
}

#define LOG_TRANSLATED_OP(message,op) \
    do { \
        DEBUG_NODE_BEATIFY_LOG(message, op); \
        INFO_OP_LOG(message, op); \
    } while (0)

static QueryOperator *
translateQueryBlock(QueryBlock *qb)
{
    List *attrsOffsets = NIL;
    boolean hasAggOrGroupBy = FALSE;
    boolean hasWindowFuncs = FALSE;

    DEBUG_NODE_BEATIFY_LOG("translate a QB:", qb);

    QueryOperator *joinTreeRoot = translateFromClause(qb->fromClause);
    LOG_TRANSLATED_OP("translatedFrom is", joinTreeRoot);
    attrsOffsets = getAttrsOffsets(qb->fromClause);
    attrsOffsetsList = appendToHeadOfList(attrsOffsetsList, attrsOffsets);

    // adapt attribute references to match new from clause root's schema
    visitAttrRefToSetNewAttrPos((Node *) qb->selectClause, attrsOffsets);
    visitAttrRefToSetNewAttrPos((Node *) qb->havingClause, attrsOffsets);
    visitAttrRefToSetNewAttrPos((Node *) qb->groupByClause, attrsOffsets);

    // translate remaining clauses
    QueryOperator *nestingOp = translateNestedSubquery(qb, joinTreeRoot,
            attrsOffsets);
    if (nestingOp != joinTreeRoot)
        LOG_TRANSLATED_OP("translatedNesting is", nestingOp);

    //DEBUG_NODE_BEATIFY_LOG("Before attrsOffsetsList: ", attrsOffsetsList);
    QueryOperator *select = translateWhereClause(qb->whereClause, nestingOp,
            attrsOffsetsList);
    if (select != nestingOp)
        LOG_TRANSLATED_OP("translatedWhere is", select);
    //attrsOffsetsList = removeFromHead(attrsOffsetsList);
    //DEBUG_NODE_BEATIFY_LOG("After attrsOffsetsList: ", attrsOffsetsList);

    QueryOperator *aggr = translateAggregation(qb, select, attrsOffsets);
    hasAggOrGroupBy = (aggr != select);
    if (hasAggOrGroupBy)
        LOG_TRANSLATED_OP("translatedAggregation is", aggr);

    QueryOperator *wind = translateWindowFuncs(qb, aggr, attrsOffsets);
    hasWindowFuncs = (wind != aggr);
    if (hasWindowFuncs)
        LOG_TRANSLATED_OP("translatedWindowFuncs is", wind);

    if (hasAggOrGroupBy && hasWindowFuncs)
        FATAL_LOG("Cannot have both window functions and aggregation/group by "
                "in same query block:\n\n%s", beatify(nodeToString(qb)));

    QueryOperator *having = translateHavingClause(qb->havingClause, wind,
            attrsOffsets);
    if (having != aggr)
        LOG_TRANSLATED_OP("translatedHaving is", having);

    QueryOperator *project = translateSelectClause(qb->selectClause, having,
            attrsOffsets, hasAggOrGroupBy);
    LOG_TRANSLATED_OP("translatedSelect is", project);

    QueryOperator *distinct = translateDistinct((DistinctClause *) qb->distinct,
            project);
    if (distinct != project)
        LOG_TRANSLATED_OP("translatedDistinct is", distinct);

    QueryOperator *orderBy = translateOrderBy(qb, distinct, attrsOffsets);
    if (orderBy != distinct)
        LOG_TRANSLATED_OP("translatedOrder is", orderBy);

    if(summaryType != NULL)
    	prop = (Node *) orderBy;

    return orderBy;
}

static QueryOperator *
translateProvenanceStmt(ProvenanceStmt *prov)
{
    QueryOperator *child;
    ProvenanceComputation *result;

    //get type from options
    result = createProvenanceComputOp(prov->provType, NIL, NIL,
            prov->selectClause, prov->dts, NULL);
    result->inputType = prov->inputType;
    result->asOf = copyObject(prov->asOf);
    translateProperties(((QueryOperator *) result), prov->options);

    switch (prov->inputType)
    {
        case PROV_INPUT_TRANSACTION:
        {
            //XID ?
            char *xid = STRING_VALUE(prov->query);
            List *scns = NIL;
            List *sqls = NIL;
            List *sqlBinds = NIL;
            IsolationLevel isoLevel;
            List *updateConds = NIL;
            Constant *commitSCN = createConstLong(-1L);
            boolean showIntermediate = HAS_STRING_PROP(result,PROP_PC_SHOW_INTERMEDIATE);
            boolean useRowidScn = HAS_STRING_PROP(result,PROP_PC_TUPLE_VERSIONS);
            List *noProv = NIL;
            int i = 0;

            DEBUG_LOG("Provenance for transaction");

            // set XID
            SET_STRING_PROP(result, PROP_PC_TRANS_XID, createConstString(strdup(xid)));

            // call metadata lookup -> SCNS + SQLS
            getTransactionSQLAndSCNs(xid, &scns, &sqls, &sqlBinds, &isoLevel, commitSCN);

            // set provenance transaction info
            ProvenanceTransactionInfo *tInfo = makeNode(ProvenanceTransactionInfo);

            result->transactionInfo = tInfo;
            //      tInfo->originalUpdates = copyObject(&sqls);
            tInfo->updateTableNames = NIL;
            tInfo->scns = scns;
            tInfo->transIsolation = isoLevel;
            tInfo->originalUpdates = NIL;

            // call parser and analyser and translate nodes
            FOREACH(char,sql,sqls)
            {
                Node *node;
                char *bindString;
                List *bindVals;

                node = parseFromString(sql);

                START_TIMER("translation - transaction - analyze update");
                analyzeParseModel(node);
                STOP_TIMER("translation - transaction - analyze update");
                node = getNthOfListP((List *) node, 0);

                START_TIMER("translation - transaction - analyze binds");
                bindString = getNthOfListP(sqlBinds, i);
                if (bindString != NULL)
                {
                    DEBUG_LOG("set parameters\n%s\nfor sql\n%s", nodeToString(node), bindString);
                    bindVals = oracleBindToConsts(bindString);
                    node = setParameterValues(node, bindVals);
                }
                STOP_TIMER("translation - transaction - analyze binds");

                /* get table name and other information about the statement */
                char *tableName = NULL;
                ReenactUpdateType updateType = UPDATE_TYPE_DELETE;
                Node *updateCond = NULL;

                getAffectedTableAndOperationType(node, &updateType, &tableName, &updateCond);

                DEBUG_NODE_BEATIFY_LOG("result of update translation is", node);

                // store in transaction info
                //TODO ok to do that?
//                if (updateCond != NULL)
                    updateConds = appendToTailOfList(updateConds, copyObject(updateCond));

                tInfo->originalUpdates = appendToTailOfList(tInfo->originalUpdates, node);
                tInfo->updateTableNames = appendToTailOfList(
                        tInfo->updateTableNames, strdup(tableName));

                // translate and add update as child to provenance computation
                START_TIMER("translation - transaction - translate update");
                child = translateQueryOracle(node);
                STOP_TIMER("translation - transaction - translate update");

                // mark for showing intermediate results
                if (showIntermediate)
                {
                    SET_BOOL_STRING_PROP(child, PROP_SHOW_INTERMEDIATE_PROV);
                    SET_STRING_PROP(child, PROP_PROV_REL_NAME, createConstString(
                            CONCAT_STRINGS("U", gprom_itoa(i + 1), "_", strdup(tableName))));
                }

                // use ROWID + SCN as provenance, set provenance attributes for each table
                if (useRowidScn)
                {
                    markTableAccessForRowidProv(child);
                }

                // mark as root of translated update
                SET_BOOL_STRING_PROP(child, PROP_PROV_IS_UPDATE_ROOT);
                SET_STRING_PROP(child, PROP_PROV_ORIG_UPDATE_TYPE, createConstInt(updateType));
                DEBUG_NODE_BEATIFY_LOG("qo model of update for transaction is\n", child);

                noProv = appendToTailOfList(noProv, createConstBool(FALSE));

                addChildOperator((QueryOperator *) result, child);
                i++;
            }

            // get commit scn, some tables that were targeted by statements might not have been modified
            gprom_long_t commitScn = INVALID_SCN;

            FOREACH(char,tableName,tInfo->updateTableNames)
            {
                DEBUG_LOG("try to get commit SCN from table <%s>", tableName);
                commitScn = getCommitScn(tableName,
                        LONG_VALUE(getHeadOfListP(tInfo->scns)),
                        xid);
                if (commitScn != INVALID_SCN)
                    break;
            }

            if (commitScn == INVALID_SCN)
                FATAL_NODE_BEATIFY_LOG("unable to determine commit SCN for transaction", tInfo);

            tInfo->commitSCN = createConstLong(commitScn);

            DEBUG_LOG("ONLY UPDATED conditions: %s", nodeToString(updateConds));
            DEBUG_LOG("no prov: %s", nodeToString(noProv));
            SET_STRING_PROP(result, PROP_PC_UPDATE_COND, updateConds);
            SET_STRING_PROP(result, PROP_REENACT_NO_TRACK_LIST, noProv);

            DEBUG_LOG("constructed translated provenance computation for PROVENANCE OF TRANSACTION");
        }
        break;
        case PROV_INPUT_UPDATE_SEQUENCE:
        {
            ProvenanceTransactionInfo *tInfo = makeNode(ProvenanceTransactionInfo);

            result->transactionInfo = tInfo;
            tInfo->originalUpdates = copyObject(prov->query);
            tInfo->updateTableNames = NIL;
            tInfo->transIsolation = ISOLATION_SERIALIZABLE;
            tInfo->scns = NIL;

            FOREACH(Node,n,(List *) prov->query)
            {
                char *tableName = NULL;

                /* get table name */
                getAffectedTableAndOperationType(n, NULL, &tableName, NULL);
                tInfo->updateTableNames = appendToTailOfList(
                        tInfo->updateTableNames, strdup(tableName));
                tInfo->scns = appendToTailOfList(tInfo->scns, createConstLong(0)); //TODO get SCN

                // translate and add update as child to provenance computation
                child = translateQueryOracle(n);
                addChildOperator((QueryOperator *) result, child);
            }
        }
        break;
        case PROV_INPUT_UPDATE:
        case PROV_INPUT_QUERY:
        {
            child = translateQueryOracle(prov->query);
            addChildOperator((QueryOperator *) result, child);
        }
        break;
        case PROV_INPUT_TEMPORAL_QUERY:
        {
            DataType tempDT = getTailOfListInt(prov->dts);
            SET_STRING_PROP(result, PROP_TEMP_ATTR_DT, createConstInt(tempDT));
            child = translateQueryOracle(prov->query);
            addChildOperator((QueryOperator *) result, child);
        }
        break;
        case PROV_INPUT_UNCERTAIN_QUERY:
        {
            child = translateQueryOracle(prov->query);
            addChildOperator((QueryOperator *) result, child);
        }
        break;
        case PROV_INPUT_REENACT:
        case PROV_INPUT_REENACT_WITH_TIMES:
        {
            ProvenanceTransactionInfo *tInfo = makeNode(ProvenanceTransactionInfo);
            HashMap *tableToTranslation = NEW_MAP(Constant, QueryOperator);
            boolean isWithTimes = (prov->inputType == PROV_INPUT_REENACT_WITH_TIMES);
            boolean showIntermediate = HAS_STRING_PROP(result,PROP_PC_SHOW_INTERMEDIATE);
            boolean useRowidScn = HAS_STRING_PROP(result,PROP_PC_TUPLE_VERSIONS);
            boolean hasIsolevel = HAS_STRING_PROP(result,PROP_PC_ISOLATION_LEVEL);
            boolean hasCommitSCN = HAS_STRING_PROP(result,PROP_PC_COMMIT_SCN);
            List *updateConds = NIL;
            List *noProv = NIL;
            int i = 0, j = 0;

            // user has asked for provenance?
            if (HAS_STRING_PROP(result, PROP_PC_GEN_PROVENANCE))
            {
                result->provType = PROV_PI_CS;
            }

            if (hasCommitSCN)
                tInfo->commitSCN = (Constant *) GET_STRING_PROP(result, PROP_PC_COMMIT_SCN);

            if (hasIsolevel)
            {
                char *isoLevel = STRING_VALUE(GET_STRING_PROP(result,PROP_PC_ISOLATION_LEVEL));

                DEBUG_LOG("has isolevel %s", isoLevel);

                if (streq(strToUpper(isoLevel), "SERIALIZABLE"))
                    tInfo->transIsolation = ISOLATION_SERIALIZABLE;
                else if (streq(strToUpper(isoLevel), "READCOMMITTED"))
                    tInfo->transIsolation = ISOLATION_READ_COMMITTED;
                else
                    FATAL_LOG("isolation level has to be either SERIALIZABLE or READCOMMITTED not <%s>", isoLevel);
            }
            else
                tInfo->transIsolation = ISOLATION_SERIALIZABLE;

            if (tInfo->transIsolation == ISOLATION_READ_COMMITTED && prov->inputType == PROV_INPUT_REENACT)
                FATAL_LOG("isolation level READ COMMITTED requires an AS OF clause for each reenacted DML.");
            if (tInfo->transIsolation == ISOLATION_READ_COMMITTED && !hasCommitSCN)
                FATAL_LOG("isolation level READ COMMITTED requires a commit scn to be specified using WITH COMMIT SCN scn.");

            result->transactionInfo = tInfo;
            tInfo->originalUpdates = NIL;
            tInfo->updateTableNames = NIL;
            tInfo->scns = NIL;

            FOREACH(KeyValue,stmtWithOpts,(List *) prov->query)
            {
                char *tableName = NULL;
                Node *n = stmtWithOpts->key;
                List *opts = (List *) stmtWithOpts->value;
                HashMap *optMap = NEW_MAP(Constant,Node);
                boolean isNoProv;

                // convert options into hashmap
                FOREACH(KeyValue,opt,opts)
                {
                    addToMap(optMap, opt->key, opt->value);
                }
                isNoProv = MAP_HAS_STRING_KEY(optMap, PROP_REENACT_DO_NOT_TRACK_PROV);
                noProv = appendToTailOfList(noProv, createConstBool(isNoProv));

                ReenactUpdateType stmtType;
                Node *cond = NULL;

                /* get table name and other info */
                getAffectedTableAndOperationType(n, &stmtType, &tableName, &cond);

                // store info
                tInfo->updateTableNames = appendToTailOfList(
                        tInfo->updateTableNames, strdup(tableName));

                if (isWithTimes)
                {
                    tInfo->scns = appendToTailOfList(tInfo->scns, MAP_GET_STRING(optMap, PROP_REENACT_ASOF));
                }

                updateConds = appendToTailOfList(updateConds, copyObject(cond));

                tInfo->originalUpdates = appendToTailOfList(tInfo->originalUpdates, n);

                // translate and add update as child to provenance computation
                child = translateQueryOracle(n);
                MAP_ADD_STRING_KEY(tableToTranslation, tableName, child);

                addChildOperator((QueryOperator *) result, child);

                // mark for showing intermediate results
                if (showIntermediate && !isNoProv)
                {
                    SET_BOOL_STRING_PROP(child, PROP_SHOW_INTERMEDIATE_PROV);
                    SET_STRING_PROP(child, PROP_PROV_REL_NAME, createConstString(
                            CONCAT_STRINGS("U", gprom_itoa(j + 1), "_", strdup(tableName))));
                    j++;
                }

                // use ROWID + SCN as provenance, set provenance attributes for each table
                if (useRowidScn)
                {
                    markTableAccessForRowidProv(child);
                }

                // mark as root of translated update
                SET_BOOL_STRING_PROP(child, PROP_PROV_IS_UPDATE_ROOT);
                if (isNoProv)
                    SET_BOOL_STRING_PROP(child, PROP_REENACT_DO_NOT_TRACK_PROV);
                SET_STRING_PROP(child, PROP_PROV_ORIG_UPDATE_TYPE, createConstInt(stmtType));
                DEBUG_NODE_BEATIFY_LOG("qo model of update for transaction is\n", child);

                i++;

                //TODO
            }
            //TODO check that no prov statements are a prefix of all the statements updating a table
            DEBUG_LOG("ONLY UPDATED conditions: %s", nodeToString(updateConds));
            DEBUG_LOG("no prov: %s", nodeToString(noProv));
            SET_STRING_PROP(result, PROP_PC_UPDATE_COND, updateConds);
            SET_STRING_PROP(result, PROP_REENACT_NO_TRACK_LIST, noProv);
            break;
        }
    }
    return (QueryOperator *) result;
}

static void
markTableAccessForRowidProv (QueryOperator *o)
{
    List *tables = NIL;
    findTableAccessVisitor((Node *) o, &tables);

    FOREACH(TableAccessOperator,t,tables)
    {
        if (HAS_STRING_PROP(t,PROP_TABLE_IS_UPDATED))
        {
            SET_BOOL_STRING_PROP(t,PROP_TABLE_USE_ROWID_VERSION);
            SET_BOOL_STRING_PROP(t,PROP_USE_PROVENANCE);
            SET_STRING_PROP(t,PROP_USER_PROV_ATTRS,
                    stringListToConstList(LIST_MAKE(
                            strdup("ROWID"),
                            strdup("VERSIONS_STARTSCN"))));

            t->op.schema->attrDefs = appendToTailOfList(t->op.schema->attrDefs,
                    createAttributeDef("ROWID", DT_STRING));
            t->op.schema->attrDefs = appendToTailOfList(t->op.schema->attrDefs,
                    createAttributeDef("VERSIONS_STARTSCN", DT_LONG));
        }
    }
}

/**
 * determines a stmts update type, affected table, and (if the stmt is an UPDATE) it's condition
 * the last three arguments are use to store the result.
 */
static void
getAffectedTableAndOperationType (Node *stmt, ReenactUpdateType *stmtType, char **tableName, Node **updateCond)
{
    Node *cond = NULL;
    ReenactUpdateType operType;
    char *tName = NULL;

    switch (stmt->type) {
        case T_Insert:
        {
            Insert *i = (Insert *) stmt;
            if (isA(i->query,  List))
                operType = UPDATE_TYPE_INSERT_VALUES;
            else
                operType = UPDATE_TYPE_INSERT_QUERY;
            tName = i->insertTableName;
        }
        break;
        case T_Update:
        {
            operType = UPDATE_TYPE_UPDATE;
            Update *up = (Update *) stmt;

            tName = up->updateTableName;
            cond = copyObject(up->cond);
        }
        break;
        case T_Delete:
            operType = UPDATE_TYPE_DELETE;
            tName = ((Delete *) stmt)->deleteTableName;
            break;
        case T_QueryBlock:
        case T_SetQuery:
            tName = strdup("_NONE");
            operType = UPDATE_TYPE_QUERY;
            break;
        case T_AlterTable:
            tName = strdup(((AlterTable *) stmt)->tableName);
            operType = UPDATE_TYPE_DDL;
            break;
        case T_CreateTable:
            tName = strdup(((CreateTable *) stmt)->tableName);
            operType = UPDATE_TYPE_DDL;
            break;
        default:
            FATAL_LOG(
                    "Unexpected node type %u as input to provenance computation",
                    stmt->type);
            break;
    }

    if (stmtType)
        *stmtType = operType;
    if (tableName)
        *tableName = tName;
    if (updateCond)
        *updateCond = cond;
}


static void
translateProperties(QueryOperator *q, List *properties)
{
    FOREACH(KeyValue,p,properties)
    {
        ASSERT(isA(p,KeyValue));
        SET_KEYVAL_PROPERTY(q, ((KeyValue *) copyObject(p)));
    }
}

static QueryOperator *
translateWithStmt(WithStmt *with)
{
//    List *withViews = NIL;
    List *transWithViews = NIL;
    QueryOperator *finalQ;

    // translate each individual view
    FOREACH(KeyValue,v,with->withViews)
    {
        Node *vQ = v->value;
        Node *opQ;

        // translate current view into operator model
        opQ = translateGeneral(vQ);

        // replace references to withViews as table access  with definition
        replaceWithViewRefsMutator(opQ, transWithViews);

        // store as with view entry
        transWithViews = appendToTailOfList(transWithViews,
                createNodeKeyValue(copyObject(v->key), opQ));
        DEBUG_LOG("translated input views <%s>:\n\n%s\n\ninto\n\n%s",
                STRING_VALUE(v->key), nodeToString(vQ),
                operatorToOverviewString(opQ));
    }

    // adapt the query
    finalQ = (QueryOperator *) translateGeneral(with->query);
    replaceWithViewRefsMutator((Node *) finalQ, transWithViews);

    return finalQ;
}



static boolean
replaceWithViewRefsMutator(Node *node, List *views)
{
    if (node == NULL)
        return TRUE;

    // table references may be temporary views
    if (isA(node, TableAccessOperator))
    {
        TableAccessOperator *t = (TableAccessOperator *) node;
        char *name = t->tableName;

        // if table access represents a view access then replace it with the view query
        FOREACH(KeyValue,v,views)
        {
            char *vName = STRING_VALUE(v->key);

            if (strcmp(name, vName) == 0)
                switchSubtreeWithExisting((QueryOperator *) t,
                        (QueryOperator *) v->value);
        }

        return TRUE;
    }

    return visit(node, replaceWithViewRefsMutator, views);
}

static QueryOperator *
translateFromClause(List *fromClause)
{
    List *opList = translateFromClauseToOperatorList(fromClause);
    return buildJoinTreeFromOperatorList(opList);
}

static QueryOperator *
buildJoinTreeFromOperatorList(List *opList)
{
    int pos = 0;
    DEBUG_LOG("build join tree from operator list\n%s", nodeToString(opList));

    QueryOperator *root = (QueryOperator *) getHeadOfListP(opList);
    FOREACH(QueryOperator, op, opList)
    {
        if (op == (QueryOperator *) getHeadOfListP(opList))
            continue;

        if (isA(op,JsonTableOperator))
        {
            QueryOperator *oldRoot = (QueryOperator *) root;
            List *inputs = NIL;

            // set children of the JsonTable Operator
            inputs = appendToTailOfList(inputs, oldRoot);
            op->inputs = inputs;

            oldRoot->parents = singleton(op);

            // contact children's attribute names as the node's attribute
            // names
            List *attrNames = concatTwoLists(getAttrNames(oldRoot->schema), getAttrNames(op->schema));

            /* get data types from inputs and attribute names from parameter
             * to create schema */
            List *l1 = getDataTypes(oldRoot->schema);
            List *l2 = getDataTypes(op->schema);
            op->schema = createSchemaFromLists(op->schema->name, attrNames, concatTwoLists(l1, l2));

            root = op;
        }
        else
        {
            QueryOperator *oldRoot = (QueryOperator *) root;
            List *inputs = NIL;
            // set children of the join node
            inputs = appendToTailOfList(inputs, oldRoot);
            inputs = appendToTailOfList(inputs, op);
            List *lAttrs = getAttrNames(oldRoot->schema);
            List *rAttrs = getAttrNames(op->schema);

//            addPrefixToAttrNames(lAttrs, gprom_itoa(pos));
//            addPrefixToAttrNames(rAttrs, gprom_itoa(pos + 1));
            // contact children's attribute names as the node's attribute
            // names
            List *attrNames = concatTwoLists(lAttrs, rAttrs);

            // create join operator
            root = (QueryOperator *) createJoinOp(JOIN_CROSS, NULL, inputs, NIL, attrNames);

            // set the parent of the operator's children
            OP_LCHILD(root)->parents = singleton(root);
            OP_RCHILD(root)->parents = singleton(root);
        }
        pos++;
    }

    DEBUG_LOG("join tree for translated from is\n%s", nodeToString(root));

    return root;
}

//static void
//addPrefixToAttrNames (List *str, char *prefix)
//{
//    FOREACH_LC(lc, str)
//    {
//        char *s =  LC_STRING_VAL(lc);
//        LC_P_VAL(lc) = CONCAT_STRINGS(prefix,"_",s);
//    }
//}

static List *
getAttrsOffsets(List *fromClause)
{
//    int len = getListLength(fromClause);
    List *offsets = NIL;
    int curOffset = 0;
    FromProvInfo *fp;

    FOREACH(FromItem, from, fromClause)
    {
       int numAttrs;
       offsets = appendToTailOfListInt(offsets, curOffset);
       numAttrs = getListLength(from->attrNames);
       if (from->provInfo != NULL)
       {
           fp = from->provInfo;
           if(!fp->intermediateProv && !fp->baserel && fp->userProvAttrs)
           {
               numAttrs -= LIST_LENGTH(fp->userProvAttrs);
           }
       }
       curOffset += numAttrs;
    }

    DEBUG_LOG("attribute offsets for from clause items are %s",
            nodeToString(offsets));

    return offsets;
}

static List *
translateFromClauseToOperatorList(List *fromClause)
{
    List *opList = NIL;

    DEBUG_LOG("translate from clause");

    FOREACH(FromItem, from, fromClause)
    {
        QueryOperator *op = NULL;
        switch (from->type)
        {
            case T_FromTableRef:
                op = createTableAccessOpFromFromTableRef((FromTableRef *) from);
                break;
            case T_FromJoinExpr:
                op = translateFromJoinExpr((FromJoinExpr *) from);
                break;
            case T_FromSubquery:
                op = translateFromSubquery((FromSubquery *) from);
                break;
            case T_FromJsonTable:
            	op = translateFromJsonTable((FromJsonTable *) from);
            	break;
            default:
                FATAL_LOG("did not expect node <%s> in from list", nodeToString(from));
                break;
        }

        op = translateFromProvInfo(op, from);

        ASSERT(op);
        opList = appendToTailOfList(opList, op);
    }

    ASSERT(opList);
    DEBUG_LOG("translated from clause into list of operator trees is \n%s", beatify(nodeToString(opList)));
    return opList;
}

static QueryOperator *
translateFromProvInfo(QueryOperator *op, FromItem *f)
{
    FromProvInfo *from = f->provInfo;
    boolean hasProv = FALSE;

    if (from == NULL)
        return op;


    /* treat as base relation or show intermediate provenance? */
    if (from->intermediateProv)
        SET_BOOL_STRING_PROP(op,PROP_SHOW_INTERMEDIATE_PROV);
    else if (from->baserel)
        SET_BOOL_STRING_PROP(op,PROP_USE_PROVENANCE);
    else
    {
        SET_BOOL_STRING_PROP(op,PROP_HAS_PROVENANCE);
        hasProv = TRUE;
    }

    /* user provided provenance attributes or all attributes of subquery? */
    if (from->userProvAttrs != NIL)
        setStringProperty(op, PROP_USER_PROV_ATTRS, (Node *) stringListToConstList(from->userProvAttrs));
    else
        setStringProperty(op, PROP_USER_PROV_ATTRS, (Node *) stringListToConstList(getQueryOperatorAttrNames(op)));

    /* user TIP attribute selected */
	if (getStringProvProperty(from, PROV_PROP_TIP_ATTR) != NULL)
	{
		setStringProperty(op, PROP_TIP_ATTR, (Node *) createConstString(STRING_VALUE(getStringProvProperty(from, PROV_PROP_TIP_ATTR))));
		hasProv = TRUE;
		from->userProvAttrs = singleton(strdup(STRING_VALUE(getStringProvProperty(from, PROV_PROP_TIP_ATTR))));
	}

    /* table selected as incomplete */
    if (from->provProperties)
	{
		if (getStringProvProperty(from, PROV_PROP_INCOMPLETE_TABLE))
		{
			setStringProperty(op, PROV_PROP_INCOMPLETE_TABLE, (Node *) createConstBool(1));
		}
	}

    /* set name for op */
    setStringProperty(op, PROP_PROV_REL_NAME, (Node *) createConstString(f->name));


    if (hasProv)
    {
        ProjectionOperator *p;
        List *attrs, *newAttrs;
        List *provAttrs = from->userProvAttrs;
        List *provPos = NIL;

        attrs = getQueryOperatorAttrNames(op);
        newAttrs = NIL;

        FOREACH(char,name,attrs)
        {
            if(!searchListString(provAttrs, name))
            {
                newAttrs = appendToTailOfList(newAttrs, strdup(name));
            }
            else
            {
                provPos = appendToTailOfListInt(provPos, getAttrPos(op, name));
            }
        }
        p = (ProjectionOperator *) createProjOnAttrsByName(op, newAttrs, NIL);
        op->provAttrs = provPos;
        // mark as dummy projection so it can be excluded from rewrite
        SET_BOOL_STRING_PROP(p, PROP_DUMMY_HAS_PROV_PROJ);
        addChildOperator((QueryOperator *) p,op);
        return (QueryOperator *) p;
    }

    return op;
}

static inline QueryOperator *
createTableAccessOpFromFromTableRef(FromTableRef *ftr)
{
    TableAccessOperator *ta = createTableAccessOp(ftr->tableId, NULL,ftr->from.name,
                NIL, ftr->from.attrNames, ftr->from.dataTypes); // TODO  get data types
    DEBUG_LOG("translated table access:\n%s\nINTO\n%s", nodeToString(ftr), nodeToString(ta));
    return ((QueryOperator *) ta);
}

static QueryOperator *
translateFromJoinExpr(FromJoinExpr *fje)
{
    QueryOperator *input1 = NULL;
    QueryOperator *input2 = NULL;
    Node *joinCond = NULL;
    List *commonAttrs = NIL;
    List *uniqueRightAttrs = NIL;
    List *attrNames = fje->from.attrNames;

    switch (fje->left->type)
    {
        case T_FromTableRef:
            input1 = createTableAccessOpFromFromTableRef(
                    (FromTableRef *) fje->left);
            break;
        case T_FromJoinExpr:
            input1 = translateFromJoinExpr((FromJoinExpr *) fje->left);
            break;
        case T_FromSubquery:
            input1 = translateFromSubquery((FromSubquery *) fje->left);
            break;
        default:
            FATAL_LOG("did not expect node <%s> in from list", nodeToString(input1));
            break;
    }
    input1 = translateFromProvInfo(input1, fje->left);
    switch (fje->right->type)
    {
        case T_FromTableRef:
            input2 = createTableAccessOpFromFromTableRef(
                    (FromTableRef *) fje->right);
            break;
        case T_FromJoinExpr:
            input2 = translateFromJoinExpr((FromJoinExpr *) fje->right);
            break;
        case T_FromSubquery:
            input2 = translateFromSubquery((FromSubquery *) fje->right);
            break;
        default:
            FATAL_LOG("did not expect node <%s> in from list", nodeToString(input2));
            break;
    }
    input2 = translateFromProvInfo(input2, fje->right);
    ASSERT(input1 && input2);

    // set children of the join operator node
    List *inputs = LIST_MAKE(input1, input2);

    // NATURAL join condition, create equality condition for all common attributes
    if (fje->joinCond == JOIN_COND_NATURAL)
    {
        List *leftAttrs = getQueryOperatorAttrNames(input1);
        List *opDefs = input1->schema->attrDefs;
        List *rightAttrs = getQueryOperatorAttrNames(input2);
//        List *commonAttRefs = NIL;
        int lPos = 0;

        // search for common attributes and create condition for equality comparisons
        FOREACH(char,rA,rightAttrs)
        {
            int rPos = listPosString(leftAttrs, rA);
            if(rPos != -1)
            {
                AttributeDef *lDef = (AttributeDef *) getNthOfListP(opDefs, lPos);
                AttributeReference *lRef = createFullAttrReference(strdup(rA), 0, lPos, 0, lDef->dataType);
                AttributeReference *rRef = createFullAttrReference(strdup(rA), 1, rPos, 0, lDef->dataType);

                commonAttrs = appendToTailOfList(commonAttrs, rA);
                joinCond = AND_EXPRS((Node *) createOpExpr("=", LIST_MAKE(lRef,rRef)), joinCond);
            }
            else
                uniqueRightAttrs = appendToTailOfList(uniqueRightAttrs, rA);
            lPos++;
        }

        DEBUG_LOG("common attributes for natural join <%s>, unique right "
                "attrs <%s>, with left <%s> and right <%s>",
                stringListToString(commonAttrs),
                stringListToString(uniqueRightAttrs),
                stringListToString(leftAttrs),
                stringListToString(rightAttrs));

        // need to update attribute names for join result
        attrNames = concatTwoLists(leftAttrs, rightAttrs);
    }
    // USING (a1, an) join create condition as l.a1 = r.a1 AND ... AND l.an = r.an
    else if (fje->joinCond == JOIN_COND_USING)
    {
        Node *curCond = NULL;

        FOREACH(char,a,(List *)fje->cond)
        {
            Node *attrCond;
            AttributeReference *lA, *rA;
            int aPos = 0;
            boolean found = FALSE;

            lA = createAttributeReference(a);
            lA->fromClauseItem = 0;
            rA = createAttributeReference(a);
            rA->fromClauseItem = 1;

            FOREACH(AttributeDef,a1,input1->schema->attrDefs)
            {
                if (strcmp(a,a1->attrName) == 0)
                {
                    if (found)
                        FATAL_LOG("USING join is using ambiguous attribute"
                                " references <%s>", a);
                    else
                        lA->attrPosition = aPos;
                }
                aPos++;
            }

            aPos = 0;
            FOREACH(AttributeDef,a2,input2->schema->attrDefs)
            {
                if (strcmp(a,a2->attrName) == 0)
                {
                    if (found)
                        FATAL_LOG("USING join is using ambiguous attribute"
                                " references <%s>", a);
                    else
                        rA->attrPosition = aPos;
                }
                aPos++;
            }

            // create equality condition and update global condition
            attrCond = (Node *) createOpExpr("=",LIST_MAKE(lA,rA));
            curCond = AND_EXPRS(attrCond,curCond);
        }

        joinCond = curCond;
    }
    // inner join
    else
        joinCond = fje->cond;

    // create join operator node
    JoinOperator *jo = createJoinOp(fje->joinType, joinCond, inputs, NIL,
            attrNames);

    // set the parent of the operator's children
    OP_LCHILD(jo)->parents = OP_RCHILD(jo)->parents = singleton(jo);

    // create projection for natural join
    if (fje->joinCond == JOIN_COND_NATURAL)
    {
        ProjectionOperator *op;
        List *projExpr = NIL;
        int pos = 0;

        FOREACH(AttributeDef,a,input1->schema->attrDefs)
        {
            projExpr = appendToTailOfList(projExpr,
                    createFullAttrReference(strdup(a->attrName), 0, pos, 0, a->dataType));
            pos++;
        }
        FOREACH(AttributeDef,a,input2->schema->attrDefs)
        {
            if (!searchListString(commonAttrs, a->attrName))
                projExpr = appendToTailOfList(projExpr,
                        createFullAttrReference(strdup(a->attrName), 1, pos, 0, a->dataType));
        }

        DEBUG_LOG("projection expressions for natural join: %s", projExpr);

        op = createProjectionOp(projExpr, (QueryOperator *) jo, NIL, fje->from.attrNames);
        jo->op.parents = singleton(op);

        return ((QueryOperator *) op);
    }
    else
        return ((QueryOperator *) jo);
}

static QueryOperator *
translateFromSubquery(FromSubquery *fsq)
{
    return translateQueryOracle(fsq->subquery);
    //TODO set attr names from FromItem
}

static QueryOperator *
translateFromJsonTable(FromJsonTable *fjt)
{
    JsonTableOperator *jto = createJsonTableOperator(fjt);
    DEBUG_LOG("translated Json Table:\n%s\nINTO\n%s", nodeToString(fjt), nodeToString(jto));
    return ((QueryOperator *) jto);
}

static QueryOperator *
translateNestedSubquery(QueryBlock *qb, QueryOperator *joinTreeRoot, List *attrsOffsets)
{
    List *nestedSubqueries = getListOfNestedSubqueries(qb);
    HashMap *subqueryToAttribute = NEW_MAP(Node,KeyValue);
    QueryOperator *lChild = joinTreeRoot;
    NestingOperator *no = NULL;
    int i = 1;

    FOREACH(NestedSubquery, nsq, nestedSubqueries)
    {
        // change attributes positions in "expr = ANY(...)"
        visitAttrRefToSetNewAttrPos(nsq->expr, attrsOffsets);
        Node *cond = NULL;
        // create condition node for nesting operator, such like "a = ANY(...)"
        if (nsq->nestingType != NESTQ_EXISTS
                && nsq->nestingType != NESTQ_SCALAR) {
            SelectItem *s = (SelectItem *) getHeadOfListP(
                    ((QueryBlock *) nsq->query)->selectClause);
            AttributeReference *subqueryAttr = createFullAttrReference(
                    strdup(s->alias), 1, 0, INVALID_ATTR, typeOf(s->expr));
            List *args = LIST_MAKE(copyObject(nsq->expr), subqueryAttr);
            cond = (Node *) createOpExpr(nsq->comparisonOp, args);
        }

        // create children of nesting operator
        // left child is the root of "from" translation tree or previous nesting operator
        // right child is the root of the current nested subquery's translation tree
        QueryOperator *rChild = translateQueryBlock((QueryBlock *) nsq->query);
        List *inputs = LIST_MAKE(lChild, rChild);

        // create attribute names of nesting operator
        List *attrNames = getAttrNames(lChild->schema);
        List *dts = getDataTypes(lChild->schema);

        // add an auxiliary attribute, which stores the result of evaluating the nested subquery expr
        char *attrName = CONCAT_STRINGS("nesting_eval_", gprom_itoa(i++));
        addToMap(subqueryToAttribute, (Node *) nsq,
                (Node *) createNodeKeyValue((Node *) createConstString(attrName),
                                            (Node *)createConstInt(LIST_LENGTH(attrNames))));

        attrNames = appendToTailOfList(attrNames,strdup(attrName));
        if (nsq->nestingType == NESTQ_EXISTS)
            dts = appendToTailOfListInt(dts, DT_BOOL);
        else if (nsq->nestingType == NESTQ_SCALAR)
            dts = appendToTailOfListInt(dts, getAttrDefByPos(rChild, 0)->dataType);
        else
            dts = appendToTailOfListInt(dts, typeOf(cond));

        // create nesting operator
        no = createNestingOp(nsq->nestingType, cond, inputs, NIL, attrNames, dts);

        // set the nesting operator as the parent of its children
        OP_LCHILD(no)->parents = singleton(no);
        OP_RCHILD(no)->parents = singleton(no);

        // set this nesting operator as the next nesting operator's left child
        lChild = (QueryOperator *) no;
        DEBUG_OP_LOG("Created nesting operator", no);
        DEBUG_NODE_BEATIFY_LOG("Created nesting operator for nested subquery", nsq);
    }

    if (no == NULL)
        return joinTreeRoot;

    replaceAllNestedSubqueriesWithAuxExprs(qb, subqueryToAttribute);
    return ((QueryOperator *) no);
}

static List *
getListOfNestedSubqueries(QueryBlock *qb)
{
    List *nestedSubqueries = NIL;

    // find nested subqueries
    findNestedSubqueries((Node *) qb->selectClause, &nestedSubqueries);
    findNestedSubqueries((Node *) qb->distinct, &nestedSubqueries);
    findNestedSubqueries((Node *) qb->fromClause, &nestedSubqueries);
    findNestedSubqueries((Node *) qb->whereClause, &nestedSubqueries);
    findNestedSubqueries((Node *) qb->groupByClause, &nestedSubqueries);
    findNestedSubqueries((Node *) qb->havingClause, &nestedSubqueries);
    findNestedSubqueries((Node *) qb->orderByClause, &nestedSubqueries);

    return nestedSubqueries;
}

static void
replaceAllNestedSubqueriesWithAuxExprs(QueryBlock *qb, HashMap *qToAttr)
{
    qb->selectClause = (List *) replaceNestedSubqueryWithAuxExpr(
            (Node *) qb->selectClause, qToAttr);
    qb->distinct = replaceNestedSubqueryWithAuxExpr((Node *) qb->distinct, qToAttr);
    qb->fromClause = (List *) replaceNestedSubqueryWithAuxExpr(
            (Node *) qb->fromClause, qToAttr);
    qb->whereClause = replaceNestedSubqueryWithAuxExpr((Node *) qb->whereClause,
            qToAttr);
    qb->groupByClause = (List *) replaceNestedSubqueryWithAuxExpr(
            (Node *) qb->groupByClause, qToAttr);
    qb->havingClause = replaceNestedSubqueryWithAuxExpr(
            (Node *) qb->havingClause, qToAttr);
    qb->orderByClause = (List*) replaceNestedSubqueryWithAuxExpr(
            (Node *) qb->orderByClause, qToAttr);

    DEBUG_NODE_BEATIFY_LOG("After replacing subqueries:", qb);
}

static Node *
replaceNestedSubqueryWithAuxExpr(Node *node, HashMap *qToAttr)
{
    if (node == NULL)
        return NULL;

    if (isA(node, NestedSubquery))
    {
        KeyValue *info = (KeyValue *) getMap(qToAttr, node);
        char *attrName = STRING_VALUE(info->key);
        int attrPos = INT_VALUE(info->value);

        // create auxiliary attribute reference "nesting_eval_i" to the nested subquery
        AttributeReference *attr = createFullAttrReference(strdup(attrName), 0, attrPos, INVALID_ATTR, typeOf(node));

        // if scalar subquery, e.g., WHERE a  = (SELECT count(*) FROM s),
        // then just replace nested subquery with auxiliary attribute reference "nesting_eval_i"
        if (((NestedSubquery *) node)->nestingType == NESTQ_SCALAR)
            return (Node *) attr;

        // create "nesting_eval_i = true" expression
        Constant *trueValue = createConstBool(TRUE);
        List *args = LIST_MAKE(attr, trueValue);
        Operator *opExpr = createOpExpr("=", args);

        // replace the nested subquery node with the auxiliary expression
        return (Node *) opExpr;
    }

    if (isQBQuery(node))
        return node;

    return mutate(node, replaceNestedSubqueryWithAuxExpr, qToAttr);
}

static QueryOperator *
translateWhereClause(Node *whereClause, QueryOperator *nestingOp,
        List *attrsOffsets) {
    if (whereClause == NULL)
        return nestingOp;

    // create selection operator node upon the root of the join tree
    SelectionOperator *so = createSelectionOp(whereClause, nestingOp, NIL,
            getAttrNames(nestingOp->schema));

    // change attributes positions in selection condition
    visitAttrRefToSetNewAttrPosList(so->cond, attrsOffsets);

    // set the parent of the operator's children
    OP_LCHILD(so)->parents = singleton(so);

    return ((QueryOperator *) so);
}

static boolean
visitAttrRefToSetNewAttrPos(Node *n, List *state)
{
    if (n == NULL)
        return TRUE;

    int *offsets = (int *) state;
    if (isA(n, AttributeReference))
    {
        AttributeReference *attrRef = (AttributeReference *) n;
        if (attrRef->fromClauseItem != INVALID_FROM_ITEM && attrRef->attrPosition != INVALID_ATTR)
        {
        		attrRef->attrPosition += getNthOfListInt(state, attrRef->fromClauseItem);
            attrRef->fromClauseItem = 0;
        }
    }

    return visit(n, visitAttrRefToSetNewAttrPos, offsets);
}


static boolean
visitAttrRefToSetNewAttrPosList(Node *n, List *offsetsList)
{
    if (n == NULL)
        return TRUE;

    	if (isA(n, AttributeReference))
    	{
    		int count = 0;
    		AttributeReference *attrRef = (AttributeReference *) n;
    		FOREACH(List, state, offsetsList)
    	    	{
    			if(attrRef->outerLevelsUp == count)
    			{
    				if (attrRef->fromClauseItem != INVALID_FROM_ITEM && attrRef->attrPosition != INVALID_ATTR)
    				{
    					attrRef->attrPosition += getNthOfListInt(state, attrRef->fromClauseItem);
    					attrRef->fromClauseItem = 0;
    				}
    				break;
    			}
    			count ++;
    	    	}
    	}

    return visit(n, visitAttrRefToSetNewAttrPosList, offsetsList);
}

static QueryOperator *
translateSelectClause(List *selectClause, QueryOperator *select,
        List *attrsOffsets, boolean hasAgg)
{
    List *attrNames = NIL;
    List *projExprs = NIL;

    // determine projection expressions
    // visit each expression in select clause to get attribute names
    FOREACH(SelectItem, s, selectClause)
    {
        Node *projExpr = copyObject(s->expr);
        projExprs = appendToTailOfList(projExprs, projExpr);

        // change attribute position in attribute reference in each projection expression
        // this is not necessary if an aggregation operator has been added
        if (!hasAgg)
            visitAttrRefToSetNewAttrPos(projExpr, attrsOffsets);

        // add attribute names
        attrNames = appendToTailOfList(attrNames, strdup(s->alias));
    }

    // create projection operator upon selection operator from select clause
    ProjectionOperator *po = createProjectionOp(projExprs, select, NIL,
            attrNames);

    // set the parent of the operator's children
    OP_LCHILD(po)->parents = singleton(po);

    return ((QueryOperator *) po);
}

static QueryOperator *
translateDistinct(DistinctClause *distinctClause, QueryOperator *input)
{
    QueryOperator *output = input;

    if (distinctClause)
    {
        List *attrNames = getAttrNames(input->schema);

        DuplicateRemoval *o = createDuplicateRemovalOp(NIL, input, NIL, attrNames);
        input->parents = singleton(o);

        output = (QueryOperator *) o;
    }

    return output;
}

static QueryOperator *
translateHavingClause(Node *havingClause, QueryOperator *input, List *attrsOffsets)
{
    QueryOperator *output = input;

    if (havingClause)
    {
        List *attrNames = getAttrNames(input->schema);

        // create selection operator over having clause
        SelectionOperator *so = createSelectionOp(havingClause, input, NIL,
                attrNames);

        // set the parent of the selection's child
        OP_LCHILD(so)->parents = singleton(so);

        // change attributes positions in having condition
        //visitAttrRefToSetNewAttrPos(so->cond, attrsOffsets);

        output = (QueryOperator *) so;
    }

    return output;
}

static QueryOperator *
translateAggregation(QueryBlock *qb, QueryOperator *input, List *attrsOffsets)
{
    QueryOperator *in;
    AggregationOperator *ao;
    List *selectClause = qb->selectClause;
    Node *havingClause = qb->havingClause;
    List *groupByClause = qb->groupByClause;
    List *attrNames = NIL;
    int i;
    List *aggrs = getListOfAggregFunctionCalls(selectClause, havingClause);
    List *aggPlusGroup;
    int numAgg = LIST_LENGTH(aggrs);
    int numGroupBy = LIST_LENGTH(groupByClause);
//    List *newGroupBy;
    ReplaceGroupByState *state;

    DEBUG_NODE_BEATIFY_LOG("aggregation and group-by expressions and select clause:",
            aggrs,
            groupByClause,
            qb->selectClause);

    // does query use aggregation or group by at all?
    if (numAgg == 0 && numGroupBy == 0)
        return input;

    // if necessary create projection for aggregation inputs that are not simple
    // attribute references
    in = createProjectionOverNonAttrRefExprs(&selectClause,
                &havingClause, &groupByClause, input, attrsOffsets);

    // create fake attribute names for aggregation output schema
    for (i = 0; i < LIST_LENGTH(aggrs); i++)
        attrNames = appendToTailOfList(attrNames,
                CONCAT_STRINGS("AGGR_", gprom_itoa(i)));
    for (i = 0; i < LIST_LENGTH(groupByClause); i++)
        attrNames = appendToTailOfList(attrNames,
                CONCAT_STRINGS("GROUP_", gprom_itoa(i)));

    // copy aggregation function calls and groupBy expressions
    // and create aggregation operator
    ao = createAggregationOp(aggrs, groupByClause, in, NIL, attrNames);

    // set the parent of the aggregation's child
    OP_LCHILD(ao)->parents = singleton(ao);

    // replace aggregation function calls and group by expressions in select and having with references to aggregation output attributes
    aggPlusGroup = CONCAT_LISTS(copyList(aggrs), copyList(groupByClause));
    DEBUG_NODE_BEATIFY_LOG("adapted aggregation and group-by expressions and select clause:",
            aggPlusGroup, selectClause);

    state = NEW(ReplaceGroupByState);
    state->expressions = aggPlusGroup;
    state->attrNames = attrNames;
    state->attrOffset = 0;

    qb->selectClause = (List *) replaceAggsAndGroupByMutator(
            (Node *) selectClause, state);
    qb->havingClause = replaceAggsAndGroupByMutator((Node *) havingClause,
            state);

    freeList(aggrs);
    FREE(state);

    return (QueryOperator *) ao;
}

static QueryOperator *
translateWindowFuncs(QueryBlock *qb, QueryOperator *input,
        List *attrsOffsets)
{
    QueryOperator *wOp = input;
    QueryOperator *child = NULL;
    List *wfuncs = NIL;
//    int numWinOp = LIST_LENGTH(wfuncs);
    int cur = 0;
    ReplaceGroupByState *state;
    List *attrNames = NIL;
    int numAttrs = getNumAttrs(input);

    // find window functions and adapt function input attribute references
    visitFindWindowFuncs((Node *) qb->selectClause, &wfuncs);
    visitAttrRefToSetNewAttrPos((Node *) wfuncs, attrsOffsets);

    // create window operator for each window function call
    FOREACH(WindowFunction,f,wfuncs)
    {
        char *aName = CONCAT_STRINGS("winf_", gprom_itoa(cur++));

        child = wOp;
        wOp = (QueryOperator *) createWindowOp(copyObject(f->f),
                copyObject(f->win->partitionBy),
                copyObject(f->win->orderBy),
                copyObject(f->win->frame),
                aName, child, NIL);
        child->parents = singleton(wOp);
        attrNames = appendToTailOfList(attrNames, aName);
    }

    // replace window function calls in select with new attributes
    state = NEW(ReplaceGroupByState);
    state->expressions = wfuncs;
    state->attrNames = attrNames;
    state->attrOffset = numAttrs;

    qb->selectClause = (List *) replaceAggsAndGroupByMutator(
            (Node *) qb->selectClause, state);
    FREE(state);

    DEBUG_LOG("adapted select clause: %s", nodeToString(qb->selectClause));

    return (QueryOperator *) wOp;
}

static Node *
replaceAggsAndGroupByMutator (Node *node, ReplaceGroupByState *state)
{
    int i = 0;

    if (node == NULL)
        return NULL;

    // if node is an expression replace it
    FOREACH(Node,e,state->expressions)
    {
        char *attrName;

        if (equal(node, e))
        {
            attrName = (char *) getNthOfListP(state->attrNames, i);
            return (Node *) createFullAttrReference(strdup(attrName), 0, i + state->attrOffset, 0, typeOf(e));
        }
        i++;
    }

    return mutate(node, replaceAggsAndGroupByMutator, state);
}

static QueryOperator *
createProjectionOverNonAttrRefExprs(List **selectClause, Node **havingClause,
        List **groupByClause, QueryOperator *input, List *attrsOffsets)
{
    // each entry of the list directly points to the original expression, not copy
    List *projExprs = getListOfNonAttrRefExprs(*selectClause, *havingClause,
            *groupByClause);

    QueryOperator *output = input;

    // do we need to another level of projection?
    if (getListLength(projExprs) > 0)
    {
        INFO_NODE_BEATIFY_LOG("create new projection for aggregation function inputs and "
                "group by expressions:", projExprs);

        // create alias for each non-AttributeReference expression
        List *attrNames = NIL;
        int i = 0;

        for(i = 0; i < LIST_LENGTH(projExprs); i++)
        {
            attrNames = appendToTailOfList(attrNames,
                    CONCAT_STRINGS("AGG_GB_ARG", gprom_itoa(i)));
        }

        ASSERT(LIST_LENGTH(projExprs) == LIST_LENGTH(attrNames));

        // copy expressions and create projection operator over the copies
        ProjectionOperator *po = createProjectionOp(projExprs, input, NIL,
                attrNames);

        // set the parent of the projection's child
        OP_LCHILD(po)->parents = singleton(po);

        // change attributes positions in each expression copy to refer to from outputs
        FOREACH(Node, exp, po->projExprs)
            visitAttrRefToSetNewAttrPos(exp, attrsOffsets);

        // replace expressions in Select, having, group-by with new projection expressions
        ReplaceGroupByState *state;

        state = NEW(ReplaceGroupByState);
        state->expressions = projExprs;
        state->attrNames = attrNames;
        state->attrOffset = 0;

        *selectClause = (List *) replaceAggsAndGroupByMutator((Node *) *selectClause, state);
        *havingClause = (Node *) replaceAggsAndGroupByMutator((Node *) *havingClause, state);
        *groupByClause = (List *) replaceAggsAndGroupByMutator((Node *) *groupByClause, state);

        output = ((QueryOperator *) po);
    }

    freeList(projExprs);
    return output;
}

static QueryOperator *
translateOrderBy(QueryBlock *qb, QueryOperator *input, List *attrsOffsets)
{
    OrderOperator *o;
    List *adaptedOrderExprs = copyObject(qb->orderByClause);

    if (!qb->orderByClause)
        return input;

    o = createOrderOp(adaptedOrderExprs, input, NIL);
    addParent(input, (QueryOperator *) o);

    return (QueryOperator *) o;
}

static List *
getListOfNonAttrRefExprs(List *selectClause, Node *havingClause, List *groupByClause)
{
    List *nonAttrRefExprs = NIL;
    List *aggregs = getListOfAggregFunctionCalls(selectClause, havingClause);
    boolean needProjection = FALSE;

    // get non-AttributeReference expressions from arguments of aggregations
    FOREACH(FunctionCall, agg, aggregs)
    {
        FOREACH(Node, arg, agg->args)
        {
            nonAttrRefExprs = appendToTailOfList(nonAttrRefExprs, arg);
            if (!isA(arg, AttributeReference))
                needProjection = TRUE;
        }
    }

    if (groupByClause)
    {
        // get non-AttributeReference expressions from group by clause
        FOREACH(Node, expr, groupByClause)
        {
            nonAttrRefExprs = appendToTailOfList(nonAttrRefExprs, expr);
            if (!isA(expr, AttributeReference))
                needProjection = TRUE;
        }
    }

    INFO_LOG("aggregation function inputs and group by expressions are %s, we "
            "do %s need to create projection before aggregation" ,
            nodeToString(nonAttrRefExprs), (needProjection) ? "": " not ");

    freeList(aggregs);
    if  (needProjection)
        return nonAttrRefExprs;
    return NULL;
}

/*
static boolean
visitNonAttrRefExpr(Node *n, void *state)
{
    if (n == NULL)
        return TRUE;

    List *exprs = (List *) state;
    if (!isA(n, AttributeReference))
    {
        exprs = appendToTailOfList(exprs, n);
    }

    return visit(n, visitNonAttrRefExpr, exprs);
}
*/

static List *
getListOfAggregFunctionCalls(List *selectClause, Node *havingClause)
{
    List *aggregs = NIL;
    // get aggregations from select clause
    FOREACH(Node, sel, selectClause)
        visitAggregFunctionCall(sel, &aggregs);

    // get aggregations from having clause
    visitAggregFunctionCall(havingClause, &aggregs);

    DEBUG_LOG("aggregation functions are\n%s", nodeToString(aggregs));

    return aggregs;
}

static boolean
visitAggregFunctionCall(Node *n, List **aggregs)
{
    if (n == NULL)
        return TRUE;

    if (isA(n, FunctionCall)) // TODO how to prevent from going into nested sub-query?
    {
        FunctionCall *fc = (FunctionCall *) n;
        if (fc->isAgg)
        {
            DEBUG_LOG("Found aggregation '%s'.", exprToSQL((Node *) fc));
            *aggregs = appendToTailOfList(*aggregs, fc);

            // do not recurse into aggregation function calls
            return TRUE;
        }
    }

    return visit(n, visitAggregFunctionCall, aggregs);
}

static boolean
visitFindWindowFuncs(Node *n, List **wfs)
{
    if (n == NULL)
        return TRUE;

    if (isA(n, WindowFunction))
    {
        DEBUG_LOG("Found window function <%s>", exprToSQL(n));
        *wfs = appendToTailOfList(*wfs, n);
        return TRUE;
    }

    return visit(n, visitFindWindowFuncs, wfs);
}

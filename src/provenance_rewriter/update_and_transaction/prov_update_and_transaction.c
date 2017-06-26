/*-----------------------------------------------------------------------------
 *
 * prov_update_and_transaction.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "mem_manager/mem_mgr.h"
#include "log/logger.h"

#include "analysis_and_translate/analyze_oracle.h"
#include "model/node/nodetype.h"
#include "model/set/set.h"
#include "model/set/hashmap.h"
#include "model/expression/expression.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "model/query_operator/operator_property.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/update_and_transaction/prov_update_and_transaction.h"
#include "operator_optimizer/cost_based_optimizer.h"
#include "configuration/option.h"
#include "utility/string_utils.h"


#define STATEMENT_ANNOTATION_SUFFIX_INSERT "ins"
#define STATEMENT_ANNOTATION_SUFFIX_UPDATE "up"
#define STATEMENT_ANNOTATION_SUFFIX_DELETE "del"

#define IS_STATEMENT_ANNOT_ATTR(a) (isSuffix(a,STATEMENT_ANNOTATION_SUFFIX_INSERT) \
        || isSuffix(a,STATEMENT_ANNOTATION_SUFFIX_UPDATE) \
        || isSuffix(a,STATEMENT_ANNOTATION_SUFFIX_DELETE))

#define VERSIONS_STARTSCN_ATTR "VERSIONS_STARTSCN"


static void mergeForTransactionProvenance(ProvenanceComputation *op);
static boolean needAnnotAttributes(ProvenanceComputation *p);
//static void mergeForReenactOnly(ProvenanceComputation *op);
static QueryOperator *createProjToRemoveAnnot (QueryOperator *o);
static void mergeSerializebleTransaction(ProvenanceComputation *op);
static void mergeReadCommittedTransaction(ProvenanceComputation *op);

static QueryOperator *getUpdateForPreviousTableVersion (ProvenanceComputation *p, char *tableName, int startPos, List *updates);
static QueryOperator *getLastUpdateForTable (ProvenanceComputation *p, char *tableName);
static boolean isAttrUpdated (Node *expr, AttributeDef *a);
static void addIgnoreAttr (QueryOperator *o, char *attrName);
static void addConditionsToBaseTables (ProvenanceComputation *op);
static boolean onlyUpdatedNeedsResultFiltering (ProvenanceComputation *op);
static void removeInputTablesWithOnlyInserts (ProvenanceComputation *op);
static void extractUpdatedFromTemporalHistory (ProvenanceComputation *op);
static Node *adaptConditionForReadCommitted(Node *cond, Constant *scn, int attrPos);

static List *findUpdatedTableAccceses (List *tables);

static void removeUpdateAnnotAttr (ProvenanceComputation *op);
static void addUpdateAnnotationAttrs (ProvenanceComputation *op);
static void addAnnotConstToUnion (QueryOperator *un, boolean leftIsTrue, char *annotName);

void
mergeUpdateSequence(ProvenanceComputation *op)
{
    List *updateTypes = NIL;

    // create a list storing the types of updates using in the transaction
    FOREACH(QueryOperator,up,op->op.inputs)
    {
        ReenactUpdateType t = INT_VALUE(GET_STRING_PROP(up, PROP_PROV_ORIG_UPDATE_TYPE));
        updateTypes = appendToTailOfListInt(updateTypes, t);
    }
    SET_STRING_PROP(op, PROP_PROV_ORIG_UPDATE_TYPE, updateTypes);

    if (op->inputType == PROV_INPUT_TRANSACTION || op->inputType == PROV_INPUT_REENACT
                || op->inputType == PROV_INPUT_REENACT_WITH_TIMES)
    {
            mergeForTransactionProvenance(op);

        return;
    }
    //TODO op->inputType == PROV_INPUT_UPDATE_SEQUENCE
    FATAL_LOG("currenly only provenance for transaction and reenactment supported");
}

static void
mergeSerializebleTransaction(ProvenanceComputation *op)
{
    List *updates;
    HashMap *curTranslation = NEW_MAP(Constant, Node);
    List *tabNames = op->transactionInfo->updateTableNames;
    List *updateTypes = (List *) GET_STRING_PROP(op, PROP_PROV_ORIG_UPDATE_TYPE);
    char *reenactTargetTable = GET_STRING_PROP_STRING_VAL(op, PROP_PC_TABLE);
    List *isNoProvs = (List *) GET_STRING_PROP(op, PROP_REENACT_NO_TRACK_LIST);
    boolean addAnnotAttrs = needAnnotAttributes(op);
    int i;

    // updates to process
    updates = copyList(op->op.inputs);

    // cut links to parent
    removeParentFromOps(op->op.inputs, (QueryOperator *) op);
    op->op.inputs = NIL;

    // mark statements for which no provenance should be tracked as BASERELATION
    if (op->provType != PROV_NONE)
    {
        HashMap *lastNoProvPerTable = NEW_MAP(Constant,Node);
        i = 0;

        DEBUG_NODE_BEATIFY_LOG("mark statements to be ignored for provenance tracking", isNoProvs);

        FORBOTH(Node,stmt,isNoProv, updates, isNoProvs)
        {
            QueryOperator *o = (QueryOperator *) stmt;
            boolean noP = BOOL_VALUE(isNoProv);
            char *tableName = (char *) getNthOfListP(tabNames, i);

            DEBUG_LOG("statement %u on table %s is provenance: %s", i, tableName, noP ? "yes" : "no");

            if (noP)
            {
                MAP_ADD_STRING_KEY(lastNoProvPerTable, tableName, o);
            }
            i++;
        }

        DEBUG_NODE_BEATIFY_LOG("the following statements will be marked to be "
                "ignored for provenance tracking", lastNoProvPerTable);

        FOREACH_HASH_ENTRY(kv,lastNoProvPerTable)
        {
            QueryOperator *o = (QueryOperator *) kv->value;
            char *tableName = STRING_VALUE(kv->key);

            SET_BOOL_STRING_PROP(o,PROP_USE_PROVENANCE);
            SET_STRING_PROP(o, PROP_USER_PROV_ATTRS,
                    (Node *) stringListToConstList(getQueryOperatorAttrNames(o)));
            SET_STRING_PROP(o, PROP_PROV_REL_NAME, (Node *) createConstString(tableName));
        }
    }

    // loop through statement replacing ops where necessary
    DEBUG_NODE_BEATIFY_LOG("reenacted statements to merge are:", updates);
    INFO_OP_LOG("reenacted statements to merge are:", updates);

    i = 0;
    FORBOTH(void,u,tN,updates,tabNames)
    {
        QueryOperator *up = (QueryOperator *) u;
        char *tName = (char *) tN;
        List *children = NULL;
        ReenactUpdateType typ = getNthOfListInt(updateTypes, i);

        DEBUG_NODE_BEATIFY_LOG("merge", (Node *) up);
        INFO_OP_LOG("Replace table access operators in", up);

        // find all table access operators
        findTableAccessVisitor((Node *) up, &children);

        FOREACH(TableAccessOperator, t, children)
        {
            // only merge if we already have statement producing previous version of table
            if (MAP_HAS_STRING_KEY(curTranslation, t->tableName))
            {
                INFO_OP_LOG("\tTable Access", t);
                QueryOperator *prevUpdate = (QueryOperator *)
                        MAP_GET_STRING(curTranslation, t->tableName);

                if (typ == UPDATE_TYPE_INSERT_QUERY && addAnnotAttrs)
                    prevUpdate = createProjToRemoveAnnot(prevUpdate);

                INFO_OP_LOG("\tUpdate is", prevUpdate);
                switchSubtreeWithExisting((QueryOperator *) t, prevUpdate);

                INFO_LOG("Table after merge %s", operatorToOverviewString((Node *) up));
            }
        }

        // if reenacted statement updates table then this is the current version of this table
        if (!streq(tName,"_NONE"))
        {
            MAP_ADD_STRING_KEY(curTranslation,tName,up);
        }

        i++;
    }

    // add root of final reenactment query as only child of provenance computation
    // if the user has specified what table's provenance to track, then use the last update affecting that table
    if (reenactTargetTable != NULL)
    {
        Node *lastUp = MAP_GET_STRING(curTranslation, reenactTargetTable);

        if (lastUp == NULL)
            FATAL_LOG("cannot track provenance of table %s that is not affected by the transaction:", reenactTargetTable);

        addChildOperator((QueryOperator *) op, (QueryOperator *) lastUp);
    }
    else
    {
        addChildOperator((QueryOperator *) op,
                (QueryOperator *) getTailOfListP(updates));
    }
    // if no provenance is requested and we are doing REENACT AS OF, then we have to set
    // the asOf field of each table access operator
    if (op->provType == PROV_NONE && op->asOf != NULL)
    {
        List *tables = NIL;

        // find all table access operators
        findTableAccessVisitor((Node *) op->op.inputs, &tables);

        FOREACH(TableAccessOperator,t,tables)
        {
            t->asOf = copyObject(op->asOf);
        }
    }

    if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
        ASSERT(checkModel((QueryOperator *) op));
}

static QueryOperator *
createProjToRemoveAnnot (QueryOperator *o)
{
    List *normalAttrs = NIL;
    CREATE_INT_SEQ(normalAttrs, 0, getNumNormalAttrs(o) - 2, 1);
    DEBUG_LOG("num attrs %i", getNumNormalAttrs(o) - 2);

    QueryOperator *newTop = createProjOnAttrs(o, normalAttrs);
    newTop->inputs = LIST_MAKE(o);
    addParent(o, newTop);

    DEBUG_OP_LOG("added projection to remove statement annotation:", newTop);

    return newTop;
}

static boolean
needAnnotAttributes(ProvenanceComputation *p)
{
    boolean result = GET_BOOL_STRING_PROP(p, PROP_PC_STATEMENT_ANNOTATIONS);

    // if user has requested to only return updated rows then we may need statement annotations
    if (HAS_STRING_PROP(p,PROP_PC_ONLY_UPDATED))
    {
        result = result || !(isRewriteOptionActivated(OPTION_UPDATE_ONLY_USE_CONDS)
                                || isRewriteOptionActivated(OPTION_UPDATE_ONLY_USE_HISTORY_JOIN));
        result = result || onlyUpdatedNeedsResultFiltering(p);
    }

    return result;
}

static void
mergeForTransactionProvenance(ProvenanceComputation *op)
{
	ProvenanceTransactionInfo *tInfo = op->transactionInfo;
	boolean addAnnotAttrs = needAnnotAttributes(op);

	// remove table access of inserts where
	if (HAS_STRING_PROP(op,PROP_PC_ONLY_UPDATED))
    {
	     removeInputTablesWithOnlyInserts(op);
    }

    if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
        ASSERT(checkModel((QueryOperator *) op));

    // add boolean attributes to store whether update did modify a row
    if (addAnnotAttrs)
    {
        addUpdateAnnotationAttrs(op);
        removeUpdateAnnotAttr(op);

        INFO_LOG("after adding projection:\n%s", operatorToOverviewString((Node *) op));
    }
    //TODO check that this is ok

    // merge updates to create transaction reenactment query
	switch (tInfo->transIsolation) {
        case ISOLATION_SERIALIZABLE:
            mergeSerializebleTransaction(op);
            break;
        case ISOLATION_READ_COMMITTED:
            mergeReadCommittedTransaction(op);
            break;
        default:
            FATAL_LOG("isolation level %u not supported:", tInfo->transIsolation);
            break;
	}

    if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
        ASSERT(checkModel((QueryOperator *) op));

	INFO_LOG("updates after merge:\n%s", operatorToOverviewString((Node *) op));
}

static void
removeUpdateAnnotAttr (ProvenanceComputation *op)
{
    char *reenactTargetTable = GET_STRING_PROP_STRING_VAL(op, PROP_PC_TABLE);
    QueryOperator *lastUp;
    List *normalAttrs = NIL;

    // get last statement affecting the table which we are tracking
    if (reenactTargetTable == NULL)
        lastUp = (QueryOperator *) getTailOfListP(op->op.inputs);
    else
    {
        lastUp = getLastUpdateForTable(op, reenactTargetTable);
    }

    CREATE_INT_SEQ(normalAttrs, 0, getNumNormalAttrs(lastUp) - 2, 1);
    DEBUG_LOG("num attrs %i", getNumNormalAttrs(lastUp) - 2);

    QueryOperator *newTop = createProjOnAttrs(lastUp, normalAttrs);
    newTop->inputs = LIST_MAKE(lastUp);
    switchSubtrees(lastUp, newTop);
    lastUp->parents = LIST_MAKE(newTop);
    SET_BOOL_STRING_PROP(newTop,PROP_PC_PROJ_TO_REMOVE_SANNOT);

    INFO_OP_LOG("after adding projection to remove annotation attrs:\n", op);
}

static void
addUpdateAnnotationAttrs (ProvenanceComputation *op)
{
    int i = 0, j = 0;
    List *noProv = (List *) GET_STRING_PROP(op, PROP_REENACT_NO_TRACK_LIST);

    // add projection for each update to create the update attribute
    FORBOTH_LC(uLc, trLc, op->transactionInfo->originalUpdates, op->op.inputs)
    {
        QueryOperator *q = (QueryOperator *) LC_P_VAL(trLc);
        Node *u = (Node *) LC_P_VAL(uLc);
        char *annotName = NULL;
        boolean noP = BOOL_VALUE(getNthOfListP(noProv, i));

        if (!noP)
        {
            switch (u->type) {
                case T_Insert:
                    annotName = strdup(STATEMENT_ANNOTATION_SUFFIX_INSERT);
                    break;
                case T_Update:
                    annotName = strdup(STATEMENT_ANNOTATION_SUFFIX_UPDATE);
                    break;
                case T_Delete:
                    annotName = strdup(STATEMENT_ANNOTATION_SUFFIX_DELETE);
                    break;
                default:
                    FATAL_LOG("expected insert, update, or delete");
            }

            // mark update annotation attribute as provenance
            SET_STRING_PROP(q, PROP_ADD_PROVENANCE, LIST_MAKE(createConstString(annotName)));
            SET_STRING_PROP(q, PROP_PROV_IGNORE_ATTRS, MAKE_STR_SET(strdup(annotName)));
            SET_STRING_PROP(q, PROP_PROV_ADD_REL_NAME, createConstString(CONCAT_STRINGS("U", itoa(j + 1))));
            SET_STRING_PROP(q, PROP_PC_VERSION_SCN_ATTR, createConstString(annotName));

            // use original update to figure out type of each update (UPDATE/DELETE/INSERT)
            // switch
            switch (u->type) {
                case T_Insert:
                {
                    if (isA(q, SetOperator))
                        addAnnotConstToUnion(q, FALSE, annotName);
                    else
                    {
                        ProjectionOperator *p;

                        p = (ProjectionOperator *) createProjOnAllAttrs(q);
                        switchSubtrees(q, (QueryOperator *) p);
                        addChildOperator((QueryOperator *) p,q);
                        //TODO ok to move properties to parent?
                        p->op.properties = q->properties;
                        q->properties = (Node *) NEW_MAP(Node,Node);

                        p->projExprs = appendToTailOfList(p->projExprs, (Node *) createConstBool(TRUE));

                        // add attribute name for annotation attribute to schema
                        p->op.schema->attrDefs =
                                appendToTailOfList(p->op.schema->attrDefs,
                                        createAttributeDef(strdup(annotName), DT_BOOL));
                    }
                }
                break;
                case T_Update:
                {
                    // if update CASE translation was used
                    if (isA(q,ProjectionOperator))
                    {
                        Node *annotAttr;
                        ProjectionOperator *p = (ProjectionOperator *) q;
                        Node *cond = getNthOfListP(
                                (List *) GET_STRING_PROP(op, PROP_PC_UPDATE_COND), i);

                        // update has no condition
                        if (cond == NULL)
                        {
                            annotAttr = (Node *) createConstBool(TRUE);
                        }
                        else
                        {
                            // add proj expr CASE WHEN C THEN TRUE ELSE FALSE END
                            annotAttr = (Node *) createCaseExpr(NULL,
                                    LIST_MAKE(createCaseWhen(cond,
                                            (Node *) createConstBool(TRUE))),
                                            (Node *) createConstBool(FALSE));
                        }

                        p->projExprs = appendToTailOfList(p->projExprs, annotAttr);

                        // add attribute name for annotation attribute to schema
                        p->op.schema->attrDefs =
                                appendToTailOfList(p->op.schema->attrDefs,
                                        createAttributeDef(strdup(annotName), DT_BOOL));
                    }
                    // else union was used
                    else
                        addAnnotConstToUnion(q, TRUE, annotName);
                }
                break;
                case T_Delete:
                {
                    ProjectionOperator *p;

                    p = (ProjectionOperator *) createProjOnAllAttrs(q);
                    switchSubtrees(q, (QueryOperator *) p);

                    p->projExprs = appendToTailOfList(p->projExprs, (Node *) createConstBool(FALSE));

                    // add attribute name for annotation attribute to schema
                    p->op.schema->attrDefs =
                            appendToTailOfList(p->op.schema->attrDefs,
                                    createAttributeDef(strdup(annotName), DT_BOOL));
                }
                break;
                default:
                    FATAL_LOG("expected insert, update, or delete");
            }
            j++;
        }

        i++;
    }
}

static void
addAnnotConstToUnion (QueryOperator *un, boolean leftIsTrue, char *annotName)
{
    QueryOperator *rChild = OP_RCHILD(un);
    QueryOperator *lChild = OP_LCHILD(un);
    ProjectionOperator *p;

    // if is a projection then just add projection expression
    if (isA(rChild, ProjectionOperator))
        p = (ProjectionOperator *) rChild;
    // otherwise add projection on all attributes first
    else
    {
        p = (ProjectionOperator *) createProjOnAllAttrs(rChild);
        switchSubtrees(rChild, (QueryOperator *) p);
        p->op.inputs = singleton(rChild);
        rChild->parents = singleton(p);
    }

    p->projExprs = appendToTailOfList(p->projExprs, createConstBool(!leftIsTrue));
    p->op.schema->attrDefs =
            appendToTailOfList(p->op.schema->attrDefs, createAttributeDef(strdup(annotName), DT_BOOL));

    // if is a projection then just add projection expression
    if (isA(lChild, ProjectionOperator))
        p = (ProjectionOperator *) lChild;
    // otherwise add projection on all attributes first
    else
    {
        p = (ProjectionOperator *) createProjOnAllAttrs(lChild);
        switchSubtrees(lChild, (QueryOperator *) p);
        p->op.inputs = singleton(lChild);
        lChild->parents = singleton(p);
    }

    p->projExprs = appendToTailOfList(p->projExprs, createConstBool(leftIsTrue));
    p->op.schema->attrDefs =
            appendToTailOfList(p->op.schema->attrDefs, createAttributeDef(strdup(annotName), DT_BOOL));

    // adapt union
    un->schema->attrDefs =
            appendToTailOfList(un->schema->attrDefs, createAttributeDef(strdup(annotName), DT_BOOL));
}

static void
mergeReadCommittedTransaction(ProvenanceComputation *op)
{
	List *scns = op->transactionInfo->scns;
    int i = 0;
    QueryOperator *mergeRoot = NULL;
    QueryOperator *finalProj = NULL;
    boolean addAnnotAttrs = needAnnotAttributes(op);
    Set *tableUpdatedBefore = STRSET();
    char *reenactTargetTable = GET_STRING_PROP_STRING_VAL(op, PROP_PC_TABLE);

    // Loop through update translations and add version_startscn condition + attribute
	FORBOTH_LC(uLc, trLc, op->transactionInfo->originalUpdates, op->op.inputs)
	{
	    QueryOperator *q = (QueryOperator *) LC_P_VAL(trLc);
	    Node *u = (Node *) LC_P_VAL(uLc);
	    char *tableName = NULL;

		// use original update to figure out type of each update (UPDATE/DELETE/INSERT)
		// switch
		switch (u->type) {
		// case t_InsertStmt:t:
		case T_Insert:
		{
		    Insert *ins = (Insert *) u;
		    tableName = ins->insertTableName;
		    QueryOperator *qRoot = NULL;
		    int attrPos = INVALID_ATTR;

		    // check whether q is a set operation
		    if (isA(q, SetOperator))
		    {
                QueryOperator *newQ = isA(q, ProjectionOperator) ? OP_LCHILD(q) : q;
                QueryOperator *lChild = OP_LCHILD(newQ);
                QueryOperator *rChild = OP_RCHILD(newQ);
                ProjectionOperator *lC; // = (ProjectionOperator *) OP_LCHILD(newQ); //TODO correct to assume that child is a projection?
                ProjectionOperator *rC; // = (ProjectionOperator *) OP_RCHILD(newQ); //TODO correct to assume that child is a projection?
                ProjectionOperator *p;

                addIgnoreAttr(newQ,VERSIONS_STARTSCN_ATTR);


                // left input may already be projections, if not, then create projections on all attributes
                if (!isA(lChild, ProjectionOperator))
                {
                    lC = (ProjectionOperator *) createProjOnAllAttrs((QueryOperator *) lChild);
                    addChildOperator((QueryOperator *) lC, (QueryOperator *) lChild);
                    switchSubtrees(lChild, (QueryOperator *) lC);
                }
                else
                    lC = (ProjectionOperator *) lChild;

                // right inputs may already be projections, if not, then create projections on all attributes
                // also set query root
                if (!isA(rChild, ProjectionOperator))
                {
                    rC = (ProjectionOperator *) createProjOnAllAttrs((QueryOperator *) rChild);
                    addChildOperator((QueryOperator *) rC, (QueryOperator *) rChild);
                    switchSubtrees(rChild, (QueryOperator *) rC);
                }
                else
                    rC = (ProjectionOperator *) rChild;

                if (addAnnotAttrs)
                {
                    qRoot = OP_LCHILD(rChild);
                }
                else
                    qRoot = rChild;

                ASSERT(isA(lC,ProjectionOperator) && isA(rC,ProjectionOperator));

                // is R UNION INSERTS transform into R + SCN UNION PROJECTION [*, SCN] (q)

                // determine attribute position in child
                attrPos = getNumAttrs(OP_LCHILD(lC));
                if (hasSetElem(tableUpdatedBefore, tableName) && addAnnotAttrs)
                    attrPos++;

                lC->op.schema->attrDefs = appendToTailOfList(lC->op.schema->attrDefs,
                                    createAttributeDef(VERSIONS_STARTSCN_ATTR, DT_LONG));
                lC->projExprs = appendToTailOfList(lC->projExprs,
                        createFullAttrReference(VERSIONS_STARTSCN_ATTR, 0,
                                attrPos, 0, //original was getNumAttrs(OP_LCHILD(lC))
                                DT_LONG));

                rC->op.schema->attrDefs = appendToTailOfList(rC->op.schema->attrDefs,
                        createAttributeDef(VERSIONS_STARTSCN_ATTR, DT_LONG));
                rC->projExprs = appendToTailOfList(rC->projExprs,
                        copyObject(getNthOfListP(scns,i)));

                // add attributes to union and table access
                newQ->schema->attrDefs = appendToTailOfList(newQ->schema->attrDefs,
                        createAttributeDef(VERSIONS_STARTSCN_ATTR, DT_LONG));

                // add projection over query, add constant SCN attr, switch with query
                if (!isA(q, ProjectionOperator))
                {
                    p = (ProjectionOperator *) createProjOnAllAttrs(qRoot);
                    addChildOperator((QueryOperator *) p,qRoot);
                    p->op.schema->attrDefs = appendToTailOfList(p->op.schema->attrDefs,
                            createAttributeDef(VERSIONS_STARTSCN_ATTR, DT_LONG));
                    p->projExprs = appendToTailOfList(p->projExprs,copyObject(getNthOfListP(scns,i)));
                    switchSubtrees(qRoot,(QueryOperator *) p);
                } //TODO ok that there is no else here?

		    }
		    // no union, then this is an insert only case where we avoid the union, just add projection on top
		    else
		    {
		        qRoot = q;
		        ProjectionOperator *p;

		        addIgnoreAttr(qRoot,VERSIONS_STARTSCN_ATTR);

		        if (!isA(q, ProjectionOperator))
		        {
		            p = (ProjectionOperator *) createProjOnAllAttrs(qRoot);
		            addChildOperator((QueryOperator *) p,qRoot);
	                switchSubtrees(qRoot,(QueryOperator *) p);
		        }
		        else
		            p = (ProjectionOperator *) q;

                p->op.schema->attrDefs = appendToTailOfList(p->op.schema->attrDefs,
                        createAttributeDef(VERSIONS_STARTSCN_ATTR, DT_LONG));
                p->projExprs = appendToTailOfList(p->projExprs,copyObject(getNthOfListP(scns,i)));
		    }

            // add projection over table access operators to remove SCN attribute that will be added later
            List *children = NULL;

            // find all table access operators
            findTableAccessVisitor((Node *) qRoot, &children);
            INFO_LOG("Replace table access operators in %s",
                    operatorToOverviewString((Node *) q));
            //TODO use merge if necessary
            FOREACH(TableAccessOperator, t, children)
            {
                ProjectionOperator *po = (ProjectionOperator *) createProjOnAllAttrs((QueryOperator *) t);
                addChildOperator((QueryOperator *) po, (QueryOperator *) t);
                switchSubtrees((QueryOperator *) t,(QueryOperator *) po);
            }

            DEBUG_OP_LOG("after replacement", q);
		}
        break;
        // case T_DeleteStmt:
		case T_Delete:
		{
            Delete *del = (Delete *) u;
            tableName = del->deleteTableName;
            AttributeReference *scnAttr;
            Node *newCond;
            SelectionOperator *s = (SelectionOperator *) q;
            //TODO attr pos
		    // assume it is selection over input (for new translation has to be adapted)
		    ASSERT(isA(q,SelectionOperator));

		    // add SCN attribute to schema and turn NOT(C) into NOT(C) OR SCN > X to selection condition
            DEBUG_LOG("Deal with condition: %s", exprToSQL((Node *) s->cond));

            // adding SCN < update SCN condition
            scnAttr = createFullAttrReference(VERSIONS_STARTSCN_ATTR, 0,
                    getNumAttrs(OP_LCHILD(q)), 0, DT_LONG);
            newCond = (Node *) createOpExpr("<=",
                    LIST_MAKE((Node *) scnAttr,
                            copyObject(getNthOfListP(scns,i))));
            s->cond = OR_EXPRS(s->cond, newCond);

		    q->schema->attrDefs = appendToTailOfList(q->schema->attrDefs,
		                          createAttributeDef(VERSIONS_STARTSCN_ATTR, DT_LONG));
		    addIgnoreAttr(q,VERSIONS_STARTSCN_ATTR);
		}
        break;
        // case T_UpdateStmt:
		case T_Update:
		{
		    Update *upd = (Update *) u;
		    tableName = upd->updateTableName;
			// either CASE translation OR union translation
			//if its case translation
			if (isA(q,ProjectionOperator))
			{
                Node *newWhen = NULL;
			    ProjectionOperator *proj = (ProjectionOperator *) q;
				List *projExprs;
				Node *newProjExpr;
				int attrPos = INVALID_ATTR;
				int j = 0;
//				boolean annotProj = isA(OP_LCHILD(proj), ProjectionOperator);
				if (HAS_STRING_PROP(proj, PROP_PC_PROJ_TO_REMOVE_SANNOT))
				{
				    proj = (ProjectionOperator *) OP_LCHILD(proj);
				}
				projExprs = proj->projExprs;

				// attribute position of STARTSCN attribute in the child
				// if this is the first update then it will be the last attribute
				// of the child operator + 1. If this is not the first update
				// and annotation attributes are added then we have to add another
				// +1
				attrPos = getNumAttrs(OP_LCHILD(proj));
				if (hasSetElem(tableUpdatedBefore, tableName) && addAnnotAttrs)
				    attrPos++;

				//Add SCN foreach CaseEpr
				FOREACH(Node, expr, projExprs)
				{
				    // is part of the set clause for an update with WHERE clause
					if(isA(expr,CaseExpr))
				    {
					    AttributeReference *scnAttr;
					    CaseExpr *cexp = (CaseExpr *) expr;
					    CaseWhen *whenC = (CaseWhen *) getNthOfListP(cexp->whenClauses, 0);
						Node *when = whenC->when;
						Node *newCond;

						DEBUG_LOG("Deal with case: %s", exprToSQL((Node *) cexp));

						// adding SCN < update SCN condition
						scnAttr = createFullAttrReference(VERSIONS_STARTSCN_ATTR, 0,
						        attrPos, 0, DT_LONG);
						newCond = (Node *) createOpExpr("<=",
								LIST_MAKE((Node *) scnAttr,
								        copyObject(getNthOfListP(scns,i))));

						newWhen = ((when == NULL) || equal(when,createConstBool(TRUE))) ?  newCond : AND_EXPRS(when, newCond);
						whenC->when = newWhen;
						DEBUG_LOG("Updated case is: %s", exprToSQL((Node *) cexp));
				    }
					// check if is part of the set clause for an update without WHERE clause
					else if (j < LIST_LENGTH(upd->schema))
					{
					    AttributeDef *attrDef = getNthOfListP(upd->schema,j);
					    if (isAttrUpdated(expr, attrDef))
					    {
					        ListCell *exprP = expr_his_cell;
					        CaseExpr *c;
					        CaseWhen *whenC;
					        Node *els;
					        Node *cond;
					        AttributeReference *scnAttr;

	                        scnAttr = createFullAttrReference(VERSIONS_STARTSCN_ATTR, 0,
	                                attrPos, 0, DT_LONG);
	                        cond = (Node *) createOpExpr("<=",
	                                LIST_MAKE((Node *) scnAttr,
	                                        copyObject(getNthOfListP(scns,i))));

	                        whenC = createCaseWhen(cond, expr);
	                        els = (Node *) createFullAttrReference(strdup(attrDef->attrName), 0, j, 0, attrDef->dataType);

	                        c = createCaseExpr(NULL,singleton(whenC), els);
	                        LC_P_VAL(exprP) = c;
					    }
					}
					else
					{
					    //TODO ignore or update too?
					}
					j++;
				}

               //make new case for SCN
                Node *els = (Node *) createFullAttrReference(VERSIONS_STARTSCN_ATTR, 0, attrPos, 0, DT_LONG);

                // TODO do not modify the SCN attribute to avoid exponential expression size blow-up
                newProjExpr = (Node *) els; // caseExpr
                proj->projExprs =
                        appendToTailOfList(projExprs, newProjExpr);
                proj->op.schema->attrDefs = appendToTailOfList(proj->op.schema->attrDefs,
                        createAttributeDef(VERSIONS_STARTSCN_ATTR, DT_LONG));
                addIgnoreAttr((QueryOperator *) proj, VERSIONS_STARTSCN_ATTR);
                DEBUG_OP_LOG("update with SCN attribute:", proj);
			}
			else
			{
			    FATAL_LOG("merging for READ COMMITTED and Union Update translation not supported yet.");
			}
		}
        break;
		default:
		    FATAL_LOG("should never have ended up here");
			break;
		}

		// add tableName to set
		addToSet(tableUpdatedBefore, tableName);
        i++;
	}

	// remove provenance compuation as parent from ops
	removeParentFromOps(op->op.inputs, (QueryOperator *) op);
	List *updates = copyList(op->op.inputs);
	i = 0;

	// cut links to parent
	op->op.inputs = NIL;

	// reverse list
	reverseList(updates);
	reverseList(op->transactionInfo->updateTableNames);

	DEBUG_NODE_BEATIFY_LOG("Updates to merge are:", updates);
	INFO_OP_LOG("Updates to merge overview are:", updates);
	/*
	 * Merge the individual queries for all updates into one
	 */
	FOREACH(QueryOperator, u, updates) {
		List *children = NULL;

		// find all table access operators
		findTableAccessVisitor((Node *) u, &children);
		INFO_OP_LOG("Replace table access operators in",u);

		FOREACH(TableAccessOperator, t, children) {
			INFO_OP_LOG("\tTable Access", t);
			QueryOperator *up = getUpdateForPreviousTableVersion(op,
					t->tableName, i, updates);

			INFO_OP_LOG("\tUpdate is",up);
			// previous table version was created by transaction
			if (up != NULL)
			{
                //TODO adapt SCN attribute reference
			    // FIX parent projection expressions attribute reference
			    //TODO what was the point of this, this seems wrong????????? Removed it for now
//                FOREACH(ProjectionOperator,p,t->op.parents)
//                {
//                    AttributeReference *a = getTailOfListP(p->projExprs);
//                    a->attrPosition = getNumAttrs((QueryOperator *) up) - 1;
//                }
				switchSubtreeWithExisting((QueryOperator *) t, up);
			}
			// previous table version is the one at transaction begin
			else
			{
//			    Node *scn = (Node *) getTailOfListP(op->transactionInfo->scns);
			    Constant *scnC = (Constant *) copyObject(op->transactionInfo->commitSCN);
			    *((long *) scnC->value) = *((long *) scnC->value) - 1; //getCommit SCN - 1

			    if (!HAS_STRING_PROP(t,PROP_TABLE_USE_ROWID_VERSION))
			    {
			        SET_BOOL_STRING_PROP(t,PROP_USE_PROVENANCE);
			        SET_STRING_PROP(t,PROP_USER_PROV_ATTRS,
			                stringListToConstList(
			                        getQueryOperatorAttrNames(
			                                (QueryOperator *) t)));
                    ((QueryOperator *) t)->schema->attrDefs = appendToTailOfList(((QueryOperator *) t)->schema->attrDefs,
                            createAttributeDef(VERSIONS_STARTSCN_ATTR, DT_LONG));
			    }

                t->asOf = (Node *) LIST_MAKE(scnC, copyObject(scnC));
			}

			DEBUG_LOG("\tTable after merge %s",
					operatorToOverviewString((Node *) u));
		}
		i++;
	}

	// add projection that removes the VERSIONS_STARTSCN attribute
	List *finalAttrs, *projExprs = NIL;
	int cnt = 0;

	// if user has specified what table's provenance to track then get last statement targeting this table
	if (reenactTargetTable != NULL)
	{
	    mergeRoot = getUpdateForPreviousTableVersion(op,
	            reenactTargetTable, 0, updates);

        if (mergeRoot == NULL)
            FATAL_LOG("cannot track provenance of table %s that is not affected by the transaction:", reenactTargetTable);
	}
	else
	    mergeRoot = (QueryOperator *) getHeadOfListP(updates);

	// create projection expressions
	finalAttrs = NIL;

    FOREACH(AttributeDef, attr, mergeRoot->schema->attrDefs)
    {
        if (strcmp(attr->attrName,VERSIONS_STARTSCN_ATTR) != 0)
        {
            projExprs = appendToTailOfList(projExprs, createFullAttrReference(attr->attrName, 0, cnt, 0, attr->dataType));
            finalAttrs = appendToTailOfList(finalAttrs, strdup(attr->attrName));
        }
        cnt++;
    }

	finalProj = (QueryOperator *) createProjectionOp(projExprs, mergeRoot, NIL, finalAttrs);
	mergeRoot->parents = singleton(finalProj);

	INFO_OP_LOG("Merged updates are:", finalProj);
	DEBUG_NODE_BEATIFY_LOG("Merged updates are:", finalProj);

	// replace updates sequence with root of the whole merged update query
	addChildOperator((QueryOperator *) op, finalProj);
	DEBUG_NODE_BEATIFY_LOG("Provenance computation for updates that will be passed "
	        "to rewriter:", op);

    if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
        ASSERT(checkModel((QueryOperator *) finalProj));
}

static boolean
isAttrUpdated (Node *expr, AttributeDef *a)
{
    char *attrN = a->attrName;
    if (isA(expr, AttributeReference))
    {
        AttributeReference *aRef = (AttributeReference *) expr;
        if (streq(aRef->name, attrN))
        {
            return FALSE;
        }
    }
    return TRUE;
}

static void
addIgnoreAttr (QueryOperator *o, char *attrName)
{
    if (HAS_STRING_PROP(o,PROP_PROV_IGNORE_ATTRS))
    {
        Set *ignoreAttrs;
        ignoreAttrs = (Set *) GET_STRING_PROP(o,PROP_PROV_IGNORE_ATTRS);
        addToSet(ignoreAttrs, strdup(attrName));
    }
    else
        SET_STRING_PROP(o, PROP_PROV_IGNORE_ATTRS, MAKE_STR_SET(strdup(attrName)));
}

static QueryOperator *
getLastUpdateForTable (ProvenanceComputation *p, char *tableName)
{
    List *tablenames = p->transactionInfo->updateTableNames;
    Node *child = NULL;
    int i = 0;

    FOREACH(char,t,tablenames)
    {
        if (streq(t,tableName))
        {
            child = getNthOfListP(p->op.inputs, i);
        }
        i++;
    }

    if (child == NULL)
        FATAL_LOG("did not find any statement updating table %s in transaction\n\n%s",
                tableName, beatify(nodeToString(p->transactionInfo->originalUpdates)));

    DEBUG_OP_LOG("last update", child);

    return (QueryOperator *) child;
}


static QueryOperator *
getUpdateForPreviousTableVersion (ProvenanceComputation *p, char *tableName, int startPos, List *updates)
{
    ProvenanceTransactionInfo *tInfo = p->transactionInfo;
    int pos = startPos + 1;
    if (startPos + 1 >= LIST_LENGTH(tInfo->updateTableNames))
        return NULL;

    for(ListCell *lc = getNthOfList(tInfo->updateTableNames, startPos + 1); lc != NULL; lc = lc->next)
    {
        char *curTable = LC_STRING_VAL(lc);
        if (!strcmp(curTable, tableName))
            return (QueryOperator *) getNthOfListP(updates, pos);
        pos++;
    }

    return NULL;
}

void
restrictToUpdatedRows (ProvenanceComputation *op)
{
    boolean simpleOnly = TRUE;

    INFO_LOG("RESTRICT TO UPDATED ROWS");

    simpleOnly = !onlyUpdatedNeedsResultFiltering(op);

    DEBUG_LOG("is simple, %u", simpleOnly);

    // do we need to filter out non-updated rows in the end, if yes then record this since we have to
    // first apply the provenance rewrite first before we can do this
    if (!simpleOnly)
    {
        INFO_LOG("filtering of updated rows in final result required.");
        SET_BOOL_STRING_PROP(op, PROP_PC_REQUIRES_POSTFILTERING);
    }

    // if is simple and CBO is activated then make a choice between prefiltering and history join
    if (getBoolOption(OPTION_COST_BASED_OPTIMIZER))
    {
    	int res;

    	res = callback(2);

    	if (res == 1)
    		addConditionsToBaseTables(op);
   		else
   			extractUpdatedFromTemporalHistory(op);
    }
    // else check which option is checked
    else
    {
		// use conditions of updates to filter out non-updated tuples early on
		if (isRewriteOptionActivated(OPTION_UPDATE_ONLY_USE_CONDS))
		{
			INFO_LOG("Use update conditions to restrict to updated;");
			addConditionsToBaseTables(op);
		}
		// use history to get tuples updated by transaction and limit provenance tracing to these tuples
		else if (isRewriteOptionActivated(OPTION_UPDATE_ONLY_USE_HISTORY_JOIN))
		{
		    INFO_LOG("Use history join to restrict to updated;");
			extractUpdatedFromTemporalHistory(op);
		}
    }


    if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
        ASSERT(checkModel((QueryOperator *) op));
}

/**
 * Determines whether reenactment can be restricted to rows affected by a
 * transaction (ONLY UPDATED) by prefiltering or history join, or if
 * statement annotations (boolean attributes that are true if a tuple was
 * affected by particular statement) are required to filter the output of
 * reenactment. Return TRUE if filtering the output is required. The following
 * rules apply:
 *
 * 1) transaction has only updates and deletes: return FALSE
 * 2) if transaction has an INSERT INTO SELECT: return TRUE
 * 3) else the following applies:
 *  let R be the table for which we want to track provenance and R_1 to R_n
 *  the other tables updated by the input transaction T
 */

static boolean
onlyUpdatedNeedsResultFiltering (ProvenanceComputation *op)
{
    boolean result = TRUE;//TODO sometimes postfiltering is only required for some ops
    FOREACH(Node,up,op->transactionInfo->originalUpdates)
        result &= isSimpleUpdate(up);
    return !result;
}


/**
 * for inserts into tables that are not read or written afterwards we should not scan the table if the
 * user requested to only see rows affected by the transaction. If
 *
 * 1) the first statement affecting a table is an insert
 * 2) this table is only affected by INSERT INTO VALUES, INSERT INTO SELECT that do not access
 * the modified query in their query, and DELETES
 *
 * then we should replace the union between the table and
 * the insert's query or VALUES clause with the query (ConstRelOperator), e.g.,
 *
 *  R u {t} -> {t}
 *
 *  or
 *
 *  R u Q -> Q
 */
static void
removeInputTablesWithOnlyInserts (ProvenanceComputation *op)
{
    Set *tableUpdateOrRead = STRSET();
    Set *firstSeenTable = STRSET();

    // determine tables that are read by an INSERT query or updated
    FOREACH(Node, u, op->transactionInfo->originalUpdates)
    {
        char *tableName = NULL;

        if(isA(u,Insert))
        {
            Insert *i = (Insert *) u;
            tableName = i->insertTableName;

            if (!isA(i->query,  List))
            {
                List *tableFromItems = findAllNodes(i->query, T_FromTableRef);
                FOREACH(FromTableRef,f,tableFromItems)
                {
                    if (hasSetElem(firstSeenTable, f->tableId))
                        addToSet(tableUpdateOrRead, f->tableId);
                }
            }
        }
        else if (isA(u,Update))
        {
            Update *up = (Update *) u;
            tableName = up->updateTableName;
            addToSet(tableUpdateOrRead, up->updateTableName);
        }
        else if (isA(u, Delete))
        {
            Delete *d = (Delete *) u;
            tableName = d->deleteTableName;
            addToSet(tableUpdateOrRead, d->deleteTableName);
        }
        addToSet(firstSeenTable, tableName);
    }

    DEBUG_NODE_BEATIFY_LOG("tables that do not only consist of constants are", tableUpdateOrRead);

//TODO for transaction reenactment his is applied after merging which does not work RC-SI where projection is added, but properties are not moved to new top node

    for(int i = 0; i < LIST_LENGTH(op->op.inputs); i++)
    {
        List *tables = NIL;
        QueryOperator *child = (QueryOperator *) getNthOfListP(op->op.inputs, i);
        ReenactUpdateType t = INT_VALUE(GET_STRING_PROP(child,PROP_PROV_ORIG_UPDATE_TYPE));
        char *upTable = getNthOfListP(op->transactionInfo->updateTableNames, i);

        // if the first statement updating a table is an INSERT then we can replace the
        // union of the insert with its right input.
        // e.g., for INSERT INTO R VALUES (...), we can replace R union {t} with {t}
        if (t == UPDATE_TYPE_INSERT_VALUES)
        {
            findTableAccessVisitor((Node *) child, &tables);
            FOREACH(TableAccessOperator,ta,tables)
            {
                if (!hasSetElem(tableUpdateOrRead, upTable))
                {
                    QueryOperator *un = OP_FIRST_PARENT(ta);
                    QueryOperator *c = OP_RCHILD(un);
                    ASSERT(isA(un,SetOperator) && isA(c,ConstRelOperator));

                    c->parents = NIL;
                    switchSubtrees(un, c);
                    c->properties = un->properties; //copy properties
                    un->properties = NULL;
                    addToSet (tableUpdateOrRead, ta->tableName);
                }
            }
        }
        else if (t == UPDATE_TYPE_INSERT_QUERY)         //TODO deal with UPDATE_TYPE_INSERT_QUERY
        {
            TableAccessOperator *ta = (TableAccessOperator *) OP_LCHILD(child);
            ASSERT(isA(ta,TableAccessOperator));

            if (!hasSetElem(tableUpdateOrRead, upTable))
            {
                QueryOperator *un = OP_FIRST_PARENT(ta);
                QueryOperator *c = OP_RCHILD(un);
                ASSERT(isA(un,SetOperator));

                c->parents = NIL;
                switchSubtrees(un, c);
                c->properties = un->properties; //copy properties
            }
        }

        addToSet(tableUpdateOrRead, upTable);
    }
}

static void
addConditionsToBaseTables (ProvenanceComputation *op)
{
    List *upConds;
    List *tableNames;
    List *updatedTables;
    List *allTables = NIL;
    List *origUpdates;
    HashMap *tabCondMap = NEW_MAP(Constant,List);
    int pos = 0;
    Set *readFromTableNames = STRSET();
    Set *updatedTableNames = STRSET();
    Set *mixedTableNames = NULL;
    HashMap *numAttrs = NEW_MAP(Constant,Constant);
    ProvenanceTransactionInfo *t = op->transactionInfo;
    Constant *constT = createConstBool(TRUE);
    char *reenactTargetTable = GET_STRING_PROP_STRING_VAL(op, PROP_PC_TABLE);


    upConds = (List *) GET_STRING_PROP(op, PROP_PC_UPDATE_COND);
    tableNames = deepCopyStringList(t->updateTableNames);
//    reverseList(tableNames);
    origUpdates = t->originalUpdates;
    findTableAccessVisitor((Node *) op, &allTables); //HAO fetch all table accesses
    updatedTables  = findUpdatedTableAccceses (allTables);

    if (reenactTargetTable == NULL)
    {
        reenactTargetTable = (char *) getNthOfListP(t->updateTableNames,0);
    }
    DEBUG_LOG("only gather conditions of table <%s> choosen for provenance tracking", reenactTargetTable);

    // check which tables are updated and which tables are read accessed (e.g., in query)
    // only updated tables can be safely prefiltered
    FOREACH(TableAccessOperator,t,allTables)
    {
        if (!MAP_HAS_STRING_KEY(numAttrs,t->tableName))
        {
            MAP_ADD_STRING_KEY(numAttrs, t->tableName,
                    createConstInt(getNumAttrs((QueryOperator *) t) - 1));
        }

        if (HAS_STRING_PROP(t,PROP_TABLE_IS_UPDATED)) //HAO figure out which tables are read from
            addToSet(updatedTableNames, strdup(t->tableName));
        else
            addToSet(readFromTableNames, strdup(t->tableName));
    }

    // if we have no info about table then we need to create one
//    FOREACH(char,tab,tableNames)
//    {
//
//    }

    DEBUG_LOG("updated tables\n%s\nread tables\n%s", beatify(nodeToString(updatedTableNames)), beatify(nodeToString(readFromTableNames)));

    // create map from table name to condition (for update only tables)
    int i = 0;
    FORBOTH(void,name,up,tableNames,origUpdates)
    {
        // only care about updates
        if (isA(up,Update))
        {
            char *tableName = (char *) name;

            if(!hasSetElem(readFromTableNames,tableName) && hasSetElem(updatedTableNames, tableName)) //HAO in second loop this check
            {
                KeyValue *tableMap = MAP_GET_STRING_ENTRY(tabCondMap, tableName); // getMapCond(tableCondMap, tableName);
                Node *cond = copyObject((Node *) getNthOfListP(upConds, i)); //TODO correct?

                // for read committed we have to also check the version column to only
                // check the condition for rows versions that will be seen by an update
                if (t->transIsolation == ISOLATION_READ_COMMITTED && cond != NULL)
                {
                    Constant *scn = (Constant *) getNthOfListP(t->scns, i);
                    cond = (Node *) adaptConditionForReadCommitted(cond, scn,
                            INT_VALUE(MAP_GET_STRING(numAttrs, tableName)));
                }


                if (cond == NULL)
                {
                    cond = (Node *) createConstBool(TRUE);
                }

                if (tableMap == NULL)
                    MAP_ADD_STRING_KEY(tabCondMap, tableName, singleton(cond));
                else
                    tableMap->value = (Node *) appendToTailOfList((List *) tableMap->value, cond);
            }
            pos++;
        }
        i++;
    }

    DEBUG_NODE_BEATIFY_LOG("condition table map is:", tabCondMap);

    // add selections to only updated tables
    FOREACH(TableAccessOperator,t,updatedTables)
    {
        char *tableName = t->tableName;
        KeyValue *prop = MAP_GET_STRING_ENTRY(tabCondMap,tableName);
        Node *cond = prop ? prop->value : NULL;
        SelectionOperator *sel;

        DEBUG_LOG("selection conditions are: ", cond);

        if (streq(tableName,reenactTargetTable) &&  cond != NULL)
        {
            List *args = (List *) cond;
            boolean allTrue = FALSE;

            FOREACH(Node,c,args)
            {
                if (c == NULL || equal(c,constT))
                    allTrue = TRUE;
            }

            if (!allTrue)
            {
                if (LIST_LENGTH(args) > 1)
                    cond = (Node *) createOpExpr("OR", (List *) cond);
                else
                    cond = (Node *) getHeadOfListP(args);

                sel = createSelectionOp(cond, (QueryOperator *) t, NIL,
                        getAttrNames(GET_OPSCHEMA(t)));
                switchSubtrees((QueryOperator *) t, (QueryOperator *) sel);
                ((QueryOperator *) t)->parents = singleton(sel);
            }
        }
    }

    // if there are tables with mixed usage (updated and read from)
    // then we have to add additional conditions to post-filter out non-update rows
    // from these tables
    mixedTableNames = intersectSets(readFromTableNames,updatedTableNames);
    if (!EMPTY_SET(mixedTableNames))
        SET_BOOL_STRING_PROP(op, PROP_PC_REQUIRES_POSTFILTERING);
}

static Node *
adaptConditionForReadCommitted(Node *cond, Constant *scn, int attrPos)
{
    Node *result;

    result = AND_EXPRS (
            cond,
            createOpExpr("<=", LIST_MAKE(createFullAttrReference(
                    VERSIONS_STARTSCN_ATTR, 0, attrPos, INVALID_ATTR, scn->constType),
                    copyObject(scn)))
        );

    DEBUG_NODE_BEATIFY_LOG("adapted condition for read committed:", result);

    return result;
}

static List *
findUpdatedTableAccceses (List *tables)
{
    List *result = NIL;

    FOREACH(TableAccessOperator,t,tables)
        if (HAS_STRING_PROP(t,PROP_TABLE_IS_UPDATED))
            result = appendToTailOfList(result, t);

    return result;
}

static void
extractUpdatedFromTemporalHistory (ProvenanceComputation *op)
{
	Constant *scn = (Constant *) getHeadOfListP(op->transactionInfo->scns);
	Constant *scnC = copyObject(op->transactionInfo->commitSCN);
	Constant *xid = (Constant *) GET_STRING_PROP(op, PROP_PC_TRANS_XID);
//	TableAccessOperator *t;
//	List *updateTableNames;
	Set *readFromTableNames = STRSET();
	Set *updatedTableNames = STRSET();
	Set *mixedTableNames = STRSET();
	List *propValue = LIST_MAKE(xid, scn, scnC);
	List *allTables = NIL;

//	SET_STRING_PROP(t, PROP_USE_HISTORY_JOIN, copyObject(propValue));
	findTableAccessVisitor((Node *) op, &allTables);

	// check with tables we are reading from
	FOREACH(TableAccessOperator,t,allTables)
	{
	    if (!HAS_STRING_PROP(t,PROP_TABLE_IS_UPDATED))
	        addToSet(readFromTableNames, t->tableName);
	    else
	        addToSet(updatedTableNames, t->tableName);
	}

	// for tables that are only updated
	FOREACH(TableAccessOperator,t,allTables)
	{
        char *tableName = (char *) t->tableName;

        if (!hasSetElem(readFromTableNames, tableName))
        {
            SET_STRING_PROP(t, PROP_USE_HISTORY_JOIN, copyObject(propValue));
            if (op->transactionInfo->transIsolation == ISOLATION_READ_COMMITTED)
                SET_BOOL_STRING_PROP(t,PROP_IS_READ_COMMITTED);
        }
	}

	//TODO need to postfilter for remaining ones (store table names here?)
    mixedTableNames = intersectSets(readFromTableNames,updatedTableNames);
    if (!EMPTY_SET(mixedTableNames))
        SET_BOOL_STRING_PROP(op, PROP_PC_REQUIRES_POSTFILTERING);
}

QueryOperator *
filterUpdatedInFinalResult (ProvenanceComputation *op, QueryOperator *rewritten)
{
    //TODO will only work if called directly and annotation attributes have been added beforehand
    //TODO extend to support making it work as a fallback method, easiest solution would be to determine whether this is necessary early on
    // for each updated table add attribute that trackes whether the table has been updated

    // add final conditions that filters out rows
    SelectionOperator *sel;
    QueryOperator *top = rewritten;
    Node *cond;
    List *condList = NIL;
    int i = 0;

    FOREACH(AttributeDef,a,top->schema->attrDefs)
    {
        if (IS_STATEMENT_ANNOT_ATTR(a->attrName))
        {
            condList = appendToTailOfList(condList, createOpExpr("=",
                    LIST_MAKE(createFullAttrReference(strdup(a->attrName), 0, i,
                                    INVALID_ATTR, a->dataType),
                            createConstBool(TRUE))
                    ));
        }
        i++;
    }

    cond = orExprList(condList);
    DEBUG_NODE_BEATIFY_LOG("create condition: %s",cond);
    sel = createSelectionOp(cond, top, NIL, NIL);
    addParent(top, (QueryOperator *) sel);

    INFO_OP_LOG("added selection for postfiltering", sel);

    switchSubtrees(top, (QueryOperator *) sel);

    return (QueryOperator *) sel;
}


boolean
isSimpleUpdate(Node *update)
{
    // type of update (UPDATE / DELETE / INSERT)
    if (isA(update,Update))
    {
        Update *up = (Update *) update;
        return !hasNestedSubqueries(up->cond);
    }
    if (isA(update,Delete))
    {
        Delete *del = (Delete *) update;
        return !hasNestedSubqueries(del->cond);
    }
    if (isA(update,Insert))
    {
        Insert *in = (Insert *) update;
        return (isA(in->query,  List));
    }
    FATAL_LOG("Expected an update node");
    return FALSE;
}

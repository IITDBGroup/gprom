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

#include "analysis_and_translate/analyze_qb.h"
#include "model/node/nodetype.h"
#include "model/set/set.h"
#include "model/set/hashmap.h"
#include "model/expression/expression.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/query_operator_model_checker.h"
#include "model/query_operator/operator_property.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/update_and_transaction/prov_update_and_transaction.h"
#include "configuration/option.h"

static QueryOperator *getUpdateForPreviousTableVersion (ProvenanceComputation *p, char *tableName, int startPos, List *updates);

static void mergeSerializebleTransaction(ProvenanceComputation *op);
static void mergeReadCommittedTransaction(ProvenanceComputation *op);

static void addConditionsToBaseTables (ProvenanceComputation *op);
static void extractUpdatedFromTemporalHistory (ProvenanceComputation *op);
static void filterUpdatedInFinalResult (ProvenanceComputation *op);
static Node *adaptConditionForReadCommitted(Node *cond, Constant *scn, int attrPos);

static List *findUpdatedTableAccceses (List *tables);

static KeyValue *getMapCond (List *props, char *key);
static void setMapCond (List **props, KeyValue *newVal);


void mergeUpdateSequence(ProvenanceComputation *op) {
	ProvenanceTransactionInfo *tInfo = op->transactionInfo;

    if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
        ASSERT(checkModel((QueryOperator *) op));

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
mergeSerializebleTransaction(ProvenanceComputation *op)
{
    List *updates = copyList(op->op.inputs);
    int i = 0;
    char *useTable = NULL;

    // cut links to parent
    removeParentFromOps(op->op.inputs, (QueryOperator *) op);
    op->op.inputs = NIL;

    // reverse list
    reverseList(updates);
    reverseList(op->transactionInfo->updateTableNames);

    DEBUG_LOG("Updates to merge are: \n\n%s", beatify(nodeToString(updates)));
    INFO_LOG("Updates to merge overview are: \n\n%s", operatorToOverviewString((Node *) updates));
    /*
     * Merge the individual queries for all updates into one
     */
    FOREACH(QueryOperator, u, updates)
    {
         List *children = NULL;

         // find all table access operators
         findTableAccessVisitor((Node *) u, &children);
         INFO_LOG("Replace table access operators in %s", operatorToOverviewString((Node *) u));

         FOREACH(TableAccessOperator, t, children)
         {
             INFO_LOG("\tTable Access %s", operatorToOverviewString((Node *) t));
             QueryOperator *up = getUpdateForPreviousTableVersion(op,
                     t->tableName, i, updates);

             INFO_LOG("\tUpdate is %s", operatorToOverviewString((Node *) up));
             // previous table version was created by transaction
             if (up != NULL)
                 switchSubtreeWithExisting((QueryOperator *) t, up);
             // previous table version is the one at transaction begin
             else
             {
                 Constant *startScn = copyObject((Constant *) getHeadOfListP(op->transactionInfo->scns));

                 // if user provenance attribute
                 if (HAS_STRING_PROP(t,PROP_TABLE_USE_ROWID_VERSION))
                 {
                     t->asOf = (Node *) LIST_MAKE(startScn, copyObject(startScn));
                 }
                 else
                     t->asOf = (Node *) startScn;
             }

             INFO_LOG("Table after merge %s", operatorToOverviewString((Node *) u));
         }
         i++;
    }
    DEBUG_LOG("Merged updates are: %s", beatify(nodeToString(updates)));

    // replace updates sequence with root of the whole merged update query
    addChildOperator((QueryOperator *) op, (QueryOperator *) getHeadOfListP(updates));
    //op->op.inputs = singleton();
    DEBUG_LOG("Provenance computation for updates that will be passed "
            "to rewriter: %s", beatify(nodeToString(op)));
}

static void
mergeReadCommittedTransaction(ProvenanceComputation *op)
{
	List *scns = op->transactionInfo->scns;
    int i = 0;
    QueryOperator *mergeRoot = NULL;
    QueryOperator *finalProj = NULL;

    removeParentFromOps(op->op.inputs, (QueryOperator *) op);

	// Loop through update translations and add version_startscn condition + attribute
	FORBOTH_LC(uLc, trLc, op->transactionInfo->originalUpdates, op->op.inputs)
	{
	    QueryOperator *q = (QueryOperator *) LC_P_VAL(trLc);
	    Node *u = (Node *) LC_P_VAL(uLc);

		// use original update to figure out type of each update (UPDATE/DELETE/INSERT)
		// switch
		switch (u->type) {
		// case t_InsertStmt:t:
		case T_Insert:
			break;

			// case T_DeleteStmt:
		case T_Delete:
			break;

			// case T_UpdateStmt:

		case T_Update:
			// either CASE translation OR union translation
			//if its case translation
			if (isA(q,ProjectionOperator))
			{
                Node *newWhen = NULL;
			    ProjectionOperator *proj = (ProjectionOperator *) q;
				List *projExprs = proj->projExprs;
				Node *newProjExpr;

				//Add SCN foreach CaseEpr
				FOREACH(Node, expr, projExprs)
				{
					if(isA(expr,CaseExpr))
				    {
					    AttributeReference *scnAttr;
					    CaseExpr *cexp = (CaseExpr *) expr;
					    CaseWhen *whenC = (CaseWhen *) getNthOfListP(cexp->whenClauses, 0);
						Node *when = whenC->when;
						Node *newCond;

						DEBUG_LOG("Deal with case: %s", exprToSQL((Node *) cexp));

						// adding SCN < update SCN condition
						scnAttr = createFullAttrReference("VERSIONS_STARTSCN", 0,
						        getNumAttrs(OP_LCHILD(q)), INVALID_ATTR);
						newCond = (Node *) createOpExpr("<=",
								LIST_MAKE((Node *) scnAttr,
								        copyObject(getNthOfListP(scns,i))));

						newWhen = andExprs(when, newCond);
						whenC->when = newWhen;
						DEBUG_LOG("Updated case is: %s", exprToSQL((Node *) cexp));
				    }
				}

               //make new case for SCN
                Node *then = (Node *) createConstLong(-1);
                Node *els = (Node *) createFullAttrReference("VERSIONS_STARTSCN", 0, getNumAttrs(OP_LCHILD(q)), INVALID_ATTR);
                CaseExpr *caseExpr;
                CaseWhen *caseWhen;

                caseWhen = createCaseWhen(copyObject(newWhen), then);
                caseExpr = createCaseExpr(NULL, singleton(caseWhen),
                        els);

                // TODO do not modify the SCN attribute to avoid exponentail expression size blow-up
                newProjExpr = (Node *) els; // caseExpr
                proj->projExprs =
                        appendToTailOfList(projExprs, newProjExpr);
                q->schema->attrDefs = appendToTailOfList(q->schema->attrDefs,
                        createAttributeDef("VERSIONS_STARTSCN", DT_LONG));
			}
			break;
		default:
			break;

		}

        i++;
	}

	List *updates = copyList(op->op.inputs);
	i = 0;

	// cut links to parent
	op->op.inputs = NIL;

	// reverse list
	reverseList(updates);
	reverseList(op->transactionInfo->updateTableNames);

	DEBUG_LOG("Updates to merge are: \n\n%s", beatify(nodeToString(updates)));
	INFO_LOG("Updates to merge overview are: \n\n%s",
			operatorToOverviewString((Node *) updates));
	/*
	 * Merge the individual queries for all updates into one
	 */
	FOREACH(QueryOperator, u, updates) {
		List *children = NULL;

		// find all table access operators
		findTableAccessVisitor((Node *) u, &children);
		INFO_LOG("Replace table access operators in %s",
				operatorToOverviewString((Node *) u));

		FOREACH(TableAccessOperator, t, children) {
			INFO_LOG("\tTable Access %s", operatorToOverviewString((Node *) t));
			QueryOperator *up = getUpdateForPreviousTableVersion(op,
					t->tableName, i, updates);

			INFO_LOG("\tUpdate is %s", operatorToOverviewString((Node *) up));
			// previous table version was created by transaction
			if (up != NULL)
				switchSubtreeWithExisting((QueryOperator *) t, up);
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
                            createAttributeDef("VERSIONS_STARTSCN", DT_LONG));
			    }

                t->asOf = (Node *) LIST_MAKE(scnC, copyObject(scnC));
			}

			DEBUG_LOG("\tTable after merge %s",
					operatorToOverviewString((Node *) u));
		}
		i++;
	}

	// add projection that removes the VERSIONS_STARTSCN attribute
	List *finalAttrs, *mergeAttrs, *projExprs = NIL;
	int cnt = 0;

	mergeRoot = (QueryOperator *) getHeadOfListP(updates);
	mergeAttrs = getQueryOperatorAttrNames(mergeRoot);
	finalAttrs = sublist(mergeAttrs, 0, LIST_LENGTH(mergeAttrs) - 1);

    FOREACH(AttributeDef, attr, mergeRoot->schema->attrDefs)
    {
        if (strcmp(attr->attrName,"VERSIONS_STARTSCN") != 0)
            projExprs = appendToTailOfList(projExprs, createFullAttrReference(attr->attrName, 0, cnt, 0));
        cnt++;
    }

	finalProj = (QueryOperator *) createProjectionOp(projExprs, mergeRoot, NIL, finalAttrs);
	mergeRoot->parents = singleton(finalProj);

	INFO_LOG("Merged updates are: %s", operatorToOverviewString((Node *) finalProj));
	DEBUG_LOG("Merged updates are: %s", beatify(nodeToString((Node *) finalProj)));

	// replace updates sequence with root of the whole merged update query
	addChildOperator((QueryOperator *) op, finalProj);
	DEBUG_LOG("Provenance computation for updates that will be passed "
	        "to rewriter: %s", beatify(nodeToString(op)));
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
    ProvenanceTransactionInfo *t = op->transactionInfo;

    INFO_LOG("RESTRICT TO UPDATED ROWS");

    FOREACH(Node,up,t->originalUpdates)
        simpleOnly &= isSimpleUpdate(up);

    //TODO for now be conservative when to apply things
    // use conditions of updates to filter out non-updated tuples early on
    if (isRewriteOptionActivated(OPTION_UPDATE_ONLY_USE_CONDS) && simpleOnly)
        addConditionsToBaseTables(op);
    // use history to get tuples updated by transaction and limit provenance tracing to these tuples
    else if (isRewriteOptionActivated(OPTION_UPDATE_ONLY_USE_HISTORY_JOIN))
        extractUpdatedFromTemporalHistory(op);
    // simply filter out non-updated rows in the end
    else
        filterUpdatedInFinalResult(op);

    if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
        ASSERT(checkModel((QueryOperator *) op));
}

static void
addConditionsToBaseTables (ProvenanceComputation *op)
{
    List *upConds;
    List *tableNames;
    List *updatedTables;
    List *allTables = NIL;
    List *origUpdates;
    List *tableCondMap = NIL;
    int pos = 0;
    KeyValue *tableCond;
    Set *readFromTableNames = STRSET();
    Set *updatedTableNames = STRSET();
    HashMap *numAttrs = NEW_MAP(Constant,Constant);
    ProvenanceTransactionInfo *t = op->transactionInfo;

    upConds = (List *) GET_STRING_PROP(op, PROP_PC_UPDATE_COND);
    tableNames = t->updateTableNames;
    origUpdates = t->originalUpdates;
    findTableAccessVisitor((Node *) op, &allTables); //HAO fetch all table accesses
    updatedTables  = findUpdatedTableAccceses (allTables);

    DEBUG_LOG("\ncond: %s, \ntables: %s, \nupdatedTable: %s",
            beatify(nodeToString(upConds)),
            stringListToString(tableNames),
            beatify(nodeToString(updatedTables)));

    // check which tables are updated and which tables are read accessed (e.g., in query)
    // only updated tables can be safely prefiltered
    FOREACH(TableAccessOperator,t,allTables)
    {
        if (!MAP_HAS_STRING_KEY(numAttrs,t->tableName))
        {
            MAP_ADD_STRING_KEY(numAttrs, t->tableName,
                    createConstInt(getNumAttrs((QueryOperator *) t) - 1));
        }

        if (HAS_STRING_PROP(t,"UPDATED TABLE")) //HAO figure out which tables are read from
            addToSet(updatedTableNames, strdup(t->tableName));
        else
            addToSet(readFromTableNames, strdup(t->tableName));
    }

    // create map from table name to condition (for update only tables)
    int i = 0;
    FORBOTH(void,name,up,tableNames,origUpdates)
    {
        // only care about updates
        if (isA(up,Update))
        {
            char *tableName = (char *) name;

            if(!hasSetElem(readFromTableNames,tableName)) //HAO in second loop this check
            {
                KeyValue *tableMap = getMapCond(tableCondMap, tableName);
                Node *cond = copyObject((Node *) getNthOfListP(upConds, pos));

                // for read committed we have to also check the version column to only
                // check the condition for rows versions that will be seen by an update
                if (t->transIsolation == ISOLATION_READ_COMMITTED)
                {
                    Constant *scn = (Constant *) getNthOfListP(t->scns, i);
                    cond = (Node *) adaptConditionForReadCommitted(cond, scn,
                            INT_VALUE(MAP_GET_STRING(numAttrs, tableName)));
                }

                if (tableMap == NULL)
                {
                    tableMap = createNodeKeyValue((Node *) createConstString(tableName),
                            (Node *) singleton(cond));
                    setMapCond(&tableCondMap, tableMap);
                }
                else
                    tableMap->value = (Node *) appendToTailOfList((List *) tableMap->value, cond);
            }
            pos++;
            i++;
        }
    }

    DEBUG_LOG("condition table map is:\n%s", tableCondMap);

    // add selections to only updated tables
    FOREACH(TableAccessOperator,t,updatedTables)
    {
        char *tableName = t->tableName;
        KeyValue *prop = getMapCond(tableCondMap, tableName);
        Node *cond = prop ? prop->value : NULL;
        SelectionOperator *sel;

        DEBUG_LOG("selection conditions are: ", cond);

        if (cond != NULL)
        {
            List *args = (List *) cond;

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

    // if there are tables with mixed usage (updated and read from)
    // then we have to add additional conditions to post-filter out non-update rows
    // from these tables
//    FOREACH
}

static Node *
adaptConditionForReadCommitted(Node *cond, Constant *scn, int attrPos)
{
    Node *result;

    result = AND_EXPRS (
            cond,
            createOpExpr("<=", LIST_MAKE(createFullAttrReference(
                    "VERSIONS_STARTSCN", 0, attrPos, INVALID_ATTR),
                    copyObject(scn)))
        );

    DEBUG_LOG("adapted condition for read committed: %s", beatify(nodeToString(result)));

    return result;
}

static KeyValue *
getMapCond (List *props, char *key)
{
    FOREACH(KeyValue,kv,props)
    {
        if (strcmp(key,STRING_VALUE(kv->key)) == 0)
            return kv;
    }

    return NULL;
}

static void
setMapCond (List **props, KeyValue *newVal)
{
    *props = appendToTailOfList(*props, newVal);
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
	TableAccessOperator *t;
//	List *updateTableNames;
	Set *readFromTableNames = STRSET();
	List *propValue = LIST_MAKE(xid, scn, scnC);
	List *allTables = NIL;

	SET_STRING_PROP(t, PROP_USE_HISTORY_JOIN, copyObject(propValue));
	findTableAccessVisitor((Node *) op, &allTables);

	// check with tables we are reading from
	FOREACH(TableAccessOperator,t,allTables)
	{
	    if (!HAS_STRING_PROP(t,PROP_TABLE_IS_UPDATED))
	        addToSet(readFromTableNames, t->tableName);
	}

	// for tables that are only updated
	FOREACH(TableAccessOperator,t,allTables)
	{
        char *tableName = (char *) t->tableName;

        if (!hasSetElem(readFromTableNames, tableName))
            SET_STRING_PROP(t, PROP_USE_HISTORY_JOIN, copyObject(propValue));
	}



}

static void
filterUpdatedInFinalResult (ProvenanceComputation *op)
{

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

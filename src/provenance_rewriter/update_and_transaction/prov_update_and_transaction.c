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
#include "model/expression/expression.h"
#include "model/query_operator/query_operator.h"
#include "provenance_rewriter/prov_utility.h"
#include "provenance_rewriter/update_and_transaction/prov_update_and_transaction.h"
#include "configuration/option.h"

static QueryOperator *getUpdateForPreviousTableVersion (ProvenanceComputation *p, char *tableName, int startPos, List *updates);

static void mergeSerializebleTransaction(ProvenanceComputation *op);
static void mergeReadCommittedTransaction(ProvenanceComputation *op);

static void addConditionsToBaseTables (ProvenanceComputation *op);
static void extractUpdatedFromTemporalHistory (ProvenanceComputation *op);
static void filterUpdatedInFinalResult (ProvenanceComputation *op);

static List *findUpdatedTableAccceses (Node *op);
static KeyValue *getMapCond (List *props, char *key);
static void setMapCond (List **props, KeyValue *newVal);


void mergeUpdateSequence(ProvenanceComputation *op) {
	ProvenanceTransactionInfo *tInfo = op->transactionInfo;

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

	INFO_LOG("updates after merge:\n%s", operatorToOverviewString((Node *) op));
}

static void
mergeSerializebleTransaction(ProvenanceComputation *op)
{
    List *updates = copyList(op->op.inputs);
    int i = 0;

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
                 t->asOf = (Node *) getHeadOfListP(op->transactionInfo->scns);

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

static void mergeReadCommittedTransaction(ProvenanceComputation *op) {
	List *scns = op->transactionInfo->scns;

	// Loop through update translations and add version_startscn condition + attribute
	FOREACH(QueryOperator,u,op->op.inputs)
	{
	    int i = 0;
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
			if (isA(u,ProjectionOperator))
			{
                Node *newWhen = NULL;
			    ProjectionOperator *proj = (ProjectionOperator *) u;
				List *projExprs = proj->projExprs;
				Node *newProjExpr;

				projExprs = getHeadOfListP(u->inputs);

				//Add SCN foreach CaseEpr
				FOREACH(Node, expr, projExprs)
				{
					if(isA(expr,CaseExpr))
				    {
					    AttributeReference *scnAttr;
					    CaseExpr *cexp = (CaseExpr *) expr;
						Node *when = ((CaseWhen *) (cexp->whenClauses))->when;
						Node *newCond;

						// adding SCN < update SCN condition
						scnAttr = createFullAttrReference("VERSION_STARTSCN", 0,
						        getNumAttrs(OP_LCHILD(u)), INVALID_ATTR);
						newCond = (Node *) createOpExpr("<=",
								LIST_MAKE((Node *) scnAttr,
								        copyObject(getNthOfList(scns,i))));

						newWhen = andExprs(when, newCond);
						((CaseWhen *) (cexp->whenClauses))->when = newWhen;
				    }
				}

               //make new case for SCN
                Node *then = (Node *) createConstLong(-1);
                Node *els = (Node *) createFullAttrReference("VERSION_STARTSCN", 0, getNumAttrs(OP_LCHILD(u)), INVALID_ATTR);
                CaseExpr *caseExpr;
                CaseWhen *caseWhen;

                caseWhen = createCaseWhen(copyObject(newWhen), then);
                caseExpr = createCaseExpr(NULL, singleton(caseWhen),
                        els);

                newProjExpr = (Node *) caseExpr;
                proj->projExprs =
                        appendToTailOfList(projExprs, newProjExpr);
                u->schema->attrDefs = appendToTailOfList(u->schema->attrDefs,
                        createAttributeDef("VERSION_STARTSCN", DT_LONG));
			}
			break;

		default:
			break;

		}

        i++;
	}

	List *updates = copyList(op->op.inputs);
	int i = 0;

	// cut links to parent
	//removeParentFromOps(op->op.inputs, (QueryOperator *) op);

	op->op.inputs = NIL;

	// reverse list
	reverseList(updates);
	reverseList(op->transactionInfo->updateTableNames);

	DEBUG_LOG("Updates to merge are: \n\n%s", beatify(nodeToString(updates)));
	INFO_LOG("Updates to merge overview are: \n\n%s",
			operatorToOverviewString((Node *) updates));
	/*
	 * Merge the individual queries for all updates into one
	 */FOREACH(QueryOperator, u, updates) {
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
			    Node *scn = (Node *) getTailOfListP(op->transactionInfo->scns);
			    Constant *scnC = (Constant *) copyObject(scnC);
			    *((long *) scnC->value) = *((long *) scnC->value) + 1; //getCommit SCN
				t->asOf = (Node *) LIST_MAKE(scnC, copyObject(scnC));
				((QueryOperator *) t)->schema->attrDefs = appendToTailOfList(((QueryOperator *) t)->schema->attrDefs,
				        createAttributeDef("VERSIONS_STARTSCN", DT_LONG));
			}

			INFO_LOG("\tTable after merge %s",
					operatorToOverviewString((Node *) u));
		}
		i++;
	}
	DEBUG_LOG("Merged updates are: %s", beatify(nodeToString(updates)));

	// replace updates sequence with root of the whole merged update query
	addChildOperator((QueryOperator *) op,
			(QueryOperator *) getHeadOfListP(updates));
	//op->op.inputs = singleton();
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
    if (isRewriteOptionActivated("only_updated_use_conditions") && simpleOnly)
        addConditionsToBaseTables(op);
    // use history to get tuples updated by transaction and limit provenance tracing to these tuples
    else if (isRewriteOptionActivated("only_updated_use_history"))
        extractUpdatedFromTemporalHistory(op);
    // simply filter out non-updated rows in the end
    else
        filterUpdatedInFinalResult(op);
}

static void
addConditionsToBaseTables (ProvenanceComputation *op)
{
    List *upConds;
    List *tableNames;
    List *updatedTables;
    List *origUpdates;
    List *tableCondMap = NIL;
    int pos = 0;
    KeyValue *tableCond;

    ProvenanceTransactionInfo *t = op->transactionInfo;

    upConds = (List *) GET_STRING_PROP(op, "UpdateConds");
    tableNames = t->updateTableNames;
    origUpdates = t->originalUpdates;
    updatedTables  = findUpdatedTableAccceses ((Node *) op);

    DEBUG_LOG("\ncond: %s, \ntables: %s, \nupdatedTable: %s",
            nodeToString(upConds),
            stringListToString(tableNames),
            nodeToString(updatedTables));

    // create map from table name to condition
    FORBOTH(void,name,up,tableNames,origUpdates)
    {
        // only care about updates
        if (isA(up,Update))
        {
            char *tableName = (char *) name;
            KeyValue *tableMap = getMapCond(tableCondMap, tableName);
            Node *cond = copyObject((Node *) getNthOfListP(upConds, pos++));

            if (tableMap == NULL)
            {
                tableMap = createNodeKeyValue((Node *) createConstString(tableName), cond);
                setMapCond(&tableCondMap, tableMap);
            }
            else
                tableMap->value = (Node *) OR_EXPRS(tableMap->value, cond);
        }
    }

    DEBUG_LOG("condtion table map is:\n%s", tableCondMap);

    // add selections
    FOREACH(TableAccessOperator,t,updatedTables)
    {
        char *tableName = t->tableName;
        KeyValue *prop = getMapCond(tableCondMap, tableName);
        Node *cond = prop ? prop->value : NULL;
        SelectionOperator *sel;

        DEBUG_LOG("selection conditions are: ", cond);

        if (cond != NULL)
        {
            sel = createSelectionOp(cond, (QueryOperator *) t, NIL,
                    getAttrNames(GET_OPSCHEMA(t)));
            switchSubtrees((QueryOperator *) t, (QueryOperator *) sel);
            ((QueryOperator *) t)->parents = singleton(sel);
        }
    }
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
findUpdatedTableAccceses (Node *op)
{
    List *tables = NIL;
    List *result = NIL;

    findTableAccessVisitor(op, &tables);
    FOREACH(TableAccessOperator,t,tables)
        if (HAS_STRING_PROP(t,"UPDATED TABLE"))
            result = appendToTailOfList(result, t);

    return result;
}

static void
extractUpdatedFromTemporalHistory (ProvenanceComputation *op)
{

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

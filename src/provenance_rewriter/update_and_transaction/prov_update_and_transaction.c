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

static QueryOperator *getUpdateForPreviousTableVersion (ProvenanceComputation *p, char *tableName, int startPos, List *updates);

static void mergeSerializebleTransaction(ProvenanceComputation *op);
static void mergeReadCommittedTransaction(ProvenanceComputation *op);

static void addConditionsToBaseTables (ProvenanceComputation *op);
static void extractUpdatedFromTemporalHistory (ProvenanceComputation *op);
static void filterUpdatedInFinalResult (ProvenanceComputation *op);
static Node *adaptConditionForReadCommitted(Node *cond, Constant *scn, int attrPos);

static List *findUpdatedTableAccceses (List *tables);

static void addUpdateAnnotationAttrs (ProvenanceComputation *op);
static void addAnnotConstToUnion (QueryOperator *un, boolean leftIsTrue, char *annotName);


void
mergeUpdateSequence(ProvenanceComputation *op)
{
	ProvenanceTransactionInfo *tInfo = op->transactionInfo;
	boolean addAnnotAttrs = GET_BOOL_STRING_PROP(op, PROP_PC_STATEMENT_ANNOTATIONS) ||
	         !(isRewriteOptionActivated(OPTION_UPDATE_ONLY_USE_CONDS)
	        || isRewriteOptionActivated(OPTION_UPDATE_ONLY_USE_HISTORY_JOIN)
	        || getBoolOption(OPTION_COST_BASED_OPTIMIZER));

    if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
        ASSERT(checkModel((QueryOperator *) op));

    // add boolean attributes to store whether update did modify a row
    if (addAnnotAttrs)
        addUpdateAnnotationAttrs (op);

    //TODO add projection to remove update annot attribute
    QueryOperator *lastUp = (QueryOperator *) getTailOfListP(op->op.inputs);
    List *normalAttrs = NIL;
    CREATE_INT_SEQ(normalAttrs, 0, getNumNormalAttrs(lastUp) - 2, 1);
    DEBUG_LOG("num attrs %i", getNumNormalAttrs(lastUp) - 2);

    QueryOperator *newTop = createProjOnAttrs(lastUp, normalAttrs);
    newTop->inputs = LIST_MAKE(lastUp);
    switchSubtrees(lastUp, newTop);
    lastUp->parents = LIST_MAKE(newTop);

    INFO_LOG("after adding projection:\n%s", operatorToOverviewString((Node *) op));

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
addUpdateAnnotationAttrs (ProvenanceComputation *op)
{
    int i = 0;

    // add projection for each update to create the update attribute
    FORBOTH_LC(uLc, trLc, op->transactionInfo->originalUpdates, op->op.inputs)
    {
        QueryOperator *q = (QueryOperator *) LC_P_VAL(trLc);
        Node *u = (Node *) LC_P_VAL(uLc);
//        Node *annotAttr;
        char *annotName;

        switch (u->type) {
            case T_Insert:
                annotName = strdup("ins");
                break;
            case T_Update:
                annotName = strdup("up");
                break;
            case T_Delete:
                annotName = strdup("del");
                break;
            default:
                FATAL_LOG("expected insert, update, or delete");
        }

        // mark update annotation attribute as provenance
        SET_STRING_PROP(q, PROP_ADD_PROVENANCE, LIST_MAKE(createConstString(annotName)));
        SET_STRING_PROP(q, PROP_PROV_REL_NAME, createConstString(CONCAT_STRINGS("STATEMENT", itoa(i))));

        // use original update to figure out type of each update (UPDATE/DELETE/INSERT)
        // switch
        switch (u->type) {
            case T_Insert:
                addAnnotConstToUnion(q, FALSE, annotName);
            break;
            case T_Update:
            {
//                Update *up = (Update *) u;

                // if update CASE translation was used
                if (isA(q,ProjectionOperator))
                {
                    Node *annotAttr;
                    ProjectionOperator *p = (ProjectionOperator *) q;
                    Node *cond = getNthOfListP(
                            (List *) GET_STRING_PROP(op, PROP_PC_UPDATE_COND), i);

                    // add proj expr CASE WHEN C THEN TRUE ELSE FALSE END
                    annotAttr = (Node *) createCaseExpr(NULL,
                            LIST_MAKE(createCaseWhen(cond,
                                    (Node *) createConstBool(TRUE))),
                            (Node *) createConstBool(FALSE));
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
mergeSerializebleTransaction(ProvenanceComputation *op)
{
    List *updates = copyList(op->op.inputs);
    int i = 0;
//    char *useTable = NULL;

    // cut links to parent
    removeParentFromOps(op->op.inputs, (QueryOperator *) op);
    op->op.inputs = NIL;

    // reverse list
    reverseList(updates);
    reverseList(op->transactionInfo->updateTableNames);

//    if(HAS_STRING_PROP(op,PROP_PC_TABLE))
//    {
//        Constant *value = (Constant *) GET_STRING_PROP(op,PROP_PC_TABLE);
////        char *v = STRING_VALUE(value);
////        char *v2 = STRING_VALUE(GET_STRING_PROP(op,PROP_PC_TABLE));
//    }

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

    if(HAS_STRING_PROP(op,PROP_PC_TABLE))   // check if user asks for specific table
    {
    	char *temp = STRING_VALUE(GET_STRING_PROP(op,PROP_PC_TABLE));
    	QueryOperator *up = getUpdateForPreviousTableVersion(op,   //Find last update to that table
                temp, -1, updates);
    	if (up == NULL)  // NULL then user has asked for non-existing table
    	{
    		FATAL_LOG("table"); //exit
    	}

    	//If not then connect the root to the table that the user wants
    	addChildOperator((QueryOperator *) op, up);

    }
    else
    {
    	// Else do the normal stuff
    	addChildOperator((QueryOperator *) op, (QueryOperator *) getHeadOfListP(updates));
    }
    // else find last update to that table
    //getUpdateForPreviousTableVersion(op,THE_TABLE_NAME, 0, updates);
    // if NULL then user has asked for non-existing table
    // FATAL_LOG("table); - exit
    if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
        ASSERT(checkModel((QueryOperator *) op));

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
		{
		    //TODO deal with
		    QueryOperator *newQ = isA(q, ProjectionOperator) ? OP_LCHILD(q) : q;
            ProjectionOperator *lC = (ProjectionOperator *) OP_LCHILD(newQ);
//		    TableAccessOperator *t = (TableAccessOperator *) OP_LCHILD(lC);
		    ProjectionOperator *rC = (ProjectionOperator *) OP_RCHILD(newQ);
		    QueryOperator *qRoot = OP_LCHILD(rC);
		    ProjectionOperator *p;

            // is R UNION INSERTS transform into R + SCN UNION PROJECTION [*, SCN] (q)
		    lC->op.schema->attrDefs = appendToTailOfList(lC->op.schema->attrDefs,
		                        createAttributeDef("VERSIONS_STARTSCN", DT_LONG));
		    lC->projExprs = appendToTailOfList(lC->projExprs,
		            createFullAttrReference("VERSIONS_STARTSCN", 0,
		                    getNumAttrs(OP_LCHILD(lC)), INVALID_ATTR,
		                    DT_LONG));

		    rC->op.schema->attrDefs = appendToTailOfList(rC->op.schema->attrDefs,
		            createAttributeDef("VERSIONS_STARTSCN", DT_LONG));
		    rC->projExprs = appendToTailOfList(rC->projExprs,
		            copyObject(getNthOfListP(scns,i)));

		    // add attributes to union and table access
		    newQ->schema->attrDefs = appendToTailOfList(newQ->schema->attrDefs,
		            createAttributeDef("VERSIONS_STARTSCN", DT_LONG));

		    // add projection over query, add constant SCN attr, switch with query
		    if (!isA(q, ProjectionOperator))
		    {
                p = (ProjectionOperator *) createProjOnAllAttrs(qRoot);
                addChildOperator((QueryOperator *) p,qRoot);

                p->op.schema->attrDefs = appendToTailOfList(p->op.schema->attrDefs,
                        createAttributeDef("VERSIONS_STARTSCN", DT_LONG));
                p->projExprs = appendToTailOfList(p->projExprs,copyObject(getNthOfListP(scns,i)));

                switchSubtrees(qRoot,(QueryOperator *) p);
		    }

		    // add projection over table access operators to remove SCN attribute that will be added later
	        List *children = NULL;

	        // find all table access operators
	        findTableAccessVisitor((Node *) qRoot, &children);
	        INFO_LOG("Replace table access operators in %s",
	                operatorToOverviewString((Node *) q));

	        FOREACH(TableAccessOperator, t, children)
	        {
	            ProjectionOperator *po = (ProjectionOperator *) createProjOnAllAttrs((QueryOperator *) t);
	            addChildOperator((QueryOperator *) po, (QueryOperator *) t);
	            switchSubtrees((QueryOperator *) t,(QueryOperator *) po);
	        }
		}
        break;
        // case T_DeleteStmt:
		case T_Delete:
		{
            AttributeReference *scnAttr;
            Node *newCond;
            SelectionOperator *s = (SelectionOperator *) q;

		    // assume it is selection over input (for new translation has to be adapted)
		    ASSERT(isA(q,SelectionOperator));

		    // add SCN attribute to schema and turn NOT(C) into NOT(C) OR SCN > X to selection condition
            DEBUG_LOG("Deal with condition: %s", exprToSQL((Node *) s->cond));

            // adding SCN < update SCN condition
            scnAttr = createFullAttrReference("VERSIONS_STARTSCN", 0,
                    getNumAttrs(OP_LCHILD(q)), INVALID_ATTR, DT_LONG);
            newCond = (Node *) createOpExpr("<=",
                    LIST_MAKE((Node *) scnAttr,
                            copyObject(getNthOfListP(scns,i))));
            s->cond = OR_EXPRS(s->cond, newCond);

		    q->schema->attrDefs = appendToTailOfList(q->schema->attrDefs,
		                          createAttributeDef("VERSIONS_STARTSCN", DT_LONG));
		}
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
				boolean annotProj = isA(OP_LCHILD(proj), ProjectionOperator);

				// just modify schema of outer projection
				if (annotProj)
				{
	                proj->projExprs = appendToTailOfList(projExprs,
	                        (Node *) createFullAttrReference("VERSIONS_STARTSCN",
	                                0,
	                                getNumAttrs(OP_LCHILD(q)),
	                                INVALID_ATTR,
	                                DT_LONG));
	                q->schema->attrDefs = appendToTailOfList(q->schema->attrDefs,
	                        createAttributeDef("VERSIONS_STARTSCN", DT_LONG));

	                proj = (ProjectionOperator *) OP_LCHILD(proj);
	                projExprs = proj->projExprs;
				}

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
						        getNumAttrs(OP_LCHILD(q)), INVALID_ATTR, DT_LONG);
						newCond = (Node *) createOpExpr("<=",
								LIST_MAKE((Node *) scnAttr,
								        copyObject(getNthOfListP(scns,i))));

						newWhen = ((when == NULL) || equal(when,createConstBool(TRUE))) ?  newCond : AND_EXPRS(when, newCond);
						whenC->when = newWhen;
						DEBUG_LOG("Updated case is: %s", exprToSQL((Node *) cexp));
				    }
				}

               //make new case for SCN
                Node *els = (Node *) createFullAttrReference("VERSIONS_STARTSCN", 0, getNumAttrs(OP_LCHILD(proj)), INVALID_ATTR, DT_LONG);

                // TODO do not modify the SCN attribute to avoid exponential expression size blow-up
                newProjExpr = (Node *) els; // caseExpr
                proj->projExprs =
                        appendToTailOfList(projExprs, newProjExpr);
                proj->op.schema->attrDefs = appendToTailOfList(proj->op.schema->attrDefs,
                        createAttributeDef("VERSIONS_STARTSCN", DT_LONG));
			}
			else
			{
			    FATAL_LOG("merging for READ COMMITTED and Union Update translation not supported yet.");
			}
			break;
		default:
		    FATAL_LOG("should never have ended up here");
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
			{
                //TODO adapt SCN attribute reference
			    // FIX parent projection expressions attribute reference
                FOREACH(ProjectionOperator,p,t->op.parents)
                {
                    AttributeReference *a = getTailOfListP(p->projExprs);
                    a->attrPosition = getNumAttrs((QueryOperator *) up) - 1;
                }
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
	List *finalAttrs, *projExprs = NIL;
	int cnt = 0;

	if(HAS_STRING_PROP(op,PROP_PC_TABLE))
	{
		char *temp = STRING_VALUE(GET_STRING_PROP(op,PROP_PC_TABLE));
		mergeRoot = getUpdateForPreviousTableVersion(op,   //Find last update to that table
				temp, -1, updates);
		if (mergeRoot == NULL)  // NULL then user has asked for non-existing table
		{
			FATAL_LOG("table"); //exit
		}

	}
	else
	{
		mergeRoot = (QueryOperator *) getHeadOfListP(updates);
	}

//	if(HAS_STRING_PROP(op,PROP_PC_TABLE))
//	{
//
//		mergeAttrs = getQueryOperatorAttrNames(up);
//		addChildOperator((QueryOperator *) op, up);
//		finalAttrs = sublist(mergeAttrs, 0, LIST_LENGTH(mergeAttrs) - 1);
//		FOREACH(AttributeDef, attr, mergeRoot->schema->attrDefs)
//		{
//		    if (strcmp(attr->attrName,"VERSIONS_STARTSCN") != 0)
//		        projExprs = appendToTailOfList(projExprs, createFullAttrReference(attr->attrName, 0, cnt, 0));
//		    cnt++;
//		}
//		finalProj = (QueryOperator *) createProjectionOp(projExprs, up, NIL, finalAttrs);
//		up->parents = singleton(finalProj);
//	}

//		mergeAttrs = getQueryOperatorAttrNames(mergeRoot);
//		finalAttrs = sublist(mergeAttrs, 0, LIST_LENGTH(mergeAttrs) - 1);

	    FOREACH(AttributeDef, attr, mergeRoot->schema->attrDefs)
	    {
	        if (strcmp(attr->attrName,"VERSIONS_STARTSCN") != 0)
	            projExprs = appendToTailOfList(projExprs, createFullAttrReference(attr->attrName, 0, cnt, 0, DT_LONG));
	        cnt++;
	    }

    FOREACH(AttributeDef, attr, mergeRoot->schema->attrDefs)
    {
        if (strcmp(attr->attrName,"VERSIONS_STARTSCN") != 0)
        {
            projExprs = appendToTailOfList(projExprs, createFullAttrReference(attr->attrName, 0, cnt, 0, attr->dataType));
            finalAttrs = appendToTailOfList(finalAttrs, strdup(attr->attrName));
        }
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

    if (isRewriteOptionActivated(OPTION_AGGRESSIVE_MODEL_CHECKING))
        ASSERT(checkModel((QueryOperator *) finalProj));
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

    DEBUG_LOG("is simple, %u", simpleOnly);

    if (getBoolOption(OPTION_COST_BASED_OPTIMIZER))
    {
    	int res;

    	res = callback(2);

    	if (res == 1)
    		addConditionsToBaseTables(op);
   		else
   			extractUpdatedFromTemporalHistory(op);
    }
    else
    {
    	//TODO for now be conservative when to apply things
		// use conditions of updates to filter out non-updated tuples early on
		if (isRewriteOptionActivated(OPTION_UPDATE_ONLY_USE_CONDS))
		{
			DEBUG_LOG("Use conditions to restrict to updated;");
			addConditionsToBaseTables(op);
		}
		// use history to get tuples updated by transaction and limit provenance tracing to these tuples
		else if (isRewriteOptionActivated(OPTION_UPDATE_ONLY_USE_HISTORY_JOIN))
		{
			DEBUG_LOG("Use history join to restrict to updated;");
			extractUpdatedFromTemporalHistory(op);
		}
		// simply filter out non-updated rows in the end
		else
		{
			DEBUG_LOG("filtering of updated rows in final result not supported yet.");
			filterUpdatedInFinalResult(op);
		}
    }


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
//    List *tableCondMap = NIL;
    HashMap *tabCondMap = NEW_MAP(Constant,List);
    int pos = 0;
//    KeyValue *tableCond;
    Set *readFromTableNames = STRSET();
    Set *updatedTableNames = STRSET();
    Set *mixedTableNames = NULL;
    HashMap *numAttrs = NEW_MAP(Constant,Constant);
    ProvenanceTransactionInfo *t = op->transactionInfo;
    Constant *constT = createConstBool(TRUE);

    upConds = (List *) GET_STRING_PROP(op, PROP_PC_UPDATE_COND);
    tableNames = t->updateTableNames;
    origUpdates = t->originalUpdates;
    findTableAccessVisitor((Node *) op, &allTables); //HAO fetch all table accesses
    updatedTables  = findUpdatedTableAccceses (allTables);

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
                KeyValue *tableMap = MAP_GET_STRING_ENTRY(tabCondMap, tableName); // getMapCond(tableCondMap, tableName);
                Node *cond = copyObject((Node *) getNthOfListP(upConds, pos));

                // for read committed we have to also check the version column to only
                // check the condition for rows versions that will be seen by an update
                if (t->transIsolation == ISOLATION_READ_COMMITTED && cond != NULL)
                {
                    Constant *scn = (Constant *) getNthOfListP(t->scns, i);
                    cond = (Node *) adaptConditionForReadCommitted(cond, scn,
                            INT_VALUE(MAP_GET_STRING(numAttrs, tableName)));
                }

                if (tableMap == NULL)
                    MAP_ADD_STRING_KEY(tabCondMap, tableName, singleton(cond));
                else
                    tableMap->value = (Node *) appendToTailOfList((List *) tableMap->value, cond);
            }
            pos++;
            i++;
        }
    }

    DEBUG_LOG("condition table map is:\n%s", tabCondMap);

    // add selections to only updated tables
    FOREACH(TableAccessOperator,t,updatedTables)
    {
        char *tableName = t->tableName;
        KeyValue *prop = MAP_GET_STRING_ENTRY(tabCondMap,tableName);
        Node *cond = prop ? prop->value : NULL;
        SelectionOperator *sel;

        DEBUG_LOG("selection conditions are: ", cond);

        if (cond != NULL)
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
        filterUpdatedInFinalResult(op);
}

static Node *
adaptConditionForReadCommitted(Node *cond, Constant *scn, int attrPos)
{
    Node *result;

    result = AND_EXPRS (
            cond,
            createOpExpr("<=", LIST_MAKE(createFullAttrReference(
                    "VERSIONS_STARTSCN", 0, attrPos, INVALID_ATTR, scn->constType),
                    copyObject(scn)))
        );

    DEBUG_LOG("adapted condition for read committed: %s", beatify(nodeToString(result)));

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

	//TODO need to postfilter for remaining ones
    mixedTableNames = intersectSets(readFromTableNames,updatedTableNames);
    if (!EMPTY_SET(mixedTableNames))
        filterUpdatedInFinalResult(op);
}

static void
filterUpdatedInFinalResult (ProvenanceComputation *op)
{
    //TODO will only work if called directly and annotation attributes have been added beforehand
    //TODO extend to support making it work as a fallback method, easiest solution would be to determine whether this is necessary early on
    // for each updated table add attribute that trackes whether the table has been updated

    // add final conditions that filters out rows
    SelectionOperator *sel;
    QueryOperator *top = OP_LCHILD(op);
    Node *cond;
    List *condList = NIL;
    int i = 0;

    FOREACH(AttributeDef,a,top->schema->attrDefs)
    {
        if (strncmp(a->attrName,"PROV_STATEMENT", strlen("PROV_STATEMENT")) == 0)
        {
            condList = appendToTailOfList(condList, createOpExpr("=",
                    LIST_MAKE(createFullAttrReference(strdup(a->attrName), 0, i,
                                    INVALID_ATTR, a->dataType),
                            createConstBool(TRUE))
                    ));
        }
        i++;
    }

    cond = andExprList(condList);
    DEBUG_LOG("create condition: %s", beatify(nodeToString(cond)));
    sel = createSelectionOp(cond, top, NIL, NIL);

    switchSubtrees(top, (QueryOperator *) sel);
}

//TODO check is still needed once we extend to nested subqueries
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

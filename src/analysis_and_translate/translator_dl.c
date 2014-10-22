/*-----------------------------------------------------------------------------
 *
 * translator_dl.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"

#include "metadata_lookup/metadata_lookup.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "model/datalog/datalog_model.h"
#include "model/set/hashmap.h"
#include "provenance_rewriter/prov_utility.h"

static List *translateProgram(DLProgram *p);
static QueryOperator *translateRule(DLRule *r);
static QueryOperator *translateGoal(DLAtom *r);
static QueryOperator *joinGoalTranslations (DLRule *r, List *goalTrans);
static Node *createJoinCondOnCommonAttrs (QueryOperator *l, QueryOperator *r);
static Node *createCondFromComparisons (List *comparisons, QueryOperator *in);
static void makeNamesUnique (List *names);
static List *connectProgramTranslation(DLProgram *p, HashMap *predToTrans);

static Node *replaceVarWithAttrRef(Node *node, List *context);

Node *
translateParseDL(Node *q)
{
    Node *result = NULL;

    INFO_LOG("translate DL model:\n\n%s", datalogToOverviewString(q));

    if (isA(q,DLProgram))
        result = (Node *) translateProgram((DLProgram *) q);
    // what other node types can be here?
    else
        FATAL_LOG("currently only DLProgram node type translation supported");

    INFO_LOG("translated DL model:\n\n%s", operatorToOverviewString(result));

    return result;
}

QueryOperator *
translateQueryDL(Node *node)
{
    // dummy implementation, DL will not have query nodes
    return NULL;
}

static List *
translateProgram(DLProgram *p)
{
    List *translation = NIL; // list of sinks of a relational algebra graph
    List *singleRuleTrans = NIL;
    HashMap *predToTrans = NEW_MAP(Constant,List);

    // translate rules
    FOREACH(DLRule,r,p->rules)
    {
        QueryOperator *tRule = translateRule(r);
        char *headPred = getHeadPredName(r);
        // not first rule for this pred
        if(MAP_HAS_STRING_KEY(predToTrans,headPred))
        {
            KeyValue *kv = MAP_GET_STRING_ENTRY(predToTrans,headPred);
            List *ruleTrans = (List *) kv->value;
            ruleTrans = appendToTailOfList(ruleTrans, tRule);
            kv->value = (Node *) ruleTrans;
        }
        // first rule for this pred
        else
        {
            List *ruleTrans = singleton(tRule);
            MAP_ADD_STRING_KEY(predToTrans,headPred,ruleTrans);
        }
        singleRuleTrans = appendToTailOfList(singleRuleTrans,
                tRule);
    }

    // for each predicate create a union between all translated rules
    // replace the lists with a reference to the top-most union op
    FOREACH_HASH_ENTRY(kv,predToTrans)
    {
        List *rTs = (List *) kv->value;
        QueryOperator *un = (QueryOperator *) popHeadOfListP(rTs);

        FOREACH(QueryOperator,o,rTs)
        {
            un = (QueryOperator *) createSetOperator(SETOP_UNION, LIST_MAKE(un,o), NIL,
                    getQueryOperatorAttrNames(un));
        }

        kv->value = (Node *) un;
    }

    // connect rules by replacing table access operators representing idb
    // relations with the corresponding alegebra expression
    translation = connectProgramTranslation(p, predToTrans);

    return translation;
}

/*
 * Create algebra expression for one datalog rule. This treats both idb and edb
 *  relations the same by representing them as relation accesses. A rule will be
 *  translated as follows:
 *
 *  1) natural joins between goal translations
 *  2) comparison atoms are translated into a selection on top of that
 *  3) the rule head will be a projection followed by duplicate removal (it's bag semantics, baby!)
 */
static QueryOperator *
translateRule(DLRule *r)
{
    ProjectionOperator *headP;
    DuplicateRemoval *dupRem;
    QueryOperator *joinedGoals;
    SelectionOperator *sel = NULL;
    List *goalTrans = NIL;
    List *conditions = NIL;

    DEBUG_LOG("translate rules: %s", datalogToOverviewString((Node *) r));

    // translate goals
    FOREACH(Node,a,r->body)
    {
        if (isA(a,DLAtom))
        {
            QueryOperator *tG = translateGoal((DLAtom *) a);
            goalTrans = appendToTailOfList(goalTrans, tG);
            DEBUG_LOG("translated body goal is: %s", operatorToOverviewString((Node *) tG));
        }
        else if (isA(a,DLComparison))
            conditions = appendToTailOfList(conditions, a);
        else
            FATAL_LOG("datalog rule should only contain atoms and comparison ops");
    }

    // add joins to connect goal translations
    joinedGoals = joinGoalTranslations(r, goalTrans);
    DEBUG_LOG("joined goals are: %s", operatorToOverviewString((Node *) joinedGoals));

    // create selection from comparison expression in the rule
    if (!LIST_EMPTY(conditions))
    {
        Node *cond = createCondFromComparisons(conditions, joinedGoals);
        sel = createSelectionOp(cond, joinedGoals, NIL, NULL);
    }

    // create projection to simulate head
    List *projExprs = NIL;
    List *headVars = getVarNames(getHeadVars(r));

    FOREACH(char,hV,headVars)
    {
        AttributeReference *a;
        int pos = getAttrPos(joinedGoals, hV);
        DataType dt = getAttrDefByPos(joinedGoals, pos)->dataType;

        a = createFullAttrReference(hV,0,pos,INVALID_ATTR, dt);
        projExprs = appendToTailOfList(projExprs, a);
    }

    headP = createProjectionOp(projExprs,
            sel ? (QueryOperator *) sel : joinedGoals,
            NIL,
            headVars);

    // add duplicate removal operator
    dupRem = createDuplicateRemovalOp(NULL, (QueryOperator *) headP, NIL,
            getQueryOperatorAttrNames((QueryOperator *) headP));

    DEBUG_LOG("translated rule:\n%s\n\ninto\n\n%s",
            datalogToOverviewString((Node *) r),
            operatorToOverviewString((Node *) dupRem));

    return (QueryOperator *) dupRem;
}

static QueryOperator *
joinGoalTranslations (DLRule *r, List *goalTrans)
{
    QueryOperator *result = (QueryOperator *) popHeadOfListP(goalTrans);

    FOREACH(QueryOperator,g,goalTrans)
    {
        JoinOperator *j;
        Node *cond;
        List *attrNames = CONCAT_LISTS(getQueryOperatorAttrNames(result),
                getQueryOperatorAttrNames(g));
        makeNamesUnique(attrNames);

        cond = createJoinCondOnCommonAttrs(result,g);

        j = createJoinOp(JOIN_INNER, cond, LIST_MAKE(result,g), NIL, attrNames);

        result =  (QueryOperator *) j;
    }

    return result;
}

static Node *
createJoinCondOnCommonAttrs (QueryOperator *l, QueryOperator *r)
{
    Node *result = NULL;
    Set *lAttrs = makeStrSetFromList(getQueryOperatorAttrNames(l));
    Set *rAttrs = makeStrSetFromList(getQueryOperatorAttrNames(r));
    Set *commonAttrs = intersectSets(lAttrs,rAttrs);

    DEBUG_LOG("common attrs are:\n%s", nodeToString(commonAttrs));

    FOREACH_SET(char,a,commonAttrs)
    {
        Operator *op;
        AttributeReference *lA;
        AttributeReference *rA;
        int lPos = getAttrPos(l,a);
        int rPos = getAttrPos(r,a);

        lA = createFullAttrReference(a, 0, lPos, INVALID_ATTR,
                getAttrDefByPos(l,lPos)->dataType);
        rA = createFullAttrReference(a, 1, rPos, INVALID_ATTR,
                getAttrDefByPos(r,rPos)->dataType);
        op = createOpExpr("=", LIST_MAKE(lA,rA));

        if (result == NULL)
            result = (Node *) op;
        else
            result = AND_EXPRS(result, op);
    }

    return result;
}

static void
makeNamesUnique (List *names)
{
    HashMap *nCount = NEW_MAP(Constant,Constant);

    FOREACH_LC(lc,names)
    {
        char *name = LC_P_VAL(lc);
        int count = MAP_INCR_STRING_KEY(nCount,name);

        if (count != 0)
            name = CONCAT_STRINGS(name, itoa(count));

        LC_P_VAL(lc) = name;
    }
}

/*
 *
 */
static Node *
createCondFromComparisons (List *comparisons, QueryOperator *in)
{
    Node *result = NULL;
    List *attrNames = getQueryOperatorAttrNames(in);

    FOREACH(DLComparison,d,comparisons)
    {
        Node *newC = replaceVarWithAttrRef(copyObject(d->opExpr), attrNames);

        if (result == NULL)
            result = newC;
        else
            result = AND_EXPRS(result,newC);
    }


    return result;
}

static Node *
replaceVarWithAttrRef(Node *node, List *context)
{
    if (node == NULL)
        return NULL;

    if (isA(node, DLVar))
    {
        DLVar *v = (DLVar *) node;
        int pos = listPosString(context, v->name);
        AttributeReference *a = createFullAttrReference(strdup(v->name),
                0,
                pos,
                INVALID_ATTR,
                v->dt);
    }

    return mutate(node, replaceVarWithAttrRef, context);
}

/*
 * Translate a datalog rule goal for relation R into
 *  1) a relation access R if it is a positive goal
 *  2) a (domain^n - R) for a negative goal
 *  In both cases we add a projection to rename attributes into the variable
 *  names of the goal. For example, R(X,Y) over relation R with attributes A1
 *  and A2 would be translated into PROJECTION[A1->X,B1->Y](R).
 */

static QueryOperator *
translateGoal(DLAtom *r)
{
    ProjectionOperator *rename;
    QueryOperator *pInput;
    TableAccessOperator *rel;

    rel = createTableAccessOp(r->rel, NULL, "REL", NIL,
                    getAttributeNames(r->rel),
                    getAttributeDataTypes(r->rel));

    // is negated goal?
    if (r->negated)
    {
        //TODO
    }
    else
        pInput = (QueryOperator *) rel;

    // add projection
    rename = (ProjectionOperator *) createProjOnAllAttrs(pInput);
    addChildOperator((QueryOperator *) rename, pInput);

    // change attribute names
    FORBOTH(Node,var,attr,r->args,rename->op.schema->attrDefs)
    {
        if(isA(var,DLVar))
        {
            AttributeDef *d = (AttributeDef *) attr;
            DLVar *v = (DLVar *) var;
            d->attrName = strdup(v->name);
        }
    }

    return (QueryOperator *) rename;
}

static List *
connectProgramTranslation(DLProgram *p, HashMap *predToTrans)
{
    List *qoRoots = NIL;

    // for each translated rule find table access operators corresponding to idb predicates
    // and replace them with their algebra expressions
    FOREACH_HASH(QueryOperator,root,predToTrans)
    {
        List *rels = NIL;

        findTableAccessVisitor((Node *) root, &rels);

        // for each rel check whether it is idb
        FOREACH(TableAccessOperator,r,rels)
        {
            if(MAP_HAS_STRING_KEY(predToTrans,r->tableName))
            {
                QueryOperator *idbImpl = (QueryOperator *) MAP_GET_STRING(predToTrans,r->tableName);
                switchSubtreeWithExisting((QueryOperator *) r,idbImpl);
            }
        }

        qoRoots = appendToTailOfList(qoRoots, root);
    }

    return qoRoots;
}



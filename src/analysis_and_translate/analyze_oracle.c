/*-----------------------------------------------------------------------------
 *
 * analyze_qb.c - analyze a query block using schema information provided
 *                by a metadata lookup plugin.
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *
 *
 *-----------------------------------------------------------------------------
 */
#include "common.h"
#include "instrumentation/timing_instrumentation.h"

#include "analysis_and_translate/analyze_oracle.h"
#include "analysis_and_translate/parameter.h"
#include "configuration/option.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"
#include "model/query_block/query_block_to_sql.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/expression/expression.h"
#include "metadata_lookup/metadata_lookup.h"
#include "provenance_rewriter/prov_schema.h"
#include "parameterized_query/parameterized_queries.h"
#include "parser/parser.h"
#include "model/query_operator/operator_property.h"
#include "temporal_queries/temporal_rewriter.h"
#include "utility/string_utils.h"
#include "provenance_rewriter/uncertainty_rewrites/uncert_rewriter.h"
#include "provenance_rewriter/coarse_grained/coarse_grained_rewrite.h"

static void analyzeStmtList (List *l, List *parentFroms);
static void analyzeQueryBlock (QueryBlock *qb, List *parentFroms);
static void analyzeSetQuery (SetQuery *q, List *parentFroms);
static void analyzeProvenanceStmt (ProvenanceStmt *q, List *parentFroms);
static void analyzeProvenanceOptions (ProvenanceStmt *prov);
static boolean reenactOptionHasTimes (List *opts);
static void analyzeWithStmt (WithStmt *w);
//static List *getAnalyzedViews(WithStmt *w);
static void analyzeCreateTable (CreateTable *c);
static void analyzeAlterTable (AlterTable *a);
static void analyzeExecQuery (ExecQuery *e);
static void analyzePreparedQuery (PreparedQuery *p);

static void analyzeJoin (FromJoinExpr *j, List *parentFroms);
static void analyzeWhere(QueryBlock *qb, List *parentFroms);
static void analyzeGroupByAgg(QueryBlock *qb, List *parentFroms);
static void analyzeLimitAndOffset (QueryBlock *qb);
static void analyzeOrderBy(QueryBlock *qb);

// search for non-group and non-aggregate expressions
static boolean searchNonGroupByRefs (Node *node, List *state);

// adapt identifiers and quoted identifiers based on backend
static void adaptIdentifiers (Node *stmt);
static boolean visitAdaptIdents(Node *node, Set *context);

// search for attributes and other relevant node types
static void analyzeFromProvInfo (FromItem *f);
static void adaptAttrPosOffset(FromItem *f, FromItem *decendent, AttributeReference *a);
static void adaptAttributeRefs(List* attrRefs, List* parentFroms);
static boolean findAttrReferences (Node *node, List **state);
static void enumerateParameters (Node *stmt);
static boolean findAttrRefInFrom (AttributeReference *a, List *fromClauses);
static FromItem *findNamedFromItem (FromItem *fromItem, char *name);
static int findAttrInFromItem (FromItem *fromItem, AttributeReference *attr);
static boolean findQualifiedAttrRefInFrom (List *nameParts, AttributeReference *a,  List *fromClauses);
static void logFromClauses(char *mes, List *fromClauses, LogLevel l);

#define DEBUG_LOG_FROM_CLAUSES(_mes, _fromClauses) \
do { \
  if (maxLevel >= LOG_DEBUG) \
  logFromClauses(_mes,_fromClauses, LOG_DEBUG); \
} while (0)




// analyze from item types
static void analyzeFromTableRef(FromTableRef *f);
static void analyzeInsert(Insert *f);
static void analyzeDelete(Delete *f);
static void analyzeUpdate(Update *f);
static void analyzeFromSubquery(FromSubquery *sq, List *parentFroms);
static void analyzeFromLateralSubquery(FromLateralSubquery *lq, List *parentFroms);
static List *analyzeNaturalJoinRef(FromTableRef *left, FromTableRef *right);
static void analyzeJoinCondAttrRefs(List *fromClause, List *parentFroms);
static boolean correctFromTableVisitor (Node *node, void *context);
static boolean checkTemporalAttributesVisitor (Node *node, DataType **context);

// analyze function calls and nested subqueries
static void analyzeFunctionCall(QueryBlock *qb);
static void analyzeNestedSubqueries(QueryBlock *qb, List *parentFroms);

// analyze FromJsonTable Item
static void analyzeFromJsonTable(FromJsonTable *f, List **state);

// real attribute name fetching
static List *expandStarExpression (SelectItem *s, List *fromClause);

//static char *getAttrNameFromNameWithBlank(char *blankName);
static List *getFromTreeLeafs (List *from);
static char *generateAttrNameFromExpr(SelectItem *s);
static List *splitTableName(char *tableName);
static void getTableSchema (char *tableName, List **attrDefs, List **attrNames, List **dts);
static boolean compareAttrDefName(AttributeDef *a, AttributeDef *b);
static void backendifyTableRef(FromTableRef *f);
static boolean setViewFromTableRefAttrs(Node *node, List *views);
static boolean schemaInfoHasTable(char *tableName);
static List *schemaInfoGetSchema(char *tableName);
static List *schemaInfoGetAttributeNames (char *tableName);
static List *schemaInfoGetAttributeDataTypes (char *tableName);

/* holder for schema information when analyzing reenactment with potential DDL */
static HashMap *schemaInfo = NULL;

Node *
analyzeOracleModel (Node *stmt)
{
    adaptIdentifiers(stmt);
    DEBUG_NODE_BEATIFY_LOG("After backendifying identifiers: ", stmt);
    analyzeQueryBlockStmt(stmt, NULL);

    return stmt;
}

static void
adaptIdentifiers (Node *stmt)
{
    Set *haveSeen = PSET();

    visit(stmt, visitAdaptIdents, haveSeen);
}

/*
 * traverse the query tree and adapt identifiers. Keep track of which identifiers have been
 * handled already. This method deals with:
 *
 * - names of functions FunctionCall nodes
 * - from clause items
 * - attribute references
 * - fromprovinfo
 * - select itmes
 * - with statements
 * - provenance sketch specfications in Provenance
 */
static boolean
visitAdaptIdents(Node *node, Set *context)
{
    if (node == NULL)
        return TRUE;

    DEBUG_LOG("adapt node type %s at %p [%s]",
              NodeTagToString(node->type),
              (void *) node,
              hasSetElem(context, node) ? "SEEN BEFORE" : "NEW");

    if(!hasSetElem(context, node))
    {
        if(isA(node,FunctionCall)
           || isA(node,WithStmt)
           || isFromItem(node)
           || isA(node,SelectItem)
           || isA(node,FromProvInfo)
           || isA(node,AttributeReference)
           || isA(node,ProvenanceStmt))
        {
            if(isA(node,FunctionCall))
            {
                FunctionCall *f = (FunctionCall *) node;
                f->functionname = backendifyIdentifier(f->functionname);
            }

            if(isFromItem(node))
            {
                FromItem *f = (FromItem *) node;

                DEBUG_NODE_BEATIFY_LOG("fix idents in from item", f);

                // change alias
                if (f->name != NULL)
                    f->name = backendifyIdentifier(f->name);

                switch(node->type)
                {
                case T_FromTableRef:
                {
                    FromTableRef *tr = (FromTableRef *) f;
                    DEBUG_LOG("updated table name %s to %s", tr->tableId,
                              backendifyIdentifier(tr->tableId));
                    tr->tableId = backendifyIdentifier(tr->tableId);
                }
                break;
                default:
                    break;
                }
            }

            if (isA(node,WithStmt))
            {
                WithStmt *w = (WithStmt *) node;

                FOREACH(KeyValue,kv,w->withViews)
                {
                    kv->key = (Node *) createConstString(backendifyIdentifier(STRING_VALUE(kv->key)));
                }
            }

            if (isA(node,FromProvInfo))
            {
                FromProvInfo *fp = (FromProvInfo *) node;

                if (fp->userProvAttrs)
                {
                    /* handle case of attribute names and quoted identifiers here */
                    FOREACH(char,name,fp->userProvAttrs)
                    {
                        ListCell *lc = FOREACH_GET_LC(name);
                        lc->data.ptr_value = backendifyIdentifier(name);
                    }
                }

                //Checking if a provProperty was declared
                if (fp->provProperties)
                {
                    //Remove the probability attribute if specified through the TIP flag
                    if (getStringProvProperty(fp, PROV_PROP_TIP_ATTR))
                    {
                        char *attrname = backendifyIdentifier(STRING_VALUE(getStringProvProperty(fp, PROV_PROP_TIP_ATTR)));
                        setStringProvProperty(fp, PROV_PROP_TIP_ATTR, (Node *) createConstString(attrname));
                    }

                    if (getStringProvProperty(fp, PROV_PROP_XTABLE_GROUPID))
                    {
                        if (getStringProvProperty(fp, PROV_PROP_XTABLE_PROB))
                        {
                            char *attrname = backendifyIdentifier(STRING_VALUE(getStringProvProperty(fp, PROV_PROP_XTABLE_GROUPID)));
                            setStringProvProperty(fp, PROV_PROP_XTABLE_GROUPID, (Node *) createConstString(attrname));

                            attrname = backendifyIdentifier(STRING_VALUE(getStringProvProperty(fp, PROV_PROP_XTABLE_PROB)));
                            setStringProvProperty(fp, PROV_PROP_XTABLE_PROB, (Node *) createConstString(attrname));
                        }
                    }
                }
            }

            if (isA(node, SelectItem))
            {
                SelectItem *s = (SelectItem *) node;

                if (s->alias == NULL)
                {
                    char *newAlias;

                    // need to first fixe attribute references in children for this to work
                    visit(node, visitAdaptIdents, context);

                    newAlias = generateAttrNameFromExpr(s);
                    s->alias = strdup(newAlias);

                    // return to avoid checking again
                    return TRUE;
                }
                else
                {
                    s->alias = backendifyIdentifier(s->alias);
                }
            }

            if(isA(node,AttributeReference))
            {
                AttributeReference *a = (AttributeReference *) node;
                List *result = NIL;
                StringInfo str = makeStringInfo();

                result = splitString(strdup(a->name), "."); //FIXME will fail when . appears in a quoted ident part
                FOREACH(char,part,result)
                {
                    ListCell *lc = FOREACH_GET_LC(part);
                    char *newName = backendifyIdentifier(part);
                    lc->data.ptr_value = newName;
                    appendStringInfo(str, "%s%s",
                                     !FOREACH_IS_FIRST(part,result) ? "." : "",
                                     newName);
                }

                TRACE_LOG("name <%s> backendified into <%s>", a->name, str->data);

                a->name = strdup(str->data);
            }

            if(isA(node,ProvenanceStmt))
            {
                ProvenanceStmt *p = (ProvenanceStmt *) node;

                FOREACH(KeyValue,kv,p->options)
                {
                    // backendify sketch table name and attribute name
                    if(isA(kv->key,Constant)
                       && ((Constant *) kv->key)->constType == DT_STRING
                       && streq(STRING_VALUE(kv->key),PROP_PC_COARSE_GRAINED))
                    {
                        List *typeSketch = (List *) kv->value;
                        Constant *type = (Constant *) getHeadOfListP(typeSketch);
                        List *sketches = (List *) getNthOfListP(typeSketch, 1);

                        //TODO check for other sketch types too
                        if(((Constant *) type)->constType == DT_STRING
                           && streq(STRING_VALUE(type),COARSE_GRAINED_RANGEB))
                        {
                            FOREACH(KeyValue,ps,sketches)
                            {
                                Constant *tableName = (Constant *) ps->key;
                                List *attrSketchDefs = (List *) ps->value;
                                tableName->value = backendifyIdentifier(STRING_VALUE(tableName));

                                FOREACH(List,attrDef,attrSketchDefs)
                                {
                                    Constant *attrName = (Constant *) getHeadOfListP(attrDef);
                                    attrName->value = backendifyIdentifier(STRING_VALUE(attrName));
                                }
                            }
                        }
                    }
                }
            }

            addToSet(context, node);
        }
    }

    return visit(node, visitAdaptIdents, context);
}


void
analyzeQueryBlockStmt (Node *stmt, List *parentFroms)
{
    switch(stmt->type)
    {
        case T_QueryBlock:
            analyzeQueryBlock((QueryBlock *) stmt, parentFroms);
            DEBUG_LOG("analyzed QB");
            break;
        case T_SetQuery:
            analyzeSetQuery((SetQuery *) stmt, parentFroms);
            DEBUG_LOG("analyzed Set Query");
            break;
        case T_ProvenanceStmt:
            analyzeProvenanceStmt((ProvenanceStmt *) stmt, parentFroms);
            DEBUG_LOG("analyzed Provenance Stmt");
            break;
        case T_List:
            analyzeStmtList ((List *) stmt, parentFroms);
            DEBUG_LOG("analyzed List");
            break;
        case T_Insert:
            analyzeInsert((Insert *) stmt);
            break;
        case T_Delete:
            analyzeDelete((Delete *) stmt);
            break;
        case T_Update:
            analyzeUpdate((Update *) stmt);
            break;
        case T_WithStmt:
            analyzeWithStmt((WithStmt *) stmt);
            DEBUG_LOG("analyzed With Stmt");
            break;
        case T_CreateTable:
            analyzeCreateTable((CreateTable *) stmt);
            break;
        case T_AlterTable:
            analyzeAlterTable((AlterTable *) stmt);
            break;
        case T_ExecQuery:
            analyzeExecQuery((ExecQuery *) stmt);
            break;
        case T_PreparedQuery:
            analyzePreparedQuery((PreparedQuery *) stmt);
            break;
        default:
            break;
    }

    if(isQBUpdate(stmt) || isQBQuery(stmt))
        enumerateParameters(stmt);

    DEBUG_NODE_BEATIFY_LOG("RESULT OF ANALYSIS IS:", stmt);
}

static void
enumerateParameters (Node *stmt)
{
    List *params = findParameters(stmt);
    int i = 1;

    FOREACH(SQLParameter,p,params)
        p->position = i++;
}

static void
analyzeStmtList (List *l, List *parentFroms)
{
    FOREACH(Node,n,l)
        analyzeQueryBlockStmt(n, parentFroms);
}

static void
adaptAttributeRefs(List* attrRefs, List* parentFroms)
{
    // adapt attribute references
    FOREACH(AttributeReference,a,attrRefs)
    {
        // split name on each "."
        boolean isFound = FALSE;
        List *nameParts = splitAttrOnDot(a->name);
        DEBUG_LOG("attr split: %s", stringListToString(nameParts));

        if (LIST_LENGTH(nameParts) == 1)
        {
            a->name = getNthOfListP(nameParts, 0);
            isFound = findAttrRefInFrom(a, parentFroms);
        }
        else if (LIST_LENGTH(nameParts) == 2)
        {
            isFound = findQualifiedAttrRefInFrom(nameParts, a, parentFroms);
        }
        else
	    {
            FATAL_LOG("right now attribute names should have at most two parts");
		}
        if (!isFound)
		{
            FATAL_LOG("attribute <%s> does not exist in FROM clause", a->name);
		}
    }
}

static void
analyzeQueryBlock (QueryBlock *qb, List *parentFroms)
{
    List *attrRefs = NIL;

    // unfold views
    FOREACH(FromItem,f,qb->fromClause)
    {
        // deal with identifiers
        /* if (f->name != NULL) */
        /*     f->name = backendifyIdentifier(f->name); */

        switch(f->type)
        {
            case T_FromTableRef:
            {
                FromTableRef *tr = (FromTableRef *) f;
                backendifyTableRef(tr);
                boolean tableExists = catalogTableExists(tr->tableId) || schemaInfoHasTable(tr->tableId);
                boolean viewExists = catalogViewExists(tr->tableId);

                if (f->attrNames != NIL)
                    tableExists = TRUE; //TODO is that ok? this is proposed to be the case when this is a CTE

                //check if it is a table or a view
                if (!tableExists && viewExists)
                {
                    char * view = getViewDefinition(((FromTableRef *)f)->tableId);
                    char *newName = f->name ? f->name : tr->tableId; // if no alias then use view name
                    DEBUG_LOG("view: %s", view);
                    StringInfo s = makeStringInfo();
                    appendStringInfoString(s,view);
					if(!regExMatch(".*[;].*$", view))
					{
						appendStringInfoString(s,";");
					}
                    view = s->data;
                    Node * n1 = getHeadOfListP((List *) parseFromString((char *) view));
                    FromItem * f1 = createFromSubquery(newName,NIL,(Node *) n1);

                    DUMMY_LC(f)->data.ptr_value = f1;
                }
                if (!tableExists && !viewExists)
                    THROW(SEVERITY_RECOVERABLE, "table %s does not exist", tr->tableId);
            }
            break;
            default:
                break;
        }
    }

    // figuring out attributes of from clause items
    // to deal with LATERAL correlated subqueries we have to keep partial copies of the from clause items
    List *priorFromItems = NIL;

    FOREACH(FromItem,f,qb->fromClause)
    {
        List *newParentFroms = appendToHeadOfList(copyList(parentFroms), copyList(priorFromItems));
        priorFromItems = appendToTailOfList(priorFromItems, f);

        switch(f->type)
        {
            case T_FromTableRef:
                analyzeFromTableRef((FromTableRef *) f);
                break;
            case T_FromSubquery:
                analyzeFromSubquery((FromSubquery *) f, parentFroms);
                break;
            case T_FromLateralSubquery:
                analyzeFromLateralSubquery((FromLateralSubquery *) f, newParentFroms);
                break;
            case T_FromJoinExpr:
                analyzeJoin((FromJoinExpr *) f, parentFroms);
                break;
            case T_FromJsonTable:
                analyzeFromJsonTable((FromJsonTable *)f, &attrRefs);
            break;
            default:
                break;
        }

        analyzeFromProvInfo(f);

        DEBUG_LOG("analyzed from item <%s>", nodeToString(f));
    }

    INFO_LOG("Figuring out attributes of from clause items done");
    DEBUG_LOG("Found the following from tables: <%s>", nodeToString(qb->fromClause));

    // expand * expressions
    List *expandedSelectClause = NIL;
    FOREACH(SelectItem,s,qb->selectClause)
    {
        if (s->expr == NULL)
            expandedSelectClause = concatTwoLists(expandedSelectClause,
                    expandStarExpression(s,qb->fromClause));
        else
            expandedSelectClause = appendToTailOfList(expandedSelectClause,s);
    }
    qb->selectClause = expandedSelectClause;
    INFO_LOG("Expanded select clause is: <%s>",nodeToString(expandedSelectClause));

    // analyze join conditions attribute references
    analyzeJoinCondAttrRefs(qb->fromClause, parentFroms);

    // collect attribute references
    findAttrReferences((Node *) qb->distinct, &attrRefs);
    findAttrReferences((Node *) qb->groupByClause, &attrRefs);
    findAttrReferences((Node *) qb->havingClause, &attrRefs);
    findAttrReferences((Node *) qb->limitClause, &attrRefs);
    //TODO orderby needs to be treated differently findAttrReferences((Node *) qb->orderByClause, &attrRefs);
    findAttrReferences((Node *) qb->selectClause, &attrRefs);
    findAttrReferences((Node *) qb->whereClause, &attrRefs);

    INFO_LOG("Collect attribute references done");
    DEBUG_LOG("Have the following attribute references: <%s>", nodeToString(attrRefs));


    // expand list of from clause to use
    parentFroms = appendToHeadOfList(copyList(parentFroms), qb->fromClause);

    // adapt attribute references
    adaptAttributeRefs(attrRefs, parentFroms);

    // adapt function call (isAgg)
    analyzeFunctionCall(qb);
    DEBUG_LOG("Analyzed functions");

    // find nested subqueries and analyze them
    analyzeNestedSubqueries(qb, parentFroms);
    DEBUG_LOG("Analyzed nested subqueries");

    // analyze where clause if exists
    if (qb->whereClause != NULL)
    {
        analyzeWhere(qb, parentFroms);
    }

    // if group by or aggregation, check that no non-group by attribute references exist
    analyzeGroupByAgg(qb, parentFroms);

    // check order by
    analyzeOrderBy(qb);

    // check limit and offset
    analyzeLimitAndOffset(qb);

    INFO_LOG("Analysis done");
}

static void
analyzeNestedSubqueries(QueryBlock *qb, List *parentFroms)
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

    DEBUG_LOG("Current query <%s>\nhas nested subqueries\n%s",
              nodeToString(qb), nodeToString(nestedSubqueries));

    // analyze each subquery
    FOREACH(NestedSubquery,q,nestedSubqueries)
        analyzeQueryBlockStmt(q->query, parentFroms);
}

static void
analyzeFromProvInfo (FromItem *f)
{
    // analyze FromProvInfo if exists
    if (f->provInfo)
    {
        FromProvInfo *fp = f->provInfo;

        /* if (fp->userProvAttrs) */
        /* { */
        /*     /\* handle case of attribute names and quoted identifiers here *\/ */
        /*     FOREACH(char,name,fp->userProvAttrs) */
        /*     { */
        /*         ListCell *lc = FOREACH_GET_LC(name); */
        /*         lc->data.ptr_value = backendifyIdentifier(name); */
        /*     } */
        /* } */

        /* if the user provides a list of attributes (that store provenance
         * or should be duplicated as provenance attributes) then we need
         * to make sure these attributes exist. */
        if (fp->userProvAttrs)
        {
            FOREACH(char,name,fp->userProvAttrs)
            {
                if(!searchListString(f->attrNames, name))
                {
                    if (strcmp(name,"ROWID") == 0 || f->type == T_FromTableRef)
                    {
                        f->attrNames = deepCopyStringList(f->attrNames);
                        f->dataTypes = copyObject(f->dataTypes);
                        f->attrNames = appendToTailOfList(f->attrNames, strdup("ROWID"));
                        f->dataTypes = appendToTailOfListInt(f->dataTypes, DT_LONG);
                    }
                    else
                        FATAL_LOG("did not find provenance attr %s in from "
                            "item attrs %s", name, stringListToString(f->attrNames));
                }
            }
        }

        //Checking if a provProperty was declared
        if (fp->provProperties)
        {
            //Remove the probability attribute if specified through the TIP flag
            if (getStringProvProperty(fp, PROV_PROP_TIP_ATTR))
            {
                char *attrname = STRING_VALUE(getStringProvProperty(fp, PROV_PROP_TIP_ATTR));
                /* setStringProvProperty(fp, PROV_PROP_TIP_ATTR, (Node *) createConstString(attrname)); */
                int pos = listPosString(f->attrNames, attrname);
                DEBUG_LOG("TIP attribute %s at position %u", attrname, pos);
                f->attrNames = deepCopyStringList(f->attrNames);
                f->dataTypes 	= copyObject(f->dataTypes);
                f->attrNames = removeListElemAtPos(f->attrNames, pos);
                f->dataTypes = removeListElemAtPos(f->dataTypes, pos);
            }

            //Indicating an incomplete table has been called
            if (getStringProvProperty(fp, PROV_PROP_INCOMPLETE_TABLE))
            {
                DEBUG_LOG("INCOMPLETE TABLE");
            }

            //Assuming a schema format of [a,ub_a,lb_a,b,ub_b,lb_b,...,cet_r,bg_r,pos_r]
            if (getStringProvProperty(fp, PROV_PROP_RADB))
            {
                DEBUG_LOG("RADB INPUT");
                //need to contain at least one real attribute
                ASSERT(f->attrNames->length >= 6);
                //need to contain 3 row range annotations and 2 attribute range annotation per attribute
                ASSERT((f->attrNames->length-3)%3 == 0);

                //number of real attributes
                int numofrealattr = (f->attrNames->length-3)/3;

                f->attrNames = deepCopyStringList(f->attrNames);
                f->dataTypes = copyObject(f->dataTypes);

                //Check Attr names
                for(int i=0; i<numofrealattr; i++){
                    char *val = (char *)getNthOfListP(f->attrNames, i);
                    char *ub = (char *)getNthOfListP(f->attrNames, numofrealattr+2*i);
                    char *lb = (char *)getNthOfListP(f->attrNames, numofrealattr+2*i+1);
                    ASSERT(strcmp(getUBString(val), ub)==0);
                    ASSERT(strcmp(getLBString(val), lb)==0);
                }
                ASSERT(strcmp((char *)getNthOfListP(f->attrNames, f->attrNames->length-3), ROW_CERTAIN)==0);
                ASSERT(strcmp((char *)getNthOfListP(f->attrNames, f->attrNames->length-2), ROW_BESTGUESS)==0);
                ASSERT(strcmp((char *)getNthOfListP(f->attrNames, f->attrNames->length-1), ROW_POSSIBLE)==0);

                List *provattr = sublist(f->attrNames, numofrealattr, f->attrNames->length-1);

                f->attrNames = sublist(f->attrNames, 0, numofrealattr-1);
                f->dataTypes = sublist(f->dataTypes, 0, numofrealattr-1);

                setStringProvProperty(fp, PROV_PROP_RADB_LIST,
                                      (Node *) stringListToConstList(provattr));
            }
            if (getStringProvProperty(fp, PROV_PROP_UADB))
            {
                DEBUG_LOG("UADB INPUT");

                f->attrNames = deepCopyStringList(f->attrNames);
                f->dataTypes = copyObject(f->dataTypes);

                int numofrealattr = f->attrNames->length-1;

                //need to contain u_r at end
                ASSERT(strcmp((char *)getNthOfListP(f->attrNames, f->attrNames->length-1), getUncertString(UNCERTAIN_ROW_ATTR))==0);

                List *provattr = sublist(f->attrNames, numofrealattr, f->attrNames->length-1);

                f->attrNames = sublist(f->attrNames, 0, numofrealattr-1);
                f->dataTypes = sublist(f->dataTypes, 0, numofrealattr-1);

                setStringProvProperty(fp, PROV_PROP_UADB_LIST,
                                      (Node *) stringListToConstList(provattr));
            }
            //Removing the probability attribute if specified through the XTABLE flag
            if (getStringProvProperty(fp, PROV_PROP_XTABLE_GROUPID))
            {
                // test unnecessary?
                if (getStringProvProperty(fp, PROV_PROP_XTABLE_PROB))
                {
                    char *attrname = STRING_VALUE(getStringProvProperty(fp, PROV_PROP_XTABLE_GROUPID));
                    /* char *attrname = backendifyIdentifier(STRING_VALUE(getStringProvProperty(fp, PROV_PROP_XTABLE_GROUPID))); */
                    /* setStringProvProperty(fp, PROV_PROP_XTABLE_GROUPID, (Node *) createConstString(attrname)); */
                    int pos = listPosString(f->attrNames, attrname);
                    DEBUG_LOG("XTABLE groupID attribute %s at position %u", attrname, pos);
                    f->attrNames = deepCopyStringList(f->attrNames);
                    f->dataTypes 	= copyObject(f->dataTypes);
                    f->attrNames = removeListElemAtPos(f->attrNames, pos);
                    f->dataTypes = removeListElemAtPos(f->dataTypes, pos);

                    attrname = STRING_VALUE(getStringProvProperty(fp, PROV_PROP_XTABLE_PROB));
                    /* attrname = backendifyIdentifier(STRING_VALUE(getStringProvProperty(fp, PROV_PROP_XTABLE_PROB))); */
                    /* setStringProvProperty(fp, PROV_PROP_XTABLE_PROB, (Node *) createConstString(attrname)); */
                    pos = listPosString(f->attrNames, attrname);
                    DEBUG_LOG("XTABLE probability attribute %s at position %u", attrname, pos);
                    f->attrNames = deepCopyStringList(f->attrNames);
                    f->dataTypes 	= copyObject(f->dataTypes);
                    f->attrNames = removeListElemAtPos(f->attrNames, pos);
                    f->dataTypes = removeListElemAtPos(f->dataTypes, pos);
                }
            }
        }

        // if user declared some attributes as provenance (HAS PROVENANCE) then these attributes are temporarily removed
        // since they should not be referenced by query for which we are computing provenance
        if (fp->baserel == FALSE && fp->intermediateProv == FALSE && fp->userProvAttrs != NIL)
        {
            INFO_LOG("from clause item HAS PROVENANCE activated - remove provenance attributes from schema for analysis");
            FOREACH(char,provAttr,fp->userProvAttrs)
            {
                int pos = listPosString(f->attrNames, provAttr);
                if (pos == -1)
                {
                    FATAL_LOG("user declared provenance attribute %s, but this attribute does not exist in this FROM clause item: %s", provAttr, stringListToString(f->attrNames));
                }
                DEBUG_LOG("attribute %s at position %u", provAttr, pos);
                f->attrNames = deepCopyStringList(f->attrNames);
                f->dataTypes = copyObject(f->dataTypes);
                f->attrNames = removeListElemAtPos(f->attrNames, pos);
                f->dataTypes = removeListElemAtPos(f->dataTypes, pos);
            }
        }
    }
}

static void
analyzeFunctionCall(QueryBlock *qb)
{
    List *functionCallList = NIL;

    // collect function call
    findFunctionCall((Node *) qb->selectClause, &functionCallList);
    findFunctionCall((Node *) qb->havingClause, &functionCallList);

    INFO_LOG("Collect function call done");
    DEBUG_LOG("Have the following function calls: <%s>", nodeToString(functionCallList));

    // adapt function call
    FOREACH(Node, f, functionCallList)
    {
        if (isA(f, FunctionCall))
        {
            FunctionCall *c = (FunctionCall *) f;
            c->isAgg = isAgg(c->functionname);
        }
        // window function
        else
        {
            WindowFunction *w = (WindowFunction *) f;
            FunctionCall *c = (FunctionCall *) w->f;
            if(!isWindowFunction(c->functionname))
                FATAL_LOG("Function %s not supported as window function",
                        c->functionname);
        }
    }
}

static void
analyzeJoinCondAttrRefs(List *fromClause, List *parentFroms)
{
    List *stack = copyList(fromClause);

    while(!MY_LIST_EMPTY(stack))
    {
        FromItem *cur = (FromItem *) popHeadOfListP(stack);

        DEBUG_NODE_BEATIFY_LOG("analyze join:", cur);

        // only interested in joins
        if (isA(cur,FromJoinExpr))
        {
            FromJoinExpr *j = (FromJoinExpr *) cur;
            List *aRefs = NIL;

            findAttrReferences(j->cond, &aRefs);
			DEBUG_LOG("Have the following join attribute references: <%s>", nodeToString(aRefs));

            // analyze children (if they are joins)
            if (isA(j->left, FromJoinExpr))
                stack = appendToTailOfList(stack, j->left);
            if (isA(j->right, FromJoinExpr))
                stack = appendToTailOfList(stack, j->right);

            DEBUG_NODE_BEATIFY_LOG("join condition has attrs:",aRefs);

            FOREACH(AttributeReference,a,aRefs)
            {
                List *nameParts = splitAttrOnDot(a->name);
                boolean isFound = FALSE;
                List *newFroms = NIL;

                DEBUG_LOG("join condition attr split: %s", stringListToString(nameParts));

                // no from item specified, check direct inputs
                if (LIST_LENGTH(nameParts) == 1)
                {
                    newFroms = copyList(parentFroms);
					newFroms = appendToHeadOfList(newFroms, LIST_MAKE(j->left, j->right));
					DEBUG_LOG("Search join cond attrs in %s", beatify(nodeToString(newFroms)));
					isFound = findAttrRefInFrom(a, newFroms);
                }
                // is R.A, search for table in subtree
                else if (LIST_LENGTH(nameParts) == 2)
                {
                    List *leftLeafs = getFromTreeLeafs(singleton(j->left));
                    List *rightLeafs = getFromTreeLeafs(singleton(j->right));
                    char *fromItemName = (char *) getNthOfListP(nameParts,0);

                    DEBUG_LOG("search attr %s from FROM item %s",
                            getNthOfListP(nameParts,1), fromItemName);

                    // if named from item occurs in both subtree -> ambigious
                    if (findNamedFromItem(j->left, fromItemName) != NULL
                        &&
                        findNamedFromItem(j->right, fromItemName) != NULL)
                    {
                        FATAL_LOG("from item reference ambigious in join:\n%s",
                                beatify(nodeToString(j)));
                    }
                    // is in left subtree
                    else if (findNamedFromItem(j->left,fromItemName) != NULL)
                    {
                        newFroms = copyList(parentFroms);
                        newFroms = appendToHeadOfList(newFroms, leftLeafs);
                        isFound = findQualifiedAttrRefInFrom(nameParts, a, newFroms);

                        DEBUG_LOG("is in left subtree");

                        if (isFound)
                        {
                            int offset = 0;
                            a->fromClauseItem = 0;

                            FOREACH(FromItem,leaf,leftLeafs)
                            {
                                if (streq(leaf->name,fromItemName))
                                {
                                    a->attrPosition +=offset;
                                    break;
                                }
                                offset += LIST_LENGTH(leaf->attrNames);
                            }
                        }
                    }
                    // else search in right subtree
                    else if (findNamedFromItem(j->right,fromItemName) != NULL)
                    {
                        newFroms = copyList(parentFroms);
                        newFroms = appendToHeadOfList(newFroms, rightLeafs);
                        isFound = findQualifiedAttrRefInFrom(nameParts, a, newFroms);

                        DEBUG_LOG("is in right subtree");

                        if (isFound)
                        {
                            int offset = 0;
                            a->fromClauseItem = 1;

                            FOREACH(FromItem,leaf,rightLeafs)
                            {
                                if (streq(leaf->name,fromItemName))
                                {
                                    a->attrPosition +=offset;
                                    break;
                                }
                                offset += LIST_LENGTH(leaf->attrNames);
                            }
                        }
                    }

					// we may be inside a nested subquery, if yes, then try again in parent
					if (LIST_LENGTH(parentFroms) > 0)
					{
						DEBUG_LOG("Check whether join attribute %s is a correlated attribute.", stringListToString(nameParts));
						List *newFroms = copyObject(parentFroms);
						newFroms = appendToHeadOfList(newFroms, NIL);
						isFound = findQualifiedAttrRefInFrom(nameParts, a, newFroms);
					}

                    if (!isFound)
                    {
                        FATAL_LOG("could not find attribute %s referenced in "
                                "condition of join:\n%s",
                                a->name,
                                beatify(nodeToString(j)));
                    }
                }

				DEBUG_LOG("Join attribute reference adapted to\n%s\nin join\n%s",
						  beatify(nodeToString(a)),
						  beatify(nodeToString(j)));
            }
        }
    }

    DEBUG_NODE_BEATIFY_LOG("finished adapting attr refs in join conds:", fromClause);
}

static boolean
findAttrRefInFrom (AttributeReference *a, List *fromClauses)
{
    boolean isFound = FALSE;
    int fromPos = 0, attrPos, levelsUp = 0;

	DEBUG_LOG_FROM_CLAUSES("find attribute in:\n%s", fromClauses);

    FOREACH(List,fClause,fromClauses)
    {
        fromPos = 0;
        FOREACH(FromItem, f, fClause)
        {
            attrPos = findAttrInFromItem(f, a);

            if (attrPos != INVALID_ATTR)
            {
                if (isFound)
                    FATAL_LOG("ambigious attribute reference %s", a->name);
                else
                {
                    isFound = TRUE;
                    a->fromClauseItem = fromPos;
                    a->attrPosition = attrPos;
                    a->outerLevelsUp = levelsUp;
                    a->attrType = getNthOfListInt(f->dataTypes, attrPos);
					DEBUG_LOG("Found attribute reference in %s\nadapted it to\n%s",
							  beatify(nodeToString(f)),
							  beatify(nodeToString(a)));
                }
            }
            fromPos++;
        }
        levelsUp++;
    }

    return isFound;
}

static FromItem *
findNamedFromItem (FromItem *fromItem, char *name)
{
    if (isA(fromItem, FromJoinExpr))
    {
        FromJoinExpr *join = (FromJoinExpr *) fromItem;
        FromItem *result;

        // if join has an alias do not recurse
        if (join->from.name != NULL)
        {
            if (strcmp(name, join->from.name) == 0)
                return fromItem;
            else
                return NULL;
        }

        result = findNamedFromItem (join->left, name);
        if (result != NULL)
            return result;
        return findNamedFromItem (join->right, name);
    }

    // is not a join
    if (strpeq(name, fromItem->name))
        return fromItem;

    return NULL;
}

static int
findAttrInFromItem (FromItem *fromItem, AttributeReference *attr)
{
    boolean isFound = FALSE;
    int attrPos = 0, foundAttr = INVALID_ATTR;

    // is not a join
    FOREACH(char, r, fromItem->attrNames)
    {
        if(streq(attr->name, r))
        {
            // is ambigious?
            if (isFound)
            {
                FATAL_LOG("Ambiguous attribute reference <%s>", attr->name);
                break;
            }
            // find occurance found
            else
            {
                isFound = TRUE;
                foundAttr = attrPos;
            }
        }
        attrPos++;
    }

    // if it is a tableaccess then allow access to ROWID column
    if(strpleq(attr->name,"ROWID") && fromItem->type == T_FromTableRef)
    {
        isFound = TRUE;
        foundAttr = LIST_LENGTH(fromItem->attrNames);
        fromItem->attrNames = appendToTailOfList(fromItem->attrNames, strdup("ROWID"));
        fromItem->dataTypes = appendToTailOfListInt(fromItem->dataTypes, DT_LONG);
    }

    return foundAttr;
}



static void
logFromClauses(char *mes, List *fromClauses, LogLevel l)
{
	StringInfo str = makeStringInfo();
	int i = 0;

	FOREACH(List,clause, fromClauses)
	{
		appendStringInfo(str,"%u: [", i++);
		FOREACH(FromItem,f,clause)
		{
			appendStringInfo(str, "%s %s", f->name, stringListToString(f->attrNames));
			if (FOREACH_HAS_MORE(f))
			{
				appendStringInfoString(str, ", ");
			}
		}
		appendStringInfoString(str,"]\n");
	}

	log_(l, __FILE__, __LINE__, mes,  str->data);
}


static boolean
findQualifiedAttrRefInFrom (List *nameParts, AttributeReference *a, List *fromClauses)
{
    boolean foundFrom = FALSE;
    boolean foundAttr = FALSE;
    int fromClauseItem = 0;
    int attrPos = 0, levelsUp = 0;
    char *tabName = (char *) getNthOfListP(nameParts, 0);
    char *attrName = (char *) getNthOfListP(nameParts, 1);
    FromItem *fromItem = NULL;
    FromItem *leafItem = NULL;

    DEBUG_LOG("looking for attribute %s.%s", tabName, attrName);
	DEBUG_LOG_FROM_CLAUSES("find attribute in:\n%s", fromClauses);

    // find table name
    FOREACH(List,fromItems,fromClauses)
    {
            fromClauseItem = 0;
        FOREACH(FromItem, f, fromItems)
        {
            FromItem *foundF = findNamedFromItem(f, tabName);

            if (foundF != NULL)
            {
                if (foundFrom)
                {
                    FATAL_LOG("from clause item name <%s> appears more than once", tabName);
                    return FALSE;
                }
                else
				{
                    fromItem = f;
                    leafItem = foundF;
                    a->fromClauseItem = fromClauseItem;
                    a->outerLevelsUp = levelsUp;
                    foundFrom = TRUE;
					DEBUG_LOG("Found attribute reference in %s\nadaped it to\n%s",
							  beatify(nodeToString(f)),
							  beatify(nodeToString(a)));

                }
            }
            fromClauseItem++;
        }
        levelsUp++;
    }

    // did we find from clause item
    if (!foundFrom)
    {
        FATAL_LOG("did not find from clause item named <%s>", tabName);
        return FALSE;
    }

    // find attribute name
    FOREACH(char,aName,leafItem->attrNames)
    {
        if (strcmp(aName, attrName) == 0)
        {
            if(foundAttr)
            {
                FATAL_LOG("ambigious attr name <%s> appears more than once in "
                        "from clause item <%s>:\n%s", attrName, tabName,
                        beatify(nodeToString(leafItem)));
                return FALSE;
            }
            else
            {
                a->attrPosition = attrPos;
                a->attrType = getNthOfListInt(leafItem->dataTypes, attrPos);
                foundAttr = TRUE;
            }
        }
        attrPos++;
    }

    if (!foundAttr)
    {
        FATAL_LOG("did not find any from clause item from attribute <%s>", attrName);
        return FALSE;
    }

    // map back attribute position and original from item
    adaptAttrPosOffset(fromItem, leafItem, a);
    a->name = strdup(attrName);

    return foundAttr;
}

static void
adaptAttrPosOffset(FromItem *f, FromItem *decendent, AttributeReference *a)
{
    List *leafs = getFromTreeLeafs(singleton(f));
    int offset = 0;

    FOREACH(FromItem,l,leafs)
    {
        if (streq(l->name, decendent->name))
        {
            a->attrPosition += offset;
            break;
        }
        offset += LIST_LENGTH(l->attrNames);
    }
}


boolean
hasNestedSubqueries (Node *node)
{
    List *nested = NIL;
    boolean result;

    findNestedSubqueries (node, &nested);
    result = LIST_LENGTH(nested) != 0;
    freeList(nested);

    return result;
}

boolean
findNestedSubqueries (Node *node, List **state)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, NestedSubquery))
    {
        *state = appendToTailOfList(*state, node);
        TRACE_LOG("found nested subquery <%s>", nodeToString(node));
        return TRUE;
    }

    if (isQBQuery(node))
        return TRUE;

    return visit(node, findNestedSubqueries, state);
}

boolean
findFunctionCall(Node *node, List **state)
{
    if(node == NULL)
        return TRUE;

    if(isA(node, FunctionCall))
        *state = appendToTailOfList(*state, node);
    else if (isA(node, WindowFunction))
    {
        *state = appendToTailOfList(*state, node);
        return TRUE;
    }

    if(isQBQuery(node))
        return TRUE;

    return visit(node, findFunctionCall, state);
}

static void
analyzeJoin (FromJoinExpr *j, List *parentFroms)
{
    FromItem *left = j->left;
    FromItem *right = j->right;

    // analyze inputs
    switch(left->type)
    {
        case T_FromTableRef:
        {
			FromTableRef *lt = (FromTableRef *) left;
		    backendifyTableRef(lt);
        	analyzeFromTableRef((FromTableRef *)left);
            analyzeFromProvInfo(left);
        }
        break;
        case T_FromJoinExpr:
            analyzeJoin((FromJoinExpr *)left, parentFroms);
            break;
        case T_FromSubquery:
        {
            FromSubquery *sq = (FromSubquery *) left;
            analyzeFromSubquery(sq, parentFroms);
        }
        break;
        default:
            break;
    }

    switch(right->type)
	{
		case T_FromTableRef:
		{
			FromTableRef *rt = (FromTableRef *) right;
			backendifyTableRef(rt);
            analyzeFromTableRef((FromTableRef *)right);
            analyzeFromProvInfo(right);
        }
        break;
        case T_FromJoinExpr:
            analyzeJoin((FromJoinExpr *) right, parentFroms);
            break;
        case T_FromSubquery:
        {
            FromSubquery *sq = (FromSubquery *) right;
            analyzeFromSubquery(sq, parentFroms);
        }
        break;
        default:
            break;
    }

    if (j->joinCond == JOIN_COND_NATURAL)
    {
        List *expectedAttrs = analyzeNaturalJoinRef((FromTableRef *)j->left,
                (FromTableRef *)j->right);
        if (j->from.attrNames == NULL)
            j->from.attrNames = expectedAttrs;
        ASSERT(LIST_LENGTH(j->from.attrNames) == LIST_LENGTH(expectedAttrs));
    }
    //JOIN_COND_USING
    //JOIN_COND_ON
    else
    {
        List *expectedAttrs = concatTwoLists(
                deepCopyStringList(left->attrNames),
                deepCopyStringList(right->attrNames));
        if (j->from.attrNames == NULL)
            j->from.attrNames = expectedAttrs;
        ASSERT(LIST_LENGTH(j->from.attrNames) == LIST_LENGTH(expectedAttrs));
    }

    j->from.dataTypes = CONCAT_LISTS((List *) copyObject(left->dataTypes),
            (List *) copyObject(right->dataTypes));

    DEBUG_NODE_BEATIFY_LOG("join analysis:", j);
}

static void
analyzeWhere (QueryBlock *qb, List *parentFroms)
{
    DataType returnType = typeOf(qb->whereClause);

    if (returnType != DT_BOOL)
        THROW(SEVERITY_RECOVERABLE,
                "WHERE clause result type should be DT_BOOL, but was %s:\n<%s>",
                DataTypeToString(returnType), beatify(nodeToString(qb->whereClause)));
}

static void
analyzeGroupByAgg(QueryBlock *qb, List *parentFroms)
{
    boolean hasAgg = FALSE;
    boolean hasWin = FALSE;
    List *funcCalls = NIL;

    // is there any aggregation in this query block?
    hasAgg = qb->groupByClause != NIL || qb->havingClause != NULL;
    findFunctionCall((Node *) qb->selectClause, &funcCalls);
    FOREACH(Node,f,funcCalls)
    {
        if(isA(f,FunctionCall))
        {
            if (((FunctionCall *)f)->isAgg)
                hasAgg = TRUE;
        }
        if(isA(f,WindowFunction))
        {
            hasWin = TRUE;
        }
    }

    // cannot have both window funciton and aggregation in the same query block
    if(hasAgg && hasWin)
    {
        THROW(SEVERITY_RECOVERABLE,
                "Query blocks with aggregation and/or group-by are not allowed to use window functions. Offender was:\n<%s>",
                beatify(nodeToString(qb)));
    }

    if (hasAgg)
    {
        // if yes, then check SELECT clause for non-groupby and unaggregated attribute references
        searchNonGroupByRefs((Node *) qb->selectClause, qb->groupByClause);
        searchNonGroupByRefs((Node *) qb->havingClause, qb->groupByClause);
    }
}

static boolean
searchNonGroupByRefs (Node *node, List *state)
{
    if(node == NULL)
        return TRUE;

    // if agg call, do not traverse deeper
    if(isA(node, FunctionCall))
    {
        FunctionCall *f = (FunctionCall *) node;
        if (f->isAgg)
        {
            return TRUE;
        }
    }

    // is node equal to one of the group-by expressions then do not traverse further
    FOREACH(Node,g,state)
    {
        if(equal(g, node))
            return TRUE;
    }

    if (isA(node, AttributeReference))
    {
        THROW(SEVERITY_RECOVERABLE,
                "Queries with aggregation and group-by only allow for group-by expressions or aggregated attributed to appear in the SELECT and HAVING clause. Offender was:\n<%s>",
                beatify(nodeToString(node)));
    }

    if(isQBQuery(node))
        return TRUE;

    return visit(node, searchNonGroupByRefs, state);
}



static void
analyzeOrderBy(QueryBlock *qb)
{
    List *attrRefs = NIL;
    List *nestedQs = NIL;
    List *selectAttrNames = NIL;

    // get attribute names from select clause
    FOREACH(SelectItem,s,qb->selectClause)
    {
        selectAttrNames = appendToTailOfList(selectAttrNames, strdup(s->alias));
    }

    findAttrReferences((Node *) qb->orderByClause, &attrRefs);
    findNestedSubqueries((Node *) qb->orderByClause, &nestedQs);

    // find attribute references in from or in select
        // adapt attribute references
    FOREACH(AttributeReference,a,attrRefs)
    {
        // split name on each "."
        boolean isFound = FALSE;
        List *nameParts = splitAttrOnDot(a->name);
        int pos;

        DEBUG_LOG("attr split: %s", stringListToString(nameParts));

        if (LIST_LENGTH(nameParts) == 1)
        {

            a->name = getNthOfListP(nameParts, 0);
            isFound = findAttrRefInFrom(a, NIL); //TODO add support for correlated attributes here?
        }
        else if (LIST_LENGTH(nameParts) == 2)
        {
            isFound = findQualifiedAttrRefInFrom(nameParts, a, NIL);
        }
        else
            FATAL_LOG(
                    "right now attribute names should have at most two parts");

        // exists in select clause? if yes, then use this
        pos = listPosString(selectAttrNames, a->name);
        if (pos != -1)
        {
            SelectItem *selectExpr = (SelectItem *) getNthOfListP(qb->selectClause, pos);
            a->attrPosition = pos;
            a->fromClauseItem = INVALID_ATTR; // use this to indicate that this is a SELECT clause attribute
            a->attrType = typeOf(selectExpr->expr);
            isFound = TRUE;
        }

        if (!isFound)
            FATAL_LOG("attribute <%s> does not exist in FROM clause", a->name);
    }
}

static void
analyzeLimitAndOffset (QueryBlock *qb)
{
    List *attrRefs = NIL;
    List *nestedQs = NIL;

    findAttrReferences(qb->limitClause, &attrRefs);
    findAttrReferences(qb->offsetClause, &attrRefs);

    if (attrRefs != NIL)
    {
        THROW(SEVERITY_RECOVERABLE,
              "Attribute references found in LIMIT or OFFSET clause."
              "These clauses only support scalar expression:\nLIMIT:%s\nOFFSET:\%s",
              nodeToString(qb->limitClause),
              nodeToString(qb->offsetClause));
    }

    findNestedSubqueries(qb->limitClause, &nestedQs);
    findNestedSubqueries(qb->offsetClause, &nestedQs);

    if (nestedQs != NIL)
    {
        THROW(SEVERITY_RECOVERABLE,
              "Nested subqueries found in LIMIT or OFFSET clause."
              "These clauses only support scalar expression:\nLIMIT:%s\nOFFSET:\%s",
              nodeToString(qb->limitClause),
              nodeToString(qb->offsetClause));
    }
}

static void
analyzeFromTableRef(FromTableRef *f)
{
    // attribute names already set (view or temporary view for now)
    // if we have schema information based on reenacting DDL then this overrides actual catalog information
    if (schemaInfoHasTable(f->tableId))
    {
        if (f->from.attrNames == NIL)
            f->from.attrNames = schemaInfoGetAttributeNames(f->tableId);

        if(!(f->from.dataTypes))
            f->from.dataTypes = schemaInfoGetAttributeDataTypes(f->tableId);
    }
    // otherwise use actual catalog information
    else
    {
        if (f->from.attrNames == NIL){
            f->from.attrNames = getAttributeNames(f->tableId);
        }

        if(!(f->from.dataTypes))
        {
            f->from.dataTypes = getAttributeDataTypes(f->tableId);
//            if(temporalAttrTypes == NIL)  //copy temporal attrs  T_BEGIN and T_END datatype, since it run two times, only need to time so check if temporalAttrTypes == NIL
//            	temporalAttrTypes = copyObject(f->from.dataTypes);
        }
    }
    if(f->from.name == NULL)
        f->from.name = f->tableId;
}

static void
recursiveAppendAttrNames(JsonColInfoItem *attr, List **attrNames, List **attrTypes)
{
    if (attr->nested)
    {
        FOREACH(JsonColInfoItem, attr1, attr->nested)
        {
            if(attr1->nested)
                recursiveAppendAttrNames(attr1, attrNames, attrTypes);
            else
            {
                *attrNames = appendToTailOfList(*attrNames, attr1->attrName);
                *attrTypes = appendToTailOfListInt(*attrTypes, DT_VARCHAR2);
            }
        }
    }
    else
    {
        *attrNames = appendToTailOfList(*attrNames, attr->attrName);
        *attrTypes = appendToTailOfListInt(*attrTypes, DT_VARCHAR2);
    }
}

static void
analyzeFromJsonTable(FromJsonTable *f, List **state)
{
    // Populate the attrnames, datatypes from columnlist
    List *attrNames = NIL;
    List *attrTypes = NIL;

    FOREACH(JsonColInfoItem, attr1, f->columns)
    {
        recursiveAppendAttrNames(attr1, &attrNames, &attrTypes);
    }

    if (f->from.attrNames == NIL)
        f->from.attrNames = attrNames;

    if (f->from.dataTypes == NIL)
        f->from.dataTypes = attrTypes;

    if(f->from.name == NULL)
        f->from.name = f->jsonTableIdentifier;

    //TODO JsonColumn can refer to column of JsonTable
    // Append jsonColumn to attributeRef list
    *state = appendToTailOfList(*state, f->jsonColumn);
}

static void
analyzeInsert(Insert * f)
{
    List *attrNames = NIL;
    List *dataTypes = NIL;
    List *attrDefs = NIL;
    HashMap *attrPos = NULL;
    Set *attrNameSet = makeStrSetFromList(attrNames);

    getTableSchema(f->insertTableName, &attrDefs, &attrNames, &dataTypes);
    f->schema = copyObject(attrDefs);

    // if user has given no attribute list, then get it from table definition
    if (f->attrList == NULL)
        f->attrList = deepCopyStringList(attrNames);
    // else use the user provided one and prepare a map from attribute name to position
    else
    {
        int i = 0;
        attrPos = NEW_MAP(Constant,Constant);

        FOREACH(char,name,f->attrList)
        {
            MAP_ADD_STRING_KEY(attrPos,name,createConstInt(i++));

            // if attribute is not an attribute of table then fail
            if (!hasSetElem(attrNameSet,name))
                FATAL_LOG("INSERT mentions attribute <%s> that is not an "
                        "attribute of table %s:<%s>",
                        name, f->insertTableName, stringListToString(attrNames));
        }
    }

    // is a VALUES clause
    if (isA(f->query,List))
    {
        if (LIST_LENGTH(f->attrList) != attrNames->length)
        {
//            int pos = 0;
            List *newValues = NIL;
            List *oldValues = (List *) f->query;
            INFO_LOG("The number of values are not equal to the number "
                    "attributes in the table");
            //TODO add NULL or DEFAULT values for remaining attributes
            FOREACH(AttributeDef,a,attrDefs)
            {
                Node *val = NULL;

                if (MAP_HAS_STRING_KEY(attrPos,a->attrName))
                {
                    val = getNthOfListP(oldValues,
                            INT_VALUE(MAP_GET_STRING(attrPos,a->attrName)));
                    // TODO sanity check value (e.g., no attribute references) tackle also corner cases
                }
                else
                {
                    List *nameParts = splitTableName(f->insertTableName);
                    Node *def = getAttributeDefaultVal(
                            (char *) getNthOfListP(nameParts, 0),
                            (char *) getNthOfListP(nameParts, 1),
                            a->attrName); //TODO get schema

                    if (def == NULL)
                        val = (Node *) createNullConst(a->dataType);
                    else
                        val = def;
                }
                newValues = appendToTailOfList(newValues, val);
            }

            f->query = (Node *) newValues;
        }
    }
    // is an INSERT INTO R (SELECT ...)
    else
    {
        QueryBlock *q = (QueryBlock *) f->query;
        analyzeQueryBlockStmt(f->query, NIL);
        //TODO check query data types
        //TODO even more important add query block for missing attributes if necessary
        if (LIST_LENGTH(f->attrList) != attrNames->length)
        {
            QueryBlock *wrap = createQueryBlock();
            List *selectClause = NIL;

            FOREACH(AttributeDef,a,attrDefs)
            {
                Node *val = NULL;
                //	            SelectItem *subItem = NULL;
                SelectItem *newItem = NULL;

                if (MAP_HAS_STRING_KEY(attrPos,a->attrName))
                {
                    //                    subItem = (SelectItem *) getNthOfListP(q->selectClause,
                    //                            INT_VALUE(MAP_GET_STRING(attrPos,a->attrName)));
                    val = (Node *) createFullAttrReference(strdup(a->attrName),
                            0,
                            INT_VALUE(MAP_GET_STRING(attrPos,a->attrName)),
                            INVALID_ATTR,
                            a->dataType);
                    // TODO sanity check DT
                }
                else
                {
                    List *nameParts = splitTableName(f->insertTableName);
                    Node *def = getAttributeDefaultVal(
                            (char *) getNthOfListP(nameParts, 0),
                            (char *) getNthOfListP(nameParts, 1),
                            a->attrName); //TODO get schema

                    if (def == NULL)
                        val = (Node *) createNullConst(a->dataType);
                    else
                        val = def;
                }
                newItem = createSelectItem(strdup(a->attrName),val);
                selectClause = appendToTailOfList(selectClause, newItem);
            }

            wrap->selectClause = selectClause;
            wrap->fromClause = singleton(createFromSubquery(strdup("origInsertQuery"),
                    getQBAttrNames((Node *) q), (Node *) q));
            f->query = (Node *) wrap;
        }
    }
}

static void
analyzeDelete(Delete * f)
{
    List *attrRefs = NIL;
    List *subqueries = NIL;
    List *attrDefs = NIL;
    List *attrNames = NIL;
    List *dataTypes =  NIL;
    FromTableRef *fakeTable;
    List *fakeFrom = NIL;

    getTableSchema(f->deleteTableName, &attrDefs, &attrNames, &dataTypes);
    f->schema = copyObject(attrDefs);

    fakeTable = (FromTableRef *) createFromTableRef(strdup(f->deleteTableName), attrNames,
            strdup(f->deleteTableName), dataTypes);
    fakeFrom = singleton(singleton(fakeTable));

    int attrPos = 0;

    findAttrReferences((Node *) f->cond, &attrRefs);
    FOREACH(AttributeReference,a,attrRefs) {
        boolean isFound = FALSE;

        attrPos = findAttrInFromItem((FromItem *) fakeTable, a);

        if (attrPos != INVALID_ATTR) {
            if (isFound)
                DEBUG_LOG("ambigious attribute reference %s", a->name);
            else {
                isFound = TRUE;
                a->fromClauseItem = 0;
                a->attrPosition = attrPos;
                a->outerLevelsUp = 0;
                a->attrType = getNthOfListInt(dataTypes, attrPos);
            }
        }

        if (!isFound)
            FATAL_LOG("do not find attribute %s", a->name);
    }

    // search for nested subqueries
    findNestedSubqueries(f->cond, &subqueries);

    // analyze each nested subqueries
    FOREACH(NestedSubquery,nq,subqueries)
        analyzeQueryBlockStmt(nq->query, fakeFrom);

}

static void
analyzeUpdate(Update* f)
{
    List *attrRefs = NIL;
    List *attrDefs = NIL;
    List *dataTypes = NIL;
    List *attrNames = NIL;
    List *subqueries = NIL;
    FromTableRef *fakeTable;
    List *fakeFrom = NIL;

    getTableSchema(f->updateTableName, &attrDefs, &attrNames, &dataTypes);
    f->schema = copyObject(attrDefs);

    fakeTable = (FromTableRef *) createFromTableRef(strdup(f->updateTableName), attrNames,
            strdup(f->updateTableName), dataTypes);
    fakeFrom = singleton(singleton(fakeTable));

//	boolean isFound = FALSE;
    int attrPos = 0;

    // find attributes
    findAttrReferences((Node *) f->cond, &attrRefs);
    findAttrReferences((Node *) f->selectClause, &attrRefs);

    // adapt attributes
    FOREACH(AttributeReference,a,attrRefs) {
        boolean isFound = FALSE;

        attrPos = findAttrInFromItem((FromItem *) fakeTable, a);

        if (attrPos != INVALID_ATTR) {
            if (isFound)
                DEBUG_LOG("ambigious attribute reference %s", a->name);
            else {
                isFound = TRUE;
                a->fromClauseItem = 0;
                a->attrPosition = attrPos;
                a->outerLevelsUp = 0;
                a->attrType = getNthOfListInt(dataTypes, attrPos);
            }
        }

        if (!isFound)
            FATAL_LOG("do not find attribute %s", a->name);
    }

    // search for nested subqueries
    findNestedSubqueries(f->cond, &subqueries);

    // analyze each nested subqueries
    FOREACH(NestedSubquery,nq,subqueries)
        analyzeQueryBlockStmt(nq->query, fakeFrom);
}

static void
analyzeFromSubquery(FromSubquery *sq, List *parentFroms)
{
    List *expectedAttrs;

    analyzeQueryBlockStmt(sq->subquery, parentFroms);
    expectedAttrs = getQBAttrNames(sq->subquery);

    // if no attr aliases given
    if (!(sq->from.attrNames))
        sq->from.attrNames = expectedAttrs;
    sq->from.dataTypes = getQBAttrDTs(sq->subquery);

    ASSERT(LIST_LENGTH(sq->from.attrNames) == LIST_LENGTH(expectedAttrs));
}

static void
analyzeFromLateralSubquery(FromLateralSubquery *lq, List *parentFroms)
{
    List *expectedAttrs;

    analyzeQueryBlockStmt(lq->subquery, parentFroms);
    expectedAttrs = getQBAttrNames(lq->subquery);

    // if no attr aliases given
    if (!(lq->from.attrNames))
        lq->from.attrNames = expectedAttrs;
    lq->from.dataTypes = getQBAttrDTs(lq->subquery);

    ASSERT(LIST_LENGTH(lq->from.attrNames) == LIST_LENGTH(expectedAttrs));
}

static void
getTableSchema (char *tableName, List **attrDefs, List **attrNames, List **dts)
{
    if (schemaInfoHasTable(tableName))
    {
        *attrDefs = schemaInfoGetSchema(tableName);
        *attrNames = schemaInfoGetAttributeNames(tableName);
        *dts = schemaInfoGetAttributeDataTypes(tableName);
    }
    else
    {
        *attrDefs = getAttributes(tableName);
        *attrNames = getAttributeNames(tableName);
        *dts = getAttributeDataTypes(tableName);
    }
}

static List *
analyzeNaturalJoinRef(FromTableRef *left, FromTableRef *right)
{
    List *lList = left->from.attrNames;
    List *rList = right->from.attrNames;
    List *result = deepCopyStringList(left->from.attrNames);

    // only add attributes from right input that are not in left input
    FOREACH(char, r, rList)
    {
        boolean found = FALSE;
        FOREACH(char , l, lList)
        {
            if(strcmp(l, r) == 0)
                found = TRUE;
        }
        if (!found)
            result = appendToTailOfList(result, r);
    }

    return result;
}

List *
splitAttrOnDot(char *dotName)
{
    List *nameParts = NIL;

    nameParts = splitString(strdup(dotName), "."); //FIXME will fail when . appears in a quoted ident part

    TRACE_LOG("Split attribute reference <%s> into <%s>", dotName, stringListToString(nameParts));

    return nameParts;
}

char *
lastAttrNamePart(char *attrName)
{
    List *nameParts = splitAttrOnDot(attrName);

    return (char *) getTailOfListP(nameParts);
}


#define DUMMY_FROM_IDENT_PREFIX backendifyIdentifier("dummyFrom")

static List *
expandStarExpression(SelectItem *s, List *fromClause)
{
    List *nameParts = splitAttrOnDot(s->alias); //TODO check why splitting here?
    List *newSelectItems = NIL;
    List *leafItems = getFromTreeLeafs(fromClause);
    ASSERT(LIST_LENGTH(nameParts) == 1 || LIST_LENGTH(nameParts) == 2);

    // should be "*" select item -> expand to all attribute in from clause
    if (LIST_LENGTH(nameParts) == 1)
    {
        int fromAliasCount = 0;
        ASSERT(strcmp((char *) getNthOfListP(nameParts,0),"*") == 0);

        FOREACH(FromItem,f,leafItems)
        {
            // create alias for join
            if (!(f->name))
            {
                StringInfo s = makeStringInfo();
                appendStringInfo(s,"%u", fromAliasCount++);
                f->name = CONCAT_STRINGS(DUMMY_FROM_IDENT_PREFIX, s->data);
                FREE(s);
            }

            FOREACH(char,attr,f->attrNames)
            {
                // do not expand ROWID column
                if (!(f->type == T_FromTableRef && strcmp(attr,"ROWID") == 0))
                {
                    AttributeReference *newA = createAttributeReference(
                              CONCAT_STRINGS(f->name,".",attr));

                    newSelectItems = appendToTailOfList(newSelectItems,
                            createSelectItem(
                                    strdup(attr),
                                    (Node *) newA
                            ));
                }
            }
        }
    }
    /*
     * should be "R.*" for some from clause item named R, expand to all
     * attributes from R
     */
    else
    {
        boolean found = FALSE;
        char *tabName = (char *) getNthOfListP(nameParts,0);
        char *attrName = (char *) getNthOfListP(nameParts,1);
        ASSERT(strcmp(attrName,"*") == 0);

        FOREACH(FromItem,f,leafItems)
        {
            if (strcmp(f->name,tabName) == 0)
            {
                if (found)
                    FATAL_LOG("Ambiguous from clause reference <%s> to from clause item <%s>", s->alias, tabName);
                else
                {
                    FOREACH(char,attr,f->attrNames)
                    {
                        // do not expand ROWID column
                        if (!(f->type == T_FromTableRef && strcmp(attr,"ROWID") == 0))
                        {
                            newSelectItems = appendToTailOfList(newSelectItems,
                                    createSelectItem(
                                           strdup(attr),
                                           (Node *) createAttributeReference(
                                                   CONCAT_STRINGS(f->name,".",attr))
                                    ));
                        }
                    }
                }
            }
        }
    }

    DEBUG_LOG("Expanded a star expression into <%s>", nodeToString(newSelectItems));

    return newSelectItems;
}

static List *
getFromTreeLeafs (List *from)
{
    List *result = NIL;

    FOREACH(FromItem,f,from)
    {
        switch(f->type)
        {
            case T_FromJoinExpr:
            {
                FromJoinExpr *j = (FromJoinExpr *) f;
                result = CONCAT_LISTS(result,
                        getFromTreeLeafs(LIST_MAKE(j->left, j->right)));
            }
            break;
            case T_FromSubquery:
             case T_FromLateralSubquery:
            case T_FromTableRef:
                result = appendToTailOfList(result, f);
                break;
            case T_FromJsonTable:
            result = appendToTailOfList(result, f);
            break;
            default:
                FATAL_LOG("expected a FROM clause item not: %s",
                        NodeTagToString(f->type));
        }
    }

    DEBUG_NODE_BEATIFY_LOG("from leaf items are:", result);

    return result;
}

static char *
generateAttrNameFromExpr(SelectItem *s)
{
    char *name = exprToSQL(s->expr, NULL, TRUE);
    char c;
    StringInfo str = makeStringInfo();

    if (streq(getOptionAsString(OPTION_BACKEND),"oracle"))
    {

        while((c = *name++) != '\0')
        {
            if (c != ' ' && c != '"' && c != '.')
            {
                appendStringInfoChar(str, toupper(c));
            }
        }
    }
    else
    {
        while((c = *name++) != '\0')
        {
            if (c != ' ' && c != '.')
            {
                appendStringInfoChar(str, c);
            }
        }

        // need to escape double quotes in generated string
        str->data = replaceSubstr(str->data, "\"", "_");
    }

    return str->data;
}

static List *
splitTableName(char *tableName)
{
    List *result = NIL;
    StringInfo split = makeStringInfo();
    char *pos = tableName - 1;
    int len = strlen(tableName);
    boolean inString = FALSE;

    while(pos++ != (tableName + len))
    {
        char c = *pos;
        switch(c)
        {
            case '.':
                if (!inString)
                {
                    result = appendToTailOfList(result, strdup(split->data));
                    resetStringInfo(split);
                }
                appendStringInfoChar(split,*pos);
                break;
            case '"':
                if (inString)
                    inString = FALSE;
                else
                    inString = TRUE;
                break;
            case '\\':
                if (inString)
                    pos++;
                break;
            default:
                appendStringInfoChar(split,*pos);
                break;
        }
    }
    result = appendToTailOfList(result, strdup(split->data));

    // if no schema is given, use connection user part
    if (LIST_LENGTH(result) == 1)
        result = appendToHeadOfList(result,
                strToUpper(getStringOption("connection.user")));
    FREE(split->data);

    return result;
}



static void
analyzeSetQuery (SetQuery *q, List *parentFroms)
{
    analyzeQueryBlockStmt(q->lChild, parentFroms);
    analyzeQueryBlockStmt(q->rChild, parentFroms);

    // get attributes from left child
    switch(q->lChild->type)
    {
        case T_QueryBlock:
        {
            QueryBlock *qb = (QueryBlock *) q->lChild;
            FOREACH(SelectItem,s,qb->selectClause)
            {
                q->selectClause = appendToTailOfList(q->selectClause,
                        strdup(s->alias));
            }
        }
        break;
        case T_SetQuery:
            q->selectClause = deepCopyStringList(
                    ((SetQuery *) q->lChild)->selectClause);
        break;
        case T_ProvenanceStmt:
            q->selectClause = deepCopyStringList(
                    ((ProvenanceStmt *) q->lChild)->selectClause);
        break;
        default:
        break;
    }
}

/*
 * Analyze a provenance computation. The main part is to figure out the attributes
 */

static void
analyzeProvenanceStmt (ProvenanceStmt *q, List *parentFroms)
{
    switch (q->inputType)
    {
        case PROV_INPUT_TRANSACTION:
        {
            //TODO need to know updates at this point
        }
        break;
        case PROV_INPUT_REENACT:
        {
            List *stmts = (List *) q->query;
            schemaInfo = NEW_MAP(Node,Node); //maps table name to schema
            boolean hasTimes = FALSE;

            FOREACH(KeyValue,sInfo,stmts)
            {
                Node *stmt = sInfo->key;
                //TODO maintain and extend a schema info
                analyzeQueryBlockStmt(stmt, NIL);
                hasTimes |= reenactOptionHasTimes((List *) sInfo->value);
//                schemaInfos = appendToTailOfList(schemaInfos, copyObject(schemaInfo));
            }
            // store schema infos in provenancestmt's options for translator
//            q->options = appendToTailOfList(q->options,
//                    createNodeKeyValue(
//                            (Node *) createConstString("SCHEMA_INFOS"),
//                            (Node *) schemaInfos));

            if (hasTimes)
            {
                FOREACH(KeyValue,sInfo,stmts)
                {
                    if (!reenactOptionHasTimes((List *) sInfo->value))
                    {
                        FATAL_NODE_BEATIFY_LOG("AS OF should be specified for all statments to be reenacted or none!", q);
                    }
                }
                q->inputType = PROV_INPUT_REENACT_WITH_TIMES;
            }

            INFO_NODE_BEATIFY_LOG("REENACT THIS:", q);
            schemaInfo = NULL;
        }
        break;
        case PROV_INPUT_UPDATE:
        {

        }
        break;
        case PROV_INPUT_QUERY:
        case PROV_INPUT_UNCERTAIN_QUERY:
        {
            List *provAttrNames = NIL;
            List *provDts = NIL;

            analyzeQueryBlockStmt(q->query, parentFroms);
            switch(q->provType)
            {
                case PROV_COARSE_GRAINED:
                case USE_PROV_COARSE_GRAINED:
                case USE_PROV_COARSE_GRAINED_BIND:
                    getQBProvenanceAttrList(q,&provAttrNames,&provDts);

                    q->selectClause = provAttrNames;
                    q->dts = provDts;
                break;
                default:
                    q->selectClause = getQBAttrNames(q->query);
                    q->dts = getQBAttrDTs(q->query);
                    /*
                     * if q->query is WithStmt
                     * /WithStmt *ws = (WithStmt *)q->query;
                     * q->selectClause = getQBAttrNames(ws->query);
                     * q->dts = getQBAttrDTs(ws->query);
                     * fixed in getQBAttrNames and getQBAttrDTs
                     */
                    // if the user has specified provenance attributes using HAS PROVENANCE then we have temporarily removed these  attributes for
                    // semantic analysis, now we need to recover the correct schema for determining provenance attribute datatypes and translation
                    DEBUG_NODE_BEATIFY_LOG("before correct table visitor", q->query);
                    correctFromTableVisitor(q->query, NULL);
                    getQBProvenanceAttrList(q,&provAttrNames,&provDts);

                    q->selectClause = concatTwoLists(q->selectClause,provAttrNames);
                    q->dts = concatTwoLists(q->dts,provDts);
                break;
            }
        }
        break;
        case PROV_INPUT_UNCERTAIN_TUPLE_QUERY:
        {
            List *provAttrNames = NIL;
            List *provDts = NIL;

            analyzeQueryBlockStmt(q->query, parentFroms);

            q->selectClause = getQBAttrNames(q->query);
            q->dts = getQBAttrDTs(q->query);
            correctFromTableVisitor(q->query, NULL);
            getQBProvenanceAttrList(q,&provAttrNames,&provDts);

            q->selectClause = concatTwoLists(q->selectClause, provAttrNames);
            q->dts = concatTwoLists(q->dts,provDts);
            //INFO_NODE_BEATIFY_LOG("RANGE:", q);
        }
        break;
        case PROV_INPUT_RANGE_QUERY:
        {
            List *provAttrNames = NIL;
            List *provDts = NIL;

            analyzeQueryBlockStmt(q->query, parentFroms);

            q->selectClause = getQBAttrNames(q->query);
            q->dts = getQBAttrDTs(q->query);
            // if the user has specified provenance attributes using HAS PROVENANCE then we have temporarily removed these  attributes for
            // semantic analysis, now we need to recover the correct schema for determining provenance attribute datatypes and translation
            correctFromTableVisitor(q->query, NULL);
            getQBProvenanceAttrList(q,&provAttrNames,&provDts);

            q->selectClause = concatTwoLists(q->selectClause, provAttrNames);
            q->dts = concatTwoLists(q->dts,provDts);
            //INFO_NODE_BEATIFY_LOG("RANGE:", q);
        }
        break;
        case PROV_INPUT_ZONO_UNCERT_QUERY:
        {
            List *provAttrNames = NIL;
            List *provDts = NIL;

            // analyze input query
            analyzeQueryBlockStmt(q->query, parentFroms);
            correctFromTableVisitor(q->query, NULL);
            getQBProvenanceAttrList(q,&provAttrNames,&provDts);

            q->selectClause = concatTwoLists(q->selectClause, provAttrNames);
            q->dts = concatTwoLists(q->dts,provDts);
        };
        case PROV_INPUT_TEMPORAL_QUERY:
        {
            DataType *tempDT = NULL;
            analyzeQueryBlockStmt(q->query, parentFroms);

            q->selectClause = getQBAttrNames(q->query);
            q->dts = getQBAttrDTs(q->query);
            correctFromTableVisitor(q->query, NULL);

            checkTemporalAttributesVisitor((Node *) q, &tempDT);
            //TODO check that table access has temporal attributes

            if (tempDT == NULL)
            {
                FATAL_LOG("sequenced temporal construct requires input to specify "
                        "time attributes for FROM clause items using WITH TIME(...)");
            }

            q->selectClause = concatTwoLists(q->selectClause,LIST_MAKE(strdup(TBEGIN_NAME), strdup(TEND_NAME)));
            q->dts = concatTwoLists(q->dts,CONCAT_LISTS(singletonInt(*tempDT), singletonInt(*tempDT)));
        }
        break;
        case PROV_INPUT_UPDATE_SEQUENCE:
            break;
        default:
            break;
    }

    analyzeProvenanceOptions(q);
}

static boolean
checkTemporalAttributesVisitor (Node *node, DataType **context)
{
    if (node == NULL)
        return TRUE;

    if(isFromItem(node))
    {
        FromItem *f = (FromItem *) node;
        FromProvInfo *p = f->provInfo;
        DataType tempD;

        // temporal attributes are stores as user provenance attributes
        if (p != NULL && p->userProvAttrs)
        {
            DataType leftDT;
            DataType rightDT;
            char *leftName;
            char *rightName;

            if (LIST_LENGTH(p->userProvAttrs) != 2)
            {
                FATAL_LOG("you have to specify exactly two temporal attributes "
                        "not %u:\n\n%s", LIST_LENGTH(p->userProvAttrs),
                        beatify(nodeToString(node)));
            }

            leftName = (char *) getNthOfListP(p->userProvAttrs, 0);
            rightName = (char *) getNthOfListP(p->userProvAttrs, 1);
            leftDT = getNthOfListInt(f->dataTypes, listPosString(f->attrNames, leftName));
            rightDT = getNthOfListInt(f->dataTypes, listPosString(f->attrNames, rightName));
            tempD = leftDT;

            if (leftDT != rightDT)
                FATAL_LOG("attributes storing the interval endpoints have to "
                        "have the same DTs (%s:%s != %s:%s):\n\n%s",
                        leftName, DataTypeToString(leftDT),
                        rightName, DataTypeToString(rightDT),
                        beatify(nodeToString(node)));

            // first WITH TIME we have found so far?
            if (*context == NULL)
            {
                *context = NEW(DataType);
                **context = tempD;
            }
            // otherwise check that the DTs are the same
            else
            {
                if (**context != tempD)
                    FATAL_LOG("All temporal FROM items in sequenced temporal "
                            "queries have to have the same data type for temporal"
                            " attributes (%s != %s)",
                            DataTypeToString(**context), DataTypeToString(tempD));
            }
            return TRUE;
        }
        // table references that are not part of a subquery which specifies temporal atttribute should specify temporal attributes
        else if (node->type == T_FromTableRef)
        {
            FATAL_LOG("Table references in a sequenced temporal query you have "
                    "to specify temporal attributes with WITH TIME () unless this "
                    "table reference is part of a subquery for which temporal "
                    "attributes are specified:\n\n%s", beatify(nodeToString(node)));
        }
    }

    return visit(node, checkTemporalAttributesVisitor, context);
}


//FIXME this messes table references up that are refer to WITH CTEs, need to account for that
static boolean
correctFromTableVisitor(Node *node, void *context)
{
    if (node == NULL)
        return TRUE;

    // neex to restore with views and pass them on
    if(isA(node,WithStmt))
    {
        WithStmt *w = (WithStmt *) node;
        List *analyzedViews = NIL;

        // analyze each view, but make sure to set attributes of dummy views upfront
        FOREACH(KeyValue,v,w->withViews)
        {
//			correctFromTableVisitor(v->value, analyzedViews);
            setViewFromTableRefAttrs(v->value, analyzedViews);
            analyzedViews = appendToTailOfList(analyzedViews, v);
        }

        setViewFromTableRefAttrs(w->query, analyzedViews);

        return TRUE;
    }

    if(isFromItem(node))
    {
        switch (node->type)
        {
            case T_FromTableRef:
            {
                FromItem *f = (FromItem *) node;
                f->attrNames = NIL;
                f->dataTypes = NIL;
                analyzeFromTableRef((FromTableRef *) node);
            }
            break;
            case T_FromSubquery:
            {
                FromSubquery *sq = (FromSubquery *) node;
                sq->from.attrNames = getQBAttrNames(sq->subquery);
                sq->from.dataTypes = getQBAttrDTs(sq->subquery);
            }
            break;
            default:
                break;
        }
    }

    return visit(node, correctFromTableVisitor, context);
}

static boolean
reenactOptionHasTimes (List *opts)
{
    FOREACH(KeyValue,kv,opts)
    {
        if (streq(STRING_VALUE(kv->key), PROP_REENACT_ASOF))
            return TRUE;
    }
    return FALSE;
}

static void
analyzeProvenanceOptions (ProvenanceStmt *prov)
{
    /* loop through options */
    FOREACH(KeyValue,kv,prov->options)
    {
        char *key = STRING_VALUE(kv->key);
        char *value = STRING_VALUE(kv->value);

        /* provenance type */
        if (!strcmp(key, PROP_PC_PROV_TYPE))
        {
            if (streq(value, "PICS"))
                prov->provType = PROV_PI_CS;
            else if (!strcmp(value, "TRANSFORMATION"))
                prov->provType = PROV_TRANSFORMATION;
            else if (!strcmp(value, "XML"))
                prov->provType = PROV_XML;
            else
                FATAL_LOG("Unkown provenance type: <%s>", value);
        }
        /* TRANSLATE AS */
        if (!strcmp(key, "TRANSLATE AS"))
        {
            if(strcmp(value, "JSON"))
                FATAL_LOG("Unknown Translate Format: <%s>", value);
        }
    }
}

static void
analyzeWithStmt (WithStmt *w)
{
    Set *viewNames = STRSET();
    //List *analyzedViews = NIL;

    // check that no two views have the same name and backendify view names
    FOREACH(KeyValue,v,w->withViews)
    {
        Constant *n = (Constant *) v->key;
        n->value = backendifyIdentifier(STRING_VALUE(v->key));
        char *vName = strdup(STRING_VALUE(v->key));
        if (hasSetElem(viewNames, vName))
            FATAL_LOG("view <%s> defined more than once in with stmt:\n\n%s",
                    vName, nodeToString(w));
        else
            addToSet(viewNames, vName);
    }

    //analyzedViews = getAnalyzedViews(w);
    DEBUG_LOG("did set view table refs:\n%s", beatify(nodeToString(w->query)));
    analyzeQueryBlockStmt(w->query, NIL);

    DEBUG_NODE_BEATIFY_LOG("analyzed view is:", w->query);
}

/*
static List *
getAnalyzedViews(WithStmt *w)
{
    List *analyzedViews = NIL;

   // analyze each view, but make sure to set attributes of dummy views upfront
    FOREACH(KeyValue,v,w->withViews)
    {
        setViewFromTableRefAttrs(v->value, analyzedViews);
        DEBUG_NODE_BEATIFY_LOG("did set view table refs:", v->value);
        analyzeQueryBlockStmt(v->value, NIL);
        analyzedViews = appendToTailOfList(analyzedViews, v);
    }

    setViewFromTableRefAttrs(w->query, analyzedViews);

    return analyzedViews;
}*/

static void
analyzeCreateTable (CreateTable *c)
{
    /*TODO support context */
    boolean tableExists;

    tableExists = catalogTableExists(c->tableName)
            || catalogViewExists(c->tableName)
            || schemaInfoHasTable(c->tableName);
    if (tableExists)
        FATAL_LOG("trying to create table that already exists: %s", c->tableName);

    // if is CREATE TABLE x AS SELECT ..., then analyze query
    if (c->query)
        analyzeQueryBlockStmt(c->query, NIL);

    // create schema info
    List *schema = NIL;
    if (c->query)
        schema = getQBAttrDefs(c->query);
    else
    {
        FOREACH(Node,el,c->tableElems)
        {
            if(isA(el,AttributeDef))
                schema = appendToTailOfList(schema, copyObject(el));
            else
                c->constraints = appendToTailOfList(c->constraints, el);//TODO check them
        }
    }
    c->tableElems = copyObject(schema);

    MAP_ADD_STRING_KEY(schemaInfo,c->tableName,schema);

    DEBUG_NODE_BEATIFY_LOG("analyzed create table is:", c);
}

static void
analyzeAlterTable (AlterTable *a)
{
    List *schema;

    if(!catalogTableExists(a->tableName) && !schemaInfoHasTable(a->tableName))
        FATAL_LOG("trying to alter table %s that does not exist", a->tableName);

    // get schema of table
    if(schemaInfoHasTable(a->tableName))
        schema = schemaInfoGetSchema(a->tableName);
    else
        schema = getAttributes(a->tableName);
    a->beforeSchema = copyObject(schema);

    // implement changes to schema based on command
    switch(a->cmdType)
    {
        case ALTER_TABLE_ADD_COLUMN:
        {
            AttributeDef *newA = createAttributeDef(strdup(a->columnName), a->newColDT);
            if (genericSearchList(schema,
                    (int (*) (void *, void *)) compareAttrDefName,
                    newA))
                FATAL_LOG("cannot add already existing column %s to table %s",
                        a->columnName, a->tableName);
            schema = appendToTailOfList(schema, newA);
        }
        break;
        case ALTER_TABLE_REMOVE_COLUMN:
        {
            AttributeDef *rmA = createAttributeDef(strdup(a->columnName), DT_INT);
            if(!genericSearchList(schema,
                    (int (*) (void *, void *)) compareAttrDefName,
                    rmA))
                FATAL_LOG("cannot remove non-existing column %s from table %s",
                                        a->columnName, a->tableName);
            schema = genericRemoveFromList(schema,
                    (int (*) (void *, void *)) compareAttrDefName, rmA);
        }
        break;
    }

    // store new schema
    MAP_ADD_STRING_KEY(schemaInfo, a->tableName, schema);
    a->schema = copyObject(schema);

    DEBUG_NODE_BEATIFY_LOG("analyzed alter table is:", a);
}

static void
analyzeExecQuery(ExecQuery *e)
{
    ParameterizedQuery *pq;
    if(!parameterizedQueryExists(e->name))
    {
        FATAL_LOG("execute asks for parameterized query that does not exist.");
    }

    pq = getParameterizedQuery(e->name);

    if(LIST_LENGTH(pq->parameters) != LIST_LENGTH(e->params))
    {
        FATAL_LOG("parameter list not of right length: expected %u but was %u",
                  LIST_LENGTH(pq->parameters),
                  LIST_LENGTH(e->params));
    }

    FORBOTH(Node, n1, n2, pq->parameters, e->params)
    {
        FATAL_LOG("non-compatible data type for parameter: %s and %s",
                  beatify(nodeToString(n1)),
                  beatify(nodeToString(n2)));
    }
}

static void
analyzePreparedQuery(PreparedQuery *p)
{
    List *dts;

    analyzeQueryBlockStmt(p->q, NIL);
    p->sqlText = parseBackQueryBlock(p->q);
    dts = getQBAttrDTs(p->q);

    if(p->dts != NIL)
    {
        if(LIST_LENGTH(p->dts) != LIST_LENGTH(dts))
        {
            FATAL_LOG("provides lists of data types has different size than the schema of the parameterized query:\n%s\n\n%s",
                      p->dts,
                      dts);
        }

        // check that data types provided by the user are compatible with the query's result data type
        FORBOTH_INT(d1,d2,dts,p->dts)
        {
            DataType dt1 = (DataType) d1;
            DataType dt2 = (DataType) d2;

            if (lcaType(dt1,dt2) != dt2)
            {
                FATAL_LOG("cannot cast parameterized query result DT %s into provided type %s",
                          DataTypeToString(dt1),
                          DataTypeToString(dt2));
            }
        }
    }
}

static boolean
compareAttrDefName(AttributeDef *a, AttributeDef *b)
{
    return (streq(a->attrName, b->attrName));
}

static void
backendifyTableRef(FromTableRef *f)
{
    if (!f->backendified)
    {
        f->backendified = TRUE;
        f->tableId = backendifyIdentifier(f->tableId);
    }
}

static boolean
setViewFromTableRefAttrs(Node *node, List *views)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, FromTableRef))
    {
        FromTableRef *f = (FromTableRef *) node;
        char *name;

        // backendify idents if necessary
        backendifyTableRef(f);
        name = f->tableId;

        FOREACH(KeyValue,v,views)
        {
            char *vName = STRING_VALUE(v->key);

            // found view, set attr names
            if (strcmp(name, vName) == 0)
            {
                ((FromItem *) f)->attrNames = getQBAttrNames(v->value);
                ((FromItem *) f)->dataTypes = getQBAttrDTs  (v->value);
            }
        }

        return TRUE;
    }

    return visit(node, setViewFromTableRefAttrs, views);
}

static boolean
schemaInfoHasTable(char *tableName)
{
    if(!schemaInfo)
        return FALSE;
    return MAP_HAS_STRING_KEY(schemaInfo,tableName);
}

static List *
schemaInfoGetSchema(char *tableName)
{
    if(!schemaInfo)
    {
        FATAL_LOG("request table information, but no schema information has been cached yet");
        return NULL;
    }
    return (List *) MAP_GET_STRING(schemaInfo,tableName);
}

static List *
schemaInfoGetAttributeNames (char *tableName)
{
    List *attrDefs = schemaInfoGetSchema(tableName);
    List *result = NIL;

    FOREACH(AttributeDef, a, attrDefs)
        result = appendToTailOfList(result, strdup(a->attrName));

    return result;
}

static List *
schemaInfoGetAttributeDataTypes (char *tableName)
{
    List *attrDefs = schemaInfoGetSchema(tableName);
    List *result = NIL;

    FOREACH(AttributeDef, a, attrDefs)
        result = appendToTailOfListInt(result, a->dataType);

    return result;
}


static boolean
findAttrReferences (Node *node, List **state)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, AttributeReference))
    {
        *state = appendToTailOfList(*state, node);
    }

    if (isQBQuery(node))
        return TRUE;

    return visit(node, findAttrReferences, state);
}

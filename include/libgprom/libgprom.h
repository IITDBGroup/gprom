/*-----------------------------------------------------------------------------
 *
 * libgprom.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_LIBGPROM_LIBGPROM_H_
#define INCLUDE_LIBGPROM_LIBGPROM_H_

// define boolean type and ExceptionHandler if not already defined
#ifndef COMMON_H
typedef int boolean;
#endif

#ifndef INCLUDE_EXCEPTION_EXCEPTION_H_
typedef enum ExceptionHandler {
    EXCEPTION_DIE,
    EXCEPTION_ABORT,
    EXCEPTION_WIPE
} ExceptionHandler;
#endif

// initialize system
extern void gprom_init(void);
extern void gprom_readOptions(int argc, char *const args[]);
extern void gprom_readOptionAndInit(int argc, char *const args[]);
extern void gprom_configFromOptions(void);
extern void gprom_reconfPlugins(void);
extern void gprom_shutdown(void);

// process an input query
extern const char *gprom_rewriteQuery(const char *query);

// callback interface for logger (application can process log messages)
// takes message, c-file, line, loglevel
typedef void (*GProMLoggerCallbackFunction) (const char *,const char *,int,int);

// callback interface for exception handling (application can deal with exceptions)
// takes message, c-file, line, severity
// return value indicates what to do
typedef ExceptionHandler (*GProMExceptionCallbackFunction) (const char *, const char *, int, int);

// register handlers and set log level
extern void gprom_registerLoggerCallbackFunction (GProMLoggerCallbackFunction callback);
extern void gprom_registerExceptionCallbackFunction (GProMExceptionCallbackFunction callback);
extern void gprom_setMaxLogLevel (int maxLevel);

// interface to configuration
extern const char *gprom_getStringOption (const char *name);
extern int gprom_getIntOption (const char *name);
extern boolean gprom_getBoolOption (const char *name);
extern double gprom_getFloatOption (const char *name);
extern const char *gprom_getOptionType(const char *name);
extern boolean gprom_optionExists(const char *name);

extern void gprom_setOption(const char *name, const char *value);
extern void gprom_setStringOption (const char *name, const char *value);
extern void gprom_setIntOption(const char *name, int value);
extern void gprom_setBoolOption(const char *name, boolean value);
extern void gprom_setFloatOption(const char *name, double value);

/* plugin definition */
typedef struct GProMMetadataLookupPlugin
{
    /* init and shutdown plugin and connection */
    boolean (*isInitialized) (void);
    int (*initMetadataLookupPlugin) (void);
    int (*databaseConnectionOpen) (void);
    int (*databaseConnectionClose) (void);
    int (*shutdownMetadataLookupPlugin) (void);

    /* catalog lookup */
    boolean (*catalogTableExists) (char * tableName);
    boolean (*catalogViewExists) (char * viewName);
    char * (*getKeyInformation) (char *tableName);
    char * (*getDataTypes) (char *tableName);
    char * (*getAttributeNames) (char *tableName);
    char * (*getAttributeDefaultVal) (char *schema, char *tableName, char *attrName);

    boolean (*isAgg) (char *functionName);
    boolean (*isWindowFunction) (char *functionName);
    char * (*getFuncReturnType) (char *fName, List* args, int numArgs);
    char * (*getOpReturnType) (char *oName, char **args, int numArgs);

    char * (*getTableDefinition) (char *tableName);
    char * (*getViewDefinition) (char *viewName);

    /* audit log access */
//    void (*getTransactionSQLAndSCNs) (char *xid, List **scns, List **sqls,
//            List **sqlBinds, IsolationLevel *iso, Constant *commitScn);
//    long (*getCommitScn) (char *tableName, long maxScn, char *xid);

    /* execution */
//    Node * (*executeAsTransactionAndGetXID) (List *statements, IsolationLevel isoLevel);
    //TODO define iterator interface for query results
//    char *** (*executeQuery) (char *query);       // returns a list of stringlist (tuples)
} GProMMetadataLookupPlugin;

extern void gprom_registerMetadataLookupPlugin (GProMMetadataLookupPlugin *plugin);

typedef enum GProMNodeTag{

    GProM_T_Invalid,
    GProM_T_Node,

    /* collection types */
    GProM_T_List,
    GProM_T_IntList,
    GProM_T_Set,
    GProM_T_HashMap,
    GProM_T_Vector,

    /* options */
    GProM_T_KeyValue,

    /* expression nodes */
	GProM_T_Constant,
    GProM_T_AttributeReference,
    GProM_T_SQLParameter,
    GProM_T_FunctionCall,
    GProM_T_Operator,
    GProM_T_CaseExpr,
    GProM_T_CaseWhen,
    GProM_T_IsNullExpr,
    GProM_T_WindowBound,
    GProM_T_WindowFrame,
    GProM_T_WindowDef,
    GProM_T_WindowFunction,
    GProM_T_RowNumExpr,
    GProM_T_OrderExpr,
    GProM_T_CastExpr,

    /* query block model nodes */
    GProM_T_SetQuery,
    GProM_T_ProvenanceStmt,
    GProM_T_ProvenanceTransactionInfo,
    GProM_T_QueryBlock,
    GProM_T_SelectItem,
    GProM_T_FromItem,
    GProM_T_FromProvInfo,
    GProM_T_FromTableRef,
    GProM_T_FromSubquery,
    GProM_T_FromJoinExpr,
    GProM_T_DistinctClause,
    GProM_T_NestedSubquery,
    GProM_T_Insert,
    GProM_T_Delete,
    GProM_T_Update,
    GProM_T_TransactionStmt,
    GProM_T_WithStmt,
    GProM_T_DDLStatement,
    GProM_T_UtilityStatement,

    /* query operator model nodes */
    GProM_T_Schema,
    GProM_T_AttributeDef,
    GProM_T_QueryOperator,
    GProM_T_SelectionOperator,
    GProM_T_ProjectionOperator,
    GProM_T_JoinOperator,
    GProM_T_AggregationOperator,
    GProM_T_ProvenanceComputation,
    GProM_T_TableAccessOperator,
    GProM_T_SetOperator,
    GProM_T_DuplicateRemoval,
    GProM_T_ConstRelOperator,
    GProM_T_NestingOperator,
    GProM_T_WindowOperator,
    GProM_T_OrderOperator,

    /* datalog model nodes */
    GProM_T_DLNode,
    GProM_T_DLAtom,
    GProM_T_DLVar,
    GProM_T_DLRule,
    GProM_T_DLProgram,
    GProM_T_DLComparison,
	GProM_T_DLDomain,

    /* Json Table GProMNode */
    GProM_T_FromJsonTable,
    GProM_T_JsonTableOperator,
    GProM_T_JsonColInfoItem,
    GProM_T_JsonPath,
			
    /* relation */
    GProM_T_Relation,

    /* rpq */
    GProM_T_Regex,
    GProM_T_RPQQuery,

    /* ddl */
    GProM_T_CreateTable,
    GProM_T_AlterTable
} GProMNodeTag;

typedef enum GProMDataType{

    GProM_DT_INT,
    GProM_DT_LONG,
    GProM_DT_STRING,
    GProM_DT_FLOAT,
    GProM_DT_BOOL,
    GProM_DT_VARCHAR2
} GProMDataType;

typedef struct GProMAttributeDef
{
    GProMNodeTag type;
    GProMDataType dataType;
    char *attrName;
} GProMAttributeDef;

typedef struct GProMListCell
{
    union
    {
        void *ptr_value;
        int  int_value;
    } data;
    struct GProMListCell *next;
} GProMListCell;

typedef struct GProMList
{
    GProMNodeTag type;
    int     length;
    GProMListCell *head;
    GProMListCell *tail;
} GProMList;

typedef struct GProMNode{
    GProMNodeTag type;
} GProMNode;

typedef struct GProMAttributeReference {
    GProMNodeTag type;
    char *name;
    int fromClauseItem;
    int attrPosition;
    int outerLevelsUp;
    GProMDataType attrType;
} GProMAttributeReference;

typedef struct GProMSchema
{
    GProMNodeTag type;
    char *name;
    GProMList *attrDefs; // GProMAttributeDef type
} GProMSchema;

typedef struct GProMQueryOperator
{
    GProMNodeTag type;
    GProMList *inputs; // children of the operator node, GProMQueryOperator type
    GProMSchema *schema; // attributes and their data types of result tables, GProMSchema type
    GProMList *parents; // direct parents of the operator node, GProMQueryOperator type
    GProMList *provAttrs; // positions of provenance attributes in the operator's schema
    GProMNode *properties; // generic node to store flexible list or map of properties (GProMKeyValue) for query operators
} GProMQueryOperator; // common fields that all operators have

typedef struct GProMTableAccessOperator
{
    GProMQueryOperator op;
    char *tableName;
    GProMNode *asOf;
} GProMTableAccessOperator;

typedef struct GProMSelectionOperator
{
    GProMQueryOperator op;
    GProMNode *cond; // condition expression
} GProMSelectionOperator;

typedef struct GProMProjectionOperator
{
    GProMQueryOperator op;
    GProMList *projExprs; // projection expressions
} GProMProjectionOperator;

typedef enum GProMJoinType{

    GProM_JOIN_INNER,
    GProM_JOIN_CROSS,
    GProM_JOIN_LEFT_OUTER,
    GProM_JOIN_RIGHT_OUTER,
    GProM_JOIN_FULL_OUTER
} GProMJoinType;

typedef struct GProMJoinOperator
{
    GProMQueryOperator op;
    GProMJoinType joinType;
    GProMNode *cond; // join condition expression
} GProMJoinOperator;

typedef struct GProMAggregationOperator
{
    GProMQueryOperator op;
    GProMList *aggrs; // aggregation expressions, GProMFunctionCall type
    GProMList *groupBy; // group by expressions
} GProMAggregationOperator;

typedef enum GProMSetOpType{

        GProM_SETOP_UNION,
        GProM_SETOP_INTERSECTION,
        GProM_SETOP_DIFFERENCE
} GProMSetOpType;

typedef struct GProMSetOperator
{
    GProMQueryOperator op;
    GProMSetOpType setOpType;
} GProMSetOperator;

typedef struct GProMDuplicateRemoval
{
    GProMQueryOperator op;
    GProMList *attrs; // attributes that need duplicate removal, GProMAttributeReference type
} GProMDuplicateRemoval;

typedef enum GProMProvenanceType{

    GProM_PROV_PI_CS,
    GProM_PROV_TRANSFORMATION,
    GProM_PROV_NONE
} GProMProvenanceType;

/* what type of database operation(s) a provenance computation is for */
typedef enum GProMProvenanceInputType{

    GProM_PROV_INPUT_QUERY,
    GProM_PROV_INPUT_UPDATE,
    GProM_PROV_INPUT_UPDATE_SEQUENCE,
    GProM_PROV_INPUT_REENACT,
    GProM_PROV_INPUT_REENACT_WITH_TIMES,
    GProM_PROV_INPUT_TRANSACTION,
    GProM_PROV_INPUT_TIME_INTERVAL
} GProMProvenanceInputType;

typedef enum GProMIsolationLevel{

    GProM_ISOLATION_SERIALIZABLE,
    GProM_ISOLATION_READ_COMMITTED,
    GProM_ISOLATION_READ_ONLY
} GProMIsolationLevel;

typedef struct GProMConstant {
    GProMNodeTag type;
    GProMDataType constType;
    void *value;
    int isNull;
} GProMConstant;

typedef struct GProMProvenanceTransactionInfo
{
    GProMNodeTag type;
    GProMIsolationLevel transIsolation;
    GProMList *updateTableNames;
    GProMList *originalUpdates;
    GProMList *scns;
    GProMConstant *commitSCN;
} GProMProvenanceTransactionInfo;

typedef struct GProMProvenanceComputation
{
    GProMQueryOperator op;
    GProMProvenanceType provType;
    GProMProvenanceInputType inputType;
    GProMProvenanceTransactionInfo *transactionInfo;
    GProMNode *asOf;
} GProMProvenanceComputation;

typedef struct GProMUpdateOperator
{
    GProMQueryOperator op;
    char *tableName;
} GProMUpdateOperator;

typedef struct GProMConstRelOperator
{
    GProMQueryOperator op;
    GProMList *values;
} GProMConstRelOperator;

typedef enum GProMNestingExprType{

    GProM_NESTQ_EXISTS,
    GProM_NESTQ_ANY,
    GProM_NESTQ_ALL,
    GProM_NESTQ_UNIQUE,
    GProM_NESTQ_SCALAR
} GProMNestingExprType;

typedef struct GProMNestingOperator
{
	GProMQueryOperator op;
	GProMNestingExprType nestingType;
	GProMNode *cond;
} GProMNestingOperator;

typedef struct GProMOrderOperator
{
    GProMQueryOperator op;
    GProMList *orderExprs;
} GProMOrderOperator;

typedef struct GProMFunctionCall {
    GProMNodeTag type;
    char *functionname;
    GProMList *args;
    int isAgg;
} GProMFunctionCall;

typedef struct GProMOperator {
    GProMNodeTag type;
    char *name;
    GProMList *args;
} GProMOperator;

typedef struct GProMSQLParameter {
    GProMNodeTag type;
    char *name;
    int position;
    GProMDataType parType;
} GProMSQLParameter;

typedef struct GProMRowNumExpr {
    GProMNodeTag type;
} GProMRowNumExpr;

typedef struct GProMCaseExpr {
    GProMNodeTag type;
    GProMNode *expr;
    GProMList *whenClauses;
    GProMNode *elseRes;
} GProMCaseExpr;

typedef struct GProMCaseWhen {
    GProMNodeTag type;
    GProMNode *when;
    GProMNode *then;
} GProMCaseWhen;

typedef struct GProMIsNullExpr {
    GProMNodeTag type;
    GProMNode *expr;
} GProMIsNullExpr;

typedef enum GProMWindowBoundType{

    GProM_WINBOUND_UNBOUND_PREC,
    GProM_WINBOUND_CURRENT_ROW,
    GProM_WINBOUND_EXPR_PREC,
    GProM_WINBOUND_EXPR_FOLLOW
} GProMWindowBoundType;

typedef struct GProMWindowBound {
    GProMNodeTag type;
    GProMWindowBoundType bType;
    GProMNode *expr;
} GProMWindowBound;

typedef enum GProMWinFrameType{

    GProM_WINFRAME_ROWS,
    GProM_WINFRAME_RANGE
} GProMWinFrameType;

typedef struct GProMWindowFrame {
    GProMNodeTag type;
    GProMWinFrameType frameType;
    GProMWindowBound *lower;
    GProMWindowBound *higher;
} GProMWindowFrame;

typedef struct GProMWindowDef {
    GProMNodeTag type;
    GProMList *partitionBy;
    GProMList *orderBy;
    GProMWindowFrame *frame;
} GProMWindowDef;

typedef struct GProMWindowFunction {
    GProMNodeTag type;
    GProMFunctionCall *f;
    GProMWindowDef *win;
} GProMWindowFunction;

typedef struct GProMCastExpr {
    GProMNodeTag type;
    GProMDataType resultDT;
    GProMNode *expr;
} GProMCastExpr;

typedef enum GProMSortOrder{

    GProM_SORT_ASC,
    GProM_SORT_DESC
} GProMSortOrder;

typedef enum GProMSortNullOrder{

    GProM_SORT_NULLS_FIRST,
    GProM_SORT_NULLS_LAST
} GProMSortNullOrder;

typedef struct GProMOrderExpr {
    GProMNodeTag type;
    GProMNode *expr;
    GProMSortOrder order;
    GProMSortNullOrder nullOrder;
} GProMOrderExpr;

extern GProMNode * gprom_rewriteQueryToOperatorModel(const char *query);

extern GProMNode * gprom_provRewriteOperator(GProMNode * nodeFromMimir);

extern GProMNode * gprom_taintRewriteOperator(GProMNode* nodeFromMimir);

extern GProMNode * gprom_optimizeOperatorModel(GProMNode * nodeFromMimir);

extern char * gprom_nodeToString(GProMNode * nodeFromMimir);

extern char * gprom_OperatorModelToQuery(GProMNode * nodeFromMimir);

extern void * gprom_createMemContext(void);

extern void * gprom_createMemContextName(const char * ctxName);

extern void gprom_freeMemContext(void * memContext);

typedef struct GProMUT_hash_bucket {
   struct GProMUT_hash_handle *hh_head;
   unsigned count;

   /* expand_mult is normally set to 0. In this situation, the max chain length
    * threshold is enforced at its default value, HASH_BKT_CAPACITY_THRESH. (If
    * the bucket's chain exceeds this length, bucket expansion is triggered). 
    * However, setting expand_mult to a non-zero value delays bucket expansion
    * (that would be triggered by additions to this particular bucket)
    * until its chain length reaches a *multiple* of HASH_BKT_CAPACITY_THRESH.
    * (The multiplier is simply expand_mult+1). The whole idea of this
    * multiplier is to reduce bucket expansions, since they are expensive, in
    * situations where we know that a particular bucket tends to be overused.
    * It is better to let its chain length grow to a longer yet-still-bounded
    * value, than to do an O(n) bucket expansion too often. 
    */
   unsigned expand_mult;

} GProMUT_hash_bucket;

typedef struct GProMUT_hash_table {
   GProMUT_hash_bucket *buckets;
   unsigned num_buckets, log2_num_buckets;
   unsigned num_items;
   struct GProMUT_hash_handle *tail; /* tail hh in app order, for fast append    */
   int hho; /* hash handle offset (byte pos of hash handle in element */

   /* in an ideal situation (all buckets used equally), no bucket would have
    * more than ceil(#items/#buckets) items. that's the ideal chain length. */
   unsigned ideal_chain_maxlen;

   /* nonideal_items is the number of items in the hash whose chain position
    * exceeds the ideal chain maxlen. these items pay the penalty for an uneven
    * hash distribution; reaching them in a chain traversal takes >ideal steps */
   unsigned nonideal_items;

   /* ineffective expands occur when a bucket doubling was performed, but 
    * afterward, more than half the items in the hash had nonideal chain
    * positions. If this happens on two consecutive expansions we inhibit any
    * further expansion, as it's not helping; this happens when the hash
    * function isn't a good fit for the key domain. When expansion is inhibited
    * the hash will still work, albeit no longer in constant time. */
   unsigned ineff_expands, noexpand;

   uint32_t signature; /* used only to find hash tables in external analysis */
#ifdef HASH_BLOOM
   uint32_t bloom_sig; /* used only to test bloom exists in external analysis */
   uint8_t *bloom_bv;
   char bloom_nbits;
#endif

} GProMUT_hash_table;

typedef struct GProMUT_hash_handle {
   struct GProMUT_hash_table *tbl;
   void *prev;                       /* prev element in app order      */
   void *next;                       /* next element in app order      */
   struct GProMUT_hash_handle *hh_prev;   /* previous hh in bucket order    */
   struct GProMUT_hash_handle *hh_next;   /* next hh in bucket order        */
   void *key;                        /* ptr to enclosing struct's key  */
   unsigned keylen;                  /* enclosing struct's key len     */
   unsigned hashv;                   /* result of hash-fcn(key)        */
} GProMUT_hash_handle;

typedef struct GProMHashElem {
    void *data;
    void *key;
    GProMUT_hash_handle hh;
} GProMHashElem;

typedef struct GProMHashMap {
    GProMNodeTag type;
    GProMNodeTag keyType;
    GProMNodeTag valueType;
    int typelen;
    int (*eq) (void *, void*);
    void * (*cpy) (void *);
    GProMHashElem *elem;
} GProMHashMap;

extern GProMHashMap* gprom_addToMap(GProMHashMap* map, GProMNode * key, GProMNode * value);
extern GProMNode * gprom_getMap(GProMHashMap* map, GProMNode* key);
extern GProMNode * gprom_getMapString(GProMHashMap* map, char* key);

typedef struct GProMKeyValue
{
    GProMNodeTag type;
    GProMNode *key;
    GProMNode *value;
} GProMKeyValue;

typedef struct GProMWindowOperator
{
    GProMQueryOperator op;
    GProMList *partitionBy;
    GProMList *orderBy;
    GProMWindowFrame *frameDef;
    char *attrName;
    GProMNode *f;
} GProMWindowOperator;

#endif /* INCLUDE_LIBGPROM_LIBGPROM_H_ */

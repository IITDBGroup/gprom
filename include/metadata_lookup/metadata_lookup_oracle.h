/*-----------------------------------------------------------------------------
 *
 * metadata_lookup_oracle.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef METADATA_LOOKUP_ORACLE_H_
#define METADATA_LOOKUP_ORACLE_H_

#include "metadata_lookup/metadata_lookup.h"
#include "model/query_operator/query_operator.h"


/* enums for aggregation and window functions */
#define AGG_FUNCTION_NAME_MAXSIZE 20

typedef enum AGG
{
    //frequently used agg functions list
    AGG_MAX,
    AGG_MIN,
    AGG_AVG,
    AGG_COUNT,
    AGG_SUM,
    AGG_FIRST,
    AGG_LAST,

    //rarely used agg functions list
    AGG_CORR,
    AGG_COVAR_POP,
    AGG_COVAR_SAMP,
    AGG_GROUPING,
    AGG_REGR,
    AGG_STDDEV,
    AGG_STDDEV_POP,
    AGG_STDEEV_SAMP,
    AGG_VAR_POP,
    AGG_VAR_SAMP,
    AGG_VARIANCE,
    AGG_XMLAGG,
    AGG_STRAGG,

    //used as the index of array, its default number is the size of this enum
    AGG_FUNCTION_COUNT

} AGG;

typedef enum WINF
{
    // standard agg
    WINF_MAX,
    WINF_MIN,
    WINF_AVG,
    WINF_COUNT,
    WINF_SUM,
    WINF_FIRST,
    WINF_LAST,

    // window specific
    WINF_FIRST_VALUE,
    WINF_ROW_NUMBER,
    WINF_RANK,
    WINF_LAG,
    WINF_LEAD,
    //TODO

    // marker for number of functions
    WINF_FUNCTION_COUNT
} WINF;


extern MetadataLookupPlugin *assembleOracleMetadataLookupPlugin (void);

/* plugin methods */
extern int oracleInitMetadataLookupPlugin (void);
extern int oracleShutdownMetadataLookupPlugin (void);
extern int oracleDatabaseConnectionOpen (void);
extern int oracleDatabaseConnectionClose();
extern boolean oracleIsInitialized (void);

extern boolean oracleCatalogTableExists (char * tableName);
extern boolean oracleCatalogViewExists (char * viewName);
extern boolean oracleCheckPostive(char *tableName, char *colName);
extern Constant *oracleTransferRawData(char *data, char *dataType);
extern HashMap *oracleGetMinAndMax(char* tableName, char* colName);
extern int oracleGetRowNum(char* tableName);
extern int oracleGetDistinct(char* tableName, char* colName);
extern List *oracleGetHist(char *tableName, char *attrName, char *numPartitions);
extern char *oracleGet2DHist(char *tableName, char *attrName, char *attrName2, char *numPartitions1, char *numPartitions2);
char *appendStatement(char *tableName, char *attrName, char *attrName2, char *numPartitions);
extern void oracleStoreInterval(char *tableName, char *attrName, char *numPartitions);
List* getAttributeFromHist(char *hist1, char *hist2);
extern char *oracleJoin2Hist(char *hist1, char *hist2, char *tableName, char *attrName);
extern char *oracleComputeSumFromHist(char *tableName, char *attrName, char *sumAttrName, char *aggName);
char* computeNumOfGroupFromHist(char *tableName, char *attrName, char *sumAttrName);
List* getLastInterval(char *tableName, char *attrName, char *numPartitions);
extern char* oracleProjectionFiltering(char *tableName, char *num, Set *attrNames);
extern char* oracleSelectionFiltering(char *tableName, char *num, char *selection);
extern char* oracleGet1Dhist(char *tableName, char *colName, char *numPartitions);
extern double oracleComputeSelectivity(char *tableName, char *selection);
extern char* oracleGetSamples(int num, char *query, char *sampleTable);
extern char* oracleGetSampleStat(char * sampleName,char *aggrName,char *groupBy, char *sampleRate, char* count_table);
extern void oracleGetPartitionSizes(char* tableName, char* attr, char* partition[], char* size[], int length);
extern char* oracleGetPartitionSizes2(char *tableName, char*num);
extern char* oracleCreateSampleTable(char * sampleName,char *aggrName,char *groupBy);
extern char* oracleStorePartitionSizes(char* tableName, char* attr, char* partition[], int length);
extern char* oracleGetSamplesDirectly(char *attributes, char *partitionAttributes, char *sampleRate, char*tableName);
extern char* oracleDropTable(char *table);
extern char* oracleStoreSelectivty(char * stateName, char *psAttribute, char *aggregation, char *aggregationAttribute, char *groupbyAttribute, char *tableName, char *constant ,char* res, char *query, char *SampleRate);
extern char *oracleStoreGroupbyCount(char *groupby, char *groupby2, char* tablename);
extern char *oracleFindTheMax(char *query, char *aggName);
extern char *oracleGetSamples2(char *groupbyAttr, char* groupbyAttr1_groupbyAttr2, char *psAttr, char*sampleRate);
extern char *oraclePartialSample(char *tableName, char *groupbyAttr, int num);
extern int oracleGetCount(char *tableName);
extern char *oracleGetStatPartialSample(char *partialSampleTableName, char *aggrName, char*groupbyAttr,char* count_table);
extern char*oracleStoreSelectivty2(char * stateName, char *psAttribute, char *aggregation, char *aggregationAttribute, char *groupbyAttribute, char *tableName, char *constant ,char* res, char *query, char *SampleRate);
extern char*oracleInsertSelectivity(char *constant, char *groupbyAttr, char *agg_attr, char* ps_attr, char *agg_name, char* table, char *sampleRate);
extern char*oracleCreateTable(char* query, char* tablename);
//extern char* selectionCondition(Operator * cond);


extern List *oracleGetAttributes (char *tableName);
extern List *oracleGetAttributeNames (char *tableName);
extern Node *oracleGetAttributeDefaultVal (char *schema, char *tableName, char *attrName);
extern boolean oracleIsAgg(char *functionName);
extern boolean oracleIsWindowFunction(char *functionName);
extern char *oracleGetTableDefinition(char *tableName);
extern char *oracleGetViewDefinition(char *viewName);
extern DataType oracleGetOpReturnType (char *oName, List *dataTypes, boolean *opExists);
extern DataType oracleGetFuncReturnType (char *fName, List *dataTypes, boolean *funcExists);
extern gprom_long_t getBarrierScn(void);
extern int oracleGetCostEstimation(char *query);
extern List *oracleGetKeyInformation(char *tableName);
extern DataType oracleBackendSQLTypeToDT (char *sqlType);
extern char * oracleBackendDatatypeToSQL (DataType dt);


extern void oracleGetTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn);
extern gprom_long_t oracleGetCommitScn (char *tableName, gprom_long_t maxScn, char *xid);

extern Node *oracleExecuteAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel);
extern Relation *oracleGenExecQuery (char *query);
extern void oracleGenExecQueryIgnoreResult (char *query);

/* specific methods */
#if HAVE_ORACLE_BACKEND
#include <ocilib.h>
    extern OCI_Connection *getConnection();
#endif



#endif /* METADATA_LOOKUP_ORACLE_H_ */

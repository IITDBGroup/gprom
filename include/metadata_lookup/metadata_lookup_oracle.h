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
extern char *oracleTransferRawData(char *data, char *dataType);
extern HashMap *oracleGetMinAndMax(char* tableName, char* colName);

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

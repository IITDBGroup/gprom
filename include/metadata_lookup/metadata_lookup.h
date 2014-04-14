/*
 * metadata_lookup.h
 *
 *      Author: zephyr
 *      Get the metadata by table name.
 *
 *      catalogTableExists() checks if the given table exists
 *      getAttributes() gets all the attribute names of the given table
 *      Don't worry about the connection establishment. It will automatically connect to oracle server
 *      if not created.
 *      But you should call databaseConnectionClose() at the end of the whole program.
 *      Note that it need only be called once because it frees the list of attributes and all resources allocated
 *      for OCI connection.
 */

#ifndef METADATA_LOOKUP_H_
#define METADATA_LOOKUP_H_

#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/query_block/query_block.h"

#if HAVE_LIBOCILIB
#include <ocilib.h>
    extern OCI_Connection *getConnection();
#endif

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

extern int initMetadataLookupPlugin (void);
extern boolean catalogTableExists (char * tableName);
extern boolean catalogViewExists (char * viewName);
extern List *getAttributes (char *tableName);
extern List *getAttributeNames (char *tableName);
extern boolean isAgg(char *functionName);
extern boolean isWindowFunction(char *functionName);
extern char *getTableDefinition(char *tableName);
extern char *getViewDefinition(char *viewName);
extern void getTransactionSQLAndSCNs (char *xid, List **scns, List **sqls,
        List **sqlBinds, IsolationLevel *iso, Constant *commitScn);
extern Node *executeAsTransactionAndGetXID (List *statements, IsolationLevel isoLevel);
extern int databaseConnectionClose();

#endif /* METADATA_LOOKUP_H_ */

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

extern int initMetadataLookupPlugin (void);
extern boolean catalogTableExists (char * tableName);
extern boolean catalogViewExists (char * viewName);
extern List *getAttributes (char *tableName);
extern List *getAttributeNames (char *tableName);
extern boolean isAgg(char *functionName);
extern char *getTableDefinition(char *tableName);
extern char *getViewDefinition(char *viewName);
extern void getTransactionSQLAndSCNs (char *xid, List **scns, List **sqls);
extern int databaseConnectionClose();

#endif /* METADATA_LOOKUP_H_ */

/*-----------------------------------------------------------------------------
 *
 * prov_schema.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef PROV_SCHEMA_H_
#define PROV_SCHEMA_H_

#include "uthash.h"

#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"

/* data types */
typedef struct RelCount {
    char *relName;
    int count;
    UT_hash_handle hh;
} RelCount;

extern int getRelNameCount(RelCount **relCount, char *tableName);
extern List *getProvenanceAttributes(QueryOperator *q, ProvenanceType type);
extern List *getProvenanceAttrNames (char *table, List *attrs, int count);
extern char *getProvenanceAttrName (char *table, char *attr, int count);
extern List *getQBProvenanceAttrList (ProvenanceStmt *stmt);
extern char *escapeUnderscore (char *str);
#endif /* PROV_SCHEMA_H_ */

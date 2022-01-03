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

/* consts */
#define PROV_ATTR_PREFIX backendifyIdentifier("prov_")

extern int getCurRelNameCount(HashMap *relCount, char *tableName);
extern int increaseRefCount(HashMap *provCounts, char *prefix);

extern List *opGetProvAttrInfo(QueryOperator *op);
extern void copyProvInfo(QueryOperator *to, QueryOperator *from);
#define COPY_PROV_INFO(to,from) copyProvInfo((QueryOperator *) to, (QueryOperator *) from)

extern List *getProvenanceAttributes(QueryOperator *q, ProvenanceType type);
extern List *getProvenanceAttrNames (char *table, List *attrs, int count);
extern char *getProvenanceAttrName (char *table, char *attr, int count);
extern List *getCoarseGrainedAttrNames(char *table, List *attrs, int count);
extern char *getCoarseGrainedAttrName(char *table, char *attr, int count);
extern void getQBProvenanceAttrList (ProvenanceStmt *stmt, List **attrNames, List **dts);

#endif /* PROV_SCHEMA_H_ */

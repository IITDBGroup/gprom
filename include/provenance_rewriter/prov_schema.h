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

#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"

extern List *getProvenanceAttributes(QueryOperator *q, ProvenanceType type);
extern char *getProvenanceAttrName (char *table, char *attr, int count);
extern List *getQBProvenanceAttrList (ProvenanceStmt *stmt);

#endif /* PROV_SCHEMA_H_ */

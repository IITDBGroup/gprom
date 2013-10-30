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

extern List *getProvenanceAttributes(QueryOperator *q, ProvenanceType type);
extern char *getProvenanceAttrName (char *table, char *attr, int count);

#endif /* PROV_SCHEMA_H_ */

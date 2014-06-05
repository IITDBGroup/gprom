/*-----------------------------------------------------------------------------
 *
 * operator_property.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef OPERATOR_PROPERTY_H_
#define OPERATOR_PROPERTY_H_

/* define keys for properties used to annotate operators */
/* general operator properties */
#define PROP_MATERIALIZE "MATERIALIZE"                      // hint to materialize output of this operator

// provenance
#define PROP_SHOW_INTERMEDIATE_PROV "SHOW_INTERMEDIATE_PROV" // show provenance for this intermediate subquery result
#define PROP_USE_PROVENANCE "USE_PROVENANCE"                // duplicate user provided attributes as provenance
#define PROP_HAS_PROVENANCE "HAS_PROVENANCE"                // indicates the subquery already has associated provenance
#define PROP_USER_PROV_ATTRS "USER_PROV_ATTRS"              // list of user selected provenance attribtues

// provenance PI-CS composable
#define PROP_RESULT_TID_ATTR "RESULT_TID_ATTR"              // result tid attribute for PI-CS composable
#define PROP_PROV_DUP_ATTR "PROV_DUP_ATTR"                  // provenance duplicate counter attribute for PI-CS composable
#define PROP_PROVENANCE_OPERATOR_TUPLE_AT_A_TIME "PROVENANCE_OPERATOR_TUPLE_AT_A_TIME" // is the operator not sensitive to duplication

// provenance of transaction
#define PROP_PROV_IS_UPDATE_ROOT "UPDATE_ROOT"                         // root of a translated update reenactment query

/* Operator type specific properties */
/* provenance computation specific properties */
#define PROP_PC_TABLE "TABLE"                               // updated table to trace proveance of transaction for
#define PROP_PC_UPDATE_COND "UPDATE CONDS"                  // conditions of updates in transaction
#define PROP_PC_ONLY_UPDATED "ONLY UPDATED"                 // show only provenance of updated rows in transaction
#define PROP_PC_SHOW_INTERMEDIATE "SHOW ALL INTERMEDIATE"   // show provenance of all intermediate statements in transaction
#define PROP_PC_TRANS_XID "TRANSACTION_XID"                 // stores transaction XID

/* table access properties */
#define PROP_TABLE_IS_UPDATED "UPDATED TABLE"               // is table access for the updated table in an DML translation
#define PROP_USE_HISTORY_JOIN "USE_HISTORY_JOIN"            // get committed rows from history and join with table at transaction start to pre-filter updated rows

/* projection properties */
#define PROP_MERGE_ATTR_REF_CNTS "MERGE SAFE ATTRIBUTE COUNTS"                        // safe to merge this projection with its child

#endif /* OPERATOR_PROPERTY_H_ */

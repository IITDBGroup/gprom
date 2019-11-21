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
#define PROP_DUMMY_HAS_PROV_PROJ "DUMMY_HAS_PROV_PROJ"      // indicates that this is a dummy projection introduced for HAS PROVENANCE
#define PROP_USER_PROV_ATTRS "USER_PROV_ATTRS"              // list of user selected provenance attribtues
#define PROP_PROV_REL_NAME "PROVENANCE_REL_NAME"            // in provenance attributes refer to subquery as this name
#define PROP_PROV_ADD_REL_NAME "PROVENANCE_ADD_REL_NAME"            // in provenance attributes that are added refer to subquery as this name
#define PROP_ADD_PROVENANCE "ADD_PROVENANCE"                // add the following attribtues to as provenance attributes (but still rewrite and add normal provenance attrs too)
#define PROP_PROV_IGNORE_ATTRS "IGNORE_PROV_ATTRS"          // these attributes should be ignored during rewrite
#define PROP_TRANSLATE_AS "TRANSLATE AS"
#define PROP_TIP_ATTR "TIP_ATTR"							// indicates TIP attribute
#define PROP_INCOMPLETE_TABLE "INCOMPLETE_TABLE"			// indicates an incomplete table
#define PROP_XTABLE_GROUPID "XTABLE_GROUPID"				// indicates group id attribute in v tables
#define PROP_XTABLE_PROB "XTABLE_PROB"						// indicated probability attribute in v tables
#define PROP_HAS_RANGE "HAS_RANGE"							// Already has range labeling

// provenance summarization
#define PROP_SUMMARIZATION_DOSUM "DO_SUMMARIZATION"         // this property is
															// set if
															// summarization
															// should be done
#define PROP_SUMMARIZATION_IS_DL "SUMM_IS_DL"               // mark as coming
															// from a datalog
															// query
#define PROP_SUMMARIZATION_QTYPE "SUMM_Q_TYPE"              // type of
															// provenance
															// question (why or why-not)
#define PROP_SUMMARIZATION_TYPE "sumtype"                   // type of summary to be produced
#define PROP_SUMMARIZATION_TO_EXPLAIN "toexpl"              // result that should be explained
#define PROP_SUMMARIZATION_SAMPLE "sumsamp"                 // sample?
#define PROP_SUMMARIZATION_TOPK "topk"                      // top-k
#define PROP_SUMMARIZATION_QTYPE_WHY "WHY"
#define PROP_SUMMARIZATION_QTYPE_WHYNOT "WHYNOT"
#define PROP_SUMMARIZATION_VARREL "VAR_REL_PAIRS"
#define PROP_SUMMARIZATION_SAMPLE_PROPS "SAMPLE_PROPS"      // options for sampling
#define PROP_SUMMARIZATION_SC_PRECISION "sc_PRECISION"
#define PROP_SUMMARIZATION_SC_RECALL "sc_RECALL"
#define PROP_SUMMARIZATION_SC_INFORMATIVENESS "sc_INFORMATIVENESS"
#define PROP_SUMMARIZATION_TH_PRECISION "th_PRECISION"
#define PROP_SUMMARIZATION_TH_RECALL "th_RECALL"
#define PROP_SUMMARIZATION_TH_INFORMATIVENESS "th_INFORMATIVENESS"
#define PROP_SUMMARIZATION_FPATTERN "fpattern"

// reenactment
#define PROP_REENACT_SCHEMA_INFO "REENACT_SCHEMA"           // store additional table schemas to support reenactment of DDL commands
#define PROP_REENACT_ASOF "REENACT_AS_OF"                   // store as of when the reenactment should happen
#define PROP_REENACT_DO_NOT_TRACK_PROV "REENACT_DO_NOT_TRACK_PROV"     // do not track provenance for the statement marked in this way
#define PROP_REENACT_NO_TRACK_LIST "REENACTMENT_NO_PROV_TRACK_LIST"    // list that indicates for each reenacted statement whether to track provenance or not

// provenance PI-CS composable
#define PROP_RESULT_TID_ATTR "RESULT_TID_ATTR"              // result tid attribute for PI-CS composable
#define PROP_PROV_DUP_ATTR "PROV_DUP_ATTR"                  // provenance duplicate counter attribute for PI-CS composable
#define PROP_PROVENANCE_OPERATOR_TUPLE_AT_A_TIME "PROVENANCE_OPERATOR_TUPLE_AT_A_TIME" // is the operator not sensitive to duplication

// provenance of transaction
#define PROP_PROV_IS_UPDATE_ROOT "UPDATE_ROOT"              // root of a translated update reenactment query
#define PROP_PROV_ORIG_UPDATE_TYPE "ORIG_UPDATE_TYPE"       // store type of update for reenacted queries

/* Operator type specific properties */
/* provenance computation specific properties */
#define PROP_PC_PROV_TYPE "PROV_TYPE"                       // type of provenance to track
#define PROP_PC_TABLE "TRACK_TABLE"                         // updated table to trace proveance of transaction for
#define PROP_PC_UPDATE_COND "UPDATE_CONDS"                  // conditions of updates in transaction
#define PROP_PC_ONLY_UPDATED "ONLY_UPDATED"                 // show only provenance of updated rows in transaction
#define PROP_PC_SHOW_INTERMEDIATE "SHOW ALL INTERMEDIATE"   // show provenance of all intermediate statements in transaction
#define PROP_PC_TRANS_XID "TRANSACTION_XID"                 // stores transaction XID
#define PROP_PC_TUPLE_VERSIONS "TUPLE_VERSIONS"             // use rowid + scn pairs as provenance
#define PROP_PC_STATEMENT_ANNOTATIONS "STATEMENT_ANNOTATIONS" // statement annotations
#define PROP_PC_STATEMENT_ANNOT_ATTR "ST_ATTR"              // attribute storing the statement annotion
#define PROP_PC_PROJ_TO_REMOVE_SANNOT "PROJ_REMOVES_S_ANN"  // marks projection operator that removes statement annotations
#define PROP_PC_VERSION_SCN_ATTR "SCN_ATTR"                 // attribute storing the version annotation
#define PROP_PC_REENACT_METADATA "REENACT_METADATA"         // store table information for reenacting DDL commands
#define PROP_PC_GEN_PROVENANCE "GENERATE_PROVENANCE"        // used for REENACT to indicate that provenance should be computed
#define PROP_PC_REQUIRES_POSTFILTERING "REQUIRES_POSTFILTER"    // set to true if the output of reenactment needs to be filtered based on version annotation attributes
#define PROP_PC_ISOLATION_LEVEL "REENACT_ISOLEVEL"          // set isolation level for reenactment
#define PROP_PC_COMMIT_SCN "COMMIT_SCN"                     // stores commit SCN for REENACT WITH COMMIT SCN
#define PROP_PC_SEMIRING_COMBINER "SEMIRING_COMBINER"       // use combiner in provenance computation
#define PROP_PC_SC_AGGR_OPT "SEMIRING_COMBINER_AGGR_OPT" //use aggregation optimization in semiring combiner

/* table access properties */
#define PROP_TABLE_IS_UPDATED "UPDATED_TABLE"               // is table access for the updated table in an DML translation
#define PROP_IS_READ_COMMITTED "TABLE_READ_COMMITTED"      // is table access for updated table in a READ COMMITTED transaction
#define PROP_USE_HISTORY_JOIN "USE_HISTORY_JOIN"            // get committed rows from history and join with table at transaction start to pre-filter updated rows
#define PROP_TABLE_USE_ROWID_VERSION "USE_ROW_ID_VERSION"   // use rowid and version as provenance

/* projection properties */
#define PROP_MERGE_ATTR_REF_CNTS "MERGE SAFE ATTRIBUTE COUNTS"                        // safe to merge this projection with its child
#define PROP_PROJ_PROV_ATTR_DUP "PROJECTION WITH PROVENANCE ATTRIBUTE DUPLICATION"    // needed by projection pull-up
#define PROP_PROJ_PROV_ATTR_DUP_PULLUP "PROJECTION WITH PROVENANCE ATTRIBUTE DUPLICATION PULL-UP"  //no need to pull-up if this op already pull-up

/* properties used to store list of set which used in selection move around */
#define PROP_STORE_LIST_SET_SELECTION_MOVE_AROUND "STORE LIST SET FOR SELECTION MOVE AROUND"
#define PROP_OPT_SELECTION_MOVE_AROUND_DONE "HAVE DONE SELECTION MOVE AROUND"
#define PROP_OPT_UNNECESSARY_COLS_REMOVED_DONE "HAVE REMOVED UNNECESSARY"
#define PROP_STORE_MERGE_DONE "MERGE IS DONE"
#define PROP_STORE_REMOVE_RED_PROJ_DONE "REMOVED RED PROJ"
#define PROP_STORE_REMOVE_RED_DUP_BY_KEY_DONE "REMOVED DUP BY KEY"
#define PROP_OPT_REMOVE_RED_DUP_BY_SET_DONE "REMOVED DUP BY SET"
#define PROP_OPT_REMOVE_RED_WIN_DONW "REMOVED WIN OP"
//#define PROP_MERGE_ATTR_REF_CNTS "MERGE SAFE ATTRIBUTE COUNTS"                        // safe to merge this projection with its child?

/* properties to store characteristics of operators for heuristic optimization */
#define PROP_STORE_LIST_KEY "STORE KEY LIST FOR REMOVE REDUNDANT DUPLICATE"
#define PROP_STORE_LIST_KEY_DONE "HAVE COMPUTED KEYS"
#define PROP_STORE_BOOL_SET "STORE SET PROPERTY FOR REMOVE REDUNDANT DUPLICATE"
#define PROP_STORE_BOOL_SET_ALL_PARENTS_DONE "SET PROPERTY ALL PARENT OPS HAVE BEEN PROCESSED"
#define PROP_STORE_BOOL_SET_DONE "SET PROPERTY COMPUTED"
#define PROP_STORE_SET_ICOLS "STORE ICOLS PROPERTY FOR REMOVE REDUNDANT DUPLICATE"
#define PROP_STORE_SET_ICOLS_DONE "DONE STORE ICOLS PROPERTY FOR REMOVE REDUNDANT DUPLICATE"
#define PROP_STORE_LIST_SCHEMA_NAMES "STORE SCHEMA NAMES PROPERTY FOR REMOVE REDUNDANT DUPLICATE"
#define PROP_STORE_SET_EC "STORE EC PROPERTY"
#define PROP_STORE_SET_EC_DONE_BU "STORE EC PROPERTY - DONE BOTTOM UP"
#define PROP_STORE_SET_EC_DONE_TD "STORE EC PROPERTY - DONE TOP DOWN"
#define PROP_STORE_DUP_MARK "STORE DUP PROPERTY"  //pull up dup op, avoid loop the same dup op two times

/* properties for temporal queries */
#define PROP_TEMP_TBEGIN_ATTR "TEMPORAL_INTERVAL_BEGIN"
#define PROP_TEMP_TEND_ATTR "TEMPORAL_INTERVAL_END"
#define PROP_TEMP_DO_COALESCE "TEMPORAL_DO_COALESCE"
#define PROP_TEMP_DO_SET_COALESCE "TEMPORAL_DO_SET_COALESCE"
#define PROP_TEMP_NORMALIZE_INPUTS "TEMPORAL_NORM_INPUTS"
#define PROP_TEMP_TNTAB "PROP_TEMP_TNTAB"
#define PROP_TEMP_IS_MINMAX "PROP_TEMP_IS_MINMAX"
#define PROP_TEMP_ATTR_DT "PROP_TEMP_ATTR_DT"

/* fromProvInfo provProperties */
#define PROV_PROP_TIP_ATTR "PROV_PROP_TIP_ATTR"
#define PROV_PROP_INCOMPLETE_TABLE "PROV_PROP_INCOMPLETE_TABLE"
#define PROV_PROP_XTABLE_GROUPID "PROV_PROP_XTABLE_GROUPID"
#define PROV_PROP_XTABLE_PROB "PROV_PROP_XTABLE_PROB"
#define PROV_PROP_RADB "PROV_PROP_RADB"
#define PROV_PROP_RADB_LIST "PROV_PROP_RADB_LIST"

#endif /* OPERATOR_PROPERTY_H_ */

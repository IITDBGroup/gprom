/*-----------------------------------------------------------------------------
 *
 * analyze_dl.h
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_ANALYSIS_AND_TRANSLATE_ANALYZE_DL_H_
#define INCLUDE_ANALYSIS_AND_TRANSLATE_ANALYZE_DL_H_

#include "model/node/nodetype.h"
#include "model/datalog/datalog_model.h"
#include "model/graph/graph.h"

extern Node *analyzeDLModel(Node *stmt);
extern void createDLanalysisStructures(DLProgram *p, boolean cleanup, boolean createRelToRelG, boolean createBityPredToRule);
extern void createRelToRuleMap(Node *stmt);
extern Graph *createRelToRelGraph(Node *stmt);
extern Graph *createInvRelToRelGraph(Node *stmt);
extern HashMap *createBodyPredToRuleMap(DLProgram *p);
extern boolean hasAggFunction(Node *n);
extern boolean atomHasExprs(DLAtom *a);
extern List *getEDBFDs(DLProgram *p);

#define ENSURE_REL_TO_REL_GRAPH(_s) \
	do { \
		if(!DL_HAS_PROP(_s,DL_REL_TO_REL_GRAPH)) \
		{ \
			createRelToRelGraph((Node *) _s);	\
		} \
	} while (0)
#define ENSURE_INV_REL_TO_REL_GRAPH(_s) \
	do { \
		if(!DL_HAS_PROP(_s,DL_INV_REL_TO_REL_GRAPH)) \
		{ \
			createInvRelToRelGraph((Node *) _s);	\
		} \
	} while (0)
#define GET_INV_REL_TO_REL_GRAPH(_s)								\
	(DL_HAS_PROP(_s,DL_INV_REL_TO_REL_GRAPH) ?						\
	 (Graph *) getDLProp((DLNode *) _s, DL_INV_REL_TO_REL_GRAPH) :	\
	 createInvRelToRelGraph((Node *) _s))
#define ENSURE_BODY_PRED_TO_RULE_MAP(_p) \
	do {\
		if(!DL_HAS_PROP(_p, DL_MAP_BODYPRED_TO_RULES)) \
		{ \
			createBodyPredToRuleMap((DLProgram *) _p);	\
		} \
	} while (0)
#define ENSURE_DL_CHECKED(_p)						\
	do {											\
		if(!DL_HAS_PROP(_p, DL_IS_CHECKED))			\
		{											\
			checkDLModel((Node *) _p);				\
			ENSURE_REL_TO_REL_GRAPH(_p);			\
			ENSURE_BODY_PRED_TO_RULE_MAP(_p);		\
			DL_SET_BOOL_PROP(_p, DL_IS_CHECKED);	\
		}											\
	} while(0)

#endif /* INCLUDE_ANALYSIS_AND_TRANSLATE_ANALYZE_DL_H_ */

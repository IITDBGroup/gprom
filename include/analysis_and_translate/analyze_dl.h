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

extern Node *analyzeDLModel (Node *stmt);
extern void createRelToRuleMap (Node *stmt);
extern void createRelToRelGraph(Node *stmt);
#define ENSURE_REL_TO_REL_GRAPH(_s) \
	do { \
		if(!DL_HAS_PROP(_s,DL_REL_TO_REL_GRAPH)) \
		{ \
			createRelToRelGraph((Node *) _s);	\
		} \
	} while (0)



#endif /* INCLUDE_ANALYSIS_AND_TRANSLATE_ANALYZE_DL_H_ */

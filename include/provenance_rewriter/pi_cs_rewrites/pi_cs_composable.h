/*-----------------------------------------------------------------------------
 *
 * pi_cs_composable.h
 *		- PI CS implementation that creates two additional columns _oid and
 *		_pid which identify duplicates of the same original result tuple (_oid)
 *		and different duplicates of the same original tuple (_pid).
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef PI_CS_COMPOSABLE_H_
#define PI_CS_COMPOSABLE_H_

#include "model/query_operator/query_operator.h"

extern QueryOperator *rewritePI_CSComposable (ProvenanceComputation *op);

#endif /* PI_CS_COMPOSABLE_H_ */

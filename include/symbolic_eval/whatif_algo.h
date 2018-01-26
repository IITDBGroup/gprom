/*-----------------------------------------------------------------------------
 *
 * whatif_algo.h
 *		
 *
 *		AUTHOR: Bahareh
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_WHATIF_ALGORITHM_H_
#define INCLUDE_WHATIF_ALGORITHM_H_

#include "model/list/list.h"

extern List *dependAlgo(List *exprs);
extern boolean checkCplex(List *exprs);
extern List *SymbolicExeAlgo(List *exprs);

#endif /* WHATIF_ALGORITHM_H_ */

/*
 * unnest_neumann.h
 *      Implements the rewrites in https://btw-2015.informatik.uni-hamburg.de/res/proceedings/Hauptband/Wiss/Neumann-Unnesting_Arbitrary_Querie.pdf
 *      TODO: Use https://doi.org/10.18420/BTW2025-01
 *
 *      Author: Felix
 */


#include "model/query_operator/query_operator.h"

extern QueryOperator *neumanning(QueryOperator *);

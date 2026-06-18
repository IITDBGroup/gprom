#ifndef PRUNE_H
#define PRUNE_H

#include "helperFunctions.h"

Int4Range prune_lt_internal_range(Int4Range a, Int4Range b, bool direction);
Int4Range prune_gt_internal_range(Int4Range a, Int4Range b, bool direction);
Int4Range prune_lte_internal_range(Int4Range a, Int4Range b, bool direction);
Int4Range prune_gte_internal_range(Int4Range a, Int4Range b, bool direction);
Int4Range prune_eq_internal_range(Int4Range a, Int4Range b);
Int4Range prune_AND_internal_range(Int4Range a, Int4Range b);
Int4RangeSet prune_OR_internal_range(Int4Range a, Int4Range b);     // returns a set

Int4RangeSet prune_lt_set_internal(Int4RangeSet a, Int4RangeSet b, bool direction);
Int4RangeSet prune_lte_set_internal(Int4RangeSet a, Int4RangeSet b, bool direction);
Int4RangeSet prune_gt_set_internal(Int4RangeSet a, Int4RangeSet b, bool direction);
Int4RangeSet prune_gte_set_internal(Int4RangeSet a, Int4RangeSet b, bool direction);
Int4RangeSet prune_eq_set_internal(Int4RangeSet a, Int4RangeSet b);
Int4RangeSet prune_AND_internal_set(Int4RangeSet a, Int4RangeSet b);
Int4RangeSet prune_OR_internal_set(Int4RangeSet a, Int4RangeSet b);

#endif
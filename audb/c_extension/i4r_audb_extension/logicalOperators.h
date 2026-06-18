#ifndef LOGICALOPERATORS_H
#define LOGICALOPERATORS_H

#include "helperFunctions.h"

// 3VL 1 true, -1 null, 0 false 

int range_greater_than(Int4Range a, Int4Range b);
int range_less_than(Int4Range a, Int4Range b);
int range_greater_than_equal(Int4Range a, Int4Range b);
int range_less_than_equal(Int4Range a, Int4Range b);
int range_equal_internal(Int4Range a, Int4Range b);

int set_less_than(Int4RangeSet a, Int4RangeSet b);
int set_less_than_equal(Int4RangeSet a, Int4RangeSet b);
int set_greater_than(Int4RangeSet a, Int4RangeSet b);
int set_greater_than_equal(Int4RangeSet a, Int4RangeSet b);
int set_equal_internal(Int4RangeSet a, Int4RangeSet b);

#endif
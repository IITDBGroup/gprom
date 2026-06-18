#ifndef ARITHMETIC_H
#define ARITHMETIC_H

#include "helperFunctions.h"

// add extra utilites for working with defined type
void printRangeSet(Int4RangeSet a);
bool validRange(Int4Range a);
int MIN(int My_array[], int len);
int MAX(int My_array[], int len);

// Range functions
Int4Range range_add_internal(Int4Range a, Int4Range b);
Int4Range range_subtract_internal(Int4Range a, Int4Range b);
Int4Range range_multiply_internal(Int4Range a, Int4Range b);
Int4Range range_divide_internal(Int4Range a, Int4Range b);

// Set functions
Int4RangeSet range_set_add_internal(Int4RangeSet a, Int4RangeSet b);
Int4RangeSet range_set_subtract_internal(Int4RangeSet a, Int4RangeSet b);
Int4RangeSet range_set_multiply_internal(Int4RangeSet a, Int4RangeSet b);
Int4RangeSet range_set_divide_internal(Int4RangeSet a, Int4RangeSet b);

Int4RangeSet agg_range_set_add_internal(Int4RangeSet a, Int4RangeSet b);

/*
  Helper function to multiply float interval set * float interval set. 
  Does not handle edge cases like mult = (0,n), or a is empty. This is handled caller function
*/ 
Int4Range floatIntervalSetMult(Int4RangeSet a, Multiplicity mult);
#endif

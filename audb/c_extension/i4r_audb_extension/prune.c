#include "postgres.h"
#include <stdlib.h>
#include <stdio.h>
#include "logicalOperators.h"
#include "prune.h"

#define malloc palloc
#define free pfree

#define INT_MIN_LOCAL INT32_MIN
#define INT_MAX_LOCAL INT32_MAX

//////////////////////////
////////// Range /////////
//////////////////////////

// handles strict less than: e1 < e2
// returns the proper range depending on direction 
Int4Range prune_lt_internal_range(Int4Range a, Int4Range b, bool direction) {
    Int4Range result;
    result.isNull = false;

    // 0 = left, 1 = right
    if (direction == 0) { 
        Int4Range bound;
        bound.lower = INT32_MIN;
        bound.upper = b.upper - 1;  // [-inf, max(b)-1]
        bound.isNull = false;
        result = intersect_range(a, bound);
    } 
    else { 
        Int4Range bound;
        bound.lower = a.lower + 1;  // [min(a)+1, +inf]
        bound.upper = INT32_MAX;
        bound.isNull = false;
        result = intersect_range(b, bound); // returns isNull on invalid range, but we check this expicilty regardless
    }

    // maybe invalidate solution also if either bound is int min/max, but this should automatically solve in intersect function
    if (!validRange(result)) result.isNull = true;
    return result;
}

// andles strict less than or equal: e1 < e2
Int4Range prune_lte_internal_range(Int4Range a, Int4Range b, bool direction) {
    Int4Range result;
    result.isNull = false;

    // 0 = left, 1 = right
    if (direction == 0) { 
        Int4Range bound;
        bound.lower = INT32_MIN;
        bound.upper = b.upper;  // [-inf, max(b)]
        bound.isNull = false;
        result = intersect_range(a, bound);
    } 
    else { 
        Int4Range bound;
        bound.lower = a.lower;  // [min(a), +inf]
        bound.upper = INT32_MAX;
        bound.isNull = false;
        result = intersect_range(b, bound);
    }

    if (!validRange(result)) result.isNull = true;
    return result;
}

// handles strict greater than: e1 > e2
Int4Range prune_gt_internal_range(Int4Range a, Int4Range b, bool direction) {
    Int4Range result;
    result.isNull = false;

    if (direction == 0) {
        Int4Range bound;
        bound.lower = b.lower + 1; // [min(b)+1, +inf]
        bound.upper = INT32_MAX;
        bound.isNull = false;
        result = intersect_range(a, bound);
    } 
    else {
        Int4Range bound;
        bound.lower = INT32_MIN;
        bound.upper = a.upper - 1; // [-inf, max(a)-1]
        bound.isNull = false;
        result = intersect_range(b, bound);
    }

    if (!validRange(result)) result.isNull = true;
    return result;
}

// handles greater than or equal: e1 >= e2
Int4Range prune_gte_internal_range(Int4Range a, Int4Range b, bool direction) {
    Int4Range result;
    result.isNull = false;

    if (direction == 0) {
        Int4Range bound;
        bound.lower = b.lower; // [min(b), +inf]
        bound.upper = INT32_MAX;
        bound.isNull = false;
        result = intersect_range(a, bound);
    } 
    else {
        Int4Range bound;
        bound.lower = INT32_MIN;
        bound.upper = a.upper; // [-inf, max(a)]
        bound.isNull = false;
        result = intersect_range(b, bound);
    }

    if (!validRange(result)) result.isNull = true;
    return result;
}

// all points of intersection. both sides shrink
Int4Range prune_eq_internal_range(Int4Range a, Int4Range b) {
    Int4Range result;
    result.isNull = false;

    result.lower = max2(a.lower, b.lower);
    result.upper = min2(a.upper, b.upper);

    if (!validRange(result)) {
        result.isNull = true;
        result.lower = -1;
        result.upper = -1;
        return result;
    }
    return result;
}

// NOTE: and is binary not unary. combining conditions required stacking ands
// all points of intersection. both sides shrink
Int4Range prune_AND_internal_range(Int4Range a, Int4Range b) {
    return prune_eq_internal_range(a, b);
}

// combines all ranges into a set. Returns at most size 2 set
Int4RangeSet prune_OR_internal_range(Int4Range a, Int4Range b) {
    Int4RangeSet result, rv;
    result.count = 0;
    result.containsNull = false;
    result.ranges = (Int4Range*) malloc(sizeof(Int4Range) * 2); 

    if (a.isNull && b.isNull) {
        result.containsNull = true;
        return result;
    }

    if (a.isNull) {
        result.ranges[0] = b;
        result.count = 1;
        return result;
    }

    if (b.isNull) {
        result.ranges[0] = a;
        result.count = 1;
        return result;
    }

    // Overlapping or contiguous ranges
    if (a.upper > b.lower && b.upper > a.lower) {
        result.ranges[0].lower = (a.lower < b.lower) ? a.lower : b.lower;
        result.ranges[0].upper = (a.upper > b.upper) ? a.upper : b.upper;
        result.ranges[0].isNull = false;
        result.count = 1;
    } 
    else {
        // Disjoint ranges → both kept in order
        if (a.lower < b.lower) {
            result.ranges[0] = a;
            result.ranges[1] = b;
        } else {
            result.ranges[0] = b;
            result.ranges[1] = a;
        }
        result.count = 2;
    }

    rv = normalize(result);
    free(result.ranges);

    return rv;
}

//////////////////////////
////////// SET ///////////
//////////////////////////

// all points of intersection fpr set. both sides shrink
// pairwise intersection
Int4RangeSet prune_eq_set_internal(Int4RangeSet a, Int4RangeSet b) {
    Int4RangeSet result, rv;
    Int4Range temp;
    int pa = 0; 
    int pb = 0;
    Int4RangeSet norm_a;
    Int4RangeSet norm_b;

    result.count = 0;
    result.containsNull = false;
    result.ranges = palloc(sizeof(Int4Range) * (a.count + b.count));

    norm_a = normalize(a);
    norm_b = normalize(b);

    while (pa < norm_a.count && pb < norm_b.count) {
        temp = intersect_range(norm_a.ranges[pa], norm_b.ranges[pb]);

        if (!temp.isNull) {
            result.ranges[result.count++] = temp;
        }

        // advance whatever range ends earlier. it cant intersect anything further right
        if (norm_a.ranges[pa].upper < norm_b.ranges[pb].upper) {
            pa++;
        } else if (norm_b.ranges[pb].upper < norm_a.ranges[pa].upper) {
            pb++;
        } else {
            // equal upper bounds, both are exhausted against each other
            pa++;
            pb++;
        }
    }

    rv = normalize(result);

    free(norm_a.ranges);
    free(norm_b.ranges);
    free(result.ranges);
    
    return rv;
}

// NOTE: and is binary not unary. combining conditions required stacking ands
// pairwise intersection
Int4RangeSet prune_AND_internal_set(Int4RangeSet a, Int4RangeSet b) {
    return prune_eq_set_internal(a, b);
}

// concat and normalize
// O(N + M)
Int4RangeSet prune_OR_internal_set(Int4RangeSet a, Int4RangeSet b) {
    Int4RangeSet result, rv;

    result.count = 0;
    result.containsNull = a.containsNull || b.containsNull;
    result.ranges = palloc(sizeof(Int4Range) * (a.count + b.count));

    // copy a
    for (int i = 0; i < a.count; i++) {
        result.ranges[result.count++] = a.ranges[i];
    }

    // copy b 
    for (int i = 0; i < b.count; i++) {
        result.ranges[result.count++] = b.ranges[i];
    }

    rv = normalize(result);
    free(result.ranges);
    return rv;
}

// handles strict less than: e1 < e2 for sets
Int4RangeSet prune_lt_set_internal(Int4RangeSet a, Int4RangeSet b, bool direction) {
    Int4RangeSet result;
    Int4RangeSet norm_a;
    Int4RangeSet norm_b;
    Int4Range temp;
    int pa;
    int pb;

    result.count = 0;
    result.containsNull = false;

    // O(NlogN)
    norm_a = normalize(a);
    norm_b = normalize(b);
    pa = 0;
    pb = 0;

    result.ranges = palloc(sizeof(Int4Range) * (norm_a.count + norm_b.count));

    if (direction == 0) {
        // advance pa on valid, advance pb on null
        while (pa < norm_a.count && pb < norm_b.count) {
            temp = prune_lt_internal_range(norm_a.ranges[pa], norm_b.ranges[pb], 0);

            if (temp.isNull) {
                // b[pb] is too far left to constrain a[pa], move b forward
                pb++;
            } else {
                result.ranges[result.count++] = temp;
                pa++;
            }
        }
    } else {
        // constrain b: mirror — advance pb on valid, advance pa on null
        while (pa < norm_a.count && pb < norm_b.count) {
            temp = prune_lt_internal_range(norm_a.ranges[pa], norm_b.ranges[pb], 1);

            if (temp.isNull) {
                // a[pa] is too far left to constrain b[pb], move a forward
                pa++;
            } else {
                result.ranges[result.count++] = temp;
                pb++;
            }
        }
    }

    return normalize(result);
}

// handles less than equal: e1 <= e2 for sets
Int4RangeSet prune_lte_set_internal(Int4RangeSet a, Int4RangeSet b, bool direction) {
    Int4RangeSet result, rv;
    Int4RangeSet norm_a;
    Int4RangeSet norm_b;
    Int4Range temp;
    int pa;
    int pb;

    result.count = 0;
    result.containsNull = false;

    // O(NlogN)
    norm_a = normalize(a);
    norm_b = normalize(b);
    pa = 0;
    pb = 0;

    result.ranges = palloc(sizeof(Int4Range) * (norm_a.count + norm_b.count));

    if (direction == 0) {
        // advance pa on valid, advance pb on null
        while (pa < norm_a.count && pb < norm_b.count) {
            temp = prune_lte_internal_range(norm_a.ranges[pa], norm_b.ranges[pb], 0);

            if (temp.isNull) {
                // b[pb] is too far left to constrain a[pa], move b forward
                pb++;
            } else {
                result.ranges[result.count++] = temp;
                pa++;
            }
        }
    } else {
        // constrain b: mirror — advance pb on valid, advance pa on null
        while (pa < norm_a.count && pb < norm_b.count) {
            temp = prune_lte_internal_range(norm_a.ranges[pa], norm_b.ranges[pb], 1);

            if (temp.isNull) {
                // a[pa] is too far left to constrain b[pb], move a forward
                pa++;
            } else {
                result.ranges[result.count++] = temp;
                pb++;
            }
        }
    }

    rv = normalize(result);
    
    free(norm_a.ranges);
    free(norm_b.ranges);
    free(result.ranges);
    
    return rv;
}

// handles less than equal: e1 <= e2 for sets
Int4RangeSet prune_gt_set_internal(Int4RangeSet a, Int4RangeSet b, bool direction) {
    Int4RangeSet result, rv;
    Int4RangeSet norm_a;
    Int4RangeSet norm_b;
    Int4Range temp;
    int pa;
    int pb;

    result.count = 0;
    result.containsNull = false;

    // O(NlogN)
    norm_a = normalize(a);
    norm_b = normalize(b);
    pa = 0;
    pb = 0;

    result.ranges = palloc(sizeof(Int4Range) * (norm_a.count + norm_b.count));

    if (direction == 0) {
        // advance pa on valid, advance pb on null
        while (pa < norm_a.count && pb < norm_b.count) {
            temp = prune_gt_internal_range(norm_a.ranges[pa], norm_b.ranges[pb], 0);

            if (temp.isNull) {
                // b[pb] is too far left to constrain a[pa], move b forward
                pb++;
            } else {
                result.ranges[result.count++] = temp;
                pa++;
            }
        }
    } else {
        // constrain b: mirror — advance pb on valid, advance pa on null
        while (pa < norm_a.count && pb < norm_b.count) {
            temp = prune_gt_internal_range(norm_a.ranges[pa], norm_b.ranges[pb], 1);

            if (temp.isNull) {
                // a[pa] is too far left to constrain b[pb], move a forward
                pa++;
            } else {
                result.ranges[result.count++] = temp;
                pb++;
            }
        }
    }

    rv = normalize(result);

    free(norm_a.ranges);
    free(norm_b.ranges);
    free(result.ranges);
    
    return rv;   
}

// handles less than equal: e1 <= e2 for sets
Int4RangeSet prune_gte_set_internal(Int4RangeSet a, Int4RangeSet b, bool direction) {
    Int4RangeSet result, rv;
    Int4RangeSet norm_a;
    Int4RangeSet norm_b;
    Int4Range temp;
    int pa;
    int pb;

    result.count = 0;
    result.containsNull = false;

    // O(NlogN)
    norm_a = normalize(a);
    norm_b = normalize(b);
    pa = 0;
    pb = 0;

    result.ranges = palloc(sizeof(Int4Range) * (norm_a.count + norm_b.count));

    if (direction == 0) {
        // advance pa on valid, advance pb on null
        while (pa < norm_a.count && pb < norm_b.count) {
            temp = prune_gte_internal_range(norm_a.ranges[pa], norm_b.ranges[pb], 0);

            if (temp.isNull) {
                // b[pb] is too far left to constrain a[pa], move b forward
                pb++;
            } else {
                result.ranges[result.count++] = temp;
                pa++;
            }
        }
    } else {
        // constrain b: mirror — advance pb on valid, advance pa on null
        while (pa < norm_a.count && pb < norm_b.count) {
            temp = prune_gte_internal_range(norm_a.ranges[pa], norm_b.ranges[pb], 1);

            if (temp.isNull) {
                // a[pa] is too far left to constrain b[pb], move a forward
                pa++;
            } else {
                result.ranges[result.count++] = temp;
                pb++;
            }
        }
    }

    rv = normalize(result);

    free(norm_a.ranges);
    free(norm_b.ranges);
    free(result.ranges);
    
    return rv;
}
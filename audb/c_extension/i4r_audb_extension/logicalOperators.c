#include "postgres.h"
#include "logicalOperators.h"

#define malloc palloc
#define free pfree

// 3VL 1 true, -1 null, 0 false 
int range_greater_than(Int4Range a, Int4Range b){
    if (a.lower > (b.upper-1)) {
        return 1;
    }
    else if ((a.upper-1) < b.lower) {
        return 0;
    }
    return -1;
}

int range_less_than(Int4Range a, Int4Range b){
    if (a.lower > (b.upper-1)) {
        return 0;
    }
    else if ((a.upper-1) < b.lower) {
        return 1;
    }
    return -1;
}

int range_greater_than_equal(Int4Range a, Int4Range b){
    if (a.lower >= (b.upper-1)) {
        return 1;
    }
    else if ((a.upper-1) < b.lower) {
        return 0;
    }
    return -1;
}

int range_less_than_equal(Int4Range a, Int4Range b){
    if (a.lower > (b.upper-1)) {
        return 0;
    }
    else if ((a.upper-1) <= b.lower) {
        return 1;
    }
    return -1;
}

// determines whether full equality (a=b=c=d), partial equality (overlap), or no equality (disjoint)
int range_equal_internal(Int4Range a, Int4Range b) {
    if (a.lower == (a.upper-1) && (a.upper-1) == (b.lower) && (b.lower) == (b.upper-1)) {        
        return 1;
    }
    else if (overlap(a, b)) {
        return -1;  // NULL (uncertain)
    }
    else if (a.lower != b.lower || a.upper != b.upper){
        return 0;
    }
    return 1;
}

int set_greater_than(Int4RangeSet a, Int4RangeSet b){
    int result;
    Int4RangeSet n1;
    Int4RangeSet n2;
    
    if (a.count == 0 && b.count == 0) return 0;
    if (b.count == 0) return 1;
    if (a.count == 0) return 0;
    
    result = -1;
    n1 = normalize(a);
    n2 = normalize(b);

    if (n1.count > 0 && n2.count > 0) {
        Int4Range f1;
        Int4Range f2;
        Int4Range l1;
        Int4Range l2;

        f1 = n1.ranges[0];
        f2 = n2.ranges[0];
        l1 = n1.ranges[n1.count-1];
        l2 = n2.ranges[n2.count-1];

        if (f1.lower > (l2.upper-1)){
            result = 1;
        }
        else if ((l1.upper-1) < f2.lower){
            result = 0;
        }
    }

    free(n1.ranges);
    free(n2.ranges);

    return result;
}

int set_greater_than_equal(Int4RangeSet a, Int4RangeSet b){
    int result;
    Int4RangeSet n1;
    Int4RangeSet n2;
    
    if (a.count == 0 && b.count == 0) return 1;
    if (b.count == 0) return 1;
    if (a.count == 0) return 0;
    
    result = -1;
    n1 = normalize(a);
    n2 = normalize(b);

    if (n1.count > 0 && n2.count > 0) {
        Int4Range f1;
        Int4Range f2;
        Int4Range l1;
        Int4Range l2;

        f1 = n1.ranges[0];
        f2 = n2.ranges[0];
        l1 = n1.ranges[n1.count-1];
        l2 = n2.ranges[n2.count-1];

        if (f1.lower >= (l2.upper-1)){
            result = 1;
        }
        else if ((l1.upper-1) < f2.lower){
            result = 0;
        }
    }

    free(n1.ranges);
    free(n2.ranges);

    return result;
}


int set_less_than(Int4RangeSet a, Int4RangeSet b){
    int result;
    Int4RangeSet n1;
    Int4RangeSet n2;
    
    if (a.count == 0 && b.count == 0) return 0;
    if (a.count == 0) return 1;
    if (b.count == 0) return 0;
    
    result = -1;
    n1 = normalize(a);
    n2 = normalize(b);

    if (n1.count > 0 && n2.count > 0) {
        Int4Range f1;
        Int4Range f2;
        Int4Range l1;
        Int4Range l2;

        f1 = n1.ranges[0];
        f2 = n2.ranges[0];
        l1 = n1.ranges[n1.count-1];
        l2 = n2.ranges[n2.count-1];

        if (f1.lower > (l2.upper-1)){
            result = 0;
        }
        else if ((l1.upper-1) < f2.lower){
            result = 1;
        }
    }

    free(n1.ranges);
    free(n2.ranges);

    return result;
}

int set_less_than_equal(Int4RangeSet a, Int4RangeSet b){
    int result;
    Int4RangeSet n1;
    Int4RangeSet n2;
    
    if (a.count == 0 && b.count == 0) return 1;
    if (a.count == 0) return 1;
    if (b.count == 0) return 0;
    
    result = -1;
    n1 = normalize(a);
    n2 = normalize(b);

    if (n1.count > 0 && n2.count > 0) {
        Int4Range f1;
        Int4Range f2;
        Int4Range l1;
        Int4Range l2;

        f1 = n1.ranges[0];
        f2 = n2.ranges[0];
        l1 = n1.ranges[n1.count-1];
        l2 = n2.ranges[n2.count-1];

        if (f1.lower > (l2.upper-1)){
            result = 0;
        }
        else if ((l1.upper-1) <= f2.lower){
            result = 1;
        }
    }

    free(n1.ranges);
    free(n2.ranges);

    return result;
}


int set_equal_internal(Int4RangeSet a, Int4RangeSet b) {
    // both empty = equal
    if (a.count == 0 && b.count == 0) {
        return 1;
    }
    
    // either empty
    if (a.count == 0 || b.count == 0) {
        return 0;
    }
    
    // diff count
    if (a.count != b.count) {
        return 0;
    }

    // vacuously true case: both single-element ranges [5,6) = [5,6)
    // a=b=c=d
    if (a.count == 1 && b.count == 1) {
        if ((a.ranges[0].lower == a.ranges[0].upper - 1) &&
            (b.ranges[0].lower == b.ranges[0].upper - 1) &&
            (a.ranges[0].lower == b.ranges[0].lower)) {
            return 1;
        }
    }    

    // check for overlap = uncertain
    for (size_t i = 0; i < a.count; i++) {
        for (size_t j = 0; j < b.count; j++) {
            if (overlap(a.ranges[i], b.ranges[j])) {
                return -1;  // NULL (uncertain)
            }
        }
    }

    // check exact equality
    for (size_t i = 0; i < a.count; i++) {
        if (a.ranges[i].lower != b.ranges[i].lower ||
            a.ranges[i].upper != b.ranges[i].upper) {
            return 0;  // Not equal
        }
    }
    
    return 1;  // Equal
}

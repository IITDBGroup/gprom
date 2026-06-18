#include "postgres.h"
#include <stdlib.h>
#include <stdio.h>
#include "arithmetic.h"

#define malloc palloc
#define free pfree

// range 1 + range 2
Int4Range range_add_internal(Int4Range a, Int4Range b){
    Int4Range rv;

    // check that both operands are NULL, or just 1
    if (a.isNull && b.isNull) {
        rv.isNull = true;
        rv.lower = 0;
        rv.upper = 0;    
        return rv;
    }
    else if (a.isNull) return b;
    else if (b.isNull) return a;    
    
    // postgres raises error on invalid range input. Not sure if this check is useful
    // if (!validRange(a) || !validRange(b)){
    //     return rv;
    // }

    rv.lower = a.lower+b.lower;
    rv.upper = a.upper+b.upper-1; // exclusive upper
    rv.isNull = false;
    return rv;
}

// range 1 - range 2
Int4Range range_subtract_internal(Int4Range a, Int4Range b){
    Int4Range rv;
    
    // if first param is null, reuslt is null
    if (a.isNull) {
        rv.isNull = true;
        rv.lower = 0;
        rv.upper = 0;    
        return rv;
    }
    else if (b.isNull) return a;    

    // account for exclusive upper with -1
    rv.lower = a.lower - (b.upper-1);
    rv.upper = ((a.upper-1) - b.lower) + 1; 
    rv.isNull = false;
    return rv;
}

// range 1 * range 2
Int4Range range_multiply_internal(Int4Range a, Int4Range b){
    Int4Range rv;
    int arr[4];
    
    // if either param is null, reuslt is null
    if (a.isNull || b.isNull) {
        rv.isNull = true;
        rv.lower = 0;
        rv.upper = 0;    
        return rv;
    }

    arr[0] = a.lower * b.lower;
    arr[1] = a.lower * (b.upper-1);
    arr[2] = (a.upper-1) * b.lower;
    arr[3] = (a.upper-1) * (b.upper-1);

    rv.lower = MIN(arr, 4);
    rv.upper = MAX(arr, 4)+1;
    rv.isNull = false;
    return rv;
}

// range 1 / range 2
// (divison with a bound crossing 0 should be 0 or ???)
Int4Range range_divide_internal(Int4Range a, Int4Range b){
    Int4Range rv;
    int arr[4];
    
    // if either param is null, reuslt is null
    if (a.isNull || b.isNull) {
        rv.isNull = true;
        rv.lower = 0;
        rv.upper = 0;    
        return rv;
    }

    arr[0] = a.lower / b.lower;
    arr[1] = a.lower / (b.upper-1);
    arr[2] = (a.upper-1) / b.lower;
    arr[3] = (a.upper-1) / (b.upper-1);

    rv.lower = MIN(arr, 4);
    rv.upper = MAX(arr, 4)+1;
    rv.isNull = false;
    return rv;
}


// Used for continuous boundaries where we want 1 range, rather a set of ranges
Int4Range floatIntervalSetMult(Int4RangeSet a, Multiplicity mult) {
    Int4RangeSet multSet;
    Int4Range multRange;
    Int4Range rv;
    int minLB;
    int maxUB;
    size_t j;
    
    // convert mult to I4R... make exlusive upper
    multSet.count = 1;
    multSet.ranges = malloc(sizeof(Int4Range));
    multRange.lower = mult.lower;
    multRange.upper = mult.upper+1;
    multSet.ranges[0] = multRange;
    
    minLB = mult.lower;
    maxUB = mult.upper+1;
    
    for (j=0; j < a.count; j++){
        int r1;
        int r2;
        int r3;
        int r4;
        int arr[6];
    
        r1 = multRange.lower * a.ranges[j].lower;
        r2 = multRange.lower * (a.ranges[j].upper-1); 
        r3 = (multRange.upper-1) * a.ranges[j].lower;
        r4 = (multRange.upper-1) * (a.ranges[j].upper-1);
        
        arr[0] = r1;
        arr[1] = r2;
        arr[2] = r3;
        arr[3] = r4;
        arr[4] = minLB;
        arr[5] = maxUB;
        
        minLB = MIN(arr, 6);
        maxUB = MAX(arr, 6);
    }
    
    rv.lower = minLB;
    rv.upper = maxUB + 1;

    free(multSet.ranges); 

    return rv;
}

Int4RangeSet range_set_add_internal(Int4RangeSet a, Int4RangeSet b){
    Int4RangeSet rv;
    size_t idx;
    size_t i;
    size_t j;

    // if (a.count == 0 && b.count == 0) return empty_set();
    // if (a.count == 0) return deep_copy(b);
    // if (b.count == 0) return deep_copy(a);

    // check NULL-only case. returns our null representation
    if (a.count == 1 && a.containsNull &&
        b.count == 1 && b.containsNull) {
        rv.count = 1;
        rv.containsNull = true;
        rv.ranges = palloc(sizeof(Int4Range));
        rv.ranges[0].isNull = true;
        rv.ranges[0].lower = 0;
        rv.ranges[0].upper = 0;
        return rv;
    }
    
    rv.count = (a.count*b.count);
    rv.containsNull = a.containsNull && b.containsNull ? true : false;  // NULL {+,-,/,*} NULL == NULL
    rv.ranges = malloc(sizeof(Int4Range) * (a.count * b.count));

    idx = 0;
    for (i=0; i<a.count; i++){
        for (j=0; j<b.count; j++){
            int currLow;
            int currHigh;

            // ignore first range if null
            if (a.ranges[i].isNull) {
                currLow = b.ranges[j].lower;
                currHigh = b.ranges[j].upper; //exclusive upper    
            }
            // ignore second range if null
            else if (b.ranges[j].isNull) {
                currLow = a.ranges[i].lower;
                currHigh = a.ranges[i].upper; //exclusive upper    
            }
            // neither ranges are null, normal calculation
            else {
                currLow = a.ranges[i].lower + b.ranges[j].lower;
                currHigh = (a.ranges[i].upper + b.ranges[j].upper)-1; //exclusive upper
            }

            // assign proper result values, and null flag
            rv.ranges[idx].lower = currLow;
            rv.ranges[idx].upper = currHigh;
            rv.ranges[idx].isNull = a.ranges[i].isNull && b.ranges[j].isNull ? true : false;
            idx++;
        }
    }
    return rv;
}

Int4RangeSet range_set_subtract_internal(Int4RangeSet a, Int4RangeSet b){
    Int4RangeSet rv;
    size_t idx;
    size_t i;
    size_t j;

    // if (a.count == 0) return empty_set();
    // if (b.count == 0) return deep_copy(a);

    // NULL guarentee if first parameter is {NULL}
    if (a.count == 1 && a.containsNull) {
        rv.count = 1;
        rv.containsNull = true;
        rv.ranges = palloc(sizeof(Int4Range));
        rv.ranges[0].isNull = true;
        rv.ranges[0].lower = 0;
        rv.ranges[0].upper = 0;
        return rv;
    }

    rv.count = (a.count*b.count);
    rv.containsNull = a.containsNull && b.containsNull ? true : false;  // NULL {+,-,/,*} NULL == NULL
    rv.ranges = malloc(sizeof(Int4Range) * (a.count * b.count));

    idx = 0;
    for (i=0; i<a.count; i++){
        for (j=0; j<b.count; j++){
            int currLow;
            int currHigh;

            // ignore first range if null. Arbitrary val assignment
            if (a.ranges[i].isNull) {
                currLow = 0;
                currHigh = 0; //exclusive upper    
            }
            // ignore second range if null
            else if (b.ranges[j].isNull) {
                currLow = a.ranges[i].lower;
                currHigh = a.ranges[i].upper; //exclusive upper    
            }
            // neither ranges are null, normal calculation
            else {
                currLow = a.ranges[i].lower - (b.ranges[j].upper-1);
                currHigh = ((a.ranges[i].upper-1) - b.ranges[j].lower); //exclusive upper
            }

            // assign proper result values, and null flag
            rv.ranges[idx].lower = currLow;
            rv.ranges[idx].upper = currHigh;
            rv.ranges[idx].isNull = a.ranges[i].isNull ? true : false;
            idx++;
        }
    }
    return rv;
}

Int4RangeSet range_set_multiply_internal(Int4RangeSet a, Int4RangeSet b){
    Int4RangeSet rv;
    size_t idx;
    size_t i;
    size_t j;
    
    // if (a.count == 0 || b.count == 0) return empty_set();

    // check NULL-only case. returns our null representation
    if ((a.count == 1 && a.containsNull) ||
        (b.count == 1 && b.containsNull)) {
        rv.count = 1;
        rv.containsNull = true;
        rv.ranges = palloc(sizeof(Int4Range));
        rv.ranges[0].isNull = true;
        rv.ranges[0].lower = 0;
        rv.ranges[0].upper = 0;
        return rv;
    }
    
    rv.count = (a.count*b.count);
    rv.containsNull = a.containsNull || b.containsNull ? true : false;  // NULL {+,-,/,*} NULL == NULL
    rv.ranges = malloc(sizeof(Int4Range) * (a.count * b.count));
    
    idx = 0;
    for (i=0; i<a.count; i++){
        for (j=0; j<b.count; j++){
            int arr[4];
            int currLow;
            int currHigh;

            // either null -> result null
            if (a.ranges[i].isNull || b.ranges[j].isNull) {
                currLow = 0;
                currHigh = 0;
            }
            else {
                arr[1] = a.ranges[i].lower * (b.ranges[j].upper-1);
                arr[0] = a.ranges[i].lower * b.ranges[j].lower;
                arr[2] = (a.ranges[i].upper-1) * b.ranges[j].lower;
                arr[3] = (a.ranges[i].upper-1) * (b.ranges[j].upper-1);
                
                currLow = MIN(arr, 4);
                currHigh = MAX(arr, 4)+1;   //exclusive UB
            }

            // assign proper result values, and null flag
            rv.ranges[idx].lower = currLow;
            rv.ranges[idx].upper = currHigh;
            rv.ranges[idx].isNull = a.ranges[i].isNull || b.ranges[j].isNull ? true : false;
            idx++;
        }
    }
    return rv;
}

Int4RangeSet range_set_divide_internal(Int4RangeSet a, Int4RangeSet b){
    Int4RangeSet rv;
    size_t idx;
    size_t i;
    size_t j;
    
    // if (a.count == 0 || b.count == 0) return empty_set();
    
    // check NULL-only case. returns our null representation
    if ((a.count == 1 && a.containsNull) ||
        (b.count == 1 && b.containsNull)) {
        rv.count = 1;
        rv.containsNull = true;
        rv.ranges = palloc(sizeof(Int4Range));
        rv.ranges[0].isNull = true;
        rv.ranges[0].lower = 0;
        rv.ranges[0].upper = 0;
        return rv;
    }

    rv.count = (a.count*b.count);
    rv.containsNull = a.containsNull || b.containsNull ? true : false;  // NULL {+,-,/,*} NULL == NULL
    rv.ranges = malloc(sizeof(Int4Range) * (a.count * b.count));
    
    idx=0;
    for (i=0; i<a.count; i++){
        for (j=0; j<b.count; j++){
            int arr[4];
            int currLow;
            int currHigh;

            // either null -> result null
            if (a.ranges[i].isNull || b.ranges[j].isNull) {
                currLow = 0;
                currHigh = 0;
            }
            else {
                arr[0] = (a.ranges[i].lower) / (b.ranges[j].lower);
                arr[1] = (a.ranges[i].lower) / (b.ranges[j].upper-1);
                arr[2] = (a.ranges[i].upper-1) / (b.ranges[j].lower);
                arr[3] = (a.ranges[i].upper-1) / (b.ranges[j].upper-1);
                
                currLow = MIN(arr, 4);
                currHigh = MAX(arr, 4)+1;   //exclusive UB
            }
            
            // assign proper result values, and null flag
            rv.ranges[idx].lower = currLow;
            rv.ranges[idx].upper = currHigh;
            rv.ranges[idx].isNull = a.ranges[i].isNull || b.ranges[j].isNull ? true : false;
            idx++;
        }
    }
    return rv;
}
// #include <string.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <stdbool.h>
// #include "helperFunctions.h" 
// #include "limits.h"

// int MIN(int My_array[], int len) {
//   int num = My_array[0];
//   for (int i = 1; i < len; i++) {
//     if (My_array[i] < num) {
//       num = My_array[i];
//     }
//   }
//   return num;
// }
// int MAX(int My_array[], int len) {
//   int num = My_array[0];
//   for (int i = 1; i < len; i++) {
//     if (My_array[i] > num) {
//       num = My_array[i];
//     }
//   }
//   return num;
// }

// // Checks if 2 ranges overlap at all
// bool overlap(Int4Range a, Int4Range b){
//   return a.lower < b.upper && b.lower < a.upper;
// }

// typedef struct{ 
//     int lower; // inclusive
//     int upper; // exclusive
//     bool isNull;
// } Int4Range;


// void printRange(Int4Range a){
//   if (a.isNull){
//     printf("NULL\n");
//   }
//   else{
//     printf("[");
//     printf("%d, %d", a.lower, a.upper);
//     printf(")\n");
//   }
// }
// void printRangeSet(Int4RangeSet a){
//   printf("{");
  
//   for (size_t i=0; i<a.count; i++){
//     if (a.ranges[i].isNull) {
//       printf(" NULL ");
//     }
//     else {
//       printf(" [%d, %d)", a.ranges[i].lower, a.ranges[i].upper);
//     }
//   }
//   printf("}\n");
// }

// Int4RangeSet filterOutNulls(Int4RangeSet vals) {
//   if (!vals.containsNull) {
//     return vals;
//   }
  
//   Int4RangeSet filteredVals;
//   size_t nonNullCount;
//   size_t i;
  
//   nonNullCount = 0;
//   for (i = 0; i < vals.count; i++) {
//     if (!vals.ranges[i].isNull) nonNullCount++;
//   }

//   filteredVals.count = nonNullCount;
//   filteredVals.ranges = malloc(sizeof(Int4Range) * filteredVals.count);
//   filteredVals.containsNull = false;
//   // filteredVals.containsNull = vals.containsNull;

//   int idx;
//   idx = 0;
//   for (i = 0; i < vals.count; i++) {
//     if (!vals.ranges[i].isNull) {
//       filteredVals.ranges[idx].isNull = false;
//       filteredVals.ranges[idx++] = vals.ranges[i];
//     }
//   }

//   return filteredVals;
// }


// // https://www.geeksforgeeks.org/c/comparator-function-of-qsort-in-c/#
// static int q_sort_compare_ranges(const void* range1, const void* range2){
//   Int4Range r1;
//   Int4Range r2;
//   r1 = *(Int4Range*)range1;
//   r2 = *(Int4Range*)range2;

//   if(r1.lower != r2.lower){
//     return r1.lower < r2.lower ? -1 : 1;
//   }

//   return r1.upper < r2.upper ? -1 : 1;
// }

// // Traverses through entire set and looks to merge any possible overlap.
// // Allocates space for new array 
// // Confusion: should it be strict overlap vs adjacancy: {[1,3) (3, 6]} => {(1,6]} ???
// Int4RangeSet normalize(Int4RangeSet vals){
//   Int4RangeSet normalized;
//   Int4RangeSet sorted;
//   Int4Range prev;
//   size_t i;

//   if (vals.count == 0){
//     normalized.count = 0;
//     normalized.ranges = NULL;
//     normalized.containsNull = false;
//     return normalized;
//   }
  
//   sorted = sort(vals);
  
//   bool hadNull = sorted.containsNull;
//   // remove null is present
//   if (hadNull) {
//     sorted = filterOutNulls(sorted);
//   }

//   normalized.count = 0;
//   normalized.ranges = malloc(sizeof(Int4Range) * sorted.count);
//   normalized.containsNull = false;
  
//   prev = sorted.ranges[0];
//   prev.isNull = false;

//   for(i=1; i<sorted.count; i++){
//     Int4Range curr;
//     curr = sorted.ranges[i];
    
//     if (overlap(prev, curr)){
//       prev.lower = (curr.lower < prev.lower) ? curr.lower : prev.lower;
//       prev.upper = (curr.upper > prev.upper) ? curr.upper : prev.upper;
//     }
//     // no overlap, so add entire range
//     else{
//       prev.isNull = false;
//       normalized.ranges[normalized.count++] = prev;
//       prev = curr;
//     }
//   }
  
//   // account for last range
//   normalized.ranges[normalized.count++] = prev;
  
//   // account for null 
//   if (hadNull) {
//     normalized.ranges = realloc(normalized.ranges, sizeof(Int4Range) * (normalized.count + 1));
//     normalized.ranges[normalized.count].isNull = true;
//     normalized.ranges[normalized.count].lower = 0;
//     normalized.ranges[normalized.count].upper = 0;
//     normalized.containsNull = true;
//     normalized.count++;
//   }

//   free(sorted.ranges);
//   return normalized;
// }

// // Allocates space for new array that is sorted on 1)lower, 2)upper using quicksort
// // prefilers result removing all potential NULLs. Then sorts.
// // returns sorted array with NULL appended if necessary.
// Int4RangeSet sort(Int4RangeSet vals){
//   Int4RangeSet sorted;
  
//   if (vals.count == 0){
//     sorted.count = 0;
//     sorted.ranges = NULL;
//     sorted.containsNull = false;
//     return sorted;
//   }

//   // filter out nulls
//   size_t nonNullCount;
//   size_t i;
  
//   nonNullCount = 0;
//   for (i = 0; i < vals.count; i++) {
//     if (!vals.ranges[i].isNull) nonNullCount++;
//   }

//   sorted.count = nonNullCount + (vals.containsNull ? 1 : 0);
//   sorted.ranges = malloc(sizeof(Int4Range) * sorted.count);
//   sorted.containsNull = vals.containsNull;

//   size_t idx;
//   idx = 0;
//   for (i = 0; i < vals.count; i++) {
//     if (!vals.ranges[i].isNull) {
//       sorted.ranges[idx++] = vals.ranges[i];
//     }
//   }

//   // sort non null ranges
//   if (nonNullCount > 1) {
//     qsort(sorted.ranges, nonNullCount, sizeof(Int4Range), q_sort_compare_ranges);
//   }

//   if (vals.containsNull) {
//     sorted.ranges[sorted.count-1].isNull = true;
//     sorted.ranges[sorted.count-1].lower = 0;
//     sorted.ranges[sorted.count-1].upper = 0;
//   }

//   return sorted;
// }

// Int4RangeSet range_set_multiply_internal(Int4RangeSet a, Int4RangeSet b){
//     Int4RangeSet rv;
//     size_t idx;
//     size_t i;
//     size_t j;
    
//     // check NULL-only case. returns our null representation
//     if ((a.count == 1 && a.containsNull) ||
//         (b.count == 1 && b.containsNull)) {
//         rv.count = 1;
//         rv.containsNull = true;
//         rv.ranges = malloc(sizeof(Int4Range));
//         rv.ranges[0].isNull = true;
//         rv.ranges[0].lower = 0;
//         rv.ranges[0].upper = 0;
//         return rv;
//     }
    
//     rv.count = (a.count*b.count);
//     rv.containsNull = a.containsNull || b.containsNull ? true : false;  // NULL {+,-,/,*} NULL == NULL
//     rv.ranges = malloc(sizeof(Int4Range) * (a.count * b.count));
    
//     idx = 0;
//     for (i=0; i<a.count; i++){
//         for (j=0; j<b.count; j++){
//             int arr[4];
//             int currLow;
//             int currHigh;

//             // either null -> result null
//             if (a.ranges[i].isNull || b.ranges[j].isNull) {
//                 currLow = 0;
//                 currHigh = 0;
//             }
//             else {
//                 arr[1] = a.ranges[i].lower * (b.ranges[j].upper-1);
//                 arr[0] = a.ranges[i].lower * b.ranges[j].lower;
//                 arr[2] = (a.ranges[i].upper-1) * b.ranges[j].lower;
//                 arr[3] = (a.ranges[i].upper-1) * (b.ranges[j].upper-1);
                
//                 currLow = MIN(arr, 4);
//                 currHigh = MAX(arr, 4)+1;   //exclusive UB
//             }

//             // assign proper result values, and null flag
//             rv.ranges[idx].lower = currLow;
//             rv.ranges[idx].upper = currHigh;
//             rv.ranges[idx].isNull = a.ranges[i].isNull || b.ranges[j].isNull ? true : false;
//             idx++;
//         }
//     }
//     return rv;
// }

// Int4RangeSet
// internal_agg_sum_combine_set_mult(Int4RangeSet set1, Int4Range mult) {
//     Int4RangeSet result;
//     bool leftNull, rightNull;
//     int total_result_ranges;

//     total_result_ranges = set1.count * (mult.upper - mult.lower);
    
//     result.count = 0;
//     result.containsNull = false;
//     result.ranges = malloc(sizeof(Int4Range) * total_result_ranges);

//     // multSet.containsNull = (mult.lower == 0);
    
//     // check if either side produces null. Append NULL to result later
//     leftNull = set1.containsNull;
//     rightNull = mult.lower == 0;
    
//     int i, idx;
//     idx = 0;
//     // traverse thru every set/mult combination and union result
//     for (i = mult.lower; i < mult.upper; i++) {
//         // ignore mult == 0 bc it produced NULL flag
//         if (i == 0) {
//             continue;
//         }

//         Int4RangeSet multSet;

//         multSet.containsNull = false;
//         multSet.count = 1;
//         multSet.ranges = malloc(sizeof(Int4Range));
//         multSet.ranges[0].lower = i;
//         multSet.ranges[0].upper = i+1;      // account for exclusive UB representation 
//         multSet.ranges[0].isNull = false;

//         Int4RangeSet tempResult;
//         tempResult = range_set_multiply_internal(set1, multSet);
//         free(multSet.ranges);

//         // union in new results
//         int j;
//         for (j = 0; j < tempResult.count; j++) {
//             result.ranges[idx] = tempResult.ranges[j];
//             idx++;
//         }

//         free(tempResult.ranges);
//         // have allocated space with count pointer incrementing with each union
//     }
//     result.count = idx;

//     if (result.count == 0){
//       free(result.ranges);
//       result.ranges = NULL;
//       return result;
//     }

//     Int4RangeSet normOutput;
//     normOutput = normalize(result);

//     free(result.ranges);
    
//     return normOutput;
// }

// Int4RangeSet min_rangeSet(Int4RangeSet a, Int4RangeSet b){
//   // empty sets
//   // if (a.count > 0 && a.ranges[0].isNull) return normalize(b);
//   // if (b.count > 0 && b.ranges[0].isNull) return normalize(a);
//   if (a.count == 0) return b;
//   if (b.count == 0) return a;
  
//   Int4RangeSet rv, result;
//   int aptr, bptr;
  
//   rv.ranges = malloc(sizeof(Int4Range) * (a.count + b.count));
//   rv.containsNull = false;
//   rv.count = 0;
  
//   aptr = 0;
//   bptr = 0;
//   while (aptr < a.count && bptr < b.count) {
//     Int4Range newRange;
//     newRange.isNull = false;

//     newRange.lower = min2(a.ranges[aptr].lower, b.ranges[bptr].lower);
//     newRange.upper = min2(a.ranges[aptr].upper, b.ranges[bptr].upper);

//     // move pts based on UB
//     a.ranges[aptr].upper <= b.ranges[bptr].upper ? aptr++ : bptr++;
//     rv.ranges[rv.count++] = newRange;
//   }
  
//   result = normalize(rv);
//   return result;
// }

// /*Return Int4RangeSet of the non reduced result of taking every range in a and b
//   and finding the min result range:= {min(aL, bL), min(aU, bU) a x b}
  
//   Room for optimization avoiding non overlapping comparisons
// */
// Int4RangeSet max_rangeSet(Int4RangeSet a, Int4RangeSet b){
//   // empty sets
//    if (a.count > 0 && a.ranges[0].lower == INT_MIN && a.ranges[0].upper == INT_MIN + 2) {
//         return b;  // a is sentinel, return b
//     }
//     if (b.count > 0 && b.ranges[0].lower == INT_MIN && b.ranges[0].upper == INT_MIN + 2) {
//         return a;  // b is sentinel, return a
//     }
  
//   Int4RangeSet rv, result;
//   int aptr, bptr;
  
//   rv.ranges = malloc(sizeof(Int4Range) * (a.count + b.count));
//   rv.containsNull = false;
//   rv.count = 0;
  
//   aptr = 0;
//   bptr = 0;
//   while (aptr < a.count && bptr < b.count) {
//     Int4Range newRange;
//     newRange.isNull = false;

//     newRange.lower = max2(a.ranges[aptr].lower, b.ranges[bptr].lower);
//     newRange.upper = max2(a.ranges[aptr].upper, b.ranges[bptr].upper);

//     // move pts based on UB
//     a.ranges[aptr].upper <= b.ranges[bptr].upper ? aptr++ : bptr++;
//     rv.ranges[rv.count++] = newRange;
//   }
  
//   result = normalize(rv);
//   return result;
// }

// // will need to change the type of neutral element depending on what datatype the user is using
// // try containsNull = true to resolve crashing pg
// Int4RangeSet
// set_mult_combine_helper(Int4RangeSet set1, Int4Range mult, int neutralElement)
// {
//     // return neutral so doesn't affect the aggregate
//     if(mult.lower == 0) {
//         Int4RangeSet result;
//         result.count = 1;
//         result.containsNull = false;
//         result.ranges = malloc(sizeof(Int4Range));
        
//         // have to adjust UB + 2 or LB -2 based on if pos or neg
//         if (neutralElement <= 0) {
//             result.ranges[0].lower = neutralElement;      //temp change to resolve crashing   
//             result.ranges[0].upper = neutralElement + 2;
//         }
//         else {
//             result.ranges[0].lower = neutralElement-2;      //temp change to resolve crashing   
//             result.ranges[0].upper = neutralElement;
//         }
        
//         result.ranges[0].isNull = false;
//         return result;
//     }

//     return set1;
// }

// #define min2(x, y) (((x) <= (y)) ? (x) : (y))
// #define max2(x, y) (((x) >= (y)) ? (x) : (y))

// int range_less_than(Int4Range a, Int4Range b){
//     if (a.lower > (b.upper-1)) {
//         return 0;
//     }
//     else if ((a.upper-1) < b.lower) {
//         return 1;
//     }
//     return -1;
// }

// Int4Range prune_lt_internal(Int4Range a, Int4Range b, char direction) {
//     Int4Range result;
//     result.isNull = false;
    
//     // 3 cases
//     // A is stricly less than B, 
//     // A is stricly greater than B,
//     // They overlap
//     if (direction == 'l') {
//         // strictly less than. Just return left side == A
//         if (range_less_than(a, b) == 1) {
//             return a;
//         }
//         // A is not LT B. result is NULL
//         else if (range_less_than(a,b) == 0) {
//             result.isNull = true;
//             return result;
//         }
//         // otherwise return A pruned on UB by min
//         result.lower = a.lower;
//         // result.upper = b.lower;
//         result.upper = min2(a.upper, b.lower);
//         printf("r.lower=%d r.upper=%d\n", result.lower, result.upper);
//     } 
//     else if (direction == 'r') {
//         // strictly less than. Just return right side == B
//         if (range_less_than(a, b) == 1) {
//             return b;
//         }
//         // A is not LT B. result is NULL
//         else if (range_less_than(a,b) == 0) {
//             result.isNull = true;
//             return result;
//         }
//         // otherwise return B pruned on LB by A
//         // result.lower = a.upper;
//         result.lower = max2(a.upper, b.lower);
//         result.upper = b.upper;

//         // if(valid)
//         printf("r.lower=%d r.upper=%d\n", result.lower, result.upper);
//     }

//     return result;
// }

// // #define PRIMARY_DATA_TYPE "int4range"

// int main(){  
//   Int4Range a = {1,12};
//   Int4Range b = {8,14};
//   Int4Range c = {2,25};
//   Int4Range d = {18,100};
// //   Int4Range e = {16,20};
// //   Int4Range f = {6,11};

//     char dir = 'r';
//     Int4Range r1 = prune_lt_internal(a, b, dir);
//     Int4Range r2 = prune_lt_internal(c, d, dir);
    
//     printRange(r1);
//     printRange(r2);

//   Int4Range n;
//   n.isNull = true;
//   Int4Range mult1 = {1,4};
//   Int4Range mult2 = {4,8};
//   Int4Range mult3 = {0,2};

//   Int4Range a_ranges[] = {f, n, a, c};
//   Int4Range b_ranges[] = {a};
//   Int4RangeSet s1 = {a_ranges, 4, true};
//   Int4RangeSet s2 = {b_ranges, 1, false};

//   // Int4RangeSet rv1 = range_set_add(s1, s2); 
//   // printRangeSet(rv1);

//   // printRangeSet(s1);
//   // Int4RangeSet rv2 = sort(s1);
//   // printRangeSet(rv2);


//   // Int4RangeSet rv3 = internal_agg_sum_combine_set_mult(s2, a);
//   // printRangeSet(rv3);

//   Int4Range c_ranges[] = {a,b,c};
//   Int4Range d_ranges[] = {d,e};
//   Int4RangeSet sC = {c_ranges, 3, false};
//   Int4RangeSet sD = {d_ranges, 2, false};
//   Int4RangeSet sCmin = {c_ranges, 3, false};
//   Int4RangeSet sDmin = {d_ranges, 2, false};
  
//   Int4RangeSet rsC = set_mult_combine_helper(sC, mult1, INT_MIN);
//   Int4RangeSet rsD = set_mult_combine_helper(sD, mult2, INT_MIN);
//   printRangeSet((sC));
//   printRangeSet((sD));
//   printf("\n\n");
//   printRangeSet(max_rangeSet(normalize(rsC), normalize(rsD)));
//   printf("\n\n");

//   // Int4RangeSet rsCmin = set_mult_combine_helper(sCmin, mult1, INT_MAX);
//   // Int4RangeSet rsDmin = set_mult_combine_helper(sDmin, mult3, INT_MAX);
//   // printRangeSet((rsCmin));
//   // printRangeSet((rsDmin));
//   // printf("\n\n");
//   // printRangeSet(min_rangeSet(normalize(rsCmin), normalize(rsDmin)));
  
  
  
//   // printRangeSet(min_rangeSet(normalize(sC), normalize(sD)));
//   // printRangeSet(max_rangeSet(normalize(sC), normalize(sD)));
//   // Int4Range rv1 = floatIntervalSetMult(s1, mult1);  
//   // printRange(rv1);
//   // Int4Range rv2 = floatIntervalSetMult(s1, mult2);  
//   // printRange(rv2);
//   // Int4Range rv3 = floatIntervalSetMult(s1, mult3);  
//   // printRange(rv3);


//   // Int4Range c_ranges[] = {f, a, e, c};
//   // Int4RangeSet s3 = {c_ranges, 4};
//   // Int4RangeSet tn_rv = testNormalize(s3);
//   // printRangeSet(tn_rv);


//   return 0;
// }


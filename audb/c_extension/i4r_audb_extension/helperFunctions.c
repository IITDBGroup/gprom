#include "postgres.h"
#include "helperFunctions.h"
#include "arithmetic.h" //
#include <string.h>

#define malloc palloc
#define free pfree

// #define min2(a, b) (((a) < (b)) ? (a) : (b))
// #define max2(a, b) (((a) > (b)) ? (a) : (b))

// empty_set is length 0 with no flags set, and no range(NULL). must be handeled by caller
Int4RangeSet empty_set() {
  Int4RangeSet r;
  r.count = 0;
  r.containsNull = false;
  r.ranges = NULL;
  return r;
}

// null_set is length 1 with only null flag set, and 1 range (0,0). must be handled by caller
Int4RangeSet null_set() {
  Int4RangeSet r;
  r.count = 1;
  r.containsNull = true;
  r.ranges = palloc(sizeof(Int4Range));
  r.ranges[0].isNull = true;
  r.ranges[0].lower = 0;
  r.ranges[0].upper = 0;
  return r;
}

// https://www.delftstack.com/howto/c/c-max-and-min-function/
// linearly scan to find the MIN value in array
int MIN(int My_array[], int len) {
  int num;
  int i;
  
  num = My_array[0];
  for (i = 1; i < len; i++) {
    if (My_array[i] < num) {
      num = My_array[i];
    }
  }
  return num;
}

// linearly scan to find the MAX value in array
int MAX(int My_array[], int len) {
  int num;
  int i;

  num = My_array[0];
  for (i = 1; i < len; i++) {
    if (My_array[i] > num) {
      num = My_array[i];
    }
  }
  return num;
}

// Print I4R type
void printRange(Int4Range a){
  if (a.isNull){
    printf("NULL\n");
  }
  else{
    printf("[");
    printf("%d, %d", a.lower, a.upper);
    printf(")\n");
  }
}

// Print I4RSet type
void printRangeSet(Int4RangeSet a){
  printf("{");
  for (size_t i=0; i<a.count; i++){
    if (a.ranges[i].isNull) {
      printf(" NULL ");
    }
    else {
      printf(" [%d, %d)", a.ranges[i].lower, a.ranges[i].upper);
    }
  }
  printf("}\n");
}

// Print I4RSet type
void printRangeSetElog(Int4RangeSet a){
  elog(INFO, "{");
  for (size_t i=0; i<a.count; i++){
    if (a.ranges[i].isNull) {
      elog(INFO, " NULL ");
    }
    else {
      elog(INFO, " [%d, %d)", a.ranges[i].lower, a.ranges[i].upper);
    }
  }
  elog(INFO, "}\n");
}

Int4RangeSet deep_copy(Int4RangeSet src) {
    Int4RangeSet r;
    r.count = src.count;
    r.containsNull = src.containsNull;
    if (src.count == 0 || src.ranges == NULL) {
        r.ranges = NULL;
        return r;
    }
    r.ranges = palloc(sizeof(Int4Range) * src.count);
    memcpy(r.ranges, src.ranges, sizeof(Int4Range) * src.count);
    return r;
}

// Checks if a is increasing: a.lower <= a.upper
bool validRange(Int4Range a){
  if (a.isNull) return true;

  return a.lower <= a.upper;
}

// Checks if a is stricly increasing: a.lower < a.upper
bool validRangeStrict(Int4Range a){
  if (a.isNull) return true;

  return a.lower < a.upper;
}

// lift a scalar int into a Int4Range
Int4Range lift_scalar_local(int x){
  Int4Range rv;
  rv.lower = x;
  rv.upper = x+1;
  rv.isNull = false;
  return rv;
}

// lift a single Int4Range into a Int4RangeSet
Int4RangeSet lift_range_local(Int4Range a){
  Int4RangeSet rv;
  rv.count = 1;
  rv.containsNull = a.isNull;
  rv.ranges = (Int4Range *) palloc(sizeof(Int4Range));
  rv.ranges[0] = a;

  return rv;
}

Int4Range min_range(Int4Range range1, Int4Range range2) {
  Int4Range result;
  result.isNull = false;
  result.lower = min2(range1.lower, range2.lower);
  result.upper = min2(range1.upper, range2.upper);
  
  return result;
}

Int4Range max_range(Int4Range range1, Int4Range range2) {
  Int4Range result;
  result.isNull = false;
  result.lower = max2(range1.lower, range2.lower);
  result.upper = max2(range1.upper, range2.upper);
  
  return result;
}

/*Return Int4RangeSet of the non reduced result of taking every range in a and b
  and finding the min result range:= {min(aL, bL), min(aU, bU) a x b}
  
  Room for optimization avoiding non overlapping comparisons
*/
Int4RangeSet min_rangeSet(Int4RangeSet a, Int4RangeSet b){
  Int4RangeSet rv;
  Int4RangeSet result;
  int aptr;
  int bptr;
  
  // empty sets
  // if (a.count > 0 && a.ranges[0].isNull) return normalize(b);
  // if (b.count > 0 && b.ranges[0].isNull) return normalize(a);
  if (a.count == 0) return b;
  if (b.count == 0) return a;
  
  
  rv.ranges = palloc(sizeof(Int4Range) * (a.count + b.count));
  rv.containsNull = false;
  rv.count = 0;
  
  // // baseline- cross product
  // for (i = 0; i < a.count; i++) {
  //   for (j = 0; j < b.count; j++) {
  //     Int4Range newRange;
  //     newRange.isNull = false;
  //     newRange.lower  = min2(a.ranges[i].lower, b.ranges[j].lower);
  //     newRange.upper  = min2(a.ranges[i].upper, b.ranges[j].upper);
  //     rv.ranges[rv.count++] = newRange;
  //   }
  // }

  // optimized merge join-type algorithm
  aptr = 0;
  bptr = 0;
  while (aptr < a.count && bptr < b.count) {
    Int4Range newRange;
    newRange.isNull = false;

    newRange.lower = min2(a.ranges[aptr].lower, b.ranges[bptr].lower);
    newRange.upper = min2(a.ranges[aptr].upper, b.ranges[bptr].upper);

    // move pts based on UB
    a.ranges[aptr].upper <= b.ranges[bptr].upper ? aptr++ : bptr++;
    rv.ranges[rv.count++] = newRange;
  }
  
  result = normalize(rv);
  return result;
}

/*Return Int4RangeSet of the non reduced result of taking every range in a and b
  and finding the min result range:= {min(aL, bL), min(aU, bU) a x b}
  
  Room for optimization avoiding non overlapping comparisons
*/
Int4RangeSet max_rangeSet(Int4RangeSet a, Int4RangeSet b){
  Int4RangeSet rv;
  Int4RangeSet result;
  int aptr;
  int bptr;
  
  // empty sets
  // if (a.count > 0 && a.ranges[0].isNull) return normalize(b);
  // if (b.count > 0 && b.ranges[0].isNull) return normalize(a);
  if (a.count == 0) return b;
  if (b.count == 0) return a;
  
  rv.ranges = palloc(sizeof(Int4Range) * (a.count + b.count));
  rv.containsNull = false;
  rv.count = 0;
  
  // // baseline- cross product
  // for (i = 0; i < a.count; i++) {
  //   for (j = 0; j < b.count; j++) {
  //     Int4Range newRange;
  //     newRange.isNull = false;
  //     newRange.lower  = max2(a.ranges[i].lower, b.ranges[j].lower);
  //     newRange.upper  = max2(a.ranges[i].upper, b.ranges[j].upper);
  //     rv.ranges[rv.count++] = newRange;
  //   }
  // }

  aptr = 0;
  bptr = 0;
  while (aptr < a.count && bptr < b.count) {
    Int4Range newRange;
    newRange.isNull = false;

    newRange.lower = max2(a.ranges[aptr].lower, b.ranges[bptr].lower);
    newRange.upper = max2(a.ranges[aptr].upper, b.ranges[bptr].upper);

    // move pts based on UB
    a.ranges[aptr].upper <= b.ranges[bptr].upper ? aptr++ : bptr++;
    rv.ranges[rv.count++] = newRange;
  }
  
  result = normalize(rv);
  return result;
}
 
// https://www.geeksforgeeks.org/c/comparator-function-of-qsort-in-c/#
static int q_sort_compare_ranges(const void* range1, const void* range2){
  Int4Range r1;
  Int4Range r2;
  r1 = *(Int4Range*)range1;
  r2 = *(Int4Range*)range2;

  if(r1.lower != r2.lower){
    return r1.lower < r2.lower ? -1 : 1;
  }

  return r1.upper < r2.upper ? -1 : 1;
}

// Allocates space for new array that is sorted on [1)lower, 2)upper] using quicksort
// prefilers result removing all potential NULLs. Then sorts.
// returns sorted array with NULL appended if necessary.
Int4RangeSet sort(Int4RangeSet vals){
  Int4RangeSet sorted;
  size_t nonNullCount;
  size_t i;
  size_t idx;

  if (vals.count == 0){
    sorted.count = 0;
    sorted.ranges = NULL;
    sorted.containsNull = false;
    return sorted;
  }

  // filter out nulls
  nonNullCount = 0;
  for (i = 0; i < vals.count; i++) {
    if (!vals.ranges[i].isNull) nonNullCount++;
  }

  sorted.count = nonNullCount + (vals.containsNull ? 1 : 0);
  sorted.ranges = palloc(sizeof(Int4Range) * sorted.count);
  sorted.containsNull = vals.containsNull;

  idx = 0;
  for (i = 0; i < vals.count; i++) {
    if (!vals.ranges[i].isNull) {
      sorted.ranges[idx++] = vals.ranges[i];
    }
  }

  // sort non null ranges
  if (nonNullCount > 1) {
    qsort(sorted.ranges, nonNullCount, sizeof(Int4Range), q_sort_compare_ranges);
  }

  if (vals.containsNull) {
    sorted.ranges[sorted.count-1].isNull = true;
    sorted.ranges[sorted.count-1].lower = 0;
    sorted.ranges[sorted.count-1].upper = 0;
  }

  return sorted;
}

// Traverses through entire set and looks to greedily merge any possible overlap.
// returns newly allocated set. At the least, result is sorted, at the most its reduced and merged into a 1 range set
Int4RangeSet normalize(Int4RangeSet vals){
  Int4RangeSet normalized;
  Int4RangeSet sorted;
  Int4Range prev;
  size_t i;
  bool hadNull;

  // 4/1/26, do not recall difference between count=0 vs isNULL. Empty vs NULL?
  if (vals.count == 0){
    normalized.count = 0;
    normalized.ranges = NULL;
    normalized.containsNull = false;
    return normalized;
  }
  
  // quicksort
  sorted = sort(vals);

  hadNull = sorted.containsNull;
  if (hadNull) {
    sorted = filterOutNulls(sorted);
  }

  // otherwise palloc largest size
  normalized.count = 0;
  normalized.ranges = palloc(sizeof(Int4Range) * sorted.count);
  normalized.containsNull = false;
  
  prev = sorted.ranges[0];
  prev.isNull = false;

  // traverse once and merge any overlap, and fully append any non overlap
  for(i=1; i<sorted.count; i++){
    Int4Range curr;
    curr = sorted.ranges[i];
    
    if (overlap(prev, curr)){
      prev.lower = (curr.lower < prev.lower) ? curr.lower : prev.lower;
      prev.upper = (curr.upper > prev.upper) ? curr.upper : prev.upper;
    }
    // no overlap, so add entire range
    else{
      prev.isNull = false;
      normalized.ranges[normalized.count++] = prev;
      prev = curr;
    }
  }
  // account for last range and append
  normalized.ranges[normalized.count++] = prev;
  
  // account for null 
  if (hadNull) {
    normalized.ranges = realloc(normalized.ranges, sizeof(Int4Range) * (normalized.count + 1));
    normalized.ranges[normalized.count].isNull = true;
    normalized.ranges[normalized.count].lower = 0;
    normalized.ranges[normalized.count].upper = 0;
    normalized.containsNull = true;
    normalized.count++;
  }

  pfree(sorted.ranges);
  return normalized;
}

// Checks if 2 ranges overlap at all
bool overlap(Int4Range a, Int4Range b){
  return a.lower < b.upper && b.lower < a.upper;
}

// Checks if a fully contains b
bool contains(Int4Range a, Int4Range b){
  return (a.lower <= b.lower && b.lower <= a.upper 
      && a.lower <= b.upper && b.upper <= a.upper);
}

// confusion example: a(1,5) b(2,9)
// Find the distance between 2 ranges
int range_distance(Int4Range a, Int4Range b){
  if ((a.upper-1) <= b.lower){
    return b.lower - (a.upper-1);
  }
  else if ((b.upper-1) <= a.lower){
    return a.lower - (b.upper-1);
  }
  // overlap ranges
  else{
    return 0;
  }
}

// reduce size and return newly allocated RangeSet
/*
  sort the input and greedily merge. Traverses through entire set while we have more ranges than desires.
  O(N^2) worst case if numRangesKeep = N, we sort (NlogN), N times
*/
Int4RangeSet reduceSize(Int4RangeSet vals, int numRangesKeep){
  Int4RangeSet normalized;
  Int4RangeSet sortedInput;
  int currNumRanges;

  // nothing to reduce
  if (vals.count <= numRangesKeep){
    return vals;
  }
  else if (vals.count == 0){
    normalized.count = 0;
    normalized.containsNull = false;
    normalized.ranges = NULL;
  }
  else if (vals.containsNull && vals.count == 1){
    normalized.count = 1;
    normalized.containsNull = true;
    normalized.ranges = palloc(sizeof(Int4Range));
    normalized.ranges[0].isNull = true;
    normalized.ranges[0].lower = 0;
    normalized.ranges[0].upper = 0;
  }
  
  // sortedInput = sort(vals);
  sortedInput = normalize(vals);
  // check size condition after normalize potential collapsing
  if (sortedInput.count <= numRangesKeep) {
    return sortedInput;
  }
  
  // ignore the NULL range at sortedInput.ranges[len-1]
  currNumRanges = sortedInput.count - (sortedInput.containsNull ? 1 : 0);

  while(currNumRanges >  numRangesKeep){
    int bestDist;
    int bestIndex;
    int currDist;
    int i;
    int j;
    Int4Range a;
    Int4Range b;
    Int4Range toInsert;

    bestDist = -1;
    bestIndex = -1;

    // greedy look for smallest remaining gap
    // O(N)
    for(i=1; i<currNumRanges; i++){
      currDist = abs(range_distance(sortedInput.ranges[i], sortedInput.ranges[i-1]));
      
      // compare distances and keep min difference between 2 ranges in entire set
      if(bestDist < 0 || currDist < bestDist){
        bestDist = currDist;
        bestIndex = i-1;
      }
    }
    
    a = sortedInput.ranges[bestIndex];
    b = sortedInput.ranges[bestIndex+1];

    toInsert.lower = (a.lower < b.lower ? a.lower : b.lower);
    toInsert.upper = (a.upper > b.upper ? a.upper : b.upper);

    sortedInput.ranges[bestIndex] = toInsert;

    for (j=bestIndex+1; j<currNumRanges-1; j++){
      sortedInput.ranges[j] = sortedInput.ranges[j+1];
    }

    currNumRanges -= 1;
  }

  sortedInput.count = currNumRanges;

  return sortedInput;
}

// removes any NULLs in the set. returns a palloc'd set with containsNULL = false.
// set no longer knows it contains nulls; this must be tracked and known with caller when called.
Int4RangeSet filterOutNulls(Int4RangeSet vals) {
  Int4RangeSet filteredVals;
  size_t nonNullCount;
  size_t i;
  int idx;
  
  if (!vals.containsNull) {
    return vals;
  }

  nonNullCount = 0;
  for (i = 0; i < vals.count; i++) {
    if (!vals.ranges[i].isNull) nonNullCount++;
  }

  filteredVals.count = nonNullCount;
  filteredVals.ranges = palloc(sizeof(Int4Range) * filteredVals.count);
  filteredVals.containsNull = false;
  // filteredVals.containsNull = vals.containsNull;

  idx = 0;
  for (i = 0; i < vals.count; i++) {
    if (!vals.ranges[i].isNull) {
      filteredVals.ranges[idx].isNull = false;
      filteredVals.ranges[idx++] = vals.ranges[i];
    }
  }

  pfree(vals.ranges);

  return filteredVals;
}

// linear scan through vals and accumulate coverage volume
long totalSpan(Int4RangeSet vals)
{
  long volume = 0;
  for (size_t i=0; i < vals.count; i++) {
    volume += (vals.ranges[i].upper - vals.ranges[i].lower);
  }
  
  return volume;
}

// reduce size and return newly allocated RangeSet
/*
  sort the input and greedily merge. Traverses through entire set while we have more ranges than desires.
  O(N^2) worst case if numRangesKeep = N, we sort (NlogN), N times
*/
Int4RangeSet reduceSizeNN(Int4RangeSet vals, int numRangesKeep){
  Int4RangeSet normalized;
  Int4RangeSet sortedInput;
  int currNumRanges;

  if (vals.count <= numRangesKeep){
    return vals;
  }
  else if (vals.count == 0){
    normalized.count = 0;
    normalized.containsNull = false;
    normalized.ranges = NULL;
  }
  else if (vals.containsNull && vals.count == 1){
    normalized.count = 1;
    normalized.containsNull = true;
    normalized.ranges = palloc(sizeof(Int4Range));
    normalized.ranges[0].isNull = true;
    normalized.ranges[0].lower = 0;
    normalized.ranges[0].upper = 0;
  }
  
  sortedInput = sort(vals);
  // sortedInput = normalize(vals);
  // check size condition after normalize potential collapsing
  if (sortedInput.count <= numRangesKeep) {
    return sortedInput;
  }
  
  // ignore the NULL range at sortedInput.ranges[len-1]
  currNumRanges = sortedInput.count - (sortedInput.containsNull ? 1 : 0);

  while(currNumRanges >  numRangesKeep){
    int bestDist;
    int bestIndex;
    int currDist;
    int i;
    int j;
    Int4Range a;
    Int4Range b;
    Int4Range toInsert;

    bestDist = -1;
    bestIndex = -1;

    // greedy look for smallest remaining gap
    // O(N)
    for(i=1; i<currNumRanges; i++){
      currDist = abs(range_distance(sortedInput.ranges[i], sortedInput.ranges[i-1]));
      
      // compare distances and keep min difference between 2 ranges in entire set
      if(bestDist < 0 || currDist < bestDist){
        bestDist = currDist;
        bestIndex = i-1;
      }
    }
    
    a = sortedInput.ranges[bestIndex];
    b = sortedInput.ranges[bestIndex+1];

    toInsert.lower = (a.lower < b.lower ? a.lower : b.lower);
    toInsert.upper = (a.upper > b.upper ? a.upper : b.upper);

    sortedInput.ranges[bestIndex] = toInsert;

    for (j=bestIndex+1; j<currNumRanges-1; j++){
      sortedInput.ranges[j] = sortedInput.ranges[j+1];
    }

    currNumRanges -= 1;
  }

  sortedInput.count = currNumRanges;

  return sortedInput;
}

// intersection between 2 ranges. handles disjoint by returning isNull flag == invalid range
Int4Range intersect_range(Int4Range a, Int4Range b) {
  Int4Range result;
  result.isNull = false;

  if (a.isNull || b.isNull) {
    result.isNull = true;
    return result;
  }

  result.lower = (a.lower > b.lower) ? a.lower : b.lower;
  result.upper = (a.upper < b.upper) ? a.upper : b.upper;

  if (result.lower >= result.upper) {
    result.isNull = true;
  }

  return result;
}

// intersection between 2 sets
Int4RangeSet intersect_set(Int4RangeSet a, Int4RangeSet b) {
  Int4RangeSet result;
  result.count = 0;
  result.containsNull = a.containsNull && b.containsNull;
  result.ranges = palloc(sizeof(Int4Range) * (a.count * b.count));

  for (int i = 0; i < a.count; i++) {
    for (int j = 0; j < b.count; j++) {
      Int4Range r = intersect_range(a.ranges[i], b.ranges[j]);
      if (!r.isNull) {
        result.ranges[result.count++] = r;
      }
    }
  }

  return result;
}

Int4RangeSet union_range(Int4Range a, Int4Range b) {
  Int4RangeSet result;
  result.count = 0;
  result.containsNull = false;
  result.ranges = palloc(sizeof(Int4Range) * 2);

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

  // Merge if overlapping or adjacent
  if (a.upper >= b.lower && b.upper >= a.lower) {
    Int4Range merged;
    merged.lower = (a.lower < b.lower) ? a.lower : b.lower;
    merged.upper = (a.upper > b.upper) ? a.upper : b.upper;
    merged.isNull = false;
    result.ranges[0] = merged;
    result.count = 1;
  } 
  else {
    // Disjoint
    if (a.lower < b.lower) {
      result.ranges[0] = a;
      result.ranges[1] = b;
    } 
    else {
      result.ranges[0] = b;
      result.ranges[1] = a;
    }
    result.count = 2;
  }

  return result;
}

// concat and normalize
Int4RangeSet union_set(Int4RangeSet a, Int4RangeSet b) {
  Int4RangeSet result;
  result.count = 0;
  result.containsNull = a.containsNull || b.containsNull;
  result.ranges = palloc(sizeof(Int4Range) * (a.count + b.count));

  // copy all ranges from a
  for (int i = 0; i < a.count; i++) {
    result.ranges[result.count++] = a.ranges[i];
  }

  // merge each range from b
  for (int i = 0; i < b.count; i++) {
    Int4RangeSet temp = union_range(result.ranges[0], b.ranges[i]);
    if (temp.count == 1) {
      result.ranges[0] = temp.ranges[0];
      result.count = 1;
    }
     else {
      result.ranges[0] = temp.ranges[0];
      result.ranges[1] = temp.ranges[1];
      result.count = 2;
    }
  }

  return result;
}

// multiply each element in set by scalar. used with Set x MULT
Int4RangeSet range_set_multiply_scalar(Int4RangeSet set1, int32 scalar) {
    Int4RangeSet result;
    result.count = set1.count;
    result.ranges = palloc(sizeof(Int4Range) * set1.count);
    
    for (int i = 0; i < set1.count; i++) {
        // pointwise
        result.ranges[i].lower = set1.ranges[i].lower * scalar;
        result.ranges[i].upper = set1.ranges[i].upper * scalar;
        result.ranges[i].isNull = set1.ranges[i].isNull;
    }
    return result;
}




////////////////////////
/// combine helpers
////////////////////////

/*
combines every range in Set with every integer value in mult. (Sn x Mn) operation.
same for set and range because range combine can result in a set worst case. Not true for min/max
*/ 
Int4RangeSet
internal_agg_sum_combine_set_mult(Int4RangeSet set1, Int4Range mult) {
    Int4RangeSet result;
    int total_result_ranges;
    int i;
    int j;
    int idx;
    Int4Range stack[1];   // create array size 1 on stack
    Int4RangeSet multSet;
    Int4RangeSet tempResult;
    Int4RangeSet normOutput;

    if (set1.count == 0) {
      return empty_set();  
    }

    if (mult.upper <= mult.lower || mult.lower < 0 || mult.upper < 0 || mult.isNull) {
      return empty_set();
    }

    total_result_ranges = set1.count * (mult.upper - mult.lower);
    result.count = 0;
    result.containsNull = false;
    result.ranges = palloc(sizeof(Int4Range) * total_result_ranges);
    
    multSet.count = 1;
    multSet.ranges = stack;
    multSet.containsNull = false;

    idx = 0;
    // traverse thru every set/mult combination and union result
    for (i = mult.lower; i < mult.upper; i++) {
      // ignore mult == 0 bc it produced NULL flag
      if (i == 0) continue;
      
      // temp lifted mult x -> [x, x+1)
      multSet.ranges[0].lower = i;
      multSet.ranges[0].upper = i+1;
      multSet.ranges[0].isNull = false;
    
      // combine
      tempResult = range_set_multiply_internal(set1, multSet);

      // union in new results
      for (j = 0; j < tempResult.count; j++) {
        if (idx >= total_result_ranges) {
          elog(ERROR, "internal_agg_sum_combine_set_mult: Internal error: exceeded pre-allocated range set size");
        }
        result.ranges[idx] = tempResult.ranges[j];
        idx++;
      }
      pfree(tempResult.ranges);

    }
    result.count = idx;

    // normalize before returning
    normOutput = normalize(result);
    pfree(result.ranges);
    
    return normOutput;
}

// returns range if mult.lb > 0. otherwise return neutral element and a NULL range
// compare with function below this one...?
Int4Range internal_agg_min_max_combine_range_mult2(Int4Range range, Int4Range mult, int64 neutral_elem) {
  Int4Range result;
  result = range;

  // invalid cases
  if (mult.isNull || mult.upper <= mult.lower || mult.lower < 0) {
    result.lower = neutral_elem;
    result.upper = neutral_elem;
    result.isNull = true;
    return result;
  }
  
  // mult is only zero. nothing should contribute
  if (mult.lower == 0 && mult.upper == 1) {
    result.lower = neutral_elem;
    result.upper = neutral_elem;
    result.isNull = true;
    return result;
  }
  
  // otherwise the mult = [0, n), with n > 1 so we return initial input
  return range;
}

// returns val if mult.lb > 0. otherwise return NULL which represents neutral element (i think)
// compare with function above this one...?
Int4Range internal_agg_min_max_combine_range_mult(Int4Range val, Int4Range mult) {
  Int4Range neutral;
  neutral.lower  = 0;
  neutral.upper  = 0;
  neutral.isNull = true;   // caller must PG_RETURN_NULL() on this 

  // invalid or empty mult 
  if (mult.isNull || mult.upper <= mult.lower || mult.lower < 0)
    return neutral;

  // [0,1) mult == exactly zero
  if (mult.lower == 0 && mult.upper == 1)
    return neutral;

  // otherwise contribute
  return val;
}

// returns val if mult.lb > 0. otherwise return NULL which represents neutral element (i think)
Int4RangeSet internal_agg_min_max_combine_set_mult(Int4RangeSet val, Int4Range mult) {
  Int4RangeSet neutral;
  neutral.ranges = NULL;
  neutral.count = 0;
  neutral.containsNull = true;  // caller must PG_RETURN_NULL() on this 

  // invalid or empty mult 
  if (mult.isNull || mult.upper <= mult.lower || mult.lower < 0)
    return neutral;

  // [0,1) mult == exactly zero
  if (mult.lower == 0 && mult.upper == 1)
    return neutral;

  // otherwise contribute
  return val;
}
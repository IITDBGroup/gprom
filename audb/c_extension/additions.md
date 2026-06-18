## 
* add aggregation logic - similar to prune functions, https://www.postgresql.org/docs/current/xaggr.html
* create test cases and answers in a runnable sql script
- [x] detect and handle lifting a i4r to an i4r set. overload funcitons pg_c functions to handle case 

additional optimizing
* reduce and convert boilerplate to macros 
* shell script entire process of building, running and testing


aggregate sum size therashold. extra parameter
if num of ranges is greater than extra param, then reduce and continue
or add 2 params, 1 target to start reducing 2




1/1/26 fixes
- [] go over arithmetic and fix empty results in Int4RangeSet   func removeEmpty()
- [] memory allocation. pass by ref vs val in self functions
- [] change rangeTypeOID and typCache to take many types other than I4R
- [] values that cross zero means ...?


1/10/26 Code Review
- [] logicalOperators.c && logicalOperators.h (any advice for .h files)
- [] arithmetic.c (confusion on division)
- []
- []


1/22
Get rid of C90 Warnings
add finalfunc for sum
work on min/max
add more reg tests


1/25
- [] serialize and deserialize need empty array checks perhaps


1/26
- [] need better handling of agg mult nulls. if mult is null then entire row should be NULL?
- [X] min and max return smallest/largest possible range or just smallest vals
- [] in-range checks. ex  c_range_multiply('[50000,60000)', '[50000,60000)') == overflow

1/27
- [] arithmetic NULL results in T or F or always unknown (Null) ex range_lt(NULL, int4range(1,3))
- [] sum doesnt use reduce size, only normalize. test optimal reduction methods
#include "serialization.h"

/*
Helper function -
    Take in RangeType of type typcache and return local definition of Int4Range (I4R)
    Must eventually be freed by caller

    * Improvements: does properly handle empty case. Although our results wouldn't result in empty for the most part (i think)
    * handle NULL
*/
Int4Range
deserialize_RangeType(RangeType *rng, TypeCacheEntry *typcache)
{
    Int4Range range;
    RangeBound lower;
    RangeBound upper;
    bool isEmpty;

    range_deserialize(typcache, rng, &lower, &upper, &isEmpty);   

    // should we care to store empty ranges? same behavior as NULL
    if (isEmpty) {
        range.isNull = true;
        range.lower = 0;
        range.upper = 0;
        return range;
    }
    
    range.lower = DatumGetInt32(lower.val);
    range.upper = DatumGetInt32(upper.val);
    range.isNull = false;

    return range;
}

/*
Helper function -
    Take the Int4Range result of type typcache and return newly palloc RangeType
    Must eventually be freed by caller

    * Improvements: does properly handle empty case. Although our results wouldn't result in empty for the most part (i think)
    * handle NULL
*/
RangeType*
serialize_RangeType(Int4Range range, TypeCacheEntry *typcache)
{
    // have no way to represent empty. Can represent NULL, but not empty
    // if (range.isempty???) does not exist
    // return result = make_range(typcache, &lower, &upper, true, NULL);

    RangeType *result;
    RangeBound lower;
    RangeBound upper;

    // NULL range != empty range, how else to encode this?
    if (range.isNull) {
        return make_empty_range(typcache);
    }

    lower = make_range_bound(range.lower, true, true);
    upper = make_range_bound(range.upper, false, false);

    result = make_range(
        typcache,
        &lower,
        &upper,
        false,
        NULL
    );

    return result;
}

/*
Helper function -
    Take in ArrayType of type typcache and return local definition of rangeset (I4RSet)
    Must be freed by caller

    * confusion on how to handle many nulls in arry: [(1,3), NULL, NULL, NULL] => I4Rset {(1,3), NULL or more NULLS?}
     i think this is a slight enhancement tho bc it reduces extra calculations later on. redundant nulls dont mean anything new

    * Improvements: does not properly handle empty case. Although our results wouldn't result in empty for the most part (i think)
*/
Int4RangeSet
deserialize_ArrayType(ArrayType *arr, TypeCacheEntry *typcache)
{
    // if (empty??)
    Int4RangeSet set;

    // get type information. General this should be RangeType[] where RangeType varies
    Oid rangeTypeOID;
    int16 typlen;
    bool typbyval;
    char typalign;
    Datum *elems;
    bool *nulls;
    int count;
    int currIdx;
    int i;

    rangeTypeOID = typcache->type_id;
    get_typlenbyvalalign(rangeTypeOID, &typlen, &typbyval, &typalign);
    
    // deconstruct array
    deconstruct_array(arr, rangeTypeOID, typlen, typbyval, typalign, &elems, &nulls, &count);

    // create an empty I4RSet if Array is empty
    // FIXME error here on return
    if (count < 1) {
        set.count = 1;
        set.containsNull = true;
        set.ranges = palloc(sizeof(Int4Range));
        set.ranges[0].isNull = true;
        set.ranges[0].lower = 0;
        set.ranges[0].upper = 0;
    }

    // initialize our representation of I4R
    set.containsNull = false;
    set.ranges = palloc(sizeof(Int4Range) * count);

    // go through every RangeType in array, and deserialize it
    currIdx = 0;
    for (i = 0; i < count; i++) {
        
        // the RangeType at this index is NULL; trigger containsNull and set index isNull bool to true
        // only insert a NULL entry ONCE into the currIdx
        if (nulls[i] && !set.containsNull) {
            set.ranges[currIdx].lower = 0;
            set.ranges[currIdx].upper = 0; // maybe remove?
            set.ranges[currIdx].isNull = true;
            set.containsNull = true;
            currIdx++;
        }
        // otherwise Extract RangeType elements... only if not NULL. ignore nulls after first NULL found
        else if (!nulls[i]){
            RangeType *curr;
            RangeBound l1, u1;
            bool isEmpty;
            
            curr = DatumGetRangeTypeP(elems[i]);
            range_deserialize(typcache, curr, &l1, &u1, &isEmpty);

            // if the RangeType is not empty, append it 
            if (!isEmpty) {
                set.ranges[currIdx].lower = DatumGetInt32(l1.val);
                set.ranges[currIdx].upper = DatumGetInt32(u1.val);
                set.ranges[currIdx].isNull = false;
                currIdx++;
            }
        }
        // otherwise ignore
    }
    set.count = currIdx;

    pfree(elems);
    pfree(nulls);

    return set;
}

/*
Helper function -
    Take in ArrayType of type typcache and return local definition of rangeset (I4RSet)

    // Improvements: does not properly handle empty case. Although our results wouldn't result in empty for the most part (i think)
*/
ArrayType* 
serialize_ArrayType(Int4RangeSet set, TypeCacheEntry *typcache)
{
    ArrayType *result;
    RangeBound lowerRv, upperRv;
    RangeType *r;
    int ndim;
    int dims[1];
    int lbs[1];

    // must handle datums and nulls independently. get errors when calling range functions (make_range, range_deserialize) on nulls
    Datum *datums;
    bool  *nulls;
    
    datums = palloc(sizeof(Datum) * set.count);
    nulls  = palloc(sizeof(bool) * set.count);

    for(int i=0; i < set.count; i++) {
        // trigger NULL index properly
        if (set.ranges[i].isNull) {
            nulls[i] = true;
            datums[i] = (Datum) 0;      // NullableDatum Flag = 0
            continue;
        }
        
        nulls[i] = false;
        
        lowerRv = make_range_bound(set.ranges[i].lower, true, true);
        upperRv = make_range_bound(set.ranges[i].upper, false, false);
        
        r = make_range(typcache, &lowerRv, &upperRv, false, NULL);
        datums[i] = RangeTypePGetDatum(r);
    }
    // 1D array of size set.count
    ndim = 1;
    dims[0] = set.count;
    lbs[0] = 1;
    
    result = construct_md_array(
            datums, 
            nulls, 
            ndim, 
            dims, 
            lbs, 
            typcache->type_id, 
            typcache->typlen, 
            typcache->typbyval, 
            typcache->typalign
    );

    pfree(datums);
    pfree(nulls);

    return result;
}

/*
Helper function -
    Take in RangeType of type typcache and return local definition of rangeset (I4R)

    * Inclusive lower, exclusive upper
*/
RangeBound make_range_bound(int32 val, bool is_lower, bool inclusive)
{
    RangeBound rvBound;
    rvBound.val = Int32GetDatum(val);
    rvBound.inclusive = inclusive;
    rvBound.infinite = false;
    rvBound.lower = is_lower;
    return rvBound;
}



///////////

/*
Helper function -
    Take in ArrayType of type typcache and return local definition of rangeset (I4RSet)

    // Improvements: does not properly handle empty case. Although our results wouldn't result in empty for the most part (i think)
*/
ArrayType* 
serialize_ArrayType2(Int4RangeSet set, Oid rangeTypeOid, TypeCacheEntry *typcache)
{
    ArrayType *result;
    RangeBound lowerRv, upperRv;
    RangeType *r;
    int ndim;
    int dims[1];
    int lbs[1];
    Datum *datums;
    bool  *nulls; 

    if (set.count == 0) {
        // Return an empty array
        return construct_empty_array(rangeTypeOid);
    }
    
    datums = palloc(sizeof(Datum) * set.count);
    nulls = palloc(sizeof(bool) * set.count);

    for (int i = 0; i < set.count; i++) {
        if (set.ranges[i].isNull) {
            nulls[i] = true;
            datums[i] = (Datum) 0;
            continue;
        }

        nulls[i] = false;

        lowerRv = make_range_bound(set.ranges[i].lower, true, true);
        upperRv = make_range_bound(set.ranges[i].upper, false, false);

        r = make_range(typcache, &lowerRv, &upperRv, false, NULL);
        datums[i] = RangeTypePGetDatum(r);
    }

    // 1D array
    ndim = 1;
    dims[0] = set.count;
    lbs[0] = 1;
    
    elog(INFO, "rangeTypeOid=%u typlen=%d typbyval=%d typalign=%c",
     rangeTypeOid, typcache->typlen, typcache->typbyval, typcache->typalign);

    result = construct_md_array(
        datums,
        nulls,
        ndim,
        dims,
        lbs,
        rangeTypeOid,         // use actual range type OID
        typcache->typlen,
        typcache->typbyval,
        typcache->typalign
    );

    pfree(datums);
    pfree(nulls);

    return result;
}
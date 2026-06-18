#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include "helperFunctions.h"    // logic for helpers

// source files
#include "postgres.h"           // src
#include "fmgr.h"               // must be included
#include "utils/rangetypes.h"   // rangeType
#include "utils/array.h"        // arrayType
#include "utils/typcache.h"     // speed type lookup
#include "utils/lsyscache.h"    // typcache convenience functions 
#include "catalog/pg_type_d.h"  // pg_type OIDs
#include "catalog/namespace.h"  // get typenames
#include "funcapi.h"

Int4Range deserialize_RangeType(RangeType *rng, TypeCacheEntry *typcache);
RangeType* serialize_RangeType(Int4Range range, TypeCacheEntry *typcache);
RangeBound make_range_bound(int32 val, bool is_lower, bool inclusive);
Int4RangeSet deserialize_ArrayType(ArrayType *arr, TypeCacheEntry *typcache);
ArrayType* serialize_ArrayType(Int4RangeSet set, TypeCacheEntry *typcache);
ArrayType* serialize_ArrayType2(Int4RangeSet set, Oid rangeTypeOid, TypeCacheEntry *typcache);
#endif

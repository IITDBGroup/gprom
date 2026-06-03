#include "provenance_rewriter/uncertainty_rewrites/uset_i4r_prune_registry.h"
#include "provenance_rewriter/uncertainty_rewrites/uncert_rewriter.h"
#include "utility/string_utils.h"

const UsetI4rPruneEntry USET_I4R_PRUNE_REGISTRY[] = {
    /* Range 层（单区间 int4range） */
    { "prune_lt_internal_range",  PRUNE_RANGE_LT_FUNC_NAME,  AUDB_RANGE_LT_FUNC_NAME,  USET_I4R_PRUNE_LAYER_RANGE },
    { "prune_lte_internal_range", PRUNE_RANGE_LTE_FUNC_NAME, AUDB_RANGE_LTE_FUNC_NAME, USET_I4R_PRUNE_LAYER_RANGE },
    { "prune_gt_internal_range",  PRUNE_RANGE_GT_FUNC_NAME,  AUDB_RANGE_GT_FUNC_NAME,  USET_I4R_PRUNE_LAYER_RANGE },
    { "prune_gte_internal_range", PRUNE_RANGE_GTE_FUNC_NAME, AUDB_RANGE_GTE_FUNC_NAME, USET_I4R_PRUNE_LAYER_RANGE },
    { "prune_eq_internal_range",  PRUNE_RANGE_EQ_FUNC_NAME,  AUDB_RANGE_EQ_FUNC_NAME,  USET_I4R_PRUNE_LAYER_RANGE },
    { "prune_AND_internal_range", PRUNE_RANGE_AND_FUNC_NAME, NULL,                     USET_I4R_PRUNE_LAYER_RANGE },
    { "prune_OR_internal_range",  PRUNE_RANGE_OR_FUNC_NAME,  NULL,                     USET_I4R_PRUNE_LAYER_RANGE },
    /* Set 层（int4range[]） */
    { "prune_lt_set_internal",    PRUNE_SET_LT_FUNC_NAME,    AUDB_SET_LT_FUNC_NAME,    USET_I4R_PRUNE_LAYER_SET },
    { "prune_lte_set_internal",     PRUNE_SET_LTE_FUNC_NAME,   AUDB_SET_LTE_FUNC_NAME,   USET_I4R_PRUNE_LAYER_SET },
    { "prune_gt_set_internal",      PRUNE_SET_GT_FUNC_NAME,    AUDB_SET_GT_FUNC_NAME,    USET_I4R_PRUNE_LAYER_SET },
    { "prune_gte_set_internal",     PRUNE_SET_GTE_FUNC_NAME,   AUDB_SET_GTE_FUNC_NAME,   USET_I4R_PRUNE_LAYER_SET },
    { "prune_eq_set_internal",      PRUNE_SET_EQ_FUNC_NAME,    AUDB_SET_EQ_FUNC_NAME,    USET_I4R_PRUNE_LAYER_SET },
    { "prune_AND_internal_set",     PRUNE_SET_AND_FUNC_NAME,   NULL,                     USET_I4R_PRUNE_LAYER_SET },
    { "prune_OR_internal_set",      PRUNE_SET_OR_FUNC_NAME,    NULL,                     USET_I4R_PRUNE_LAYER_SET },
};

const int USET_I4R_PRUNE_REGISTRY_LEN =
    (int)(sizeof(USET_I4R_PRUNE_REGISTRY) / sizeof(USET_I4R_PRUNE_REGISTRY[0]));

boolean
usetI4rIsPruneSqlFunc(const char *sqlName)
{
    int i;
    if (!sqlName)
        return FALSE;
    for (i = 0; i < USET_I4R_PRUNE_REGISTRY_LEN; i++)
        if (strieq((char *)sqlName, (char *)USET_I4R_PRUNE_REGISTRY[i].sqlName))
            return TRUE;
    /* 旧 PL/pgSQL 名 */
    return strieq((char *)sqlName, "prune_eq") || strieq((char *)sqlName, "prune_lt")
        || strieq((char *)sqlName, "prune_gt") || strieq((char *)sqlName, "prune_and")
        || strieq((char *)sqlName, "prune_or");
}

boolean
usetI4rIsCompareSqlFunc(const char *compareName)
{
    int i;
    if (!compareName)
        return FALSE;
    for (i = 0; i < USET_I4R_PRUNE_REGISTRY_LEN; i++)
        if (USET_I4R_PRUNE_REGISTRY[i].compareName != NULL
            && strieq((char *)compareName, (char *)USET_I4R_PRUNE_REGISTRY[i].compareName))
            return TRUE;
    return FALSE;
}

const char *
usetI4rPruneSqlForCompare(const char *compareName)
{
    int i;
    if (!compareName)
        return NULL;
    for (i = 0; i < USET_I4R_PRUNE_REGISTRY_LEN; i++)
        if (USET_I4R_PRUNE_REGISTRY[i].compareName != NULL
            && strieq((char *)compareName, (char *)USET_I4R_PRUNE_REGISTRY[i].compareName))
            return USET_I4R_PRUNE_REGISTRY[i].sqlName;
    return NULL;
}

const char *
usetI4rPruneAndSqlName(boolean rangeFamily)
{
    return rangeFamily ? PRUNE_RANGE_AND_FUNC_NAME : PRUNE_SET_AND_FUNC_NAME;
}

const char *
usetI4rPruneOrSqlName(boolean rangeFamily)
{
    return rangeFamily ? PRUNE_RANGE_OR_FUNC_NAME : PRUNE_SET_OR_FUNC_NAME;
}

UsetI4rPruneLayer
usetI4rPruneLayerForCompare(const char *compareName)
{
    int i;
    if (!compareName)
        return USET_I4R_PRUNE_LAYER_SET;
    for (i = 0; i < USET_I4R_PRUNE_REGISTRY_LEN; i++)
        if (USET_I4R_PRUNE_REGISTRY[i].compareName != NULL
            && strieq((char *)compareName, (char *)USET_I4R_PRUNE_REGISTRY[i].compareName))
            return USET_I4R_PRUNE_REGISTRY[i].layer;
    return USET_I4R_PRUNE_LAYER_SET;
}

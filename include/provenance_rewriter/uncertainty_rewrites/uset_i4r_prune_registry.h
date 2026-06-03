#ifndef USET_I4R_PRUNE_REGISTRY_H
#define USET_I4R_PRUNE_REGISTRY_H

#include "common.h"

/*
 * i4r_audb_extension 剪枝函数注册表（GProM 生成 SQL 用名 ↔ i4r 内部 C 名）。
 *
 * 设计原则：
 * - GProM 只生成 PostgreSQL 可执行的 SQL 名（prune_set_* / prune_range_*）。
 * - i4r 内部名（prune_*_internal_*）仅作文档/对照，由 PG 别名层或 C 扩展实现。
 * - WHERE：set_* / range_* 三值比较；WITH PRUNING 投影：prune_* + direction。
 */

typedef enum UsetI4rPruneLayer
{
    USET_I4R_PRUNE_LAYER_RANGE = 0, /* int4range 单区间 */
    USET_I4R_PRUNE_LAYER_SET = 1    /* int4range[] 区间集合 */
} UsetI4rPruneLayer;

typedef struct UsetI4rPruneEntry
{
    const char *internalName;  /* i4r C 内部实现名（对照表） */
    const char *sqlName;       /* GProM / PostgreSQL 函数名 */
    const char *compareName;   /* 对应 WHERE 三值比较（NULL 表示纯逻辑） */
    UsetI4rPruneLayer layer;
} UsetI4rPruneEntry;

/* 全表：Range 7 + Set 7 = 14 项（与 i4r 剪枝对照表一致） */
extern const UsetI4rPruneEntry USET_I4R_PRUNE_REGISTRY[];
extern const int USET_I4R_PRUNE_REGISTRY_LEN;

extern boolean usetI4rIsPruneSqlFunc(const char *sqlName);
extern boolean usetI4rIsCompareSqlFunc(const char *compareName);
extern const char *usetI4rPruneSqlForCompare(const char *compareName);
extern const char *usetI4rPruneAndSqlName(boolean rangeFamily);
extern const char *usetI4rPruneOrSqlName(boolean rangeFamily);
extern UsetI4rPruneLayer usetI4rPruneLayerForCompare(const char *compareName);

#endif /* USET_I4R_PRUNE_REGISTRY_H */

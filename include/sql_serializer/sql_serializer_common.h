/*-----------------------------------------------------------------------------
 *
 * sql_serializer_common.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_COMMON_H_
#define INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_COMMON_H_

#include "model/query_operator/query_operator.h"
#include "model/list/list.h"

/* data structures */
typedef enum MatchState {
    MATCH_START,
    MATCH_DISTINCT,
    MATCH_FIRST_PROJ,
    MATCH_HAVING,
    MATCH_AGGREGATION,
    MATCH_SECOND_PROJ,
    MATCH_WHERE,
    MATCH_WINDOW,
    MATCH_ORDER,
    MATCH_NEXTBLOCK
} MatchState;

#define OUT_MATCH_STATE(_state) \
    (_state == MATCH_START ? "MATCH_START" : \
     _state == MATCH_DISTINCT ? "MATCH_DISTINCT" : \
     _state == MATCH_FIRST_PROJ ? "MATCH_FIRST_PROJ" : \
     _state == MATCH_HAVING ? "MATCH_HAVING" : \
     _state == MATCH_AGGREGATION ? "MATCH_AGGREGATION" : \
     _state == MATCH_SECOND_PROJ ? "MATCH_SECOND_PROJ" : \
     _state == MATCH_WHERE ? "MATCH_WHERE" : \
     _state == MATCH_WINDOW ? "MATCH_WINDOW" : \
     _state == MATCH_WINDOW ? "MATCH_ORDER" : \
             "MATCH_NEXTBLOCK" \
     )

typedef struct QueryBlockMatch {
    DuplicateRemoval *distinct;
    ProjectionOperator *firstProj;
    SelectionOperator *having;
    AggregationOperator *aggregation;
    ProjectionOperator *secondProj;
    SelectionOperator *where;
    QueryOperator *fromRoot;
    WindowOperator *windowRoot;
    OrderOperator *orderBy;
} QueryBlockMatch;

#define OUT_BLOCK_MATCH(_level,_m,_message) \
    do { \
        _level ## _LOG ("MATCH INFO: %s", _message); \
        _level ## _LOG ("distinct: %s", operatorToOverviewString((Node *) _m->distinct)); \
        _level ## _LOG ("firstProj: %s", operatorToOverviewString((Node *) _m->firstProj)); \
        _level ## _LOG ("having: %s", operatorToOverviewString((Node *) _m->having)); \
        _level ## _LOG ("aggregation: %s", operatorToOverviewString((Node *) _m->aggregation)); \
        _level ## _LOG ("secondProj: %s", operatorToOverviewString((Node *) _m->secondProj)); \
        _level ## _LOG ("where: %s", operatorToOverviewString((Node *) _m->where)); \
        _level ## _LOG ("fromRoot: %s", operatorToOverviewString((Node *) _m->fromRoot)); \
        _level ## _LOG ("windowRoot: %s", operatorToOverviewString((Node *) _m->windowRoot)); \
        _level ## _LOG ("orderBy: %s", operatorToOverviewString((Node *) _m->orderBy)); \
    } while(0)

typedef struct TemporaryViewMap {
    QueryOperator *viewOp; // the key
    char *viewName;
    char *viewDefinition;
    List *attrNames;
    UT_hash_handle hh;
} TemporaryViewMap;

typedef struct UpdateAggAndGroupByAttrState {
    List *aggNames;
    List *groupByNames;
} UpdateAggAndGroupByAttrState;

typedef struct JoinAttrRenameState {
    int rightFromOffsets;
    List *fromAttrs;
} JoinAttrRenameState;

typedef struct SerializeClausesAPI {
    List * (*serializeProjectionAndAggregation) (QueryBlockMatch *m, StringInfo select,
            StringInfo having, StringInfo groupBy, List *fromAttrs, boolean materialize);
    void (*serializeFrom) (QueryOperator *q, StringInfo from, List **fromAttrs);
    void (*serializeWhere) (SelectionOperator *q, StringInfo where, List *fromAttrs);
    List *(*serializeSetOperator) (QueryOperator *q, StringInfo str);
} SerializeClausesAPI;

/* macros */
#define OPEN_PARENS(str) appendStringInfoChar(str, '(')
#define CLOSE_PARENS(str) appendStringInfoChar(str, ')')
#define WITH_PARENS(str,operation) \
    do { \
        OPEN_PARENS(str); \
        operation; \
        CLOSE_PARENS(str); \
    } while(0)

#endif /* INCLUDE_SQL_SERIALIZER_SQL_SERIALIZER_COMMON_H_ */

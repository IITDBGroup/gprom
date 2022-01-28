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
#include "model/set/hashmap.h"

/* data structures */
NEW_ENUM_WITH_TO_STRING(MatchState,
    MATCH_START,
    MATCH_DISTINCT,
    MATCH_FIRST_PROJ,
    MATCH_HAVING,
    MATCH_AGGREGATION,
    MATCH_SECOND_PROJ,
    MATCH_WHERE,
    MATCH_WINDOW,
    MATCH_ORDER,
	MATCH_LIMIT,
    MATCH_NEXTBLOCK
);

#define OUT_MATCH_STATE(_state) MatchStateToString(_state)

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
	LimitOperator *limitOffset;
} QueryBlockMatch;

#define OUT_BLOCK_MATCH(_level,_m,_message) \
    do { \
        _level ## _LOG ("\n================================================================================\n\t\tMATCH INFO: %s\n================================================================================", _message); \
        _level ## _LOG ("distinct: %s", operatorToOverviewString((Node *) _m->distinct)); \
        _level ## _LOG ("firstProj: %s", operatorToOverviewString((Node *) _m->firstProj)); \
        _level ## _LOG ("having: %s", operatorToOverviewString((Node *) _m->having)); \
        _level ## _LOG ("aggregation: %s", operatorToOverviewString((Node *) _m->aggregation)); \
        _level ## _LOG ("secondProj: %s", operatorToOverviewString((Node *) _m->secondProj)); \
        _level ## _LOG ("where: %s", operatorToOverviewString((Node *) _m->where)); \
        _level ## _LOG ("fromRoot: %s", operatorToOverviewString((Node *) _m->fromRoot)); \
        _level ## _LOG ("windowRoot: %s", operatorToOverviewString((Node *) _m->windowRoot)); \
        _level ## _LOG ("orderBy: %s", operatorToOverviewString((Node *) _m->orderBy)); \
		_level ## _LOG ("limitOffset: %s", operatorToOverviewString((Node *) _m->limitOffset)); \
		_level ## _LOG ("\n================================================================================\n\n================================================================================"); \
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


typedef struct FromAttrsContext {
    List *fromAttrs;
    List *fromAttrsList;
	HashMap *nestAttrMap;
} FromAttrsContext;

typedef struct JoinAttrRenameState {
    int rightFromOffsets;
    FromAttrsContext *fac;
} JoinAttrRenameState;


//typedef struct JoinStateFac {
//	JoinAttrRenameState *state;
//	FromAttrsContext *fac;
//} JoinStateFac;

#define TVIEW_NAME_FIELD "ViewName"
#define TVIEW_ATTRNAMES_FIELD "AttrNames"
#define TVIEW_DEF_FIELD "Definition"

#define TVIEW_GET_NAME(tview) STRING_VALUE(MAP_GET_STRING(tview, TVIEW_NAME_FIELD))
#define TVIEW_GET_ATTRNAMES(tview) ((List *) MAP_GET_STRING(tview, TVIEW_ATTRNAMES_FIELD))
#define TVIEW_GET_DEF(tview) STRING_VALUE(MAP_GET_STRING(tview, TVIEW_DEF_FIELD))

#define TVIEW_SET_NAME(tview, name) MAP_ADD_STRING_KEY(tview, strdup(TVIEW_NAME_FIELD), createConstString(name))
#define TVIEW_SET_ATTRNAMES(tview, attrs) MAP_ADD_STRING_KEY(tview, strdup(TVIEW_ATTRNAMES_FIELD), attrs)
#define TVIEW_SET_DEF(tview,def) MAP_ADD_STRING_KEY(tview, strdup(TVIEW_DEF_FIELD), createConstString(def))

typedef enum {
	NEST_SER_SELECTION = 1,
	NEST_SER_FROM = 2,
	NEST_SER_SELECT = 4
} NestedQSerLocationFlags;

// struct that stores functions that serialize clauses of a select statement
typedef struct SerializeClausesAPI {
    List *(*serializeQueryOperator) (QueryOperator *q, StringInfo str,
            QueryOperator *parent, FromAttrsContext *fac, struct SerializeClausesAPI *api);
    List *(*serializeQueryBlock) (QueryOperator *q, StringInfo str, FromAttrsContext *fac, struct SerializeClausesAPI *api);
    List * (*serializeProjectionAndAggregation) (QueryBlockMatch *m, StringInfo select,
            StringInfo having, StringInfo groupBy, FromAttrsContext *fac, boolean materialize, struct SerializeClausesAPI *api);
    void (*serializeFrom) (QueryOperator *q, StringInfo from, FromAttrsContext *fac, struct SerializeClausesAPI *api);
    void (*serializeWhere) (SelectionOperator *q, StringInfo where, FromAttrsContext *fac, struct SerializeClausesAPI *api);
    List *(*serializeSetOperator) (QueryOperator *q, StringInfo str, FromAttrsContext *fac, struct SerializeClausesAPI *api);
    void (*serializeFromItem) (QueryOperator *fromRoot, QueryOperator *q, StringInfo from,
            int *curFromItem, int *attrOffset, FromAttrsContext *fac, struct SerializeClausesAPI *api);
    void (*serializeTableAccess) (StringInfo from, TableAccessOperator* t, int* curFromItem,
    			FromAttrsContext *fac, int* attrOffset, struct SerializeClausesAPI *api);
    void (*serializeConstRel) (StringInfo from, ConstRelOperator* t, FromAttrsContext *fac,
            int* curFromItem, struct SerializeClausesAPI *api);
    void (*serializeJoinOperator) (StringInfo from, QueryOperator* fromRoot, JoinOperator* j,
            int* curFromItem, int* attrOffset, FromAttrsContext *fac, struct SerializeClausesAPI *api);
 	void (*serializeOrderByOperator) (OrderOperator *q, StringInfo orderby, FromAttrsContext *fac,
									struct SerializeClausesAPI *api);
	void (*serializeLimitOperator) (LimitOperator *q, StringInfo limit, struct SerializeClausesAPI *api);
	void (*serializePreparedStatment) (QueryOperator *q, StringInfo prep, struct SerializeClausesAPI *api);
    int (*getNestedSerLocations) (NestingOperator *n, struct SerializeClausesAPI *api);
	void (*serializeExecPreparedOperator) (ExecPreparedOperator *q, StringInfo exec);
    List *(*createTempView) (QueryOperator *q, StringInfo str,
            QueryOperator *parent, FromAttrsContext *fac, struct SerializeClausesAPI *api);
    HashMap *tempViewMap;
    int viewCounter;
} SerializeClausesAPI;

/* generic functions for serializing queries that call an API provided as a parameter */
extern char *serLocationsToString(int serloc);
extern SerializeClausesAPI *createAPIStub (void);
extern void genQuoteAttributeNames (Node *q);
extern List *genSerializeQueryOperator (QueryOperator *q, StringInfo str,
        QueryOperator *parent, FromAttrsContext *fac, SerializeClausesAPI *api);
extern List *genSerializeQueryBlock (QueryOperator *q, StringInfo str, FromAttrsContext *fac,
        SerializeClausesAPI *api);
extern int genGetNestedSerializationLocations(NestingOperator *n, struct SerializeClausesAPI *api);
extern void genMarkSubqueriesSerializationLocation(QueryBlockMatch *qbMatch, QueryOperator *op, SerializeClausesAPI *api);
extern void genSerializeFrom (QueryOperator *q, StringInfo from,
		FromAttrsContext *fac, SerializeClausesAPI *api);
extern void genSerializeFromItem (QueryOperator *fromRoot, QueryOperator *q,
        StringInfo from, int *curFromItem, int *attrOffset,FromAttrsContext *fac,
        SerializeClausesAPI *api);
extern void genSerializeWhere (SelectionOperator *q, StringInfo where, FromAttrsContext *fac,
        SerializeClausesAPI *api);
extern void genSerializeLimitOperator (LimitOperator *q, StringInfo limit,
									struct SerializeClausesAPI *api);
extern void genSerializePreparedStatement (QueryOperator *q, StringInfo prep, SerializeClausesAPI *api);
extern void genSerializeExecPreparedOperator (ExecPreparedOperator *q, StringInfo exec);
extern void genSerializeOrderByOperator (OrderOperator *q, StringInfo order,  FromAttrsContext *fac,
									struct SerializeClausesAPI *api);
extern List *genCreateTempView (QueryOperator *q, StringInfo str,
        QueryOperator *parent, FromAttrsContext *fac, SerializeClausesAPI *api);
extern char *exprToSQLWithNamingScheme (Node *expr, int rOffset, FromAttrsContext *fac);
extern boolean updateAggsAndGroupByAttrs(Node *node, UpdateAggAndGroupByAttrState *state);
extern boolean updateAttributeNames(Node *node, FromAttrsContext *fac);
extern boolean updateAttributeNamesSimple(Node *node, List *attrNames);

//for nesting
extern FromAttrsContext *copyFromAttrsContext(FromAttrsContext *fac);
extern void printFromAttrsContext(FromAttrsContext *fac);
extern FromAttrsContext *initializeFromAttrsContext ();

#define UPDATE_ATTR_NAME(cond,expr,falseAttrs,trueAttrs) \
    do { \
        Node *_localExpr = (Node *) (expr); \
        if (m->secondProj == NULL) \
            updateAttributeNames(_localExpr, falseAttrs);	\
        else \
            updateAttributeNamesSimple(_localExpr, trueAttrs); \
    } while(0)


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

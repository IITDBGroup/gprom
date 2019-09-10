#include "config.h"
#ifdef HAVE_LIBJSONPARSER
#include <json.h>
#endif // HAVE_LIBJSONPARSER

// afterwards to not run into problems if boolean is defined
#include "common.h"

#include "mem_manager/mem_mgr.h"
#include "exception/exception.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "model/query_operator/query_operator.h"
#include "metadata_lookup/metadata_lookup_postgres_plans.h"
#include "model/query_operator/query_operator_plan.h"

typedef struct PlanNodeInformation {
    char *nodetype;
    double startup_time;
    double total_time;
    int num_rows;
    int loops;
    uint64_t bytes_per_row;
    List *output_cols;
    uint64_t blocks_read_disk;
    uint64_t blocks_read_buffer;
    json_value **children;
    size_t num_children;
} PlanNodeInformation;

typedef struct NodeTypeToOpType {
	const char *nodeType;
	NodeTag opType;
} NodeTypeToOpType;

// static mapping from plan node types to operator types
static NodeTypeToOpType node_to_op_mapping[] = {
	{
		"Aggregation",
		T_AggregationOperator
	},
	{
		"Seq Scan",
		T_TableAccessOperator
	},
	{
		NULL,
		0
	}
};

#define MEMCONTEXT_NAME "JSON_PLAN_TRANSLATION_CONTEXT"

// global vars
static HashMap *node_to_op = NULL;
static MemContext *mem_context = NULL;
static boolean initialized = FALSE;

#ifdef HAVE_LIBJSONPARSER

#define FIELD_PLAN "Plan"
#define FIELD_PLANS "Plans"
#define FIELD_NODE_TYPE "Node Type"
#define FIELD_STRATEGY "Strategy"
#define FIELD_PARTIAL_MODE "Partial Mode"
#define FIELD_PARALLEL_AWARE "Parallel Aware"
#define FIELD_STARTUP_COST "Startup Cost"
#define FIELD_TOTAL_COST "Total Cost"
#define FIELD_PLAN_ROWS "Plan Rows"
#define FIELD_PLAN_WIDTH "Plan Width"
#define FIELD_ACTUAL_STARTUP_TIME "Actual Startup Time"
#define FIELD_ACTUAL_TOTAL_TIME "Actual Total Time"
#define FIELD_ACTUAL_ROWS "Actual Rows"
#define FIELD_ACTUAL_LOOPS "Actual Loops"
#define FIELD_OUTPUT "Output"
#define FIELD_SHARED_HIT_BLOCKS "Shared Hit Blocks"
#define FIELD_SHARED_READ_BLOCKS "Shared Read Blocks"
#define FIELD_SHARED_DIRTIED_BLOCKS "Shared Dirtied Blocks"
#define FIELD_SHARED_WRITTEN_BLOCKS "Shared Written Blocks"
#define FIELD_LOCAL_HIT_BLOCKS "Local Hit Blocks"
#define FIELD_LOCAL_READ_BLOCKS "Local Read Blocks"
#define FIELD_LOCAL_DIRTIED_BLOCKS "Local Dirtied Blocks"
#define FIELD_LOCAL_WRITTEN_BLOCKS "Local Written Blocks"
#define FIELD_TEMP_READ_BLOCKS "Temp Read Blocks"
#define FIELD_TEMP_WRITTEN_BLOCKS "Temp Written Blocks"
#define FIELD_FILTER "Filter"

// operator specific fields
// aggregation
#define FIELD_GROUP_KEY "Group Key"
#define FIELD_STRATEGY "Strategy"

// relation access
#define FIELD_RELATION_NAME "Relation Name"
#define FIELD_SCHEMA "Schema"
#define FIELD_ALIAS "Alias"

// access fields of particular type
#define GET_INT_FIELD(obj,fld) (getObjectField((obj), (fld)))->u.integer
#define GET_FLOAT_FIELD(obj,fld) (getObjectField((obj), (fld)))->u.dbl
#define GET_BOOL_FIELD(obj,fld) (getObjectField((obj), (fld)))->u.boolean
#define GET_STRING_FIELD(obj,fld) (getObjectField((obj), (fld)))->u.string.ptr

// static functions
static QueryOperator *parsePlanNode(json_value *node, QueryOperator *parent);
static PlanNodeInformation *readCommonPlanNodeInfo(json_value *p);
static void setCommonProperties(QueryOperator *q, PlanNodeInformation *info);
static boolean hasField(json_value *o, const char *field);
static json_value *getObjectField(json_value *o, const char *field);
static NodeTag planNodeToOp(char *plan_node_type);
static QueryOperator *createSelectionFromFilterString(QueryOperator *in, const char *filterCondStr);

/**
 * @brief Translates a JSON query plan produced by Postgres's EXPLAIN command into an RA graph.
 *
 * @param json the JSON document storing the plan
 */
Node *
translateJSONplanToRA (char *json)
{
    json_char *j = (json_char *) json;
    size_t jsize = strlen(json);
    json_value *root;
    json_value *rootPlan;
    Node *result;

	DEBUG_LOG("translate json:\n%s", json);

    // parse document
    root = json_parse(j, jsize);

    if (root == NULL)
        THROW(SEVERITY_RECOVERABLE, "error parsing json plan:\n\n%s", json);

    // extract root plan node
    ASSERT(root->type == json_array);
    rootPlan = getObjectField(root->u.array.values[0], FIELD_PLAN);

    // create algebra tree
    result = (Node *) parsePlanNode(rootPlan, NULL);

    // free up
    json_value_free(root);

    return result;
}

/**
 * @brief      Parses one JSON plan node.
 *
 * @details    Calls itself recursively to parse the node's children.
 *
 * @param      node the plan node to be translated
 * @param      parent the parent for the current operator (or `NULL` if no parent)
 * @return     QueryOperator* returns the root of the translated algebra graph
 */
static QueryOperator *
parsePlanNode(json_value *node, QueryOperator *parent)
{
    PlanNodeInformation *info;
    QueryOperator *result = NULL;
	NodeTag opType;
	List *children = NIL;

    // read common plan node fields and store them in info
    info = readCommonPlanNodeInfo(node);
	opType = planNodeToOp(info->nodetype);

	// first parse children (may need data to determine schema and other things)
	for(int i = 0; i < info->num_children; i++)
	{
		json_value *childJson = info->children[i];
		QueryOperator *child;

		child = parsePlanNode(childJson, result);
		children = appendToTailOfList(children, child);
	}

    // create operator
	switch(opType)
	{
	case T_TableAccessOperator:
	{
		char *table_name = strdup(GET_STRING_FIELD(node, FIELD_RELATION_NAME));
		char *alias = strdup(GET_STRING_FIELD(node, FIELD_ALIAS));
		DEBUG_LOG("PARSED PLAN OP: table access: %s, %s", table_name, alias);
		result = (QueryOperator *) createTableAccessOp(table_name, NULL, alias, singleton(parent), info->output_cols, NIL);
	}
	break;
	case T_AggregationOperator:
	{
		//result = createAggregationOp(List *aggrs, List *groupBy, , singleton(parent), attrNames);
	}
	break;
	default:
		result = NULL;
	}

	// deal with filters
	if (opType == T_TableAccessOperator || opType == T_AggregationOperator)
	{
		if(hasField(node, FIELD_FILTER))
		{
			//TODO create function that creates a selection from the filter string
			result = createSelectionFromFilterString(result, getObjectField(node, FIELD_FILTER));
		}
	}

    // register with parent if exists
	//TODO determine how to connect children here
    if (parent != NULL)
        addChildOperator(parent, result);

	// set common plan properties based on info
	setCommonProperties(result, info);

    return result;
}

/**
 * @brief      Create a selection operator from a filter condition (string)
 *
 * @param      in the query operator that will be the child of the new selection op
 * @param      filterCondStr the filter condition
 *
 * @return     the selection operator created from the filter condition
 */
static QueryOperator *
createSelectionFromFilterString(QueryOperator *in, const char *filterCondStr)
{
	QueryOperator *result;
	Node *cond;
	List *attrNammes;

	//TODO compile string
	cond = (Node *) getHeadOfListP(parseFromStringOracle(filterCondStr));
	attrNames = getAttrNames(in->schema);
	result = createSelectionOp(cond, in, NIL, attrNames);
	in->parents = singleton(result);

	return result;
}


/**
 * @brief      Store plan node information as properties of a query operator.
 *
 * @details    GProM's RA model is not provisioned to store any runtime information. Use the generic property mechanism to store this information.
 *
 * @param      q the translated operator
 * @param      info helper struct storing information extracted from the JSON plan for this operator
 * @return     void
 */
static void
setCommonProperties(QueryOperator *q, PlanNodeInformation *info)
{
	SET_STRING_PROP(q, QO_PLAN_TIME, createConstFloat(info->total_time));
	SET_STRING_PROP(q, QO_PLAN_PHYSICAL_OP, createConstString(info->nodetype));
	SET_STRING_PROP(q, QO_PLAN_LOOPS, createConstInt(info->loops));
	SET_STRING_PROP(q, QO_PLAN_BYTES_PER_ROW, createConstInt(info->bytes_per_row));
	SET_STRING_PROP(q, QO_PLAN_PAGES_READ_FROM_BUFFER, createConstInt(info->blocks_read_buffer));
	SET_STRING_PROP(q, QO_PLAN_PAGES_READ_FROM_DISK, createConstInt(info->blocks_read_disk));
	SET_STRING_PROP(q, QO_PLAN_STARTUP_TIME, createConstFloat(info->startup_time));
}

/**
 * @brief      Extract information from the JSON object representing a plan-node.
 *
 * @details    Returns a datastructure storing the extracted information.
 *
 * @param      p the json object representing the plan
 *
 * @return     PlanNodeInformation
 */
static PlanNodeInformation *
readCommonPlanNodeInfo(json_value *p)
{
    PlanNodeInformation *result = MALLOC(sizeof(PlanNodeInformation));
    json_value *arr;
	size_t arr_len;
	List *outputs = NIL;

    result->blocks_read_buffer = 0;
    result->bytes_per_row = GET_INT_FIELD(p, FIELD_PLAN_WIDTH);
	result->loops = GET_INT_FIELD(p, FIELD_ACTUAL_LOOPS);
	result->nodetype = GET_STRING_FIELD(p, FIELD_NODE_TYPE);
	result->startup_time = GET_FLOAT_FIELD(p, FIELD_ACTUAL_STARTUP_TIME);
	result->total_time = GET_FLOAT_FIELD(p, FIELD_ACTUAL_TOTAL_TIME);

	// get output columns
	arr = getObjectField(p, FIELD_OUTPUT);
	arr_len = arr->u.array.length;

	for(int i = 0; i < arr_len; i++)
	{
		outputs = appendToTailOfList(outputs, arr->u.array.values[i]->u.string.ptr);
	}
	result->output_cols = outputs;

	// read children
	if (hasField(p, FIELD_PLANS))
	{
		arr = getObjectField(p, FIELD_PLANS);
		arr_len = arr->u.array.length;
		result->num_children = arr_len;
		result->children =
			MALLOC(sizeof(json_value *) * result->num_children);
		for (int i = 0; i < arr_len; i++) {
            result->children[i] = arr->u.array.values[i];
		}
	}
	else
	{
		result->num_children = 0;
	}

    return result;
}

static json_value *
getObjectField(json_value *o, const char *field)
{
	ASSERT(o->type == json_object);
	size_t len = o->u.object.length;
	for (int i = 0; i < len; i++)
	{
	    char *nam = o->u.object.values[i].name;
		if (streq(nam, field))
			return o->u.object.values[i].value;
	}
	THROW(SEVERITY_RECOVERABLE, "did not find field %s in JSON object", field);
}

static boolean
hasField(json_value *o, const char *field)
{
	ASSERT(o->type == json_object);
	size_t len = o->u.object.length;
	for (int i = 0; i < len; i++)
	{
	    char *nam = o->u.object.values[i].name;
		if (streq(nam, field))
			return TRUE;
	}
	return FALSE;
}

static NodeTag
planNodeToOp(char *plan_node_type)
{
	// create hashmap if needed
	if (!initialized)
	{
		NEW_AND_ACQUIRE_LONGLIVED_MEMCONTEXT(MEMCONTEXT_NAME);
		mem_context = getCurMemContext();
		initialized = TRUE;
		node_to_op = NEW_MAP(Constant,Constant);

		for (NodeTypeToOpType *m = node_to_op_mapping; m->nodeType != NULL; m++)
		{
			MAP_ADD_STRING_KEY(node_to_op, (char *) strdup((char *) m->nodeType), createConstInt(m->opType));
		}
		RELEASE_MEM_CONTEXT();
	}
	DEBUG_LOG("operator type for %s is %s",
			  plan_node_type,
			  NodeTagToString(INT_VALUE(MAP_GET_STRING(node_to_op, plan_node_type))));
	return (NodeTag) INT_VALUE(MAP_GET_STRING(node_to_op, plan_node_type));
}

#else

Node *
translateJSONplanToRA (char *json)
{
    return NULL;
}

#endif // HAVE_LIBJSONPARSER

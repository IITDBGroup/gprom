/*-----------------------------------------------------------------------------
 *
 * to_dot.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"

#include "mem_manager/mem_mgr.h"
#include "log/logger.h"

#include "model/node/nodetype.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "model/query_operator/query_operator.h"


#define SCRIPT_PREAMBLE "digraph G {\n" \
		"\tnode [fontsize=24];\n"

#define SCRIPT_POST "\n}\n"

#define INNER_PREAMBLE "\n\n{\n" \
        "\tnode [style=filled];\n"
#define INNER_POST "\n}\n"

#define REL_PREAMBLE "\n\n{\n" \
     "\trank=same;\n" \
     "\toderdir=LR;\n"
#define REL_INTER "\n\nedge [style=invis,constraint=true];\n\n"
#define REL_FINAL "}\n\n"

#define GET_OP_ID(result,map,op,curId) \
		do { \
			if (MAP_HAS_LONG_KEY(map, (long) op)) \
			    result = STRING_VALUE(MAP_GET_LONG(map, (long) op)); \
			else \
            { \
			    char *newKey = nextKey(curId); \
			    MAP_ADD_LONG_KEY(map, (long) op, createConstString(newKey)); \
			    result = newKey; \
            } \
		} while(0)

#define GET_EXIST_OP_ID(map,op) (STRING_VALUE(MAP_GET_LONG(map, (long) op)))

static HashMap *opsToDot (StringInfo str, QueryOperator *op, int *curId);
static void leafsToDot (StringInfo str, QueryOperator *op, HashMap *nodeToId, int *curId);
static char *nextKey (int *id);
static void outputOpDefs(StringInfo str, QueryOperator *op, int *curId, HashMap *nodeToId);
static void opToDot(StringInfo str, QueryOperator *op, HashMap *nodeToId, int *curId);
static void gatherLeafs(QueryOperator *op, List **leafs);


static HashMap *
opsToDot (StringInfo str, QueryOperator *op, int *curId)
{
    HashMap *nodeToId = NEW_MAP(Constant,Constant);

    appendStringInfoString(str, INNER_PREAMBLE);

    outputOpDefs(str, op, curId, nodeToId);

    appendStringInfoString(str, INNER_POST);

    return nodeToId;
}

static void
outputOpDefs(StringInfo str, QueryOperator *op, int *curId, HashMap *nodeToId)
{
    // do not output same operator twice
    if (MAP_HAS_LONG_KEY(nodeToId,(long) op))
        return;

    opToDot(str, op, nodeToId, curId);

    FOREACH(QueryOperator,child,op->inputs)
        outputOpDefs(str, child, curId, nodeToId);
}

static void
opToDot(StringInfo str, QueryOperator *op, HashMap *nodeToId, int *curId)
{
    char *opName;

    if (LIST_LENGTH(op->inputs) == 0)
       return;

    GET_OP_ID(opName, nodeToId, op, curId);

    switch(op->type)
    {
        case T_ProjectionOperator:
            appendStringInfo(str, "\t%s [label=\"&#928;\",color=violet];\n",
                    opName);
            break;
        case T_SelectionOperator:
            appendStringInfo(str, "\t%s [label=\"&#963;\",color=orange];\n",
                    opName);
            break;
        case T_AggregationOperator:
            appendStringInfo(str, "\t%s [label=\"&#945;\",color=red];\n",
                    opName);
            break;
        case T_JoinOperator:
            appendStringInfo(str, "\t%s [label=\"|X|\",color=green];\n",
                    opName);
            break;
        case T_SetOperator:
        {
            SetOperator *s = (SetOperator *) op;
            switch(s->setOpType)
            {
                case SETOP_UNION:
                    appendStringInfo(str, "\t%s [label=\"&#8746;\","
                            "color=lightblue2];\n", opName);
                    break;
                case SETOP_INTERSECTION:
                    appendStringInfo(str, "\t%s [label=\"&#8745;\","
                            "color=lightblue2];\n", opName);
                    break;
                case SETOP_DIFFERENCE:
                    appendStringInfo(str, "\t%s [label=\"-\","
                            "color=lightblue2];\n", opName);
                    break;
            }
        }
        break;
        case T_ProvenanceComputation:
            appendStringInfo(str, "\t%s [label=\"P\",color=lightgrey];\n",
                                opName);
            break;
        default:
            FATAL_LOG("unkown op type %s", NodeTagToString(op->type));
            break;
    }

    FOREACH(QueryOperator,p,op->parents)
    {
        char *pName = NULL;
        GET_OP_ID(pName, nodeToId, p, curId);
        appendStringInfo(str, "\t%s -> %s;\n", pName, opName);
    }
}

static void
leafsToDot (StringInfo str, QueryOperator *op, HashMap *nodeToId, int *curId)
{
    List *leafs = NIL;

    // get leaf operators
    gatherLeafs(op, &leafs);
    DEBUG_LOG("leafs:\n%s", operatorToOverviewString((Node *) leafs));

    appendStringInfoString(str, REL_PREAMBLE);

    // output leaf nodes
    FOREACH(QueryOperator,o,leafs)
    {
        char *nodeId = NULL;
        GET_OP_ID(nodeId, nodeToId, o, curId);
        if (o->type == T_TableAccessOperator)
        {
            TableAccessOperator *t = (TableAccessOperator *) o;
            appendStringInfo(str, "\t%s [label=\"%s\",shape=box,color=yellow,"
                    "style=filled];\n", nodeId, t->tableName);
        }
        else if (o->type == T_ConstRelOperator)
        {
//            ConstRelOperator *c = (ConstRelOperator *) o;
            appendStringInfo(str, "\t%s [label=\"%s\",shape=box,"
                    "color=yellow,style=filled];", nodeId, "{}");
        }
    }

    DEBUG_LOG("script created so far:\n%s", str->data);
    DEBUG_LOG("node id map:\n%s", nodeToString(nodeToId));

    // output invisible edges between leaf nodes that enforce order
    appendStringInfoString(str, REL_INTER);

    FOREACH(QueryOperator,o,leafs)
    {
        QueryOperator *next = (QueryOperator *) (o_his_cell->next ?
                LC_P_VAL(o_his_cell->next) : NULL);

        if (next)
            appendStringInfo(str, "%s -> %s;\n",
                    GET_EXIST_OP_ID(nodeToId,o),
                    GET_EXIST_OP_ID(nodeToId,next));
    }

    // output edges between intermediate nodes and leafs
    appendStringInfoString(str, REL_FINAL);

    FOREACH(QueryOperator,l,leafs)
    {
        char *nodeId = GET_EXIST_OP_ID(nodeToId,l);
        FOREACH(QueryOperator,p,l->parents)
            appendStringInfo(str, "%s -> %s;\n", GET_EXIST_OP_ID(nodeToId,p), nodeId);
    }
}

static char *
nextKey (int *id)
{
    int res = *id;
    (*id)++;
    return CONCAT_STRINGS("opNode", itoa(res));
}

static void
gatherLeafs(QueryOperator *op, List **leafs)
{
    if (LIST_LENGTH(op->inputs) == 0)
        *leafs = appendToTailOfList(*leafs, op);
    else
        FOREACH(QueryOperator,child,op->inputs)
            gatherLeafs(child, leafs);
}

char *
nodeToDot(void *obj)
{
    StringInfo script = makeStringInfo();
    char *result;
    int id = 0;
    HashMap *nodeToId;
    List *graphs = NIL;
    ASSERT(IS_OP(obj) || isA(obj,List));

    if (isA(obj,List))
        graphs = (List *) obj;
    else
        graphs = singleton(obj);

    script = makeStringInfo();

    FOREACH(QueryOperator,g,graphs)
    {
        appendStringInfoString(script, SCRIPT_PREAMBLE);

        nodeToId = opsToDot(script, g, &id);
        DEBUG_LOG("script so far: %s", script->data);

        leafsToDot(script, g, nodeToId, &id);
        DEBUG_LOG("script so far: %s", script->data);

        appendStringInfoString(script, SCRIPT_POST);
    }

    INFO_LOG("script:\n %s", script->data);
    result = script->data;
    FREE(script);
    return result;
}

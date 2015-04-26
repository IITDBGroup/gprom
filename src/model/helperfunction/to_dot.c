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
#include "model/set/set.h"
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

#define NODE_ID_PROP "NODE_ID_FOR_DOT"

#define GET_OP_ID(result,map,op) \
		do { \
		    result = STRING_VALUE(GET_STRING_PROP(op, NODE_ID_PROP)); \
			if (!hasSetLongElem(map, (long) op)) \
			    addLongToSet(map, (long) op); \
		} while(0)

#define GET_EXIST_OP_ID(op) STRING_VALUE((GET_STRING_PROP(op, NODE_ID_PROP)))

static void opsToDot (StringInfo str, QueryOperator *op, int *curId);
static void determineOpIds(QueryOperator *op, int *curId);
static void leafsToDot (StringInfo str, QueryOperator *op);
static char *nextKey (int *id);
static void outputOpDefs(StringInfo str, QueryOperator *op, Set *nodeDone);
static void opToDot(StringInfo str, QueryOperator *op, Set *nodeDone);
static void outputEdges(StringInfo str, QueryOperator *op, Set *nodeDone);
static void gatherLeafs(QueryOperator *op, List **leafs);
static void removeNodeIdProp (QueryOperator *op);


static void
opsToDot (StringInfo str, QueryOperator *op, int *curId)
{
    appendStringInfoString(str, INNER_PREAMBLE);

    // set node ids as properties to nodes and create code for nodes except leaf nodes
    determineOpIds(op, curId);
    DEBUG_LOG("setting op ids: %s", beatify(nodeToString((Node *) op)));

    Set *nodeDone = LONGSET();
    outputOpDefs(str, op, nodeDone);

    // output edges except edges to leaf nodes
    nodeDone = LONGSET();
    outputEdges(str ,op, nodeDone);

    appendStringInfoString(str, INNER_POST);
}

static void
outputOpDefs(StringInfo str, QueryOperator *op, Set *nodeDone)
{
    // do not output same operator twice
    if (hasSetLongElem(nodeDone, (long) op))
        return;

    opToDot(str, op, nodeDone);

    FOREACH(QueryOperator,child,op->inputs)
        outputOpDefs(str, child, nodeDone);
}

static void
determineOpIds(QueryOperator *op, int *curId)
{
    // do not output same operator twice
    if (HAS_STRING_PROP(op, NODE_ID_PROP))
        return;

    SET_STRING_PROP(op, NODE_ID_PROP, createConstString(nextKey(curId)));

    FOREACH(QueryOperator,child,op->inputs)
        determineOpIds(child, curId);
}


static void
opToDot(StringInfo str, QueryOperator *op, Set *nodeDone)
{
    char *opName;

    if (LIST_LENGTH(op->inputs) == 0)
       return;

    GET_OP_ID(opName, nodeDone, op);

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
        case T_DuplicateRemoval:
            appendStringInfo(str, "\t%s [label=\"D\",color=lightgrey];\n",
                    opName);
            break;
        default:
            FATAL_LOG("unkown op type %s", NodeTagToString(op->type));
            break;
    }
}

static void
outputEdges(StringInfo str, QueryOperator *op, Set *nodeDone)
{
    char *opId;
    char *cId;

    if (hasSetLongElem(nodeDone, (long) op))
        return;

    // mark operator as processed to avoid outputting the same edges more than once
    addLongToSet(nodeDone, (long) op);
    opId = STRING_VALUE(GET_STRING_PROP(op, NODE_ID_PROP));

    FOREACH(QueryOperator,c,op->inputs)
    {
        // Do not creates edges to leaf operators here
        if (LIST_LENGTH(c->inputs) != 0)
        {
            cId = STRING_VALUE(GET_STRING_PROP(c, NODE_ID_PROP));
            appendStringInfo(str, "\t%s -> %s;\n", opId, cId);
        }
    }

    FOREACH(QueryOperator,c,op->inputs)
        if (LIST_LENGTH(c->inputs) != 0)
            outputEdges(str, c, nodeDone);
}


static void
leafsToDot (StringInfo str, QueryOperator *op)
{
    List *leafs = NIL;
    Set *nodeDone = LONGSET();
    // get leaf operators
    gatherLeafs(op, &leafs);
    DEBUG_LOG("leafs:\n%s", operatorToOverviewString((Node *) leafs));

    appendStringInfoString(str, REL_PREAMBLE);

    // output leaf nodes
    FOREACH(QueryOperator,o,leafs)
    {
        if (!hasSetLongElem(nodeDone, (long) o))
        {
            char *nodeId = STRING_VALUE(GET_STRING_PROP(o, NODE_ID_PROP));
            addLongToSet(nodeDone, (long) o);
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
    }

    DEBUG_LOG("script created so far:\n%s", str->data);

    // output invisible edges between leaf nodes that enforce order
    nodeDone = LONGSET();
    appendStringInfoString(str, REL_INTER);

    FOREACH(QueryOperator,o,leafs)
    {
        if (!hasSetLongElem(nodeDone, (long) o))
        {
            addLongToSet(nodeDone, (long) o);
            QueryOperator *next = (QueryOperator *) (o_his_cell->next ?
                    LC_P_VAL(o_his_cell->next) : NULL);

            if (next)
                appendStringInfo(str, "%s -> %s;\n",
                        GET_EXIST_OP_ID(o),
                        GET_EXIST_OP_ID(next));
        }
    }

    // output edges between intermediate nodes and leafs
    nodeDone = LONGSET();
    appendStringInfoString(str, REL_FINAL);

    FOREACH(QueryOperator,l,leafs)
    {
        if (!hasSetLongElem(nodeDone, (long) l))
       {
            addLongToSet(nodeDone, (long) l);
            char *nodeId = GET_EXIST_OP_ID(l);
            FOREACH(QueryOperator,p,l->parents)
                appendStringInfo(str, "%s -> %s;\n", GET_EXIST_OP_ID(p), nodeId);
       }
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

        opsToDot(script, g, &id);
        DEBUG_LOG("script so far: %s", script->data);

        leafsToDot(script, g);
        DEBUG_LOG("script so far: %s", script->data);

        appendStringInfoString(script, SCRIPT_POST);

        removeNodeIdProp(g);
    }



    INFO_LOG("script:\n %s", script->data);
    result = script->data;
    FREE(script);
    return result;
}

static void
removeNodeIdProp (QueryOperator *op)
{
    removeStringProperty(op,NODE_ID_PROP);
    FOREACH(QueryOperator,c,op->inputs)
        removeNodeIdProp(c);
}

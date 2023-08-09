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
#include "configuration/option.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"
#include "model/query_operator/query_operator.h"


#define SCRIPT_PREAMBLE "digraph G {\n" \
		"\tnode [fontsize=24];\n" \
        "d2tdocpreamble=\"\\def\\ojoin{\\setbox0=\\hbox{$\\bowtie$}%\n" \
        "  \\rule[-.02ex]{.25em}{.4pt}\\llap{\\rule[\\ht0]{.25em}{.4pt}}}\n" \
        "\\def\\leftouterjoin{\\mathbin{\\ojoin\\mkern-5.8mu\\bowtie}}\n" \
        "\\def\\rightouterjoin{\\mathbin{\\bowtie\\mkern-5.8mu\\ojoin}}\n" \
        "\\def\\fullouterjoin{\\mathbin{\\ojoin\\mkern-5.8mu\\bowtie\\mkern-5.8mu\\ojoin}}\n" \
        "\";\n"

#define SCRIPT_POST "\n}\n"

#define INNER_PREAMBLE "\n\n{\n" \
        "\tnode [style=filled,shape=box];\n"
#define INNER_POST "\n}\n"

#define REL_PREAMBLE "\n\n{\n" \
     "\trank=same;\n" \
     "\toderdir=LR;\n"
#define REL_INTER "\n\nedge [style=invis,constraint=true];\n\n"
#define REL_FINAL "}\n\n"

#define NODE_ID_PROP "NODE_ID_FOR_DOT"

#define COLOR_DARK_GREEN    "\"#D12138\""
#define COLOR_LIGHT_GREEN   "\"#FFC8CC\""
#define COLOR_DARK_RED      "\"#0AB20A\""
#define COLOR_LIGHT_RED     "\"#D9FFD9\""
#define COLOR_DARK_BLUE     "\"#005493\""
#define COLOR_LIGHT_BLUE    "\"#D9D9FF\""
#define COLOR_BLACK         "\"#000000\""
#define COLOR_LIGHT_GREY    "\"#EEEEEE\""
#define COLOR_DARK_YELLOW   "\"#FCC200\""
#define COLOR_LIGHT_YELLOW  "\"#FFE3A8\""
#define COLOR_DARK_PURPLE   "\"#800180\""
#define COLOR_LIGHT_PURPLE  "\"#D5CBFF\""
#define COLOR_DARK_BROWN    "\"#800180\"" //TODO
#define COLOR_LIGHT_BROWN   "\"#D5CBFF\""

#define GET_OP_ID(result,map,op) \
		do { \
		    result = STRING_VALUE(GET_STRING_PROP(op, NODE_ID_PROP)); \
			if (!hasSetLongElem(map, (gprom_long_t) op)) \
			    addLongToSet(map, (gprom_long_t) op); \
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
    DEBUG_NODE_BEATIFY_LOG("setting op ids:", op);

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
    if (hasSetLongElem(nodeDone, (gprom_long_t) op))
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
    boolean showParameters = getBoolOption(OPTION_GRAPHVIZ_DETAILS);
    if (LIST_LENGTH(op->inputs) == 0)
       return;

    GET_OP_ID(opName, nodeDone, op);

    switch(op->type)
    {
        case T_ProjectionOperator:
        {
            ProjectionOperator *p = (ProjectionOperator *) op;

            if (showParameters)
            {
                StringInfo latexLabel = makeStringInfo();

                FORBOTH(Node,proj,a,p->projExprs,op->schema->attrDefs)
                {
                    AttributeDef *def = (AttributeDef *) a;
                    appendStringInfo(latexLabel, "%s \\to %s%s",
                            exprToLatex(proj),
                            latexEscapeString(def->attrName),
                            FOREACH_HAS_MORE(proj) ? "," : "");
                }
                appendStringInfo(str, "\t%s [label=\"&#928;\",color="
                        COLOR_DARK_PURPLE ",fillcolor="
                        COLOR_LIGHT_PURPLE ",texlbl=\"$\\pi_{%s}$\"];\n",
                        opName, latexLabel->data);
            }
            else
                appendStringInfo(str, "\t%s [label=\"&#928;\",color="
                        COLOR_DARK_PURPLE ",fillcolor="
                        COLOR_LIGHT_PURPLE ",texlbl=\"$\\pi$\"];\n",
                                    opName);
        }
            break;
        case T_SelectionOperator:
        {
            SelectionOperator *sel = (SelectionOperator *) op;

            if (showParameters)
            {
                char *latexLabel = exprToLatex((Node *) sel->cond);
                appendStringInfo(str, "\t%s [label=\"&#963;\",color="
                        COLOR_DARK_GREEN ",fillcolor="
                        COLOR_LIGHT_GREEN ",texlbl=\"$\\sigma_{%s}$\"];\n",
                        opName, latexLabel);
            }
            else
                appendStringInfo(str, "\t%s [label=\"&#963;\",color="
                        COLOR_DARK_GREEN ",fillcolor="
                        COLOR_LIGHT_GREEN ",texlbl=\"$\\sigma$\"];\n",
                        opName);
        }
            break;
        case T_AggregationOperator:
        {
            AggregationOperator *a = (AggregationOperator *) op;

            if (showParameters)
            {
                StringInfo aggLabel = makeStringInfo ();
                StringInfo groupLabel = makeStringInfo ();

                FOREACH(Node,agg,a->aggrs)
                {
                    appendStringInfo(aggLabel, "%s%s",
                            exprToLatex(agg),
                            FOREACH_HAS_MORE(agg) ? "," : "");
                }

                FOREACH(Node,g,a->groupBy)
                {
                    appendStringInfo(groupLabel, "%s%s",
                            exprToLatex(g),
                            FOREACH_HAS_MORE(g) ? "," : "");
                }

                appendStringInfo(str, "\t%s [label=\"&#945;\",color="
                        COLOR_DARK_RED ",fillcolor="
                        COLOR_LIGHT_RED ",texlbl=\"$_{%s}\\alpha_{%s}$\"];\n",
                        opName, groupLabel->data, aggLabel->data);
            }
            else
                appendStringInfo(str, "\t%s [label=\"&#945;\",color="
                        COLOR_DARK_RED ",fillcolor="
                        COLOR_LIGHT_RED ",texlbl=\"$\\alpha$\"];\n",
                        opName);
        }
            break;
        case T_JoinOperator:
        {
            JoinOperator *j = (JoinOperator *) op;
            char *joinSymbol;

            switch(j->joinType)
            {
                case JOIN_INNER:
                    joinSymbol = "\\bowtie";
                    break;
                case JOIN_CROSS:
                    joinSymbol = "\\times";
                    break;
                case JOIN_LEFT_OUTER:
                    joinSymbol = "\\leftouterjoin";
                    break;
                case JOIN_RIGHT_OUTER:
                    joinSymbol = "\\rightouterjoin";
                    break;
                case JOIN_FULL_OUTER:
                    joinSymbol = "\\fullouterjoin";
                    break;
	        default:
		    joinSymbol = "";
		    break;
            }

            if (showParameters)
            {
                char *latexLabel = exprToLatex((Node *) j->cond);
                appendStringInfo(str, "\t%s [label=\"|X|\",color="
                        COLOR_DARK_YELLOW ",fillcolor="
                        COLOR_LIGHT_YELLOW ",texlbl=\"$%s_{%s}$\"];\n",
                        opName, joinSymbol, latexLabel);
            }
            else
                appendStringInfo(str, "\t%s [label=\"|X|\",color="
                        COLOR_DARK_YELLOW ",fillcolor="
                        COLOR_LIGHT_YELLOW ",texlbl=\"$%s$\"];\n",
                        opName, joinSymbol);
        }
            break;
        case T_SetOperator:
        {
            SetOperator *s = (SetOperator *) op;
            switch(s->setOpType)
            {
                case SETOP_UNION:
                    appendStringInfo(str, "\t%s [label=\"&#8746;\","
                            "color=" COLOR_DARK_BLUE ",fillcolor="
                            COLOR_LIGHT_BLUE ",texlbl=\"$\\cup$\"];\n",
                            opName);
                    break;
                case SETOP_INTERSECTION:
                    appendStringInfo(str, "\t%s [label=\"&#8745;\","
                            "color=" COLOR_DARK_BLUE ",fillcolor="
                            COLOR_LIGHT_BLUE ",texlbl=\"$\\cap$\"];\n",
                            opName);
                    break;
                case SETOP_DIFFERENCE:
                    appendStringInfo(str, "\t%s [label=\"-\","
                            "color=" COLOR_DARK_BLUE ",fillcolor="
                            COLOR_LIGHT_BLUE ",texlbl=\"$-$\"];\n",
                            opName);
                    break;
            }
        }
        break;
        case T_RecursiveOperator:
        {
            appendStringInfo(str, "\t%s [label=\"&#964;\",color="
                    COLOR_DARK_PURPLE ",fillcolor="
                    COLOR_LIGHT_PURPLE ",texlbl=\"$\\mu$\"];\n",
                    opName);
        }
            break;
        case T_SplitOperator:
        {
            appendStringInfo(str, "\t%s [label=\"&#963;\",color="
                    COLOR_DARK_GREEN ",fillcolor="
                    COLOR_LIGHT_GREEN ",texlbl=\"$\\sigma$\"];\n",
                    opName);
        }
            break;
        case T_ProvenanceComputation:
            appendStringInfo(str, "\t%s [label=\"P\",color="
                    COLOR_BLACK ",fillcolor="
                    COLOR_LIGHT_GREY ",texlbl=\"${\\cal P}$\"];\n",
                    opName);
            break;
        case T_DuplicateRemoval:
            appendStringInfo(str, "\t%s [label=\"&#948;\",color="
                    COLOR_DARK_PURPLE ",fillcolor="
                    COLOR_LIGHT_PURPLE ",texlbl=\"$\\delta$\"];\n",
                    opName);
            break;
        case T_WindowOperator:
        {
            WindowOperator *w = (WindowOperator *) (op);

            if (showParameters)
            {
                StringInfo partLabel = makeStringInfo();
                StringInfo orderLabel = makeStringInfo();
                StringInfo fLabel = makeStringInfo();

                FOREACH(Node,p,w->partitionBy)
                {
                    appendStringInfo(partLabel, "%s%s",
                            exprToLatex(p),
                            FOREACH_HAS_MORE(p) ? "," : "");
                }

                FOREACH(Node,o,w->orderBy)
                {
                    appendStringInfo(orderLabel, "%s%s",
                            exprToLatex(o),
                            FOREACH_HAS_MORE(o) ? "," : "");
                }

                appendStringInfo(fLabel, "%s", exprToLatex(w->f));

                appendStringInfo(str, "\t%s [label=\"&#945;\",color="
                        COLOR_DARK_RED ",fillcolor="
                        COLOR_LIGHT_RED ",texlbl=\"$_{%s}^{%s}\\omega_{%s}$\"];\n",
                        opName,
                        strdup(partLabel->data),
                        strdup(orderLabel->data),
                        strdup(fLabel->data));
            }
            else
            {
                appendStringInfo(str, "\t%s [label=\"&#945;\",color="
                          COLOR_DARK_RED ",fillcolor="
                          COLOR_LIGHT_RED ",texlbl=\"$\\omega$\"];\n",
                          opName);
            }
        }
        break;
        case T_OrderOperator:
        {
            OrderOperator *o = (OrderOperator *) op;

            if (showParameters)
            {
                StringInfo orderLabel = makeStringInfo();

                FOREACH(Node,p,o->orderExprs)
                {
                    appendStringInfo(orderLabel, "%s%s",
                            exprToLatex(p),
                            FOREACH_HAS_MORE(p) ? "," : "");
                }

                appendStringInfo(str, "\t%s [label=\"&#945;\",color="
                        COLOR_DARK_BROWN ",fillcolor="
                        COLOR_LIGHT_BROWN ",texlbl=\"$\\omicron_{%s}$\"];\n",
                        opName,
                        strdup(orderLabel->data));
            }
            else
            {
                appendStringInfo(str, "\t%s [label=\"&#945;\",color="
                        COLOR_DARK_BROWN ",fillcolor="
                        COLOR_LIGHT_BROWN ",texlbl=\"$\\omicron$\"];\n",
                        opName);
            }
        }
        break;
	    case T_LimitOperator:
        {
            LimitOperator *o = (LimitOperator *) op;

            if (showParameters)
            {
                StringInfo limitLabel = makeStringInfo();

				if(o->limitExpr != NULL)
					appendStringInfo(limitLabel, "limit: %s",
									 exprToLatex(o->limitExpr));

				if(o->offsetExpr != NULL)
					appendStringInfo(limitLabel, "limit: %s",
									 exprToLatex(o->offsetExpr));

                appendStringInfo(str, "\t%s [label=\"&#945;\",color="
                        COLOR_DARK_BROWN ",fillcolor="
                        COLOR_LIGHT_BROWN ",texlbl=\"$\\zeta_{%s}$\"];\n",
                        opName,
								 strdup(limitLabel->data));
            }
            else
            {
                appendStringInfo(str, "\t%s [label=\"&#945;\",color="
                        COLOR_DARK_BROWN ",fillcolor="
                        COLOR_LIGHT_BROWN ",texlbl=\"$\\zeta$\"];\n",
                        opName);
            }
        }
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

    if (hasSetLongElem(nodeDone, (gprom_long_t) op))
        return;

    // mark operator as processed to avoid outputting the same edges more than once
    addLongToSet(nodeDone, (gprom_long_t) op);
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
        if (!hasSetLongElem(nodeDone, (gprom_long_t) o))
        {
            char *nodeId = STRING_VALUE(GET_STRING_PROP(o, NODE_ID_PROP));
            addLongToSet(nodeDone, (gprom_long_t) o);
            if (o->type == T_TableAccessOperator)
            {
                TableAccessOperator *t = (TableAccessOperator *) o;
                appendStringInfo(str, "\t%s [label=\"%s\",shape=box,color=" COLOR_BLACK ",fillcolor=" COLOR_LIGHT_GREY
                        "style=filled];\n", nodeId, t->tableName);
            }
            else if (o->type == T_ConstRelOperator)
            {
                ConstRelOperator *c = (ConstRelOperator *) o;
                StringInfo latexLabel = makeStringInfo();

                FORBOTH(Node,v,a,c->values,op->schema->attrDefs)
                {
                    AttributeDef *def = (AttributeDef *) a;
                    appendStringInfo(latexLabel, "%s \\to %s%s",
                            exprToLatex(v),
                            latexEscapeString(def->attrName),
                            FOREACH_HAS_MORE(v) ? "," : "");
                }
                appendStringInfo(str, "\t%s [label=\"%s\",texlbl=\"%s\",shape=box,"
                        "color=" COLOR_BLACK ",fillcolor=" COLOR_LIGHT_GREY ",style=filled];", nodeId, "{}", strdup(latexLabel->data));
            }
        }
    }

    DEBUG_LOG("script created so far:\n%s", str->data);

    // output invisible edges between leaf nodes that enforce order
    nodeDone = LONGSET();
    appendStringInfoString(str, REL_INTER);

    FOREACH(QueryOperator,o,leafs)
    {
        if (!hasSetLongElem(nodeDone, (gprom_long_t) o))
        {
            addLongToSet(nodeDone, (gprom_long_t) o);
            QueryOperator *next = (QueryOperator *) (o_his_cell->next ?
                    LC_P_VAL(o_his_cell->next) : NULL);

            if (next)
            {
                if (!hasSetLongElem(nodeDone, (gprom_long_t) next))
                    appendStringInfo(str, "%s -> %s;\n",
                            GET_EXIST_OP_ID(o),
                            GET_EXIST_OP_ID(next));
            }
        }
    }

    // output edges between intermediate nodes and leafs
    nodeDone = LONGSET();
    appendStringInfoString(str, REL_FINAL);

    FOREACH(QueryOperator,l,leafs)
    {
        if (!hasSetLongElem(nodeDone, (gprom_long_t) l))
       {
            addLongToSet(nodeDone, (gprom_long_t) l);
            char *nodeId = GET_EXIST_OP_ID(l);
            FOREACH(QueryOperator,p,l->parents)
            {
                char *pId = GET_EXIST_OP_ID(p);
                appendStringInfo(str, "%s -> %s;\n", pId, nodeId);
            }
       }
    }
}

static char *
nextKey (int *id)
{
    int res = *id;
    (*id)++;
    return CONCAT_STRINGS("opNode", gprom_itoa(res));
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

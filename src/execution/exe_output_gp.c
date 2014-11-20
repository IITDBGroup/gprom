/*-----------------------------------------------------------------------------
 *
 * exe_output_gp.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"

#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "utility/string_utils.h"
#include "execution/exe_output_gp.h"
#include "model/node/nodetype.h"
#include "model/set/set.h"
#include "model/set/hashmap.h"

#include "metadata_lookup/metadata_lookup.h"

#define DOT_PREFIX "digraph\n" \
     "{\n" \
     "\trankdir=\"TB\""
#define DOT_POSTFIX "\n}\n"

#define DOT_EDGE_TEMP "\t%s -> %s\n"

NEW_ENUM_WITH_TO_STRING(GPNodeType,
        GP_NODE_RULE_WON,
        GP_NODE_RULE_LOST,
        GP_NODE_GOAL_WON,
        GP_NODE_GOAL_LOST,
        GP_NODE_POSREL_WON,
        GP_NODE_POSREL_LOST,
        GP_NODE_NEGREL_WON,
        GP_NODE_NEGREL_LOST,
        GP_NODE_EBD_WON,
        GP_NODE_EBD_LOST
);

static char *nodeTypeLabel[] = {
        "RULEWON",
        "RULELOST",
        "GOALWON",
        "GOALLOST",
        "RELWON",
        "RELLOST",
        "notRELWON",
        "notRELLOST",
        "EDBWON",
        "EDBLOST"
};

//static char *nodeTypeLabelPostfix[] = {
//        "WON",
//        "LOST",
//        "WON",
//        "LOST",
//        "WON",
//        "LOST",
//        "WON",
//        "LOST",
//        "WON",
//        "LOST"
//};

#define WON_COLOR "#CBFFCB"
#define LOST_COLOR "#FF8383"

static char *nodeTypeCode[] = {
        // RULE
        "\n\n\tnode [shape=\"box\", style=filled, color=black, fillcolor=\"" WON_COLOR "\"]\n",
        "\n\n\tnode [shape=\"box\", style=filled, color=black, fillcolor=\"" LOST_COLOR "\"]\n",
        // GOAL
        "\n\n\tnode [shape=\"box\", style=filled, color=black, fillcolor=\"" WON_COLOR "\"]\n",
        "\n\n\tnode [shape=\"box\", style=filled, color=black, fillcolor=\"" LOST_COLOR "\"]\n",
        // REL
        "\n\n\tnode [shape=\"ellipse\", style=filled, color=black, fillcolor=\"" WON_COLOR "\"]\n",
        "\n\n\tnode [shape=\"ellipse\", style=filled, color=black, fillcolor=\"" LOST_COLOR "\"]\n",
        // notREL
        "\n\n\tnode [shape=\"ellipse\", style=filled, color=black, fillcolor=\"" WON_COLOR "\"]\n",
        "\n\n\tnode [shape=\"ellipse\", style=filled, color=black, fillcolor=\"" LOST_COLOR "\"]\n",
        // EDB
        "\n\n\tnode [shape=\"box\", style=filled, color=black, fillcolor=\"" WON_COLOR "\"]\n",
        "\n\n\tnode [shape=\"box\", style=filled, color=black, fillcolor=\"" LOST_COLOR "\"]\n",
};


static char *nodeTypeNodeCode[] = {
        "%s [label=\"%s\", texlbl=\"%s\"]\n",
        "%s [label=\"%s\", texlbl=\"%s\"]\n",
        "%s [label=\"%s\", texlbl=\"%s\"]\n",
        "%s [label=\"%s\", texlbl=\"%s\"]\n",
        "%s [label=\"%s\", texlbl=\"%s\"]\n",
        "%s [label=\"%s\", texlbl=\"%s\"]\n",
        "%s [label=\"%s\", texlbl=\"%s\"]\n",
        "%s [label=\"%s\", texlbl=\"%s\"]\n",
        "%s [label=\"%s\", texlbl=\"%s\"]\n",
        "%s [label=\"%s\", texlbl=\"%s\"]\n",
};

static GPNodeType getNodeType (char *node);
static char *getNodeId (char *node);
static char *getNodeLabel (char *node, GPNodeType t);
static char *getTexNodeLabel (char *node, GPNodeType t);

void
executeOutputGP(void *sql)
{

    StringInfo script = makeStringInfo();
    StringInfo edges = makeStringInfo();
    HashMap *noteTypes = NEW_MAP(Constant,Set);
    List *queryRes;

    for (int i = 0; i < NUM_ELEM_GPNodeType; i++)
        MAP_ADD_INT_KEY(noteTypes, i, STRSET());

    // append pre fix
    appendStringInfoString(script, DOT_PREFIX);

    // execute GP query
    sql = replaceSubstr((char *) sql, ";", "");
    queryRes = executeQuery(sql);

    // loop through query result creating edges and caching nodes
    // add loop
    FOREACH(List,t,queryRes)
    {
        Set *nodes;

        char *lRawId = (char *) getNthOfListP(t,0);
        GPNodeType lType = getNodeType(lRawId);
        char *lId = getNodeId(lRawId);
        nodes = (Set *) MAP_GET_INT(noteTypes, lType);
        addToSet(nodes, lRawId);

        char *rRawId = (char *) getNthOfListP(t,1);
        GPNodeType rType = getNodeType(rRawId);
        char *rId = getNodeId(rRawId);
        nodes = (Set *) MAP_GET_INT(noteTypes, rType);
        addToSet(nodes, rRawId);

        DEBUG_LOG("edge <%s - %s> between nodes of types %s, %s", lRawId, rRawId,
                GPNodeTypeToString(lType), GPNodeTypeToString(rType));

        appendStringInfo(edges, DOT_EDGE_TEMP, lId, rId);
    }

    // for each node type add code to create nodes to script
    FOREACH_HASH_ENTRY(e,noteTypes)
    {
        GPNodeType t  = (GPNodeType) INT_VALUE(e->key);
        char *template = nodeTypeNodeCode[t];
        Set *nodes = (Set *) e->value;

        // add node type settings to script
        appendStringInfoString(script,nodeTypeCode[t]);

        FOREACH_SET(char,n,nodes)
        {
            char *label = getNodeLabel(n,t);
            char *id = getNodeId(n);
            char *texLabel = getTexNodeLabel(n,t);
            DEBUG_LOG("label and id for node: <%s> and <%s>", label, id);
            appendStringInfo(script,template,id,label,texLabel);
        }
    }
    // append post fix
    appendStringInfoString(script, edges->data);
    appendStringInfoString(script, DOT_POSTFIX);

    // keep compiler qiet
    GPNodeType x = stringToGPNodeType("GP_NODE_RULE_WON");
    TRACE_LOG("%u", x);

    // output script
    printf("%s", script->data);
    fflush(stdout);
}

// test prefix and suffix of name it, e.g., RULE_0_WON(...) -> 0_WON(...)

static GPNodeType
getNodeType (char *node)
{
    char *prefix  = strdup(node);
    char *wonLost = strdup(node);
    char *comb;

    prefix = strtok(prefix,"_");
    wonLost = strEndTok(strtok(wonLost,"("), "_");
    comb = CONCAT_STRINGS(prefix,wonLost);

    DEBUG_LOG("name <%s> has prefix <%s> and is won/lost <%s>", node, prefix, wonLost);

    for(int i = 0; i < NUM_ELEM_GPNodeType; i++)
    {
        if (strncmp(comb,nodeTypeLabel[i],strlen(nodeTypeLabel[i])) == 0)
            return (GPNodeType) i;
    }

    FATAL_LOG("unkown node type for node id <%s>", *node);
    return 0;
}

/*
 * return id for the node in the dot script
 */

static char *
getNodeId (char *node)
{
    char *id = strdup(node);
    char *idP = id;

    while(*(idP++) != '\0')
    {
        if (*idP == '(')
            *idP = '_';
        if (*idP == ')')
            *idP = '_';
        if (*idP == ',')
            *idP = '_';
        if (*idP == ' ')
            *idP = '_';
        if (*idP == '.')
            *idP = '_';
        if (*idP == '-')
            *idP = '_';
        if (*idP == '?')
            *idP = '_';
    }

    DEBUG_LOG("node id: <%s>", id);

    return id;
}

static char *
getNodeLabel (char *node, GPNodeType t)
{
    char *id;
    char *args;

    // compile pattern and match to get node id info
    id = getMatchingSubstring(node, "[^_]+[_]([^\\(]+)");
    id = replaceSubstr(id, "_WON", "");
    id = replaceSubstr(id, "_LOST", "");

    args = getMatchingSubstring(node, "[^\\(]+\(\\([^\\)]+\\))");

    switch(t)
    {
        // $r_0(a,1,3)$
        case GP_NODE_RULE_WON:
        case GP_NODE_RULE_LOST:
            return CONCAT_STRINGS("r", id, " ", args);
            break;
        // $g_{i}^{j}(a,1,2)$
        case GP_NODE_GOAL_WON:
        case GP_NODE_GOAL_LOST:
        {
            char *newId = replaceSubstr(id, "_", ",");
            return CONCAT_STRINGS("g[", newId, "]", args);
        }
        case GP_NODE_POSREL_WON:
        case GP_NODE_POSREL_LOST:
        {
            return CONCAT_STRINGS(id, args);
        }
        case GP_NODE_NEGREL_WON:
        case GP_NODE_NEGREL_LOST:
        {
            return CONCAT_STRINGS("not ", id, args);
        }
        case GP_NODE_EBD_WON:
        case GP_NODE_EBD_LOST:
        {
            return CONCAT_STRINGS(id, args);
        }
        break;
    }

    return node; //TODO
}

static char *
getTexNodeLabel (char *node, GPNodeType t)
{
    char *id;
    char *args;

    // compile pattern and match to get node id info
    id = getMatchingSubstring(node, "[^_]+[_]([^\\(]+)");
    id = replaceSubstr(id, "_WON", "");
    id = replaceSubstr(id, "_LOST", "");

    args = getMatchingSubstring(node, "[^\\(]+\(\\([^\\)]+\\))");

    switch(t)
    {
        // $r_0(a,1,3)$
        case GP_NODE_RULE_WON:
        case GP_NODE_RULE_LOST:
            return CONCAT_STRINGS("$r_", id, args, "$");
            break;
        // $g_{i}^{j}(a,1,2)$
        case GP_NODE_GOAL_WON:
        case GP_NODE_GOAL_LOST:
        {
            char *newId = replaceSubstr(id, "_", "}^{");
            return CONCAT_STRINGS("$g_{", newId, "}", args, "$");
        }
        case GP_NODE_POSREL_WON:
        case GP_NODE_POSREL_LOST:
        {
            return CONCAT_STRINGS("$", id, args, "$");
        }
        case GP_NODE_NEGREL_WON:
        case GP_NODE_NEGREL_LOST:
        {
            return CONCAT_STRINGS("$\\neg ", id, args, "$");
        }
        case GP_NODE_EBD_WON:
        case GP_NODE_EBD_LOST:
        {
            return CONCAT_STRINGS("$", id, args, "$");
        }
        break;
    }

    return node; //TODO
}

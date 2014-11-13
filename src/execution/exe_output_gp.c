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
#include <regex.h>

#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
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

#define WON_COLOR "00AA00"
#define LOST_COLOR "AA0000"

static char *nodeTypeCode[] = {
        // RULE
        "\n\n\tnode [shape=\"rectangle, fill=white, draw\", style=\"rounded corners\" color=\"" WON_COLOR "\"]\n",
        "\n\n\tnode [shape=\"rectangle, fill=white, draw\", style=\"rounded corners\" color=\"" LOST_COLOR "\"]\n",
        // GOAL
        "\n\n\tnode [shape=\"rectangle, fill=white, draw\", style=\"rounded corners\" color=\"" WON_COLOR "\"]\n",
        "\n\n\tnode [shape=\"rectangle, fill=white, draw\", style=\"rounded corners\" color=\"" LOST_COLOR "\"]\n",
        // REL
        "\n\n\tnode [shape=\"rectangle, fill=white, draw\", style=\"rounded corners\" color=\"" WON_COLOR "\"]\n",
        "\n\n\tnode [shape=\"rectangle, fill=white, draw\", style=\"rounded corners\" color=\"" LOST_COLOR "\"]\n",
        // notREL
        "\n\n\tnode [shape=\"rectangle, fill=white, draw\", style=\"rounded corners\" color=\"" WON_COLOR "\"]\n",
        "\n\n\tnode [shape=\"rectangle, fill=white, draw\", style=\"rounded corners\" color=\"" LOST_COLOR "\"]\n",
        // EDB
        "\n\n\tnode [shape=\"rectangle, fill=white, draw\", style=\"rounded corners\" color=\"" WON_COLOR "\"]\n",
        "\n\n\tnode [shape=\"rectangle, fill=white, draw\", style=\"rounded corners\" color=\"" LOST_COLOR "\"]\n",
};


static char *nodeTypeNodeCode[] = {
        "%s [texlbl=\"%s\"]\n",
        "%s [texlbl=\"%s\"]\n",
        "%s [texlbl=\"%s\"]\n",
        "%s [texlbl=\"%s\"]\n",
        "%s [texlbl=\"%s\"]\n",
        "%s [texlbl=\"%s\"]\n",
        "%s [texlbl=\"%s\"]\n",
        "%s [texlbl=\"%s\"]\n",
        "%s [texlbl=\"%s\"]\n",
        "%s [texlbl=\"%s\"]\n",
};

static GPNodeType getNodeType (char *node);
static char *getNodeId (char *node);
static char *getNodeLabel (char *node, GPNodeType t);

static char *getMatchingSubstring(const char *string, const char *pattern);
static char *replaceSubstr(char *str, char *pattern, char *repl);
static char *strEndTok(char *string, char *delim);

void
executeOutputGP(void *sql)
{

    StringInfo script = makeStringInfo();
    StringInfo edges = makeStringInfo();
    HashMap *noteTypes = NEW_MAP(Constant,Set);
    List *queryRes;

    for (int i = 0; i < NUM_ELEM_GPNodeType; i++)
        MAP_ADD_INT_KEY(noteTypes, i, NODESET());

    // append pre fix
    appendStringInfoString(script, DOT_PREFIX);

    // execute GP query
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

        char *rRawId = (char *) getNthOfListP(t,0);
        GPNodeType rType = getNodeType(rRawId);
        char *rId = getNodeId(rRawId);
        nodes = (Set *) MAP_GET_INT(noteTypes, rType);
        addToSet(nodes, rRawId);

        TRACE_LOG("edge <%s - %s> between nodes of type", lRawId, rRawId,
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
            appendStringInfo(script,template,id,label);
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
    char *prefix = strtok(node,"_");
    char *wonLost = strEndTok(strtok(node,"("), "_");
    char *comb = CONCAT_STRINGS(prefix,wonLost);

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

    while(*(id++) != '\0')
    {
        if (*id == '(')
            *id = '_';
        if (*id == ')')
            *id = '_';
        if (*id == ',')
            *id = '_';
    }

    return id;
}

static char *
getNodeLabel (char *node, GPNodeType t)
{
    // extract actual name of node
//    regex_t idPat;
//    regex_t argsPat;
//    const int n_matches = 2;
//    regmatch_t m[n_matches];
//    const char *match;
//    int nomatch;
    char *id;
    char *args;

    // compile pattern and match to get node id info
    id = getMatchingSubstring(node, "[^_]+[_]([^\\(]+)");
    args = getMatchingSubstring(node, "[^\\(]+\\(([^\\)]+\\))");

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
            return CONCAT_STRINGS("$\neg", id, args, "$");
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

static char *
getMatchingSubstring(const char *string, const char *pattern)
{
    char *result;
    regex_t p;
    const int n_matches = 2;
    regmatch_t m[n_matches];
    int matchRes;
    int length;

    // compile
    regcomp(&p, pattern, REG_EXTENDED);

    // match
    matchRes = regexec (&p, string, n_matches, m, 0);
    ASSERT(matchRes == 0);

    // return substring
    length = m[1].rm_eo - m[1].rm_so;
    result = MALLOC(length + 1);
    memcpy(result, string + m[1].rm_so, length);
    result[length] = '\0';

    TRACE_LOG("matched <%s> string <%s> with result <%s>", pattern, string, result);

    return result;
}

static char *
strEndTok(char *string, char *delim)
{
    char *result;
    int startPos = strlen(string) - strlen(delim);
    int newLen = -1;

    while(--startPos > 0 && strncmp(string + startPos, delim, strlen(delim)) != 0)
        ;

    if (startPos == 0)
        return NULL;

    newLen = strlen(string) - startPos + 1 + strlen(delim);
    result = MALLOC(newLen);
    memcpy(result,string + startPos + strlen(delim), newLen);
    return result;
}

static char *
replaceSubstr(char *str, char *pattern, char *repl)
{
    StringInfo result = makeStringInfo();
    int patternLen = strlen(pattern);
    while(*str++ != '\0')
    {
        if (strncmp(str, pattern, patternLen) == 0)
            appendStringInfoString(result, repl);
        else
            appendStringInfoChar(result, *str);
    }

    return result->data;
}

/*
 *------------------------------------------------------------------------------
 *
 * test_graph.c - testing the Graph data structure
 *
 *
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2021-10-08
 *        SUBDIR: test/
 *
 *-----------------------------------------------------------------------------
 */


#include "test_main.h"

#include "model/set/set.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"
#include "model/set/set.h"
#include "model/set/hashmap.h"
#include "model/graph/graph.h"

static rc testConstGraph(void);
static rc testGraphReachability(void);
static rc testGraphUpdates(void);
static Set *constSetToStringSet(Set *constSet);

rc
testGraph(void)
{
    RUN_TEST(testConstGraph(), "test const graphs");
	RUN_TEST(testGraphReachability(), "test reachability in graphs");
	RUN_TEST(testGraphUpdates(), "test updates of graphs");

    return PASS;
}

#define ADD_STR_E(g,from,to) addEdge(g,(Node *) createConstString(from),(Node *) createConstString(to))
#define DEL_STR_E(g,from,to) deleteEdge(g,(Node *) createConstString(from),(Node *) createConstString(to))
#define DEL_STR_NODE(g,n) deleteNode(g,(Node *) createConstString(n))
#define ASSERT_NODE(_g,_str) ASSERT_TRUE(hasNode(_g,(Node *) createConstString(_str)), "graph contains node " _str)
#define ASSERT_EDGE(_g,_f,_t) ASSERT_TRUE(hasEdge(_g,(Node *) createConstString(_f), (Node *) createConstString(_t)), "graph contains edge "  _f  " - "  _t)
#define ASSERT_NO_EDGE(_g,_f,_t) ASSERT_TRUE(!hasEdge(_g,(Node *) createConstString(_f), (Node *) createConstString(_t)), "graph contains no edge "  _f  " - "  _t)

static rc
testConstGraph(void)
{
	Graph *g;

	g = createEmptyGraph();

    ASSERT_EQUALS_INT(0, numNodes(g) + numEdges(g), "empty graph");

	ADD_STR_E(g,"a","b");
	ADD_STR_E(g,"a","c");
	ADD_STR_E(g,"c","d");
	ADD_STR_E(g,"d","e");

    ASSERT_EQUALS_INT(5, numNodes(g), "5 nodes");
    ASSERT_EQUALS_INT(4, numEdges(g), "4 edges");

	ASSERT_NODE(g,"a");
	ASSERT_NODE(g,"b");
	ASSERT_NODE(g,"c");
	ASSERT_NODE(g,"d");
	ASSERT_NODE(g,"e");

	ASSERT_EDGE(g,"a","b");
	ASSERT_EDGE(g,"a","c");
	ASSERT_EDGE(g,"c","d");
	ASSERT_EDGE(g,"d","e");
	ASSERT_NO_EDGE(g,"a","e");
	ASSERT_NO_EDGE(g,"b","a");

    return PASS;
}

#define ASSERT_REACHABLE(_g,_f,_t) ASSERT_TRUE(isReachable(_g,(Node *) createConstString(_f), (Node *) createConstString(_t)), "node "  _t  " is reachable from "  _f)
#define ASSERT_NOT_REACHABLE(_g,_f,_t) ASSERT_TRUE(!isReachable(_g,(Node *) createConstString(_f), (Node *) createConstString(_t)), "node "  _t  " is not reachable from "  _f)

#define ASSERT_REACHABLE_FROM(_g,_f, ...) ASSERT_EQUALS_NODE(MAKE_STR_SET( __VA_ARGS__ ), \
															 constSetToStringSet(reachableFrom(_g,(Node *) createConstString(_f))), \
															 "reachable from " _f);

#define ASSERT_DIRECTLY_REACHABLE_FROM(_g,_f, ...) ASSERT_EQUALS_NODE(MAKE_STR_SET( __VA_ARGS__ ), \
															 constSetToStringSet(directlyReachableFrom(_g,(Node *) createConstString(_f))), \
															 "reachable from " _f);


static rc
testGraphReachability(void)
{
	Graph *g;

	g = createEmptyGraph();
	ADD_STR_E(g,"a","b");
	ADD_STR_E(g,"a","c");
	ADD_STR_E(g,"c","d");
	ADD_STR_E(g,"d","e");

	ASSERT_REACHABLE(g,"a","d");
	ASSERT_REACHABLE(g,"a","e");
	ASSERT_REACHABLE(g,"c","e");
	ASSERT_REACHABLE(g,"a","b");

	ASSERT_NOT_REACHABLE(g,"c","a");
	ASSERT_NOT_REACHABLE(g,"e","d");
	ASSERT_NOT_REACHABLE(g,"b","a");
	ASSERT_NOT_REACHABLE(g,"d","b");
	ASSERT_NOT_REACHABLE(g,"a","a");
	ASSERT_NOT_REACHABLE(g,"b","c");
	ASSERT_NOT_REACHABLE(g,"c","c");

	ASSERT_REACHABLE_FROM(g,"a","b","c","d","e");
	ASSERT_REACHABLE_FROM(g,"c","d","e");

	ASSERT_DIRECTLY_REACHABLE_FROM(g,"a","b","c");
	ASSERT_DIRECTLY_REACHABLE_FROM(g,"c","d");

	// graph with cycles
	g = createEmptyGraph();
	ADD_STR_E(g,"a","b");
	ADD_STR_E(g,"b","c");
	ADD_STR_E(g,"c","d");
	ADD_STR_E(g,"c","a");

	ASSERT_REACHABLE(g,"a","a");
	ASSERT_REACHABLE(g,"b","b");
	ASSERT_REACHABLE(g,"c","c");
	ASSERT_REACHABLE(g,"a","d");

	ASSERT_NOT_REACHABLE(g,"d","a");
	ASSERT_NOT_REACHABLE(g,"d","d");
	ASSERT_NOT_REACHABLE(g,"d","b");
	ASSERT_NOT_REACHABLE(g,"d","c");

	ASSERT_REACHABLE_FROM(g,"a","a", "b","c","d");
	ASSERT_REACHABLE_FROM(g,"c","a", "b","c","d");

	ASSERT_DIRECTLY_REACHABLE_FROM(g,"a","b");
	ASSERT_DIRECTLY_REACHABLE_FROM(g,"c","d","a");

	return PASS;
}

static rc
testGraphUpdates(void)
{
	Graph *g;

	g = createEmptyGraph();
	ADD_STR_E(g,"a","b");
	ADD_STR_E(g,"a","c");
	ADD_STR_E(g,"c","d");
	ADD_STR_E(g,"c","e");

	ASSERT_EDGE(g,"c","d");
	ASSERT_EQUALS_INT(numNodes(g), 5, "graph has 5 nodes");
	ASSERT_EQUALS_INT(numEdges(g), 4, "graph has 4 edges");

    DEL_STR_E(g, "c", "d");

	ASSERT_EQUALS_INT(numNodes(g), 5, "graph has 5 nodes");
	ASSERT_EQUALS_INT(numEdges(g), 3, "graph has 3 edges");
	ASSERT_EDGE(g,"c","e");
	ASSERT_NO_EDGE(g,"c","d");

	DEL_STR_NODE(g, "c");
	ASSERT_EQUALS_INT(numNodes(g), 4, "graph has 4 nodes");
	ASSERT_EQUALS_INT(numEdges(g), 1, "graph has 1 edge");
	ASSERT_NO_EDGE(g,"c","e");

	return PASS;
}

static Set *
constSetToStringSet(Set *constSet)
{
	Set *result = STRSET();

	FOREACH_SET(Constant,c,constSet)
	{
		addToSet(result, STRING_VALUE(c));
	}

	return result;
}

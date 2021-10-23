/*
 *------------------------------------------------------------------------------
 *
 * graph.h - Directed graphs with vertices that are GProM Node structures.
 *
 *     Implementation of graphs and standard graph algorithms (for now only
 *     rechability-related, we will implemented more where need be).
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2021-10-01
 *        SUBDIR: include/model/graph/
 *
 *-----------------------------------------------------------------------------
 */

#ifndef _GRAPH_H_
#define _GRAPH_H_

#include "model/set/set.h"
#include "model/set/hashmap.h"

typedef struct Graph {
	NodeTag type;
	Set *nodes;
	HashMap *edges;
} Graph;

extern Graph *createGraph(Set *nodes, List *edges);
extern Graph *createEmptyGraph(void);

extern boolean hasNode(Graph *g, Node *node);
extern boolean hasEdge(Graph *g, Node *from, Node *to);
extern int numNodes(Graph *g);
extern int numEdges(Graph *g);

#define HAS_NODE(_g,_n) hasNode(g,(Node *) _n)
#define HAS_EDGE(_g,_f,_t) hasEdge(g, (Node *) _f, (Node *) _t)

extern void addNode(Graph *g, Node *n);
extern void addEdge(Graph *g, Node *from, Node *to);
extern void deleteNode(Graph *g, Node *n);
extern void deleteEdge(Graph *g, Node *from, Node *to);

#define ADD_NODE(_g, _n) addNode(_g, (Node *) _n)
#define ADD_EDGE(_g, _from, _to) addEdge(_g, (Node *) _from, (Node *) _to)
#define DELETE_NODE(_g, _n) deleteNode(_g, (Node *) _n)
#define DELETE_EDGE(_g, _from, _to) deleteEdge(_g, (Node *) _from, (Node *) _to)

extern boolean isReachable(Graph *g, Node *start, Node *end);
extern Set *directlyReachableFrom(Graph *g, Node *start);
extern Set *reachableFrom(Graph *g, Node *start);
extern Graph *transitiveClosure(Graph *g);
extern Graph *invertEdges(Graph *g);

extern Set *sourceNodes(Graph *g);
extern Set *sinkNodes(Graph *g);

#endif /* _GRAPH_H_ */

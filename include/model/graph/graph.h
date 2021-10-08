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

extern void addEdge(Graph *g, Node *from, Node *to);
extern void deleteNode(Graph *g, Node *n);
extern void deleteEdge(Graph *g, Node *from, Node *to);

extern boolean isReachable(Graph *g, Node *start, Node *end);
extern Set *directlyReachableFrom(Graph *g, Node *start);
extern Set *reachableFrom(Graph *g, Node *start);
extern Graph *transitiveClosure(Graph *g);

#endif /* _GRAPH_H_ */

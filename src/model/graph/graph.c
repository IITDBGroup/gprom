/*
 *------------------------------------------------------------------------------
 *
 * graph.c - graphs of GProM Node data structures.
 *
 *     This implements a graph data structure with vertices that can be abitary
 *     GProM nodes.
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2021-10-01
 *        SUBDIR: src/model/graph/
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "log/logger.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/set/set.h"
#include "model/set/hashmap.h"
#include "model/graph/graph.h"

Graph *
createGraph(Set *nodes, List *edges)
{
	Graph *result = makeNode(Graph);
	HashMap *eds = NEW_MAP(Node,Set);

	result->nodes = nodes;
	result->edges = eds;

	FOREACH(KeyValue,ed,edges)
	{
		addEdge(result, ed->key, ed->value);
	}

	return result;
}

Graph *
createEmptyGraph(void)
{
	Graph *result = makeNode(Graph);

	result->nodes = NODESET();
	result->edges = NEW_MAP(Node,Set);

	return result;
}

void
addNode(Graph *g, Node *n)
{
	if(!hasSetElem(g->nodes, n))
	{
		addToSet(g->nodes, n);
	}
}

void
addEdge(Graph *g, Node *from, Node *to)
{
	Set *adjacent;

	if(!hasSetElem(g->nodes, from))
	{
		addToSet(g->nodes, from);
	}
	if(!hasSetElem(g->nodes, to))
	{
		addToSet(g->nodes, to);
	}
	if(hasMapKey(g->edges, from))
	{
		adjacent = (Set *) getMap(g->edges, from);
	}
	else
	{
		adjacent = NODESET();
	    addToMap(g->edges, from, (Node *) adjacent);
	}
	addToSet(adjacent, to);
}

void
deleteEdge(Graph *g, Node *from, Node *to)
{
	Set *adjacent = (Set *) getMap(g->edges, from);

	removeSetElem(adjacent, to);
}

void
deleteNode(Graph *g, Node *n)
{
	removeSetElem(g->nodes, n);
	removeMapElem(g->edges, n);

	FOREACH_HASH_ENTRY(kv,g->edges)
	{
		Set *adjacent = (Set *) kv->value;

		removeSetElem(adjacent, n);
	}
}

int
numNodes(Graph *g)
{
	return setSize(g->nodes);
}

int
numEdges(Graph *g)
{
	size_t size = 0;

	FOREACH_HASH_ENTRY(kv,g->edges)
	{
		size += setSize((Set *) kv->value);
	}

	return size;
}

boolean
hasNode(Graph *g, Node *node)
{
	return hasSetElem(g->nodes, node);
}

boolean
hasEdge(Graph *g, Node *from, Node *to)
{
	return hasMapKey(g->edges, from)
		&& hasSetElem((Set *) getMap(g->edges, from), to);
}


boolean
isReachable(Graph *g, Node *start, Node *end)
{
	return hasSetElem(reachableFrom(g,start), end);
}

Set *
directlyReachableFrom(Graph *g, Node *start)
{
	Set *result = NODESET();

	if(hasMapKey(g->edges, start))
	{
		Set *adjacent = (Set *) getMap(g->edges, start);
		FOREACH_SET(Node,e,adjacent)
		{
			if(!hasSetElem(result, e))
			{
				addToSet(result, e);
			}
		}
	}

	return result;
}

Set *
reachableFrom(Graph *g, Node *start)
{
	Set *todo = MAKE_NODE_SET(start);
	Set *result = NODESET();

	while(!EMPTY_SET(todo))
	{
		Node *cur = popSet(todo);
	    TRACE_NODE_LOG("Process node", cur);
	    TRACE_NODE_LOG("Reachable set:", result);
		if(!hasSetElem(result, cur) && !equal(cur,start))
		{
			addToSet(result, cur);
		}
		if(hasMapKey(g->edges, cur))
		{
			Set *adjacent = (Set *) getMap(g->edges, cur);
		    TRACE_NODE_LOG("adjacent nodes:", adjacent);
			FOREACH_SET(Node,e,adjacent)
			{
				if(!hasSetElem(result, e))
				{
					addToSet(todo, e);
					addToSet(result, e);
				}
			}
		}
	}

    TRACE_NODE_LOG("All reachable nodes:", result);

	return result;
}

boolean
hasOutgoingEdges(Graph *g, Node *n)
{
	return !EMPTY_SET((Set *) getMap(g->edges, n));
}

Graph *
transitiveClosure(Graph *g)
{
	return g; //TODO
}

Graph *
invertEdges(Graph *g)
{
	Graph *result = createEmptyGraph();

	result->nodes = copyObject(g->nodes);

	FOREACH_HASH_ENTRY(e, g->edges)
	{
		Node *start = e->key;
		Set *adj = (Set *) e->value;

		FOREACH_SET(Node,end,adj)
		{
			addEdge(result, end, start);
		}
	}

	return result;
}

Set *
sourceNodes(Graph *g)
{
	Set *result = copyObject(g->nodes);

	FOREACH_HASH_ENTRY(kv, g->edges)
	{
		Set *ends = (Set *) kv->value;

		result = setDifference(result, ends);
	}

	return result;
}

Set *
sinkNodes(Graph *g)
{
	Set *result = NODESET();

	FOREACH_SET(Node,n,g->nodes)
	{
		if(!hasMapKey(g->edges, n))
		{
			addToSet(result, n);
		}
		else
		{
			Set *ends = (Set *) getMap(g->edges, n);
			if (EMPTY_SET(ends))
			{
				addToSet(result, n);
			}
		}
	}

	return result;
}

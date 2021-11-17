/*
 *------------------------------------------------------------------------------
 *
 * integrity_constraints.c - Integrity constraints and inference for them.
 *
 *     Data structures to represent integrity constraints and implementations of
 *     inference algorithms for them.
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2021-10-01
 *        SUBDIR: src/model/integrity_constraints/
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"
#include "model/list/list.h"
#include "model/set/set.h"
#include "model/set/vector.h"
#include "model/set/hashmap.h"
#include "model/integrity_constraints/integrity_constraints.h"
#include "model/set/vector.h"

static void addIntToSetValue(HashMap *map, char *a, int val);
static void icToStringInternal(Node *n, StringInfo str);
static char *FDtoStr(FD *f);

FD *
createFD(char *table, Set *lhs, Set *rhs)
{
	FD *f = makeNode(FD);

	f->table = table;
	f->lhs = lhs;
	f->rhs = rhs;

	return f;
}

FOdep *
createFODep(List *lhs, List *rhs)
{
	FOdep *result = makeNode(FOdep);

	result->lhs = lhs;
	result->rhs = rhs;

	return result;
}

Set *
attrListToSet(List *attrs)
{
	Set *result = STRSET();

	FOREACH(char,a,attrs)
	{
		addToSet(result, a);
	}

	return result;
}


#define LOG_AC_STATE(message)								\
	do {													\
	    TRACE_LOG("%s\n"									\
				  "Computing attribute closure for %s\n\n"	\
				  "using FDs: %s\n\n"						\
				  "LS: %s\n"								\
				  "RS: %s\n"								\
				  "counter: %s\n"							\
				  "attrList: %s\n"							\
				  "implied attrs: %s (new: %s)\n",			\
				  message,									\
				  beatify(nodeToString(attrs)),				\
				  beatify(nodeToString(fds)),				\
				  beatify(nodeToString(ls)),				\
				  beatify(nodeToString(rs)),				\
				  beatify(nodeToString(counter)),			\
				  beatify(nodeToString(attrList)),			\
				  beatify(nodeToString(implied)),			\
				  beatify(nodeToString(newdepend)));		\
	} while(0)

Set *
attributeClosure(List *fds, Set *attrs, char *table)
{
	// use the linear time algorithm from Computational Problems Related to the Design of Normal Form Relational Schemas. C. Beeri, P.A. Bernstein. ACM Transactions on Database Systems (TODS), 1979.

	/*
	 * DATA STRUCTURES:
	 *  - LS: [{lhs}] LHS of each FD as a set
	 *  - RS: [{rhs}] RHS of each FD as a set
	 *  - IMPLIED: attributes determined to be dependent
	 *  - NEWDEPEND: subset of IMPLIED of attributes yet to be examined
	 *  - COUNTER: [INT] counting the number of LHS attributes of each FD that are not yet in DEPEND
	 *  - ATTRLIST: ATTR -> {FDs} storing FDs that have the attribute on their LHS
	 */
	Set *implied = copyObject(attrs);
	Vector *ls = makeVector(VECTOR_NODE, T_Set);
	Vector *rs = makeVector(VECTOR_NODE, T_Set);
	Vector *counter = makeVector(VECTOR_INT, T_Constant);
	HashMap *attrList = NEW_MAP(Constant, Set);
	Set *newdepend = copyObject(implied);
	int i = 0;

	FOREACH(FD,f,fds)
	{
		int fdpos = i++;
		vecAppendInt(counter, setSize(f->lhs));
		VEC_ADD_NODE(ls, copyObject(f->lhs));
		VEC_ADD_NODE(rs, copyObject(f->rhs));
		FOREACH_SET(char, a, f->lhs)
		{
			addIntToSetValue(attrList, a, fdpos);
		}
	}

	LOG_AC_STATE("Computing AC: INIT STATE:");

	// process attributes in newdepend
	while(!EMPTY_SET(newdepend))
	{
		char *a = (char *) popSet(newdepend);
		Set *aFDs = MAP_HAS_STRING_KEY(attrList, a) ? (Set *) MAP_GET_STRING(attrList, a) : INTSET();

		DEBUG_LOG("Process attr: %s", a);
		LOG_AC_STATE("Computing AC: STATE:");

		// update fd info for FDS that have this node in their LHS
		FOREACH_SET_INT(fd, aFDs)
		{
			incrVecInt(counter, fd, -1);
			// fd fires
			if(getVecInt(counter, fd) == 0)
			{
				Set *rhs = (Set *) getVecNode(rs, fd);
				FOREACH_SET(char,a,rhs)
				{
					if(!hasSetElem(implied, a))
					{
						addToSet(implied, a);
						addToSet(newdepend, a);
					}
				}
			}
		}
	}

	return implied;
}


static void
addIntToSetValue(HashMap *map, char *a, int val)
{
	Set *els;

	if(MAP_HAS_STRING_KEY(map, a))
	{
		els = (Set *) MAP_GET_STRING(map, a);
	}
	else
	{
		els = INTSET();
		MAP_ADD_STRING_KEY(map, a , els);
	}

	addIntToSet(els, val);
}

List *
getFDsForAttributes(List *fds, Set *attrs)
{
	List *result = NIL;
	Set *allAttrs;

	FOREACH(FD,f,fds)
	{
		allAttrs = unionSets(copyObject(f->lhs), f->rhs);
		if(containsSet(allAttrs, attrs))
		{
			result = appendToTailOfList(result, f);
		}
	}

	return result;
}

List *
getFDsForAttributesOnRels(List *fds, Set *attrs, Set *rels)
{
	List *attrFDs = getFDsForAttributes(fds, attrs);
	List *results = NIL;

	FOREACH(FD,f,attrFDs)
	{
		if(hasSetElem(rels, f->table))
		{
			results = appendToTailOfList(results, f);
		}
	}

	return results;
}

List *
normalizeFDs(List *fds)
{
	List *result = NIL;

	FOREACH(FD,f,fds)
	{
		if(overlapsSet(f->lhs, f->rhs))
		{
			f->rhs = setDifference(f->rhs, f->lhs);
		}

		if(setSize(f->rhs) == 1)
		{
			result = appendToTailOfList(result, f);
		}
		else
		{
			FOREACH_SET(char,a,f->rhs)
			{
				result = appendToTailOfList(result,
											createFD(strdup(f->table),
													 copyObject(f->lhs),
													 MAKE_STR_SET(a)));
			}
		}
	}

	return result;
}

char *
icToString(Node *ics)
{
	StringInfo str = makeStringInfo();

	icToStringInternal(ics, str);

	return str->data;
}

static void
icToStringInternal(Node *n, StringInfo str)
{
	if(isA(n,List))
	{
		List *l = (List *) n;

		FOREACH(Node,e,l)
		{
			icToStringInternal(e, str);
			if(FOREACH_HAS_MORE(e))
			{
				appendStringInfoString(str, "\n");
			}
		}
	}
	else if(isA(n,FD))
	{
		appendStringInfoString(str, FDtoStr((FD *) n));
	}
}

static char *
FDtoStr(FD *f)
{
	StringInfo str = makeStringInfo();

	if(f->table)
	{
		appendStringInfo(str,"%s: ",f->table);
	}

	FOREACH_SET(char,a,f->lhs)
	{
		appendStringInfo(str,"%s%s",a,FOREACH_SET_HAS_MORE(a) ? ", " : "");
	}

	appendStringInfoString(str, "->");

	FOREACH_SET(char,a,f->rhs)
	{
		appendStringInfo(str,"%s%s",a,FOREACH_SET_HAS_MORE(a) ? ", " : "");
	}

	return str->data;
}

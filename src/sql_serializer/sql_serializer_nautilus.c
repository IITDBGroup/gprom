/*
 *------------------------------------------------------------------------------
 *
 * sql_serializer_nautilus.c - Functions that translate a query operator graph from GProM that has annotations to store backend plan information into plan expressed in NautilusDB's task langauge.
 *
 *     The input is assumed to be a query operator graph that is generated from a Postgres JSON explain plan (maybe we'll support other systems in the future). We use properties to store plan information like costs and size of intermediate results. We translate such a query graph into the nautilusDB task language.
 *
 *        AUTHOR: lord_pretzel
 *        DATE: 2019-08-27
 *        SUBDIR: src/sql_serializer/
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"

#include "exception/exception.h"
#include "sql_serializer/sql_serializer_nautilus.h"

static void serializeOpInternal(StringInfo str, QueryOperator *o);

/**
 * @brief      Translate a query into a NautilusDB task graph
 *
 * @param      q the root of the query to be translated
 *
 * @return     the NautilusDB task description (string)
 */
char *
serializeOperatorModelNautilus(Node *q)
{

    if(IS_OP(q))
		return serializeQueryNautilus((QueryOperator *) q);

	THROW(SEVERITY_RECOVERABLE, "can only translate query operator graphs into nautilus data flows");
	return NULL;
}

char *
serializeQueryNautilus(QueryOperator *q)
{
	StringInfo result = makeStringInfo();

	serializeOpInternal(result, q);

	return result->data; //TODO translate into tasks
}

static void
serializeOpInternal(StringInfo str, QueryOperator *o)
{
	//TODO write function

	FOREACH(QueryOperator,c,o->inputs)
	{
		serializeOpInternal(str, c);
	}
}


char *
quoteIdentifierNautilus(char *ident)
{
	return ident;
}

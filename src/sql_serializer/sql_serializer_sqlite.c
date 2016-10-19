/*-----------------------------------------------------------------------------
 *
 * sql_serializer_sqlite.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "instrumentation/timing_instrumentation.h"
#include "mem_manager/mem_mgr.h"

#include "log/logger.h"

#include "sql_serializer/sql_serializer_sqlite.h"
#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"
#include "model/query_operator/operator_property.h"
#include "model/list/list.h"
#include "model/set/set.h"

#include "sql_serializer/sql_serializer_common.h"

char *
serializeOperatorModelSQLite(Node *q)
{
    return "";
}

char *
serializeQuerySQLite(QueryOperator *q)
{
    return "";
}

char *
quoteIdentifierSQLite (char *ident)
{
    int i = 0;
    boolean needsQuotes = FALSE;
    boolean containsQuotes = FALSE;

    // already quoted
    if (ident[0] == '"')
        return ident;

    // sqlite completely ignores case no matter whether the identifier is quoted or not
    // so upper/lower case does not indicate whether we need to escape
    for(i = 0; i < strlen(ident); i++)
    {
        switch(ident[i])
        {
            case '$':
            case '#':
            case '_':
                break;
            default:
                if (ident[i] == ' ')
                    needsQuotes = TRUE;
                if (ident[i] == '"')
                {
                    needsQuotes = TRUE;
                    containsQuotes = TRUE;
                }
                break;
        }
        if (needsQuotes)
            break;
    }

    if (containsQuotes)
        ident = replaceSubstr(ident, "\"", "\"\"");

    if (needsQuotes)
        ident = CONCAT_STRINGS("\"",ident,"\"");

    return ident;
}

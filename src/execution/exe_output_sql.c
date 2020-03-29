/*-----------------------------------------------------------------------------
 *
 * exe_output_sql.c
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "execution/exe_output_sql.h"
#include "configuration/option.h"

void
executeOutputSQL(void *sql)
{
    char *SQLCode = (char *) sql;

    boolean showResult = getBoolOption(OPTION_SHOW_QUERY_RESULT);

	if(showResult)
	{
		printf("%s", SQLCode);
		fflush(stdout);
	}
}

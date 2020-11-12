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

void
executeOutputSQL(void *sql)
{
    char *SQLCode = (char *) sql;

    printf("%s", SQLCode);
    fflush(stdout);
}

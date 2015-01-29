/*-----------------------------------------------------------------------------
 *
 * exe_output_dl.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "execution/exe_output_dl.h"
#include "model/node/nodetype.h"

void
executeOutputDL(void *dl)
{
    char *out;

    out = datalogToOverviewString(dl);
    printf("%s", out);
    fflush(stdout);
}

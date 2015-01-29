/*-----------------------------------------------------------------------------
 *
 * gp_main.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "log/logger.h"

#include "model/datalog/datalog_model.h"

#include "provenance_rewriter/game_provenance/gp_main.h"
#include "provenance_rewriter/game_provenance/gp_bottom_up_program.h"
#include "provenance_rewriter/game_provenance/gp_top_down_program.h"

Node *
rewriteForGP(Node *input)
{

    if (isA(input, DLProgram))
    {
        DLProgram *p = (DLProgram *) input;
        if (IS_GP_PROV(p))
        {
            p = createBottomUpGPprogram(p);
            //TODO TopDown decide based on option
        }
        INFO_LOG("program for compute GP is:\n%s",
                datalogToOverviewString((Node *) p));
        return (Node *) p;
    }
    else
        FATAL_LOG("currently only GP computation for DLPrograms supported.");

    return input;
}

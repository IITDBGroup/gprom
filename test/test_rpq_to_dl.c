/*-----------------------------------------------------------------------------
 *
 * test_rpq_to_dl.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */


#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "instrumentation/timing_instrumentation.h"

#include "log/logger.h"
#include "configuration/option.h"
#include "configuration/option_parser.h"
#include "model/list/list.h"
#include "model/node/nodetype.h"
#include "model/rpq/rpq_model.h"
#include "parser/parser_rpq.h"
#include "rpq/rpq_to_datalog.h"
#include "rpq/rpq_to_sql.h"
#include "model/query_operator/query_operator.h"
#include "metadata_lookup/metadata_lookup.h"
#include "sql_serializer/sql_serializer_dl.h"
#include "execution/executor.h"
#include "rewriter.h"


/* if OCI is not available then add dummy versions */
#if HAVE_A_BACKEND

int
main (int argc, char* argv[])
{
    Regex *rpq;
    Node *dl;
    char *result;
    boolean outSQL = FALSE;

//    printf("%s\n", argv[1]);
    if (streq(argv[1],"-tosql"))
    {
        outSQL = TRUE;
        argv++;
        argc--;
//        printf("%s\n", argv[1]);
    }
//    exit(1);

    READ_OPTIONS_AND_INIT("testrpq", "test binary for regular path query to datalog compilation.");


    // read from terminal
    if (getStringOption("input.sql") != NULL)
    {
        rpq = (Regex *) parseFromStringrpq(getStringOption("input.sql"));
    }
    else if (getStringOption("input.sqlFile") != NULL)
    {
        char *fName = getStringOption("input.sqlFile");
        FILE *file = fopen(fName, "r");

        if (file == NULL)
            FATAL_LOG("could not open file %s with error %s", fName, strerror(errno));

        rpq = (Regex *) parseStreamrpq(file);
        fclose(file);
    }
    // parse input string
    else
    {
        rpq = (Regex *) parseStreamrpq(stdin);
    }
    ERROR_LOG("RPQ INPUT IS <%s>", rpqToShortString(rpq));

    // output datalog
    if (!outSQL)
    {
        // translate to datalog
        dl = rpqToDatalog(rpq);
        ERROR_LOG("Datalog translation is <%s>", datalogToOverviewString(dl));

        // output as string
        result = serializeOperatorModelDL(dl);
        ERROR_LOG("data program is <%s>", result);
        printf("%s", result);
    }
    // output SQL
    else
    {
        result = rpqToSQL(rpq);
        printf("%s", result);
    }
    shutdownApplication();

    return EXIT_SUCCESS;
}



/* if OCI or OCILIB are not avaible replace with dummy test */
#else

int main()
{
    return EXIT_SUCCESS;
}

#endif






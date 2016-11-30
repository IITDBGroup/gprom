/*-----------------------------------------------------------------------------
 *
 * test_common.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "test/test_main.h"

int test_count = 0;
int test_rec_depth = 0;

void
checkResult(int r, char *msg, const char *file, const char *func, int line,
        int tests_passed)
{
    char *indentation = getIndent(test_rec_depth);

    if (tests_passed == -1)
    {
        if (r == PASS)
        {
            printf("%s" T_FG_BG(BLACK,GREEN,"TEST PASS") TBCOL(GREEN,"[%s-%s-%u]:")
                    " %s\n", indentation, file, func, line, msg);
            test_count++;
            free(indentation);
            return;
        }
        else
        {
            printf("%s" T_FG_BG(WHITE,RED,"TEST FAIL") TBCOL(RED,"[%s-%s-%u]:")
                    " %s\n", indentation, file, func,line, msg);
            free(indentation);
            exit(1);
        }
    }
    else
    {
        if (r == PASS)
        {
            printf("%s" T_FG_BG(BLACK,GREEN,"TEST SUITE")
                    TBCOL(GREEN,"[%s-%s-%u]:") " %s - PASSED %u TESTS\n",
                    indentation, file, func, line, msg, tests_passed);
            test_count++;
            free(indentation);
            return;
        }
        else
        {
            printf("%s" T_FG_BG(BLACK,RED,"TEST SUITE FAILED") TBCOL(RED,"[%s-%s-%u]:")
                    " %s AFTER %u TESTS\n",
                    indentation, file, func, line, msg, tests_passed);
            free(indentation);
            exit(1);
        }
    }
}

char *
getIndent(int depth)
{
    char *result = malloc(depth + 1);

    for(int i = 0; i < depth; i++)
        result[i] = '\t';
    result[depth] = '\0';

    return result;
}

boolean
testQuery (char *query, char *expectedResult)
{
    return TRUE;
}

boolean
fileExists (char *file)
{
    FILE *f;
    if ((f = fopen(file, "r")))
    {
        fclose(f);
        return 1;
    }
    return 0;
}



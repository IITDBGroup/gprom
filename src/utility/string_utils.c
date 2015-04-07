/*-----------------------------------------------------------------------------
 *
 * string_utils.c
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
#include "mem_manager/mem_mgr.h"
#include "utility/string_utils.h"
#include <regex.h>
#include "model/node/nodetype.h"

char *
getMatchingSubstring(const char *string, const char *pattern)
{
    char *result;
    regex_t p;
    const int n_matches = 2;
    regmatch_t m[n_matches];
    int matchRes;
    int length;

    // compile
    regcomp(&p, pattern, REG_EXTENDED);

    // match
    matchRes = regexec (&p, string, n_matches, m, 0);
    ASSERT(matchRes == 0);

    // return substring
    length = m[1].rm_eo - m[1].rm_so;
    result = MALLOC(length + 1);
    memcpy(result, string + m[1].rm_so, length);
    result[length] = '\0';

    TRACE_LOG("matched <%s> string <%s> with result <%s>", pattern, string, result);

    return result;
}

char *
strEndTok(char *string, char *delim)
{
    char *result;
    int startPos = strlen(string) - strlen(delim);
    int newLen = -1;

    while(--startPos >= 0 && strncmp(string + startPos, delim, strlen(delim)) != 0)
        ;

    if (startPos == -1)
        return NULL;

    newLen = strlen(string) - startPos + 1 + strlen(delim);
    result = MALLOC(newLen);
    memcpy(result,string + startPos + strlen(delim), newLen);
    return result;
}

char *
replaceSubstr(char *str, char *pattern, char *repl)
{
    StringInfo result = makeStringInfo();
    int patternLen = strlen(pattern);
    do
    {
        if (strncmp(str, pattern, patternLen) == 0)
        {
            str += patternLen - 1;
            if (strlen(repl) > 0)
                appendStringInfoString(result, repl);
        }
        else
            appendStringInfoChar(result, *str);
    } while (*str++ != '\0');

    return result->data;
}

char *
substr(char *str, int from, int to)
{
    ASSERT(from >= 0 && from < to && to < strlen(str));
    int len = to - from  + 1;
    char *result = MALLOC(len + 1);
    memcpy(result, str + from, len);
    result[len] = '\0';
    return result;
}

char *
strRemPostfix(char *str, int postFixSize)
{
    ASSERT(postFixSize < strlen(str));
    int len = strlen(str) - postFixSize;
    char *result = MALLOC(len + 1);
    memcpy(result, str, len);
    result[len] = '\0';
    return result;
}

int
strCompare(const void *a, const void *b)
{
    const char **l = ((const char **) a);
    const char **r = ((const char **) b);

    return strcmp(*l,*r);
}

char *
strToUpper(const char *input)
{
    char *result;

    if (input == NULL)
        return NULL;
    result = strdup((char *) input);

    for(char *p = result; *p != '\0'; p++)
        *p = toupper(*p);

    return result;
}

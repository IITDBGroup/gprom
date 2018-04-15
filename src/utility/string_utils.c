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
#include "model/list/list.h"
#include "utility/string_utils.h"
#include "model/node/nodetype.h"

#ifdef HAVE_REGEX_H

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
getFullMatchingSubstring(const char *string, const char *pattern)
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
    length = m[0].rm_eo - m[0].rm_so;
    result = MALLOC(length + 1);
    memcpy(result, string + m[0].rm_so, length);
    result[length] = '\0';

    TRACE_LOG("matched <%s> string <%s> with result <%s>", pattern, string, result);

    return result;
}

boolean
regExMatch (const char *reg, const char *str)
{
    regex_t regex;
    int reti;
    char msgbuf[100];

    DEBUG_LOG("Match <%s> against <%s>", reg, str);

    reti = regcomp(&regex, reg, 0);
    if (reti)
    {
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        FATAL_LOG("Could not compile regex <%s>:\n%s", reg, msgbuf);
    }

    /* Execute regular expression */
    reti = regexec(&regex, str, 0, NULL, 0);
    if (!reti)
    {
        regfree(&regex);
        return TRUE;
    }
    else if (reti == REG_NOMATCH)
    {
        regfree(&regex);
        return FALSE;
    }
    else
    {
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        FATAL_LOG("Regex match for <%s> on <%s> failed: %s", reg, str, msgbuf);
    }

    return FALSE;
}

#else

char *
getMatchingSubstring(const char *string, const char *pattern)
{
    FATAL_LOG("regex not supported if regex.h not available for your architecture!");

    return NULL;
}

char *
getFullMatchingSubstring(const char *string, const char *pattern)
{
    FATAL_LOG("regex not supported if regex.h not available for your architecture!");

    return NULL;
}

boolean
regExMatch (const char *reg, const char *str)
{
    FATAL_LOG("regex not supported if regex.h not available for your architecture!");

    return FALSE;
}

#endif

#ifndef HAVE_STRSEP

static char*strsep(char** stringp, const char* delim);

static char*
strsep(char** stringp, const char* delim)
{
  char* start = *stringp;
  char* p;

  p = (start != NULL) ? strpbrk(start, delim) : NULL;

  if (p == NULL)
  {
      *stringp = NULL;
  }
  else
  {
      *p = '\0';
      *stringp = p + 1;
  }

  return start;
}

#endif

List *
splitString(char *str, const char *delim)
{
    char* token;
    List *result = NIL;

    if (str == NULL)
        return NIL;
    while ((token = strsep(&str, delim)) != NULL)
        if (*token != '\0')
            result = appendToTailOfList(result, strdup(token));

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
    ASSERT(from >= 0 && from <= to && to < strlen(str));
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

boolean
isPrefix(char *str, char *prefix)
{
    if(str == NULL || prefix == NULL)
        return FALSE;

    if (prefix[0] != str[0])
        return FALSE;

    while(*prefix != '\0' && *prefix++ == *str++)
        ;
    return *prefix == '\0';
}

boolean
isSuffix(char *str, char *suffix)
{
    if(str == NULL || suffix == NULL || strlen(str) < strlen(suffix))
        return FALSE;

    str = str + (strlen(str) - strlen(suffix));
    while(*suffix != '\0')
        if (*suffix++ != *str++)
            return FALSE;

    return TRUE;
}

boolean
isSubstr(char *str, char *substr)
{
    if(str == NULL || substr == NULL || strlen(str) < strlen(substr))
        return FALSE;

    return strstr(str, substr) != NULL;
}

char *
strtrim (char *in)
{
    StringInfo result = makeStringInfo();

    if (in == NULL)
        return NULL;

    while(*in != '\0')
    {
        if (*in != '\t' && *in != ' ' && *in != '\n')
            appendStringInfoChar(result, *in);
        in++;
    }
    return result->data;
}

boolean
strieq(char *left, char *right)
{
    if (left == NULL && right == NULL)
        return TRUE;
    if (left == NULL || right == NULL)
        return FALSE;
    if (strlen(left) != strlen(right))
        return FALSE;
    for(int i = 0; i < strlen(left); i++)
    {
        if (toupper(left[i]) != toupper(right[i]))
            return FALSE;
    }
    return TRUE;
}

char *
specializeTemplate(char *template, List *args)
{
    int i = 1;
    char *result = strdup(template);

    FOREACH(char,arg,args)
    {
        char *var = CONCAT_STRINGS("$",gprom_itoa(i));

        result = replaceSubstr(result,var,arg);
        i++;
    }

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

char *
strToLower(const char *input)
{
    char *result;

    if (input == NULL)
        return NULL;
    result = strdup((char *) input);

    for(char *p = result; *p != '\0'; p++)
        *p = tolower(*p);

    return result;
}

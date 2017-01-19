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

List *
splitString(char *str, const char *delim)
{
    char* token;
//    char **pos = &str;
    List *result = NIL;

    if (str == NULL)
        return NIL;
    while ((token = strsep(&str, delim)) != NULL)
        if (*token != '\0')
            result = appendToTailOfList(result, strdup(token));
//    token = strsep(str, delim, pos);
//
//    while (token)
//    {
//
//        token = strtok_r(str, delim, pos);
//    }

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

char *
specializeTemplate(char *template, List *args)
{
    int i = 1;
    char *result = strdup(template);

    FOREACH(char,arg,args)
    {
        char *var = CONCAT_STRINGS("$",itoa(i));

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

int
strToUpperCmp(char *s1, char *s2)
{
	int i = 0;
	while(s1[i]){
		s1[i] = toupper(s1[i]);
		i++;
	}
	i = 0;
	while(s2[i]){
		s2[i] = toupper(s2[i]);
		i++;
	}
	return strcmp(s1, s2);
}

/*strcmpci function*/
static const unsigned char cicharascii[] = {
	'\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
	'\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
	'\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
	'\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
	'\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
	'\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
	'\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
	'\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
	'\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	'\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
	'\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	'\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
	'\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
	'\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
	'\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
	'\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
	'\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
	'\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
	'\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
	'\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
	'\300', '\301', '\302', '\303', '\304', '\305', '\306', '\307',
	'\310', '\311', '\312', '\313', '\314', '\315', '\316', '\317',
	'\320', '\321', '\322', '\323', '\324', '\325', '\326', '\327',
	'\330', '\331', '\332', '\333', '\334', '\335', '\336', '\337',
	'\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
	'\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
	'\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
	'\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
};

int
strcmpci(char *s1, char *s2)
{
	register const unsigned char *cm = cicharascii, *us1 = (const unsigned char *)s1, *us2 = (const unsigned char *)s2;
	while (cm[*us1] == cm[*us2++])
		if (*us1++ == '\0')
			return (0);
	return (cm[*us1] - cm[*--us2]);
}

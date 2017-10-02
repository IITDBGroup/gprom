/*-----------------------------------------------------------------------------
 *
 * string_utils.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_UTILITY_STRING_UTILS_H_
#define INCLUDE_UTILITY_STRING_UTILS_H_

#include "model/list/list.h"

extern char *getMatchingSubstring(const char *string, const char *pattern);
extern char *getFullMatchingSubstring(const char *string, const char *pattern);
extern List *splitString(char *string, const char *delim);
extern char *strEndTok(char *string, char *delim);
extern char *replaceSubstr(char *str, char *pattern, char *repl);
extern char *substr(char *str, int from, int to);
extern char *strRemPostfix(char *str, int postFixSize);
extern boolean isPrefix(char *str, char *prefix);
extern boolean isSuffix(char *str, char *suffix);
extern boolean isSubstr(char *str, char *substr);
extern char *specializeTemplate(char *template, List *args);
extern char *strtrim (char *in);

// string comparison for sorting
extern int strCompare(const void *a, const void *b);

// string to upper
extern char *strToUpper(const char *input);

extern int strToUpperCmp(char *s1, char *s2);

extern int strcmpci(char *s1, char *s2);

#define streqci(_l,_r) (strcmpci(_l,_r) == 0)
#define strpeqci(_l,_r) (((_l) == (_r)) || ((_l != NULL) && (_r != NULL) && (strcmpci(_l,_r) == 0)))
#define strneqci(_l,_r,n) (strcmpci(_l,_r,n) == 0)

#define streqtuci(_l,_r) (strToUpperCmp(_l,_r) == 0)
#define strpeqtuci(_l,_r) (((_l) == (_r)) || ((_l != NULL) && (_r != NULL) && (strToUpperCmp(_l,_r) == 0)))
#define strneqtuci(_l,_r,n) (strToUpperCmp(_l,_r,n) == 0)

#endif /* INCLUDE_UTILITY_STRING_UTILS_H_ */

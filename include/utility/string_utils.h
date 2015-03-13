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

extern char *getMatchingSubstring(const char *string, const char *pattern);
extern char *strEndTok(char *string, char *delim);
extern char *replaceSubstr(char *str, char *pattern, char *repl);
extern char *substr(char *str, int from, int to);
extern char *strRemPostfix(char *str, int postFixSize);

// string comparison for sorting
extern int strCompare(const void *a, const void *b);

// string to upper
extern char *strToUpper(const char *input);

#endif /* INCLUDE_UTILITY_STRING_UTILS_H_ */

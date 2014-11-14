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


#endif /* INCLUDE_UTILITY_STRING_UTILS_H_ */

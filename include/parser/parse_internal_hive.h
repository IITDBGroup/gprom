/*-----------------------------------------------------------------------------
 *
 * parse_internal_hive.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef PARSE_INTERNAL_HIVE_H_
#define PARSE_INTERNAL_HIVE_H_

/* for storing results */
extern Node *hiveParseResult;

/* interface to the lexer */
extern int hivelineno; /* from lexer */
int hivelex(void);
void hiveerror(char *s);
extern FILE *hivein;
extern void hiveSetupStringInput(char *input);


#endif /* PARSE_INTERNAL_HIVE_H_ */

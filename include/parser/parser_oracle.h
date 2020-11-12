/*-----------------------------------------------------------------------------
 *
 * parser_oracle.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef PARSER_ORACLE_H_
#define PARSER_ORACLE_H_

extern Node *parseStreamOracle (FILE *stream);
extern Node *parseFromStringOracle (char *input);
extern const char *languageHelpOracle(void);

#endif /* PARSER_ORACLE_H_ */

/*-----------------------------------------------------------------------------
 *
 * parser_hive.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef PARSER_HIVE_H_
#define PARSER_HIVE_H_

extern Node *parseStreamHive (FILE *stream);
extern Node *parseFromStringHive (char *input);

#endif /* PARSER_HIVE_H_ */

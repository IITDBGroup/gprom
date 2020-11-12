/*-----------------------------------------------------------------------------
 *
 * parser_postgres.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef PARSER_POSTGRES_H_
#define PARSER_POSTGRES_H_

extern Node *parseStreamPostgres (FILE *stream);
extern Node *parseFromStringPostgres (char *input);


#endif /* PARSER_POSTGRES_H_ */

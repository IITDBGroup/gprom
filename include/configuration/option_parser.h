#ifndef OPTIONPARSER_H
#define OPTIONPARSER_H

#include "configuration/option.h"

extern char *errorMessage;

#define OPTION_PARSER_RETURN_OK 0
#define OPTION_PARSER_RETURN_ERROR -1
#define OPTION_PARSER_RETURN_HELP 1
#define OPTION_PARSER_RETURN_VERSION 2

extern int parseOption(int const argc, char* const argv[]);
extern boolean isOption(char* const value);
extern int getNumberOfRewrite(int const argc, char* const argv[]);
extern void printOptionParseError(FILE *out);

extern boolean parseBool (char *b);
extern int parseInt (char *i);


#endif

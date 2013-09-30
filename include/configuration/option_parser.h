#ifndef OPTIONPARSER_H
#define OPTIONPARSER_H

#include "configuration/option.h"

extern int parseOption(int const argc, char* const argv[]);
extern boolean isOption(char* const value);
extern int getNumberOfRewrite(int const argc, char* const argv[]);

#endif

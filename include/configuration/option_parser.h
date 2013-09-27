#ifndef OPTIONPARSER_H
#define OPTIONPARSER_H

#include "configuration/option.h"

int parseOption(int const argc, char* const argv[]);
boolean isOption(char* const value);
int getNumberOfRewrite(int const argc, char* const argv[]);

#endif

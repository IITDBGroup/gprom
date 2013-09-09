#ifndef OPTIONPARSER_H
#define OPTIONPARSER_H

#include "CommandLine/Option.h"

extern int parseOption(int const argc, char* const argv[], Options* options);

boolean isOption(char* const value);

#endif

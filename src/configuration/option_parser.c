/*-------------------------------------------------------------------------
 *
 * option_parser.c
 *    Parse user input into data structure in memory
 *
 *        It gets user input from argc, argv, then parses each options or
 *        parameters into internal data structures in memory, and allocates
 *        memory if necessary.
 *
 *-------------------------------------------------------------------------
 */

#include "common.h"
#include "configuration/option_parser.h"
#include "configuration/option.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"
#include "exception/exception.h"

// use standard malloc to circumvent the memory manager
#ifndef MALLOC_REDEFINED
#undef malloc
#endif

// error message
char *errorMessage = NULL;

// private version of contextStrDup, because mem manager cannot be used here
static char *contextStringDup (char *);
static int parseOneOption (char *opt, char* const argv[], const int argc, int *pos);

/*
 * Input: argc, argv
 * Output: parameters parsed into structure Options
 * Memory allocation for options and parameters in here
 * return 1 if user asks for help, return 0 if no error, -1 if errors occur
 */
int
parseOption(int const argc, char* const argv[])
{
	int i;

	//parse each option in turn
	for(i=1;i<argc;i++)
	{
		char* value=argv[i];
		if(isOption(value))
		{
			if(strcmp(value,"-help")==0)
				return OPTION_PARSER_RETURN_HELP;

			if(strcmp(value,"-version")==0)
			    return OPTION_PARSER_RETURN_VERSION;

			if (parseOneOption(value, argv, argc, &i) != 0)
			    return OPTION_PARSER_RETURN_ERROR;
		}
	}

	return OPTION_PARSER_RETURN_OK;
}

boolean
isOption(char* const value)
{
	if(value[0]=='-')
		return TRUE;

	else return FALSE;
}

#define ASSERT_HAS_NEXT() \
    do { \
        if((*pos) >= argc) \
        { \
        	errorMessage = "Expected additional value for option"; \
            return 1; \
        } \
    } while(0)

static int
parseOneOption (char *opt, char* const argv[], const int argc, int *pos)
{
    if (strcmp(opt,"-activate") == 0)
    {
        ASSERT_HAS_NEXT();
        char *o = argv[++(*pos)];
        if (!hasCommandOption(o))
        {
            errorMessage = o;
            return 1;
        }

        (*pos)++;
        (*pos) -= 2;
        setBoolOption(o,TRUE);
        return 0;
    }
    else if (strcmp(opt,"-deactivate") == 0)
    {
        ASSERT_HAS_NEXT();
        char *o = argv[++(*pos)];
        if (!hasCommandOption(o))
        {
            errorMessage = o;
            return 1;
        }

        (*pos)++;
        (*pos) -= 2;
        setBoolOption(o,FALSE);
        return 0;
    }
    else if (hasCommandOption(opt))
    {
        char *o = commandOptionGetOption(opt);
        (*pos)++;

        switch(getOptionType(o))
        {
            case OPTION_INT:
            {
                ASSERT_HAS_NEXT();
                int val = atoi(argv[*pos]);
                setIntOption(o,val);
                (*pos)++;
                (*pos) -= 2;
            }
            break;
            case OPTION_STRING:
            {
                ASSERT_HAS_NEXT();
                setStringOption(o,strdup(argv[*pos]));
                (*pos)++;
                (*pos) -= 2;
            }
            break;
            case OPTION_FLOAT:
            {
                ASSERT_HAS_NEXT();
                //TODO
                (*pos)++;
                (*pos) -= 2;
            }
            break;
            case OPTION_BOOL:
                // if user has given value as separate argument
                if (*pos < argc && !isOption(argv[*pos]))
                {
                    char *bVal = argv[*pos];
//                    if (streq(bVal,"TRUE") || streq(bVal,"t") || streq(bVal,"1") || streq(bVal,"true"))
                        setBoolOption(o,parseBool(bVal));
//                    else
//                        setBoolOption(o,FALSE);
                }
                // otherwise mentioning an option sets it to TRUE
                else
                {
                    setBoolOption(o, TRUE);
                    (*pos) -= 1;
                }
            break;
        }
        return 0;
    }
    else
    {
        errorMessage = opt;
        return 1;
    }
}

static char *
contextStringDup (char *input)
{
    char *result = malloc(strlen(input) + 1);
    strcpy(result,input);
    return result;
}

extern void
printOptionParseError(FILE *out)
{
    fprintf(out, "ERROR PARSING OPTIONS: %s\n\n", errorMessage ? errorMessage : "");
}

boolean
parseBool (char *b)
{
    if (streq(b,"TRUE") || streq(b,"T") || streq(b,"t") || streq(b,"1") || streq(b,"true"))
        return TRUE;
    if (streq(b,"FALSE") || streq(b,"F") || streq(b,"f") || streq(b,"0") || streq(b,"false"))
        return FALSE;
//    THROW(SEVERITY_RECOVERABLE, "<%s> is not a boolean.", b);

    do { \
            storeExceptionInfo(SEVERITY_RECOVERABLE, formatMes("<%s> is not a boolean.", b),"/Users/lord_pretzel/Documents/workspace/GProM/src/configuration/option_parser.c",189); \
            if (exceptionBuf != ((void*)0)) \
                longjmp(*exceptionBuf, 1); \
            else \
                exit(1); \
        } while(0);

    return FALSE;
}

int
parseInt (char *i)
{
    int result = atoi(i);
    return result;
}


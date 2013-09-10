#include <string.h>
#include <stdlib.h>
#include "Configuration/OptionParser.h"

/*
 * Input: argc, argv
 * Output: parameters parsed into structure Options
 * Memory allocation for options and parameters in here
 * return 1 if user asks for help, return 0 if no error, -1 if errors occur
 */
int parseOption(int const argc, char* const argv[])
{
	Options* options=getOptions();
	int i;
	for(i=1;i<argc;i++)
	{
		char* value=argv[i];
		if(isOption(value))
		{
			if(strcmp(value,"-help")==0)
			{
				return 1;
			}
			else if(strcmp(value,"-host")==0)
			{
				if(i+1>=argc)
					return -1;
				options->optionConnection->host=(char*)malloc(sizeof(argv[i+1]));
				strcpy(options->optionConnection->host,argv[i+1]);
			}
			else if(strcmp(value,"-db")==0)
			{
				if(i+1>=argc)
					return -1;
				options->optionConnection->db=(char*)malloc(sizeof(argv[i+1]));
				strcpy(options->optionConnection->db,argv[i+1]);
			}
			else if(strcmp(value,"-port")==0)
			{
				if(i+1>=argc)
					return -1;
				options->optionConnection->port=atoi(argv[i+1]);
			}
			else if(strcmp(value,"-user")==0)
			{
				if(i+1>=argc)
					return -1;
				options->optionConnection->user=(char*)malloc(sizeof(argv[i+1]));
				strcpy(options->optionConnection->user,argv[i+1]);
			}
			else if(strcmp(value,"-passwd")==0)
			{
				if(i+1>=argc)
					return -1;
				options->optionConnection->passwd=(char*)malloc(sizeof(argv[i+1]));
				strcpy(options->optionConnection->passwd,argv[i+1]);
			}
			else if(strcmp(value,"-log")==0)
				options->optionDebug->log=TRUE;
			else if(strcmp(value,"-loglevel")==0)
			{
				if(i+1>=argc)
					return -1;
				options->optionDebug->loglevel=atoi(argv[i+1]);
			}
			else if(strcmp(value,"-debugmemory")==0)
				options->optionDebug->debugMemory=TRUE;
			else if(strcmp(value,"-alternative")==0)
				options->optionRewrite->alternative=TRUE;
			else
				return -1;
		}
	}

	return 0;
}

boolean isOption(char* const value)
{
	if(value[0]=='-')
		return TRUE;
	else return FALSE;
}


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

#include "model/node/nodetype.h"
#include "model/expression/expression.h"

// private version of contextStrDup, because mem manager cannot be used here
static char *contextStringDup (char *);

/*
 * Input: argc, argv
 * Output: parameters parsed into structure Options
 * Memory allocation for options and parameters in here
 * return 1 if user asks for help, return 0 if no error, -1 if errors occur
 */
int
parseOption(int const argc, char* const argv[])
{
	Options* options=getOptions();
	int i;
	List *rewriteOptions;

	//parse each option in turn
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
				options->optionConnection->host = strdup(argv[i+1]);
				i++;
			}
			else if(strcmp(value,"-db")==0)
			{
				if(i+1>=argc)
					return -1;
				options->optionConnection->db = strdup(argv[i+1]);
				i++;
			}
			else if(strcmp(value,"-port")==0)
			{
				if(i+1>=argc)
					return -1;
				options->optionConnection->port=atoi(argv[i+1]);
				i++;
			}
			else if(strcmp(value,"-user")==0)
			{
				if(i+1>=argc)
					return -1;
				options->optionConnection->user = strdup(argv[i+1]);
				i++;
			}
			else if(strcmp(value,"-passwd")==0)
			{
				if(i+1>=argc)
					return -1;
				options->optionConnection->passwd = strdup(argv[i+1]);
				i++;
			}
			else if(strcmp(value,"-log")==0)
				options->optionDebug->log=TRUE;
			else if(strcmp(value,"-loglevel")==0)
			{
				if(i+1>=argc)
					return -1;
				options->optionDebug->loglevel=atoi(argv[i+1]);
				i++;
			}
			else if(strcmp(value,"-debugmemory")==0)
				options->optionDebug->debugMemory=TRUE;
			else if(strcmp(value,"-activate")==0)
			{
			    KeyValue *op;

			    op = createNodeKeyValue(
			            (Node *) createConstString(strdup(argv[i+1])),
			            (Node *) createConstBool(TRUE));

			    options->optionRewrite = appendToTailOfList(options->optionRewrite, op);
			    i++;
//			    int size=getNumberOfRewrite(argc,argv);
//				options->optionRewrite->size=size;
//				if(i+size>=argc)
//					return -1;
//				options->optionRewrite->rewriteMethods=(RewriteMethod**)malloc(sizeof(RewriteMethod*)*size);
//				int j;
//				for(j=0;j<size;j++)
//				{
//					options->optionRewrite->rewriteMethods[j]=(RewriteMethod*)malloc(sizeof(RewriteMethod));
//					options->optionRewrite->rewriteMethods[j]->name=(char*)malloc(strlen(argv[i+1+j])+1);
//					strcpy(options->optionRewrite->rewriteMethods[j]->name,argv[i+1+j]);
//					options->optionRewrite->rewriteMethods[j]->isActive=TRUE;
//				}
//				i+=size;
			}
			else if (strcmp(value, "-sql") == 0)
			{
			    if(i+1>=argc)
                    return -1;
                options->optionConnection->sql=strdup(argv[i + 1]);
//                        (char*)malloc(strlen(argv[i+1])+1);
//                strcpy(options->optionConnection->sql,argv[i+1]);
                i++;
			}
			else
				return -1;
		}
	}

	return 0;
}

boolean
isOption(char* const value)
{
	if(value[0]=='-')
		return TRUE;
	else return FALSE;
}

//int
//getNumberOfRewrite(int const argc, char* const argv[])
//{
//	int i,start=0,end=0,size=0;
//	//get how many rewrites are activated
//	for(i=1;i<argc;i++)
//	{
//		char* value=argv[i];
//		if(isOption(value) && !start)
//		{
//			if(strcmp(value,"-activate")==0)
//			{
//				start=i;
//				break;
//			}
//		}
//	}
//	for(i=start+1;i<argc;i++)
//	{
//		char* value=argv[i];
//		if(isOption(value) && start)
//		{
//			end=i;
//			size=end-start-1;
//			break;
//		}
//		if(i==argc-1&&start)
//		{
//			end=i;
//			size=end-start;
//		}
//	}
//	return size;
//}

static char *
contextStringDup (char *input)
{
    char *result = malloc(strlen(input) + 1);
    strcpy(result,input);
    return result;
}

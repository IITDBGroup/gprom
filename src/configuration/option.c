/*-------------------------------------------------------------------------
 *
 * option.c
 *    The basic functions for options.
 *
 *        mallocOptions, freeOptions manage the memory for options.
 *        getOptions let other parts of program to have access to options.
 *
 *-------------------------------------------------------------------------
 */

#include "common.h"
#include "configuration/option.h"

Options* options;

void
mallocOptions()
{
	options=MAKE_OPTIONS();
	options->optionConnection=MAKE_OPTION_CONNECTION();
	memset(options->optionConnection, 0, sizeof(OptionConnection));
	options->optionDebug=MAKE_OPTION_DEBUG();
	options->optionRewrite=MAKE_OPTION_REWRITE();
}

void
freeOptions()
{
	free(options->optionConnection->host);
	free(options->optionConnection->db);
	free(options->optionConnection->user);
	free(options->optionConnection->passwd);
	free(options->optionConnection->sql);
	free(options->optionConnection);
	free(options->optionDebug);
	int i,size=options->optionRewrite->size;
	for(i=0;i<size;i++)
	{
		free(options->optionRewrite->rewriteMethods[i]->name);
		free(options->optionRewrite->rewriteMethods[i]);
	}
	free(options->optionRewrite->rewriteMethods);
	free(options->optionRewrite);
	free(options);
}

Options*
getOptions()
{
	return options;
}

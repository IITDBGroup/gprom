#include "configuration/option.h"

Options* options;

void
mallocOptions()
{
	options=MAKE_OPTIONS();
	options->optionConnection=MAKE_OPTION_CONNECTION();
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
	free(options->optionConnection);
	free(options->optionDebug);
	free(options->optionRewrite);
	free(options);
}

Options*
getOptions()
{
	return options;
}

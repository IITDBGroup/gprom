#include "Configuration/Option.h"

Options* options;

void mallocOptions()
{
	options=MakeOptions();
	options->optionConnection=MakeOptionConnection();
	options->optionDebug=MakeOptionDebug();
	options->optionRewrite=MakeOptionRewrite();
}

void freeOptions()
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

Options* getOptions()
{
	return options;
}

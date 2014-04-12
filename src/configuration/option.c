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
#include "model/list/list.h"
#include "model/expression/expression.h"

// we have to use actual free here
#undef free

Options* options;

void
mallocOptions()
{
	options=MAKE_OPTIONS();
	options->optionConnection=MAKE_OPTION_CONNECTION();
	options->optionDebug=MAKE_OPTION_DEBUG();
	options->optionRewrite=NIL;
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
//	FOREACH(KeyValue,o,options->optionRewrite)
//	{
//	    Constant *c;
//
//	    c = (Constant *) o->key;
//	    if (c->constType == DT_STRING)
//	        free(c->value);
//	    free(c);
//
//	    c = (Constant *) o->value;
//        if (c->constType == DT_STRING)
//            free(c->value);
//        free(c);
//
//        free(o);
//	}
//	freeList(options->optionRewrite);
//	free(options->optionRewrite);
	free(options);
}

Options*
getOptions()
{
	return options;
}

boolean
isRewriteOptionActivated(char *name)
{
    Options *ops = getOptions();

    FOREACH(KeyValue,op,ops->optionRewrite)
    {
        if (strcmp(STRING_VALUE(op->key),name) == 0)
            return BOOL_VALUE(op->value);
    }

    return FALSE;
}

#ifndef OPTION_H
#define OPTION_H

#include "common.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"

typedef enum OptionType {
    OPTION_BOOL,
    OPTION_STRING,
    OPTION_INT,
    OPTION_FLOAT
} OptionType;

/* define rewrite methods */
#define OPTION_TIMING "timing"
#define OPTION_AGGRESSIVE_MODEL_CHECKING "aggressive_model_checking"
#define OPTION_UPDATE_ONLY_USE_CONDS "only_updated_use_conditions"
#define OPTION_UPDATE_ONLY_USE_HISTORY_JOIN "only_updated_use_history"
#define OPTION_TREEIFY_OPERATOR_MODEL "treefiy_prov_rewrite_input"
#define OPTION_PI_CS_USE_COMPOSABLE "pi_cs_use_composable"
#define OPTION_OPTIMIZE_OPERATOR_MODEL "optimize_operator_model"
#define OPTION_TRANSLATE_UPDATE_WITH_CASE "translate_update_with_case"
//#define OPTION_

// new option interface
extern char *getStringOption (char *name);
extern int getIntOption (char *name);
extern boolean getBoolOption (char *name);
extern double getFloatOption (char *name);

extern void setStringOption (char *name, char *value);
extern void setIntOption(char *name, int value);
extern void setBoolOption(char *name, boolean value);
extern void setFloatOption(char *name, double value);

extern boolean hasOption(char *name);
extern boolean hasCommandOption(char *name);
extern char *commandOptionGetOption(char *name);
extern OptionType getOptionType(char *name);
extern boolean optionSet(char *name);

extern void printOptionsHelp(FILE *stream, char *progName, char *description);
extern void printCurrentOptions(FILE *stream);

extern void mallocOptions();
extern void freeOptions();
//extern Options *getOptions();
extern boolean isRewriteOptionActivated(char *name);

#endif

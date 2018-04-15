#include "common.h"
#include "model/query_operator/query_operator.h"
#include "model/set/set.h"

#define UNCERTAIN_ROW_ATTR backendifyIdentifier("R")
#define UNCERTAIN_FULL_ROW_ATTR getUncertString(UNCERTAIN_ROW_ATTR)
#define UNCERTAIN_MAKER_FUNC_NAME "UNCERT"

extern QueryOperator *rewriteUncert(QueryOperator *op);
extern Node *getUncertaintyExpr(Node *expr, HashMap *hmp); //hashmap of attrref->attrref(attribute->attribute uncertainty)
extern Node *removeUncertOpFromExpr(Node *expr);
extern char *getUncertString(char *in);

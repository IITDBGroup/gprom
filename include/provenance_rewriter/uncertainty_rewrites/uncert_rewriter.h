#include "common.h"
#include "model/query_operator/query_operator.h"
#include "model/set/set.h"


extern QueryOperator *rewriteUncert(QueryOperator *op);
extern Node *getUncertaintyExpr(Node *expr, HashMap *hmp); //hashmap of attrref->attrref(attribute->attribute uncertainty)
extern Node *removeUncertOpFromExpr(Node *expr);

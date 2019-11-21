#include "common.h"
#include "model/query_operator/query_operator.h"
#include "model/set/set.h"

#define UNCERTAIN_ROW_ATTR backendifyIdentifier("R")
#define UNCERTAIN_ROW_NAME backendifyIdentifier("U_R")
#define UNCERTAIN_ROW_NAME_TWO backendifyIdentifier("U_R1")
#define UNCERTAIN_FULL_ROW_ATTR getUncertString(UNCERTAIN_ROW_ATTR)
#define UNCERTAIN_MAKER_FUNC_NAME "UNCERT"
#define ROW_CERTAIN backendifyIdentifier("CET_R")
#define ROW_BESTGUESS backendifyIdentifier("BST_R")
#define ROW_POSSIBLE backendifyIdentifier("POS_R")
#define ROW_CERTAIN_TWO backendifyIdentifier("CET_R1")
#define ROW_BESTGUESS_TWO backendifyIdentifier("BST_R1")
#define ROW_POSSIBLE_TWO backendifyIdentifier("POS_R1")

extern QueryOperator *rewriteUncert(QueryOperator *op);
extern QueryOperator *rewriteRange(QueryOperator *op);
extern QueryOperator *rewriteUncertRow(QueryOperator *op);
extern Node *getUncertaintyExpr(Node *expr, HashMap *hmp); //hashmap of attrref->attrref(attribute->attribute uncertainty)
extern Node *removeUncertOpFromExpr(Node *expr);
extern char *getUncertString(char *in);
extern char *getUBString(char *in);
extern char *getLBString(char *in);

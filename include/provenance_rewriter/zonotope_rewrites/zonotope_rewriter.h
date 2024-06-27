#include "common.h"

#include "model/query_operator/query_operator.h"

#define ZONOTOPE_ROW_ATTR backendifyIdentifier("Z")
#define ZONOTOPE_FULL_ROW_ATTR getZonoString(ZONOTOPE_ROW_ATTR)
#define ROW_ZONO backendifyIdentifier("ZONO")
#define ZONO_MAPPING_PROP "ZONO_MAPPING"
#define ZUNCERT_MAKER_FUNC_NAME "ZUNCERT"
#define ZUNCERT_FUNC_NAME backendifyIdentifier("zuncert")

#define ZONO_ROW_CERTAIN backendifyIdentifier("CET_R")
#define ZONO_ROW_BESTGUESS backendifyIdentifier("BST_R")
#define ZONO_ROW_POSSIBLE backendifyIdentifier("POS_R")

#define ZONO_ATTR_LOW_BOUND backendifyIdentifier("Z_LB_")
#define ZONO_ATTR_HIGH_BOUND backendifyIdentifier("Z_UB_")

extern QueryOperator *rewriteZono(QueryOperator *op);

extern Node *removeZonoOpFromExpr(Node *expr);

extern char *getZonoString(char *in);
extern char *getZonoUBString(char *in);
extern char *getZonoLBString(char *in);

extern Node *getUBExpr(Node *expr, HashMap *hmp);
extern Node *getLBExpr(Node *expr, HashMap *hmp);

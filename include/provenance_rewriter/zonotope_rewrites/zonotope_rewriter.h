#include "common.h"
#include "model/query_operator/query_operator.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"

#define ZONOTOPE_ROW_ATTR backendifyIdentifier("R")
#define ROW_ZONO backendifyIdentifier("ZONO")
#define ZONOTOPE_FULL_ROW_ATTR getZonoString(ZONOTOPE_ROW_ATTR)
#define ZONOTOPE_MAKER_FUNC_NAME "ZONO"

extern QueryOperator *rewriteZono(QueryOperator *op);
extern char *getZonoString(char *in);

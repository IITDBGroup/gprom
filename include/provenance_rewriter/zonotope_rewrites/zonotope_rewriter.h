#include "common.h"

#include "model/query_operator/query_operator.h"

#define ZUNCERT_FUNC_NAME backendifyIdentifier("zuncert")

#define ZONO_MAPPING_PROP "ZONO_MAPPING"
#define ROW_ZONO backendifyIdentifier("ZONO")

#define ZONOTOPE_ROW_ATTR backendifyIdentifier("Z")
#define ZONOTOPE_FULL_ROW_ATTR getZonoString(ZONOTOPE_ROW_ATTR)

extern QueryOperator *rewriteZono(QueryOperator *op);
extern char *getZonoString(char *in);

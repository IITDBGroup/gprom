#include "common.h"
#include "model/query_operator/query_operator.h"
#include "model/set/hashmap.h"
#include "model/set/set.h"

#define UNCERTAIN_ROW_ATTR backendifyIdentifier("R")
#define UNCERTAIN_FULL_ROW_ATTR getUncertString(UNCERTAIN_ROW_ATTR)
#define UNCERTAIN_MAKER_FUNC_NAME "UNCERT"
#define ROW_CERTAIN backendifyIdentifier("CET_R")
#define ROW_BESTGUESS backendifyIdentifier("BST_R")
#define ROW_POSSIBLE backendifyIdentifier("POS_R")
#define ROW_CERTAIN_TWO backendifyIdentifier("CET_R1")
#define ROW_BESTGUESS_TWO backendifyIdentifier("BST_R1")
#define ROW_POSSIBLE_TWO backendifyIdentifier("POS_R1")
#define ALPHABET " !\"#$\%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"

// USET相关常量定义
#define USET_FUNC_NAME "USET"
#define RANGE_SET_ADD_FUNC_NAME "range_set_add"
#define RANGE_SET_MULTIPLY_FUNC_NAME "range_set_multiply"
#define RANGE_SET_SMALLERTHAN_FUNC_NAME "range_set_smallerthan"
#define RANGE_SET_LARGERTHAN_FUNC_NAME "range_set_largerthan"
#define RANGE_SET_EQUAL_FUNC_NAME "range_set_equal"
#define RANGE_SET_LOGIC_FUNC_NAME "range_set_logic"
#define RANGE_SET_SUBTRACT_FUNC_NAME "range_set_subtract"

extern QueryOperator *rewriteUncert(QueryOperator *op);
extern QueryOperator *rewriteUncertTuple(QueryOperator *op);
extern QueryOperator *rewriteRange(QueryOperator *op);
extern QueryOperator *rewriteUset(QueryOperator *op);
extern Node *getUncertaintyExpr(Node *expr, HashMap *hmp); //hashmap of attrref->attrref(attribute->attribute uncertainty)
extern Node *removeUncertOpFromExpr(Node *expr);
extern char *getUncertString(char *in);
extern char *getUBString(char *in);
extern char *getLBString(char *in);

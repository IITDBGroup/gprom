#ifndef NODETYPE_H
#define NODETYPE_H

#include "expression.h"

typedef enum NodeTag{
T_Invalid=0,

/*tags for exe nodes*/
T_IndexInfo,
T_ResultInfo,

/*tags for plan nodes*/
T_Result,
T_Join,
T_Merge,
T_Sort,
T_Group,

/*tags for we now use*/
T_Constant,
T_AttributeReference,
T_FunctionCall,
T_Operator

}NodeTag;

typedef struct Node{
NodeTag type;

}Node;


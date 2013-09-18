#ifndef NODETYPE_H
#define NODETYPE_H

#include "common.h"

typedef enum NodeTag {
    T_Invalid=0,

    /*tags for value nodes*/
    T_Value = 200,
    T_Integer,
    T_Float,
    T_String,
    T_BitString,
    T_Null,
    
    /*tags for list nodes*/
    T_List,
    T_IntList,

    /*tags for we now use*/
    T_Constant,
    T_AttributeReference,
    T_FunctionCall,
    T_Operator
} NodeTag;

typedef struct Node{
    NodeTag type;
} Node;


#define nodeTag(nodeptr) (((Node*)(nodeptr))->type)
#define newNode(size, tag)
#define makeNode()  ((type*)newNode(sizeof(type),T_type)
#define nodeSetTag(nodeptr,t) (((Node*)(nodeptr))->type = (t))
#define isA(nodeptr, type)  (nodeTag(nodeptr)) == T_type)

/*extern declaration */
extern char *nodeToString(void *obj);
extern void outNode(/* TODO */ void *obj);

/*readfun.c*/
extern void *stringToNode(char *str);

/*copyfun.c*/
extern void *copyObject(void *obj);

/*equalfun.c*/
extern boolean equal(void *a, void *b);


#endif /*NODETYPE_H*/

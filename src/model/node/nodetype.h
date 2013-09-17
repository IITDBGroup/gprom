#ifndef NODETYPE_H
#define NODETYPE_H



typedef enum NodeTag{
    T_Invalid=0,

    /*tags for exe nodes*/
    T_IndexInfo,
    T_ResultInfo,
 
    /*tags for prim nodes*/
    T_Alias = 100,
    T_Var,
    T_Const,
    T_FuncExpr,
    
    /*tags for plan nodes*/
    T_Result,
    T_Join,
    T_Merge,
    T_Sort,
    T_Group,

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

}NodeTag;

typedef struct Node{
    NodeTag type;

}Node;


#define nodeTag(nodeptr) (((Node*)(nodeptr))->type)
#define newNode(size, tag)
#define makeNode()  ((type*)newNode(sizeof(type),T_type)
#define nodeSetTag(nodeptr,t) (((Node*)(nodeptr))->type = (t))
#define isA(nodeptr, type)  (nodeTag(nodeptr)) == T_type)

/*extern declaration */

typedef struct StringInfoData{
     char *data;
     int len;
     int maxlen;
     int cursor;

}StringInfoData;

typedef StringInfoData *StringInfo;

extern char *nodeToString(void *obj);
extern void outNode(StringInfo str, void *obj);
extern void outBitmapset(StringInfo str, Bitmapset *bms);
extern void outData(StringInfo str, Data value, int typlen, bool typbyval);

/*readfun.c*/

extern void *stringToNode(char *str);
extern Data readData(bool typbyval);

/*copyfun.c*/

extern void *copyObject(void *obj);

/*equalfun.c*/

extern bool equal(void *a, void *b);

/*CmdType for type of operation represented by a Query*/

typedef enum CmdType{
     CMD_UNKNOWN,
     CMD_SELECT,
     CMD_UPDATE,
     CMD_INSERT,
     CMD_DELETE,
     CMD_UTILITY,
     CMD_NOTHING
}CmdType;

/*JoinType enums for types of relation joins*/

typedef enum JoinType{
     JOIN_INNER,      /*tuple pairs only*/
     JOIN_LEFT,
     JOIN_RIGHT,
     JOIN_FULL,
     JOIN_IN,          /*at most one result per outer row*/
     JOIN_REVERSE_IN,  /*at most one result per inner row*/
     JOIN_UNIQUE_OUTER,/*outer path must be made unique*/
     JOIN_UNIQUE_INNER /*inner path must be made unique*/

}JoinType;

#define IS_OUTER_JOIN(jointype) ((jointype)==JOIN_LEFT||(jointype)==JOIN_FULL||(jointype)==JOIN_RIGHT)

#endif /*NODETYPE_H*/

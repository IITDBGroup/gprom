/*-----------------------------------------------------------------------------
 *
 * datalog_model.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_MODEL_DATALOG_DATALOG_MODEL_H_
#define INCLUDE_MODEL_DATALOG_DATALOG_MODEL_H_

#include "model/node/nodetype.h"
#include "model/expression/expression.h"
#include "model/set/hashmap.h"

// data types
typedef struct DLNode
{
    NodeTag type;
    HashMap *properties;
} DLNode;

typedef struct DLAtom
{
    DLNode n;
    char *rel;
    List *args;     // vars or consts
    boolean negated;
} DLAtom;

typedef struct DLVar
{
    DLNode n;
    char *name;
    DataType dt;
} DLVar;

typedef struct DLComparison
{
    DLNode n;
    Operator *opExpr;
} DLComparison;

typedef struct DLDomain
{
    DLNode n;
    char *name;
} DLDomain;

typedef struct DLRule
{
    DLNode n;
    DLAtom *head;
    List *body;     // list of atoms and comparisons
} DLRule;

typedef struct DLProgram
{
    DLNode n;
    List *rules;
    List *facts;
    char *ans;
    char *dom;
} DLProgram;

NEW_ENUM_WITH_TO_STRING(GPNodeType,
        GP_NODE_RULE,
        GP_NODE_GOAL,
        GP_NODE_POSREL,
        GP_NODE_NEGREL,
        GP_NODE_EDB,
		GP_NODE_TUPLE,
        GP_NODE_RULEHYPER,
		GP_NODE_GOALHYPER
        );

// provenance
#define IS_GP_PROV(prog) (DL_HAS_PROP(prog,DL_PROV_WHY) \
    || DL_HAS_PROP(prog,DL_PROV_WHYNOT) \
    || DL_HAS_PROP(prog,DL_PROV_FULL_GP))

#define IS_DL_NODE(n) (isA(n,DLNode) || isA(n,DLAtom) || isA(n,DLVar) \
        || isA(n,DLComparison) || isA(n,DLRule) || isA(n,DLProgram))

// convenience functions
extern DLAtom *createDLAtom (char *rel, List *args, boolean negated);
extern DLVar *createDLVar (char *vName, DataType vType);
extern boolean isConstAtom (DLAtom *a);
extern DLRule *createDLRule (DLAtom *head, List *body);
extern DLProgram *createDLProgram (List *dlRules, List *facts, char *ans, char *dom);
extern DLComparison *createDLComparison (char *op, Node *lArg, Node *rArg);
extern DLDomain *createDLDomain (char *dom);

// get information about DL program elements
extern char *getHeadPredName(DLRule *r);
extern List *getRuleVars (DLRule *r);
extern List *getBodyVars (DLRule *r);
List *getBodyArgs (DLRule *r);
extern List *getBodyPredVars (DLRule *r);
extern List *getHeadVars (DLRule *r);

//extern List *getAtomVars (DLAtom *r);

extern List *getVarNames (List *vars);

// unification and variable mappings
extern DLRule *unifyRule (DLRule *r, List *headBinds);
// take a datalog model M as input and mappings of type Var -> Node
// return  h(M)
extern Node *applyVarMap(Node *input, HashMap *h);
extern boolean argListsUnifyable (List *argsL, List *argsR);
extern Node *applyVarMapAsLists(Node *input, List *vars, List *replacements);
extern DLAtom *getNormalizedAtom(DLAtom *a);
extern void makeVarNamesUnique(List *nodes);
extern char *getUnificationString(DLAtom *a);


// properties
extern Node *getDLProp(DLNode *n, char *key);
extern void setDLProp(DLNode *n, char *key, Node *value);
extern void delDLProp(DLNode *n, char *key);

#define DL_HAS_PROP(node,key) (getDLProp((DLNode *) node, key) != NULL)
#define DL_GET_PROP(node,key) (getDLProp((DLNode *) node, key))
#define DL_SET_BOOL_PROP(node,key) setDLProp((DLNode *) node, key, (Node *) createConstBool(TRUE));
#define DL_SET_STRING_PROP(node,key,value) setDLProp((DLNode *) node, key, (Node *) createConstString(value));
#define DL_DEL_PROP(node,key) (delDLProp((DLNode *) node, key))
#define DL_COPY_PROP(node1,node2,key) (setDLProp((DLNode *) node2, key, getDLProp((DLNode *) node1,key)))

// property keys
#define DL_IS_IDB_REL "IS_IDB_REL"
#define DL_IS_EDB_REL "IS_EDB_REL"
#define DL_IS_FACT_REL "IS_FACT_REL"

#define DL_PROV_WHY "WHY_PROV"
#define DL_PROV_WHYNOT "WHYNOT_PROV"
#define DL_PROV_FULL_GP "FULL_GP_PROV"
#define DL_PROV_PROG "GAME PROVENANCE PROGRAM"

#define DL_PROV_FORMAT "PROV_FORMAT"
#define DL_PROV_FORMAT_GP "FULL_GP"
#define DL_PROV_FORMAT_GP_REDUCED "REDUCED_GP"
#define DL_PROV_FORMAT_TUPLE_ONLY "TUPLE_ONLY"
#define DL_PROV_FORMAT_TUPLE_RULE_TUPLE "TUPLE_RULE_TUPLE"
#define DL_PROV_FORMAT_TUPLE_RULE_GOAL_TUPLE "TUPLE_RULE_GOAL_TUPLE"
#define DL_PROV_FORMAT_HEAD_RULE_EDB "HEAD_RULE_EDB"
#define DL_PROV_FORMAT_TUPLE_RULE_TUPLE_REDUCED "TUPLE_RULE_TUPLE_REDUCED"

// property keys for storing analysis results for a program
#define DL_MAP_RELNAME_TO_RULES "REL_TO_RULES"
#define DL_MAP_UN_PREDS_TO_RULES "UN_PREDS_TO_RULES"
#define DL_MAP_ADORNED_PREDS_TO_RULES "ADORNED_PREDS_TO_RULES"
#define DL_IDB_RELS "IDB_RELS"
#define DL_EDB_RELS "EDB_RELS"
#define DL_FACT_RELS "FACT_RELS"

#define DL_WON "WON"
#define DL_LOST "LOST"
#define DL_UNDER_NEG_WON "UNDER_NEG_WON"
#define DL_UNDER_NEG_LOST "UNDER_NEG_LOST"

#define DL_ORIGINAL_RULE "ORIG_RULE"
#define DL_NORM_ATOM "NORMALIZED_ATOM"
#define DL_ORIG_ATOM "ORIG_ATOM"
#define DL_RULE_ID "RULE_ID"

// property keys for DT information
#define DL_PRED_DTS "PREDICATE_DATATYPES"

#endif /* INCLUDE_MODEL_DATALOG_DATALOG_MODEL_H_ */

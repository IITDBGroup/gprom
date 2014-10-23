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
} DLProgram;

// convenience functions
extern DLAtom *createDLAtom (char *rel, List *args, boolean negated);
extern DLVar *createDLVar (char *vName, DataType vType);
extern boolean isConstAtom (DLAtom *a);
extern DLRule *createDLRule (DLAtom *head, List *body);
extern DLProgram *createDLProgram (List *dlRules, List *facts, char *ans);
extern DLComparison *createDLComparison (char *op, Node *lArg, Node *rArg);

extern char *getHeadPredName(DLRule *r);
extern List *getRuleVars (DLRule *r);
extern List *getBodyVars (DLRule *r);
extern List *getBodyPredVars (DLRule *r);
extern List *getHeadVars (DLRule *r);

//extern List *getAtomVars (DLAtom *r);

extern List *getVarNames (List *vars);

// properties
extern Node *getDLProp(DLNode *n, char *key);
extern void setDLProp(DLNode *n, char *key, Node *value);

#define DL_HAS_PROP(node,key) (getDLProp((DLNode *) node,key) != NULL)
#define DL_SET_BOOL_PROP(node,key) setDLProp((DLNode *) node, key, (Node *) createConstBool(TRUE));

// property keys
#define DL_IS_IDB_REL "IS_IDB_REL"

#endif /* INCLUDE_MODEL_DATALOG_DATALOG_MODEL_H_ */

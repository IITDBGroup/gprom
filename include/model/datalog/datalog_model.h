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
    char *rel;
    char *attr;
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
    List *doms;
    List *comp;
    List *func;
    List *sumOpts;
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

#define IS_LINEAGE_PROV(prog) DL_HAS_PROP(prog,DL_PROV_LINEAGE)

#define IS_DL_NODE(n) (isA(n,DLNode) || isA(n,DLAtom) || isA(n,DLVar) \
					   || isA(n,DLComparison) || isA(n,DLRule) || isA(n,DLProgram))

#define IDB_ATTR_NAME(i) backendifyIdentifier(CONCAT_STRINGS("a", gprom_itoa(i)))

// convenience functions
extern DLAtom *createDLAtom(char *rel, List *args, boolean negated);
extern DLAtom *createDLAtomFromStrs(char *rel, boolean negated, char *vars, ...);
#define DLATOM_FROM_STRS(_rel, _negated, ...) createDLAtomFromStrs(_rel, _negated, ##__VA_ARGS__, (char *) NULL)
extern DLVar *createDLVar(char *vName, DataType vType);
extern boolean isConstAtom(DLAtom *a);
extern DLRule *createDLRule(DLAtom *head, List *body);
extern DLProgram *createDLProgram(List *dlRules, List *facts, char *ans, List *doms, List *func, List *sumOpts);
extern DLComparison *createDLComparison(char *op, Node *lArg, Node *rArg);
extern DLDomain *createDLDomain(char *rel, char *attr, char *dom);

// get information about DL program elements
extern char *getHeadPredName(DLRule *r);
extern List *getHeadAttrNames(DLRule *r);
extern List *getRuleVars(DLRule *r);
extern List *getBodyVars(DLRule *r);
extern List *getBodyArgs(DLRule *r);
extern List *getBodyPredVars(DLRule *r);
extern List *getHeadVars(DLRule *r);
extern List *getHeadVarNames(DLRule *r);
extern List *getHeadExprVars(DLRule *r);
extern List *getAtomExprVars(DLAtom *a);
extern List *getAtomTopLevelVars(DLAtom *a);
extern List *getComparisonVars(DLComparison *a);
extern List *getExprVars(Node *expr);
extern size_t getIDBPredArity(DLProgram *p, char *pred);
extern boolean isIDB(DLProgram *p, char *pred);
extern boolean isEDB(DLProgram *p, char *pred);
extern List *getVarNames(List *vars);
extern List *predGetAttrNames(DLProgram *p, char *pred);
extern List *getComparisonAtoms(DLRule *r);
extern List *getGoalsForPred(DLRule *r, char *p);

// unification and variable mappings
extern DLRule *unifyRule(DLRule *r, List *headBinds);
// take a datalog model M as input and mappings of type Var -> Node
// return  h(M)
extern Node *applyVarMap(Node *input, HashMap *h);
extern boolean argListsUnifyable(List *argsL, List *argsR);
extern Node *applyVarMapAsLists(Node *input, List *vars, List *replacements);
extern DLAtom *getNormalizedAtom(DLAtom *a);
extern void makeVarNamesUnique(List *nodes, boolean replaceOrigNames);
extern DLVar *createUniqueVar(Node *n, DataType dt);
extern char *getUnificationString(DLAtom *a);

// rewrites by substituting rule bodies for rule heads
extern DLProgram *mergeSubqueries(DLProgram *p, boolean allowRuleNumberIncrease);

// properties
extern Node *getDLProp(DLNode *n, char *key);
extern void setDLProp(DLNode *n, char *key, Node *value);
extern void delDLProp(DLNode *n, char *key);
extern void delAllProps(DLNode *n);

#define DL_HAS_PROP(node,key) (getDLProp((DLNode *) node, key) != NULL)
#define DL_SET_PROP(node,key,value) (setDLProp((DLNode *) node, key, (Node *) value));
#define DL_GET_PROP(node,key) (getDLProp((DLNode *) node, key))
#define DL_SET_BOOL_PROP(node,key) setDLProp((DLNode *) node, key, (Node *) createConstBool(TRUE));
#define DL_SET_STRING_PROP(node,key,value) setDLProp((DLNode *) node, key, (Node *) createConstString(value));
#define DL_GET_STRING_PROP(node,key) STRING_VALUE(getDLProp((DLNode *) node, key))
#define DL_GET_PROP_DEFAULT(node,key,_default) (DL_HAS_PROP(node,key) ? DL_GET_PROP(node,key) : _default)
#define DL_GET_STRING_PROP_DEFAULT(node,key,_default) (DL_HAS_PROP(node,key) ? STRING_VALUE(DL_GET_PROP(node,key)) : _default)
#define DL_DEL_PROP(node,key) (delDLProp((DLNode *) node, key))
#define DL_COPY_PROP(node1,node2,key) (setDLProp((DLNode *) node2, key, getDLProp((DLNode *) node1,key)))

// property keys
#define DL_IS_IDB_REL "IS_IDB_REL"
#define DL_IS_EDB_REL "IS_EDB_REL"
#define DL_IS_FACT_REL "IS_FACT_REL"
#define DL_IS_DOMAIN_REL "IS_DOMAIN_REL"
#define DL_HAS_AGG "HAS_AGGR"

#define DL_PROG_FDS "PROGRAM_FDS"
#define DL_IS_CHECKED "HAS_BEEN_CHECKED"

// provenance
#define DL_PROV_WHY "WHY_PROV"
#define DL_PROV_WHYNOT "WHYNOT_PROV"
#define DL_PROV_FULL_GP "FULL_GP_PROV"
#define DL_PROV_PROG "GAME PROVENANCE PROGRAM"
#define DL_PROV_LINEAGE "LINEAGE"

#define DL_PROV_FORMAT "PROV_FORMAT"
#define DL_PROV_FORMAT_GP "FULL_GP"
#define DL_PROV_FORMAT_GP_REDUCED "REDUCED_GP"
#define DL_PROV_FORMAT_TUPLE_ONLY "TUPLE_ONLY"
#define DL_PROV_FORMAT_TUPLE_RULE_TUPLE "TUPLE_RULE_TUPLE"
#define DL_PROV_FORMAT_TUPLE_RULE_GOAL_TUPLE "TUPLE_RULE_GOAL_TUPLE"
#define DL_PROV_FORMAT_TUPLE_RULE_GOAL_TUPLE_REDUCED "TUPLE_RULE_GOAL_TUPLE_REDUCED"
#define DL_PROV_FORMAT_HEAD_RULE_EDB "HEAD_RULE_EDB"
#define DL_PROV_FORMAT_TUPLE_RULE_TUPLE_REDUCED "TUPLE_RULE_TUPLE_REDUCED"
#define DL_PROV_LINEAGE_TARGET_TABLE "LINEAGE_TARGET_TABLE"
#define DL_PROV_LINEAGE_RESULT_FILTER_TABLE "LINEAGE_Q_FILTER"

#define DL_PROV_IS_REWRITTEN_PROG "DL_IS_REWRITTEN_PROP"

// property keys for storing analysis results for a program
#define DL_MAP_RELNAME_TO_RULES "REL_TO_RULES"
#define DL_MAP_BODYPRED_TO_RULES "GOAL_TO_RULES"
#define DL_REL_TO_REL_GRAPH "REL_TO_REL_G"
#define DL_INV_REL_TO_REL_GRAPH "REL_TO_REL_INV_G"
#define DL_MAP_UN_PREDS_TO_RULES "UN_PREDS_TO_RULES"
#define DL_MAP_ADORNED_PREDS_TO_RULES "ADORNED_PREDS_TO_RULES"
#define DL_IDB_RELS "IDB_RELS"
#define DL_EDB_RELS "EDB_RELS"
#define DL_FACT_RELS "FACT_RELS"
#define DL_AGGR_RELS "AGGR_DEF_RELS"
#define DL_GEN_PROJ_RELS "GEN_PROJ_RELS"
#define DL_RULE_IDS "DL_RULE_IDS"

// GP rule information
#define DL_WON "WON"
#define DL_LOST "LOST"
#define DL_UNDER_NEG_WON "UNDER_NEG_WON"
#define DL_UNDER_NEG_LOST "UNDER_NEG_LOST"

#define DL_ORIGINAL_RULE "ORIG_RULE"
#define DL_NORM_ATOM "NORMALIZED_ATOM"
#define DL_ORIG_ATOM "ORIG_ATOM"
#define DL_RULE_ID "RULE_ID"
#define DL_DOMAIN_RULE "DOMAIN_RULE"

// property keys for DT information
#define DL_PRED_DTS "PREDICATE_DATATYPES"

#endif /* INCLUDE_MODEL_DATALOG_DATALOG_MODEL_H_ */

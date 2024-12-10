/*-----------------------------------------------------------------------------
 *
 * expression.c
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "mem_manager/mem_mgr.h"
#include "log/logger.h"
#include "metadata_lookup/metadata_lookup.h"
#include "sql_serializer/sql_serializer.h"
#include "model/node/nodetype.h"
#include "model/list/list.h"
#include "model/expression/expression.h"
#include "model/datalog/datalog_model.h"
#include "configuration/option.h"
#include "utility/string_utils.h"
#include "provenance_rewriter/uncertainty_rewrites/uncert_rewriter.h"

typedef struct FindNotesContext
{
    NodeTag type;
    List **result;
} FindNotesContext;

typedef struct CastGraphEdge
{
    DataType in;
    DataType out;
} CastGraphEdge;

#define STOPPER { DT_BOOL, DT_BOOL }
static CastGraphEdge stopper = STOPPER;

static CastGraphEdge castGraph[] = {
        { DT_INT, DT_FLOAT },
        { DT_INT, DT_STRING },
        { DT_INT, DT_LONG },
        { DT_LONG, DT_FLOAT },
        { DT_LONG, DT_STRING },
        { DT_FLOAT, DT_STRING },
        { DT_BOOL, DT_INT },
        { DT_BOOL, DT_STRING },
        STOPPER
};

#define PREFSTOPPER -1
static int typePreferences[] = { DT_BOOL, DT_INT, DT_FLOAT, DT_STRING, PREFSTOPPER };

static boolean findAllNodesVisitor(Node *node, FindNotesContext *context);
static boolean findAttrReferences (Node *node, List **state);
static boolean findDLVars (Node *node, List **state);
static boolean findDLVarsIgnoreProps (Node *node, List **state);
static DataType typeOfOp (Operator *op, boolean *exists);
static DataType typeOfOpSplit (char *opName, List *argDTs, boolean *exists);
static DataType typeOfFunc (FunctionCall *f);
static Node *addCastsMutator (Node *node, boolean errorOnFailure);
static void castOperatorArgs(Operator *o);
static void castFunctionArgs(FunctionCall *f);
static List*typeOfArgs(List* args);
static boolean opExists (char *opName, List *argDTs);
static boolean funcExists (char *fName, List *argDTs);
static DataType nextCastableType (DataType in);
static Set *castsAsSet(DataType in);


AttributeReference *
createAttributeReference (char *name)
{
    AttributeReference *result = makeNode(AttributeReference);

    result->name = strdup(name);
    result->fromClauseItem = INVALID_FROM_ITEM;
    result->attrPosition = INVALID_ATTR;
    result->outerLevelsUp = INVALID_ATTR;
    result->attrType= DT_STRING;

    return result;
}

AttributeReference *
createFullAttrReference (char *name, int fromClause, int attrPos,
        int outerLevelsUp, DataType attrType)
{
    AttributeReference *result = makeNode(AttributeReference);

    result->name = strdup(name);
    result->fromClauseItem = fromClause;
    result->attrPosition = attrPos;
    result->outerLevelsUp = outerLevelsUp;
    result->attrType= attrType;

    return result;
}

CastExpr *
createCastExpr (Node *expr, DataType resultDt)
{
    CastExpr *result = makeNode(CastExpr);

    result->expr = expr;
    result->resultDT = resultDt;
    result->otherDT = NULL;
    result->num = -1;

    return result;
}

CastExpr *
createCastExprOtherDT(Node *expr, char* otherDT, int num, DataType gpromDT)
{
    CastExpr *result = makeNode(CastExpr);

    result->expr = expr;
    result->resultDT = gpromDT;
    result->otherDT = otherDT;
    result->num = num;

    return result;
}

SQLParameter *
createSQLParameter (char *name)
{
    SQLParameter *result = makeNode(SQLParameter);

    result->name = strdup(name);
    result->position = INVALID_PARAM;
    result->parType = DT_STRING;//TODO

    return result;
}

CaseExpr *
createCaseExpr (Node *expr, List *whenClauses, Node *elseRes)
{
    CaseExpr *result = makeNode(CaseExpr);

    result->expr = expr;
    result->whenClauses = whenClauses;
    result->elseRes = elseRes;

    return result;
}

CaseWhen *
createCaseWhen (Node *when, Node *then)
{
    CaseWhen *result = makeNode(CaseWhen);

    result->when = when;
    result->then = then;

    return result;
}

WindowBound *
createWindowBound (WindowBoundType bType, Node *expr)
{
    WindowBound *result = makeNode(WindowBound);

    result->bType = bType;
    result->expr = expr;

    return result;
}

WindowFrame *
createWindowFrame (WinFrameType winType, WindowBound *lower, WindowBound *upper)
{
    WindowFrame *result = makeNode(WindowFrame);

    result->frameType = winType;
    result->lower = lower;
    result->higher = upper;

    return result;
}

WindowDef *
createWindowDef (List *partitionBy, List *orderBy, WindowFrame *frame)
{
    WindowDef *result = makeNode(WindowDef);

    result->partitionBy = partitionBy;
    result->orderBy = orderBy;
    result->frame = frame;

    return result;
}

WindowFunction *
createWindowFunction (FunctionCall *f, WindowDef *win)
{
    WindowFunction *result = makeNode(WindowFunction);

    result->f = f;
    result->win = win;

    return result;
}

OrderExpr *
createOrderExpr (Node *expr, SortOrder order, SortNullOrder nullOrder)
{
    OrderExpr *result = makeNode(OrderExpr);

    result->expr = expr;
    result->order = order;
    result->nullOrder = nullOrder;

    return result;
}

QuantifiedComparison *
createQuantifiedComparison (char *nType, Node *checkExpr, char *opName, List *exprList)
{
	QuantifiedComparison *result = makeNode(QuantifiedComparison);

    result->checkExpr = checkExpr;
    result->exprList = exprList;
    result->opName = strdup(opName);

    if (!strcmp(nType, "ANY"))
        result->qType = QUANTIFIED_EXPR_ANY;
    if (!strcmp(nType, "ALL"))
        result->qType = QUANTIFIED_EXPR_ALL;

    return result;
}

Node *
concatExprs (Node *expr, ...)
{
    Node *result = NULL;
    Node *curArg = NULL;
    List *argList = singleton(expr);

    va_list args;

    va_start(args, expr);

    while((curArg = va_arg(args,Node*)))
        argList = appendToTailOfList(argList, copyObject(curArg));

    va_end(args);

    if (LIST_LENGTH(argList) == 1)
        return expr;

    result = (Node *) createFunctionCall(OPNAME_CONCAT, argList);

    return result;
}

Node *
concatExprList (List *exprs)
{
    Node *result = popHeadOfListP(exprs);

    FOREACH(Node,e,exprs)
        result = CONCAT_EXPRS(result,e);

    return result;
}


Node *
andExprs (Node *expr, ...)
{
    Node *result = NULL;
    Node *curArg = NULL;
    List *argList = singleton(expr);

    va_list args;

    va_start(args, expr);

    while((curArg = va_arg(args,Node*)))
        argList = appendToTailOfList(argList, copyObject(curArg));

    va_end(args);

    if (LIST_LENGTH(argList) == 1)
        return expr;

    result = (Node *) createOpExpr(OPNAME_AND, argList);

    return result;
}

Node *
andExprList (List *exprs)
{
    Node *result = popHeadOfListP(exprs);

    FOREACH(Node,e,exprs)
        result = AND_EXPRS(result,e);

    return result;
}

Node *
orExprList (List *exprs)
{
    Node *result = popHeadOfListP(exprs);

    FOREACH(Node,e,exprs)
        result = OR_EXPRS(result,e);

    return result;
}

Node *
orExprs (Node *expr, ...)
{
    Node *result = NULL;
    Node *curArg = NULL;
    List *argList = singleton(expr);

    va_list args;

    va_start(args, expr);

    while((curArg = va_arg(args,Node*)))
        argList = appendToTailOfList(argList, copyObject(curArg));

    va_end(args);

    if (LIST_LENGTH(argList) == 1)
        return expr;

    result = (Node *) createOpExpr(OPNAME_OR, argList);

    return result;
}

FunctionCall *
createFunctionCall(char *fName, List *args)
{
    FunctionCall *result = makeNode(FunctionCall);

    if(fName != NULL)
    {
        result->functionname = (char *) CALLOC(1,strlen(fName) + 1);
        strcpy(result->functionname, fName);
    }
    else
        result->functionname = NULL;

    result->args = args; //should we copy?
    result->isAgg = FALSE;
    result->isDistinct = FALSE;

    return result;
}

Operator *
createOpExpr(char *name, List *args)
{
    Operator *result = makeNode(Operator);

    if(name != NULL)
    {
        result->name = (char *) CALLOC(1, strlen(name) + 1);
        strcpy(result->name, name);
    }
    else
        result->name = NULL;

    result->args = args;

    return result;
}

IsNullExpr *
createIsNullExpr (Node *expr)
{
    IsNullExpr *result = makeNode(IsNullExpr);

    result->expr = expr;

    return result;
}

Node *
createIsNotDistinctExpr (Node *lArg, Node *rArg)
{
    Node *eq; //, *nullTest;
    SqlserializerPluginType backend = getActiveSqlserializerPlugin();

    switch(backend)
    {
        case SQLSERIALIZER_PLUGIN_ORACLE:
        {
            eq = (Node *) createOpExpr(OPNAME_EQ, LIST_MAKE(
                    createFunctionCall("sys_op_map_nonnull", singleton(copyObject(lArg))),
                    createFunctionCall("sys_op_map_nonnull", singleton(copyObject(rArg)))));
        }
        break;
        case SQLSERIALIZER_PLUGIN_POSTGRES:
        {
            eq = (Node *) createOpExpr("IS NOT DISTINCT FROM",
                    LIST_MAKE(copyObject(lArg), copyObject(rArg)));
        }
        break;
        default:
        {
            Node *nullTest, *eqTest;

            eqTest = (Node *) createOpExpr(OPNAME_EQ, LIST_MAKE(copyObject(lArg), copyObject(rArg)));
            nullTest = (Node *) createOpExpr(OPNAME_AND, LIST_MAKE(
                    createIsNullExpr(copyObject(lArg)),
                    createIsNullExpr(copyObject(rArg))));

            eq = (Node *) createOpExpr(OPNAME_OR, LIST_MAKE(eqTest, nullTest));
        }
    }
    return (Node *) eq;

}

Constant *
createConstInt (int value)
{
    Constant *result = makeNode(Constant);
    int *v = NEW(int);

    *v = value;
    result->constType = DT_INT;
    result->value = v;
    result->isNull = FALSE;

    return result;
}

Constant *
createConstLong (gprom_long_t value)
{
    Constant *result = makeNode(Constant);
    gprom_long_t *v = NEW(gprom_long_t);

    *v = value;
    result->constType  = DT_LONG;
    result->value = v;
    result->isNull = FALSE;

    return result;
}

Constant *
createConstString (char *value)
{
    Constant *result = makeNode(Constant);

    result->constType = DT_STRING;
    result->value = strdup(value);
    result->isNull = FALSE;

    return result;
}

Constant *
createConstFloat (double value)
{
    Constant *result = makeNode(Constant);
    double *v = NEW(double);

    *v = value;
    result->constType = DT_FLOAT;
    result->value = v;
    result->isNull = FALSE;

    return result;
}

Constant *
createConstBoolFromString (char *v)
{
    if (streq(v,"TRUE") || streq(v,"t") || streq(v,"true"))
        return createConstBool(TRUE);
    if (streq(v,"FALSE") || streq(v,"f") || streq(v,"false"))
        return createConstBool(FALSE);
    FATAL_LOG("not a boolean value %s", v);
    return FALSE; //keep compiler quiet
}

Constant *
createConstBool (boolean value)
{
    Constant *result = makeNode(Constant);
    boolean *v = NEW(boolean);

    *v = value;
    result->constType = DT_BOOL;
    result->value = v;
    result->isNull = FALSE;

    return result;
}

Constant *
createNullConst (DataType dt)
{
    Constant *result = makeNode(Constant);

    result->constType = dt;
    result->value = NULL;
    result->isNull = TRUE;

    return result;
}

Constant *
makeConst(DataType dt)
{
	switch(dt)
	{
	case DT_INT:
		return createConstInt(0);
	case DT_FLOAT:
		return createConstFloat(0.0);
	case DT_BOOL:
		return createConstBool(TRUE);
	case DT_LONG:
		return createConstLong(0L);
	case DT_STRING:
		return createConstString("");
	case DT_VARCHAR2:
	{
		Constant *result = createConstString("");
	    result->constType = DT_VARCHAR2;
		return result;
	}
	}
    return NULL;
}

Constant *
minConsts(Constant *l, Constant *r, boolean nullIsMin)
{
	Constant *result;


	if(l->isNull || r->isNull)
	{
		if (nullIsMin)
		{
			return createNullConst(l->constType);
		}
		if(l->isNull)
			return copyObject(r);
		return copyObject(l);
	}

	result = makeConst(l->constType);

	switch(result->constType)
	{
	case DT_INT:
	{
		INT_VALUE(result) = INT_VALUE(l) < INT_VALUE(r) ? INT_VALUE(l) : INT_VALUE(r);
	}
	case DT_FLOAT:
	{
		FLOAT_VALUE(result) = FLOAT_VALUE(l) < FLOAT_VALUE(r) ? FLOAT_VALUE(l) : FLOAT_VALUE(r);
	}
	case DT_STRING:
	case DT_VARCHAR2:
	{
	    result->value = strcmp(STRING_VALUE(l),STRING_VALUE(r)) < 0 ? STRING_VALUE(l) : STRING_VALUE(r);
	}
	case DT_BOOL:
	{
		BOOL_VALUE(result) = BOOL_VALUE(l) < BOOL_VALUE(r) ? BOOL_VALUE(l) : BOOL_VALUE(r);
	}
	case DT_LONG:
	{
		LONG_VALUE(result) = LONG_VALUE(l) < LONG_VALUE(r) ? LONG_VALUE(l) : LONG_VALUE(r);
	}
	}

	return result;
}

Constant *
maxConsts(Constant *l, Constant *r, boolean nullIsMax)
{
	Constant *result;

	if(l->isNull || r->isNull)
	{
		if (nullIsMax)
		{
			return createNullConst(l->constType);
		}
		if(l->isNull)
			return copyObject(r);
		return copyObject(l);
	}

	result = makeConst(l->constType);

	switch(result->constType)
	{
	case DT_INT:
	{
		INT_VALUE(result) = INT_VALUE(l) >= INT_VALUE(r) ? INT_VALUE(l) : INT_VALUE(r);
	}
	case DT_FLOAT:
	{
		FLOAT_VALUE(result) = FLOAT_VALUE(l) >= FLOAT_VALUE(r) ? FLOAT_VALUE(l) : FLOAT_VALUE(r);
	}
	case DT_STRING:
	case DT_VARCHAR2:
	{
	    result->value = strcmp(STRING_VALUE(l),STRING_VALUE(r)) >= 0 ? STRING_VALUE(l) : STRING_VALUE(r);
	}
	case DT_BOOL:
	{
		BOOL_VALUE(result) = BOOL_VALUE(l) >= BOOL_VALUE(r) ? BOOL_VALUE(l) : BOOL_VALUE(r);
	}
	case DT_LONG:
	{
		LONG_VALUE(result) = LONG_VALUE(l) >= LONG_VALUE(r) ? LONG_VALUE(l) : LONG_VALUE(r);
	}
	}

	return result;
}

void
incrConst(Constant *c)
{
	ASSERT(!c->isNull && (c->constType == DT_INT || c->constType == DT_LONG));

	switch(c->constType)
	{
	case DT_INT:
		INT_VALUE(c) = INT_VALUE(c) + 1;
		break;
	case DT_LONG:
		LONG_VALUE(c) = LONG_VALUE(c) + 1;
		break;
	default:
		c = NULL;
		// will never end up here
	}
}

DataType
typeOf (Node *expr)
{
    switch(expr->type)
    {
    	case T_AttributeReference:
        {
            AttributeReference *a = (AttributeReference *) expr;
            return a->attrType;
        }
        case T_Constant:
        {
            Constant *c = (Constant *) expr;
            return c->constType;
        }
        case T_FunctionCall:
        {
            FunctionCall *f = (FunctionCall *) expr;
            return typeOfFunc(f);
        }
        case T_WindowFunction:
            return typeOf((Node *) ((WindowFunction *) expr)->f);
        case T_Operator:
        {
            Operator *o = (Operator *) expr;
            boolean exists = FALSE;
            return typeOfOp(o, &exists);
        }
        case T_CaseExpr:
        {
            CaseExpr *c = (CaseExpr *) expr;
            CaseWhen *w = (CaseWhen *) getNthOfListP(c->whenClauses, 0);
            return typeOf(w->then);
        }
        case T_IsNullExpr:
            return DT_BOOL;
			// Oracle has an ROWNUM pseudo attribute for that. All other systems we have to use ROW_NUMBER()
        case T_RowNumExpr:
			if(getBackend() == BACKEND_ORACLE)
			{
				return DT_INT;
			}
			else
			{
				DataType rt;
				boolean exists;
				rt = getFuncReturnType(ROW_NUMBER_FUNC_NAME, NIL, &exists);
				return rt;
			}
        case T_SQLParameter:
            return ((SQLParameter *) expr)->parType;
        case T_OrderExpr:
            return DT_INT;//TODO should use something else?
        case T_QuantifiedComparison:
            return DT_BOOL;
        case T_DLVar:
            return ((DLVar *) expr)->dt;
        case T_CastExpr:
            return ((CastExpr *) expr)->resultDT;
        case T_NestedSubquery:
        {
            NestedSubquery *q = (NestedSubquery *) expr;

            switch(q->nestingType)
            {
                case NESTQ_EXISTS:
                case NESTQ_ANY:
                case NESTQ_ALL:
                case NESTQ_UNIQUE:
                {
                    return DT_BOOL;
                }
                case NESTQ_SCALAR:
                case NESTQ_LATERAL:
                {
                    //return DT_LONG; //TODO
                		return q->nestingAttrDatatype;
                }
                break;
            }
        }
        break;
        default:
             FATAL_LOG("unknown expression type for node: %s", nodeToString(expr));
             break;
    }
    return DT_STRING;
}

boolean
isConstExpr (Node *expr)
{
    if (expr == NULL)
        return TRUE;

    switch(expr->type)
    {
        case T_List:
        {
            FOREACH(Node,e,(List *) expr)
            {
                if (!isConstExpr(e))
                    return FALSE;
            }
            return TRUE;
        }
        case T_AttributeReference:
            return FALSE;
        case T_Constant:
            return TRUE;
        case T_FunctionCall:
        {
            FunctionCall *f = (FunctionCall *) expr;
            FOREACH(Node,arg,f->args)
            {
                if (!isConstExpr(arg))
                    return FALSE;
            }
            return TRUE;
        }
        case T_WindowFunction:
        {
            //TODO
            return FALSE;
        }
        case T_Operator:
        {
            Operator *o = (Operator *) expr;
            FOREACH(Node,arg,o->args)
            {
                if (!isConstExpr(arg))
                    return FALSE;
            }
            return TRUE;
        }
        case T_CaseExpr:
        {
            CaseExpr *c = (CaseExpr *) expr;
            if (!isConstExpr(c->expr))
                return FALSE;
            FOREACH(Node,w,c->whenClauses)
            {
                if (!isConstExpr(w))
                    return FALSE;
            }
            return isConstExpr(c->elseRes);
        }
        case T_CaseWhen:
        {
            CaseWhen *w = (CaseWhen *) expr;
            if (!isConstExpr(w->when))
                return FALSE;
            return isConstExpr(w->then);
        }
        case T_IsNullExpr:
        {
            IsNullExpr *n = (IsNullExpr *) expr;
            return isConstExpr(n->expr);
        }
        case T_RowNumExpr:
            return FALSE;
        case T_SQLParameter:
            return FALSE;
        case T_OrderExpr:
        {
            OrderExpr *o = (OrderExpr *) expr;
            return isConstExpr(o->expr);
        }
        case T_QuantifiedComparison:
        {
        		QuantifiedComparison *o = (QuantifiedComparison *) expr;
            return isConstExpr(o->checkExpr);
        }
        case T_DLVar:
            return FALSE;
        case T_CastExpr:
        {
            CastExpr *c = (CastExpr *) expr;
            return isConstExpr(c->expr);
        }
        default:
             FATAL_LOG("unknown expression type for node: %s", nodeToString(expr));
             break;
    }
    return FALSE;
}

boolean
isCondition(Node *expr)
{
    if (isA(expr,Operator))
    {
        Operator *o = (Operator *) expr;
        if (streq(o->name, OPNAME_EQ)
                || streq(o->name, OPNAME_LT)
                || streq(o->name, OPNAME_GT)
                || streq(o->name, "!=")
            ) //TODO what else
            return TRUE;
    }
    if (isA(expr,CaseWhen))
        return TRUE;

    return FALSE;
}

boolean
isAggFunction(Node *expr)
{
	if(isA(expr,FunctionCall))
	{
		FunctionCall *f = (FunctionCall *) expr;
		return f->isAgg;
	}

	return FALSE;
}

char *
getAttributeReferenceName(AttributeReference *a)
{
    return a->name;
}

char *
backendifyIdentifier(char *name)
{
    char *result;

    // remove quotes of quoted identifier
    if (strlen(name) > 0 && name[0] == '"')
    {
        result = substr(name, 1, strlen(name) - 2);
		// SQLite ignores all cases for matching, make sure we do too!
		if (getBackend() == BACKEND_SQLITE)
			result = strToUpper(result);
    }
    // non quoted part upcase or downcase based on database system
    else
    {
        switch(getBackend())
        {
            case BACKEND_ORACLE:
                result = strToUpper(name);
                break;
            case BACKEND_POSTGRES:
            case BACKEND_MONETDB:
                result = strToLower(name);
                break;
		    case BACKEND_SQLITE: // treat everything as upper case since SQLite completely ignores all cases when it comes to matching attribute names even through internally identifiers are stored case sensitive
				result = strToUpper(name);
				break;
            case BACKEND_DUCKDB:
				result = name;
				break;
		    case BACKEND_MSSQL:
				result = strToLower(name);
				break;
            default:
                result = strToUpper(name);
                break;
        }
    }

    return result;
}


List *
createCasts(Node *lExpr, Node *rExpr)
{
	DataType lType, rType;
    lType = typeOf(lExpr);
    rType = typeOf(rExpr);

    // same expr return inputs
    if (lType == rType)
        return LIST_MAKE(lExpr,rExpr);

    // one is DT_STRING, cast the other one
    if (lType == DT_STRING)
        return LIST_MAKE(lExpr, createCastExpr(rExpr, DT_STRING));
    if (rType == DT_STRING)
        return LIST_MAKE(createCastExpr(lExpr, DT_STRING), rExpr);

    return LIST_MAKE(createCastExpr(lExpr, DT_STRING), createCastExpr(rExpr, DT_STRING));
}

Node *
addCastsToExpr(Node *expr, boolean errorOnFailure)
{
    return addCastsMutator(expr, errorOnFailure);
}

static Node *
addCastsMutator (Node *node, boolean errorOnFailure)
{
    Node *casted = node;

    if (node == NULL)
        return NULL;

    // deal with nodes that have childres, process children first
    switch(node->type)
    {
        case T_List:
        {
            FOREACH(Node,e,(List *) node)
            {
                addCastsMutator(e, errorOnFailure);
            }
        }
        break;
        case T_AttributeReference:
        case T_Constant:
        case T_DLVar:
        case T_IsNullExpr:
        case T_RowNumExpr:
        case T_SQLParameter:
        case T_QuantifiedComparison:
        case T_OrderExpr:
        {
            return node;
        }
        case T_FunctionCall:
        {
            FunctionCall *f = (FunctionCall *) node;
            FOREACH(Node,arg,f->args)
                arg_his_cell->data.ptr_value = addCastsMutator(arg, errorOnFailure);
            castFunctionArgs(f);
        }
        break;
        case T_WindowFunction:
        {
            WindowFunction *w = (WindowFunction *) node;
            FunctionCall *f = w->f;
            FOREACH(Node,arg,f->args)
                arg_his_cell->data.ptr_value = addCastsMutator(arg, errorOnFailure);
            castFunctionArgs(f);
        }
        break;
        case T_Operator:
        {
            Operator *o = (Operator *) node;
            FOREACH(Node,arg,o->args)
                arg_his_cell->data.ptr_value = addCastsMutator(arg, errorOnFailure);
            castOperatorArgs(o);
        }
        break;
        case T_CaseExpr:
        {
            CaseExpr *c = (CaseExpr *) node;
            DataType whenLca;

            if (c->elseRes)
                whenLca = typeOf(c->elseRes);
            else
            {
                CaseWhen *w = (CaseWhen *) getNthOfList(c->whenClauses,0);
                whenLca = typeOf(w->then);
            }
            FOREACH(CaseWhen,w, c->whenClauses)
            {
                whenLca = lcaType(whenLca,typeOf(w->then));
            }

            FOREACH(CaseWhen,w, c->whenClauses)
            {
                if(whenLca != typeOf(w->then))
                {
                    w->then = (Node *) createCastExpr(w->then, whenLca);
                }
            }

            return node;
        }
        case T_CastExpr://TODO
            return node;
        default:
            FATAL_LOG("unknown expression type for node: %s", nodeToString(node));
            break;
    }

    return casted;
}

#define REPLICATE_ELEM(_result, _elem, _num) \
    do { \
        for(int _i = 0 ; _i < _num; _i++) \
        { \
            _result = appendToTailOfListInt(_result, _elem); \
        } \
    } while(0)

static void
castOperatorArgs(Operator *o)
{
    List *args = o->args;
    List *inTypes = NIL;
    DataType commonLca;
    List *curTypes = NIL;
    int numArgs = LIST_LENGTH(o->args);

    // determine expression types
    FOREACH(Node,arg,args)
    {
        inTypes = appendToTailOfListInt(inTypes, typeOf(arg));
    }

    // determine a common type these expressions can be cast into
    commonLca = getNthOfListInt(inTypes, 0);
    FOREACH_INT(dt, inTypes)
    {
        commonLca = lcaType(dt, commonLca);
    }

    DataType previousType = DT_INT;

    // check whether function exists for common type or any type this type can be cast into
    while(previousType != DT_STRING)
    {
        DEBUG_LOG("try type %s", DataTypeToString(commonLca));
        curTypes = NIL;
        REPLICATE_ELEM(curTypes, commonLca, numArgs);
        if (opExists(o->name, curTypes))
        {
            FOREACH(Node,arg,o->args)
            {
                if (typeOf(arg) != commonLca)
                    arg_his_cell->data.ptr_value = createCastExpr(arg, commonLca);
            }
            return;
        }
        previousType = commonLca;
        commonLca = nextCastableType(commonLca);
    }

    ERROR_NODE_BEATIFY_LOG("did not find a way to cast arguments of operator", o);
}




static void
castFunctionArgs(FunctionCall *f)
{
    List *args = f->args;
    List *inTypes = NIL;
    List *curTypes = NIL;
    int numArgs = LIST_LENGTH(inTypes);
    char *fName = f->functionname;
    DataType commonLca;

    // determine expression types
    inTypes = typeOfArgs(args);

    if (funcExists(fName, inTypes))
        return;

    // determine a common type these expressions can be cast into
    //TODO deal with casting when function takes different arguments
    commonLca = getNthOfListInt(inTypes, 0);
    FOREACH_INT(dt, inTypes)
    {
        commonLca = lcaType(dt, commonLca);
    }

    DataType previousType = DT_INT;

    // check whether function exists for common type or any type this type can be cast into
    while(previousType != DT_STRING)
    {
        REPLICATE_ELEM(curTypes, commonLca, numArgs);
        if (funcExists(fName, curTypes))
        {
            FOREACH(Node,arg,f->args)
            {
                arg_his_cell->data.ptr_value = createCastExpr(arg, commonLca);
            }
            return;
        }
        previousType = commonLca;
        commonLca = nextCastableType(commonLca);
    }

    ERROR_NODE_BEATIFY_LOG("did not find a way to cast arguments of function", f);
}

DataType
lcaTypes(List *types)
{
	List *ts = copyList(types);
	DataType result = popHeadOfListInt(ts);

	FOREACH_INT(dt, ts)
	{
		result = lcaType(result, dt);
	}

	return result;
}

DataType
lcaType(DataType l, DataType r)
{
    Set *lCasts = INTSET();
    Set *rCasts = INTSET();

    // same type no cast needed
    if (l == r)
        return l;

    int i = 0;
    for (CastGraphEdge e = castGraph[i]; castGraph[i].in != stopper.in || castGraph[i].out != stopper.out; e = castGraph[i++])
    {
        if (e.in == l)
            addIntToSet(lCasts, e.out);
        if (e.in == r)
            addIntToSet(rCasts, e.out);
    }

    if (hasSetIntElem(rCasts,l))
        return l;
    if (hasSetIntElem(lCasts,r))
        return r;

    i = 0;
    for(int d = typePreferences[i]; typePreferences[i] != PREFSTOPPER; d = typePreferences[i++])
    {
        if (hasSetIntElem(lCasts,d) && hasSetIntElem(rCasts,d))
            return d;
    }

    return DT_STRING;
}

DataType
SQLdataTypeToDataType (char *dt)
{
    return backendSQLTypeToDT (dt);
}

DataType
typeOfInOpModel (Node *expr, List *inputOperators)
{
    switch(expr->type)
    {
        case T_AttributeReference:
            return ((AttributeReference *) expr)->attrType;
        case T_FunctionCall:
            return typeOfFunc((FunctionCall *) expr);
        case T_Operator:
        {
            boolean exists = FALSE;
            return typeOfOp((Operator *) expr, &exists);
        }
        case T_Constant:
            return typeOf(expr);
        case T_CaseExpr:
        {
            CaseExpr *c = (CaseExpr *) expr;
            CaseWhen *w = (CaseWhen *) getNthOfListP(c->whenClauses, 0);
            return typeOf(w->then);
        }
        case T_RowNumExpr:
            return DT_INT;
        case T_CastExpr:
            return ((CastExpr *) expr)->resultDT;
        default:
            ERROR_LOG("unknown expression type for node: %s", nodeToString(expr));
            break;
    }
    return DT_STRING;
}

/* figure out operator return type */
static DataType
typeOfOp (Operator *op, boolean *exists)
{
    char *opName = op->name;
    List *dts = typeOfArgs(op->args);
    DataType result = typeOfOpSplit(opName, dts, exists);

    if (!*exists)
        DEBUG_NODE_BEATIFY_LOG("operator with this argument types does not exist",op);
    return result;
}

static DataType
typeOfOpSplit (char *opName, List *argDTs, boolean *exists)
{
    *exists = TRUE;
    char *upCaseOpName = strToUpper(opName);
    DataType result = DT_INT;
    DataType dLeft = DT_INT;
    DataType dRight = DT_INT;

    DEBUG_LOG("check whether op <%s> exists with argument types <%s>", opName, nodeToString(argDTs));

    if (LIST_LENGTH(argDTs) >= 1)
        dLeft = getNthOfListInt(argDTs,0);
    if (LIST_LENGTH(argDTs) >= 2)
        dRight = getNthOfListInt(argDTs,1);

    // logical operators
    if (streq(upCaseOpName,OPNAME_OR)
            || streq(upCaseOpName,OPNAME_AND))
    {
        if (dLeft == dRight && dLeft == DT_BOOL)
            return DT_BOOL;
    }

    // TODO: operator name is OPNAME_NOT or OPNAME_not
    if (streq(opName,OPNAME_NOT) || streq(opName,OPNAME_not))
    {
        if (dLeft == DT_BOOL)
            return DT_BOOL;
    }

    // standard arithmetic operators
    if (streq(opName,OPNAME_ADD)
		|| streq(opName,OPNAME_MULT)
		|| streq(opName,OPNAME_DIV)
		|| streq(opName,OPNAME_MINUS)
		|| streq(opName,OPNAME_MOD)
		)
    {
        // if the same input data types then we can safely assume that we get the same return data type
        // otherwise we use the metadata lookup plugin to make sure we get the right type
        if(dLeft == dRight && (dLeft == DT_INT || dLeft == DT_FLOAT || dLeft == DT_LONG))
            return dLeft;

        // if a int with a float, the result type should be float
        if((dLeft == DT_INT && dRight == DT_FLOAT) || (dLeft == DT_FLOAT && dRight == DT_INT))
            return DT_FLOAT;
    }

    // string ops
    if (streq(opName,OPNAME_STRING_CONCAT))
    {
        if (dLeft == dRight && dLeft == DT_STRING)
            return DT_STRING;
    }

    // comparison operators
    if (streq(opName,OPNAME_LT)
		|| streq(opName,OPNAME_GT)
		|| streq(opName,OPNAME_LE)
		|| streq(opName,OPNAME_GE)
		|| streq(opName,"=>") //TODO is this supposed ot be >=?
		|| streq(opName,OPNAME_NEQ)
		|| streq(opName,OPNAME_NEQ_HAT)
		|| streq(opName,OPNAME_EQ)
		|| streq(opName,OPNAME_NEQ_BANG)
		|| streq(opName,OPNAME_LIKE)
    )
    {
       return DT_BOOL;
    }

    *exists = FALSE;

    result = getOpReturnType(opName, argDTs, exists);

    return result;
}

static boolean
opExists (char *opName, List *argDTs)
{
    boolean fExists = FALSE;

    typeOfOpSplit(opName, argDTs, &fExists);

    return fExists;
}

static List*
typeOfArgs(List* args)
{
    List *argDTs = NIL;
    FOREACH(Node,arg,args)
        argDTs = appendToTailOfListInt(argDTs, typeOf(arg));
    return argDTs;
}

/* figure out function return type */
static DataType
typeOfFunc(FunctionCall *f)
{
    List *argDTs = NIL;
    boolean fExists = FALSE;
    DataType result;

	if (strieq(f->functionname, COALESCE_FUNC_NAME))
	{
		return lcaTypes(mapToIntList(f->args, (int (*) (void *)) typeOf));
	}

    if (strieq(f->functionname, UNCERTAIN_MAKER_FUNC_NAME))
    {
        return getNthOfListInt(typeOfArgs(f->args), 0);
    }

    argDTs = typeOfArgs(f->args);
    result = getFuncReturnType(f->functionname, argDTs, &fExists);
    if (!fExists)
        DEBUG_NODE_BEATIFY_LOG("Function does not exist: ", f);

    return result;
}

static boolean
funcExists(char *fName, List *argDTs)
{
    boolean fExists = FALSE;

    // uncertainty dummy functions
    if (strieq(fName, UNCERTAIN_MAKER_FUNC_NAME))
    {
        return TRUE;
    }

    getFuncReturnType(fName, argDTs, &fExists);

    return fExists;
}
static Set *
castsAsSet(DataType in)
{
    Set *parents = INTSET();
    int i = 0;
    for (CastGraphEdge e = castGraph[i];
            castGraph[i].in != stopper.in || castGraph[i].out != stopper.out;
            e = castGraph[i++])
    {
        if (e.in == in)
            addIntToSet(parents, e.out);
    }
    return parents;
}

static DataType
nextCastableType (DataType in)
{
    Set *parents = INTSET();

    if (in == DT_STRING)
        return in;

    parents = castsAsSet(in);

    int i = 0;
    for(DataType d = typePreferences[i]; typePreferences[i] != PREFSTOPPER; d = typePreferences[i++])
    {
        if (hasSetIntElem(parents,d))
            return d;
    }

    return DT_STRING;
}



/* search functions */
List *
getAttrReferences (Node *node)
{
    List *result = NIL;

    findAttrReferences(node, &result);

    return result;
}

static boolean
findAttrReferences (Node *node, List **state)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, AttributeReference))
    {
        *state = appendToTailOfList(*state, node);
    }

    return visit(node, findAttrReferences, state);
}

List *
exprGetReferencedAttrNames(Node *node)
{
	List *refs = getAttrReferences(node);
	List *names = NIL;

	FOREACH(AttributeReference,a,refs)
	{
		names = appendToTailOfList(names, strdup(a->name));
	}

	return names;
}

boolean
doesExprReferenceAttribute(Node *expr, char *a)
{
	List *names = exprGetReferencedAttrNames(expr);
	return searchListString(names, a);
}

List *
getDLVars (Node *node)
{
    List *result = NIL;

    findDLVars(node, &result);

    return result;
}

static boolean
findDLVars (Node *node, List **state)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, DLVar))
        *state = appendToTailOfList(*state, node);

    return visit(node, findDLVars, state);
}

List *
getDLVarsIgnoreProps (Node *node)
{
    List *result = NIL;

    findDLVarsIgnoreProps(node, &result);

    return result;
}

static boolean
findDLVarsIgnoreProps (Node *node, List **state)
{
    if (node == NULL)
        return TRUE;

    if (isA(node, HashMap))
        return TRUE;

    if (isA(node, DLVar))
        *state = appendToTailOfList(*state, node);

    return visit(node, findDLVars, state);
}

//TODO this is unsafe, callers are passing op as an operator even though it may not be one.
void
getSelectionCondOperatorList(Node *expr, List **opList)
{
    // only are interested in operators here
	if (isA(expr,Operator)) {
	    Operator *op = (Operator *) copyObject(expr);

	    // uppercase operator name
	    char *opName = op->name;
	    while (*opName) {
	      *opName = toupper((unsigned char) *opName);
	      opName++;
	    }

	    if(streq(op->name,OPNAME_AND))
	    {
	        FOREACH(Node,arg,op->args)
                getSelectionCondOperatorList(arg,opList);
	    }
	    else
	        *opList = appendToTailOfList(*opList, op);
	}
}

Node *
changeListOpToAnOpNode(List *l1)
{
    List *helpList;
    Node *opNode1;

    if (LIST_LENGTH(l1) == 2)
        opNode1 = (Node *) createOpExpr(OPNAME_AND, (List *) l1);
    else if(LIST_LENGTH(l1) > 2)
    {
        int i;
        helpList = NIL;
        Operator *helpO1 = getHeadOfListP(l1);
        l1 = REMOVE_FROM_LIST_PTR(l1, helpO1);
        Operator *helpO2 = getHeadOfListP(l1);
        l1 = REMOVE_FROM_LIST_PTR(l1, helpO2);
        helpList = appendToTailOfList(helpList, helpO1);
        helpList = appendToTailOfList(helpList, helpO2);

	Operator *helpO = createOpExpr(OPNAME_AND, (List *) helpList);
        int length_l1 = LIST_LENGTH(l1);

        for(i=0; i<length_l1; i++)
        {
            helpList = NIL;
            helpList = appendToTailOfList(helpList, helpO);
            helpO = getHeadOfListP(l1);
            l1 = REMOVE_FROM_LIST_PTR(l1, helpO);
            helpList = appendToTailOfList(helpList, helpO);
            helpO =  createOpExpr(OPNAME_AND, (List *) helpList);
        }
        opNode1 = (Node *)helpO;
    }
    else
        opNode1 = (Node *) getHeadOfListP(l1);

    return opNode1;
}

List *
findAllNodes(Node *node, NodeTag type)
{
    FindNotesContext *context = NEW(FindNotesContext);
    List *result = NULL;

    context->type = type;
    context->result = &result;

    findAllNodesVisitor(node, context);

    FREE(context);

    return result;
}


static boolean
findAllNodesVisitor(Node *node, FindNotesContext *context)
{
    if (node == NULL)
       return TRUE;

    if ((node->type) == context->type)
    {
        *(context->result) = appendToTailOfList(*(context->result), node);
        return TRUE;
    }

    return visit(node, findAllNodesVisitor, context);
}

/*-----------------------------------------------------------------------------
 *
 * expr_to_sql.c
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

#include "configuration/option.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"
#include "model/datalog/datalog_model.h"
#include "metadata_lookup/metadata_lookup.h"
#include "model/query_block/query_block.h"
#include "model/query_operator/query_operator.h"
#include "utility/string_utils.h"

/* function declarations */
static void exprToSQLString(StringInfo str, Node *expr, HashMap *nestedSubqueries);
static void exprToLatexString(StringInfo str,  Node *expr, HashMap *nestedSubqueries);
static void attributeReferenceToSQL (StringInfo str, AttributeReference *node, HashMap *nestedSubqueries);
static void constantToSQL (StringInfo str, Constant *node);
static void functionCallToSQL (StringInfo str, FunctionCall *node, HashMap *nestedSubqueries);
static void operatorToSQL (StringInfo str, Operator *node, HashMap *nestedSubqueries);
static void caseToSQL(StringInfo str, CaseExpr *expr, HashMap *nestedSubqueries);
static void winFuncToSQL(StringInfo str, WindowFunction *expr, HashMap *nestedSubqueries);
static void winBoundToSQL (StringInfo str, WindowBound *b, HashMap *nestedSubqueries);
static void orderExprToSQL (StringInfo str, OrderExpr *o, HashMap *nestedSubqueries);
static void dataTypeToSQL (StringInfo str, DataType dt);

static void functionCallToLatex (StringInfo str, FunctionCall *node, HashMap *nestedSubqueries);
static void operatorToLatex (StringInfo str, Operator *node, HashMap *nestedSubqueries);
static void constantToLatex (StringInfo str, Constant *node);
static void attributeReferenceToLatex (StringInfo str, AttributeReference *node);

//xmlelement first arg
static void xmlConstantToSQL (StringInfo str, Node *node);

static void
attributeReferenceToSQL (StringInfo str, AttributeReference *node, HashMap *nestedSubqueries)
{
	if (nestedSubqueries != NULL && hasMapStringKey(nestedSubqueries, node->name))
	{
		//TODO serialize query instead  (store string instead)
	}
	else
	{
		appendStringInfoString(str, node->name);
	}
}

static void
constantToSQL (StringInfo str, Constant *node)
{
    if (node->isNull)
    {
        appendStringInfoString(str, "NULL");
        return;
    }

    switch(node->constType)
    {
        case DT_INT:
            appendStringInfo(str, "%d", *((int *) node->value));
            break;
        case DT_FLOAT:
            appendStringInfo(str, "%f", *((double *) node->value));
            break;
        case DT_LONG:
            appendStringInfo(str, "%ld", *((gprom_long_t *) node->value));
            break;
        case DT_STRING:
            appendStringInfo(str, "'%s'", (char *) node->value);
            break;
        case DT_BOOL:
            appendStringInfo(str, "%s", *((boolean *) node->value) == TRUE ? "TRUE" : "FALSE");
            break;
        case DT_VARCHAR2:
            appendStringInfo(str, "'%s'", (char *) node->value);
            break;
    }
}

static void
xmlConstantToSQL (StringInfo str, Node *node)
{
	if(node->type == T_Constant)
	{
		Constant *n = (Constant *) node;

		if (n->isNull)
		{
			appendStringInfoString(str, "NULL");
			return;
		}

		if(n->constType == DT_STRING)
			appendStringInfo(str, "%s", (char *) n->value);
	}
}


static void
functionCallToSQL (StringInfo str, FunctionCall *node, HashMap *nestedSubqueries)
{

    int flag = 0;
    if (streq(node->functionname, "AGG_STRAGG"))
    {
        flag = 1;
        /* Zeroth approach our first version */
        //appendStringInfoString(str, "replace(rtrim(extract(xmlagg(xmlelement(E,");
        /* Zhen 1st approach */
        //appendStringInfoString(str, "replace(rtrim(xmlquery('/E/text()' passing xmlagg(xmlelement(E,");
        /* Zhen 2nd approach */
        //appendStringInfoString(str, "replace(rtrim(xmlagg(XMLParse(content ");
        /* Zhen 3rd approach */
        //appendStringInfoString(str, "replace(rtrim(xmlserialize(content xmlagg(XMLParse(content ");
        /* Zhen 4th approach modified 3rd*/
        appendStringInfoString(str, "replace(rtrim(xmlserialize(content xmlagg(xmlcdata( ");
        /* Zhen 5th approach modified 2nd */
        //appendStringInfoString(str, "replace(rtrim(xmlagg(xmlcdata( ");
    }
    else
        appendStringInfoString(str, node->functionname);

    appendStringInfoString(str, "(");
    if (node->isDistinct)
    {
        appendStringInfoString(str, "DISTINCT ");
    }

    int i = 0;
    //Node *entity;
    int xmlCnt = 0; //used to get the first element of xmlelement like Constant 'tuple', then I want to remove ''
    FOREACH(Node,arg,node->args)
    {
        appendStringInfoString(str, ((i++ == 0) ? "" : ", "));
        if (streq(node->functionname, "XMLELEMENT") && xmlCnt == 0)
        	xmlConstantToSQL(str, arg);
        else
        	exprToSQLString(str, arg, nestedSubqueries);
        //entity = arg;
        xmlCnt ++;
    }

    if (flag == 1)
    {
        /* Zeroth approach our first version */
        //appendStringInfoString(str, "))),'/E/text()').getclobval(),','),chr(38) || 'quot;','\"'");
        /* Zhen 1st approach */
        //appendStringInfoString(str, "))) returning content).getclobval(),','),chr(38) || 'quot;','\"'");
        /* Zhen 2nd approach */
        //appendStringInfoString(str, "))).getclobval(),','),chr(38) || 'quot;','\"'");
        /* Zhen 3rd approach */
        //appendStringInfoString(str, "))) as clob),','),chr(38) || 'quot;','\"'");
        /* Zhen 4th approach modified 3rd*/
        appendStringInfoString(str, "), 1)) as clob),','),chr(38) || 'quot;','\"'");
        /* Zhen 5th approach modified 2nd */
        //appendStringInfoString(str, "), 1)).getclobval(),','),chr(38) || 'quot;','\"'");

        //    	appendStringInfoString(str, "',')");
        //    	appendStringInfoString(str, " WITHIN GROUP (ORDER BY ");
        //    	exprToSQLString(str, entity);
    }
    appendStringInfoString(str,")");

    /*	int flag = 0;
	if (streq(node->functionname, "AGG_STRAGG"))
	{
		flag = 1;
		appendStringInfoString(str, "LISTAGG");
	}
	else
		appendStringInfoString(str, node->functionname);

    appendStringInfoString(str, "(");

    int i = 0;
    Node *entity;
    FOREACH(Node,arg,node->args)
    {
        appendStringInfoString(str, ((i++ == 0) ? "" : ", "));
        exprToSQLString(str, arg);
        entity = arg;
    }

    if (flag == 1)
    {
    	appendStringInfoChar(str, ',');
    	appendStringInfoString(str, "',')");
    	appendStringInfoString(str, " WITHIN GROUP (ORDER BY ");
    	exprToSQLString(str, entity);
    }
    appendStringInfoString(str,")");*/
}

static void
operatorToSQL (StringInfo str, Operator *node, HashMap *nestedSubqueries)
{
    // handle special operators
    if (streq(node->name,"BETWEEN"))
    {
        char *expr = exprToSQL(getNthOfListP(node->args,0), NULL);
        char *lower = exprToSQL(getNthOfListP(node->args,1), NULL);
        char *upper = exprToSQL(getNthOfListP(node->args,2), NULL);

        appendStringInfo(str, "(%s BETWEEN %s AND %s)", expr, lower, upper);
    }
    else if (LIST_LENGTH(node->args) == 1)
    {
        appendStringInfo(str, "(%s ", node->name);
        appendStringInfoString(str, "(");
        exprToSQLString(str,getNthOfListP(node->args,0), nestedSubqueries);
        appendStringInfoString(str, "))");
    }
    else
    {
        appendStringInfoString(str, "(");

        FOREACH(Node,arg,node->args)
        {
            exprToSQLString(str,arg, nestedSubqueries);
            if(arg_his_cell != node->args->tail)
                appendStringInfo(str, " %s ", node->name);
        }

        appendStringInfoString(str, ")");
    }
}

static void
caseToSQL(StringInfo str, CaseExpr *expr, HashMap *nestedSubqueries)
{
    appendStringInfoString(str, "(CASE ");

    // CASE expression
    if (expr->expr != NULL)
    {
        exprToSQLString(str, expr->expr, nestedSubqueries);
        appendStringInfoString(str, " ");
    }

    // WHEN ... THEN ...
    FOREACH(CaseWhen,w,expr->whenClauses)
    {
        appendStringInfoString(str, " WHEN ");
        exprToSQLString(str, w->when, nestedSubqueries);
        appendStringInfoString(str, " THEN ");
        exprToSQLString(str, w->then, nestedSubqueries);
    }

    // ELSE
    if (expr->elseRes != NULL)
    {
        appendStringInfoString(str, " ELSE ");
        exprToSQLString(str, expr->elseRes, nestedSubqueries);
    }
    appendStringInfoString(str, " END)");
}

static void
winFuncToSQL(StringInfo str, WindowFunction *expr, HashMap *nestedSubqueries)
{
    WindowDef *w = expr->win;

    // the function call
    functionCallToSQL(str, expr->f, nestedSubqueries);

    // OVER clause
    appendStringInfoString(str, " OVER (");
    if (w->partitionBy != NULL)
    {
        appendStringInfoString(str, "PARTITION BY ");
        FOREACH(Node,p,w->partitionBy)
        {
            exprToSQLString(str, p, nestedSubqueries);
            if(FOREACH_HAS_MORE(p))
                appendStringInfoString(str, ", ");
        }
    }
    if (w->orderBy != NULL)
    {
        appendStringInfoString(str, " ORDER BY ");

        FOREACH(Node,o,w->orderBy)
        {
            exprToSQLString(str, o, nestedSubqueries);
            if(FOREACH_HAS_MORE(o))
                appendStringInfoString(str, ", ");
        }

    }
    if (w->frame != NULL)
    {
        WindowFrame *f = w->frame;
        switch(f->frameType)
        {
            case WINFRAME_ROWS:
                appendStringInfoString(str, " ROWS ");
                break;
            case WINFRAME_RANGE:
                appendStringInfoString(str, " RANGE ");
                break;
        }
        if (f->higher)
        {
            appendStringInfoString(str, "BETWEEN ");
            winBoundToSQL(str, f->lower, nestedSubqueries);
            appendStringInfoString(str, " AND ");
            winBoundToSQL(str, f->higher, nestedSubqueries);
        }
        else
            winBoundToSQL(str, f->lower, nestedSubqueries);
    }

    appendStringInfoString(str, ")");
}

static void
winBoundToSQL (StringInfo str, WindowBound *b, HashMap *nestedSubqueries)
{
    switch(b->bType)
    {
        case WINBOUND_UNBOUND_PREC:
            appendStringInfoString(str, "UNBOUNDED PRECEDING");
            break;
        case WINBOUND_CURRENT_ROW:
            appendStringInfoString(str,"CURRENT ROW");
            break;
        case WINBOUND_EXPR_PREC:
            exprToSQLString(str, b->expr, nestedSubqueries);
            appendStringInfoString(str," PRECEDING");
            break;
        case WINBOUND_EXPR_FOLLOW:
            exprToSQLString(str, b->expr, nestedSubqueries);
            appendStringInfoString(str," FOLLOWING");
            break;
    }
}

static void
orderExprToSQL (StringInfo str, OrderExpr *o, HashMap *nestedSubqueries)
{
    exprToSQLString(str, (Node *) o->expr, nestedSubqueries);

    if (o->order == SORT_ASC)
        appendStringInfoString(str, " ASC");
    else if (o->order == SORT_DESC)
        appendStringInfoString(str, " DESC");

    if (o->nullOrder == SORT_NULLS_FIRST)
        appendStringInfoString(str, " NULLS FIRST");
    else if (o->nullOrder == SORT_NULLS_LAST)
        appendStringInfoString(str, " NULLS LAST");
}

static void
sqlParamToSQL(StringInfo str, SQLParameter *p)
{
    appendStringInfo(str, ":%s", p->name);
}

static void
castExprToSQL(StringInfo str, CastExpr *c, HashMap *nestedSubqueries)
{
    switch(getBackend())
    {
        case BACKEND_POSTGRES:
        {
            appendStringInfoString(str, "(");
            exprToSQLString(str, c->expr, nestedSubqueries);
            appendStringInfoString(str, ")::");
            dataTypeToSQL(str, c->resultDT);
        }
        break;
        default:
        {
            appendStringInfoString(str, "CAST (");
            exprToSQLString(str, c->expr, nestedSubqueries);
            appendStringInfoString(str, " AS ");
            dataTypeToSQL(str, c->resultDT);
            appendStringInfoString(str, ")");
        }
    }
}

static void
dataTypeToSQL (StringInfo str, DataType dt)
{
    appendStringInfoString(str, backendDatatypeToSQL(dt));
}

static void
exprToSQLString(StringInfo str, Node *expr, HashMap *nestedSubqueries)
{
    if (expr == NULL)
        return;

    switch(expr->type)
    {
        case T_AttributeReference:
            attributeReferenceToSQL(str, (AttributeReference *) expr, nestedSubqueries);
            break;
        case T_DLVar:
            appendStringInfo(str, "%s", ((DLVar *) expr)->name);
            break;
        case T_Constant:
            constantToSQL(str, (Constant *) expr);
            break;
        case T_FunctionCall:
            functionCallToSQL(str, (FunctionCall *) expr, nestedSubqueries);
            break;
        case T_Operator:
            operatorToSQL(str, (Operator *) expr, nestedSubqueries);
            break;
        case T_List:
        {
            int i = 0;
            FOREACH(Node,arg,(List *) expr)
            {
                appendStringInfoString(str, ((i++ == 0) ? "(" : ", "));
                exprToSQLString(str, arg, nestedSubqueries);
            }
            appendStringInfoString(str,")");
        }
        break;
        case T_CaseExpr:
            caseToSQL(str, (CaseExpr *) expr, nestedSubqueries);
        break;
        case T_WindowFunction:
            winFuncToSQL(str, (WindowFunction *) expr, nestedSubqueries);
        break;
        case T_IsNullExpr:
            appendStringInfo(str, "(%s IS NULL)", exprToSQL(((IsNullExpr *) expr)->expr, NULL));
        break;
        case T_RowNumExpr:
            appendStringInfoString(str, "ROWNUM");
        break;
        case T_OrderExpr:
            orderExprToSQL(str, (OrderExpr *) expr, nestedSubqueries);
        break;
        case T_SQLParameter:
            sqlParamToSQL(str, (SQLParameter *) expr);
        break;
        case T_CastExpr:
            castExprToSQL(str, (CastExpr *) expr, nestedSubqueries);
        break;
        default:
            FATAL_LOG("not an expression node <%s>", nodeToString(expr));
    }
}

char *
exprToSQL (Node *expr, HashMap *nestedSubqueries)
{
    StringInfo str = makeStringInfo();
    char *result;

    if (expr == NULL)
        return "";

    exprToSQLString(str, expr, nestedSubqueries);

    result = str->data;
    FREE(str);

    return result;
}

static void
exprToLatexString(StringInfo str,  Node *expr, HashMap *nestedSubqueries)
{
    if (expr == NULL)
        return;

    switch(expr->type)
    {
        case T_AttributeReference:
            attributeReferenceToLatex(str, (AttributeReference *) expr);
            break;
        case T_DLVar:
            appendStringInfo(str, "%s", ((DLVar *) expr)->name);
            break;
        case T_Constant:
            constantToLatex(str, (Constant *) expr);
            break;
        case T_FunctionCall:
            functionCallToLatex(str, (FunctionCall *) expr, nestedSubqueries);
            break;
        case T_Operator:
            operatorToLatex(str, (Operator *) expr, nestedSubqueries);
            break;
        case T_List:
        {
            int i = 0;
            FOREACH(Node,arg,(List *) expr)
            {
                appendStringInfoString(str, ((i++ == 0) ? "(" : ", "));
                exprToLatexString(str, arg, nestedSubqueries);
            }
            appendStringInfoString(str,")");
        }
        break;
        case T_CaseExpr:
            caseToSQL(str, (CaseExpr *) expr, nestedSubqueries);
        break;
        case T_WindowFunction:
            winFuncToSQL(str, (WindowFunction *) expr, nestedSubqueries);
        break;
        case T_IsNullExpr:
            appendStringInfo(str, "(%s IS NULL)", exprToLatex(((IsNullExpr *) expr)->expr));
        break;
        case T_RowNumExpr:
            appendStringInfoString(str, "ROWNUM");
        break;
        case T_OrderExpr:
            orderExprToSQL(str, (OrderExpr *) expr, nestedSubqueries);
        break;
        case T_SQLParameter:
            sqlParamToSQL(str, (SQLParameter *) expr);
        break;
        case T_CastExpr:
        {
            CastExpr *c = (CastExpr *) expr;
            exprToLatexString(str, c->expr, nestedSubqueries);
            //TODO should we show it or not?
        }
        break;
        default:
            FATAL_LOG("not an expression node <%s>", nodeToString(expr));
    }
}

char *
exprToLatex (Node *expr)
{
    StringInfo str = makeStringInfo();
    char *result;

    if (expr == NULL)
        return "";

    exprToLatexString(str, expr, NULL); //TODO change this too?

    result = str->data;
    FREE(str);

    return result;
}


static void
functionCallToLatex (StringInfo str, FunctionCall *node, HashMap *nestedSubqueries)
{

    //int flag = 0;
    if (streq(node->functionname, "AGG_STRAGG"))
    {
        //flag = 1;
        appendStringInfoString(str, "strcat");
    }
    else
        appendStringInfoString(str, node->functionname);

    appendStringInfoString(str, "(");

    int i = 0;
    FOREACH(Node,arg,node->args)
    {
        appendStringInfoString(str, ((i++ == 0) ? "" : ", "));
        exprToLatexString(str, arg, nestedSubqueries);
    }

    appendStringInfoString(str,")");
}

static void
operatorToLatex (StringInfo str, Operator *node, HashMap *nestedSubqueries)
{
    // handle special operators
    if (streq(node->name,"BETWEEN"))
    {
        char *expr = exprToSQL(getNthOfListP(node->args,0), NULL);
        char *lower = exprToSQL(getNthOfListP(node->args,1), NULL);
        char *upper = exprToSQL(getNthOfListP(node->args,2), NULL);

        appendStringInfo(str, "(%s < %s \\wedge %s < %s)", lower, expr, expr, upper);
    }
    //TODO deal with other specific operators, e.g., comparison
    else if (LIST_LENGTH(node->args) == 1)
    {
        if (streq(node->name,"NOT"))
            appendStringInfoString(str, "\\neg");
        else
            appendStringInfo(str, "%s ", node->name);
        appendStringInfoString(str, "(");
        exprToLatexString(str,getNthOfListP(node->args,0), nestedSubqueries);
        appendStringInfoString(str, ")");
    }
    else
    {
        appendStringInfoString(str, "(");

        FOREACH(Node,arg,node->args)
        {
            exprToLatexString(str,arg, nestedSubqueries);
            if(arg_his_cell != node->args->tail)
            {
                if (streq(node->name,"AND"))
                    appendStringInfoString(str, "\\wedge");
                else if (streq(node->name,"OR"))
                    appendStringInfoString(str, "\\vee");
                else
                    appendStringInfo(str, " %s ", node->name);
            }
        }

        appendStringInfoString(str, ")");
    }
}

static void
constantToLatex (StringInfo str, Constant *node)
{
    if (node->isNull)
    {
        appendStringInfoString(str, "NULL");
        return;
    }

    switch(node->constType)
    {
        case DT_INT:
            appendStringInfo(str, "%d", *((int *) node->value));
            break;
        case DT_FLOAT:
            appendStringInfo(str, "%f", *((double *) node->value));
            break;
        case DT_LONG:
            appendStringInfo(str, "%ld", *((gprom_long_t *) node->value));
            break;
        case DT_STRING:
            appendStringInfo(str, "\\texttt{\\textcolor{blue}{%s}}", latexEscapeString((char *) node->value));
            break;
        case DT_BOOL:
            appendStringInfo(str, "%s", *((boolean *) node->value) == TRUE ? "true" : "false");
            break;
        case DT_VARCHAR2:
            appendStringInfo(str, "\\texttt{\\textcolor{blue}{%s}}", latexEscapeString((char *) node->value));
            break;
    }
}

static void
attributeReferenceToLatex (StringInfo str, AttributeReference *node)
{
    appendStringInfoString(str, latexEscapeString(strdup(node->name)));
}

char *
latexEscapeString (char *st)
{
    char *escapedName = replaceSubstr(st, "PROV_", "P_");
    escapedName = replaceSubstr(escapedName, "_", "\\_");
    escapedName = replaceSubstr(escapedName, "\"", "\\rq \\rq ");
    return escapedName;
}

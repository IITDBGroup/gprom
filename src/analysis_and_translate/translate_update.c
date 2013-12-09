/*-----------------------------------------------------------------------------
 *
 * translate_update.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "analysis_and_translate/translate_update.h"
#include "parser/parser.h"
#include "model/query_operator/query_operator.h"
#include "analysis_and_translate/translator.h"

QueryOperator *
translateUpdate (Node *update)
{
    //for example UPDATE R SET a = a+2 WHERE C>5
    StringInfo update = makeStringInfo();
    Node *update;

    if(!cond)
   {
    TableAccessOperator *ta = makeNode(TableAccessOperator);
    ta->tableName = tableName;

    SelectionOperator *sel = makeNode(SelectionOperator);
    sel->cond = copyObject(cond);

    ProjectionOperator *prj = makeNode(ProjectionOperator);
    prj->projExprs = appendToTailOfList(prj->projExprs, (Node *) copyObject(expr));
    
    SetOperator *set = makeNode(SetOperator);
    set->setOpType = setOpType;
   }   
   return update;
}


QueryOperator *
translateInsert (Node *insert)
{
    StringInfo insert = makeStringInfo();
    Node *insert;

    if(!cond)
   {
    TableAccessOperator *ta = makeNode(TableAccessOperator);
    ta->tableName = tableName;

    SelectionOperator *sel = makeNode(SelectionOperator);
    sel->cond = copyObject(cond);

    ProjectionOperator *prj = makeNode(ProjectionOperator);
    prj->projExprs = appendToTailOfList(prj->projExprs, (Node *) copyObject(expr));
    
    SetOperator *set = makeNode(SetOperator);
    set->setOpType = setOpType;
   }   
   return insert; 
}

QueryOperator *
translateDelete (Node *delete)
{
    StringInfo delete = makeStringInfo();
    Node *delete;

    if(!cond)
   {
    TableAccessOperator *ta = makeNode(TableAccessOperator);
    ta->tableName = tableName;
   }   
   return delete;   
}








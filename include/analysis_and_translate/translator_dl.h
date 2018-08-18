/*-----------------------------------------------------------------------------
 *
 * translator_dl.h
 *		translate DL into relational algebra (AGM model)
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_ANALYSIS_AND_TRANSLATE_TRANSLATOR_DL_H_
#define INCLUDE_ANALYSIS_AND_TRANSLATE_TRANSLATOR_DL_H_

#include "model/node/nodetype.h"
#include "model/query_operator/query_operator.h"
#include "model/set/hashmap.h"
#include "model/datalog/datalog_model.h"

#define APPEND_TO_MAP_VALUE_LIST(map,key,elem) \
    do { \
        if(MAP_HAS_STRING_KEY((map),(key))) \
        { \
            KeyValue *_kv = MAP_GET_STRING_ENTRY((map),(key)); \
            List *_valList = (List *) _kv->value; \
            _valList = appendToTailOfList(_valList, (elem)); \
            _kv->value = (Node *) _valList; \
        } \
        else \
        { \
            List *_valList = singleton((elem)); \
            MAP_ADD_STRING_KEY((map),(key),_valList); \
        } \
    } while(0)


extern Node *translateParseDL(Node *q);
extern QueryOperator *translateQueryDL(Node *node);
extern void analyzeProgramDTs (DLProgram *p, HashMap *predToRules);
//extern HashMap *taRel;

#endif /* INCLUDE_ANALYSIS_AND_TRANSLATE_TRANSLATOR_DL_H_ */

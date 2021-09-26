/*-----------------------------------------------------------------------------
 *
 * coarse_grained_rewrite.h
 *
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_COARSE_GRAINED_REWRITE_H_
#define INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_COARSE_GRAINED_REWRITE_H_

#include "model/query_operator/query_operator.h"
#include "model/bitset/bitset.h"

typedef struct psInfo
{
	NodeTag type;
    char    *psType;
    HashMap *tablePSAttrInfos;
} psInfo;

typedef struct psAttrInfo
{
	NodeTag type;
    char    *attrName;
    List    *rangeList;
    BitSet  *BitVector;
    List *psIndexList;
} psAttrInfo;

#define ORACLE_SKETCH_AGG_FUN "dbgroup.BITORAGG"
#define POSTGRES_SET_BITS_FUN "set_bits"
#define POSTGRES_FAST_BITOR_FUN "fast_bit_or"

#define POSTGRES_BIT_DT "bit"

#define COARSE_GRAINED_HASH "HASH"
#define COARSE_GRAINED_PAGE "PAGE"
#define COARSE_GRAINED_RANGEB "RANGEB"
#define COARSE_GRAINED_RANGEA "RANGEA"
#define COARSE_GRAINED_FRAGMENT "FRAGMENT"

extern QueryOperator *addTopAggForCoarse (QueryOperator *op);
extern void autoMarkTableAccessAndAggregation (QueryOperator *op, Node *psPara);
extern void markTableAccessAndAggregation (QueryOperator *op, Node *psPara);
extern void markUseTableAccessAndAggregation (QueryOperator *op, Node *psPara);
extern void markNumOfTableAccess(QueryOperator *op);
extern void markAutoUseTableAccess (QueryOperator *op, HashMap *psMap);
extern psAttrInfo* createPSAttrInfo(List *l, char *tableName);
extern psInfo* createPSInfo(Node *coarsePara);
extern List *getRangeList(int numRanges, char* attrName, char *tableName);
extern void bottomUpPropagateLevelAggregation(QueryOperator *op, psInfo *psPara);
extern char *parameterToCharsSepByComma(List* paras);
//extern psInfo *addPsIntoPsInfo(psInfo *psPara,HashMap *psMap);
extern void cachePsInfo(QueryOperator *op, psInfo *psPara, HashMap *psMap);
extern int getPsSize(BitSet* psBitVector);







#endif /* INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_COARSE_GRAINED_REWRITE_H_ */

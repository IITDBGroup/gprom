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

//typedef struct psInfoCell
//{
//	NodeTag type;
//	char 	*storeTable; 		//the table for storing the ps information
//	char	*pqSql;      		//query template SQL
//	char 	*paraValues;        //parameter values
//	char	*tableName;  		//the table name of current ps
//    char    *attrName;	 		//the attribute name of current ps
//    char 	*provTableAttr;     // prov_table_attr#
//    int 	numRanges;          //number of ranges for range partition
//    int     psSize;             //number of ranges which contains the provenance
//    BitSet  *ps;				//provenance sketch in bitset format
//} psInfoCell;

typedef struct psInfoCell
{
	NodeTag type;
	char	*tableName;  		//the table name of current ps
    char    *attrName;	 		//the attribute name of current ps
    char 	*provTableAttr;     // prov_table_attr#
    int 	numRanges;          //number of ranges for range partition
    int     psSize;             //number of ranges which contains the provenance
    BitSet  *ps;				//provenance sketch in bitset format
} psInfoCell;

//TODO use the right function dependent on system
#define ORACLE_SKETCH_AGG_FUN "dbgroup.BITORAGG"
#define POSTGRES_SET_BITS_FUN "set_bits"
#define POSTGRES_FAST_BITOR_FUN "fast_bit_or"
#define POSTGRES_BITOR_FUN "bitor"
#define POSTGRES_SET_BIT_FUN "set_bit_on"
#define POSTGRES_BIT_DT "bit"
#define POSTGRES_BIT_AND_FUN "bit_and"

#define COARSE_GRAINED_HASH "HASH"
#define COARSE_GRAINED_PAGE "PAGE"
#define COARSE_GRAINED_RANGEB "RANGEB"
#define COARSE_GRAINED_RANGEA "RANGEA"
#define COARSE_GRAINED_FRAGMENT "FRAGMENT"

//extern List *psinfos;
//extern List *psinfosLoad;
QueryOperator *addTopAggForCoarseUpdatePS(QueryOperator *op); //for update ps, just "fast_bit_or", no need "set_bits";
extern QueryOperator *addTopAggForCoarse (QueryOperator *op);
extern void autoMarkTableAccessAndAggregation (QueryOperator *op, Node *psPara);
extern void markTableAccessAndAggregation (QueryOperator *op, Node *psPara);
extern void markUseTableAccessAndAggregation (QueryOperator *op, Node *psPara);
extern void markTableAccessAndAggregationUpdatePS(QueryOperator *op, Node *psPara);// for update ps, mark ta and agg ops;
extern void markNumOfTableAccess(QueryOperator *op);
extern void markAutoUseTableAccess (QueryOperator *op, HashMap *psMap);
extern psAttrInfo* createPSAttrInfo(List *l, char *tableName);
extern psInfo* createPSInfo(Node *coarsePara);
extern psInfoCell* createPSInfoCell(char *tableName, char *attrName,
		char *provTableAttr, int numRanges, int psSize, BitSet *ps);
extern List *getRangeList(int numRanges, char* attrName, char *tableName);
extern void bottomUpPropagateLevelAggregation(QueryOperator *op, psInfo *psPara);
//extern void bottomUpPropagateLevelWindow(QueryOperator *op, psInfo *psPara);
extern char *parameterToCharsSepByComma(List* paras);
//extern psInfo *addPsIntoPsInfo(psInfo *psPara,HashMap *psMap);
extern void cachePsInfo(QueryOperator *op, psInfo *psPara, HashMap *psMap);
extern int getPsSize(BitSet* psBitVector);
//extern void printListPSInfoCells(List *l);
extern void loadPSInfoFromTable();
//extern void storePSInfoToTable();
extern void storePS();
extern HashMap *getPSFromCache(QueryOperator *op);

extern char *getPSCellsTableName();
extern char *getTemplatesTableName();
extern char *getHistTableName();
extern List *splitHistMapKey(char *k);
//extern HashMap *getLtempNoMap();
//extern HashMap *getpsCellMap();
//extern HashMap *setLtempNoMap(HashMap *map);
//extern HashMap *setpsCellMap(HashMap *map);
extern char *getHistMapKey(char *table, char *attr, char *numRanges);
extern void emptyPSProperty(QueryOperator *root);


#endif /* INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_COARSE_GRAINED_REWRITE_H_ */

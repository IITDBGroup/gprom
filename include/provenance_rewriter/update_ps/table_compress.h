#ifndef INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_TABLE_COMPRESS_H_
#define INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_TABLE_COMPRESS_H_

#include "provenance_rewriter/coarse_grained/coarse_grained_rewrite.h"

extern void tableCompress(char* tablename, char* psAttr, List* ranges);
extern void updateCompressedTable(QueryOperator* updateQuery, char* tablename, psAttrInfo* attrInfo);

#endif /* INCLUDE_PROVENANCE_REWRITER_UPDATE_PS_TABLE_COMPRESS_H_ */

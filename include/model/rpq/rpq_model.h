/*-----------------------------------------------------------------------------
 *
 * rpq_model.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef INCLUDE_MODEL_RPQ_RPQ_MODEL_H_
#define INCLUDE_MODEL_RPQ_RPQ_MODEL_H_

#include "model/node/nodetype.h"
#include "utility/enum_magic.h"

NEW_ENUM_WITH_TO_STRING(RegexOpType,
REGEX_LABEL,
REGEX_OR,
REGEX_PLUS,
REGEX_STAR,
REGEX_CONC,
REGEX_OPTIONAL
);

//NEW_ENUM_WITH_TO_STRING(RegexQueryType,
//RPQ_REGULAR,
//RPQ_PROV,
//RPQ_SUBGRAPH
//);

NEW_ENUM_WITH_TO_STRING(RPQQueryType,
RPQ_QUERY_RESULT,
RPQ_QUERY_SUBGRAPH,
RPQ_QUERY_PROV
);

typedef struct Regex {
    NodeTag type;
    List *children;
    RegexOpType opType;
    char *label;
} Regex;

typedef struct RPQQuery {
    NodeTag type;
    Regex *q;
    RPQQueryType t;
    char *edgeRel;
    char *resultRel;
} RPQQuery;

extern Regex* makeRegex(List *args, char *type);
extern Regex* makeRegexLabel(char *label);
extern RegexOpType parseRegexOp (char *label);
extern char *rpqToShortString (Regex *rpq);
extern char *rpqToReversePolish(Regex *rpq);

extern RPQQueryType getRPQQueryType (char *t);
extern RPQQuery *makeRPQQuery(char *query, char *rpqType, char *edgeRel, char *resultRel);

#endif /* INCLUDE_MODEL_RPQ_RPQ_MODEL_H_ */

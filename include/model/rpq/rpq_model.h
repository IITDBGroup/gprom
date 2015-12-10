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

NEW_ENUM_WITH_TO_STRING(RegexQueryType,
RPQ_REGULAR,
RPQ_PROV,
RPQ_SUBGRAPH
);

typedef struct Regex {
    NodeTag type;
    List *children;
    RegexOpType opType;
    char *label;
} Regex;

extern Regex* makeRegex(List *args, char *type);
extern Regex* makeRegexLabel(char *label);
extern RegexOpType parseRegexOp (char *label);
extern char *rpqToShortString (Regex *rpq);
extern char *rpqToReversePolish(Regex *rpq);

#endif /* INCLUDE_MODEL_RPQ_RPQ_MODEL_H_ */

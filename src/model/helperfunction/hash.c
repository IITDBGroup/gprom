/*-----------------------------------------------------------------------------
 *
 * hash.c
 *			  
 *		
 *		AUTHOR: lord_pretzel
 *
 *		
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"
#include "model/node/nodetype.h"
#include "model/expression/expression.h"
#include "log/logger.h"

// hash constants
#define FNV_OFFSET ((uint64_t) 14695981039346656037U)
#define FNV_PRIME ((uint64_t) 1099511628211U)

// hash macros
#define HASH_RETURN() return cur

#define HASH_STRING(a) cur = hashString(cur, node->a)
#define HASH_INT(a) cur = hashInt(cur, node->a)
#define HASH_BOOLEAN(a) cur = hashBool(cur, node->a)
#define HASH_NODE(a) cur = hashValueInternal(cur, node->a)

// hash functions for simple types
static inline uint64_t hashInt(uint64_t cur, int value);
static inline uint64_t hashLong(uint64_t cur, long value);
static inline uint64_t hashFloat(uint64_t cur, float value);
static inline uint64_t hashString(uint64_t cur, char *value);
static inline uint64_t hashBool(uint64_t cur, boolean value);
static inline uint64_t hashMemory(uint64_t cur, void *memP, size_t bytes);

// hash for expression nodes
static uint64_t hashConstant (uint64_t cur, Constant *node);
static uint64_t hashAttributeReference (uint64_t cur, AttributeReference *node);

// hash function entry point
static uint64_t hashValueInternal(uint64_t h, void *a);

/* hash simple values */
static inline uint64_t
hashInt(uint64_t cur, int value)
{
    return hashMemory(cur, &value, sizeof(int));
}

static inline uint64_t
hashString(uint64_t cur, char *value)
{
    size_t len = strlen(value);
    return hashMemory(cur, &value, len);
}

static inline uint64_t
hashBool(uint64_t cur, boolean value)
{
    return hashMemory(cur, &value, sizeof(boolean));
}

static inline
uint64_t hashLong(uint64_t cur, long value)
{
    return hashMemory(cur, &value, sizeof(long));
}

static inline
uint64_t hashFloat(uint64_t cur, float value)
{
    return hashMemory(cur, &value, sizeof(float));
}

static inline
uint64_t hashMemory(uint64_t cur, void *memP, size_t bytes)
{
    // byte pointers
    char *curP = (char *) &cur;
    char *valueP = (char *) &memP;

    for(int i = 0; i < bytes; i++, valueP++)
    {
        cur *= FNV_PRIME;
        *curP ^= *curP | *valueP;
    }

    return cur;
}

/* expression node hash functions */
static uint64_t
hashConstant (uint64_t cur, Constant *node)
{
    HASH_INT(constType);

    switch(node->constType)
    {
        case DT_INT:
            hashInt(cur, INT_VALUE(node));
            break;
        case DT_LONG:
            hashLong(cur, LONG_VALUE(node));
            break;
        case DT_FLOAT:
            hashFloat(cur, FLOAT_VALUE(node));
            break;
        case DT_BOOL:
            hashBool(cur, BOOL_VALUE(node));
            break;
        case DT_STRING:
            hashString(cur, STRING_VALUE(node));
            break;
    }

    HASH_BOOLEAN(isNull);

    HASH_RETURN();
}

static uint64_t
hashAttributeReference (uint64_t cur, AttributeReference *node)
{
    HASH_STRING(name);
    HASH_INT(fromClauseItem);
    HASH_INT(attrPosition);
    HASH_INT(outerLevelsUp);

    HASH_RETURN();
}

/* generic hash function */
static uint64_t
hashValueInternal(uint64_t h, void *a)
{
    Node *n;

    if (a == NULL)
        return h;

    n = (Node *) a;

    switch(n->type)
    {
        case T_Constant:
            return hashConstant(h,(Constant *) n);
        case T_AttributeReference:
            return hashAttributeReference (h, (AttributeReference *) n);
        default:
            FATAL_LOG("unknown node type");
            break;
    }

    return h;
}

uint64_t
hashValue(void *a)
{
    uint64_t h = FNV_OFFSET;

    return hashValueInternal(h, a);
}

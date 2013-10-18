/*-----------------------------------------------------------------------------
 *
 * node.c
 *		Basic functions for nodes.
 *		
 *		AUTHOR: lord_pretzel
 *
 *		Right now only implements node creation.
 *
 *-----------------------------------------------------------------------------
 */

#include "common.h"

#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "model/node/nodetype.h"

#define INITIAL_STRINGINFO_LENGTH 256

/* function declarations */
static void makeStringInfoSpace (StringInfo str, int needs);

/*
 * Create a new node of a certain type.
 */
Node *
newNode(size_t size, NodeTag type)
{
    Node *newNode;

    newNode = (Node *) CALLOC(size,1);
    newNode->type = type;

    return newNode;
}

/*
 * Create a new StringInfo.
 */
StringInfo
makeStringInfo(void)
{
    StringInfo result = NEW(StringInfoData);
    initStringInfo(result);

    return result;
}

/*
 * The function initializes a StringInfoData.
 */
void
initStringInfo(StringInfo str)
{
    str->cursor = 0;
    str->len = 0;
    str->maxlen = INITIAL_STRINGINFO_LENGTH;
    str->data = (char *) MALLOC(INITIAL_STRINGINFO_LENGTH);
}

/*
 *The function is clear the current content of StringInfo.
 */
void
resetStringInfo(StringInfo str)
{
    str->cursor = 0;
    memset(str->data, 0, str->len);
    str->len = 0;
}

void
appendStringInfoChar(StringInfo str, const char c)
{
    makeStringInfoSpace(str, 1);

    str->data[str->len] = c;
    str->len++;
    str->data[str->len] = '\0';
}

/*------------------------------------------------------------------
*appendStringInfoString
*The function is append a string to str.
*-------------------------------------------------------------------*/
void
appendStringInfoString(StringInfo str, const char *s)
{
    makeStringInfoSpace(str, strlen(s));

    // copy s including final '\0' starting after last char of str
    memcpy(str->data + str->len, s, strlen(s) + 1);
    str->len += strlen(s);
}


void
appendStringInfo(StringInfo str, const char *format, ...)
{
    for(;;)
    {
        va_list     args;
        int needed, have;

        have = str->maxlen - str->len - 1;
        va_start(args, format);
        needed = vsnprintf(str->data + str->len, have, format, args);
        va_end(args);

        if (needed >= 0 && needed <= have)
        {
            str->len += needed;
            break;
        }
        if (needed < 0)
            FATAL_LOG("encoding error in appendStringInfo <%s>", format);

        makeStringInfoSpace(str, needed);
    }
}


void
prependStringInfo (StringInfo str, const char *format, ...)
{
    va_list args;
    int preLen, appendSize;
    char *temp = MALLOC(str->len);

    // copy string A
    memcpy(temp, str->data, str->len);
    preLen = str->len;

    // append to end leading AB
    va_start(args, format);
    appendStringInfo(str, format, args);
    va_end(args);

    // move new parts to the beginning to create BA
    appendSize = str->len - preLen;
    memcpy(str->data, str->data + preLen, appendSize);
    memcpy(str->data + appendSize, temp, preLen);
    FREE(temp);
}


/*
 * Make sure that a string info has enough space to be enlarged by additonal
 * neededSize char's.
 */
static void
makeStringInfoSpace(StringInfo str, int neededSize)
{
    while(str->len + neededSize >= str->maxlen)
    {
        char *newData;

        str->maxlen *= 2;
        newData = MALLOC(str->maxlen); // change after we have REALLOC
        memcpy(newData, str->data, str->len + 1);
        FREE(str->data);
        str->data = newData;

        TRACE_LOG("increased maxlen to <%u> total requirement was <%u>",
                str->maxlen, str->len + neededSize);
    }
}

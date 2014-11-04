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
#include "model/expression/expression.h"

#define INITIAL_STRINGINFO_LENGTH 256

/* function declarations */
static void makeStringInfoSpace (StringInfo str, int needs);
static boolean checkString (StringInfo str, int line, char *file);
#define CHECK_STRING(str) if (maxLevel > LOG_TRACE) { checkString(str, __LINE__, __FILE__); }

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
 * Create a key value pair with string key and value.
 */
KeyValue *
createStringKeyValue (char *key, char *value)
{
    KeyValue *result = makeNode(KeyValue);

    result->key = (Node *) createConstString (key);
    result->value = (Node *) createConstString (value);

    return result;
}

/*
 * Create a key value pair with node keys
 */
KeyValue *
createNodeKeyValue(Node *key, Node *value)
{
    KeyValue *result = makeNode(KeyValue);

    result->key = key;
    result->value = value;

    return result;
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
appendStringInfoStrings(StringInfo str, ...)
{
    va_list args;
    char *arg;

    va_start(args,str);

    while((arg = va_arg(args, char *)) != NULL)
    {
//        DEBUG_LOG("append <%s> to <%s>", arg, str->len ? str->data : "");
        appendStringInfoString(str, arg);
    }

    va_end(args);
}

extern char *
concatStrings(const char *s, ...)
{
    StringInfo str;
    char *result;
    va_list args;
    char *arg;

    str = makeStringInfo();
    appendStringInfoString(str, s);

    va_start(args,s);
    while((arg = va_arg(args,char*)) != NULL)
        appendStringInfoString(str, arg);
    va_end(args);

    result = str->data;
    FREE(str);
    return result;
}

void
appendStringInfo(StringInfo str, const char *format, ...)
{
    for(;;)
    {
        va_list     args;
//        int have;
        boolean success;

//        have = str->maxlen - str->len - 1;

        va_start(args, format);
        success = vAppendStringInfo(str, format, args);
        va_end(args);

        CHECK_STRING(str);

        if (success)
            break;
    }
}

boolean
vAppendStringInfo(StringInfo str, const char *format, va_list args)
{
    int needed, have;

    have = str->maxlen - str->len - 1;

    // need at least one byte
    if (have == 0)
    {
        makeStringInfoSpace(str, 256);
        have = str->maxlen - str->len - 1;
    }

    // try to output with available space
    needed = vsnprintf(str->data + str->len, have + 1, format, args);

    if (needed >= 0 && needed <= have)
    {
        str->len += needed;
        CHECK_STRING(str);
        return TRUE;
    }
    if (needed < 0)
        FATAL_LOG("encoding error in appendStringInfo <%s>", format);

    makeStringInfoSpace(str, needed);
    CHECK_STRING(str);

    return FALSE;
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
 * Make sure that a string info has enough space to be enlarged by additional
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
        CHECK_STRING(str);

        TRACE_LOG("increased maxlen to <%u> total requirement was <%u>",
                str->maxlen, str->len + neededSize);
    }
}

static boolean
checkString (StringInfo str, int line, char *file)
{
    if (str->len > str->maxlen - 1)
    {
        FATAL_LOG("string to long: maxlen <%u> len <%u> string\n%s", str->data);
    }
    for(int i = 0; i < str->len; i++)
    {
        if (str->data[i] == 0)
            FATAL_LOG("%s-%u at pos %i ", file, line, i);
    }

    return TRUE;
}

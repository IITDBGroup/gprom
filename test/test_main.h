/*-------------------------------------------------------------------------
 *
 * test_main.h
 *    This is the main header file for the test framework.
 *
 *    Author: Ying Ni yni6@hawk.iit.edu
 *
 *    This header defines macros for running test, for comparing results to
 *    expected results, and defines the top level test methods to run. Each
 *    top level test method is implemented in its own .c file.
 *
 *-------------------------------------------------------------------------
 */

#ifndef TEST_MAIN_H_
#define TEST_MAIN_H_

#include "common.h"

#include "model/node/nodetype.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"
#include "log/termcolor.h"

/* are using actual free here */
#undef free
#undef malloc

/* return values for tests */
#define PASS 0
#define FAIL -1
typedef int rc;

/* global counter for recursion depth of tests */
extern int test_rec_depth;
extern int test_count;

#define RUN_TEST(testCase, msg) \
    do { \
        char *indentation = getIndent(test_rec_depth); \
        int prev_count = test_count; \
    	printf("%s" T_FG_BG(WHITE,BLACK,"TEST SUITE STARTED") "[" TB("%s") "-%s-%u]: %s\n", indentation, __FILE__, \
    	        __func__, __LINE__, msg); \
    	free(indentation); \
    	test_rec_depth++; \
    	rc returnCode = (testCase); \
    	test_rec_depth--; \
    	checkResult(returnCode, msg, __FILE__, __func__, __LINE__, \
                test_count - prev_count); \
    } while (0)

/* assertion macros */
#define CHECK_RESULT(rcExpr, msg) \
    do { \
        test_rec_depth++; \
        rc returnCode = (rcExpr); \
        test_rec_depth--; \
        checkResult(returnCode, msg, __FILE__, __func__, __LINE__, -1); \
    } while (0)

#define EQUALS_EQUALS(_a,_b) \
    equal(_a,_b)

#define EQUALS_EQ(_a,_b) \
    (_a) == (_b)

#define EQUALS_STRINGP(_a,_b) \
	((_a == _b) || (_a != NULL && _b != NULL && strcmp(_a,_b) == 0))

#define EQUALS_STRING(_a,_b) \
    ((_a == NULL && _b == NULL) || (_a != NULL && _b != NULL && strcmp(_a,_b) == 0))

#define TOSTRING_NODE(a) nodeToString(a)

#define TOSTRING_SELF(a) a

#define TOSTRING_TOKENIZE(a) TOSTRING_ ## a

#define ASSERT_EQUALS_INTERNAL(_type,a,b,_equals,message,format,_tostring) \
	    do { \
	        _type _aVal = (_type) (a); \
	        _type _bVal = (_type) (b); \
	        boolean result = _equals(_aVal,_bVal); \
	        TRACE_LOG("result was: <%s>", result ? "TRUE": "FALSE"); \
	        char *m = (char *) malloc(2048); \
	        if (!result) \
	            sprintf(m, ("expected <" format ">, but was " \
                        "<" format ">: %s"), TOSTRING_TOKENIZE(_tostring)(_aVal), TOSTRING_TOKENIZE(_tostring)(_bVal), message); \
	        else \
	            sprintf(m, ("as expected <" format "> was equal to" \
                        " <" format ">: %s"), TOSTRING_TOKENIZE(_tostring)(_aVal), TOSTRING_TOKENIZE(_tostring)(_bVal), message); \
	        CHECK_RESULT((result ? PASS : FAIL), m); \
	        free(m); \
	    } while(0)

#define ASSERT_EQUALS_NODE(a,b,message) \
    ASSERT_EQUALS_INTERNAL(Node*,a,b,EQUALS_EQUALS,message,"%s",NODE)

#define ASSERT_EQUALS_INT(a,b,message) \
    ASSERT_EQUALS_INTERNAL(int,a,b,EQUALS_EQ,message,"%u",SELF);

#define ASSERT_EQUALS_LONG(a,b,message) \
    ASSERT_EQUALS_INTERNAL(long,a,b,EQUALS_EQ,message,"%lu",SELF);

#define ASSERT_EQUALS_FLOAT(a,b,message) \
    ASSERT_EQUALS_INTERNAL(double,a,b,EQUALS_EQ,message,"%f",SELF);

#define ASSERT_EQUALS_P(a,b,message) \
    ASSERT_EQUALS_INTERNAL(void*,a,b,EQUALS_EQ,message,"%p",SELF);

#define ASSERT_EQUALS_STRINGP(a,b,message) \
    ASSERT_EQUALS_INTERNAL(char*,a,b,EQUALS_STRINGP,message,"%s",SELF);

#define ASSERT_EQUALS_STRING(a,b,message) \
	ASSERT_EQUALS_INTERNAL(char*,a,b,EQUALS_STRING,message,"%s",SELF);

#define ASSERT_TRUE(a,message) \
	CHECK_RESULT(((a) ? PASS : FAIL), message);

#define ASSERT_FALSE(a,message) \
    CHECK_RESULT((!(a) ? PASS : FAIL), message);

/* run all tests */
extern void testSuites(void);

/* helper functions */
extern void checkResult(rc r, char *msg, const char *file, const char *func,
        int line, int tests_passed);
extern char *getIndent(int depth);
extern boolean testQuery (char *query, char *expectedResult);

/* individual tests */
extern rc testList(void);
extern rc testSet(void);
extern rc testHashMap(void);
extern rc testVector(void);
extern rc testExpr(void);
extern rc testCopy(void);
extern rc testEqual(void);
extern rc testToString(void);
extern rc testLogger(void);
extern rc testMemManager(void);
extern rc testParse(void);
extern rc testMetadataLookup(void);
extern rc testMetadataLookupPostgres(void);
extern rc testString(void);
extern rc testStringUtils(void);
extern rc testParameter(void);
extern rc testPICSGraph(void);
extern rc testDatalogModel(void);
extern rc testHash(void);

#endif

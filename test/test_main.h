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

#include <stdio.h>
#include <string.h>

#include "model/node/nodetype.h"
#include "log/logger.h"
#include "mem_manager/mem_mgr.h"

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
    	printf("%sTEST SUITED STARTED [%s-%s-%u]: %s\n", indentation, __FILE__, \
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

#define ASSERT_EQUALS_INTERNAL(a,b,resultExpr,message,format,stringa,stringb) \
	    do { \
	        boolean result = (resultExpr); \
	        TRACE_LOG("result was: <%s>", result ? "TRUE": "FALSE"); \
	        char *m = (char *) malloc(2048); \
	        if (!result) \
	            sprintf(m, ("expected <" format ">, but was " \
                        "<" format ">: %s"), stringa, stringb, message); \
	        else \
	            sprintf(m, ("as expected <" format "> was equal to" \
                        " <" format ">: %s"), stringa, stringb, message); \
	        CHECK_RESULT((result ? PASS : FAIL), m); \
	        free(m); \
	    } while(0)

#define ASSERT_EQUALS_NODE(a,b,message) \
    ASSERT_EQUALS_INTERNAL(a,b,equal(a,b),message,"%s",nodeToString(a),nodeToString(b))

#define ASSERT_EQUALS_INT(a,b,message) \
    ASSERT_EQUALS_INTERNAL(a,b,(a) == (b),message,"%u",a,b);

#define ASSERT_EQUALS_FLOAT(a,b,message) \
    ASSERT_EQUALS_INTERNAL(a,b,(a) == (b),message,"%f",a,b);

#define ASSERT_EQUALS_P(a,b,message) \
    ASSERT_EQUALS_INTERNAL(a,b,(a) == (b),message,"%p",a,b);

#define ASSERT_EQUALS_STRINGP(a,b,message) \
    ASSERT_EQUALS_INTERNAL(a,b,((a == b) || (a != NULL && b != NULL && strcmp(a,b) == 0)),message,"%s",a,b);

#define ASSERT_EQUALS_STRING(a,b,message) \
	ASSERT_EQUALS_INTERNAL(a,b,(strcmp(a,b) == 0),message,"%s",a,b);

/* run all tests */
extern void testSuites(void);

/* helper functions */
extern void checkResult(rc r, char *msg, const char *file, const char *func,
        int line, int tests_passed);
extern char *getIndent(int depth);

/* individual tests */
extern rc testExpr(void);
extern rc testCopy(void);
extern rc testEqual(void);
extern rc testLogger(void);
extern rc testMemManager(void);

#endif

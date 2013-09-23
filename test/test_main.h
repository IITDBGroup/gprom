/*-------------------------------------------------------------------------
 *
 * test_main.h
 *    Author: Ying Ni yni6@hawk.iit.edu
 *    One-line description
 *
 *        Here starts the more detailed description where we
 *        explain in more detail how this works.
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


#define RUN_TEST(testCase, msg) \
    do { \
        char *indentation = getIndent(test_rec_depth); \
    	printf("%sTEST START [%s-%s-%u]: %s\n", indentation, __FILE__, \
    	        __func__, __LINE__, msg); \
    	free(indentation); \
    	test_rec_depth++; \
    	rc returnCode = (testCase); \
    	test_rec_depth--; \
    	checkResult(returnCode, msg, __FILE__, __func__, __LINE__); \
    } while (0)

/* assertion macros */
#define CHECK_RESULT(rcExpr, msg) \
    do { \
        test_rec_depth++; \
        rc returnCode = (rcExpr); \
        test_rec_depth--; \
        checkResult(returnCode, msg, __FILE__, __func__, __LINE__); \
    } while (0)

#define ASSERT_EQUALS_INTERNAL(a,b,resultExpr,message,format,stringa,stringb) \
	    do { \
	        boolean result = (resultExpr); \
	        TRACE_LOG("result was: <%s>", result ? "TRUE": "FALSE"); \
	        char *m = (char *) MALLOC(2048); \
	        if (!result) \
	            sprintf(m, ("expected <" format ">, but was " \
                        "<" format ">: %s"), stringa, stringb, message); \
	        else \
	            sprintf(m, ("as expected <" format "> was equal to" \
                        " <" format ">: %s"), stringa, stringb, message); \
	        CHECK_RESULT((result ? PASS : FAIL), m); \
	        FREE(m); \
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
        int line);
extern char *getIndent(int depth);

/* individual tests */
extern rc testExpr(void);
extern rc testLogger(void);
extern rc testMemManager(void);

#endif

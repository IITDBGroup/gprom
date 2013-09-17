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

#define PASS 0
#define FAIL -1
typedef int rc;

extern void
testSuites(void);

extern rc
testLogger(void);
extern rc
testMemManager(void);

#endif

/*
 * ps_safety_check.h
 *
 *  Created on: 2018年10月25日
 *      Author: liuziyu
 */

#ifndef INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_PS_SAFETY_CHECK_H_
#define INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_PS_SAFETY_CHECK_H_

extern void monotoneCheck(Node *qbModel);
void checkM(QueryOperator* op, int *num);
boolean check(Node* node, int *state);



#endif /* INCLUDE_PROVENANCE_REWRITER_COARSE_GRAINED_PS_SAFETY_CHECK_H_ */

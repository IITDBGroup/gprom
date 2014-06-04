/*-----------------------------------------------------------------------------
 *
 * vector.h
 *		
 *
 *		AUTHOR: lord_pretzel
 *
 *-----------------------------------------------------------------------------
 */

#ifndef VECTOR_H_
#define VECTOR_H_

#include "model/list/list.h"

typedef enum VectorType {
    VECTOR_INT,
    VECTOR_NODE,
    VECTOR_STRING
} VectorType;

typedef struct Vector
{
    NodeTag type;
    VectorType elType;
    NodeTag elNodeType;
    int length;
    int maxLength;
    char *data;
} Vector;

// access data of vector as array
#define VEC_TO_ARR(vec,type) ((type **) ((Vector *) vec)->data)
#define VEC_TO_IA(vec) ((int *) ((Vector *) vec)->data)

// length of vector
#define VEC_LENGTH(v) ((v == NULL) ? 0 : ((Vector *) v)->length)

// create a new vector
extern Vector *makeVector(VectorType type, NodeTag nodeType);
extern Vector *makeVectorOfSize(VectorType type, NodeTag nodeType, int numElem);

extern Vector *makeVectorFromElem(VectorType set, NodeTag nodeType, void *elem, ...);
extern Vector *makeVectorIntFromElem(int elem, ...);
extern Vector *makeVectorIntSeq (int start, int length, int step);
extern Vector *makeVectorFromList (List *input);
#define MAKE_VEC_NODE(tag, ...) makeVectorFromElem(VECTOR_NODE, T_ ## tag, __VA_ARGS__, NULL)
#define MAKE_VEC_STRING(...) makeVectorFromElem(VECTOR_STRING, T_Invalid, __VA_ARGS__, NULL)
#define MAKE_VEC_INT(...) makeVectorIntFromElem(__VA_ARGS__, -1)


// vector size
extern size_t getVecElemSize(Vector *v);
extern size_t getVecDataSize(Vector *v);

// append something to the end of a vector
extern void vecAppendNode(Vector *v, Node *el);
extern void vecAppendInt(Vector *v, int el);
#define VEC_ADD_NODE(v,el) vecAppendNode((Vector *) v, (Node *) el)

// get elements from a vector
extern Node *getVecNode(Vector *v, int pos);
extern int getVecInt(Vector *v, int pos);

// find a certain element in a vector
extern boolean findVecNode(Vector *v, Node *el);
extern boolean findVecInt(Vector *v, int el);

#define FOREACH_VEC(_type_,_elem_,_vec_) \
    for(_type_ **_elem_ = (_vec_ == NULL) ? NULL : VEC_TO_ARR(_vec_,_type_); \
            _elem_ != NULL && ((_elem_ - VEC_TO_ARR(_vec_,_type_)) < VEC_LENGTH(_vec_)); \
            _elem_++)

#define FOREACH_VEC_INT(_elem_,_vec_) \
    for(int *_elem_ = (_vec_ == NULL) ? NULL : VEC_TO_IA(_vec_); \
            _elem_ != NULL && ((_elem_ - VEC_TO_IA(_vec_)) < VEC_LENGTH(_vec_)); \
            _elem_++)

#define VEC_IS_LAST(_elem_,_vec_) (VEC_TO_ARR(_vec_,void)[_vec_->length - 1] == _elem_)

// remove elements from a vector
extern boolean shrinkVector(Vector *v, int newSize);
extern int popVecInt(Vector *v);
extern Node *popVecNode(Vector *v);

// deep free a vector
extern void freeVec (Vector *v);
extern void deepFreeVec (Vector *v);

#endif /* VECTOR_H_ */

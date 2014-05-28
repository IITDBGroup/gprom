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
#define ARR(vec,type) ((type **) vec->data)
#define VEC_TO_IA(vec) ((int *) vec->data)

// length of vector
#define VEC_LENGTH(v) ((v == NULL) ? 0 : v->length)

// create a new vector
extern Vector *makeVector(VectorType type, NodeTag nodeType);

// append something to the end of a vector
extern void vecAppend(Vector *v, Node *el);
extern void vecAppendInt(Vector *v, int el);

// get elements from a vector
extern Node *getVec(Vector *v, int pos);
extern int getVecInt(Vector *v, int pos);

// find a certain element in a vector
extern boolean findVecElem(Vector *v, Node *el);
extern boolean findVecInt(Vector *v, int el);

#define FOREACH_VEC(_type_,_elem_,_vec_) \
    for(_type_ *_elem_ = (_vec_ == NULL) ? NULL : ARR(_vec_,_type_); \
            _elem_ != NULL && (_elem_ - ARR(_vec_,_type_) < VEC_LENGTH(_vec_)); \
            _elem_++)

// deep free a vector
extern void freeVec (Vector *v);
extern void deepFreeVec (Vector *v);

#endif /* VECTOR_H_ */

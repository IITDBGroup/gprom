#ifndef ENUM_MAGIC_H_
#define ENUM_MAGIC_H_

//Utitily macros
#define EMPTY()
#define DEFER2(m) m EMPTY EMPTY()()
#define CAT(a, b) a ## b
#define SECOND(a, b, ...) b
#define FIRST(a, ...) a


//Manual Expansion
#define EVAL1(...) __VA_ARGS__
#define EVAL2(...) EVAL1(EVAL1(__VA_ARGS__))
#define EVAL4(...) EVAL2(EVAL2(__VA_ARGS__))
#define EVAL8(...) EVAL4(EVAL4(__VA_ARGS__))
#define EVAL16(...) EVAL8(EVAL8(__VA_ARGS__))
#define EVAL32(...) EVAL16(EVAL16(__VA_ARGS__))
#define EVAL64(...) EVAL32(EVAL32(__VA_ARGS__))
#define EVAL128(...) EVAL64(EVAL64(__VA_ARGS__))
#define EVAL256(...) EVAL128(EVAL128(__VA_ARGS__))//Can be extended easily

#define EVAL(...) EVAL256(__VA_ARGS__)


//IF_ELSE(cond)(do if true)(do if false)
#define IF_ELSE(cond) CAT(_IF_, cond)
#define _IF_1(...) __VA_ARGS__ _IF_1_ELSE
#define _IF_0(...) _IF_0_ELSE

#define _IF_0_ELSE(...) __VA_ARGS__
#define _IF_1_ELSE(...)


//BOOL, expand to 1 or 0
#define IS_PROBE(...) SECOND(__VA_ARGS__, 0)
#define PROBE() ~, 1

#define NOT(x) IS_PROBE(CAT(_NOT_, x))
#define _NOT_0 PROBE()

#define BOOL(x) NOT(NOT(x))


//HAS_ARGS, check if __VA_ARGS__ is empty
#define HAS_ARGS(...) BOOL(FIRST(_END_OF_ARGUMENTS_ __VA_ARGS__ ()))
#define _END_OF_ARGUMENTS_() 0


//MAP a function-like macro to all arguments
#define MAP(m, first, ...) \
    m(first) \
    IF_ELSE(HAS_ARGS(__VA_ARGS__))( \
        DEFER2(_MAP)()(m, __VA_ARGS__)   \
    )()
#define _MAP() MAP


#define VA_NUM_ARGS(...)    \
    0 EVAL(MAP(ADDONE, __VA_ARGS__))
#define ADDONE(x) +1

#define ENUM_STRING_CASE(x) \
    case x: return #x;

#define STRING_ENUM_IF(x) \
    if (strcmp(in, #x) == 0) return x;

//Usage: NEW_ENUM_WITH_TO_STRING(GGG,a,b,c,d,e,f);
#define NEW_ENUM_WITH_TO_STRING(_name_,...) \
    typedef enum _name_ { \
        __VA_ARGS__ \
    } _name_; \
    \
    static int NUM_ELEM_ ## _name_ = VA_NUM_ARGS(__VA_ARGS__);  \
    \
    static inline char * _name_ ## ToString (_name_ in) \
    { \
        switch(in) {\
            EVAL(MAP(ENUM_STRING_CASE, __VA_ARGS__)) \
            default: return NULL;   \
        }   \
    }   \
    \
    static inline _name_ stringTo ## _name_ (char *in) \
    { \
        EVAL(MAP(STRING_ENUM_IF, __VA_ARGS__))   \
        return 0;   \
    }


#define NEW_ENUM_WITH_ONLY_TO_STRING(_name_,...) \
    typedef enum _name_ { \
        __VA_ARGS__ \
    } _name_; \
    \
    static int NUM_ELEM_ ## _name_ = VA_NUM_ARGS(__VA_ARGS__);  \
    \
    static inline char * _name_ ## ToString (_name_ in) \
    { \
        switch(in) {\
            EVAL(MAP(ENUM_STRING_CASE, __VA_ARGS__)) \
            default: return NULL;   \
        }   \
    }


#endif /* ENUM_MAGIC_H_ */

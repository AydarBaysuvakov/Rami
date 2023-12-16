#ifndef STACK_H
#define STACK_H

#include <limits.h>

#define STACK_DEFN_ARGS const char*, const unsigned, const char*, const char*
#define STACK_PASS_ARGS __LINE__, __FILE__, __FUNCTION__
#define StackCtor(stk)         MyStackCtor((stk),           #stk, STACK_PASS_ARGS)
#define StackDump(stk, stk_st) MyStackDump((stk), (stk_st), #stk, STACK_PASS_ARGS)

typedef Node** Elem_t;

const size_t   DEFAULT_SIZE     = 0;
const size_t   DEFAULT_CAPACITY = 8;

const long long NOT_VALID_VALUE  = 0xBAD;
const Elem_t    POISON           = (Elem_t) (0xBAD % (1ull << CHAR_BIT * sizeof(Elem_t) - 1));

const int REALLOC_COEFFICIENT   = 2;
const int MIN_REALLOC_DOWN_SIZE = 16;
const int REALLOC_DOWN_BORDER   = 4;

struct Stack
    {
    Elem_t *data;
    size_t  size;
    size_t  capacity;

    const char  *name;
    const char  *file;
    const char  *func;
    unsigned     line;

    FILE *logfile;
    };

enum StackErrorBit
{
    STK_NULLPTR               = 1 << 0,
    STK_DATA_NULL             = 1 << 1,
    STK_INVALID_SIZE          = 1 << 2,
    STK_POISON_VAL            = 1 << 3,
};

Error_t MyStackCtor(Stack *stk, STACK_DEFN_ARGS);
Error_t StackDtor(Stack *stk);

Error_t FillStack(Stack *stk);

Error_t StackPush(Stack *stk, Elem_t value);
Elem_t  StackPop(Stack *stk);
Elem_t  StackTop(const Stack *stk);

Error_t StackRealloc(Stack* stk, size_t new_capacity);

State_t StackVerify(const Stack *stk);

Error_t MyStackDump(const Stack *stk, State_t state, STACK_DEFN_ARGS);


#endif//STACK_H

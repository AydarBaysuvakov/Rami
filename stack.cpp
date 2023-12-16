#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "errors.h"
#include "logfiles.h"
#include "node.h"
#include "stack.h"

static bool IsStackErrorState(const State_t state);
static void StackAssert(const Stack* stk);
static const char* GetStackErrorBitMsg(const size_t bit);

Error_t MyStackCtor(Stack *stk, const char* name, const unsigned line, const char* file, const char* func)
    {
    assert(stk != NULL);

    stk->size     = DEFAULT_SIZE;
    stk->capacity = DEFAULT_CAPACITY;

    Elem_t *data = (Elem_t*) calloc(DEFAULT_CAPACITY, sizeof(Elem_t));
    if (data == NULL)
        {
        perror("ERROR: cannot allocate memory");
        return AllocationError;
        }

    stk->data = data;
    FillStack(stk);

    stk->name = name;
    stk->line = line;
    stk->file = file;
    stk->func = func;

    if (LogFileInit(&stk->logfile, "logfile", name, "html") == FileError)
        {
        perror("cannot open stack logfile");
        return FileError;
        }

    return Ok;
    }

Error_t StackDtor(Stack *stk)
    {
    StackAssert(stk);

    free(stk->data);

    stk->size     = NOT_VALID_VALUE;
    stk->capacity = NOT_VALID_VALUE;

    fclose(stk->logfile);

    return Ok;
    }

Error_t FillStack(Stack *stk)
    {
    assert(stk != NULL);
    assert(stk->data != NULL);
    assert(stk->size <= stk->capacity);

    for (size_t i = stk->size; i < stk->capacity; i++)
        {
        stk->data[i] = POISON;
        }

    return Ok;
    }

Error_t StackPush(Stack *stk, Elem_t value)
    {
    StackAssert(stk);

    if (stk->size == stk->capacity)
        {
        if (StackRealloc(stk, stk->capacity * REALLOC_COEFFICIENT) == AllocationError)
            {
            perror("ERROR: cannot allocate memory");
            return AllocationError;
            }
        }

    stk->data[stk->size++] = value;

    return Ok;
    }

Elem_t StackPop(Stack *stk)
    {
    StackAssert(stk);

    if (stk->size == 0)
        {
        return POISON;
        }

    Elem_t ret_val = stk->data[--stk->size];
    stk->data[stk->size] = POISON;

    if (stk->size <= stk->capacity / REALLOC_DOWN_BORDER && stk->capacity > MIN_REALLOC_DOWN_SIZE)
        {
        if (StackRealloc(stk, stk->capacity / REALLOC_COEFFICIENT) == AllocationError)
            {
            perror("ERROR: cannot allocate memory");
            return POISON;
            }
        }

    return ret_val;
    }

Elem_t StackTop(const Stack *stk)
    {
    StackAssert(stk);

    if (stk->size == 0)
        {
        return POISON;
        }

    return stk->data[stk->size - 1];
    }

Error_t StackRealloc(Stack* stk, size_t new_capacity)
    {
    StackAssert(stk);

    Elem_t *data = (Elem_t*) realloc(stk->data, new_capacity * sizeof(Elem_t));
    if (!data)
        {
        return AllocationError;
        }

    stk->data     = data;
    stk->capacity = new_capacity;

    FillStack(stk);
    return Ok;
    }

static void StackAssert(const Stack* stk)
    {
    State_t state = StackVerify(stk);
    if (state) StackDump(stk, state);
    }

static bool IsStackErrorState(const State_t state)
    {
    return state;
    }

State_t StackVerify(const Stack *stk)
    {
    State_t state = 0;

    if (stk == NULL)
        {
        state |= STK_NULLPTR;
        return state;
        }
    if (stk->data == NULL)
        {
        state |= STK_DATA_NULL;
        }
    if (stk->size > stk->capacity)
        {
        state |= STK_INVALID_SIZE;
        }
    if (stk->size != 0 && stk->data[stk->size - 1] == POISON)
        {
        state |= STK_POISON_VAL;
        }

    return state;
    }

Error_t MyStackDump(const Stack *stk, State_t state, const char* name, const unsigned line, const char* file, const char* func)
    {
    fprintf(stk->logfile, "<pre>\n\n");

    fprintf(stk->logfile, "Stack[%p] '%s' from %s(%u) %s()\n", stk, stk->name, stk->file, stk->line, stk->func);
    fprintf(stk->logfile, "\tcalled like '%s' from %s(%u) %s()\n",       name,      file,      line,      func);
    fprintf(stk->logfile, "\t{\n\tsize = %lu\n\tcapacity = %lu\n\tdata[%p]\n\t\t{\n", stk->size, stk->capacity, stk->data);

    for (size_t i = 0; i < stk->capacity; i++)
        {
        fprintf(stk->logfile, (stk->data[i] != POISON) ? "\t\t*" : "\t\t ");
        fprintf(stk->logfile, "[%lu] = ", i);
        fprintf(stk->logfile, (stk->data[i] == POISON) ? "POISON\n" : "%p\n", stk->data[i]);
        }
    fprintf(stk->logfile, "\t\t}\n");

    fprintf(stk->logfile, "\t}\n\n");

    for(size_t bit = 0; bit < CHAR_BIT * sizeof(state); bit++)
        {
        if (state & 1 << bit)
            {
            fprintf(stk->logfile, "%s\n", GetStackErrorBitMsg(bit));
            }
        }

    fprintf(stk->logfile, "</pre>\n\n");

    return Ok;
    }

static const char* GetStackErrorBitMsg(const size_t bit)
{
    static const int  ERROR_COUNT = sizeof(State_t) * CHAR_BIT;
    static const char * const ERROR_MESSAGE[ERROR_COUNT] = {
        "Stack is nullptr",
        "Stack->data is nullptr",
        "Stack->size > Stack->capacity",
        "Curent value is poison",
        "Left canary killed",
        "Right canary killed",
        "Data left canary killed",
        "Data right canary killed",
        "Data hash not compare",
        "Stack hash not compare"
    };

    return ERROR_MESSAGE[bit];
}

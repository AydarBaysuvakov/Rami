#include <stdio.h>
#include <stdlib.h>

void syntax_assert(int expr, Compiler *cmp);
void syntax_error(Compiler *cmp);

void syn_assert(int expr, Compiler *cmp)
    {
    if (!expr) syntax_error(cmp);
    }

void syntax_error(Compiler *cmp)
    {
    printf("Syntax error\n");
    cmp->error = SyntaxError;
    exit(1);
    }

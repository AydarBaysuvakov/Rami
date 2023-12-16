#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "errors.h"
#include "node.h"
#include "tree.h"
#include "list.h"
#include "frontend.h"

void syntax_assert(int expr, Compiler *cmp);
void syntax_error(Compiler *cmp);

Error_t Frontend(const char* filename)
    {
    assert(filename);

    Compiler cmp = {};
    CompilerCtor(&cmp, filename);

    //TokenParsing(&cmp);

    CompilerDtor(&cmp);
    return Ok;
    }

Error_t CompilerCtor(Compiler* cmp, const char* filename)
    {
    assert(cmp);
    assert(filename);

    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
        {
        perror("Cannot open file\n");
        return FileError;
        }

    struct stat sb = {0};
    int fd = fileno(fp);

    if (fstat(fd, &sb) == -1)
        {
        perror("fstat() returned -1");
        fclose(fp);
        return FileError;
        }

    cmp->size = sb.st_size;

    cmp->str = (char*) calloc(cmp->size, sizeof(char));
    if (cmp->str == nullptr)
        {
        perror("Cannot allocate memory for cmp->str");
        fclose(fp);
        return AllocationError;
        }

    fread(cmp->str, sizeof(char), cmp->size, fp);
    if (ferror(fp))
        {
        perror("Cannot read file\n");
        fclose(fp);
        free(cmp->str);
        return FileError;
        }

    fclose(fp);


    cmp->name_table = nullptr;
    cmp->error      = 0;

    ListCtor(&cmp->tokens);
    TreeCtor(&cmp->tree);

    return Ok;
    }

Error_t CompilerDtor(Compiler* cmp)
    {
    assert(cmp);

    cmp->pos        = 0;
    cmp->size       = 0;
    cmp->error      = 0;

    free(cmp->str);
    free(cmp->name_table);

    ListDtor(&cmp->tokens);
    TreeDtor(&cmp->tree);

    return Ok;
    }

Error_t TokenParsing(Compiler* cmp)
    {
    assert(cmp != NULL);

    #define DEF_OPER(op, sym) else if (cmp->str[cmp->pos] == sym)                \
                                    {                                         \
                                    Data_t data = {.oper = op};               \
                                    NewNode(&cmp->tokens, OPERATION, data);   \
                                    node = &(*node)->right;                   \
                                    }

    Node** node = &cmp->tokens;

    while (cmp->str[cmp->pos] != '\0')
        {
        if ('0' <= cmp->str[cmp->pos] && cmp->str[cmp->pos] <= '9')
            {
            int num = cmp->str[cmp->pos++];
            while ('0' <= cmp->str[cmp->pos] && cmp->str[cmp->pos] <= '9')
                {
                num *= 10;
                num += cmp->str[cmp->pos++] - '0';
                }
            Data_t data = {.val = num};
            NewNode(node, VALUE, data);
            node = &(*node)->right;
            }
        else if ('a' <= cmp->str[cmp->pos] && cmp->str[cmp->pos] <= 'z' ||
                 'A' <= cmp->str[cmp->pos] && cmp->str[cmp->pos] <= 'Z' ||
                 cmp->str[cmp->pos] == '_' || cmp->str[cmp->pos] == '$')
            {
            long var = 0;
            while ('0' <= cmp->str[cmp->pos] && cmp->str[cmp->pos] <= '9' ||
                   'a' <= cmp->str[cmp->pos] && cmp->str[cmp->pos] <= 'z' ||
                   'A' <= cmp->str[cmp->pos] && cmp->str[cmp->pos] <= 'Z' ||
                   cmp->str[cmp->pos] == '_' || cmp->str[cmp->pos] == '$')
                {
                var *= 256;
                var += cmp->str[cmp->pos++];
                }
            Data_t data = {.var = var};
            NewNode(node, VARIABLE, data);
            node = &(*node)->right;
            }
        #include "operations.h"
        else if (cmp->str[cmp->pos] == ' ' || cmp->str[cmp->pos] == '\n' || cmp->str[cmp->pos] == '\t');
        }

    #undef DEF_OPER
    return 0;
    }

/*
int GetG(const char* s)
    {
    Compiler solver = {.str = s, .pos = 0, .error = 0, .tokens = nullptr};
    TreeCtor(solver.tree);

    int val = GetE(&solver);
    syn_assert(solver.str[solver.pos] == '\0', &solver);

    return val;
    }

int GetN(Solver *solver)
    {
    int val = 0;
    int old_pos = solver->pos;

    while ('0' <= solver->str[solver->pos] && solver->str[solver->pos] <= '9')
        {
        val *= 10;
        val += solver->str[solver->pos] - '0';
        ++solver->pos;
        }

    syn_assert(solver->pos > old_pos, solver);

    return val;
    }

int GetT(Solver *solver)
    {
    int main_val = GetP(solver);
    while (solver->str[solver->pos] == '*' || solver->str[solver->pos] == '/')
        {
        int oper = solver->str[solver->pos++];
        int val  = GetP(solver);
        switch (oper)
            {
            case '*': main_val *= val; break;
            case '/': main_val /= val; break;
            default: syntax_error(solver);
            }
        }
    return main_val;
    }

int GetE(Solver *solver)
    {
    int main_val = GetT(solver);
    while (solver->str[solver->pos] == '+' || solver->str[solver->pos] == '-')
        {
        int oper = solver->str[solver->pos++];
        int val  = GetT(solver);
        switch (oper)
            {
            case '+': main_val += val; break;
            case '-': main_val -= val; break;
            default: syntax_error(solver);
            }
        }
    return main_val;
    }

int GetP(Solver *solver)
    {
    if (solver->str[solver->pos] == '(')
        {
        ++solver->pos;
        int val = GetE(solver);
        syn_assert(solver->str[solver->pos] == ')', solver);
        ++solver->pos;
        return val;
        }
    return GetN(solver);
    }
*/

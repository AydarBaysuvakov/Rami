#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include "errors.h"
#include "node.h"
#include "tree.h"
#include "list.h"
#include "frontend.h"

static void syntax_assert(int expr, Compiler *cmp);
static void syntax_error(Compiler *cmp);

static void ReadNumber(Compiler* cmp, Node* node);
static void ReadKeyWord(Compiler* cmp, Node* node);
static void ReadOperation(Compiler* cmp, Node* node);

Error_t Frontend(const char* filename)
    {
    assert(filename);

    Compiler cmp = {};
    CompilerCtor(&cmp, filename);
    if (cmp.error)
        {
        return cmp.error;
        }

    TokenParsing(&cmp);
    if (cmp.error)
        {
        CompilerDtor(&cmp);
        return cmp.error;
        }

    ListDump(&cmp.tokens, 0);

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
        cmp->error = FileError;
        return FileError;
        }

    struct stat sb = {0};
    int fd = fileno(fp);

    if (fstat(fd, &sb) == -1)
        {
        perror("fstat() returned -1");
        fclose(fp);
        cmp->error = FileError;
        return FileError;
        }

    cmp->size = sb.st_size;

    cmp->str = (char*) calloc(cmp->size, sizeof(char));
    if (cmp->str == nullptr)
        {
        perror("Cannot allocate memory for cmp->str");
        fclose(fp);
        cmp->error = AllocationError;
        return AllocationError;
        }

    fread(cmp->str, sizeof(char), cmp->size, fp);
    if (ferror(fp))
        {
        perror("Cannot read file\n");
        fclose(fp);
        free(cmp->str);
        cmp->error = FileError;
        return FileError;
        }

    fclose(fp);

    cmp->name_count = DEFAULT_NAME_COUNT;
    cmp->name_table = (Object*) calloc(cmp->name_count, sizeof(Object));
    if (cmp->name_table == nullptr)
        {
        perror("Cannot allocate memory for cmp->name_table");
        free(cmp->str);
        cmp->error = AllocationError;
        return AllocationError;
        }

    for (int i = 0; i < cmp->name_count; i++)
        {
        cmp->name_table[i].type = VARIABLE;
        cmp->name_table[i].name = nullptr;
        }

    cmp->error      = Ok;

    ListCtor(&cmp->tokens);
    TreeCtor(&cmp->tree);

    return Ok;
    }

Error_t CompilerDtor(Compiler* cmp)
    {
    assert(cmp);

    cmp->pos        = 0;
    cmp->size       = 0;
    cmp->error      = Ok;

    free(cmp->str);
    for (int i = 0; i < cmp->name_count; i++)
        {
        if (cmp->name_table[i].name) free(cmp->name_table[i].name);
        }
    free(cmp->name_table);

    ListDtor(&cmp->tokens);
    TreeDtor(&cmp->tree);

    return Ok;
    }

Error_t TokenParsing(Compiler* cmp)
    {
    assert(cmp);

    Node* node = cmp->tokens.head;

    while (cmp->str[cmp->pos] != '\0')
        {
        if (isspace(cmp->str[cmp->pos]));
        else if ('0' <= cmp->str[cmp->pos] && cmp->str[cmp->pos] <= '9' ||
                (cmp->str[cmp->pos] == '-' || cmp->str[cmp->pos] == '+')
                && '0' <= cmp->str[cmp->pos + 1] && cmp->str[cmp->pos + 1] <= '9')
            {
            ReadNumber(cmp, node);
            node = node->right;
            if (cmp->error == SyntaxError)
                {
                printf("Syntax error in pos %d: %s\n", cmp->pos, cmp->str);
                return SyntaxError;
                }
            }
        else if ('a' <= cmp->str[cmp->pos] && cmp->str[cmp->pos] <= 'z' ||
                 'A' <= cmp->str[cmp->pos] && cmp->str[cmp->pos] <= 'Z' ||
                 cmp->str[cmp->pos] == '_' || cmp->str[cmp->pos] == '$')
            {
            ReadKeyWord(cmp, node);
            node = node->right;
            if (cmp->error == SyntaxError)
                {
                printf("Syntax error in pos %d: %s\n", cmp->pos, cmp->str);
                return SyntaxError;
                }
            else if (cmp->error)
                {
                return cmp->error;
                }
            }
        else
            {
            ReadOperation(cmp, node);
            node = node->right;
            if (cmp->error == SyntaxError)
                {
                printf("Syntax error in pos %d: %s\n", cmp->pos, cmp->str);
                return SyntaxError;
                }
            }
        }

    Data_t data = {.punc = NULL_TERMINATOR};
    ListInsert(node, PUNCTUATION, data);

    return Ok;
    }

static void ReadNumber(Compiler* cmp, Node* node)
    {
    assert(cmp);
    assert(node);

    Data_t data = {.val = atof(cmp->str + cmp->pos)};
    ListInsert(node, VALUE, data);

    bool dot = false;

    if (cmp->str[cmp->pos] == '-' || cmp->str[cmp->pos] == '+')
        {
        ++cmp->pos;
        }

    while ('0' <= cmp->str[cmp->pos] && cmp->str[cmp->pos] <= '9' || cmp->str[cmp->pos] == '.')
        {
        if (cmp->str[cmp->pos] == '.')
            {
            if (dot)
                {
                cmp->error = SyntaxError;
                return;
                }
            else dot = true;
            }

        cmp->pos++;
        }
    }

static void ReadKeyWord(Compiler* cmp, Node* node)
    {
    assert(cmp);
    assert(node);

    if ('0' <= cmp->str[cmp->pos - 1] && cmp->str[cmp->pos - 1] <= '9' || cmp->str[cmp->pos - 1] == '.')
        {
        cmp->error = SyntaxError;
        return;
        }

    for (int i = 0; i < KEY_WORDS_COUNT; i++)
        {
        if (!strncmp(cmp->str + cmp->pos, KEY_WORDS[i].name, strlen(KEY_WORDS[i].name)))
            {
            cmp->pos += strlen(KEY_WORDS[i].name);
            if ('a' <= cmp->str[cmp->pos] && cmp->str[cmp->pos] <= 'z' ||
                'A' <= cmp->str[cmp->pos] && cmp->str[cmp->pos] <= 'Z' ||
                '0' <= cmp->str[cmp->pos] && cmp->str[cmp->pos] <= '9' ||
                cmp->str[cmp->pos] == '_' || cmp->str[cmp->pos] == '$')
                {
                cmp->pos -= strlen(KEY_WORDS[i].name);
                continue;
                }
            Data_t data = {.oper = KEY_WORDS[i].code};
            ListInsert(node, KEY_WORDS[i].type, data);
            return;
            }
        }

    int index = 0;

    for (; index < cmp->name_count; index++)
        {
        if (cmp->name_table[index].name &&
            !strncmp(cmp->str + cmp->pos, cmp->name_table[index].name, strlen(cmp->name_table[index].name)))
            {
            cmp->pos += strlen(cmp->name_table[index].name);
            if ('a' <= cmp->str[cmp->pos] && cmp->str[cmp->pos] <= 'z' ||
                'A' <= cmp->str[cmp->pos] && cmp->str[cmp->pos] <= 'Z' ||
                '0' <= cmp->str[cmp->pos] && cmp->str[cmp->pos] <= '9' ||
                cmp->str[cmp->pos] == '_' || cmp->str[cmp->pos] == '$')
                {
                cmp->pos -= strlen(cmp->name_table[index].name);
                continue;
                }
            Data_t data = {.var = index};
            ListInsert(node, cmp->name_table[index].type, data);
            return;
            }
        else if (!cmp->name_table[index].name) break;
        }

    if (index == cmp->name_count)
        {
        Object* new_table = (Object*) realloc(cmp->name_table, cmp->name_count * REALLOC_COEFFICENT * sizeof(Object));
        if (new_table == nullptr)
            {
            printf("Error: cannot allocate memory for name_table\n");
            cmp->error = AllocationError;
            return;
            }
        cmp->name_table = new_table;
        }

    int word_lenght = 0;

    while ( 'a' <= cmp->str[cmp->pos + word_lenght] && cmp->str[cmp->pos + word_lenght] <= 'z' ||
            'A' <= cmp->str[cmp->pos + word_lenght] && cmp->str[cmp->pos + word_lenght] <= 'Z' ||
            '0' <= cmp->str[cmp->pos + word_lenght] && cmp->str[cmp->pos + word_lenght] <= '9' ||
            cmp->str[cmp->pos + word_lenght] == '_' || cmp->str[cmp->pos + word_lenght] == '$')
            {
            word_lenght++;
            }

    char* name = (char*) calloc(word_lenght + 1, sizeof(char));
    if (name == nullptr)
        {
        printf("Error: cannot allocate memory for parametr\n");
        cmp->error = AllocationError;
        return;
        }
    strncpy(name, cmp->str + cmp->pos, word_lenght);
    name[word_lenght] = '\0';

    if (cmp->str[cmp->pos + word_lenght] == '(')
        {
        cmp->name_table[index].type = FUNCTION;
        }
    else
        {
        cmp->name_table[index].type = VARIABLE;
        }

    cmp->name_table[index].name = name;
    }


static void ReadOperation(Compiler* cmp, Node* node)
    {
    assert(cmp);
    assert(node);

    for (int i = 0; i < KEY_WORDS_COUNT; i++)
        {
        if (!strncmp(cmp->str + cmp->pos, KEY_WORDS[i].name, strlen(KEY_WORDS[i].name)))
            {
            cmp->pos += strlen(KEY_WORDS[i].name);
            Data_t data = {.oper = KEY_WORDS[i].code};
            ListInsert(node, KEY_WORDS[i].type, data);
            return;
            }
        }
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

static void syn_assert(int expr, Compiler *cmp)
    {
    if (!expr) syntax_error(cmp);
    }

static void syntax_error(Compiler *cmp)
    {
    printf("Syntax error\n");
    cmp->error = SyntaxError;
    exit(1);
    }

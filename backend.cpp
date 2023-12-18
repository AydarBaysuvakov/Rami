#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "errors.h"
#include "node.h"
#include "tree.h"
#include "backend.h"

static const char DEFAULT_ASM_FILENAME[] = "output.txt";

int main(int argc, char *argv[])
    {
    const char* file_from = nullptr;
    const char* file_to   = DEFAULT_ASM_FILENAME;

    if (argc < 2)
        {
        printf("Incorrect args number\n");
        return FileError;
        }
    else if (argc == 2)
        {
        file_from = argv[1];
        }
    else if (argc > 2)
        {
        file_from = argv[1];
        file_to   = argv[2];
        }

    Backend(file_from, file_to);
    return 0;
    }

Error_t Backend(const char* file_from, const char* file_to)
    {
    assert(file_from);
    assert(file_to);

    Compiler cmp = {};
    if (CompilerCtor(&cmp, file_from, file_to) != Ok)
        {
        return AllocationError;
        }

    if (ReadTree(&cmp.tree.root, cmp.file_from) != Ok)
        {
        CompilerDtor(&cmp);
        return SyntaxError;
        }

    TreeDump(&cmp.tree, 0);
    CompilerDtor(&cmp);

    return Ok;
    }


Error_t CompilerCtor(Compiler* cmp, const char* file_from, const char* file_to)
    {
    assert(cmp);
    assert(file_from);
    assert(file_to);

    cmp->file_from = fopen(file_from, "r");
    if (cmp->file_from == NULL)
        {
        perror("Cannot open file\n");
        return FileError;
        }

    cmp->file_to = fopen(file_to, "w");
    if (cmp->file_to == NULL)
        {
        perror("Cannot open file\n");
        fclose(cmp->file_from);
        return FileError;
        }

    TreeCtor(&cmp->tree);

    return Ok;
    }

Error_t CompilerDtor(Compiler* cmp)
    {
    assert(cmp);

    fclose(cmp->file_from);
    fclose(cmp->file_to);

    TreeDtor(&cmp->tree);

    return Ok;
    }

Error_t ReadTree(Node** node, FILE* fp)
    {
    assert(node);
    assert(fp);

    char c = ' ';
    while (isspace(c)) c = fgetc(fp);

    switch (c)
        {
        case '_': return Ok;
        case '(': break;
        case EOF: printf("Error: reached End of file\n");
                  return SyntaxError;
        default:  printf("Syntax error, wrong %c symbol\n", c);
                  return SyntaxError;
        }

    int type = 0;
    Data_t data = {.val = 0};
    fscanf(fp, "%d", &type);

    if  (type == VALUE)  fscanf(fp, "%lf", &data.val);
    else                 fscanf(fp, "%d", &data.var);

    if (NewNode(node, type, data) == AllocationError)
        {
        return AllocationError;
        }

    if (ReadTree(&(*node)->left,  fp) != Ok ||
        ReadTree(&(*node)->right, fp) != Ok)
        {
        return SyntaxError;
        }

    c = ' ';
    while (isspace(c)) c = fgetc(fp);

    if (c == ')') return Ok;
    else
        {
        printf("Error: forget to close bracket\n");
        return SyntaxError;
        }

    return Ok;
    }

Error_t WriteAsmCode(Compiler* cmp)
    {
    assert(cmp);

    return Ok;
    }

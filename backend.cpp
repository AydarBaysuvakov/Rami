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

    WriteAsmCode(&cmp);
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

    Node* command = cmp->tree.root;

    while (command)
        {
        if (WriteCommand(command->left, cmp->file_to) != Ok)
            {
            printf("Syntax error in program\n");
            return SyntaxError;
            }
        command = command->right;
        }

    return Ok;
    }

static int if_number = 0;
static int while_number = 0;

Error_t WriteCommand(Node* node, FILE* fp)
    {
    assert(node);
    assert(fp);

    fprintf(fp, "\n");

    if (node->type == OPERATION && node->data.oper == OP_ASSIGMENT)
        {
        return WriteAssigment(node, fp);
        }
    if (node->type == OPERATION && node->data.oper == OP_WHILE)
        {
        while_number += 1;
        return WriteWhile(node, fp);
        }
    if (node->type == OPERATION && (node->data.oper == OP_IF || node->data.oper == OP_ELSE))
        {
        if_number += 1;
        Error_t response = WriteIf(node, fp, 0);
        fprintf(fp, "end_if_%d:\n", if_number);
        return response;
        }
    if (node->type == OPERATION && node->data.oper == OP_NEXT_COMMAND)
        {
        return WriteBody(node, fp);
        }

    return WriteEquation(node, fp);
    }

Error_t WriteEquation(Node* node, FILE* fp)
    {
    assert(node);
    assert(fp);

    #define DEFINE_OPERATION(oper, code) case oper: code; break;

    switch (node->type)
        {
        case VALUE:
            fprintf(fp, "push %f\n", node->data.val);
            break;
        case VARIABLE:
            fprintf(fp, "push [%d]\n", node->data.var);
            break;
        case FUNCTION:
            fprintf(fp, "call func_%d\n", node->data.func);
            break;
        case OPERATION:
            switch (node->data.oper)
                {
                #include "operations.h"
                default:
                    printf("Syntax error: wrong operation\n");
                    return SyntaxError;
                }
            break;
        default:
            printf("Syntax error: wrong argument type\n");
            return SyntaxError;
        }

    #undef DEFINE_OPERATION

    return Ok;
    }

Error_t WriteAssigment(Node* node, FILE* fp)
    {
    assert(node);
    assert(fp);

    WriteEquation(node->right, fp);
    fprintf(fp, "pop [%d]\n", node->left->data.var);

    return Ok;
    }

Error_t WriteBody(Node* node, FILE* fp)
    {
    assert(node);
    assert(fp);

    while (node)
        {
        if (WriteCommand(node->left, fp) != Ok)
            {
            printf("Syntax error in program body\n");
            return SyntaxError;
            }
        node = node->right;
        }

    return Ok;
    }

Error_t WriteWhile(Node* node, FILE* fp)
    {
    assert(node);
    assert(fp);

    fprintf(fp, "while_%d:\n", while_number);
    WriteEquation(node->left, fp);
    fprintf(fp, "push 0\n");
    fprintf(fp, "je end_while_%d", while_number);
    WriteBody(node->right, fp);
    fprintf(fp, "jmp while_%d\n", while_number);
    fprintf(fp, "end_while_%d:\n", while_number);

    return Ok;
    }

Error_t WriteIf(Node* node, FILE* fp, int order)
    {
    assert(node);
    assert(fp);

    if (node->type == OPERATION && node->data.oper == OP_IF)
        {
        WriteEquation(node->left, fp);
        fprintf(fp, "push 0\n");
        fprintf(fp, "je end_if_%d\n", if_number);
        WriteBody(node->right, fp);
        fprintf(fp, "jmp end_if_%d\n\n", if_number);
        }
    else if (node->type == OPERATION && node->data.oper == OP_ELSE)
        {
        Node* if_node = node->left;
        WriteEquation(if_node->left, fp);
        fprintf(fp, "push 0\n");
        fprintf(fp, "jne if_%d_%d\n", if_number, order);
        WriteIf(node->right, fp, order + 1);
        fprintf(fp, "if_%d_%d:\n", if_number, order);
        WriteBody(if_node->right, fp);
        fprintf(fp, "jmp end_if_%d\n\n", if_number);
        }
    else
        {
        WriteBody(node, fp);
        fprintf(fp, "jmp end_if_%d\n", if_number);
        }

    return Ok;
    }

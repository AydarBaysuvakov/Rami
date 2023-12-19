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
    else                 fscanf(fp, "%d", &data.id);

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

    if (node->type == OPERATION)
        switch (node->data.id)
            {
            case OP_ASSIGMENT:
            case OP_ADD_ASSIGMENT:
            case OP_SUB_ASSIGMENT:
            case OP_MUL_ASSIGMENT:
            case OP_DIV_ASSIGMENT:
            case OP_POW_ASSIGMENT:
                {
                return WriteAssigment(node, fp);
                }
            case OP_WHILE:
                {
                while_number += 1;
                WriteWhile(node, fp);
                fprintf(fp, "end_while_%d:\n", while_number);
                return Ok;
                }
            case OP_IF: case OP_ELSE:
                {
                if_number += 1;
                WriteIf(node, fp, 0);
                fprintf(fp, "end_if_%d:\n", if_number);
                return Ok;
                }
            case OP_NEXT_COMMAND:
                {
                return WriteBody(node, fp);
                }
            case OP_DEFINE_VARIABLE:
                {
                return WriteDefineVariable(node, fp);
                }
            case OP_DEFINE_FUNCTION:
                {
                return WriteDefineFunction(node, fp);
                }
            case OP_DEFINE_ARRAY:
                {
                return WriteDefineArray(node, fp);
                }
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
            {
            fprintf(fp, "push %f\n", node->data.val);
            break;
            }
        case VARIABLE:
            {
            fprintf(fp, "push [%d]\n", node->data.id);
            break;
            }
        case FUNCTION:
            {
            Node* parametr = node->right;
            int   param_number = 0;
            while (parametr)
                {
                fprintf(fp, "push [%d]\n", parametr->left->data.id);
                fprintf(fp, "pop reg%d\n", param_number);

                parametr = parametr->right;
                param_number += 1;
                }
            fprintf(fp, "call func_%d\n", node->data.id);
            break;
            }
        case ARRAY:
            {
            fprintf(fp, "push [%d]\n", node->data.id * ARRAY_MAX_SIZE + ARRAY_SEGMENT + (int) node->right->data.val);
            break;
            }
        case OPERATION:
            {
            switch (node->data.id)
                {
                #include "operations.h"
                default:
                    printf("Syntax error: wrong operation %d %d\n", node->type, node->data.id);
                    return SyntaxError;
                }
            break;
            }
        default:
            {
            printf("Syntax error: wrong argument type\n");
            return SyntaxError;
            }
        }

    #undef DEFINE_OPERATION

    return Ok;
    }

Error_t WriteAssigment(Node* node, FILE* fp)
    {
    assert(node);
    assert(fp);

    int index = 0;

    if (node->left->type == VALUE)
        {
        index = node->left->data.id;
        }
    else if (node->left->type == ARRAY)
        {
        index = node->left->data.id * ARRAY_MAX_SIZE + ARRAY_SEGMENT;
        }

    fprintf(fp, "push [%d]\n", index);
    WriteEquation(node->right, fp);
    switch (node->data.id)
        {
        case OP_ASSIGMENT:
            break;
        case OP_ADD_ASSIGMENT:
            fprintf(fp, "add\n");
            break;
        case OP_SUB_ASSIGMENT:
            fprintf(fp, "sub\n");
            break;
        case OP_MUL_ASSIGMENT:
            fprintf(fp, "mul\n");
            break;
        case OP_DIV_ASSIGMENT:
            fprintf(fp, "div\n");
            break;
        case OP_POW_ASSIGMENT:
            fprintf(fp, "pow\n");
            break;
        }
    fprintf(fp, "pop [%d]\n", index);

    return Ok;
    }

Error_t WriteDefineVariable(Node* node, FILE* fp)
    {
    assert(node);
    assert(fp);

    WriteEquation(node->right, fp);
    fprintf(fp, "pop [%d]\n", node->left->data.id);

    return Ok;
    }

Error_t WriteDefineFunction(Node* node, FILE* fp)
    {
    assert(node);
    assert(fp);

    fprintf(fp, "call func_guard_%d\n", node->left->data.id);

    fprintf(fp, "func_%d:\n", node->left->data.id);

    Node* parametr = node->left->right;
    int   param_number = 0;
    while (parametr)
        {
        fprintf(fp, "push reg%d\n", param_number);
        fprintf(fp, "pop [%d]\n", parametr->left->left->data.id);

        parametr = parametr->right;
        param_number += 1;
        }

    WriteBody(node->right, fp);
    fprintf(fp, "ret\n");

    fprintf(fp, "func_guard_%d:\n", node->left->data.id);

    return Ok;
    }

Error_t WriteDefineArray(Node* node, FILE* fp)
    {
    assert(node);
    assert(fp);

    Node* parametr = node->right;
    int   param_number = 0;
    while (parametr && param_number < node->left->right->data.val)
        {
        WriteEquation(parametr->left, fp);
        fprintf(fp, "pop [%d]\n", node->left->data.id * ARRAY_MAX_SIZE + ARRAY_SEGMENT + param_number);

        parametr = parametr->right;
        param_number += 1;
        }

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

    return Ok;
    }

Error_t WriteIf(Node* node, FILE* fp, int order)
    {
    assert(node);
    assert(fp);

    if (node->type == OPERATION && node->data.id == OP_IF)
        {
        WriteEquation(node->left, fp);
        fprintf(fp, "push 0\n");
        fprintf(fp, "je end_if_%d\n", if_number);
        WriteBody(node->right, fp);
        fprintf(fp, "jmp end_if_%d\n\n", if_number);
        }
    else if (node->type == OPERATION && node->data.id == OP_ELSE)
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

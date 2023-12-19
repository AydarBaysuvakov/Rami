#ifndef BACKEND_H
#define BACKEND_H

const int ARRAY_MAX_SIZE = 60;
const int ARRAY_SEGMENT  = 800;

struct Compiler
    {
    FILE*       file_from;
    FILE*       file_to;
    Tree        tree;
    };

Error_t Backend(const char* file_from, const char* file_to);

Error_t CompilerCtor(Compiler* cmp, const char* file_from, const char* file_to);
Error_t CompilerDtor(Compiler* cmp);

Error_t ReadTree(Node** node, FILE* fp);

Error_t WriteAsmCode(Compiler* cmp);
Error_t WriteCommand(Node* node, FILE* fp);

Error_t WriteEquation(Node* node, FILE* fp);
Error_t WriteAssigment(Node* node, FILE* fp);
Error_t WriteBody(Node* node, FILE* fp);
Error_t WriteDefineVariable(Node* node, FILE* fp);
Error_t WriteDefineFunction(Node* node, FILE* fp);
Error_t WriteDefineArray(Node* node, FILE* fp);
Error_t WriteIf(Node* node, FILE* fp, const int order);
Error_t WriteWhile(Node* node, FILE* fp);

#endif //BACKEND_H

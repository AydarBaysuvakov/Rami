#ifndef BACKEND_H
#define BACKEND_H

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

#endif //BACKEND_H

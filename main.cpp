#include <stdio.h>
#include "errors.h"
#include "node.h"
#include "tree.h"
#include "list.h"
#include "frontend.h"

int main()
    {

    Compiler cmp = {};
    CompilerCtor(&cmp, "test.txt");
    printf("%s", cmp.str);
    CompilerDtor(&cmp);
    /*Compiler cmp = {.str = "568*52+45/5*(6-2)", .pos = 0, .tokens = nullptr, .error = 0};
    TreeCtor(&cmp.tree);
    TokenParsing(&cmp);
    TreeDump(&cmp.tree, 0);
    TreeDtor(&cmp.tree);*/
    return 0;
    }

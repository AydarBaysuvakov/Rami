#ifndef TREE_H
#define TREE_H

#define TREE_DEFN_ARGS const char* /* name */, const unsigned /* line */, \
                       const char* /* file */, const char*    /* func */
#define TREE_PASS_ARGS __LINE__, __FILE__, __FUNCTION__
#define TreeCtor(stk)         MyTreeCtor((stk),           #stk, TREE_PASS_ARGS)
#define TreeDump(stk, stk_st) MyTreeDump((stk), (stk_st), #stk, TREE_PASS_ARGS)

struct Tree
    {
    Node*  root;
    size_t size;

    const char* name;
    unsigned    line;
    const char* file;
    const char* func;

    FILE* logfile;
    int   dumps_count;
    };

enum TreeErrorBit
    {
    TreeNullptr                = 1 << 0,
    TreeWrongSize              = 1 << 1
    };

Error_t MyTreeCtor(Tree* tree, TREE_DEFN_ARGS);
Error_t TreeDtor(Tree* tree);

Error_t CopyTree(Node** dest, const Node* src);

Error_t PreorderNode(const Node* node, FILE* file = stdout);
Error_t PostorderNode(const Node* node, FILE* file = stdout);
Error_t InorderNode(const Node* node, FILE* file = stdout);

State_t TreeVerify(const Tree *tree);

Error_t MyTreeDump(Tree *tree, State_t state, TREE_DEFN_ARGS);
Error_t TreeDumpMessage(const Tree *tree, State_t state, TREE_DEFN_ARGS);
Error_t TreeDumpImage(const Tree *tree, FILE* fp, TREE_DEFN_ARGS);
Error_t TreeNodeDump(const Node *node, FILE *fp);

#endif //TREE_H

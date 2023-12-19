#ifndef LIST_H
#define LIST_H

#define LIST_DEFN_ARGS const char* /* name */, const unsigned /* line */, \
                       const char* /* file */, const char*    /* func */
#define LIST_PASS_ARGS __LINE__, __FILE__, __FUNCTION__
#define ListCtor(stk)         MyListCtor((stk),           #stk, LIST_PASS_ARGS)
#define ListDump(stk, stk_st) MyListDump((stk), (stk_st), #stk, LIST_PASS_ARGS)

struct List
    {
    Node*   head;

    const char* name;
    unsigned    line;
    const char* file;
    const char* func;

    FILE* logfile;
    int   dumps_count;
    };

enum ListErrorBit
    {
    ListNullptr                     = 1 << 0,
    InvalidHead                     = 1 << 1,
    InvalidTail                     = 1 << 2
    };

Error_t MyListCtor(List* list, LIST_DEFN_ARGS);
Error_t ListDtor(List *list);

Error_t ListInsert(Node* node, const int type, Data_t data);
Error_t ListExtract(Node* node);

State_t ListVerify(const List* list);

Error_t MyListDump(List *list, State_t state, LIST_DEFN_ARGS);
Error_t ListDumpMessage(const List *list, State_t state, LIST_DEFN_ARGS);
Error_t ListDumpImage(const List *list, FILE* fp, LIST_DEFN_ARGS);
Error_t ListNodeDump(const Node* node, FILE* fp);

#endif //LIST_H

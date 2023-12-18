#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "errors.h"
#include "logfiles.h"
#include "node.h"
#include "list.h"

static bool IsListErrorState(const State_t state);
static void ListAssert(List* list);
static const char* GetListErrorBitMsg(const size_t bit);

Error_t MyListCtor(List* list,
                   const char* name,
                   const unsigned line,
                   const char* file,
                   const char* func)
    {
    assert(list != NULL);

    list->head = (Node*) calloc(1, sizeof(Node));
    if (!list->head)
        {
        printf("Error: cannot allocate memory\n");
        return AllocationError;
        }

    list->head->type          = PUNCTUATION;
    list->head->data.punc     = PROGRAM_START;

    list->head->left   = nullptr;
    list->head->right  = nullptr;

    list->size = 0;

    list->name = name;
    list->line = line;
    list->file = file;
    list->func = func;

    if (LogFileInit(&list->logfile, "logfile", name, "html") == FileError)
        {
        perror("cannot open list logfile\n");
        return FileError;
        }

    list->dumps_count = 0;

    return Ok;
    }

Error_t ListDtor(List *list)
    {
    ListAssert(list);

    if (list->head) DestroyListNode(list->head);

    fclose(list->logfile);

    return Ok;
    }

Error_t ListInsert(Node* node, const int type, const Data_t data)
    {
    assert(node != NULL);

    Node* new_node = (Node*) calloc(1, sizeof(Node));
    if (!new_node)
        {
        printf("Error: cannot allocate memory\n");
        return AllocationError;
        }

    new_node->type   = type;

    switch (type)
        {
        case VALUE:
            new_node->data.val  = data.val;
            break;
        case VARIABLE:
            new_node->data.var  = data.var;
            break;
        case OPERATION:
            new_node->data.oper = data.oper;
            break;
        case FUNCTION:
            new_node->data.func = data.func;
            break;
        case PUNCTUATION:
            new_node->data.punc = data.punc;
            break;
        }

    new_node->left   = node;
    new_node->right  = node->right;

    if (node->right) node->right->left = new_node;
    node->right                        = new_node;

    return Ok;
    }

Error_t ListExtract(Node* node)
    {
    assert(node);

    if (node->right) node->right->left = node->left;
    if (node->left ) node->left->right = node->right;

    free(node);

    return Ok;
    }

Error_t DestroyListNode(Node* node)
    {
    assert(node != NULL);

    if (node->left) DestroyListNode(node->left);

    free(node);

    return Ok;
    }

State_t ListVerify(const List* list)
    {
    State_t state = 0;

    if (list == NULL)
        {
        state |= ListNullptr;
        return state;
        }

    return state;
    }

static void ListAssert(List* list)
    {
    State_t state = ListVerify(list);
    if (state) ListDump(list, state);
    }

static bool IsListErrorState(State_t state)
    {
    return state;
    }

Error_t MyListDump( List *list,
                   State_t state,
                   const char* name,
                   const unsigned line,
                   const char* file,
                   const char* func)
    {
    assert(list != NULL);
    assert(list->logfile != NULL);

    ListDumpMessage(list, state, name, line, file, func);

    FILE* img_file_ptr  = nullptr;
    char  img_file_name[LOG_FILE_NAME_LENGTH] = "";
    char  obj_name[LOG_FILE_NAME_LENGTH] = "";
    snprintf(obj_name, sizeof(obj_name), "%s%d", name, list->dumps_count);

    if (LogFileInit(&img_file_ptr, "img_logfile", obj_name, "dot", img_file_name) == FileError)
        {
        printf("ERROR: cannot open img_logfile");
        return FileError;
        }

    ListDumpImage(list, img_file_ptr, name, line, file, func);

    fclose(img_file_ptr);

    char command[COMMAND_LENGTH] = "";
    snprintf(command, sizeof(command), "dot -v -Tpng %s -o logfiles/list_img_log_%d.png", img_file_name, list->dumps_count);
    system(command);

    ++list->dumps_count;

    return Ok;
    }

Error_t ListDumpMessage(const List *list,
                        State_t state,
                        const char* name,
                        const unsigned line,
                        const char* file,
                        const char* func)
    {
    assert(list != NULL);
    assert(list->logfile != NULL);

    fprintf(list->logfile, "<pre>\n\n");

    fprintf(list->logfile, "List[%p] '%s' from %s(%u) %s()\n", list, list->name, list->file, list->line, list->func);
    fprintf(list->logfile, "\tcalled like '%s' from %s(%u) %s()\n",        name,       file,       line,       func);
    fprintf(list->logfile, "\tlist:\n");
    Node* node = list->head;

    while (node->right)
        {
        switch (node->type)
            {
            case VALUE:
                fprintf(list->logfile, "\t\tVALUE: %f\n", node->data.val);
                break;
            case VARIABLE:
                fprintf(list->logfile, "\t\tVARIABLE: %d\n", node->data.var);
                break;
            case OPERATION:
                fprintf(list->logfile, "\t\tOPERATION: %d\n", node->data.oper);
                break;
            case FUNCTION:
                fprintf(list->logfile, "\t\tFUNCTION: %d\n", node->data.func);
                break;
            case PUNCTUATION:
                fprintf(list->logfile, "\t\tPUNCTUATION: %d\n", node->data.punc);
                break;
            }
        node = node->right;
        }

    fprintf(list->logfile, "\tsize = %ld\n\t}\n\n", list->size);

    for(size_t bit = 0; bit < CHAR_BIT * sizeof(state); bit++)
        {
        if (state & 1 << bit)
            {
            fprintf(list->logfile, "%s\n", GetListErrorBitMsg(bit));
            }
        }

    fprintf(list->logfile, "</pre>\n\n");

    return Ok;
    }

static const char* GetListErrorBitMsg(size_t bit)
    {
    static const int  ERROR_COUNT = sizeof(State_t) * CHAR_BIT;
    static const char * const ERROR_MESSAGES[ERROR_COUNT] = {
        "List is nullptr",
        "List->array is nullptr",
        "List->free is invalid",
        "List->head is invalid",
        "List->tail is invalid",
        "list->array[0].next != list->head",
        "list->array[0].prev != list->tail"
    };

    return ERROR_MESSAGES[bit];
    }

Error_t ListDumpImage(const List *list,
                      FILE* fp,
                      const char* name,
                      const unsigned line,
                      const char* file,
                      const char* func)
    {
    assert(list != NULL);
    assert(fp != NULL);

    fprintf(fp,                 "digraph MyList\n{\n"
                                "\trankdir = LR;\n"
                                "\tbgcolor = darkgreen;\n"
                                "\tgraph [splines = splines];\n"
                                "\tnode  [width = 2, style = filled, color = wheat];\n"
                                "\tedge  [color = darkgreen, fontsize = 15];\n\n");

    //style

    fprintf(fp,                 "\tsubgraph cluster%d\n    {\n"
                                "\t\tbgcolor = yellowgreen;"
                                "\t\theight  = 20"
                                "\t\tstyle   = filled;\n"
                                "\t\tlabel   = \"List[%p] called like '%s' from %s(%u) %s()\";\n\n",
                                list->dumps_count, list, name, file, line, func);

    ListNodeDump(list->head, fp);

    fprintf(fp, "\t}\n}\n");

    return Ok;
    }

Error_t ListNodeDump(const Node *node, FILE *fp)
    {
    assert(fp != NULL);

    if (!node)
        {
        fprintf(fp,  "\t\t\"0\" [shape=oval, height = 1, label = \"nil\"];\n");
        return Ok;
        }
    else if (node->type == VALUE) fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"VALUE: %f\"];\n", node, node->data.val);
    else if (node->type == VARIABLE) fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"VARIABLE %d\"];\n", node, node->data.var);
    else if (node->type == FUNCTION) fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"FUNCTIOn %d\"];\n", node, node->data.func);
    else if (node->type == PUNCTUATION)
        {
        switch (node->data.punc)
            {
            case PROGRAM_START:     fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"START\"];\n", node); break;
            case NULL_TERMINATOR:   fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"END\"];\n", node); break;
            case OPEN_BRACKET:      fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"(\"];\n", node); break;
            case CLOSE_BRACKET:     fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \")\"];\n", node); break;
            case OPEN_BRACE:        fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"{\"];\n", node); break;
            case CLOSE_BRACE:       fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"}\"];\n", node); break;
            case COLON:             fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \":\"];\n", node); break;
            case COMMA:             fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \",\"];\n", node); break;
            default:                fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"unknown punctuation\"];\n", node);
            }
        }
    else if (node->type == OPERATION)
        {
        switch (node->data.oper)
            {
            case OP_NEXT_COMMAND:   fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \";\"];\n", node); break;
            case OP_NEXT_PARAMETR:  fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \",\"];\n", node); break;
            case OP_ASSIGMENT:      fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"=\"];\n", node); break;
            case OP_EQUAL:          fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"==\"];\n", node); break;
            case OP_ADD:            fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"+\"];\n", node); break;
            case OP_SUB:            fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"-\"];\n", node); break;
            case OP_MUL:            fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"*\"];\n", node); break;
            case OP_DIV:            fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"/\"];\n", node); break;
            case OP_POW:            fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"^\"];\n", node); break;
            case OP_SIN:            fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"sin\"];\n", node); break;
            case OP_COS:            fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"cos\"];\n", node); break;
            case OP_LOG:            fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"log\"];\n", node); break;
            case OP_EXP:            fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"exp\"];\n", node); break;
            case OP_SQRT:           fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"sqrt\"];\n", node); break;
            case OP_IF:             fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"if\"];\n", node); break;
            case OP_WHILE:          fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"while\"];\n", node); break;
            case OP_ELSE:           fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"else\"];\n", node); break;
            case OP_AND:            fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"and\"];\n", node); break;
            case OP_OR:             fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"or\"];\n", node); break;
            default:                fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"unknown operator\"];\n", node);
            }
        }

    if (node->right)
        {
        fprintf(fp, "\t\t\"%p\" -> \"%p\" [color = cyan];\n", node, node->right);
        ListNodeDump(node->right, fp);
        }

    return Ok;
    }

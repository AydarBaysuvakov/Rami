#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "errors.h"
#include "logfiles.h"
#include "node.h"
#include "tree.h"

static bool IsTreeErrorState(const State_t state);
static void TreeAssert(Tree* tree);
static const char* GetTreeErrorBitMsg(const size_t bit);

Error_t MyTreeCtor(Tree* tree,
                   const char* name,
                   const unsigned line,
                   const char* file,
                   const char* func)
    {
    assert(tree != NULL);

    tree->root = nullptr;
    tree->size = 0;

    tree->name = name;
    tree->line = line;
    tree->file = file;
    tree->func = func;

    if (LogFileInit(&tree->logfile, "txt_logfile", name, "html") == FileError)
        {
        perror("cannot open tree logfile\n");
        return FileError;
        }

    tree->dumps_count = 0;

    return Ok;
    }

Error_t TreeDtor(Tree* tree)
    {
    TreeAssert(tree);

    if (tree->root) DeleteNode(tree->root);

    fclose(tree->logfile);

    return Ok;
    }

Error_t CopyTree(Node** dest, const Node* src)
    {
    assert(src != NULL);

    if (*dest) DeleteNode(*dest);
    *dest = nullptr;

    if (NewNode(dest, src->type, src->data) == AllocationError)
        {
        printf("Внимание: в связи с ошибкой копирование дерева не завершилось.\n");
        return CopyError;
        }

    if (src->left)  CopyTree(&(*dest)->left,  src->left);
    if (src->right) CopyTree(&(*dest)->right, src->right);

    return Ok;
    }

Error_t PreorderNode(const Node* node, FILE* file)
    {
    if (!node)
        {
        fprintf(file, "_ ");
        return Ok;
        }

    fprintf(file, "( ");

    fprintf(file, "%d ", node->type);

    if (node->type == VALUE) fprintf(file, "%f ", node->data.val);
    else fprintf(file, "%d ", node->data.oper);

    PreorderNode(node->left, file);
    PreorderNode(node->right, file);

    fprintf(file, ") ");

    return Ok;
    }

Error_t PostorderNode(const Node* node, FILE* file)
    {
    if (!node)
        {
        fprintf(file, "_ ");
        return Ok;
        }

    fprintf(file, "( ");

    PostorderNode(node->left, file);
    PostorderNode(node->right, file);

    fprintf(file, "%d ", node->type);

    if (node->type == VALUE) fprintf(file, "%f ", node->data.val);
    else fprintf(file, "%d ", node->data.oper);

    fprintf(file, ") ");

    return Ok;
    }


Error_t InorderNode(const Node* node, FILE* file)
    {
    if (!node)
        {
        fprintf(file, "_ ");
        return Ok;
        }

    fprintf(file, "( ");

    InorderNode(node->left, file);

    fprintf(file, "%d ", node->type);

    if (node->type == VALUE) fprintf(file, "%f ", node->data.val);
    else fprintf(file, "%d ", node->data.oper);

    InorderNode(node->right, file);

    fprintf(file, ") ");

    return Ok;
    }

State_t TreeVerify(const Tree *tree)
    {
    State_t state = 0;

    if (tree == NULL)
        {
        state |= TreeNullptr;
        return state;
        }

    return state;
    }

static void TreeAssert(Tree* tree)
    {
    State_t state = TreeVerify(tree);
    if (state) TreeDump(tree, state);
    }

static bool IsTreeErrorState(const State_t state)
    {
    return state;
    }

Error_t MyTreeDump(Tree *tree,
                   State_t state,
                   const char* name,
                   const unsigned line,
                   const char* file,
                   const char* func)
    {
    assert(tree != NULL);
    assert(tree->logfile != NULL);

    TreeDumpMessage(tree, state, name, line, file, func);

    FILE* img_file_ptr  = nullptr;
    char  img_file_name[LOG_FILE_NAME_LENGTH] = "";
    char  obj_name[LOG_FILE_NAME_LENGTH] = "";
    snprintf(obj_name, sizeof(obj_name), "%s%d", name, tree->dumps_count);

    if (LogFileInit(&img_file_ptr, "img_logfile", obj_name, "dot", img_file_name) == FileError)
        {
        printf("ERROR: cannot open img_logfile");
        return FileError;
        }

    TreeDumpImage(tree, img_file_ptr, name, line, file, func);

    fclose(img_file_ptr);

    char command[COMMAND_LENGTH] = "";
    snprintf(command, sizeof(command), "dot -v -Tpng %s -o logfiles/tree_img_log_%d.png", img_file_name, tree->dumps_count);
    system(command);

    ++tree->dumps_count;

    return Ok;
    }


Error_t TreeDumpMessage(const Tree *tree,
                        State_t state,
                        const char* name,
                        const unsigned line,
                        const char* file,
                        const char* func)
    {
    assert(tree != NULL);
    assert(tree->logfile != NULL);

    fprintf(tree->logfile, "<pre>\n\n");

    fprintf(tree->logfile, "Tree[%p] '%s' from %s(%u) %s()\n", tree, tree->name, tree->file, tree->line, tree->func);
    fprintf(tree->logfile, "\tcalled like '%s' from %s(%u) %s()\n\n",      name,       file,       line,       func);
    fprintf(tree->logfile, "\ttree:");

    if (tree->root)
        {
        PreorderNode(tree->root, tree->logfile);
        }
    else
        {
        fprintf(tree->logfile, "\t\tnil");
        }

    fprintf(tree->logfile, "\tsize = %ld\n\t}\n\n", tree->size);

    for(size_t bit = 0; bit < CHAR_BIT * sizeof(state); bit++)
        {
        if (state & 1 << bit)
            {
            fprintf(tree->logfile, "%s\n", GetTreeErrorBitMsg(bit));
            }
        }

    fprintf(tree->logfile, "</pre>\n\n");

    return Ok;
    }

static const char* GetTreeErrorBitMsg(const size_t bit)
    {
    static const int  ERROR_COUNT = sizeof(State_t) * CHAR_BIT;
    static const char * const ERROR_MESSAGE[ERROR_COUNT] = {
        "Tree is nullptr",
        "Tree fas wrong size",};

    return ERROR_MESSAGE[bit];
    }

Error_t TreeDumpImage(const Tree *tree,
                        FILE* fp,
                        const char* name,
                        const unsigned line,
                        const char* file,
                        const char* func)
    {
    assert(tree != NULL);
    assert(fp != NULL);

    fprintf(fp,                 "digraph MyTree\n{\n"
                                "\tbgcolor = darkgreen;\n"
                                "\tgraph [splines = splines];\n"
                                "\tnode  [width = 2, style = filled, color = wheat];\n"
                                "\tedge  [color = darkgreen, fontsize = 15];\n\n");

    //style

    fprintf(fp,                 "\tsubgraph cluster%d\n    {\n"
                                "\t\tbgcolor = yellowgreen;"
                                "\t\theight  = 20"
                                "\t\tstyle   = filled;\n"
                                "\t\tlabel   = \"Tree[%p] called like '%s' from %s(%u) %s()\";\n\n",
                                tree->dumps_count, tree, name, file, line, func);

    TreeNodeDump(tree->root, fp);

    fprintf(fp, "\t}\n}\n");

    return Ok;
    }

Error_t TreeNodeDump(const Node *node, FILE *fp)
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
            case OP_GREATER:        fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \">\"];\n", node); break;
            case OP_LESS:           fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"<\"];\n", node); break;
            case OP_GREATER_EQUAL:  fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \">=\"];\n", node); break;
            case OP_LESS_EQUAL:     fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"<=\"];\n", node); break;
            case OP_EQUAL:          fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"==\"];\n", node); break;
            case OP_NOT_EQUAL:      fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"!=\"];\n", node); break;
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
            case OP_NOT:            fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"not\"];\n", node); break;
            case OP_INPUT:          fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"input\"];\n", node); break;
            case OP_OUTPUT:         fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"output\"];\n", node); break;
            default:                fprintf(fp,  "\t\t\"%p\" [shape=oval, height = 1, label = \"unknown operator\"];\n", node);
            }
        }

    if (node->left)
        {
        fprintf(fp, "\t\t\"%p\" -> \"%p\" [color = red];\n", node, node->left);
        TreeNodeDump(node->left, fp);
        }

    if (node->right)
        {
        fprintf(fp, "\t\t\"%p\" -> \"%p\" [color = cyan];\n", node, node->right);
        TreeNodeDump(node->right, fp);
        }

    return Ok;
    }

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include "errors.h"
#include "node.h"
#include "tree.h"
#include "list.h"
#include "frontend.h"

static void ReadNumber(Compiler* cmp, Node* node);
static void ReadKeyWord(Compiler* cmp, Node* node);
static void NewKeyWord(Compiler* cmp, Node* node, int index);
static void ReadOperation(Compiler* cmp, Node* node);

static int  IsLetter(char* str, int pos);
static int  IsNumber(char* str, int pos);

static const char DEFAULT_TREE_FILENAME[] = "tree.txt";

int main(int argc, char *argv[])
    {
    const char* file_from = nullptr;
    const char* file_to   = DEFAULT_TREE_FILENAME;

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

    Frontend(file_from, file_to);
    return 0;
    }

Error_t Frontend(const char* file_from, const char* file_to)
    {
    assert(file_from);
    assert(file_to);

    Compiler cmp = {};
    CompilerCtor(&cmp, file_from);
    if (cmp.error)
        {
        return cmp.error;
        }

    TokenParsing(&cmp);
    if (cmp.error)
        {
        CompilerDtor(&cmp);
        return cmp.error;
        }

    //ListDump(&cmp.tokens, 0);

    if (GetGrammar(&cmp) != Ok)
        {
        TreeDump(&cmp.tree, 0);
        CompilerDtor(&cmp);
        return SyntaxError;
        }
    TreeDump(&cmp.tree, 0);

    WriteTree(&cmp, file_to);
    if (cmp.error)
        {
        CompilerDtor(&cmp);
        return cmp.error;
        }

    CompilerDtor(&cmp);
    return Ok;
    }

Error_t CompilerCtor(Compiler* cmp, const char* filename)
    {
    assert(cmp);
    assert(filename);

    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
        {
        perror("Cannot open file\n");
        cmp->error = FileError;
        return FileError;
        }

    struct stat sb = {0};
    int fd = fileno(fp);

    if (fstat(fd, &sb) == -1)
        {
        perror("fstat() returned -1");
        fclose(fp);
        cmp->error = FileError;
        return FileError;
        }

    cmp->size = sb.st_size;

    cmp->str = (char*) calloc(cmp->size, sizeof(char));
    if (cmp->str == nullptr)
        {
        perror("Cannot allocate memory for cmp->str");
        fclose(fp);
        cmp->error = AllocationError;
        return AllocationError;
        }

    fread(cmp->str, sizeof(char), cmp->size, fp);
    if (ferror(fp))
        {
        perror("Cannot read file\n");
        fclose(fp);
        free(cmp->str);
        cmp->error = FileError;
        return FileError;
        }

    fclose(fp);

    cmp->name_count = DEFAULT_NAME_COUNT;
    cmp->name_table = (Object*) calloc(cmp->name_count, sizeof(Object));
    if (cmp->name_table == nullptr)
        {
        perror("Cannot allocate memory for cmp->name_table");
        free(cmp->str);
        cmp->error = AllocationError;
        return AllocationError;
        }

    for (int i = 0; i < cmp->name_count; i++)
        {
        cmp->name_table[i].type = VARIABLE;
        cmp->name_table[i].name = nullptr;
        }

    cmp->arr_count = 0;
    cmp->var_count = 0;
    cmp->func_count = 0;

    cmp->error      = Ok;

    ListCtor(&cmp->tokens);
    TreeCtor(&cmp->tree);

    return Ok;
    }

Error_t CompilerDtor(Compiler* cmp)
    {
    assert(cmp);

    cmp->pos        = 0;
    cmp->size       = 0;
    cmp->error      = Ok;

    free(cmp->str);
    for (int i = 0; i < cmp->name_count; i++)
        {
        if (cmp->name_table[i].name) free(cmp->name_table[i].name);
        }
    free(cmp->name_table);

    cmp->name_count = 0;
    cmp->var_count = 0;
    cmp->func_count = 0;

    ListDtor(&cmp->tokens);
    TreeDtor(&cmp->tree);

    return Ok;
    }

Error_t WriteTree(Compiler* cmp, const char* filename)
    {
    assert(cmp);
    assert(filename);

    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
        {
        perror("Cannot open file\n");
        cmp->error = FileError;
        return FileError;
        }

    PreorderNode(cmp->tree.root, fp);

    fclose(fp);

    return Ok;
    }

Error_t TokenParsing(Compiler* cmp)
    {
    assert(cmp);

    Node* node = cmp->tokens.head;

    while (cmp->pos < cmp->size)
        {
        if (isspace(cmp->str[cmp->pos])) cmp->pos++;
        else if (IsNumber(cmp->str, cmp->pos))
            {
            ReadNumber(cmp, node);
            }
        else if (IsLetter(cmp->str, cmp->pos))
            {
            ReadKeyWord(cmp, node);
            }
        else
            {
            ReadOperation(cmp, node);
            }
        if (node->right) node = node->right;
        if (cmp->error)  return cmp->error;
        }

    Data_t data = {.id = NULL_TERMINATOR};
    ListInsert(node, PUNCTUATION, data);

    return Ok;
    }

static void ReadNumber(Compiler* cmp, Node* node)
    {
    assert(cmp);
    assert(node);

    Data_t data = {.val = atof(cmp->str + cmp->pos)};
    ListInsert(node, VALUE, data);

    bool dot = false;

    if (cmp->str[cmp->pos] == '-' || cmp->str[cmp->pos] == '+')
        {
        ++cmp->pos;
        }

    while ('0' <= cmp->str[cmp->pos] && cmp->str[cmp->pos] <= '9' || cmp->str[cmp->pos] == '.')
        {
        if (cmp->str[cmp->pos] == '.')
            {
            if (dot)
                {
                printf("Syntax error in pos %d: %s\n", cmp->pos, cmp->str + cmp->pos);
                cmp->error = SyntaxError;
                return;
                }
            else dot = true;
            }

        ++cmp->pos;
        }
    }

static void ReadKeyWord(Compiler* cmp, Node* node)
    {
    assert(cmp);
    assert(node);

    if (cmp->pos > 0 && ('0' <= cmp->str[cmp->pos - 1] && cmp->str[cmp->pos - 1] <= '9' || cmp->str[cmp->pos - 1] == '.'))
        {
        printf("Syntax error in pos %d: %s\n", cmp->pos, cmp->str + cmp->pos);
        cmp->error = SyntaxError;
        return;
        }

    for (int i = 0; i < KEY_WORDS_COUNT; i++)
        {
        if (!strncmp(cmp->str + cmp->pos, KEY_WORDS[i].name, strlen(KEY_WORDS[i].name)))
            {
            cmp->pos += strlen(KEY_WORDS[i].name);
            if (IsLetter(cmp->str, cmp->pos) ||
                '0' <= cmp->str[cmp->pos] && cmp->str[cmp->pos] <= '9')
                {
                cmp->pos -= strlen(KEY_WORDS[i].name);
                continue;
                }
            Data_t data = {.id = KEY_WORDS[i].code};
            ListInsert(node, KEY_WORDS[i].type, data);
            return;
            }
        }

    int index = 0;

    for (; index < cmp->name_count && cmp->name_table[index].name; index++)
        {
        if (!strncmp(cmp->str + cmp->pos, cmp->name_table[index].name, strlen(cmp->name_table[index].name)))
            {
            cmp->pos += strlen(cmp->name_table[index].name);
            if (IsLetter(cmp->str, cmp->pos) ||
                '0' <= cmp->str[cmp->pos] && cmp->str[cmp->pos] <= '9')
                {
                cmp->pos -= strlen(cmp->name_table[index].name);
                continue;
                }
            Data_t data = {.id = cmp->name_table[index].index};
            ListInsert(node, cmp->name_table[index].type, data);
            return;
            }
        }

    NewKeyWord(cmp, node, index);
    }

static void NewKeyWord(Compiler* cmp, Node* node, int index)
    {
    assert(cmp);
    assert(node);

    if (index == cmp->name_count)
        {
        Object* new_table = (Object*) realloc(cmp->name_table, cmp->name_count * REALLOC_COEFFICENT * sizeof(Object));
        if (new_table == nullptr)
            {
            printf("Error: cannot allocate memory for name_table\n");
            cmp->error = AllocationError;
            return;
            }
        cmp->name_table = new_table;
        }

    int word_length = 0;
    int letter_length = 0;

    while (true)
        {
        if (letter_length = IsLetter(cmp->str, cmp->pos + word_length)) word_length += letter_length;
        else if ('0' <= cmp->str[cmp->pos + word_length] && cmp->str[cmp->pos + word_length] <= '9') word_length += 1;
        else break;
        }

    char* name = (char*) calloc(word_length + 1, sizeof(char));
    if (name == nullptr)
        {
        printf("Error: cannot allocate memory for parametr\n");
        cmp->error = AllocationError;
        return;
        }
    strncpy(name, cmp->str + cmp->pos, word_length);
    name[word_length] = '\0';

    cmp->name_table[index].name = name;
    if (cmp->str[cmp->pos + word_length] == '(')
        {
        if (node->type == OPERATION && node->data.id == OP_DEFINE_FUNCTION)
            {
            cmp->name_table[index].index = cmp->func_count++;
            cmp->name_table[index].type  = FUNCTION;
            }
        else
            {
            printf("Syntax error: function %s not defined\n", name);
            cmp->error = SyntaxError;
            return;
            }
        }
    else if (cmp->str[cmp->pos + word_length] == '[')
        {
        if (node->type == OPERATION && node->data.id == OP_DEFINE_ARRAY)
            {
            cmp->name_table[index].index = cmp->arr_count++;
            cmp->name_table[index].type = ARRAY;
            }
        else
            {
            printf("Syntax error: array %s not defined\n", name);
            cmp->error = SyntaxError;
            return;
            }
        }
    else
        {
        if (node->type == OPERATION && node->data.id == OP_DEFINE_VARIABLE)
            {
            cmp->name_table[index].index = cmp->var_count++;
            cmp->name_table[index].type = VARIABLE;
            }
        else
            {
            printf("Syntax error: variable %s not defined\n", name);
            cmp->error = SyntaxError;
            return;
            }
        }

    Data_t data = {.id = cmp->name_table[index].index};
    ListInsert(node, cmp->name_table[index].type, data);

    cmp->pos += word_length;
    }

static void ReadOperation(Compiler* cmp, Node* node)
    {
    assert(cmp);
    assert(node);

    for (int i = 0; i < KEY_WORDS_COUNT; i++)
        {
        if (!strncmp(cmp->str + cmp->pos, KEY_WORDS[i].name, strlen(KEY_WORDS[i].name)))
            {
            cmp->pos += strlen(KEY_WORDS[i].name);
            Data_t data = {.id = KEY_WORDS[i].code};
            ListInsert(node, KEY_WORDS[i].type, data);
            return;
            }
        }

    //printf("Syntax error in pos %d: %s\n", cmp->pos, cmp->str + cmp->pos);
    printf("Syntax error in pos %d %d\n", cmp->pos, cmp->size);
    cmp->error = SyntaxError;
    return;
    }

static const int ENG_LETTER_SIZE  = 1;
static const int BASH_LETTER_SIZE = 2;
static const int BASHKIR_LETTERS_COUNT = 84;
static const char * const BASHKIR_LETTERS[BASHKIR_LETTERS_COUNT] = {    "А", "а", "Б", "б", "В", "в", "Г", "г", "Ғ", "ғ",
                                                                        "Д", "д", "Ҙ", "ҙ", "Е", "е", "Ё", "ё", "Ж", "ж",
                                                                        "З", "з", "И", "и", "Й", "й", "К", "к", "Ҡ", "ҡ",
                                                                        "Л", "л", "М", "м", "Н", "н", "Ң", "ң", "О", "о",
                                                                        "Ө", "ө", "П", "п", "Р", "р", "С", "с", "Ҫ", "ҫ",
                                                                        "Т", "т", "У", "у", "Ү", "ү", "Ф", "ф", "Х", "х",
                                                                        "Һ", "һ", "Ц", "ц", "Ч", "ч", "Ш", "ш", "Щ", "щ",
                                                                        "Ъ", "ъ", "Ы", "ы", "Ь", "ь", "Э", "э", "Ә", "ә",
                                                                        "Ю", "ю", "Я", "Я"};


static int IsLetter(char* str, int pos)
    {
    assert(str);

    if ('A' <= str[pos] && str[pos] <= 'Z') return ENG_LETTER_SIZE;
    if ('a' <= str[pos] && str[pos] <= 'z') return ENG_LETTER_SIZE;
    if (str[pos] == '_' || str[pos] == '$') return ENG_LETTER_SIZE;

    for (int i = 0; i < BASHKIR_LETTERS_COUNT; i++)
        {
        if (!strncmp(str + pos, BASHKIR_LETTERS[i], BASH_LETTER_SIZE))
            {
            return BASH_LETTER_SIZE;
            }
        }

    return 0;
    }

static int IsNumber(char* str, int pos)
    {
    assert(str);

    return '0' <= str[pos] && str[pos] <= '9' ||
            (str[pos] == '-' || str[pos] == '+')
            && '0' <= str[pos + 1] && str[pos + 1] <= '9';
    }

Error_t GetGrammar(Compiler* cmp)
    {
    assert(cmp);

    if (cmp->tokens.head->type == PUNCTUATION && cmp->tokens.head->data.id == PROGRAM_START)
        {
        cmp->tokens.head = cmp->tokens.head->right;
        if (GetOperation(&cmp->tree.root, &cmp->tokens) == Ok)
            {
            if (cmp->tokens.head->type == PUNCTUATION && cmp->tokens.head->data.id == NULL_TERMINATOR)
                {
                return Ok;
                }
            }
        }

    printf("Grammar error\n");
    return SyntaxError;
    }

Error_t GetOperation(Node** node, List* tokens)
    {
    assert(node);
    assert(tokens);

    if  (tokens->head->type == PUNCTUATION &&
        (tokens->head->data.id == NULL_TERMINATOR || tokens->head->data.id == CLOSE_BRACE))
        {
        return Ok;
        }

    Error_t response = Ok;
    if (tokens->head->type == OPERATION && tokens->head->data.id == OP_IF)
        {
        response = GetIf(node, tokens);
        }
    else if (tokens->head->type == OPERATION && tokens->head->data.id == OP_WHILE)
        {
        response = GetWhile(node, tokens);
        }
    else if (tokens->head->type == PUNCTUATION && tokens->head->data.id == OPEN_BRACE)
        {
        response = GetBody(node, tokens);
        }
    else if (tokens->head->type == OPERATION && tokens->head->data.id == OP_DEFINE_VARIABLE)
        {
        response = GetDefineVariable(node, tokens);
        }
    else if (tokens->head->type == OPERATION && tokens->head->data.id == OP_DEFINE_ARRAY)
        {
        response = GetDefineArray(node, tokens);
        }
    else if (tokens->head->type == OPERATION && tokens->head->data.id == OP_DEFINE_FUNCTION)
        {
        response = GetDefineFunction(node, tokens);
        }
    else if (tokens->head->type == OPERATION      &&
            (tokens->head->data.id == OP_BREAK    ||
             tokens->head->data.id == OP_CONTINUE ||
             tokens->head->data.id == OP_RETURN))
        {
        response = NewNode(node, OPERATION, tokens->head->data);
        tokens->head = tokens->head->right;
        if (response == Ok && tokens->head->data.id != OP_NEXT_COMMAND)
            {
            response = GetExpression2(&(*node)->right, tokens);
            }
        }
    else
        {
        response = GetAssigment(node, tokens);
        if (response == NotAssigment)
            {
            response = GetExpression2(node, tokens);
            }
        }

    if (response != Ok) return SyntaxError;

    if (tokens->head->type == OPERATION && tokens->head->data.id == OP_NEXT_COMMAND)
        {
        Node* operation = *node;
        *node = nullptr;
        if (NewNode(node, OPERATION, tokens->head->data) == Ok)
            {
            (*node)->left = operation;

            tokens->head = tokens->head->right;
            return GetOperation(&(*node)->right, tokens);
            }
        return SyntaxError;
        }

    printf("Operation error (Missed ';')\n");
    return SyntaxError;
    }

Error_t GetDefineVariable(Node** node, List* tokens)
    {
    assert(node);
    assert(tokens);

    if (tokens->head->type == OPERATION && tokens->head->data.id == OP_DEFINE_VARIABLE)
        {
        if (NewNode(node, OPERATION, tokens->head->data) == Ok)
            {
            tokens->head = tokens->head->right;
            if (tokens->head->type == VARIABLE)
                {
                if (GetVariable(&(*node)->left, tokens) == Ok)
                    {
                    if (tokens->head->type == OPERATION && tokens->head->data.id == OP_ASSIGMENT)
                        {
                        tokens->head = tokens->head->right;
                        return GetExpression2(&(*node)->right, tokens);
                        }
                    else
                        {
                        Data_t zero = {.val = 0};
                        return NewNode(&(*node)->right, VALUE, zero);
                        }
                    }
                }
            }
        }

    printf("Variable define error\n");
    return SyntaxError;
    }

Error_t GetDefineArray(Node** node, List* tokens)
    {
    assert(node);
    assert(tokens);

    if (tokens->head->type == OPERATION && tokens->head->data.id == OP_DEFINE_ARRAY)
        {
        if (NewNode(node, OPERATION, tokens->head->data) == Ok)
            {
            tokens->head = tokens->head->right;
            if (tokens->head->type == ARRAY)
                {
                if (GetArray(&(*node)->left, tokens) == Ok)
                    {
                    if (tokens->head->type == OPERATION && tokens->head->data.id == OP_ASSIGMENT)
                        {
                        tokens->head = tokens->head->right;

                        if (tokens->head->type == PUNCTUATION && tokens->head->data.id == OPEN_SQUARE)
                            {
                            tokens->head = tokens->head->right;

                            if (GetParametr(&(*node)->right, tokens) == Ok)
                                {
                                if (tokens->head->type == PUNCTUATION && tokens->head->data.id == CLOSE_SQUARE)
                                    {
                                    tokens->head = tokens->head->right;
                                    return Ok;
                                    }
                                }
                            }
                        }
                    else
                        {
                        Data_t next = {.id = OP_NEXT_PARAMETR};
                        Data_t zero = {.val = 0};
                        if (NewNode(&(*node)->right, OPERATION, next) == Ok &&
                            NewNode(&(*node)->right->left, VALUE, zero) == Ok)
                            {
                            return Ok;
                            }
                        }
                    }
                }
            }
        }

    printf("Array define error\n");
    return SyntaxError;
    }

Error_t GetDefineFunction(Node** node, List* tokens)
    {
    assert(node);
    assert(tokens);

    if (tokens->head->type == OPERATION && tokens->head->data.id == OP_DEFINE_FUNCTION)
        {
        if (NewNode(node, OPERATION, tokens->head->data) == Ok)
            {
            tokens->head = tokens->head->right;
            if (tokens->head->type == FUNCTION)
                {
                if (NewNode(&(*node)->left, FUNCTION, tokens->head->data) == Ok)
                    {
                    tokens->head = tokens->head->right;
                    if (tokens->head->type == PUNCTUATION && tokens->head->data.id == OPEN_BRACKET)
                        {
                        tokens->head = tokens->head->right;

                        if (tokens->head->type == PUNCTUATION && tokens->head->data.id == CLOSE_BRACKET)
                            {
                            tokens->head = tokens->head->right;
                            return GetBody(&(*node)->right, tokens);
                            }

                        if (GetFunctionParametr(&(*node)->left->right, tokens) == Ok)
                            {
                            if (tokens->head->type == PUNCTUATION && tokens->head->data.id == CLOSE_BRACKET)
                                {
                                tokens->head = tokens->head->right;
                                return GetBody(&(*node)->right, tokens);
                                }
                            }
                        }
                    }
                }
            }
        }

    printf("Function define error\n");
    return SyntaxError;
    }

Error_t GetFunctionParametr(Node** node, List* tokens)
    {
    assert(node);
    assert(tokens);

    Data_t parametr = {.id = OP_NEXT_PARAMETR};
    if (NewNode(node, OPERATION, parametr) == Ok)
        {
        Error_t response = Ok;
        if (tokens->head->type == OPERATION && tokens->head->data.id == OP_DEFINE_VARIABLE)
            {
            response = GetDefineVariable(&(*node)->left, tokens);
            }
        else if (tokens->head->type == OPERATION && tokens->head->data.id == OP_DEFINE_ARRAY)
            {
            response = GetDefineArray(&(*node)->left, tokens);
            }
        else
            {
            printf("Wrong function parametr\n");
            response = SyntaxError;
            }

        if (response) return response;

        if (tokens->head->type == OPERATION && tokens->head->data.id == OP_NEXT_PARAMETR)
            {
            tokens->head = tokens->head->right;
            return GetFunctionParametr(&(*node)->right, tokens);
            }
        return Ok;
        }

    printf("Function parametr error\n");
    return SyntaxError;
    }

Error_t GetIf(Node** node, List* tokens)
    {
    assert(node);
    assert(tokens);

    if (tokens->head->type == OPERATION && tokens->head->data.id == OP_IF)
        {
        if (NewNode(node, OPERATION, tokens->head->data) == Ok)
            {
            tokens->head = tokens->head->right;
            if (GetExpression2(&(*node)->left, tokens) == Ok)
                {
                if (tokens->head->type == PUNCTUATION && tokens->head->data.id == COLON)
                    {
                    tokens->head = tokens->head->right;
                    if (GetBody(&(*node)->right, tokens) == Ok)
                        {
                        if (tokens->head->type == OPERATION && tokens->head->data.id == OP_ELSE)
                            {
                            Node* op_if = *node;
                            *node = nullptr;
                            if (GetElse(node, tokens) == Ok)
                                {
                                (*node)->left = op_if;
                                return Ok;
                                }
                            return SyntaxError;
                            }
                        return Ok;
                        }
                    }
                }
            }
        }

    printf("If error\n");
    return SyntaxError;
    }

Error_t GetElse(Node** node, List* tokens)
    {
    assert(node);
    assert(tokens);

    if (tokens->head->type == OPERATION && tokens->head->data.id == OP_ELSE)
        {
        if (NewNode(node, OPERATION, tokens->head->data) == Ok)
            {
            tokens->head = tokens->head->right;

            if (tokens->head->type == OPERATION && tokens->head->data.id == OP_IF)
                {
                return GetIf(&(*node)->right, tokens);
                }

            if (tokens->head->type == PUNCTUATION && tokens->head->data.id == COLON)
                {
                tokens->head = tokens->head->right;
                }
            return GetBody(&(*node)->right, tokens);
            }
        }

    printf("Else error\n");
    return SyntaxError;
    }

Error_t GetWhile(Node** node, List* tokens)
    {
    assert(node);
    assert(tokens);

    if (tokens->head->type == OPERATION && tokens->head->data.id == OP_WHILE)
        {
        if (NewNode(node, OPERATION, tokens->head->data) == Ok)
            {
            tokens->head = tokens->head->right;
            if (GetExpression2(&(*node)->left, tokens) == Ok)
                {
                if (tokens->head->type == PUNCTUATION && tokens->head->data.id == COLON)
                    {
                    tokens->head = tokens->head->right;
                    return GetBody(&(*node)->right, tokens);
                    }
                }
            }
        }

    printf("While error\n");
    return SyntaxError;
    }

Error_t GetAssigment(Node** node, List* tokens)
    {
    assert(node);
    assert(tokens);

    if (tokens->head->type == VARIABLE || tokens->head->type == ARRAY)
        {
        Node* var = nullptr;
        if (GetObject(&var, tokens) == Ok)
            {
            if (tokens->head->type == OPERATION)
                {
                switch (tokens->head->data.id)
                    {
                    case OP_ASSIGMENT:
                    case OP_ADD_ASSIGMENT:
                    case OP_SUB_ASSIGMENT:
                    case OP_MUL_ASSIGMENT:
                    case OP_DIV_ASSIGMENT:
                    case OP_POW_ASSIGMENT:
                        {
                        if (NewNode(node, OPERATION, tokens->head->data) == Ok)
                            {
                            tokens->head = tokens->head->right;
                            (*node)->left = var;
                            return GetExpression2(&(*node)->right, tokens);
                            }
                        printf("Assigment error\n");
                        return SyntaxError;
                        }
                    }
                }
            }
        free(var);
        while (tokens->head->left->data.id != OP_NEXT_COMMAND && tokens->head->left->data.id != NULL_TERMINATOR)
            {
            tokens->head = tokens->head->left;
            }
        }
    return NotAssigment;
    }

Error_t GetBody(Node** node, List* tokens)
    {
    assert(node);
    assert(tokens);

    if (tokens->head->type == PUNCTUATION && tokens->head->data.id == OPEN_BRACE)
        {
        tokens->head = tokens->head->right;
        if (GetOperation(node, tokens) == Ok)
            {
            if (tokens->head->type == PUNCTUATION && tokens->head->data.id == CLOSE_BRACE)
                {
                tokens->head = tokens->head->right;
                return Ok;
                }
            }
        }

    printf("Body error\n");
    return SyntaxError;
    }

Error_t GetExpression2(Node** node, List* tokens)
    {
    assert(node);
    assert(tokens);

    if (GetExpression1(node, tokens) == Ok)
        {
        if (tokens->head->type == OPERATION)
            {
            switch (tokens->head->data.id)
                {
                case OP_AND: case OP_OR:
                    {
                    Node* expression = *node;
                    *node = nullptr;
                    if (NewNode(node, OPERATION, tokens->head->data) == Ok)
                        {
                        (*node)->left = expression;

                        tokens->head = tokens->head->right;
                        return GetExpression2(&(*node)->right, tokens);
                        }
                    return SyntaxError;
                    }
                }
            }
        return Ok;
        }

    printf("Expression2 error\n");
    return SyntaxError;
    }

Error_t GetExpression1(Node** node, List* tokens)
    {
    assert(node);
    assert(tokens);

    if (GetExpression0(node, tokens) == Ok)
        {
        if (tokens->head->type == OPERATION)
            {
            switch (tokens->head->data.id)
                {
                case OP_EQUAL: case OP_NOT_EQUAL:
                case OP_GREATER: case OP_LESS:
                case OP_GREATER_EQUAL: case OP_LESS_EQUAL:
                    {
                    Node* expression = *node;
                    *node = nullptr;
                    if (NewNode(node, OPERATION, tokens->head->data) == Ok)
                        {
                        (*node)->left = expression;

                        tokens->head = tokens->head->right;
                        return GetExpression0(&(*node)->right, tokens);
                        }
                    return SyntaxError;
                    }
                }
            }
        return Ok;
        }

    printf("Expression1 error\n");
    return SyntaxError;
    }

Error_t GetExpression0(Node** node, List* tokens)
    {
    assert(node);
    assert(tokens);

    if (GetTerm(node, tokens) == Ok)
        {
        if (tokens->head->type == OPERATION)
            {
            switch (tokens->head->data.id)
                {
                case OP_ADD: case OP_SUB:
                    {
                    Node* term = *node;
                    *node = nullptr;
                    if (NewNode(node, OPERATION, tokens->head->data) == Ok)
                        {
                        (*node)->left = term;

                        tokens->head = tokens->head->right;
                        return GetExpression0(&(*node)->right, tokens);
                        }
                    return SyntaxError;
                    }
                }
            }
        return Ok;
        }

    printf("Expression0 error\n");
    return SyntaxError;
    }

Error_t GetTerm(Node** node, List* tokens)
    {
    assert(node);
    assert(tokens);

    if (GetUnary(node, tokens) == Ok)
        {
        if (tokens->head->type == OPERATION)
            {
            switch (tokens->head->data.id)
                {
                case OP_MUL: case OP_DIV: case OP_POW:
                    {
                    Node* unary = *node;
                    *node = nullptr;
                    if (NewNode(node, OPERATION, tokens->head->data) == Ok)
                        {
                        (*node)->left = unary;

                        tokens->head = tokens->head->right;
                        return GetTerm(&(*node)->right, tokens);
                        }
                    return SyntaxError;
                    }
                }
            }
        return Ok;
        }

    printf("Term error\n");
    return SyntaxError;
    }

Error_t GetUnary(Node** node, List* tokens)
    {
    assert(node);
    assert(tokens);

    if (tokens->head->type == OPERATION)
        {
        switch (tokens->head->data.id)
            {
            case OP_INPUT: case OP_OUTPUT:
            case OP_SIN: case OP_COS: case OP_SQRT:
            case OP_LOG: case OP_EXP: case OP_NOT:
            case OP_INCREMENT: case OP_DECREMENT:
                {
                if (NewNode(node, OPERATION, tokens->head->data) == Ok)
                    {
                    tokens->head = tokens->head->right;
                    if (GetPriority(&(*node)->right, tokens) != Ok)
                        {
                        printf("Unary error\n");
                        return SyntaxError;
                        }
                    }
                break;
                }
            default:
                printf("Unary error\n");
                return SyntaxError;
            }
        }
    else if (GetPriority(node, tokens) != Ok)
        {
        printf("Unary error\n");
        return SyntaxError;
        }

    if  (tokens->head->type == OPERATION && tokens->head->data.id == OP_NOT)
        {
        Node* priority = *node;
        *node = nullptr;
        if (NewNode(node, OPERATION, tokens->head->data) == Ok)
            {
            (*node)->right = priority;

            tokens->head = tokens->head->right;
            return Ok;
            }
        return SyntaxError;
        }

    return Ok;
    }

Error_t GetPriority(Node** node, List* tokens)
    {
    assert(node);
    assert(tokens);

    if (tokens->head->type == PUNCTUATION && tokens->head->data.id == OPEN_BRACKET)
        {
        tokens->head = tokens->head->right;
        if (GetExpression2(node, tokens) == Ok)
            {
            if (tokens->head->type == PUNCTUATION && tokens->head->data.id == CLOSE_BRACKET)
                {
                tokens->head = tokens->head->right;
                return Ok;
                }
            }
        printf("Priority error\n");
        return SyntaxError;
        }

    return GetObject(node, tokens);
    }

Error_t GetObject(Node** node, List* tokens)
    {
    assert(node);
    assert(tokens);

    switch (tokens->head->type)
        {
        case VALUE:    return GetNumber(node, tokens);
        case VARIABLE: return GetVariable(node, tokens);
        case FUNCTION: return GetFunction(node, tokens);
        case ARRAY:    return GetArray(node, tokens);
        }

    printf("Object error (get not object) get %d, %d\n", tokens->head->type, tokens->head->data.id);
    return SyntaxError;
    }

Error_t GetParametr(Node** node, List* tokens)
    {
    assert(node);
    assert(tokens);

    Data_t parametr = {.id = OP_NEXT_PARAMETR};
    if (NewNode(node, OPERATION, parametr) == Ok)
        {
        if (GetExpression2(&(*node)->left, tokens) == Ok)
            {
            if (tokens->head->type == OPERATION && tokens->head->data.id == OP_NEXT_PARAMETR)
                {
                tokens->head = tokens->head->right;
                return GetParametr(&(*node)->right, tokens);
                }

            return Ok;
            }
        }

    printf("Parametr error\n");
    return SyntaxError;
    }

Error_t GetFunction(Node** node, List* tokens)
    {
    assert(node);
    assert(tokens);

    if (NewNode(node, FUNCTION, tokens->head->data) == Ok)
        {
        tokens->head = tokens->head->right;

        if (tokens->head->type == PUNCTUATION && tokens->head->data.id == OPEN_BRACKET)
            {
            tokens->head = tokens->head->right;

            if (tokens->head->type == PUNCTUATION && tokens->head->data.id == CLOSE_BRACKET)
                {
                tokens->head = tokens->head->right;
                return Ok;
                }

            if (GetParametr(&(*node)->right, tokens) == Ok)
                {
                if (tokens->head->type == PUNCTUATION && tokens->head->data.id == CLOSE_BRACKET)
                    {
                    tokens->head = tokens->head->right;
                    return Ok;
                    }
                }
            }
        }

    printf("Function error\n");
    return SyntaxError;
    }

Error_t GetArray(Node** node, List* tokens)
    {
    assert(node);
    assert(tokens);

    if (NewNode(node, ARRAY, tokens->head->data) == Ok)
        {
        tokens->head = tokens->head->right;

        if (tokens->head->type == PUNCTUATION && tokens->head->data.id == OPEN_SQUARE)
            {
            tokens->head = tokens->head->right;

            if (GetExpression0(&(*node)->right, tokens) == Ok)
                {
                if (tokens->head->type == PUNCTUATION && tokens->head->data.id == CLOSE_SQUARE)
                    {
                    tokens->head = tokens->head->right;
                    return Ok;
                    }
                }
            }
        }

    printf("Array error\n");
    return SyntaxError;
    }

Error_t GetVariable(Node** node, List* tokens)
    {
    assert(node);
    assert(tokens);

    if (NewNode(node, VARIABLE, tokens->head->data) == Ok)
        {
        tokens->head = tokens->head->right;
        return Ok;
        }

    printf("Variable error\n");
    return SyntaxError;
    }

Error_t GetNumber(Node** node, List* tokens)
    {
    assert(node);
    assert(tokens);

    if (NewNode(node, VALUE, tokens->head->data) == Ok)
        {
        tokens->head = tokens->head->right;
        return Ok;
        }

    printf("Number error\n");
    return SyntaxError;
    }

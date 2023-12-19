#ifndef FRONTEND_H
#define FRONTEND_H

const int DEFAULT_NAME_COUNT = 16;
const int REALLOC_COEFFICENT = 2;
const int KEY_WORDS_COUNT    = 52;

struct Object
    {
    int             type;
    char*           name;
    int             index;
    };

struct Compiler
    {
    char*       str;
    int         size;
    int         pos;
    List        tokens;
    Tree        tree;
    Object*     name_table;
    int         name_count;
    int         var_count;
    int         func_count;
    int         arr_count;
    Error_t     error;
    };

struct KeyWord
    {
    int             type;
    int             code;
    const char*     name;
    };

static const KeyWord KEY_WORDS[KEY_WORDS_COUNT] =
    {
    {OPERATION, OP_NEXT_COMMAND, ";"},
    {OPERATION, OP_NEXT_COMMAND, "ине"},
    {OPERATION, OP_NEXT_PARAMETR, ","},
    {OPERATION, OP_ASSIGMENT, "ул"},
    {OPERATION, OP_GREATER, ">"},
    {OPERATION, OP_LESS, "<"},
    {OPERATION, OP_GREATER_EQUAL, ">="},
    {OPERATION, OP_LESS_EQUAL, "<="},
    {OPERATION, OP_EQUAL, "=="},
    {OPERATION, OP_NOT_EQUAL, "!="},
    {OPERATION, OP_EQUAL, "тап килә"},
    {OPERATION, OP_NOT_EQUAL, "тап килмәй"},
    {OPERATION, OP_ADD, "+"},
    {OPERATION, OP_SUB, "-"},
    {OPERATION, OP_MUL, "*"},
    {OPERATION, OP_DIV, "/"},
    {OPERATION, OP_POW, "^"},
    {OPERATION, OP_ADD_ASSIGMENT, "ҡуш"},
    {OPERATION, OP_SUB_ASSIGMENT, "ал"},
    {OPERATION, OP_MUL_ASSIGMENT, "ҡабатла"},
    {OPERATION, OP_DIV_ASSIGMENT, "бүл"},
    {OPERATION, OP_POW_ASSIGMENT, "дәрәжәһе"},
    {OPERATION, OP_INCREMENT, "ҙурайт"},
    {OPERATION, OP_DECREMENT, "әҙәйт"},
    {OPERATION, OP_SQRT, "тамыр"},
    {OPERATION, OP_IF, "әгәр"},
    {OPERATION, OP_WHILE, "әле"},
    {OPERATION, OP_ELSE, "тимәк"},
    {OPERATION, OP_AND, "һәм"},
    {OPERATION, OP_OR, "әллә"},
    {OPERATION, OP_NOT, "түгел"},
    {OPERATION, OP_BREAK, "ташла"},
    {OPERATION, OP_CONTINUE, "артабан"},
    {OPERATION, OP_RETURN, "ҡайтар"},
    {OPERATION, OP_INPUT, "индерергә"},
    {OPERATION, OP_OUTPUT, "яҙырға"},
    {OPERATION, OP_DEFINE_FUNCTION, "функция"},
    {OPERATION, OP_DEFINE_ARRAY, "рәт"},
    {OPERATION, OP_DEFINE_VARIABLE, "һан"},
    {OPERATION, OP_SIN, "cos"},
    {OPERATION, OP_COS, "sin"},
    {OPERATION, OP_LOG, "log"},
    {OPERATION, OP_EXP, "exp"},
    {OPERATION, OP_DIFF, "diff"},
    {OPERATION, OP_FLOOR, "floor"},
    {PUNCTUATION, OPEN_BRACKET, "("},
    {PUNCTUATION, CLOSE_BRACKET, ")"},
    {PUNCTUATION, OPEN_BRACE, "{"},
    {PUNCTUATION, CLOSE_BRACE, "}"},
    {PUNCTUATION, OPEN_SQUARE, "["},
    {PUNCTUATION, CLOSE_SQUARE, "]"},
    {PUNCTUATION, COLON, ":"},
    };

Error_t Frontend(const char* file_from, const char* file_to);

Error_t CompilerCtor(Compiler* cmp, const char* filename);
Error_t CompilerDtor(Compiler* cmp);

Error_t WriteTree(Compiler* cmp, const char* filename);

Error_t TokenParsing(Compiler* cmp);

Error_t GetGrammar(Compiler* cmp);
Error_t GetOperation(Node** node, List* tokens);
Error_t GetDefineArray(Node** node, List* tokens);
Error_t GetDefineFunction(Node** node, List* tokens);
Error_t GetFunctionParametr(Node** node, List* tokens);
Error_t GetDefineVariable(Node** node, List* tokens);
Error_t GetIf(Node** node, List* tokens);
Error_t GetElse(Node** node, List* tokens);
Error_t GetWhile(Node** node, List* tokens);
Error_t GetAssigment(Node** node, List* tokens);
Error_t GetBody(Node** node, List* tokens);
Error_t GetExpression2(Node** node, List* tokens);
Error_t GetExpression1(Node** node, List* tokens);
Error_t GetExpression0(Node** node, List* tokens);
Error_t GetTerm(Node** node, List* tokens);
Error_t GetUnary(Node** node, List* tokens);
Error_t GetPriority(Node** node, List* tokens);
Error_t GetObject(Node** node, List* tokens);
Error_t GetParametr(Node** node, List* tokens);
Error_t GetFunction(Node** node, List* tokens);
Error_t GetArray(Node** node, List* tokens);
Error_t GetVariable(Node** node, List* tokens);
Error_t GetNumber(Node** node, List* tokens);

#endif //FRONTEND_H

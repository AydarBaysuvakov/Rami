#ifndef FRONTEND_H
#define FRONTEND_H

struct Compiler
    {
    char*       str;
    int         size;
    int         pos;
    List        tokens;
    Tree        tree;
    char**      name_table;
    int         error;
    };

static const char* const KEY_WORDS[] =
    {
    ";", "ул", ">", "<", ">=", "<=", "==", "!=",
    "тап килә", "тап килмәй", "ҡуш", "ал", "ҡабатла", "бүл", "дәрәжәһе",
    "+", "-", "*", "/", "^", "++", "--",
    "әгәр", "әле", "тимәк", "һәм", "әллә", "түгел", "ташла", "артабан", "ҡайтар",
    "булмаһаң", "донъяға тыумаһаң", "ине", "үкенер", "үлмәҫ", "һүнмәҫ", "һүнер"
    };

Error_t Frontend(const char* s);

Error_t CompilerCtor(Compiler* cmp, const char* filename);
Error_t CompilerDtor(Compiler* cmp);

Error_t TokenParsing(Compiler* cmp);

int GetGrammar(const char*  s);
int GetEquation(Compiler* cmp);
int GetTerm(Compiler* front);
int GetNumber(Compiler* cmp);
int GetPriority(Compiler* cmp);

#endif //FRONTEND_H

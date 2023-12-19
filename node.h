#ifndef NODE_H
#define NODE_H

typedef int Type_t;

enum Type
    {
    OPERATION    = 1,
    VALUE        = 2,
    VARIABLE     = 3,
    FUNCTION     = 4,
    ARRAY        = 5,
    PUNCTUATION  = 6
    };

enum Punctuation
    {
    NULL_TERMINATOR  = 0,
    OPEN_BRACKET     = 1,
    CLOSE_BRACKET    = 2,
    OPEN_BRACE       = 3,
    CLOSE_BRACE      = 4,
    OPEN_SQUARE      = 5,
    CLOSE_SQUARE     = 6,
    COLON            = 7,
    PROGRAM_START    = 8
    };

enum Operation
    {
    OP_NEXT_COMMAND     = 1,
    OP_NEXT_PARAMETR    = 2,
    OP_ASSIGMENT        = 51,
    OP_GREATER          = 52,
    OP_LESS             = 53,
    OP_GREATER_EQUAL    = 54,
    OP_LESS_EQUAL       = 55,
    OP_EQUAL            = 56,
    OP_NOT_EQUAL        = 57,
    OP_ADD              = 101,
    OP_SUB              = 102,
    OP_MUL              = 103,
    OP_DIV              = 104,
    OP_POW              = 105,
    OP_ADD_ASSIGMENT    = 106,
    OP_SUB_ASSIGMENT    = 107,
    OP_MUL_ASSIGMENT    = 108,
    OP_DIV_ASSIGMENT    = 109,
    OP_POW_ASSIGMENT    = 110,
    OP_INCREMENT        = 111,
    OP_DECREMENT        = 112,
    OP_IF               = 201,
    OP_WHILE            = 202,
    OP_AND              = 203,
    OP_OR               = 204,
    OP_NOT              = 205,
    OP_ELSE             = 206,
    OP_SIN              = 301, // Я не буду пока реализовывать математические функции
    OP_COS              = 302, // Они занимают лишнее место в наборе команд
    OP_FLOOR            = 303, // Если я в будующем расширю диапозон для команд и их будет больше 32
    OP_DIFF             = 304, // тогда добавлю это операции (пока будет добавлен только корень)
    OP_SQRT             = 305,
    OP_LOG              = 306,
    OP_EXP              = 307,
    OP_INPUT            = 351,
    OP_OUTPUT           = 352,
    OP_BREAK            = 401,
    OP_CONTINUE         = 402,
    OP_RETURN           = 403,
    OP_DEFINE_FUNCTION  = 404,
    OP_DEFINE_ARRAY     = 405,
    OP_DEFINE_VARIABLE  = 406,
    NO_OPER             = 0
    };

union Data_t
    {
    double   val;
    int      id;
    };

struct Node
    {
    Data_t data;
    Type_t type;
    Node*  left;
    Node*  right;
    };

Error_t NewNode(Node** node, const int type, const Data_t data);
Error_t EditNode(Node* node, const int type, const Data_t data);
Error_t DeleteNode(Node* node);

#endif //NODE_H

#ifndef NODE_H
#define NODE_H

typedef int Type_t;

enum Type
    {
    VALUE        = 1,
    OPERATION    = 2,
    VARIABLE     = 3,
    FUNCTION     = 4,
    PUNCTUATION  = 5
    };

enum Punctuation
    {
    NULL_TERMINATOR  = 0,
    OPEN_BRACKET     = 1,
    CLOSE_BRACKET    = 2,
    OPEN_BRACE       = 3,
    CLOSE_BRACE      = 4,
    COLON            = 5
    };

enum Operation
    {
    OP_NEXT             = 1,
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
    OP_IF               = 201,
    OP_WHILE            = 202,
    OP_AND              = 203,
    OP_OR               = 204,
    OP_NOT              = 205,
    OP_ELSE             = 206,
    OP_SIN              = 301,
    OP_COS              = 302,
    OP_FLOOR            = 303,
    OP_DIFF             = 304,
    OP_SQRT             = 305,
    OP_LOG              = 306,
    OP_EXP              = 307,
    OP_BREAK            = 401,
    OP_CONTINUE         = 402,
    OP_RETURN           = 403,
    NO_OPER             = 0
    };

union Data_t
    {
    double   val;
    int      oper;
    int      var;
    int      func;
    int      punc;
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

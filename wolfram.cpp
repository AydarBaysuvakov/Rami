#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "errors.h"
#include "node.h"
#include "tree.h"
#include "stack.h"
#include "wolfram.h"

static bool IsEqual(double a, double b);
static bool FindAddSub(Node* node);
static bool FindVariable(Node* node);

double Eval(const Node* node, double x)
    {
    if (!node)
        {
        return NAN;
        }
    if (node->type == VALUE)
        {
        return node->data.val;
        }
    if (node->type == VARIABLE)
        {
        return x;
        }

    double left  = Eval(node->left,  x);
    double right = Eval(node->right, x);

    switch (node->data.oper)
        {
        case OP_ADD:  return left + right;
        case OP_SUB:  return left - right;
        case OP_MUL:  return left * right;
        case OP_DIV:  return left / right;
        case OP_POW:  return pow(left, right);
        case OP_SIN:  return sin(right);
        case OP_COS:  return cos(right);
        case OP_LOG:  return log(right);
        case OP_EXP:  return exp(right);
        case OP_SQRT: return sqrt(right);
        case NO_OPER: return 0;
        default: printf("Error: unknown operation");
        }
    return 0;
    }

Error_t Differentiation(Node** dest, const Node* src)
    {
    assert(dest != NULL);
    assert(src  != NULL);

    if (*dest) DeleteNode(*dest);
    *dest = nullptr;

    Data_t add = {.oper = OP_ADD};
    Data_t sub = {.oper = OP_SUB};
    Data_t mul = {.oper = OP_MUL};
    Data_t div = {.oper = OP_DIV};
    Data_t pow = {.oper = OP_POW};
    Data_t sin = {.oper = OP_SIN};
    Data_t cos = {.oper = OP_COS};
    Data_t log = {.oper = OP_LOG};
    Data_t exp = {.oper = OP_EXP};
    Data_t sqrt = {.oper = OP_SQRT};

    if (src->type == VALUE)
        {
        Data_t data = {.val = 0};
        if (NewNode(dest, VALUE, data) == AllocationError)
            {
            printf("Внимание: в связи с ошибкой дифференцирование дерева не завершилось.\n");
            return DifferentiationError;
            }
        return Ok;
        }
    else if (src->type == VARIABLE)
        {
        Data_t data = {.val = 1};
        if (NewNode(dest, VALUE, data) == AllocationError)
            {
            printf("Внимание: в связи с ошибкой дифференцирование дерева не завершилось.\n");
            return DifferentiationError;
            }
        return Ok;
        }

    switch (src->data.oper)
        {
        case OP_ADD: case OP_SUB:
            {
            if (NewNode(dest, OPERATION, src->data) == AllocationError)
                {
                printf("Внимание: в связи с ошибкой дифференцирование дерева не завершилось.\n");
                return DifferentiationError;
                }

            if (Differentiation(&(*dest)->left,  src->left)  == DifferentiationError ||
                Differentiation(&(*dest)->right, src->right) == DifferentiationError)
                {
                return DifferentiationError;
                }

            break;
            }
        case OP_MUL:
            {
            if (NewNode(dest,            OPERATION, add) == AllocationError ||
                NewNode(&(*dest)->left,  OPERATION, mul) == AllocationError ||
                NewNode(&(*dest)->right, OPERATION, mul) == AllocationError)
                {
                printf("Внимание: в связи с ошибкой дифференцирование дерева не завершилось.\n");
                return DifferentiationError;
                }

            if (Differentiation(&(*dest)->left->left,   src->left)  == DifferentiationError ||
                Differentiation(&(*dest)->right->right, src->right) == DifferentiationError)
                {
                return DifferentiationError;
                }

            if (CopyTree(&(*dest)->left->right, src->right) == CopyError ||
                CopyTree(&(*dest)->right->left, src->left)  == CopyError)
                {
                return DifferentiationError;
                }

            break;
            }
        case OP_DIV:
            {
            if (NewNode(dest,                   OPERATION, div) == AllocationError ||
                NewNode(&(*dest)->left,         OPERATION, sub) == AllocationError ||
                NewNode(&(*dest)->left->left,   OPERATION, mul) == AllocationError ||
                NewNode(&(*dest)->left->right,  OPERATION, mul) == AllocationError ||
                NewNode(&(*dest)->right,        OPERATION, mul) == AllocationError)
                {
                printf("Внимание: в связи с ошибкой дифференцирование дерева не завершилось.\n");
                return DifferentiationError;
                }

            if (Differentiation(&(*dest)->left->left->left,   src->left)  == DifferentiationError ||
                Differentiation(&(*dest)->left->right->right, src->right) == DifferentiationError)
                {
                return DifferentiationError;
                }

            if (CopyTree(&(*dest)->left->left->right, src->right) == CopyError ||
                CopyTree(&(*dest)->left->right->left, src->left)  == CopyError ||
                CopyTree(&(*dest)->right->left,       src->right) == CopyError ||
                CopyTree(&(*dest)->right->right,      src->right) == CopyError)
                {
                return DifferentiationError;
                }

            break;
            }
        case OP_POW:
            {
            bool left_is_func  = FindVariable(src->left);
            bool right_is_func = FindVariable(src->right);
            if (left_is_func && right_is_func)
                {
                if (NewNode(dest,                           OPERATION, mul) == AllocationError ||
                    NewNode(&(*dest)->left,                 OPERATION, pow) == AllocationError ||
                    NewNode(&(*dest)->right,                OPERATION, add) == AllocationError ||
                    NewNode(&(*dest)->right->left,          OPERATION, mul) == AllocationError ||
                    NewNode(&(*dest)->right->right,         OPERATION, mul) == AllocationError ||
                    NewNode(&(*dest)->right->left->left,    OPERATION, div) == AllocationError ||
                    NewNode(&(*dest)->right->right->right,  OPERATION, log) == AllocationError)
                    {
                    printf("Внимание: в связи с ошибкой дифференцирование дерева не завершилось.\n");
                    return DifferentiationError;
                    }

                if (Differentiation(&(*dest)->right->left->left->left,  src->left)  == DifferentiationError ||
                    Differentiation(&(*dest)->right->right->left,       src->right) == DifferentiationError)
                    {
                    return DifferentiationError;
                    }

                if (CopyTree(&(*dest)->left->left,                  src->left)  == CopyError ||
                    CopyTree(&(*dest)->right->left->left->right,    src->left)  == CopyError ||
                    CopyTree(&(*dest)->right->right->right->right,  src->left)  == CopyError ||
                    CopyTree(&(*dest)->left->right,                 src->right) == CopyError ||
                    CopyTree(&(*dest)->right->left->right,          src->right) == CopyError)
                    {
                    return DifferentiationError;
                    }

                break;
                }
            if (left_is_func && !right_is_func)
                {
                if (NewNode(dest,                   OPERATION, mul) == AllocationError ||
                    NewNode(&(*dest)->right,        OPERATION, mul) == AllocationError ||
                    NewNode(&(*dest)->right->left,  OPERATION, pow) == AllocationError)
                    {
                    printf("Внимание: в связи с ошибкой дифференцирование дерева не завершилось.\n");
                    return DifferentiationError;
                    }

                if (Differentiation(&(*dest)->right->right,     src->left) == DifferentiationError ||
                    CopyTree(&(*dest)->right->left->left,       src->left) == CopyError)
                    {
                    return DifferentiationError;
                    }

                Data_t data = {.val = Eval(src->right, 0)};
                if (NewNode(&(*dest)->left, VALUE, data) == AllocationError)
                    {
                    printf("Внимание: в связи с ошибкой дифференцирование дерева не завершилось.\n");
                    return DifferentiationError;
                    }

                data.val -= 1;
                if (NewNode(&(*dest)->right->left->right, VALUE, data) == AllocationError)
                    {
                    printf("Внимание: в связи с ошибкой дифференцирование дерева не завершилось.\n");
                    return DifferentiationError;
                    }

                break;
                }
            if (!left_is_func && right_is_func)
                {
                if (NewNode(dest,                   OPERATION, mul) == AllocationError ||
                    NewNode(&(*dest)->left,         OPERATION, pow) == AllocationError ||
                    NewNode(&(*dest)->right,        OPERATION, mul) == AllocationError ||
                    NewNode(&(*dest)->right->left,  OPERATION, log) == AllocationError)
                    {
                    printf("Внимание: в связи с ошибкой дифференцирование дерева не завершилось.\n");
                    return DifferentiationError;
                    }

                if (Differentiation(&(*dest)->right->right,  src->right) == DifferentiationError ||
                    CopyTree(&(*dest)->left->right,          src->right) == CopyError)
                    {
                    return DifferentiationError;
                    }

                Data_t data = {.val = Eval(src->left, 0)};
                if (NewNode(&(*dest)->left->left, VALUE, data) == AllocationError)
                    {
                    printf("Внимание: в связи с ошибкой дифференцирование дерева не завершилось.\n");
                    return DifferentiationError;
                    }

                if (NewNode(&(*dest)->right->left->right, VALUE, data) == AllocationError)
                    {
                    printf("Внимание: в связи с ошибкой дифференцирование дерева не завершилось.\n");
                    return DifferentiationError;
                    }

                break;
                }
            else
                {
                Data_t data = {.val = 0};
                if (NewNode(dest, VALUE, data) == AllocationError)
                    {
                    printf("Внимание: в связи с ошибкой дифференцирование дерева не завершилось.\n");
                    return DifferentiationError;
                    }
                }

            break;
            }
        case OP_SIN:
            {
            if (NewNode(dest,           OPERATION, mul) == AllocationError ||
                NewNode(&(*dest)->left, OPERATION, cos) == AllocationError)
                {
                printf("Внимание: в связи с ошибкой дифференцирование дерева не завершилось.\n");
                return DifferentiationError;
                }

            if (Differentiation(&(*dest)->right,  src->right) == DifferentiationError ||
                CopyTree(&(*dest)->left->right,   src->right) == CopyError)
                {
                return DifferentiationError;
                }

            break;
            }
        case OP_COS:
            {
            if (NewNode(dest,                   OPERATION, mul) == AllocationError ||
                NewNode(&(*dest)->left,         OPERATION, mul) == AllocationError ||
                NewNode(&(*dest)->left->right,  OPERATION, sin) == AllocationError)
                {
                printf("Внимание: в связи с ошибкой дифференцирование дерева не завершилось.\n");
                return DifferentiationError;
                }

            if (Differentiation(&(*dest)->right,        src->right) == DifferentiationError ||
                CopyTree(&(*dest)->left->right->right,  src->right) == CopyError)
                {
                return DifferentiationError;
                }

            Data_t data = {.val = -1};
            if (NewNode(&(*dest)->left->left, VALUE, data) == AllocationError)
                {
                printf("Внимание: в связи с ошибкой дифференцирование дерева не завершилось.\n");
                return DifferentiationError;
                }

            break;
            }
        case OP_LOG:
            {
            if (NewNode(dest, OPERATION, div) == AllocationError)
                {
                printf("Внимание: в связи с ошибкой дифференцирование дерева не завершилось.\n");
                return DifferentiationError;
                }

            if (Differentiation(&(*dest)->left, src->right) == DifferentiationError ||
                CopyTree(&(*dest)->right,       src->right) == CopyError)
                {
                return DifferentiationError;
                }

            break;
            }
        case OP_EXP:
            {
            if (NewNode(dest,           OPERATION, div) == AllocationError ||
                NewNode(&(*dest)->left, OPERATION, exp) == AllocationError)
                {
                printf("Внимание: в связи с ошибкой дифференцирование дерева не завершилось.\n");
                return DifferentiationError;
                }

            if (Differentiation(&(*dest)->right,  src->right) == DifferentiationError ||
                CopyTree(&(*dest)->left->right,   src->right) == CopyError)
                {
                return DifferentiationError;
                }

            break;
            }
        case OP_SQRT:
            {
            if (NewNode(dest,                    OPERATION, div)  == AllocationError ||
                NewNode(&(*dest)->right,         OPERATION, mul)  == AllocationError ||
                NewNode(&(*dest)->right->right,  OPERATION, sqrt) == AllocationError)
                {
                printf("Внимание: в связи с ошибкой дифференцирование дерева не завершилось.\n");
                return DifferentiationError;
                }

            if (Differentiation(&(*dest)->left,         src->right) == DifferentiationError ||
                CopyTree(&(*dest)->right->right->right, src->right) == CopyError)
                {
                return DifferentiationError;
                }

            Data_t data = {.val = 2};
            if (NewNode(&(*dest)->right->left, VALUE, data) == AllocationError)
                {
                printf("Внимание: в связи с ошибкой дифференцирование дерева не завершилось.\n");
                return DifferentiationError;
                }

            break;
            }
        }

    return Ok;
    }

bool Simplifier(Node** node)
    {
    assert(node  != NULL);
    if (!*node) return false;

    if ((*node)->type == VALUE || (*node)->type == VARIABLE) return false;

    bool changed = false;

    if (!FindVariable(*node))
        {
        Data_t data = {.val = Eval(*node, 0)};

        DeleteNode((*node)->left);
        (*node)->left = nullptr;
        DeleteNode((*node)->right);
        (*node)->right = nullptr;

        EditNode(*node, VALUE, data);
        return true;
        }

    while (Simplifier(&(*node)->left))  changed = true;
    while (Simplifier(&(*node)->right)) changed = true;

    // 1 * x || 0 + x
    if ( (*node)->left                && (*node)->left->type == VALUE &&
        ((*node)->data.oper == OP_MUL && (*node)->left->data.val == 1 ||
         (*node)->data.oper == OP_ADD && (*node)->left->data.val == 0))
        {
        DeleteNode((*node)->left);
        (*node)->left = nullptr;

        Node* old_node = *node;
        *node = (*node)->right;
        free(old_node);

        return true;
        }
    // x * 1 || x + 0 || x ^ 1 || x - 0 || x / 1
    if ( (*node)->right               && (*node)->right->type == VALUE &&
        ((*node)->data.oper == OP_MUL && (*node)->right->data.val == 1 ||
         (*node)->data.oper == OP_ADD && (*node)->right->data.val == 0 ||
         (*node)->data.oper == OP_POW && (*node)->right->data.val == 1 ||
         (*node)->data.oper == OP_SUB && (*node)->right->data.val == 0 ||
         (*node)->data.oper == OP_DIV && (*node)->right->data.val == 1))
        {
        DeleteNode((*node)->right);
        (*node)->right = nullptr;

        Node* old_node = *node;
        *node = (*node)->left;
        free(old_node);

        return true;
        }
    // 0 * x || x * 0 || 0 ^ x
    if ( (*node)->data.oper == OP_MUL && (*node)->left->type  == VALUE && (*node)->left->data.val  == 0 ||
         (*node)->data.oper == OP_MUL && (*node)->right->type == VALUE && (*node)->right->data.val == 0 ||
         (*node)->data.oper == OP_POW && (*node)->left->type  == VALUE && (*node)->left->data.val == 0)
        {
        DeleteNode((*node)->left);
        (*node)->left = nullptr;

        DeleteNode((*node)->right);
        (*node)->right = nullptr;

        Data_t data = {.val = 0};
        EditNode(*node, VALUE, data);

        return true;
        }
    // x ^ 0 || 1 ^ x
    if (  (*node)->data.oper == OP_POW &&
        ((*node)->right->type == VALUE && (*node)->right->data.val == 0 ||
         (*node)->left->type  == VALUE &&  (*node)->left->data.val == 1))
        {
        DeleteNode((*node)->left);
        (*node)->left = nullptr;

        DeleteNode((*node)->right);
        (*node)->right = nullptr;

        Data_t data = {.val = 1};
        EditNode(*node, VALUE, data);

        return true;
        }
    // x + (-y) || x - (-y)
    if (((*node)->data.oper == OP_ADD  || (*node)->data.oper == OP_SUB) &&
         (*node)->right->type == VALUE && (*node)->right->data.val < 0)
        {
        Data_t data = {0};
        data.oper = ((*node)->data.oper == OP_ADD) ? (OP_SUB) : (OP_ADD);
        EditNode(*node, OPERATION, data);
        data.val = -1 * (*node)->right->data.val;
        EditNode((*node)->right, VALUE, data);
        return true;
        }

    return changed;
    }

static bool FindAddSub(Node* node)
    {
    if (!node) return false;
    if (node->type == OPERATION && (node->data.oper == OP_ADD || node->data.oper == OP_SUB))
        {
        return true;
        }
    return FindAddSub(node->left) || FindAddSub(node->right);
    }

static bool IsEqual(double a, double b)
    {
    assert(isfinite(a));
    assert(isfinite(b));

    return abs(a - b) <= MEASURE_ERROR;
    }

static bool FindVariable(Node* node)
    {
    if (!node) return false;
    if (node->type == VARIABLE) return true;
    return FindVariable(node->left) || FindVariable(node->right);
    }

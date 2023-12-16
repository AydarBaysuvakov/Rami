#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "errors.h"
#include "node.h"

Error_t NewNode(Node** node, const int type, const Data_t data)
    {
    assert(node != NULL);

    if (*node)
        {
        printf("Error: node already exist\n");
        return NodeExist;
        }

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
        }

    new_node->left   = nullptr;
    new_node->right  = nullptr;

    *node = new_node;

    return Ok;
    }


Error_t DeleteNode(Node* node)
    {
    assert(node != NULL);

    if (node->left)  DeleteNode(node->left);
    if (node->right) DeleteNode(node->right);

    node->data.val = 0;
    free(node);

    return Ok;
    }

Error_t EditNode(Node* node, const int type, const Data_t data)
    {
    if (!node)
        {
        printf("Error: node not exist");
        return NodeNotExist;
        }

    node->type = type;

    switch (type)
        {
        case VALUE:
            node->data.val  = data.val;
            break;
        case VARIABLE:
            node->data.var  = data.var;
            break;
        case OPERATION:
            node->data.oper = data.oper;
            break;
        }

    return Ok;
    }

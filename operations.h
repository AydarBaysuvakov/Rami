#define WriteBothNodes()    WriteEquation(node->left, fp); \
                            WriteEquation(node->right, fp);

DEFINE_OPERATION (OP_GREATER,       {
                                    WriteBothNodes()
                                    fprintf(fp, "gt\n");
                                    })

DEFINE_OPERATION (OP_LESS,          {
                                    WriteBothNodes()
                                    fprintf(fp, "lt\n");
                                    })

DEFINE_OPERATION (OP_GREATER_EQUAL, {
                                    WriteBothNodes()
                                    fprintf(fp, "ge\n");
                                    })

DEFINE_OPERATION (OP_LESS_EQUAL,    {
                                    WriteBothNodes()
                                    fprintf(fp, "le\n");
                                    })

DEFINE_OPERATION (OP_EQUAL,         {
                                    WriteBothNodes()
                                    fprintf(fp, "eq\n");
                                    })

DEFINE_OPERATION (OP_NOT_EQUAL,     {
                                    WriteBothNodes()
                                    fprintf(fp, "ne\n");
                                    })

DEFINE_OPERATION (OP_ADD,           {
                                    WriteBothNodes()
                                    fprintf(fp, "add\n");
                                    })

DEFINE_OPERATION (OP_SUB,           {
                                    WriteBothNodes()
                                    fprintf(fp, "sub\n");
                                    })

DEFINE_OPERATION (OP_INCREMENT,     {
                                    fprintf(fp, "push [%d]\n", node->right->data.id);
                                    fprintf(fp, "push 1\n");
                                    fprintf(fp, "add\n");
                                    fprintf(fp, "pop [%d]\n", node->right->data.id);
                                    fprintf(fp, "push [%d]\n", node->right->data.id);
                                    })

DEFINE_OPERATION (OP_DECREMENT,     {
                                    fprintf(fp, "push [%d]\n", node->right->data.id);
                                    fprintf(fp, "push 1\n");
                                    fprintf(fp, "sub\n");
                                    fprintf(fp, "pop [%d]\n", node->right->data.id);
                                    fprintf(fp, "push [%d]\n", node->right->data.id);
                                    })

DEFINE_OPERATION (OP_MUL,           {
                                    WriteBothNodes()
                                    fprintf(fp, "mul\n");
                                    })

DEFINE_OPERATION (OP_DIV,           {
                                    WriteBothNodes()
                                    fprintf(fp, "div\n");
                                    })

DEFINE_OPERATION (OP_POW,           {
                                    WriteBothNodes()
                                    fprintf(fp, "pow\n");
                                    })

DEFINE_OPERATION (OP_AND,           {
                                    WriteBothNodes()
                                    fprintf(fp, "and\n");
                                    })

DEFINE_OPERATION (OP_OR,            {
                                    WriteBothNodes()
                                    fprintf(fp, "or\n");
                                    })

DEFINE_OPERATION (OP_NOT,           {
                                    WriteEquation(node->right, fp);
                                    fprintf(fp, "not\n");
                                    })

DEFINE_OPERATION (OP_SIN,           {
                                    WriteEquation(node->right, fp);
                                    fprintf(fp, "sin\n");
                                    })

DEFINE_OPERATION (OP_COS,           {
                                    WriteEquation(node->right, fp);
                                    fprintf(fp, "cos\n");
                                    })

DEFINE_OPERATION (OP_SQRT,          {
                                    WriteEquation(node->right, fp);
                                    fprintf(fp, "sqrt\n");
                                    })

DEFINE_OPERATION (OP_INPUT,         {
                                    if (node->right->type == VARIABLE)
                                        {
                                        fprintf(fp, "in\n");
                                        fprintf(fp, "pop [%d]\n", node->right->data.id);
                                        fprintf(fp, "push [%d]\n", node->right->data.id);
                                        }
                                    else if (node->right->type == ARRAY)
                                        {
                                        fprintf(fp, "in\n");
                                        fprintf(fp, "pop [%d]\n", node->right->data.id * ARRAY_MAX_SIZE + ARRAY_SEGMENT + (int) node->right->right->data.val);
                                        fprintf(fp, "push [%d]\n", node->right->data.id);
                                        }
                                    else
                                        {
                                        printf("Syntax error: input type is not variable\n");
                                        return SyntaxError;
                                        }
                                    })

DEFINE_OPERATION (OP_OUTPUT,        {
                                    WriteEquation(node->right, fp);
                                    fprintf(fp, "out\n");
                                    })

DEFINE_OPERATION (OP_RETURN,        {
                                    if (node->right)
                                        {
                                        WriteEquation(node->right, fp);
                                        fprintf(fp, "pop reg0\n");
                                        }
                                    else
                                        {
                                        fprintf(fp, "push 0\n");
                                        fprintf(fp, "pop reg0\n");
                                        }
                                    fprintf(fp, "ret\n");
                                    })

#ifndef WOLFRAM_H
#define WOLFRAM_H

const int LINE_LENGTH = 1000;

const double E  = 2.7182818;
const double PI = 3.1415926;

const double MEASURE_ERROR = 0.000001;

double Eval(const Node* node, double x);
Error_t Differentiation(Node** dest, const Node* src);
bool Simplifier(Node** node);

#endif // WOLFRAM_H

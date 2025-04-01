// calculator.h
#ifndef CALCULATOR_H
#define CALCULATOR_H

// Function declarations
double calculate(double num1, char operator, double num2, int *error);
void save_ans(double result);
double get_ans(void);

// Error codes
#define NO_ERROR 0
#define SYNTAX_ERROR 1
#define MATH_ERROR 2

#endif

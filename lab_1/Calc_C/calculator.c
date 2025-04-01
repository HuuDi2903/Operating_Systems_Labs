// calculator.c
#include "calculator.h"
#include <stdio.h>

#define ANS_FILE "ans.txt"

double calculate(double num1, char operator, double num2, int *error) {
    double result = 0;
    *error = NO_ERROR;

    switch (operator) {
        case '+':
            result = num1 + num2;
            break;
        case '-':
            result = num1 - num2;
            break;
        case 'x':
            result = num1 * num2;
            break;
        case '/':
            if (num2 == 0) {
                *error = MATH_ERROR;
                return 0;
            }
            result = num1 / num2;
            break;
        case '%':
            if (num2 == 0) {
                *error = MATH_ERROR;
                return 0;
            }
            result = (int)num1 % (int)num2;
            break;
        default:
            *error = SYNTAX_ERROR;
            return 0;
    }

    return result;
}

void save_ans(double result) {
    FILE *file = fopen(ANS_FILE, "w");
    if (file) {
        fprintf(file, "%.2f", result);
        fclose(file);
    }
}

double get_ans(void) {
    FILE *file = fopen(ANS_FILE, "r");
    double ans = 0;
    if (file) {
        fscanf(file, "%lf", &ans);
        fclose(file);
    }
    return ans;
}

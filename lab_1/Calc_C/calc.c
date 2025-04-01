// calc.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "calculator.h"

#define MAX_INPUT 100

int is_number(const char *str) {
    char *endptr;
    strtod(str, &endptr);
    return *endptr == '\0';
}

void clear_screen(void) {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

int main(void) {
    char input[MAX_INPUT];
    char num1_str[MAX_INPUT], operator_str[2], num2_str[MAX_INPUT];
    double num1, num2, result;
    int error;

    while (1) {
        clear_screen();
        printf(">> ");
        
        if (!fgets(input, sizeof(input), stdin)) {
            continue;
        }

        // Remove newline
        input[strcspn(input, "\n")] = 0;

        // Check for EXIT command
        if (strcmp(input, "EXIT") == 0) {
            break;
        }

        // Parse input
        if (sscanf(input, "%s %s %s", num1_str, operator_str, num2_str) != 3) {
            printf("SYNTAX ERROR\n");
            getchar();
            continue;
        }

        // Handle ANS keyword
        if (strcmp(num1_str, "ANS") == 0) {
            num1 = get_ans();
        } else if (is_number(num1_str)) {
            num1 = atof(num1_str);
        } else {
            printf("SYNTAX ERROR\n");
            getchar();
            continue;
        }

        if (strcmp(num2_str, "ANS") == 0) {
            num2 = get_ans();
        } else if (is_number(num2_str)) {
            num2 = atof(num2_str);
        } else {
            printf("SYNTAX ERROR\n");
            getchar();
            continue;
        }

        // Perform calculation
        result = calculate(num1, operator_str[0], num2, &error);

        // Handle errors and display result
        if (error == SYNTAX_ERROR) {
            printf("SYNTAX ERROR\n");
        } else if (error == MATH_ERROR) {
            printf("MATH ERROR\n");
        } else {
            printf("%.2f\n", result);
            save_ans(result);
        }

        getchar();
    }

    return 0;
}

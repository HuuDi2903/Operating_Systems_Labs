#include <stdio.h>

int main(int argc, char* argv[]) {
    printf("Hello world\n");
    getc(stdin);  // Wait for input before exiting
    return 0;
}


#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    int pid;
    printf("Start of main...\n");
    pid = fork();
    
    if (pid > 0) {
        /* parent process */
        printf("Parent section...\n");
        printf("pid = %d", pid);
    }
    else if (pid == 0) {
        /* child process */
        printf("\nfork created...\n");
        printf("pid = %d", pid);
    }
    else {
        /* fork creation failed */
        printf("\nfork creation failed!!!\n");
    }
    
    return 0;
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define PERMS 0644
#define MSG_KEY 0x123

struct my_msgbuf {
    long mtype;
    char mtext[200];
};

int main(void) {
    struct my_msgbuf buf;
    int msqid;
    int len;
    key_t key;
    
    system("touch msgq.txt");
    
    if ((msqid = msgget(MSG_KEY, PERMS | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }
    
    printf("message queue: ready to send messages.\n");
    printf("Enter lines of text, ^D to quit:\n");
    
    buf.mtype = 1;
    
    while (fgets(buf.mtext, sizeof buf.mtext, stdin) != NULL) {
        len = strlen(buf.mtext);
        
        if (buf.mtext[len-1] == '\n') buf.mtext[len-1] = '\0';
        
        if (msgsnd(msqid, &buf, len+1, 0) == -1)
            perror("msgsnd");
    }
    
    strcpy(buf.mtext, "end");
    len = strlen(buf.mtext);
    if (msgsnd(msqid, &buf, len+1, 0) == -1)
        perror("msgsnd");
    
    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }
    
    printf("message queue: done sending messages.\n");
    return 0;
}

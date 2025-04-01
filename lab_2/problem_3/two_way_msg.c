#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <unistd.h>

#define MSG_KEY_A 0x123  // Key for first queue (A→B)
#define MSG_KEY_B 0x456  // Key for second queue (B→A)
#define PERMS 0644

struct my_msgbuf {
    long mtype;
    char mtext[200];
};

// Function for receiving thread
void *receiver_thread(void *arg) {
    int msqid = *((int *)arg);
    struct my_msgbuf buf;
    
    printf("Receiver thread started, listening on queue %d\n", msqid);
    
    while (1) {
        if (msgrcv(msqid, &buf, sizeof(buf.mtext), 0, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }
        
        printf("Received: \"%s\"\n", buf.mtext);
        
        // Exit if "end" is received
        if (strcmp(buf.mtext, "end") == 0) {
            break;
        }
    }
    
    printf("Receiver thread exiting\n");
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    char mode = argv[1][0];
    if (mode != 'a' && mode != 'b') {
        printf("Invalid mode. Use 'a' or 'b'\n");
        return 1;
    }
    
    // Set up message queues
    int send_msqid, recv_msqid;
    key_t send_key, recv_key;
    
    if (mode == 'a') {
        send_key = MSG_KEY_A;  // A→B
        recv_key = MSG_KEY_B;  // B→A
        printf("Starting as Process A\n");
    } else {
        send_key = MSG_KEY_B;  // B→A
        recv_key = MSG_KEY_A;  // A→B
        printf("Starting as Process B\n");
    }
    
    // Create or connect to send queue
    if ((send_msqid = msgget(send_key, PERMS | IPC_CREAT)) == -1) {
        perror("msgget (send)");
        exit(1);
    }
    
    // Create or connect to receive queue
    if ((recv_msqid = msgget(recv_key, PERMS | IPC_CREAT)) == -1) {
        perror("msgget (recv)");
        exit(1);
    }
    
    // printf("Send queue ID: %d, Receive queue ID: %d\n", send_msqid, recv_msqid);
    
    // Start receiver thread
    pthread_t tid;
    if (pthread_create(&tid, NULL, receiver_thread, &recv_msqid) != 0) {
        perror("pthread_create");
        exit(1);
    }
    
    // Main thread handles sending
    struct my_msgbuf buf;
    buf.mtype = 1;
    
    printf("Enter lines of text:\n");
    while (fgets(buf.mtext, sizeof(buf.mtext), stdin) != NULL) {
        int len = strlen(buf.mtext);
        
        // Remove newline if present
        if (buf.mtext[len-1] == '\n') buf.mtext[len-1] = '\0';
        
        if (msgsnd(send_msqid, &buf, strlen(buf.mtext) + 1, 0) == -1) {
            perror("msgsnd");
        }
        
        // Exit if "end" is sent
        if (strcmp(buf.mtext, "end") == 0) {
            break;
        }
    }
    
    // Wait for receiver thread to finish
    pthread_join(tid, NULL);
    
    printf("Communication ended\n");
    return 0;
}
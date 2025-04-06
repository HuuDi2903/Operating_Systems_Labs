#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

typedef struct Node {
    int value;
    struct Node* next;
} Node;

typedef struct LockFreeStack {
    _Atomic(Node*) head;
} LockFreeStack;

// Push an item onto the stack
void push(LockFreeStack* stack, int value) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->value = value;
    new_node->next = NULL;

    Node* old_head;
    do {
        old_head = atomic_load(&stack->head);
        new_node->next = old_head;
    } while (!atomic_compare_exchange_weak(&stack->head, &old_head, new_node));

    /*  
    Atomically try to update the new node to be the head.
    Ensure that no thread can interrupt.  
    */
}

// Pop an item from the stack
int pop(LockFreeStack* stack, int* value) {
  Node* old_head;
  do {
      old_head = atomic_load(&stack->head);   
      if (old_head == NULL) {
          return 0;                          // Stack is empty
      }
  } while (!atomic_compare_exchange_weak(&stack->head, &old_head, old_head->next));

  *value = old_head->value;                 
  free(old_head);                        
  return 1;                               
}

int main() {
  LockFreeStack stack;
  atomic_init(&stack.head, NULL);         

  printf("Pushing elements onto the stack...\n");
  for(int i = 0; i < 5; i++) {       
      push(&stack, i);
      printf("Pushed: %d\n", i);
  }

  printf("Popping elements from the stack...\n");
  int value;
  while (pop(&stack, &value)) {       
      printf("Popped: %d\n", value);
  }

  return 0;
}
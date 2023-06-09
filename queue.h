#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>

typedef enum QueueResult_t {
    QUEUE_SUCCESS,
    QUEUE_NULL_ARGUMENT,
    QUEUE_EMPTY,
    QUEUE_ADD_FAILED,
    QUEUE_FULL
} QueueResult;

typedef struct Node_t {
    int descriptor;
    struct timeval arrival;
    struct Node_t* next;
    //struct Node_t* prev;
} *Node ;

typedef struct Queue_t {
    Node head; //head is the oldest in the queue
    Node tail;
} *Queue ;

// ---------------- Node funcs ---------------------------

Node NodeCreate(int descriptor, struct timeval arrival);
void NodeDelete(Node node);

// ---------------- Queue funcs ---------------------------
Queue QueueCreate();
void QueueDestroy(Queue queue);

QueueResult QueueAdd(Queue queue, int descriptor,  struct timeval arrival);
QueueResult QueueDeleteByDescriptor(Queue queue, int descriptor);
int QueueDeleteByIndex(Queue queue, int index);
Node QueueGetByIndex (Queue queue, int index);
int QueueRemoveHead(Queue queue);
Node QueueGetHead(Queue queue);


#endif //QUEUE_H

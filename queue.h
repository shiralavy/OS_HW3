#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>

#ifndef HW3_WET_OS_QUEUE_H
#define HW3_WET_OS_QUEUE_H

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
} Node ;

typedef struct Queue_t {
    Node* head; //head is the oldest in the queue
    Node* tail;
    int size;
    int max_size;
    //int max_size;
} Queue ;


// ---------------- Node funcs ---------------------------

Node* NodeCreate(int descriptor, struct timeval arrival);
void NodeDelete(Node* node);

// ---------------- Queue funcs ---------------------------
Queue* QueueCreate(int max_size);
void QueueDestroy(Queue* queue);
int QueueGetSize(Queue* queue);
QueueResult QueueAdd(Queue* queue, int descriptor,  struct timeval arrival);
QueueResult QueueDeleteByDescriptor(Queue* queue, int descriptor);
int QueueDeleteByIndex(Queue* queue, int index);
Node* QueueGetByDescriptor(Queue* queue, int descriptor);
Node* QueueGetByIndex (Queue* queue, int index);
int QueueRemoveHead(Queue* queue);
Node* QueueGetHead(Queue* queue);

#endif //HW3_WET_OS_QUEUE_H
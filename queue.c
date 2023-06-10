#include "queue.h"
#include "segel.h"
#include "request.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// ---------------- Node funcs ---------------------------

Node* NodeCreate(int descriptor, struct timeval arrival){
    Node* node = malloc(sizeof(node));
    if (!node){
        return NULL;
    }
    node->descriptor = descriptor;
    node->next = NULL;
    node->arrival = arrival;
    return node;
}

void NodeDelete(Node* node){
    free(node);
}

// ---------------- Queue funcs ---------------------------

Queue* QueueCreate(int max_size) {
    Queue* queue = malloc(sizeof(queue));
    if (!queue) {
        return NULL;
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    queue->max_size = max_size;
    return queue;
}

void QueueDestroy(Queue* queue){
    Node* curr = queue->head;
    Node* next = NULL;
    while (curr){
        next = curr->next;
        NodeDelete(curr);
        curr = next;
    }
    free(queue);
}

int QueueGetSize(Queue* queue){
    if (!queue){
        return 0;
    }
    return queue->size;
}

QueueResult QueueAdd(Queue* queue, int descriptor, struct timeval arrival) {
    if(!queue) {
        return QUEUE_NULL_ARGUMENT;
    }
    Node* new_node = NodeCreate(descriptor, arrival);
    if (!new_node) {
        return QUEUE_ADD_FAILED;
    }
    if(QueueGetSize(queue) == 0) {
        queue->head = new_node;
        queue->tail = new_node;
    }
    else {
        queue->tail->next = new_node;
        queue->tail = new_node;
    }
    queue->size++;
    return QUEUE_SUCCESS;
}

QueueResult QueueDeleteByDescriptor(Queue* queue, int descriptor) {
    if (!queue) {
        return QUEUE_EMPTY;
    }
    Node *to_delete = QueueGetByDescriptor(queue, descriptor);
    if (!to_delete) {
        return QUEUE_NULL_ARGUMENT;
    }
    if (to_delete == queue->head){
        if (queue->tail != NULL && to_delete == queue->tail){ //node is the only element in the queue
            queue->tail = NULL;
            queue->head = NULL;
        }
        else{ //node is the head of the queue
            queue->head = to_delete->next;
        }
    }
    else if (queue->tail != NULL && to_delete == queue->tail){
        queue->tail->next = NULL;
    }

    NodeDelete(to_delete);
    queue->size--;
    return QUEUE_SUCCESS;
}

int QueueDeleteByIndex(Queue* queue, int index){
    if (!queue || QueueGetSize(queue) == 0) {
        return -1;
    }
    Node *to_delete = QueueGetByIndex(queue, index);
    if (!to_delete) {
        return QUEUE_NULL_ARGUMENT;
    }
    if (to_delete == queue->head){
        if (queue->tail != NULL && to_delete == queue->tail){ //node is the only element in the queue
           queue->tail = NULL;
           queue->head = NULL;
        }
        else{ //node is the head of the queue
            queue->head = to_delete->next;
        }
    }
    else if (queue->tail != NULL && to_delete == queue->tail){
        queue->tail->next = NULL;
    }
    int descriptor =  to_delete->descriptor;
    NodeDelete(to_delete);
    queue->size--;
    return descriptor;

}

Node* QueueGetByDescriptor(Queue* queue, int descriptor){
    if (!queue || QueueGetSize(queue) == 0){
        return NULL;
    }
    Node* temp = queue->head;
    while (temp){
        if (temp->descriptor == descriptor){
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}
Node* QueueGetByIndex(Queue* queue, int index){
    if (!queue || QueueGetSize(queue) == 0){
        return NULL;
    }
    Node* temp = queue->head;
    int i = 0;
    while (temp){
        if (i == index){
            return temp;
        }
        i++;
        temp = temp->next;
    }
    return NULL;
}

int QueueRemoveHead(Queue* queue){
    if (!queue || QueueGetSize(queue) == 0){
        return -1;
    }
    Node* to_delete = queue->head;
    if (queue->tail != NULL && to_delete == queue->tail){ //head is the only element in the queue
        queue->tail = NULL;
        queue->head = NULL;
    }
    else{ //node is the head of the queue
        queue->head = to_delete->next;
    }
    int descriptor =  to_delete->descriptor;
    NodeDelete(to_delete);
    queue->size--;
    return descriptor;
}

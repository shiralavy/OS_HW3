#include "queue.h"
#include "segel.h"
#include "request.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// ---------------- Node funcs ---------------------------

Node NodeCreate(int descriptor, struct timeval arrival){
    Node node = malloc(sizeof(*node));
    if (!node){
        return NULL;
    }
    node->descriptor = descriptor;
    node->next = NULL;
    node->arrival = arrival;
    return node;
}

void NodeDelete(Node node){
    free(node);
}

// ---------------- Queue funcs ---------------------------
Queue QueueCreate(){
    Queue queue = malloc(sizeof(*queue));
    if (!queue) {
        return NULL;
    }
    queue->head = NULL;
    queue->tail = NULL;
    return queue;
}

void QueueDestroy(Queue queue){
    if (!queue){
        return;
    }
    Node curr = queue->head;
    Node to_delete = NULL;
    while (curr){
        to_delete = curr;
        curr = curr->next;
        NodeDelete(to_delete);
    }
    free(queue);
}

QueueResult QueueAdd(Queue queue, int descriptor,  struct timeval arrival){
    if(!queue) {
        return QUEUE_NULL_ARGUMENT;
    }
    Node new_node = NodeCreate(descriptor, arrival);
    if (!new_node) {
        return QUEUE_ADD_FAILED;
    }
    if(!queue->head) {
        queue->head = new_node;
        queue->tail = new_node;
    }
    else {
        queue->tail->next = new_node;
        queue->tail = new_node;
    }
    return QUEUE_SUCCESS;
}

QueueResult QueueDeleteByDescriptor(Queue queue, int descriptor){
    if (!queue) {
        return QUEUE_NULL_ARGUMENT;
    }
    Node to_delete = queue->head;
    int index = 0;
    while (to_delete) {
        if (to_delete->descriptor == descriptor) {
            break;
        }
        index++;
        to_delete = to_delete->next;
    }
    if (!to_delete) {
        return QUEUE_NULL_ARGUMENT;
    }
    if (to_delete == queue->head){
        return QueueRemoveHead(queue);
    }
    else if (queue->tail != NULL && to_delete == queue->tail){
        Node new_tail = QueueGetByIndex(queue, index-1);
        queue->tail = new_tail;
        queue->tail->next = NULL;
    }
    else {
        Node prev_to_delete = QueueGetByIndex(queue, index-1);
        prev_to_delete->next = to_delete->next;
    }
    NodeDelete(to_delete);
    return QUEUE_SUCCESS;
}

int QueueDeleteByIndex(Queue queue, int index){
    if (!queue) {
        return QUEUE_NULL_ARGUMENT;
    }
    Node to_delete = QueueGetByIndex(queue, index);
    if (!to_delete) {
        return QUEUE_NULL_ARGUMENT;
    }
    if (to_delete == queue->head){
        return QueueRemoveHead(queue);
    }
    else if (queue->tail != NULL && to_delete == queue->tail){
        Node new_tail = QueueGetByIndex(queue, index-1);
        queue->tail = new_tail;
        queue->tail->next = NULL;
    }
    else {
        Node prev_to_delete = QueueGetByIndex(queue, index-1);
        prev_to_delete->next = to_delete->next;
    }
    int descriptor = to_delete->descriptor;
    NodeDelete(to_delete);
    return descriptor;
}


Node QueueGetByIndex (Queue queue, int index){
    if (!queue || !queue->head){
        return NULL;
    }
    Node temp = queue->head;
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

int QueueRemoveHead(Queue queue){
    if (!queue || !queue->head){
        return -1;
    }
    Node to_delete = queue->head;
    if (queue->tail != NULL && to_delete == queue->tail){ //head is the only element in the queue
        queue->tail = NULL;
        queue->head = NULL;
    }
    else{ //node is the head of the queue
        queue->head = to_delete->next;
    }
    int descriptor =  to_delete->descriptor;
    //maybe not needed
    to_delete->next = NULL;
    to_delete->descriptor = 0;
    NodeDelete(to_delete);
    return descriptor;
}

Node QueueGetHead(Queue queue){
    if (queue){
        return queue->head;
    }
    return NULL;
}



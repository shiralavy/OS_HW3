#include "segel.h"
#include "request.h"
#include "queue.h"
#include <pthread.h>
#include <sys/time.h>

// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// HW3: Parse the new arguments too

pthread_mutex_t mutex;
pthread_cond_t cond;
pthread_cond_t block_cond;

Queue* pending_queue = NULL;
Queue* running_queue = NULL;

struct thread_element{
    pthread_t thread;
    int thread_id;              //0 to num_of_threads-1
    int thread_total_count;     //num of requests thread handled
    int thread_static_count;    //num of static requests thread handled
    int thread_dynamic_count;   //num of dynamic thread handled
};

void getargs(int *port, int *threads, int *queue_size, char* schedalg, int* max_size, int argc, char *argv[])
{
    if (argc < 2) {
	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    *threads = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
    strcpy(schedalg, argv[4]);
    if (argv[5]) {
        *max_size = atoi(argv[5]);
    }
    else {
        max_size = NULL;
    }
}


void* thread_action(void *args) {
    struct thread_element* thread_element = (struct thread_element*)args;
    while(1){
        pthread_mutex_lock(&mutex);
        while (QueueGetSize(pending_queue) == 0){
            pthread_cond_wait(&cond, &mutex);
        }
        Node* head = QueueGetByIndex(pending_queue, 0);
        int fd = head->descriptor;
        struct timeval arrival = head->arrival;
        QueueRemoveHead(pending_queue);
        QueueAdd(running_queue, fd, arrival);

        pthread_mutex_unlock(&mutex);
        struct timeval handeling;
        gettimeofday(&handeling, NULL);
        struct timeval dispatch;
        timersub(&handeling,&arrival,&dispatch);
        requestHandle(fd, arrival, dispatch, thread_element->thread_id, &thread_element->thread_total_count,
                      &thread_element->thread_static_count, &thread_element->thread_dynamic_count);
        Close(fd);
        pthread_mutex_lock(&mutex);
        QueueDeleteByDescriptor(running_queue, fd);
        pthread_cond_signal(&block_cond);
        pthread_mutex_unlock(&mutex);

    }
    return NULL;
}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, threads_num, queue_size, max_size;
    char schedalg[7];
    struct sockaddr_in clientaddr;

    getargs(&port, &threads_num, &queue_size, schedalg, &max_size, argc, argv);
    //Create some threads
    pending_queue = QueueCreate(queue_size);
    running_queue = QueueCreate(threads_num);

    for (int i = 0; i < threads_num; i++) {
        struct thread_element* new_thread_element = malloc(sizeof(struct thread_element));
        new_thread_element->thread_id = i;
        new_thread_element->thread_total_count = 0;
        new_thread_element->thread_static_count = 0;
        new_thread_element->thread_dynamic_count = 0;

        pthread_create(&(new_thread_element->thread), NULL, thread_action, (void*)new_thread_element);
    }

    //mutex create
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_cond_init(&block_cond, NULL);

    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

        pthread_mutex_lock(&mutex);
        if(QueueGetSize(&running_queue) + QueueGetSize(&pending_queue) == queue_size) {
            if(strcmp("block", schedalg) == 0) {
                while(QueueGetSize(&running_queue) + QueueGetSize(&pending_queue) == queue_size) {
                    pthread_cond_wait(&block_cond, &mutex);
                }
            }
            else if (strcmp("dt", schedalg) == 0) {
                Close(connfd);
                pthread_mutex_unlock(&mutex);
                continue;
            }
            else if (strcmp("dh", schedalg) == 0) {
                if(QueueGetSize(pending_queue) == 0){
                    Close(connfd);
                    pthread_mutex_unlock(&mutex);
                    continue;
                }
                else {
                    int fd = QueueRemoveHead(pending_queue);
                    Close(fd);
                }
            }
            else if (strcmp("bf", schedalg) == 0){
                while(QueueGetSize(&running_queue) + QueueGetSize(&pending_queue) > 0) {
                    pthread_cond_wait(&cond, &mutex);
                }
            }
            else if (strcmp("dynamic", schedalg) == 0){
                if (queue_size >= max_size){
                    Close(connfd);
                    pthread_mutex_unlock(&mutex);
                    continue;
                }
                else{
                    Close(connfd);
                    queue_size++;
                    pthread_mutex_unlock(&mutex);
                    continue;
                }
            }
            else if (strcmp("random", schedalg) == 0){
                if (QueueGetSize(pending_queue) == 0) {
                    close(connfd);
                    pthread_mutex_unlock(&mutex);
                    continue;
                }
                else {
                    int half = QueueGetSize(pending_queue)/2;
                    int index;
                    for (int i = 0; i < half ;i++) {
                       index = rand() % QueueGetSize(pending_queue);
                       int fd = QueueDeleteByIndex(pending_queue, index);
                       Close(fd);
                    }
                }
            }
        }

        struct timeval arrival;
        gettimeofday(&arrival,NULL);

        QueueAdd(pending_queue, connfd, arrival);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);

        //TODO free malloced data?
//        requestHandle(connfd);
//        Close(connfd);
    }

}


    


 

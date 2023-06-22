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

pthread_mutex_t mutex;
pthread_cond_t waiting_queue_has_requests;
pthread_cond_t block_cond;

//Queue running_queue = NULL;
Queue waiting_queue = NULL;
int waiting_size;
int running_size;

typedef struct thread_element_t{
    pthread_t thread;
    int thread_id;              //0 to num_of_threads-1
    int thread_total_count;     //num of requests thread handled
    int thread_static_count;    //num of static requests thread handled
    int thread_dynamic_count;   //num of dynamic thread handled
} thread_element;

// HW3: Parse the new arguments too
void getargs(int *port, int *threads, int *queue_size, char* schedalg, int* max_size, int argc, char *argv[])
{
    if (argc < 5) {
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
    thread_element* thread = (thread_element*)args;
    int fd;
    struct timeval arrival;
    while(1){
        pthread_mutex_lock(&mutex);
        while (waiting_size == 0){
            pthread_cond_wait(&waiting_queue_has_requests, &mutex);
        }
        arrival = QueueGetHead(waiting_queue)->arrival;
        fd = QueueRemoveHead(waiting_queue);
        waiting_size--;
        running_size++;
        pthread_mutex_unlock(&mutex);

        struct timeval handeling;
        gettimeofday(&handeling, NULL);
        struct timeval dispatch;
        timersub(&handeling,&arrival,&dispatch);

        requestHandle(fd, arrival, dispatch, thread->thread_id, &thread->thread_total_count,
                      &thread->thread_static_count, &thread->thread_dynamic_count);
        Close(fd);

        pthread_mutex_lock(&mutex);
        running_size--;
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&block_cond);
    }
}


int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, threads_num, queue_size, max_size;
    char schedalg[7];
    struct sockaddr_in clientaddr;

    getargs(&port, &threads_num, &queue_size, schedalg, &max_size, argc, argv);

    waiting_size = 0;
    running_size = 0;

    waiting_queue = QueueCreate();
    //running_queue = QueueCreate();

    //mutex create
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&waiting_queue_has_requests, NULL);
    pthread_cond_init(&block_cond, NULL);

    for (int i = 0; i < threads_num; i++) {
        thread_element* new_thread_element = malloc(sizeof(new_thread_element));
        new_thread_element->thread_id = i;
        new_thread_element->thread_total_count = 0;
        new_thread_element->thread_static_count = 0;
        new_thread_element->thread_dynamic_count = 0;

        pthread_create(&(new_thread_element->thread), NULL, thread_action, (void*)new_thread_element);
    }

    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        struct timeval arrival;
        gettimeofday(&arrival,NULL);

        pthread_mutex_lock(&mutex);

        if ((waiting_size + running_size) >= queue_size){
            if(strcmp("block", schedalg) == 0) {
                while((waiting_size + running_size) == queue_size) {
                    pthread_cond_wait(&block_cond, &mutex);
                }
            }
            else if (strcmp("dt", schedalg) == 0) {
                Close(connfd);
                pthread_mutex_unlock(&mutex);
                continue;
            }
            else if (strcmp("dh", schedalg) == 0) {
                if(waiting_size == 0){
                    Close(connfd);
                    pthread_mutex_unlock(&mutex);
                    continue;
                }
                else{
                    int fd = QueueRemoveHead(waiting_queue);
                    Close(fd);
                }
            }
            else if (strcmp("bf", schedalg) == 0){
                while(waiting_size + running_size > 0) {
                    pthread_cond_wait(&waiting_queue_has_requests, &mutex);
                    Close(connfd);
                    pthread_mutex_unlock(&mutex);
                    continue;
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
                if (waiting_size == 0) {
                    Close(connfd);
                    pthread_mutex_unlock(&mutex);
                    continue;
                }
                else {
                    int half = (int) ((waiting_size + 1) / 2);
                    int index;
                    for (int i = 0; i < half ;i++) {
                        index = rand() % waiting_size;
                        int fd = QueueDeleteByIndex(waiting_queue, index);
                        Close(fd);
                        waiting_size--;
                    }
                }
            }
        }

        QueueAdd(waiting_queue, connfd, arrival);
        waiting_size++;
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&waiting_queue_has_requests);
    }

}


    


 

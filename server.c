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
        Node* head = QueueGetHead(pending_queue);
        int fd = head->descriptor;
        struct timeval arrival = head->arrival;
        if(running_queue->max_size > running_queue->size) {
            QueueRemoveHead(pending_queue);
            printf("pending queue size1: %d\n", pending_queue->size);
            printf("running queue size1: %d\n", running_queue->size);

            printf("queue result1:%d\n", QueueAdd(running_queue, fd, arrival));
            printf("pending queue size2: %d\n", pending_queue->size);
            printf("running queue size2: %d\n", running_queue->size);

            pthread_mutex_unlock(&mutex);
        }
        struct timeval handeling;
        gettimeofday(&handeling, NULL);
        struct timeval dispatch;
        timersub(&handeling,&arrival,&dispatch);
        pthread_mutex_lock(&mutex);
        requestHandle(fd, arrival, dispatch, thread_element->thread_id, &thread_element->thread_total_count,
                      &thread_element->thread_static_count, &thread_element->thread_dynamic_count);
        Close(fd);
        QueueDeleteByDescriptor(running_queue, fd);
        printf("pending queue size3: %d\n", pending_queue->size);
        printf("running queue size3: %d\n", running_queue->size);

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

    printf("pending queue size4: %d\n", pending_queue->size);
    printf("running queue size4: %d\n", running_queue->size);

    for (int i = 0; i < threads_num; i++) {
        struct thread_element* new_thread_element = malloc(sizeof(struct thread_element));
        new_thread_element->thread_id = i;
        new_thread_element->thread_total_count = 0;
        new_thread_element->thread_static_count = 0;
        new_thread_element->thread_dynamic_count = 0;

        pthread_create(&(new_thread_element->thread), NULL, thread_action, (void*)new_thread_element);
        printf("created thread: %d", i);
    }

    //mutex create
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_cond_init(&block_cond, NULL);

    int running_q_size;
    int pending_q_size;

    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

        pthread_mutex_lock(&mutex);
        printf("lock\n");
        running_q_size = QueueGetSize(running_queue);
        pending_q_size = QueueGetSize(pending_queue);
        printf("q_size: %d %d\n", running_q_size, pending_q_size);
        printf("pending queue size5: %d\n", pending_queue->size);
        printf("running queue size5: %d\n", running_queue->size);

        if(running_q_size + pending_q_size >= queue_size) {
            if(strcmp("block", schedalg) == 0) {
                while((QueueGetSize(running_queue) + QueueGetSize(pending_queue)) == queue_size) {
                    printf("block while\n");
                    pthread_cond_wait(&block_cond, &mutex);
                }
            }
            else if (strcmp("dt", schedalg) == 0) {
                printf("dt\n");
                Close(connfd);
                pthread_mutex_unlock(&mutex);
                continue;
            }
            else if (strcmp("dh", schedalg) == 0) {
                if(QueueGetSize(pending_queue) == 0){
                    printf("dh\n");
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
                printf("bf\n");
                while((QueueGetSize(running_queue) + QueueGetSize(pending_queue)) > 0) {
                    pthread_cond_wait(&cond, &mutex);
                }
            }
            else if (strcmp("dynamic", schedalg) == 0){
                printf("dynamic\n");
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
                printf("random\n");
                if (QueueGetSize(pending_queue) == 0) {
                    Close(connfd);
                    pthread_mutex_unlock(&mutex);
                    continue;
                }
                else {
                    printf("pending_size_random: %d\n",(QueueGetSize(pending_queue)));
                    int half = (int) ((QueueGetSize(pending_queue) + 1) / 2);
                    int index;
                    printf("half: %d\n", half);
                    for (int i = 0; i < half ;i++) {
                        index = rand() % QueueGetSize(pending_queue);
                        printf("index %d\n", index);
                        int fd = QueueDeleteByIndex(pending_queue, index);
                        Close(fd);
                    }
                    Close(connfd);
                    pthread_mutex_unlock(&mutex);
                    continue;
                }
            }
        }

        struct timeval arrival;
        gettimeofday(&arrival,NULL);

        printf("queue result2:%d\n", QueueAdd(pending_queue, connfd, arrival));
        printf("pending queue size6: %d\n", pending_queue->size);
        printf("running queue size6: %d\n", running_queue->size);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
        printf("unlock1\n");

        //TODO free malloced data?
//        requestHandle(connfd);
//        Close(connfd);
    }

}






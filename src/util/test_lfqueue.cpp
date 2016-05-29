#include <atomic>
#include <cstdint>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "nvm0lfqueue.h"
#include "nvm0lfqueue.cpp"

void test_queue1()
{
    printf("\nTest 1\n");
    lfqueue<int>* queue = new lfqueue<int>(10);
    
    queue->enqueue(1);
    printf("value dequeued: %d\n", queue->dequeue());

    delete queue;
}

void test_queue10()
{
    printf("\nTest 10\n");
    lfqueue<int>* queue = new lfqueue<int>(10);
    
    int i;
    for(i=0; i<10; i++) {
        queue->enqueue(i);
    }
    for(i=0; i<10; i++) {
        printf("value dequeued: %d\n", queue->dequeue());
    }

    delete queue;
}

void test_is_empty()
{
    printf("\nTest is_empty\n");
    lfqueue<int>* queue = new lfqueue<int>(10);
    
    printf("At first, is_empty() = %d\n", queue->is_empty());
    queue->enqueue(1);
    printf("enqueue 1, is_empty() = %d\n", queue->is_empty());
    queue->dequeue();
    printf("dequeue 1, is_empty() = %d\n", queue->is_empty());

    delete queue;
}

lfqueue<int>* gqueue;
atomic<int> gcount;
int timeout;
int queuesize = 10000;
void* producer(void* what)
{
    while(!timeout) {
        int value = gcount;
        if(value < queuesize) {
            gcount++;
            gqueue->enqueue(value);
            printf("Producer: %d enqueued\n", value);
        }
    }
    printf("producer timeout!\n");
}
void* consumer(void* what)
{
    while(!timeout) {
        gcount--;
        printf("Consumer: %d dequeued\n", gqueue->dequeue());
    }
    printf("consumer timeout!\n");
}

void test_concurrency()
{
    printf("\nTest concurrency(1 producers, 10 consumers)\n");
    gqueue = new lfqueue<int>(queuesize);
    
    int i;
    gcount = 0;
    timeout = 0;
    pthread_t p_thread;
    pthread_create(&p_thread, NULL, producer, NULL);

    pthread_t c_thread[10];
    for(i=0; i<10; i++) {
        pthread_create(&c_thread[i], NULL, consumer, NULL);
    }

    usleep(5000000);
    timeout = 1;

    pthread_join(p_thread, NULL);
    gqueue->enqueue(9999);
    gqueue->enqueue(9999);
    gqueue->enqueue(9999);
    gqueue->enqueue(9999);
    gqueue->enqueue(9999);
    gqueue->enqueue(9999);
    gqueue->enqueue(9999);
    gqueue->enqueue(9999);
    gqueue->enqueue(9999);
    gqueue->enqueue(9999);
    for(i=0; i<10; i++) {
        pthread_join(c_thread[i], NULL);
    }

    delete gqueue;
}

void test_lfqueue()
{
    test_concurrency();
    test_queue1();
    test_queue10();
    test_is_empty();
}

//g++ -std=c++11 -o test_lfqueue nvm0lfqueue.cpp -lpthread
int main()
{
    test_lfqueue();
}

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

void test_get_size()
{
    printf("\nTest get_size\n");
    lfqueue<int>* queue = new lfqueue<int>(10);
    
    printf("At first, get_size() = %d\n", queue->get_size());
    queue->enqueue(1);
    printf("enqueue 1, get_size() = %d\n", queue->get_size());
    queue->enqueue(2);
    printf("enqueue 2, get_size() = %d\n", queue->get_size());
    queue->dequeue();
    printf("dequeue 1, get_size() = %d\n", queue->get_size());
    queue->dequeue();
    printf("dequeue 2, get_size() = %d\n", queue->get_size());

    delete queue;
}

lfqueue<int>* queue1;
lfqueue<int>* queue2;
int queuesize = 10000;
int timeout;

void* producer(void* arg)
{
    int i = *((int*) arg);
    int value;
    int counter = 0; 
    while(!timeout) {
        value = queue2->dequeue();
        queue1->enqueue(value);
        //printf("Producer %d: %d enqueued\n", i, value);
        counter++;
    }
    printf("producer %d produced %d times!\n", i, counter);
}
void* consumer(void* arg)
{
    int i = *((int*) arg);
    int value;
    int counter = 0; 
    while(!timeout) {
        value = queue1->dequeue();
        queue2->enqueue(value);
        //printf("Consumer %d: %d dequeued\n", i, value);
        counter++;
    }
    printf("consumer %d consumed %d times!\n", i, counter);
}

void test_concurrency()
{
    int i;
    int nthread = 10;

    printf("\nTest concurrency(%d producers, %d consumers)\n", nthread, nthread);
    queue1 = new lfqueue<int>(queuesize);
    queue2 = new lfqueue<int>(queuesize);
    for(i=1; i<=queuesize; i++) {
        queue1->enqueue(i);
    }

    timeout = 0;
    pthread_t p_thread[nthread];
    pthread_t c_thread[nthread];
    for(i=0; i<nthread; i++) {
        pthread_create(&p_thread[i], NULL, producer, (void*) &i);
        pthread_create(&c_thread[i], NULL, consumer, (void*) &i);
    }

    usleep(5000000);
    timeout = 1;

    for(i=0; i<nthread; i++) {
        pthread_join(p_thread[i], NULL);
        pthread_join(c_thread[i], NULL);
    }

    delete queue1;
    delete queue2;
}

void test_lfqueue()
{
    test_queue1();
    test_queue10();
    test_is_empty();
    test_get_size();
    test_concurrency();
}

//g++ -std=c++11 -o test_lfqueue test_lfqueue.cpp nvm0lfqueue.cpp -lpthread
int main()
{
    test_lfqueue();
}

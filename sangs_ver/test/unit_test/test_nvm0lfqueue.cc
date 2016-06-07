#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "nvm0common.h"

void test_queue1()
{
    //init
    lfqueue<int>* queue = new lfqueue<int>(10);
    
    //test
    queue->enqueue(1);
    int val = queue->dequeue();

    //assert
    if(val == 1) {
        printf("test_queue1 passed.\n");
    }
    else {
        printf("test_queue1 failed.\n");
        exit(0);
    }

    //clean
    delete queue;
}

void test_queue10()
{
    //init
    int i;
    lfqueue<int>* queue = new lfqueue<int>(10);
    
    //test
    for(i=0; i<10; i++) {
        queue->enqueue(i);
    }

    //assert
    for(i=0; i<10; i++) {
        if(queue->dequeue() != i)
        {
            printf("test_queue10 failed. : %d\n", i);
            exit(0);
        }
    }
    printf("test_queue10 passed.\n");

    //clean
    delete queue;
}

void test_is_empty()
{
    //init
    lfqueue<int>* queue = new lfqueue<int>(10);
    
    //test & assert
    if(queue->is_empty() == false) {
        printf("test_is_empty failed.\n");
        exit(0);
    }
    queue->enqueue(1);
    if(queue->is_empty() == true) {
        printf("test_is_empty failed.\n");
        exit(0);
    }
    queue->dequeue();
    if(queue->is_empty() == false) {
        printf("test_is_empty failed.\n");
        exit(0);
    }
    printf("test_is_empty passed.\n");

    //clean
    delete queue;
}

void test_get_size()
{
    //init
    lfqueue<int>* queue = new lfqueue<int>(10);
    
    //test & assert
    if(queue->get_size() != 0) {
        printf("test_get_size failed.\n");
        exit(0);
    }
    queue->enqueue(1);
    if(queue->get_size() != 1) {
        printf("test_get_size failed.\n");
        exit(0);
    }
    queue->enqueue(2);
    if(queue->get_size() != 2) {
        printf("test_get_size failed.\n");
        exit(0);
    }
    queue->dequeue();
    if(queue->get_size() != 1) {
        printf("test_get_size failed.\n");
        exit(0);
    }
    queue->dequeue();
    if(queue->get_size() != 0) {
        printf("test_get_size failed.\n");
        exit(0);
    }
    printf("test_get_size passed.\n");

    //clean
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
    //init
    int i;
    int nthread = 10;
    printf("Test concurrency(%d producers, %d consumers) started.\n", nthread, nthread);
    queue1 = new lfqueue<int>(queuesize);
    queue2 = new lfqueue<int>(queuesize);

    //test
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

    //assert
    printf("Test concurrency(%d producers, %d consumers) passed.\n", nthread, nthread);

    //clean
    delete queue1;
    delete queue2;
}

void test_nvm0lfqueue()
{
    printf("====================TEST nvm0lfqueue.cc====================\n");
    test_queue1();
    test_queue10();
    test_is_empty();
    test_get_size();
    test_concurrency();
}

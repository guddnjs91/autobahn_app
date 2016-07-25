#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include "nvm0common.h"
#include "test.h"

///////////////////////////////
//////////APPEND TEST//////////
///////////////////////////////

/**
 * Write thread fills in buffer and write it to nvm */
void
*thread_nvm_write_append(
    void *data)
{
    long long unsigned int n = filesize / nthread / nbytes;
    long long unsigned int i;
    uint32_t tid = *((uint32_t *)data);

    clock_t start = clock();
    
//    printf("Thread %u writes %u Bytes * %llu to VOL_%u.txt\n", tid, nbytes, n, tid);

    for(i = 0; i < n; i++) {
//        if(i == n-1)
//        {
//            printf("Here\n");
//        }
        nvm_write(tid, (off_t) (i * nbytes) , buffer, nbytes);
    }

    durations[tid-1] = ( clock() - start ) / (double) CLOCKS_PER_SEC;

    return NULL;
}

void
test_nvm_write_append()
{
    durations = new double[nthread];
    pthread_t write_thread[nthread];
    int tid[nthread];
    int i;
    double averageDuration = 0.0;

    for(i=0; i<nthread; i++) {
        tid[i] = i + 1;
        pthread_create(&write_thread[i], NULL, thread_nvm_write_append, (void *)&tid[i]);
    }
    for(i=0; i<nthread; i++) {
        pthread_join(write_thread[i], NULL);
    }

    for(i=0; i<nthread; i++) {
        averageDuration += durations[i];
    }

    averageDuration /= nthread;

    printf("average duration of %d thread : %f sec\n",nthread, averageDuration);
}

///////////////////////////////
//////////RANDOM TEST//////////
///////////////////////////////

void
*thread_nvm_write_random(
    void *data)
{
    long long unsigned int n = filesize / nthread / nbytes;
    long long unsigned int i;
    uint32_t tid = *((uint32_t *)data);

    clock_t start = clock();

    //TODO: fix to generate 64bit random value
    for(i = 0; i < n; i++) {
        off_t rand_pos = rand() % (filesize/nthread - nbytes * 2);
        nvm_write(tid, rand_pos , buffer, nbytes);
    }

    durations[tid-1] = ( clock() - start ) / (double) CLOCKS_PER_SEC;

    return NULL;
}

void
test_nvm_write_random()
{
    durations = new double[nthread];
    pthread_t write_thread[nthread];
    int tid[nthread];
    int i;
    double averageDuration = 0.0;

    for(i=0; i<nthread; i++) {
        tid[i] = i + 1;
        pthread_create(&write_thread[i], NULL, thread_nvm_write_random, (void *)&tid[i]);
    }
    for(i=0; i<nthread; i++) {
        pthread_join(write_thread[i], NULL);
    }

    for(i=0; i<nthread; i++) {
        averageDuration += durations[i];
    }

    averageDuration /= nthread;

    printf("average duration of %d thread : %f sec\n", nthread, averageDuration);
}
void
test_nvm_write(
    long long unsigned int file_size,
    int n_thread,
    size_t n_bytes,
    int type)
{
    //declarations
    filesize = file_size;
    nthread = n_thread;
    nbytes = n_bytes;
    srand(time(NULL));
    buffer = (char*) malloc(nbytes);
    fill_buf(buffer, nbytes);

    //nvm init
    nvm_structure_build();
    nvm_system_init();
//    print_nvm_info();

    //test
    if(type == _WRITE_APPEND_) {
        test_nvm_write_append();
    } else if(type == _WRITE_RANDOM_) {
        test_nvm_write_random();
    }

    //nvm close
    nvm_system_close();
    nvm_structure_destroy();

    free(buffer);
}
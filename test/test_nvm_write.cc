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

    for(i = 0; i < n; i++) {
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

    for(i=0; i<nthread; i++) {
        tid[i] = i + 1;
        pthread_create(&write_thread[i], NULL, thread_nvm_write_append, (void *)&tid[i]);
    }
    for(i=0; i<nthread; i++) {
        pthread_join(write_thread[i], NULL);
    }

    for(i=0; i<nthread; i++) {
        printf("file %d took %f seconds\n", i+1, durations[i]);
    }
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
        nvm_write(tid, (off_t) rand() , buffer, nbytes);
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

    for(i=0; i<nthread; i++) {
        tid[i] = i + 1;
        pthread_create(&write_thread[i], NULL, thread_nvm_write_append, (void *)&tid[i]);
    }
    for(i=0; i<nthread; i++) {
        pthread_join(write_thread[i], NULL);
    }

    for(i=0; i<nthread; i++) {
        printf("file %d took %f seconds\n", i+1, durations[i]);
    }
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
    print_nvm_info();

    //test
    if(type == _WRITE_APPEND_) {
        test_nvm_write_append();
    } else if(type == _WRITE_RANDOM_) {
        test_nvm_write_random();
    }
    
    //nvm close
    nvm_system_close();
    nvm_structure_destroy();
}

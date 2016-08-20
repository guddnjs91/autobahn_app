#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include "test.h"
#include "nvm0nvm.h"


static double TimeSpecToSeconds(struct timespec* ts)
{
    return (double)ts->tv_sec + (double)ts->tv_nsec / 1000000000.0;
}

/**
 * Write thread fills in buffer and write it to nvm */
void
*thread_nvm_durable_write_append(
    void *data)
{
    long long unsigned int n = filesize / nthread / nbytes;
    long long unsigned int i;
    uint32_t tid = *((uint32_t *)data);

    for(i = 0; i < n; i++)
    {
        nvm_durable_write(tid, (off_t) (i * nbytes) , buffer, nbytes);
    }

    return NULL;
}

void
test_nvm_durable_write_append()
{
    pthread_t write_thread[nthread];
    int tid[nthread];
    int i;

    for(i=0; i<nthread; i++) {
        tid[i] = i + 1;
        pthread_create(&write_thread[i], NULL, thread_nvm_durable_write_append, (void *)&tid[i]);
    }

    for(i=0; i<nthread; i++) {
        pthread_join(write_thread[i], NULL);
    }
}

void
*thread_nvm_durable_write_random(
    void *data)
{
    long long unsigned int n = filesize / nthread / nbytes;
    long long unsigned int i;
    uint32_t tid = *((uint32_t *)data);

    //TODO: fix to generate 64bit random value
    for(i = 0; i < n; i++) {
        off_t rand_pos = rand() % (filesize/nthread - nbytes * 2);
        nvm_durable_write(tid, rand_pos , buffer, nbytes);
    }

    return NULL;
}

void
test_nvm_durable_write_random()
{
    pthread_t write_thread[nthread];
    int tid[nthread];
    int i;

    for(i=0; i<nthread; i++) {
        tid[i] = i + 1;
        pthread_create(&write_thread[i], NULL, thread_nvm_durable_write_random, (void *)&tid[i]);
    }
    for(i=0; i<nthread; i++) {
        pthread_join(write_thread[i], NULL);
    }
}
void
test_nvm_durable_write(
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

    struct timespec start;
    struct timespec end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    //test
    if(type == _WRITE_APPEND_) {
        test_nvm_durable_write_append();
    } else if(type == _WRITE_RANDOM_) {
        test_nvm_durable_write_random();
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time = TimeSpecToSeconds(&end) - TimeSpecToSeconds(&start);

    //nvm close
    nvm_system_close();
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time2 = TimeSpecToSeconds(&end) - TimeSpecToSeconds(&start);

    printf("\t>>>>>>>>> total time after write finished: %f sec, time after system close(): %f sec <<<<<<<<<<\n\n", time, time2);
   
    nvm_structure_destroy();

    free(buffer);
}

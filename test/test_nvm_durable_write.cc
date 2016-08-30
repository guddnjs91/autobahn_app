#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include "test.h"
#include "nvm0nvm.h"

/**
 * Write thread fills in buffer and write it to nvm */
void
*thread_nvm_durable_write_append(
    void *data)
{
    uint64_t n = TOTAL_FILE_SIZE / nthread / BYTES_PER_WRITE;
    uint32_t tid = *((uint32_t *)data);

    for(uint64_t i = 0; i < n; i++) {
        nvm_durable_write(tid, (off_t) (i * BYTES_PER_WRITE) , buffer, BYTES_PER_WRITE);
    }

    return NULL;
}

void
test_nvm_durable_write_append()
{
    pthread_t write_thread[nthread];
    int tid[nthread];

    for(int i = 0; i < nthread; i++) {
        tid[i] = i + 1;
        pthread_create(&write_thread[i], NULL, thread_nvm_durable_write_append, (void *)&tid[i]);
    }

    for(int i = 0; i < nthread; i++) {
        pthread_join(write_thread[i], NULL);
    }
}

void
*thread_nvm_durable_write_random(
    void *data)
{
    uint64_t n = TOTAL_FILE_SIZE / nthread / BYTES_PER_WRITE;
    uint32_t tid = *((uint32_t *)data);

    srand(time(NULL));

    //TODO: fix to generate 64bit random value
    for(uint64_t i = 0; i < n; i++) {
        off_t rand_pos = rand() % (TOTAL_FILE_SIZE / nthread - BYTES_PER_WRITE * 2);
        nvm_durable_write(tid, rand_pos , buffer, BYTES_PER_WRITE);
    }

    return NULL;
}

void
test_nvm_durable_write_random()
{
    pthread_t write_thread[nthread];
    int tid[nthread];

    for(int i = 0; i < nthread; i++) {
        tid[i] = i + 1;
        pthread_create(&write_thread[i], NULL, thread_nvm_durable_write_random, (void *)&tid[i]);
    }
    for(int i = 0; i < nthread; i++) {
        pthread_join(write_thread[i], NULL);
    }
}
void
test_nvm_durable_write(
    int n_thread)
{
    nthread = n_thread;
    buffer = (char*) malloc(BYTES_PER_WRITE);
    fill_buf(buffer, BYTES_PER_WRITE);

    nvm_structure_build();
    nvm_system_init();
//  print_nvm_info();

    struct timespec start;
    struct timespec end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    //test
    if(WRITE_MODE == WRITE_MODE_APPEND) {
        test_nvm_durable_write_append();
    } else if(WRITE_MODE == WRITE_MODE_RANDOM) {
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

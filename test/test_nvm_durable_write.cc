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
void *thread_nvm_durable_write_append(void *data)
{
    uint64_t n = TOTAL_FILE_SIZE / kNumThread / BYTES_PER_WRITE;
    uint32_t tid = *((uint32_t *)data);

    for (uint64_t i = 0; i < n; i++) {
        nvm_durable_write(tid, (off_t) (i * BYTES_PER_WRITE) , buffer, BYTES_PER_WRITE);
    }

    return NULL;
}

void test_nvm_durable_write_append()
{
    pthread_t write_thread[kNumThread];
    int tid[kNumThread];

    for (uint32_t i = 0; i < kNumThread; i++) {
        tid[i] = i + 1;
        pthread_create(&write_thread[i], NULL, thread_nvm_durable_write_append, (void *)&tid[i]);
    }

    for (uint32_t i = 0; i < kNumThread; i++) {
        pthread_join(write_thread[i], NULL);
    }
}

void *thread_nvm_durable_write_random(void *data)
{
    uint64_t n = TOTAL_FILE_SIZE / kNumThread / BYTES_PER_WRITE;
    uint32_t tid = *((uint32_t *)data);

    srand(time(NULL));

    //TODO: fix to generate 64bit random value
    for (uint64_t i = 0; i < n; i++) {
        off_t rand_pos = rand() - BYTES_PER_WRITE;//% (TOTAL_FILE_SIZE / kNumThread - BYTES_PER_WRITE * 2);
        rand_pos = rand_pos - (rand_pos % BYTES_PER_WRITE);
        nvm_durable_write(tid, rand_pos , buffer, BYTES_PER_WRITE);
//        printf("TID%d: randon test running %ld out of %ld\n", tid, i, n);
    }

    return NULL;
}

void test_nvm_durable_write_random()
{
    pthread_t write_thread[kNumThread];
    int tid[kNumThread];

    for (uint32_t i = 0; i < kNumThread; i++) {
        tid[i] = i + 1;
        pthread_create(&write_thread[i], NULL, thread_nvm_durable_write_random, (void *)&tid[i]);
    }
    for (uint32_t i = 0; i < kNumThread; i++) {
        pthread_join(write_thread[i], NULL);
    }
}

void test_nvm_durable_write()
{
    // declarations
    struct timespec start;
    struct timespec end;

    buffer = (char*) malloc(BYTES_PER_WRITE);
    fill_buf(buffer, BYTES_PER_WRITE);

    nvm_structure_build();
    nvm_system_init();
//  print_nvm_info();

    // TODO: random
    if (WRITE_MODE == WRITE_MODE_RANDOM || WRITE_MODE == WRITE_MODE_SKEWED) {
        printf("Random Test: appending to a new file...\n");
        test_nvm_durable_write_append();
        nvm_system_close();
        nvm_structure_destroy();
        nvm_structure_build();
        nvm_system_init();
        printf("Random Test: Random test start\n");
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    if (WRITE_MODE == WRITE_MODE_APPEND) {
        test_nvm_durable_write_append();
    } else if (WRITE_MODE == WRITE_MODE_RANDOM) {
        test_nvm_durable_write_random();
    } else if (WRITE_MODE == WRITE_MODE_SKEWED) {
        //test_nvm_durable_write_skewed();
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    kTimeSec = TimeSpecToSeconds(&end) - TimeSpecToSeconds(&start);

    nvm_system_close();

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_after_sync = TimeSpecToSeconds(&end) - TimeSpecToSeconds(&start);

    printf("\t>>>>>>>>> total time after write finished: %f sec, time after system close(): %f sec <<<<<<<<<<\n\n", kTimeSec, time_after_sync);
   
    nvm_structure_destroy();

    free(buffer);

    remove_files(kNumThread);
}

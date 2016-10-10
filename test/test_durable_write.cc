#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include "test.h"

using namespace std;

/**
 * Write thread fills in buffer and write it to nvm */
void *thread_durable_write_append(void *data)
{
    uint64_t n = TOTAL_FILE_SIZE / kNumThread / BYTES_PER_WRITE;
    uint32_t tid = *((uint32_t *)data);

    int fd = open(("./VOL_" + to_string(tid) + ".txt").c_str(), O_RDWR | O_CREAT, 0666);

    for (uint64_t i = 0; i < n; i++) {
        write(fd, buffer, BYTES_PER_WRITE);
        fsync(fd);
        fsync(fd);
    }

    close(fd);

    return nullptr;
}

void test_durable_write_append()
{
    pthread_t write_thread[kNumThread];
    int tid[kNumThread];

    for (uint32_t i = 0; i < kNumThread; i++) {
        tid[i] = i + 1;
        pthread_create(&write_thread[i], NULL, thread_durable_write_append, (void *)&tid[i]);
    }

    for (uint32_t i = 0; i < kNumThread; i++) {
        pthread_join(write_thread[i], NULL);
    }
}

void *thread_durable_write_random(void *data)
{
    uint64_t n = TOTAL_FILE_SIZE / kNumThread / BYTES_PER_WRITE;
    uint32_t tid = *((uint32_t *)data);

    int fd = open(("./VOL_" + to_string(tid) + ".txt").c_str(), O_RDWR | O_CREAT, 0666);

    srand(time(NULL));

    //TODO: fix to generate 64bit random value
    for (uint64_t i = 0; i < n; i++) {
        off_t rand_pos = rand() % (TOTAL_FILE_SIZE / kNumThread - BYTES_PER_WRITE * 2);
        lseek(fd, rand_pos, SEEK_SET);
        write(fd, buffer, BYTES_PER_WRITE);
        fsync(fd);
        fsync(fd);
    }

    close(fd);

    return nullptr;
}

void test_durable_write_random()
{
    pthread_t write_thread[kNumThread];
    int tid[kNumThread];

    for (uint32_t i = 0; i < kNumThread; i++) {
        tid[i] = i + 1;
        pthread_create(&write_thread[i], NULL, thread_durable_write_random, (void *)&tid[i]);
    }

    for (uint32_t i = 0; i < kNumThread; i++) {
        pthread_join(write_thread[i], NULL);
    }
}

void test_durable_write()
{
    //declarations
    struct timespec start;
    struct timespec end;

    buffer = (char*) malloc(BYTES_PER_WRITE);
    fill_buf(buffer, BYTES_PER_WRITE);

    // for random tests
    if (WRITE_MODE == WRITE_MODE_RANDOM || WRITE_MODE == WRITE_MODE_SKEWED) {
        printf("Random Test: appending to a new file...\n");
        test_durable_write_append();
        printf("Random Test: Random test start\n");
    }

    //test
    clock_gettime(CLOCK_MONOTONIC, &start);

    if (WRITE_MODE == WRITE_MODE_APPEND) {
        test_durable_write_append();
    } else if (WRITE_MODE == WRITE_MODE_RANDOM) {
        test_durable_write_random();
    } else if (WRITE_MODE == WRITE_MODE_SKEWED) {
        //test_durable_write_skewed();
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    kTimeSec = TimeSpecToSeconds(&end) - TimeSpecToSeconds(&start);

    printf("total time after write finished: %f sec\n\n", kTimeSec);

    free(buffer);

    remove_files(kNumThread);
}

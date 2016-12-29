#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include "test.h"
#include "ut0random.h"
#include "nvm0volume.h"

using namespace std;

extern Random *g_rand_obj;

/**
 * Write thread fills in buffer and write it to nvm */
void *thread_durable_write_append(void *data)
{
    uint64_t n = TOTAL_FILE_SIZE / kNumThread / BYTES_PER_WRITE;
    uint32_t tid = *((uint32_t *)data);
    int fd;

    fd = open( get_filename(tid), O_RDWR | O_CREAT | O_DIRECT | O_SYNC, 0666);

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
    int fd;

    fd = open( get_filename(tid), O_RDWR | O_CREAT | O_DIRECT | O_SYNC, 0666);

    for (uint64_t i = 0; i < n; i++) {
        off_t pos = g_rand_obj->unif_rand64() % n;
        lseek(fd, pos * BYTES_PER_WRITE, SEEK_SET);
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

void *thread_durable_write_skewed(void *data)
{
    uint64_t n = TOTAL_FILE_SIZE / kNumThread / BYTES_PER_WRITE;
    uint32_t tid = *((uint32_t *)data);
    int fd;
    
    fd = open( get_filename(tid), O_RDWR | O_CREAT | O_DIRECT | O_SYNC, 0666);

    for (uint64_t i = 0; i < n; i++) {
        off_t pos = g_rand_obj->skew_rand64() % n;
        lseek(fd, pos * BYTES_PER_WRITE, SEEK_SET);
        write(fd, buffer, BYTES_PER_WRITE);
        fsync(fd);
        fsync(fd);
    }

    close(fd);

    return nullptr;
}

void test_durable_write_skewed()
{
    pthread_t write_thread[kNumThread];
    int tid[kNumThread];

    for (uint32_t i = 0; i < kNumThread; i++) {
        tid[i] = i + 1;
        pthread_create(&write_thread[i], NULL, thread_durable_write_skewed, (void *)&tid[i]);
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
    g_rand_obj = new Random();
    if (WRITE_MODE == WRITE_MODE_RANDOM || WRITE_MODE == WRITE_MODE_SKEWED) {
        g_rand_obj->skew_init(THETA, TOTAL_FILE_SIZE / kNumThread / BYTES_PER_WRITE);
        printf("Random Test: appending to a new file...\n");
        fill_buf_append(buffer, BYTES_PER_WRITE);
        test_durable_write_append();
        fill_buf_random(buffer, BYTES_PER_WRITE);
        printf("Random Test: Random test start\n");
    }

    //test
    clock_gettime(CLOCK_MONOTONIC, &start);

    if (WRITE_MODE == WRITE_MODE_APPEND) {
        test_durable_write_append();
    } else if (WRITE_MODE == WRITE_MODE_RANDOM) {
        test_durable_write_random();
    } else if (WRITE_MODE == WRITE_MODE_SKEWED) {
        test_durable_write_skewed();
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    kTimeSec = TimeSpecToSeconds(&end) - TimeSpecToSeconds(&start);

    printf("total time after write finished: %f sec\n\n", kTimeSec);

    free(buffer);

    remove_files(kNumThread);
}

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <string>
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
void *thread_write_append(void *data)
{
    uint64_t n = TOTAL_FILE_SIZE / kNumThread / BYTES_PER_WRITE;
    uint32_t tid = *((uint32_t *)data);
    int fd;

    fd = open( get_filename(tid), O_RDWR | O_CREAT, 0666);

    for (uint64_t i = 0; i < n; i++) {
        write(fd, buffer, BYTES_PER_WRITE);
    }

    close(fd);

    return nullptr;
}

void test_write_append()
{
    pthread_t write_thread[kNumThread];
    int tid[kNumThread];

    for (uint32_t i = 0; i < kNumThread; i++) {
        tid[i] = i + 1;
        pthread_create(&write_thread[i], NULL, thread_write_append, (void *)&tid[i]);
    }

    for (uint32_t i = 0; i < kNumThread; i++) {
        pthread_join(write_thread[i], NULL);
    }
}

void *thread_write_random(void *data)
{
    uint64_t n = TOTAL_FILE_SIZE / kNumThread / BYTES_PER_WRITE;
    uint32_t tid = *((uint32_t *)data);
    int fd;

    fd = open( get_filename(tid), O_RDWR | O_CREAT, 0666);

    for (uint64_t i = 0; i < n; i++) {
        off_t pos = g_rand_obj->unif_rand64() % n;
        lseek(fd, pos * BYTES_PER_WRITE, SEEK_SET);
        write(fd, buffer, BYTES_PER_WRITE);
    }

    close(fd);

    return nullptr;
}

void test_write_random()
{
    pthread_t write_thread[kNumThread];
    int tid[kNumThread];

    for (uint32_t i = 0; i < kNumThread; i++) {
        tid[i] = i + 1;
        pthread_create(&write_thread[i], NULL, thread_write_random, (void *)&tid[i]);
    }

    for (uint32_t i = 0; i < kNumThread; i++) {
        pthread_join(write_thread[i], NULL);
    }
}

void *thread_write_skewed(void *data)
{
    uint64_t n = TOTAL_FILE_SIZE / kNumThread / BYTES_PER_WRITE;
    uint32_t tid = *((uint32_t *)data);
    int fd;

    fd = open( get_filename(tid), O_RDWR | O_CREAT, 0666);

    for (uint64_t i = 0; i < n; i++) {
        off_t pos = g_rand_obj->skew_rand64() % n;
        lseek(fd, pos * BYTES_PER_WRITE, SEEK_SET);
        write(fd, buffer, BYTES_PER_WRITE);
    }

    close(fd);

    return nullptr;
}

void test_write_skewed()
{
    pthread_t write_thread[kNumThread];
    int tid[kNumThread];

    for (uint32_t i = 0; i < kNumThread; i++) {
        tid[i] = i + 1;
        pthread_create(&write_thread[i], NULL, thread_write_skewed, (void *)&tid[i]);
    }

    for (uint32_t i = 0; i < kNumThread; i++) {
        pthread_join(write_thread[i], NULL);
    }
}


void test_write()
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
        test_write_append();
        fill_buf_random(buffer, BYTES_PER_WRITE);
        printf("Random Test: Random test start\n");
    }

    //test
    clock_gettime(CLOCK_MONOTONIC, &start);
     
    if (WRITE_MODE == WRITE_MODE_APPEND) {
        test_write_append();
    } else if (WRITE_MODE == WRITE_MODE_RANDOM) {
        test_write_random();
    } else if (WRITE_MODE == WRITE_MODE_SKEWED) {
        test_write_skewed();
    }

    clock_gettime(CLOCK_MONOTONIC, &end); 
    kTimeSec = TimeSpecToSeconds(&end) - TimeSpecToSeconds(&start);
    
    sync();
    sync();

    clock_gettime(CLOCK_MONOTONIC, &end); 
    double time_after_sync = TimeSpecToSeconds(&end) - TimeSpecToSeconds(&start);

    printf("\t>>>>>>>>> total time after write finished: %f sec, time after sync finished: %f sec <<<<<<<<<<\n\n", kTimeSec, time_after_sync);

    free(buffer);

    remove_files(kNumThread);
    
    delete g_rand_obj;
}

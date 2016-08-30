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
using namespace std;

/**
 * Write thread fills in buffer and write it to nvm */
void
*thread_write_append(
   void *data)
{
    uint64_t n = TOTAL_FILE_SIZE / nthread / BYTES_PER_WRITE;
    uint32_t tid = *((uint32_t *)data);
    int fd;

    if(tid % 2 == 1) {
        fd = open( ("/opt/nvm1/NVM/VOL_" + to_string(tid) + ".txt").c_str(), O_RDWR | O_CREAT, 0666);
    } else {
        fd = open( ("/opt/nvm2/NVM/VOL_" + to_string(tid) + ".txt").c_str(), O_RDWR | O_CREAT, 0666);
    }

    for(uint64_t i = 0; i < n; i++) {
        write(fd, buffer, BYTES_PER_WRITE);
    }

    close(fd);

    return nullptr;
}

void
test_write_append()
{
    pthread_t write_thread[nthread];
    int tid[nthread];

    for(int i = 0; i < nthread; i++) {
        tid[i] = i + 1;
        pthread_create(&write_thread[i], NULL, thread_write_append, (void *)&tid[i]);
    }

    for(int i = 0; i < nthread; i++) {
        pthread_join(write_thread[i], NULL);
    }
}

void
*thread_write_random(
    void *data)
{
    uint64_t n = TOTAL_FILE_SIZE / nthread / BYTES_PER_WRITE;
    uint32_t tid = *((uint32_t *)data);
    int fd;

    if(tid % 2 == 1) {
        fd = open( ("/opt/nvm1/NVM/VOL_" + to_string(tid) + ".txt").c_str(), O_RDWR | O_CREAT, 0666);
    } else {
        fd = open( ("/opt/nvm2/NVM/VOL_" + to_string(tid) + ".txt").c_str(), O_RDWR | O_CREAT, 0666);
    }

    srand(time(NULL));

    //TODO: fix to generate 64bit random value
    for(uint64_t i = 0; i < n; i++) {
        off_t rand_pos = rand() % (TOTAL_FILE_SIZE / nthread - BYTES_PER_WRITE * 2);
        lseek(fd, rand_pos, SEEK_SET);
        write(fd, buffer, BYTES_PER_WRITE);
    }

    close(fd);

    return nullptr;
}

void
test_write_random()
{
    pthread_t write_thread[nthread];
    int tid[nthread];

    for(int i = 0; i < nthread; i++) {
        tid[i] = i + 1;
        pthread_create(&write_thread[i], NULL, thread_write_random, (void *)&tid[i]);
    }

    for(int i = 0; i < nthread; i++) {
        pthread_join(write_thread[i], NULL);
    }
}

void
test_write(
    int n_thread)
{
    //declarations
    struct timespec start;
    struct timespec end;

    nthread = n_thread;
    buffer = (char*) malloc(BYTES_PER_WRITE);
    fill_buf(buffer, BYTES_PER_WRITE);

     //test
     clock_gettime(CLOCK_MONOTONIC, &start);
     
     if(WRITE_MODE == WRITE_MODE_APPEND) {
         test_write_append();
     } else if(WRITE_MODE == WRITE_MODE_RANDOM) {
         test_write_random();
     }

    clock_gettime(CLOCK_MONOTONIC, &end); 
    double time = TimeSpecToSeconds(&end) - TimeSpecToSeconds(&start);
    
    sync();
    sync();

    clock_gettime(CLOCK_MONOTONIC, &end); 
    double time2 = TimeSpecToSeconds(&end) - TimeSpecToSeconds(&start);

    printf("\t>>>>>>>>> total time after write finished: %f sec, time after sync finished: %f sec <<<<<<<<<<\n\n", time, time2);

    free(buffer);
}

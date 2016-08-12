#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <string>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include "nvm0common.h"
#include "test.h"

static double TimeSpecToSeconds(struct timespec* ts)
{
    return (double)ts->tv_sec + (double)ts->tv_nsec / 1000000000.0;
}

/**
 * Write thread fills in buffer and write it to nvm */
void
*thread_write_append(
   void *data)
{
    long long unsigned int n = filesize / nthread / nbytes;
    long long unsigned int i;
    uint32_t tid = *((uint32_t *)data);

    int fd = open( ("VOL_" + to_string(tid) + ".txt").c_str(), O_RDWR | O_CREAT, 0666);

    for(i = 0; i < n; i++)
    {
        write(fd, buffer, nbytes);
    }


    close(fd);
}

void
test_write_append()
{
    pthread_t write_thread[nthread];
    int tid[nthread];
    int i;

    printf("%u * Thread writes %u Bytes * %llu to each VOL_X.txt\n", nthread, nbytes,filesize / nthread / nbytes);
    
    struct timespec start;
    struct timespec end;
    double duration = 0;

    clock_gettime(CLOCK_MONOTONIC, &start);

    for(i=0; i<nthread; i++) {
        tid[i] = i + 1;
        pthread_create(&write_thread[i], NULL, thread_write_append, (void *)&tid[i]);
    }

    for(i=0; i<nthread; i++) {
        pthread_join(write_thread[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    duration = TimeSpecToSeconds(&end) - TimeSpecToSeconds(&start);

    printf("duration of %d thread : %f sec\n",nthread, duration);
}

void
*thread_write_random(
    void *data)
{
    long long unsigned int n = filesize / nthread / nbytes;
    long long unsigned int i;
    uint32_t tid = *((uint32_t *)data);

    int fd = open(("VOL_" + to_string(tid) + ".txt").c_str(), O_RDWR | O_CREAT, 0666);

    //TODO: fix to generate 64bit random value
    for(i = 0; i < n; i++) {
        off_t rand_pos = rand() % (filesize/nthread - nbytes * 2);
        lseek(fd, rand_pos, SEEK_SET);
        write(fd, buffer, nbytes);
        //printf("thread %u writes %u Bytes to VOL_%u.txt : %llu / %llu\r", tid, nbytes, tid, i, n);
    }

    close(fd);
}

void
test_write_random()
{
    pthread_t write_thread[nthread];
    int tid[nthread];
    int i;

    for(i=0; i<nthread; i++) {
        tid[i] = i + 1;
        pthread_create(&write_thread[i], NULL, thread_write_random, (void *)&tid[i]);
    }

    struct timespec start;
    struct timespec end;
    double duration = 0;

    clock_gettime(CLOCK_MONOTONIC, &start);

    for(i=0; i<nthread; i++) {
        pthread_join(write_thread[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    duration = TimeSpecToSeconds(&end) - TimeSpecToSeconds(&start);

    printf("duration of %d thread : %f sec\n",nthread, duration);
}

void
test_write(
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

    //test
    if(type == _WRITE_APPEND_) {
        test_write_append();
    } else if(type == _WRITE_RANDOM_) {
        test_write_random();
    }

    free(buffer);
}

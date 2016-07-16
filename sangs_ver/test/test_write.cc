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

long long unsigned int filesize;
int nthread;
size_t nbytes;
char* buffer;
double* durations;

/**
 * Fill in buffer with given size 
 * write each element with random characters */
void
fill_buf(
    char *buf,
    size_t size)
{
    int i;
    for(i = 0; i < (int)size - 1; i++) {
        if(rand()%10 == 0) {
            buf[i] = '\n';
        }
        else if(rand()%5 == 0) {
            buf[i] = ' ';
        } else {
            buf[i] = rand() % 26 + 'A';
        }
    }
    
    buf[size-1] = '\0';
}

///////////////////////////////
//////////APPEND TEST//////////
///////////////////////////////

/**
 * Write thread fills in buffer and write it to nvm */
void
*thread_write_append(
   void *data)
{
    long long unsigned int n = filesize / nthread / nbytes;
    long long unsigned int i;
    uint32_t tid = *((uint32_t *)data);

    clock_t start = clock();

    int fd = open( ("VOL_" + to_string(tid) + ".txt").c_str(), O_RDWR | O_CREAT, 0666);

    for(i = 0; i < n; i++) {
        write(fd, buffer, nbytes);
    }

    durations[tid-1] = ( clock() - start ) / (double) CLOCKS_PER_SEC;
}

void
test_append()
{
    durations = new double[nthread];
    pthread_t write_thread[nthread];
    int tid[nthread];
    int i;

    for(i=0; i<nthread; i++) {
        tid[i] = i + 1;
        pthread_create(&write_thread[i], NULL, thread_write_append, (void *)&tid[i]);
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
*thread_write_random(
    void *data)
{
    long long unsigned int n = filesize / nthread / nbytes;
    long long unsigned int i;
    uint32_t tid = *((uint32_t *)data);

    clock_t start = clock();

    int fd = open(("VOL_" + to_string(tid) + ".txt").c_str(), O_RDWR | O_CREAT, 0666);

    //TODO: fix to generate 64bit random value
    for(i = 0; i < n; i++) {
        lseek(fd, (off_t) rand(), SEEK_SET);
        write(tid, buffer, nbytes);
    }

    durations[tid-1] = ( clock() - start ) / (double) CLOCKS_PER_SEC;
}

void
test_random()
{
    durations = new double[nthread];
    pthread_t write_thread[nthread];
    int tid[nthread];
    int i;

    for(i=0; i<nthread; i++) {
        tid[i] = i + 1;
        pthread_create(&write_thread[i], NULL, thread_write_append, (void *)&tid[i]);
    }
    for(i=0; i<nthread; i++) {
        pthread_join(write_thread[i], NULL);
    }

    for(i=0; i<nthread; i++) {
        printf("file %d took %f seconds\n", i+1, durations[i]);
    }
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
        test_append();
    } else if(type == _WRITE_RANDOM_) {
        test_random();
    }
}
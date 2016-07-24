#include "nvm0common.h"
#include <stdlib.h>
#include <stdio.h>
#include "test.h"

long long unsigned int filesize;
int nthread;
size_t nbytes;
char* buffer;
double* durations;

void test_write(long long unsigned int, int, size_t, int);
void test_nvm_write(long long unsigned int, int, size_t, int);
void remove_files(int);

void test_write_performance(void (*test_func)(long long unsigned int, int, size_t, int) )
{
    int nthread;

    system("clear");
 
    /* write (WRITE_BYTES1) bytes at a time */
    for(nthread = 1; nthread <= MAX_THREADS; nthread*=2) {
        printf("#-------- %d threads APPEND TEST -------\n", nthread);
        (*test_func)(TOTAL_FILE_SIZE, nthread, WRITE_BYTES1, _WRITE_APPEND_);
//        printf("#-------------- RANDOM TEST ---------------------\n");
//        (*test_func)(TOTAL_FILE_SIZE, nthread, WRITE_BYTES1, _WRITE_RANDOM_);
        printf("\n");
        if(nthread == MAX_THREADS)
            break;
        remove_files(nthread);
    }

    /* write (WRITE_BYTES2) bytes at a time */
//    for(nthread = 1; nthread <= MAX_THREADS; nthread*=2) {
//        (*test_func)(TOTAL_FILE_SIZE, nthread, WRITE_BYTES2, _WRITE_APPEND_);
//        (*test_func)(TOTAL_FILE_SIZE, nthread, WRITE_BYTES2, _WRITE_RANDOM_);
//        remove_files(nthread);
//    }
}

int main()
{
    printf("\n[write test]\n");
    test_write_performance(test_write);
    remove_files(MAX_THREADS);
    printf("\n[nvm write test]\n");
    test_write_performance(test_nvm_write);
}

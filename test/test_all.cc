#include "nvm0common.h"
#include <stdio.h>
#include "test.h"

void test_write(long long unsigned int, int, size_t, int);
void test_nvm_write(long long unsigned int, int, size_t, int);
void remove_files(int);

void test_write_performance(void (*test_func)(long long unsigned int, int, size_t, int) )
{
    int nthread;

    //write (WRITE_BYTES1) bytes at a time
    for(nthread = 1; nthread <= MAX_THREADS; nthread*=2) {
        printf("#-------------- %d Threads ----------------------\n", nthread);
        printf("#-------------- APPEND TEST ---------------------\n");
        (*test_func)(TOTAL_FILE_SIZE, nthread, WRITE_BYTES1, _WRITE_APPEND_);
        printf("#-------------- RANDOM TEST ---------------------\n");
        (*test_func)(TOTAL_FILE_SIZE, nthread, WRITE_BYTES1, _WRITE_RANDOM_);
        printf("\n");
        remove_files(nthread);
    }

    //write (WRITE_BYTES2) bytes at a time
//    for(nthread = 1; nthread <= MAX_THREADS; nthread*=2) {
//        (*test_func)(TOTAL_FILE_SIZE, nthread, WRITE_BYTES2, _WRITE_APPEND_);
//        (*test_func)(TOTAL_FILE_SIZE, nthread, WRITE_BYTES2, _WRITE_RANDOM_);
//        remove_files();
//    }
}

int main()
{
    test_write_performance(test_write);
//    test_write_performance(test_nvm_write);
}
